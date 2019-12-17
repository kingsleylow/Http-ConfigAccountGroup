#include "MT4PumpConn.h"
#include "Logger.h"
#include <chrono>
#include <thread>
#include "Filter.h"
#include "Common.h"
#include "MailWarn.h"

std::mutex PumpConn::m_mtxSingleton;
std::mutex PumpConn::m_mtxSendData;
PumpConn* PumpConn::m_self = nullptr;

PumpConn* PumpConn::getInstance()
{
	if (nullptr == m_self)
	{
		std::lock_guard<std::mutex> lck(m_mtxSingleton);
		{
			if (nullptr == m_self)
			{
				m_self = new PumpConn;
			}
		}
	}
	return m_self;
}

PumpConn::PumpConn()
{
	m_stop = false;
	m_managerInterBak = nullptr;
	m_managerInter = nullptr;
}


PumpConn::~PumpConn()
{
	m_stop = true;
}

bool PumpConn::mt4Release()
{
	if (m_managerInter != nullptr)
	{
		m_managerInter->Release();
		m_managerInter = nullptr;
	}

	if (m_managerInterBak != nullptr)
	{
		m_managerInterBak->Release();
		m_managerInterBak = nullptr;
	}
	return true;
}

bool PumpConn::init()
{
	std::string path = Common::getInstance()->getProjectPath();
	path = path + Config::getInstance().getMT4ConnConf().find("lib")->second;
	m_factoryInter.Init(path.c_str());
	if (m_factoryInter.IsValid() == FALSE)
	{
		SPDLOG(error, "PumpConn mt4 lib init failed");
		return false;
	}
	if (m_factoryInter.WinsockStartup() != RET_OK)
	{
		SPDLOG(error, "PumpConn mt4 lib init winsock lib failed.");
		return false;
	}

	return true;
}

bool PumpConn::mt4Init(CManagerInterface*& inter)
{

	if (NULL == (inter = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "PumpConn mt4 factory create mananger interface failed.");
		return false;
	}

	Args arg;
	arg.inter = inter;
	arg.pump = this;
	m_inter[inter] = arg;
	return true;
}

bool PumpConn::mt4Login(const int login, const char* passwd, CManagerInterface*& inter)
{
	if (!mt4Init(inter))
		return false;
	if (RET_OK != mt4Conn(Config::getInstance().getMT4ConnConf().find("host")->second.c_str(), inter))
		return false;
	int cnt = 0;
	while (cnt < 3)
	{
		if (RET_OK != inter->Login(login, passwd))
		{
			SPDLOG(error, "PumpConn mt4 server login failed.---{}", cnt+1);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "PumpConn 3 times of login to mt4 server failed.");
		return false;
	}

	if (RET_OK != inter->PumpingSwitchEx(NotifyCallBack, 0, &m_inter[inter]))
	{
		SPDLOG(error, "pumpswitch failed.")
		return false;
	}
		
	return true;
}

int PumpConn::mt4Conn(const char* host, CManagerInterface*& inter)
{
	int cnt = 0;
	while (cnt < 3)
	{
		if (inter->Connect(host) != RET_OK)
		{
			SPDLOG(error, "PumpConn try to connect to mt4 server failed---{}", cnt + 1);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "PumpConn 3 times of connecting to mt4 server failed.");
		return RET_NO_CONNECT;
	}
	return RET_OK;
}

void PumpConn::NotifyCallBack(int code, int type, void* data, void *param)
{
	Args* ptrArgs = (Args*)param;
	TradeRecord* tradeRecords = nullptr;
	int orders = 0;
	SymbolInfo si[32];
	int quoteCount = 0;

	ConSymbol *cs = NULL;
	int csCount = 0;

	ConGroup *cg = NULL;
	int cgCount = 0;

	MarginLevel* ml = nullptr;
	int mlCount = 0;
	switch (code)
	{
	case PUMP_UPDATE_BIDASK:
		while ((quoteCount = ptrArgs->inter->SymbolInfoUpdated(si, 32)) > 0)
		{
			ptrArgs->pump->storeQuotes(si, quoteCount);
		}
		break;
	case PUMP_UPDATE_SYMBOLS:
		cs = ptrArgs->inter->SymbolsGetAll(&csCount);
		if (cs != NULL && csCount > 0)
		{
			ptrArgs->pump->storeConSymbol(cs, csCount);
			ptrArgs->inter->MemFree(cs);
			cs = NULL;
		}
		break;
	case PUMP_UPDATE_GROUPS:
		cg = ptrArgs->inter->GroupsGet(&cgCount);
		if (cg != NULL && cgCount > 0)
		{
			ptrArgs->pump->storeConGroup(cg, cgCount);
			ptrArgs->inter->MemFree(cg);
			cg = NULL;
		}
		break;
	default:
		break;
	}

}

void PumpConn::storeConGroup(ConGroup* cg, int size)
{
	std::lock_guard<std::mutex> lock(m_mtxGroup);
	for (auto i = 0; i < size; i++)
	{
		m_conGroup[cg[i].group] = cg[i];
	}
}

bool PumpConn::symbolAdd(std::string symbol)
{
	int res = 0;
	std::lock_guard<std::mutex> lock(m_mtxPump);
	if (RET_OK != (res = m_managerInter->SymbolAdd(symbol.c_str())))
	{
		if (RET_OK != (res = m_managerInterBak->SymbolAdd(symbol.c_str())))
		{
			Logger::getInstance()->error("SymbolAdd failed, error {}", m_managerInterBak->ErrorDescription(res));
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool PumpConn::getConGroup(std::string group, ConGroup& cg)
{
	std::lock_guard<std::mutex> lock(m_mtxGroup);
	if (m_conGroup.find(group) != m_conGroup.end())
	{
		cg = m_conGroup[group];
		return true;
	}
	else
	{
		int res = RET_OK;
		if (RET_OK != (res = m_managerInter->GroupRecordGet(group.c_str(), &cg)))
		{
			if (RET_OK != (res = m_managerInterBak->GroupRecordGet(group.c_str(), &cg)))
			{
				Logger::getInstance()->error("GroupRecordGet failed, {}", m_managerInterBak->ErrorDescription(res));
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
}

void PumpConn::storeConSymbol(ConSymbol *cs, int size)
{
	std::lock_guard<std::mutex> lock(m_mtxSymbol);
	for (auto i = 0; i < size; i++)
	{
		m_conSymbol[cs[i].symbol] = cs[i];
	}
}

bool PumpConn::getConSymbol(std::string symbol, ConSymbol& cs)
{
	std::lock_guard<std::mutex> lock(m_mtxSymbol);
	if (m_conSymbol.find(symbol) != m_conSymbol.end())
	{
		cs = m_conSymbol[symbol];
		return true;
	}
	else
	{
		return false;
	}
}

void PumpConn::storeQuotes(SymbolInfo *si, int size)
{
	std::lock_guard<std::mutex> lock(m_mtxQuotes);
	for (auto i = 0; i < size; i++)
	{
		m_quotes[si[i].symbol] = si[i];
	}
}

bool PumpConn::getSymbolInfo(std::string symbol, SymbolInfo& si)
{
	std::lock_guard<std::mutex> lock(m_mtxQuotes);
	if (m_quotes.find(symbol) != m_quotes.end())
	{
		si = m_quotes[symbol];
		return true;
	}
	else
	{
		int res = RET_OK;
		std::lock_guard<std::mutex> lock(m_mtxPump);
		if (RET_OK != (res = m_managerInter->SymbolInfoGet(symbol.c_str(), &si)))
		{
			if (RET_OK != (res = m_managerInterBak->SymbolInfoGet(symbol.c_str(), &si)))
			{
				Logger::getInstance()->error("SymbolInfoGet failed, {}", m_managerInterBak->ErrorDescription(res));
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
}

bool PumpConn::getQuotes(std::string symbol, double& bid, double& ask)
{
	std::lock_guard<std::mutex> lock(m_mtxQuotes);
	if (m_quotes.find(symbol) != m_quotes.end())
	{
		bid = m_quotes[symbol].bid;
		ask = m_quotes[symbol].ask;
		return true;
	}
	else
	{
		return false;
	}
}

bool PumpConn::createConnToMT4(CManagerInterface*& inter)
{
	std::lock_guard<std::mutex> lock(m_mtxPump);
	if (inter != nullptr)
	{
		inter->Disconnect();
		inter->Release();
		inter = nullptr;
	}
	std::string login = Config::getInstance().getMT4ConnConf().find("login")->second;
	std::string pass = Config::getInstance().getMT4ConnConf().find("passwd")->second;

	if (mt4Login(std::stoi(login), pass.c_str(), inter))
		return true;
	else
		return false;
}

bool PumpConn::getUserRecord(const int login, UserRecord& ur)
{
	std::lock_guard<std::mutex> lock(m_mtxPump);
	if (nullptr == m_managerInter || m_managerInter->IsConnected() == 0)
		return false;
	int res = m_managerInter->UserRecordGet(login, &ur);
	if (res != RET_OK)
	{
		if (RET_OK != (res = m_managerInterBak->UserRecordGet(login, &ur)))
		{
			Logger::getInstance()->error("getUserRecord error:{}", m_managerInterBak->ErrorDescription(res));
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool PumpConn::createConnToMT4()
{
	bool connectionStatus = true;
	std::string login = Config::getInstance().getMT4ConnConf().find("login")->second;
	std::string pass = Config::getInstance().getMT4ConnConf().find("passwd")->second;

	if (mt4Login(std::stoi(login), pass.c_str(), m_managerInter) &&
		mt4Login(std::stoi(login), pass.c_str(), m_managerInterBak))
	{

	}
	else
	{
		connectionStatus = connectionStatus && false;
	}
	monitorConn();
	return connectionStatus;
}

TradeRecord* PumpConn::tradesGetByLogin(const int login, std::string group, int& total)
{
	std::lock_guard<std::mutex> lock(m_mtxPump);
	return m_managerInter->TradesGetByLogin(login, group.c_str(), &total);
}

void PumpConn::monitorConn()
{
	m_thdMonitor = std::thread([this]()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));

			if ((nullptr == m_managerInter) ||
				(nullptr != m_managerInter && 0 == m_managerInter->IsConnected()))
			{
				Logger::getInstance()->warn("PumpConn master disconnected to mt4. will try to connect to mt4.");

				if (createConnToMT4(m_managerInter))
				{
					Logger::getInstance()->info("PumpConn master connect to mt4 success.");
					MailWarn::getInstance()->SendWarnMail("PumpConn", "PumpConn master connect to mt4 success.");
				}
				else
				{
					Logger::getInstance()->info("PumpConn master connect to mt4 failed.");
					MailWarn::getInstance()->SendWarnMail("PumpConn", "PumpConn master connect to mt4 failed.");
				}
			}

			if ((nullptr == m_managerInterBak)||
				(nullptr != m_managerInterBak && 0 == m_managerInterBak->IsConnected()))
			{
				Logger::getInstance()->warn("PumpConn slave disconnected to mt4. will try to connect to mt4.");

				if (createConnToMT4(m_managerInterBak))
				{
					Logger::getInstance()->info("PumpConn slave connect to mt4 bak success.");
					MailWarn::getInstance()->SendWarnMail("PumpConn", "PumpConn slave connect to mt4 success.");
				}
				else
				{
					Logger::getInstance()->info("PumpConn slave connect to mt4 bak failed.");
					MailWarn::getInstance()->SendWarnMail("PumpConn", "PumpConn slave connect to mt4 failed.");
				}
			}
			if (m_stop)
				break;
		}
	});
	m_thdMonitor.detach();
}