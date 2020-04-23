#include "MT4DirectConn.h"
#include "Logger.h"
#include <chrono>
#include <thread>
#include "Filter.h"
#include "Common.h"
#include "MailWarn.h"

std::mutex DirectConn::m_mtxSingleton;
std::mutex DirectConn::m_mtxSendData;
DirectConn* DirectConn::m_self = nullptr;

DirectConn* DirectConn::getInstance()
{
	if (nullptr == m_self)
	{
		std::lock_guard<std::mutex> lck(m_mtxSingleton);
		{
			if (nullptr == m_self)
			{
				m_self = new DirectConn;
			}
		}
	}
	return m_self;
}

DirectConn::DirectConn()
{
	m_stop = false;
	m_managerInter = nullptr;
	m_managerInterBak = nullptr;
}


DirectConn::~DirectConn()
{
	m_stop = true;
}

bool DirectConn::mt4Release()
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

bool DirectConn::init()
{
	std::string path = Common::getInstance()->getProjectPath();
	path = path + Config::getInstance().getMT4ConnConf().find("direct")->second;
	m_factoryInter.Init(path.c_str());
	if (m_factoryInter.IsValid() == FALSE)
	{
		SPDLOG(error, "DirectConn mt4 lib init failed");
		return false;
	}
	if (m_factoryInter.WinsockStartup() != RET_OK)
	{
		SPDLOG(error, "DirectConn mt4 lib init winsock lib failed.");
		return false;
	}
	return true;
}

bool DirectConn::mt4Init(CManagerInterface*& inter)
{

	if (NULL == (inter = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "DirectConn mt4 factory create mananger interface failed.");
		return false;
	}

	return true;
}

bool DirectConn::mt4Login(const int login, const char* passwd, CManagerInterface*& inter)
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
			SPDLOG(error, "DirectConn mt4 server login failed.---{}", cnt + 1);
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
		SPDLOG(error, "DirectConn 3 times of login to mt4 server failed.");
		return false;
	}
	return true;
}

int DirectConn::mt4Conn(const char* host, CManagerInterface*& inter)
{
	int cnt = 0;
	while (cnt < 3)
	{
		if (inter->Connect(host) != RET_OK)
		{
			SPDLOG(error, "DirectConn try to connect to mt4 server failed---{}", cnt + 1);
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
		SPDLOG(error, "DirectConn 3 times of connecting to mt4 server failed.");
		return RET_NO_CONNECT;
	}
	return RET_OK;
}

bool DirectConn::mt4IsConnected()
{
	if (m_managerInter->Ping() == RET_OK)
		return true;
	else
		return false;
}


bool DirectConn::createConnToMT4(CManagerInterface*& inter)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	if (inter != nullptr)
	{
		inter->Disconnect();
		inter->Release();
		inter = nullptr;
	}
	std::string login = Config::getInstance().getMT4ConnConf().find("login")->second;
	std::string pass = Config::getInstance().getMT4ConnConf().find("passwd")->second;

	if (mt4Login(std::stoi(login), pass.c_str(), inter))
	{
		storeConSymbol();
		storeConGroup();
	}
	else
	{
		return false;
	}
	return true;
}

bool DirectConn::createConnToMT4()
{
	std::string login = Config::getInstance().getMT4ConnConf().find("login")->second;
	std::string pass = Config::getInstance().getMT4ConnConf().find("passwd")->second;

	bool connectionStatus = true;

	if (mt4Login(std::stoi(login), pass.c_str(), m_managerInter) &&
		mt4Login(std::stoi(login), pass.c_str(), m_managerInterBak))
	{
		storeConSymbol();
		storeConGroup();
	}
	else
	{
		connectionStatus = connectionStatus && false;
	}
	monitorConn();
	return connectionStatus;
}

bool DirectConn::tradeTransaction(TradeTransInfo* info, int &code)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	int res = RET_OK;
	if (RET_OK != (res = m_managerInter->TradeTransaction(info)) && RET_OK_NONE != res)
	{
		if (RET_OK != (res = m_managerInterBak->TradeTransaction(info)) && RET_OK_NONE != res)
		{
			Logger::getInstance()->error("TradeTransaction failed, order #{}, error {}", info->order, m_managerInterBak->ErrorDescription(res));
			code = res;
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

bool DirectConn::storeConSymbol()
{
	int total = 0;
	ConSymbol* symbols = NULL;
	symbols = m_managerInter->CfgRequestSymbol(&total);
	std::lock_guard<std::mutex> lock(m_mtxSymbols);
	if (symbols == NULL)
	{
		if (NULL == (symbols = m_managerInterBak->CfgRequestSymbol(&total)))
		{
			Logger::getInstance()->error("get symbols config info failed by direct mode.");
			MailWarn::getInstance()->SendWarnMail("GetConSymbol", "get symbols config failed by direct mode");
			return false;
		}
		else
		{
			for (int i = 0; i < total; i++)
			{
				m_symbols[symbols[i].symbol] = symbols[i];
				m_managerInterBak->SymbolAdd(symbols[i].symbol);
			}

			m_managerInterBak->MemFree(symbols);
			return true;
		}
	}
	for (int i = 0; i < total; i++)
	{
		m_symbols[symbols[i].symbol] = symbols[i];
		m_managerInter->SymbolAdd(symbols[i].symbol);
	}

	m_managerInter->MemFree(symbols);
	return true;
}

bool DirectConn::storeConGroup()
{
	int total = 0;
	ConGroup* groups = NULL;
	groups = m_managerInter->CfgRequestGroup(&total);
	std::lock_guard<std::mutex> lock(m_mtxGroups);
	if (groups == NULL)
	{
		if (NULL == (groups = m_managerInterBak->CfgRequestGroup(&total)))
		{
			Logger::getInstance()->error("get groups config info failed by direct mode.");
			MailWarn::getInstance()->SendWarnMail("GetConGroup", "get group config failed by direct mode");
			return false;
		}
		else
		{
			for (int i = 0; i < total; i++)
			{
				m_groups[groups[i].group] = groups[i];
			}
			m_managerInterBak->MemFree(groups);
			return true;
        }
	}
	for (int i = 0; i < total; i++)
	{
		m_groups[groups[i].group] = groups[i];
	}
	m_managerInter->MemFree(groups);
	return true;
}

bool DirectConn::getConSymbol(std::string symbol, ConSymbol& cs)
{
	std::lock_guard<std::mutex> lock(m_mtxSymbols);
	if (m_symbols.find(symbol) != m_symbols.end())
	{
		cs = m_symbols[symbol];
		return true;
	}
	else
	{
		return false;
	}
}
bool DirectConn::getConGroup(std::string group, ConGroup& cg)
{
	std::lock_guard<std::mutex> lock(m_mtxGroups);
	if (m_groups.find(group) != m_groups.end())
	{
		cg = m_groups[group];
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectConn::marginLevelRequest(int login, MarginLevel& ml)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	int res = m_managerInter->MarginLevelRequest(login, &ml);
	if (RET_OK != res)
	{
		if (RET_OK != (res = m_managerInterBak->MarginLevelRequest(login, &ml)))
		{
			Logger::getInstance()->error("MarginLevelRequest failed, error {}", m_managerInterBak->ErrorDescription(res));
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
bool DirectConn::getUserRecord(int login, UserRecord& ur)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	const int logins[1] = { login };
	int total = sizeof(logins);
	UserRecord* tmpUr = m_managerInter->UserRecordsRequest(logins, &total);
	if (NULL == tmpUr)
	{
		if (NULL == (tmpUr = m_managerInterBak->UserRecordsRequest(logins, &total)))
		{
			return false;
		}
		else
		{
			ur = tmpUr[0];
			return true;
		}
	}
	else
	{
		ur = tmpUr[0];
		return true;
	}
}

bool DirectConn::setConSymbolTradeMode(std::string& symbol, int mode)
{
	ConSymbol cs = {};
	if (mode < 0 || mode > 2)
	{
		Logger::getInstance()->error("param invalid, symbol :{}, mode: {}",symbol, mode);
		return false;
	}
		
	if (!getConSymbol(symbol, cs))
		return false;
	cs.trade = mode;
	int ret = m_managerInter->CfgUpdateSymbol(&cs);
	if (ret != RET_OK)
	{
		Logger::getInstance()->error("update trade mode failed. symbol: {}, mode: {}", symbol, mode);
		return false;
	}
	else
		return true;
}

TradeRecord* DirectConn::TradesRequest(std::string loginList, int open_only, int& total)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	TradeRecord* tr = m_managerInter->AdmTradesRequest(loginList.c_str(), open_only, &total);
	return tr;
}
void DirectConn::tradesRelease(TradeRecord* tr)
{
	std::lock_guard<std::mutex> lock(m_mtxDirect);
	m_managerInter->MemFree(tr);
}

void DirectConn::monitorConn()
{
	m_thdMonitor = std::thread([this]()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));

			if ((nullptr == m_managerInter) ||
				(nullptr != m_managerInter && RET_OK != m_managerInter->Ping()))
			{
				Logger::getInstance()->warn("DirectConn master disconnected to mt4. will try to connect to mt4.");

				if (createConnToMT4(m_managerInter))
				{
					Logger::getInstance()->info("DirectConn master connect to mt4 success.");
					MailWarn::getInstance()->SendWarnMail("DirectConn", "DirectConn master connect to mt4 success.");
				}
				else
				{
					Logger::getInstance()->info("DirectConn master connect to mt4 failed.");
					MailWarn::getInstance()->SendWarnMail("DirectConn", "DirectConn master connect to mt4 failed.");
				}
			}

			if ((nullptr == m_managerInterBak) ||
				(nullptr != m_managerInterBak && RET_OK != m_managerInterBak->Ping()))
			{
				Logger::getInstance()->warn("DirectConn slave disconnected to mt4. will try to connect to mt4.");

				if (createConnToMT4(m_managerInterBak))
				{
					Logger::getInstance()->info("DirectConn slave connect to mt4 success.");
					MailWarn::getInstance()->SendWarnMail("DirectConn", "DirectConn slave connect to mt4 succss.");
				}
				else
				{
					Logger::getInstance()->info("DirectConn slave connect to mt4 failed.");
					MailWarn::getInstance()->SendWarnMail("DirectConn", "DirectConn slave connect to mt4 failed.");
				}
			}
			if (m_stop)
				break;
		}
	});
	m_thdMonitor.detach();
}