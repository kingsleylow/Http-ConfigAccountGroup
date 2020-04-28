#include "MT4Module.h"
#include "Logger.h"
#include <chrono>
#include <thread>
#include <regex>
#include <iostream>

MT4Conn::MT4Conn()
{
}


MT4Conn::~MT4Conn()
{
	m_stop = true;
	m_memCache.stopEventLoop();
	m_tdExpire.join();
}

bool MT4Conn::mt4Init()
{
	std::string path = Utils::getInstance().getProjectPath();
	path += Config::getInstance().getMT4ConnConf().find("lib")->second;
	m_factoryInter.Init(path.c_str());
	if (m_factoryInter.IsValid() == FALSE)
	{
		SPDLOG(error, "mt4 lib init failed");
		return false;
	}
	if (m_factoryInter.WinsockStartup() != RET_OK)
	{
		SPDLOG(error, "mt4 lib init winsock lib failed.");
		return false;
	}
	if (NULL == (m_directInter = m_factoryInter.Create(ManAPIVersion)) ||
		NULL == (m_pumpInter = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "mt4 factory create mananger interface failed.");
		return false;
	}
	watchDirectConntoMT4();
	watchPumpConnToMT4();

	std::string channel = Config::getInstance().getRedisConf().find("cache-channel")->second;
	std::string host = Config::getInstance().getRedisConf().find("host")->second;
	std::string port = Config::getInstance().getRedisConf().find("port")->second;
	std::string pass = Config::getInstance().getRedisConf().find("pass")->second;

	m_redis.init(channel);
	m_redis.connectToServer(host, std::stoi(port), pass);
	m_memCache.init(std::bind(&MT4Conn::pumpEvent, this, std::placeholders::_1, std::placeholders::_2), this);
	m_memCache.startEventLoop();

	m_tdExpire = std::thread(&MT4Conn::expireCache, this);
	return true;
}

int  MT4Conn::pumpEvent(std::string& data, void* param)
{
#ifdef  MY_DEBUG
	std::cout << data << std::endl;
#endif //  MY_DEBUG

	m_redis.enqueue(data);
	return 0;
}

void MT4Conn::forwardEvent(std::vector<std::string> data, int msgtype, int mt4type)
{
	std::string out;
	switch (msgtype)
	{
	case 0://symbols
		out = Utils::getInstance().serializePumpSymbols(data);
		break;
	case 1://users
		out = Utils::getInstance().serializePumpUsers(data);
		break;
	case 2://groups
		out = Utils::getInstance().serializePumpGroups(data);
		break;
	}

	if (TRANS_ADD == mt4type)
	{
		std::string md5 = Utils::getInstance().Md5(out);
		if (m_pumpCache.find(md5) == m_pumpCache.end())
		{
			std::lock_guard<std::mutex> lock(m_pumpCacheMtx);
			time_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			m_pumpCache[md5] = now;
			m_memCache.add(out);
		}
	}
	else
	{
		m_memCache.add(out);
	}
	
}

void MT4Conn::expireCache()
{
	while (true)
	{
		if (m_stop)
			break;
		if (!m_pumpCache.empty())
		{
			time_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			std::vector<std::string> expireKey;
			for (auto& c : m_pumpCache)
			{
				if (c.second - now >= 5)
				{
					expireKey.push_back(c.first);
				}
			}

			for (auto& e : expireKey)
			{
				std::lock_guard<std::mutex> lock(m_pumpCacheMtx);
				m_pumpCache.erase(e);
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
}

bool MT4Conn::mt4Login(const int login, const char* passwd, CManagerInterface*& manager)
{
	if (RET_OK != mt4Conn(Config::getInstance().getMT4ConnConf().find(std::move("host"))->second.c_str(), manager))
		return false;
	int cnt = 0;
	while (cnt < 3)
	{
		if (RET_OK != manager->Login(login, passwd))
		{
			SPDLOG(error, "mt4 server login failed.---{}", cnt+1);
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
		SPDLOG(error, "3 times of login to mt4 server failed.");
		return false;
	}
	std::cout << "login " << login << "success." << std::endl;
	Logger::getInstance()->info("{} login success.", login);
	return true;
}

int MT4Conn::mt4Conn(const char* host, CManagerInterface*& manager)
{
	int cnt = 0;
	while (cnt < 3)
	{
		if (manager->Connect(host) != RET_OK)
		{
			SPDLOG(error, "try to connect to mt4 server failed---{}", cnt + 1);
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
		SPDLOG(error, "3 times of connecting to mt4 server failed.");
		return RET_NO_CONNECT;
	}
	return RET_OK;
}

void MT4Conn::onPumpingFunc(int code, int type, void *data, void *param)
{
	MT4Conn* pThis = (MT4Conn*)param;
	switch (code)
	{
	case PUMP_START_PUMPING:
		Logger::getInstance()->info("PUMP_START_PUMPING...");
		break;
	case PUMP_STOP_PUMPING:
		Logger::getInstance()->info("PUMP_STOP_PUMPING...");
		break;
	case PUMP_UPDATE_BIDASK:
		pThis->storePrices();
		break;
	case PUMP_UPDATE_SYMBOLS:
		pThis->storeSymbolsInfo(data, type);
		break;
	case PUMP_UPDATE_GROUPS:
		pThis->storeGroupsInfo(data, type);
		break;
	case PUMP_UPDATE_USERS:
		pThis->storeUserInfo(data, type);
		break;
	case PUMP_UPDATE_ONLINE:
		break;
	case PUMP_UPDATE_TRADES:
		break;
	case PUMP_UPDATE_ACTIVATION:
		break;
	case PUMP_UPDATE_MARGINCALL:
		break;
	case PUMP_UPDATE_REQUESTS:
		break;
	case PUMP_UPDATE_PLUGINS:
		break;
	case PUMP_UPDATE_NEWS:
		break;
	case PUMP_UPDATE_MAIL:
		break;
	default: break;
	}
}

int MT4Conn::updateOrderOpenPrice(int orderNo, double profit, std::string& err)
{
	TradeRecord tr = {0};
	int code = m_pumpInter->TradeRecordGet(orderNo, &tr);
	if (code == RET_OK)
	{
		double bid, ask, conv, currencyProfit;
		if (!getSymbolPrice(tr.symbol, bid, ask))
			return -1;
		if (!getProfitConverRate(tr.symbol, conv))
			return -1;
		else
		{
			currencyProfit = profit / conv;
		}

		ConSymbol cs = {0};
		std::string err;
		if (0 != getGlobalSymbols(std::string(tr.symbol), cs, err))
			return -1;

		double modifyPrice = 0;
		if (tr.cmd == 0)
		{
			modifyPrice = bid - currencyProfit/cs.contract_size/tr.volume*100;
		}
		else
		{
			modifyPrice = ask + currencyProfit / cs.contract_size /tr.volume*100;
		}
		char tmp[10] = {0};
		sprintf_s(tmp, "%.*f", cs.digits, modifyPrice);
		tr.open_price = std::stod(tmp);

		code = m_directInter->AdmTradeRecordModify(&tr);
		if(RET_OK == code)
			return code;
		else
		{
			err = m_directInter->ErrorDescription(code);
			Logger::getInstance()->error("update order {} open_price failed, error:{}", orderNo, err);
			return code;
		}
	}
	else
	{
		err = m_pumpInter->ErrorDescription(code);
		return code;
	}
}

bool MT4Conn::getProfitConverRate(std::string symbol, double& conv)
{
	std::string currency = symbol.substr(3, 3);
	if (currency.compare("USD") == 0)
	{
		conv = 1;
		return true;
	}
		
	if (!std::regex_match(currency, std::regex("[A-Z]{3}")))
		return false;
	else
	{
		std::string cA = currency + "USD";
		std::string cB = "USD" + currency;
		double bid, ask;
		if (getSymbolPrice(cA, bid, ask))
		{
			conv = (bid + ask) / 2;
			return true;
		}
		else
		{
			conv = (bid + ask) / 2;
			conv = 1 / conv;
			return true;
		}
	}
}

bool MT4Conn::storePrices()
{
	SymbolInfo si[32] = {0};
	int total = 0;
	while (total = m_pumpInter->SymbolInfoUpdated(si, 32) > 0) {
		for (int i = 0; i < total; i++)
		{
			std::array<double, 2> quote;
			quote[0] = si[i].bid;
			quote[1] = si[i].ask;
			m_Quotes[si[i].symbol] = quote;
		}
	}
	return true;
}

bool MT4Conn::getSymbolPrice(std::string symbol, double& bid, double& ask)
{
	if (m_Quotes.find(symbol) != m_Quotes.end())
	{
		bid = m_Quotes.at(symbol).at(0);
		ask = m_Quotes.at(symbol).at(1);
		return true;
	}
	else
	{
		int total = 0;
		TickInfo* ti = m_pumpInter->TickInfoLast(symbol.c_str(), &total);
		if (ti == nullptr)
			return false;
		else
		{
			int tm = 0;
			for (int i = 0; i < total; i++)
			{
				if (tm < ti[i].ctm)
				{
					bid = ti[i].bid;
					ask = ti[i].ask;
				}
			}
			m_pumpInter->MemFree(ti);
			return true;
		}
	}
}

bool MT4Conn::storeGroupsInfo(void* data, int type)
{
	bool res = true;
	if (data == nullptr)
	{
		std::vector<std::string> groupsV;
		int total = 0;
		ConGroup* groupInfo = m_pumpInter->GroupsGet(&total);
		if (total)
		{
			for (int i = 0; i < total; i++)
			{
				m_GroupsInfo[groupInfo[i].group] = groupInfo[i];
				groupsV.push_back(groupInfo[i].group);
				std::string md5 = Utils::getInstance().Md5(Utils::getInstance().serializeConGroup(groupInfo[i]));
				m_groupCache[groupInfo[i].group] = md5;
			}
		}
		else
		{
			res = false;
		}
		m_pumpInter->MemFree(groupInfo);

		if(type == TRANS_ADD || type == TRANS_UPDATE)
			forwardEvent(groupsV, 2, type);
	}
	else
	{
		ConGroup* cg = (ConGroup*)data;
		std::string md5 = Utils::getInstance().Md5(Utils::getInstance().serializeConGroup(*cg));
		if (m_GroupsInfo.find(cg->group) != m_GroupsInfo.end() &&
			m_groupCache.find(cg->group) != m_groupCache.end() &&
			m_groupCache.at(cg->group) == md5)
			return res;

		m_GroupsInfo[cg->group] = *cg;
		m_groupCache[cg->group] = md5;
		if (type == TRANS_ADD || type == TRANS_UPDATE)
			forwardEvent(std::vector<std::string>{cg->group}, 2, type);
	}
	return res;
}

bool MT4Conn::storeSymbolsInfo(void* data, int type)
{
	if (data == nullptr)
	{
		std::vector<std::string> symbolsV;
		int total = 0;
		ConSymbol* symbols = m_pumpInter->SymbolsGetAll(&total);
		if (NULL == symbols)
			return false;
		for (int i = 0; i < total; i++)
		{
			m_SymbolsInfo[symbols[i].symbol] = symbols[i];
			symbolsV.push_back(symbols[i].symbol);
		}
		m_pumpInter->MemFree(symbols);

		if (type == TRANS_ADD || type == TRANS_UPDATE)
			forwardEvent(symbolsV, 0, type);
	}
	else
	{
		ConSymbol* cs = (ConSymbol*)data;
		m_SymbolsInfo[cs->symbol] = *cs;
		
		if (type == TRANS_ADD || type == TRANS_UPDATE)
			forwardEvent(std::vector<std::string>{cs->symbol}, 0, type);
	}
	return true;
}

bool MT4Conn::storeUserInfo(void* data, int type)
{
	if (data == nullptr)
		return false;
	else
	{
		UserRecord* ur = (UserRecord*)data;
		std::string out;

		if (type == TRANS_ADD || type == TRANS_UPDATE)
			forwardEvent(std::vector<std::string>{std::to_string(ur->login)}, 1, type);
	}
	return true;
}

bool MT4Conn::switchToPumpMode(CManagerInterface*& managerInter)
{
	if (RET_OK != managerInter->PumpingSwitchEx(&MT4Conn::onPumpingFunc, 0, this))
	{
		SPDLOG(error, "pumpswitch failed.");
		return false;
	}
	Logger::getInstance()->info("pumpswitch success.");
	return true;
}

bool MT4Conn::mt4DirtIsConnected()
{
	try {
		if (nullptr != m_directInter && m_directInter->Ping() == RET_OK)
			return true;
		else
			return false;
	}
	catch (...)
	{
		Logger::getInstance()->error("IsConnected exception...");
	}
	return true;
}

bool MT4Conn::createDirectConnToMT4(CManagerInterface*& manager)
{
	if (nullptr != manager && manager->Ping() == RET_OK)
	{
		manager->Disconnect();
	}
	manager->Release();
	manager = nullptr;
	if (nullptr == (manager = m_factoryInter.Create(ManAPIVersion)))
		return false;
	if (mt4Login(std::stoi(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), manager))
		return true;
	else
		return false;
}
bool MT4Conn::createPumpConnToMT4(CManagerInterface*& manager)
{
	if (nullptr != manager && manager->IsConnected())
	{
		manager->Disconnect();
	}
	manager->Release();
	manager = nullptr;
	if (nullptr == (manager = m_factoryInter.Create(ManAPIVersion)))
		return false;


	if (mt4Login(std::stoi(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), manager) &&
		switchToPumpMode(manager))
		return true;
	else
		return false;
}

bool MT4Conn::createConnToMT4()
{
	if (mt4Login(std::stoi(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), m_directInter) &&
		mt4Login(std::stoi(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
			Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), m_pumpInter) &&
		switchToPumpMode(m_pumpInter))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool MT4Conn::heartBeat()
{
	return m_directInter->Ping();
}

void MT4Conn::watchDirectConntoMT4()
{
	m_monitorConn.push_back(std::thread([&]()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));

			if (!mt4DirtIsConnected())
			{
				Logger::getInstance()->warn("disconnected to mt4. will try to connect to mt4.");
				if (!createDirectConnToMT4(m_directInter))
				{
					Logger::getInstance()->info("connect to mt4 failed.");
					std::cout << "direct connect to mt4 failed" << std::endl;
				}
			}
		}
	}));
}
void MT4Conn::watchPumpConnToMT4()
{
	m_monitorConn.push_back(std::thread([&]()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));

			if (nullptr == m_pumpInter || 0 == m_pumpInter->IsConnected())
			{
				if (!createPumpConnToMT4(m_pumpInter))
				{
					Logger::getInstance()->info("connect to mt4 failed.");
					std::cout << "pump connect to mt4 failed" << std::endl;
				}
			}
		}
	}));
}

ConGroup MT4Conn::getGroupCfg(const std::string& group)
{
	ConGroup ret = { 0 };
	ret = m_GroupsInfo[group];

	return ret;
}

bool MT4Conn::getGroupNames(std::vector<std::string>& groups)
{
	int total = 0;
	ConGroup* cfgGroup = nullptr;

	int tryTimes = 0;
	do
	{
		if (mt4DirtIsConnected())
		{
			cfgGroup = m_directInter->CfgRequestGroup(&total);
			if (total)
			{
				for (int i = 0; i < total; i++)
				{
					groups.push_back(cfgGroup[i].group);
				}
			}
		}
		else
		{
			if (tryTimes > 5)
				break;
			createConnToMT4();
			tryTimes++;
			continue;
		}
	} while (0);

	m_directInter->MemFree(cfgGroup);
	return true;
}

int MT4Conn::getSecuritiesNames(ConSymbolGroup securities[], std::string& err)
{
	int res = 0;
	int tryTimes = 0;
	do
	{
		if (mt4DirtIsConnected())
		{
			res = m_directInter->CfgRequestSymbolGroup(securities);
			if (RET_OK != res)
				err = m_directInter->ErrorDescription(res);
		}
		else
		{
			if (tryTimes > 3)
				break;
			createConnToMT4();
			tryTimes++;
			continue;
		}
	} while (0);

	return res;
}


int MT4Conn::updateGroupSec(const std::string& group,const std::map<int, ConGroupSec>& cfgGroupSec, std::set<int> index, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	int size = sizeof(cfgGroup.secgroups) / sizeof(ConGroupSec);

	int tmpIndex = 0;
	if (group.compare(cfgGroup.group) == 0)
	{
		for (int i = 0; i < size; i++)
		{
			if (index.find(i) != index.end())
			{
				cfgGroup.secgroups[i] = cfgGroupSec.at(i);
				tmpIndex = i;//log use
			}
		}
	}
	else
	{
		return -1;
	}	

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group securities failed.{}, group:{},index:{}", err, group, tmpIndex);
	}
	return res;
}


int MT4Conn::updateGroupSymbol(const std::string& group, const std::map<std::string, ConGroupMargin>& cfgGroupMargin, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	int oldSize = cfgGroup.secmargins_total;

	std::set<std::string> exclusiveSymbol;
	if (group.compare(cfgGroup.group) == 0)
	{
		for (int i = 0; i < cfgGroup.secmargins_total; i++)
		{
			if (cfgGroupMargin.find(cfgGroup.secmargins[i].symbol) != cfgGroupMargin.end())
			{
				cfgGroup.secmargins[i] = cfgGroupMargin.at(cfgGroup.secmargins[i].symbol);
				exclusiveSymbol.insert(cfgGroup.secmargins[i].symbol);
			}
		}
		for (auto& m : cfgGroupMargin)
		{
			if (exclusiveSymbol.find(m.first) != exclusiveSymbol.end())
				continue;
			cfgGroup.secmargins[oldSize++] = m.second;
		}
	}
	else
	{
		return -1;
	}

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group margins failed.{}", err);
	}
	return res;
}


GroupCommon MT4Conn::getGroupCommon(const std::string& group)
{
	//ConGroup cfgGroup(std::move(getGroupCfg(group)));
	ConGroup cfgGroup(m_GroupsInfo[group]);
	GroupCommon common;
	common.company = cfgGroup.company;
	common.currency = cfgGroup.currency;
	common.default_deposit = cfgGroup.default_deposit;
	common.default_leverage = cfgGroup.default_leverage;
	common.enable = cfgGroup.enable;
	common.group = cfgGroup.group;
	common.interestrate = cfgGroup.interestrate;
	common.otp_mode = cfgGroup.otp_mode;
	common.support_page = cfgGroup.support_page;

	return common;
}

int MT4Conn::updateGroupCommon(const std::string& group, const GroupCommon& common, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	if (group.compare(cfgGroup.group) == 0)
	{
		memcpy(cfgGroup.group, common.group.c_str(), common.group.size());
		cfgGroup.enable = common.enable;
		cfgGroup.otp_mode = common.otp_mode;
		memcpy(cfgGroup.company, common.company.c_str(), common.company.size());
		memcpy(cfgGroup.support_page, common.support_page.c_str(), common.support_page.size());
		cfgGroup.default_deposit = common.default_deposit;
		cfgGroup.default_leverage = common.default_leverage;
		cfgGroup.interestrate = common.interestrate;
		memcpy(cfgGroup.currency, common.currency.c_str(), common.currency.size());
	}
	else
	{
		return -1;
	}
	

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group common failed.{}",err);
	}
	return res;
}

GroupMargin MT4Conn::getGroupMargin(const std::string& group)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	GroupMargin margin;
	margin.margin_call = cfgGroup.margin_call;
	margin.margin_stopout = cfgGroup.margin_stopout;
	margin.margin_type = cfgGroup.margin_type;
	margin.margin_mode = cfgGroup.margin_mode;
	margin.credit = cfgGroup.credit;
	margin.stopout_skip_hedged = cfgGroup.stopout_skip_hedged;
	margin.hedge_large_leg = cfgGroup.hedge_largeleg;

	return margin;
}

int MT4Conn::updateGroupMargin(const std::string group, const GroupMargin& margin, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	if (group.compare(cfgGroup.group) == 0)
	{
		cfgGroup.margin_call = margin.margin_call;
		cfgGroup.margin_stopout = margin.margin_stopout;
		cfgGroup.margin_type = margin.margin_type;
		cfgGroup.margin_mode = margin.margin_mode;
		cfgGroup.credit = margin.credit;
		cfgGroup.stopout_skip_hedged = margin.stopout_skip_hedged;
		cfgGroup.hedge_largeleg = margin.hedge_large_leg;
	}
	else
	{
		return -1;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group margin failed.{}", err);
	}
	return res;
}

GroupArchive MT4Conn::getGroupArchive(const std::string& group)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	GroupArchive archive;
	archive.archive_max_balance = cfgGroup.archive_max_balance;
	archive.archive_pending_period = cfgGroup.archive_pending_period;
	archive.archive_period = cfgGroup.archive_period;

	return archive;
}
int MT4Conn::updateGroupArchive(const std::string group, const GroupArchive& archive, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	if (group.compare(cfgGroup.group) == 0)
	{
		cfgGroup.archive_max_balance = archive.archive_max_balance;
		cfgGroup.archive_pending_period = archive.archive_pending_period;
		cfgGroup.archive_period = archive.archive_period;
	}
	else
	{
		return -1;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group margin failed.{}", err);
	}
	return res;
}

GroupReport MT4Conn::getGroupReport(const std::string& group)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	GroupReport report;
	report.copies = cfgGroup.copies;
	report.reports = cfgGroup.reports;
	report.signature = cfgGroup.signature;
	report.smtp_login = cfgGroup.smtp_login;
	report.smtp_passwd = cfgGroup.smtp_password;
	report.smtp_server = cfgGroup.smtp_server;
	report.support_email = cfgGroup.support_email;
	report.template_path = cfgGroup.templates;
	
	return report;
}

int MT4Conn::upateGroupReport(const std::string group, const GroupReport& report, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	if (group.compare(cfgGroup.group) == 0)
	{
		cfgGroup.copies = report.copies;
		cfgGroup.reports = report.reports;
		memcpy(cfgGroup.signature, report.signature.c_str(),report.signature.size());
		memcpy(cfgGroup.smtp_login, report.smtp_login.c_str(), report.smtp_login.size());
		memcpy(cfgGroup.smtp_password, report.smtp_passwd.c_str(), report.smtp_passwd.size());
		memcpy(cfgGroup.smtp_server, report.smtp_server.c_str(), report.smtp_server.size());
		memcpy(cfgGroup.support_email, report.support_email.c_str(), report.support_email.size());
		memcpy(cfgGroup.templates, report.template_path.c_str(), report.template_path.size());
	}
	else
	{
		return -1;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group margin failed.{}", err);
	}
	return res;
}

GroupPermission MT4Conn::getGroupPermission(const std::string& group)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	GroupPermission permission;
	permission.check_ie_prices = cfgGroup.check_ie_prices;
	permission.close_fifo = cfgGroup.close_fifo;
	permission.close_reopen = cfgGroup.close_reopen;
	permission.hedge_prohibited = cfgGroup.hedge_prohibited;
	permission.maxpositions = cfgGroup.maxpositions;
	permission.maxsecurities = cfgGroup.maxsecurities;
	permission.news = cfgGroup.news;
	memcpy(permission.news_language ,cfgGroup.news_languages, 8);
	permission.news_language_total = cfgGroup.news_languages_total;
	permission.rights = cfgGroup.rights;
	permission.securities_hash = cfgGroup.securities_hash;
	permission.timeout = cfgGroup.timeout;
	memcpy(permission.unused_rights , cfgGroup.unused_rights, 2);
	permission.use_swap = cfgGroup.use_swap;

	return permission;
}

int MT4Conn::updateGroupPerssion(const std::string group, const GroupPermission& permission, std::string& err)
{
	ConGroup cfgGroup(std::move(getGroupCfg(group)));
	if (group.compare(cfgGroup.group) == 0)
	{
		cfgGroup.check_ie_prices = permission.check_ie_prices;
		cfgGroup.close_fifo = permission.close_fifo;
		cfgGroup.close_reopen = permission.close_reopen;
		cfgGroup.hedge_prohibited = permission.hedge_prohibited;
		cfgGroup.maxpositions = permission.maxpositions;
		cfgGroup.maxsecurities = permission.maxsecurities;
		cfgGroup.news = permission.news;
		memcpy(cfgGroup.news_languages, permission.news_language, 8);
		cfgGroup.news_languages_total = permission.news_language_total;
		cfgGroup.rights = permission.rights;
		//memcpy(cfgGroup.securities_hash, permission.securities_hash.c_str(), permission.securities_hash.size());
		cfgGroup.timeout = permission.timeout;
		memcpy(cfgGroup.unused_rights, permission.unused_rights, 2);
		cfgGroup.use_swap = permission.use_swap;
	}
	else
	{
		return -1;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		err = m_directInter->ErrorDescription(res);
		Logger::getInstance()->error("update group margin failed.{}", err);
	}
	return res;
}

int MT4Conn::updateAccounts(const std::string login, const AccountConfiguration& configuration, std::string& err)
{
	if (login.empty())
		return false;
	int _login = std::stoi(login);
	UserRecord ur = {0};
	int ret = m_pumpInter->UserRecordGet(_login, &ur);
	if (ret == RET_OK)
	{
		strncpy_s(ur.password, sizeof(ur.password), configuration.password.c_str(), configuration.password.length()+1);
		ur.enable_change_password = configuration.enable_change_password;
		if (RET_OK != (ret = m_directInter->UserRecordUpdate(&ur)))
		{
			err = m_directInter->ErrorDescription(ret);
			Logger::getInstance()->info("error :{}", err);
			return ret;
		}
		if (RET_OK != (ret = m_directInter->UserPasswordSet(_login, configuration.password.c_str(), 0, 0)))
		{
			err = m_directInter->ErrorDescription(ret);
			Logger::getInstance()->info("error :{}", err);
			return ret;
		}
	}
	else
	{
		err = m_pumpInter->ErrorDescription(ret);
		Logger::getInstance()->info("error :{}", err);
		return ret;
	}
	return ret;
}

ConFeeder* MT4Conn::getGlobalDatafeed(int& total)
{
	ConFeeder* feeder = nullptr;
	if (mt4DirtIsConnected())
	{
		feeder = m_directInter->CfgRequestFeeder(&total);
	}
	return feeder;
}

void MT4Conn::releaseGlobalDatafeed(ConFeeder* feeder)
{
	if (mt4DirtIsConnected())
	{
		m_directInter->MemFree(feeder);
	}
}

ConCommon MT4Conn::getGlobalCommon()
{
	ConCommon common = {0};
	if (mt4DirtIsConnected())
	{
		m_directInter->CfgRequestCommon(&common);
	}
	return common;
}

ConAccess* MT4Conn::getGlobalIPList(int& total)
{
	ConAccess* ip = nullptr;
	if (mt4DirtIsConnected())
	{
		ip = m_directInter->CfgRequestAccess(&total);
	}
	return ip;
}

int MT4Conn::getGlobalSymbols(std::vector<ConSymbol>& symbols, std::string& err)
{
	if (m_SymbolsInfo.empty())
		return -1;
	for (auto& s : m_SymbolsInfo)
	{
		symbols.push_back(s.second);
	}	
	return 0;
}

int  MT4Conn::getGlobalSymbols(std::string& symbol, ConSymbol& con, std::string& err)
{
	if (m_SymbolsInfo.empty() || m_SymbolsInfo.find(symbol) == m_SymbolsInfo.end())
		return -1;
	else
	{
		con = m_SymbolsInfo[symbol];
		return 0;
	}
}

ConDataServer* MT4Conn::getGlobalDCList(int& total)
{
	ConDataServer* dc = nullptr;
	if (mt4DirtIsConnected())
	{
		dc = m_directInter->CfgRequestDataServer(&total);
	}
	return dc;
}
void MT4Conn::releaseGlobalDCList(ConDataServer* dc)
{
	if (mt4DirtIsConnected())
	{
		m_directInter->MemFree(dc);
	}
}

ConPluginParam* MT4Conn::getGlobalPluginList(int& total)
{
	ConPluginParam* cp = nullptr;
	if (mt4DirtIsConnected())
	{
		cp = m_directInter->CfgRequestPlugin(&total);
	}
	return cp;
}
void MT4Conn::releaseGlobalPluginList(ConPluginParam* cp)
{
	if (mt4DirtIsConnected())
	{
		m_directInter->MemFree(cp);
	}
}

PerformanceInfo* MT4Conn::getGlobalPerformance(const time_t from, int& total)
{
	PerformanceInfo* pi = nullptr;
	if (mt4DirtIsConnected())
	{
		pi = m_directInter->PerformanceRequest(from, &total);
	}
	return pi;
}
void MT4Conn::releaseGlobalPerformance(PerformanceInfo* pi)
{
	if (mt4DirtIsConnected())
	{
		m_directInter->MemFree(pi);
	}
}

ConHoliday* MT4Conn::getHoliday(int& total)
{
	ConHoliday *ch = nullptr;
	if (mt4DirtIsConnected())
	{
		ch = m_directInter->CfgRequestHoliday(&total);
	}
	return ch;
}

void MT4Conn::releaseHoliday(ConHoliday* ch)
{
	if (mt4DirtIsConnected())
	{
		m_directInter->MemFree(ch);
	}
}

int MT4Conn::updateSymbolsSessions(const std::string& symbol, const ConSessions cs[7], std::string& err)
{
	ConSymbol csConf = {};
	if (mt4DirtIsConnected())
	{
		std::string err;
		if (0 != getGlobalSymbols(const_cast<std::string&>(symbol), csConf, err))
			return -1;

		memcpy(csConf.sessions, cs, sizeof(ConSessions) * 7);

		int res = RET_OK;
		if (RET_OK != (res = m_directInter->CfgUpdateSymbol(&csConf)))
		{
			err = m_directInter->ErrorDescription(res);
			return res;
		}
		else
		{
			return res;
		}
	}
	return -1;
}

int MT4Conn::setSymbolSwap(std::string& err, std::string symbol, int swap_long, int swap_short, int swap_enable, int swap_rollover3days)
{
	ConSymbol cs = {0};
	bool found = false;
	for (auto& c : m_SymbolsInfo)
	{
		if (std::string(c.second.symbol).compare(symbol) == 0)
		{
			cs = c.second;
			found = true;
			break;
		}
	}
	if (!found)
	{
		return -1;
	}
	else
	{
		cs.swap_long = swap_long;
		cs.swap_short = swap_short;
		if (swap_enable != -1)
			cs.swap_enable = swap_enable;
		if (swap_rollover3days != -1)
			cs.swap_rollover3days = swap_rollover3days;

		int code = m_directInter->CfgUpdateSymbol(&cs);
		if (code != RET_OK)
		{
			err = m_directInter->ErrorDescription(code);
			Logger::getInstance()->error("update swap failed. error {}", err);
			return code;
		}
		else
		{
			return code;
		}
	}
}

int MT4Conn::setSymbolTradeMode(std::string symbol, int mode, std::string& err)
{
	ConSymbol cs = {};
	if (mode < 0 || mode > 2)
	{
		Logger::getInstance()->error("param invalid, symbol :{}, mode: {}", symbol, mode);
		return -1;
	}

	if (!getConSymbol(symbol, cs, err))
		return -1;
	cs.trade = mode;
	int ret = m_directInter->CfgUpdateSymbol(&cs);
	if (ret != RET_OK)
	{
		err = m_directInter->ErrorDescription(ret);
		Logger::getInstance()->error("update trade mode failed. symbol: {}, mode: {}", symbol, mode);
		return ret;
	}
	else
		return ret;
}

int MT4Conn::getConSymbol(std::string symbol, ConSymbol& cs, std::string& err)
{
	bool found = false;
	for (auto& c : m_SymbolsInfo)
	{
		if (std::string(c.second.symbol).compare(symbol) == 0)
		{
			cs = c.second;
			found = true;
			break;
		}
	}
	if (!found)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int MT4Conn::updateConSymbol(const ConSymbol cs)
{
	return m_directInter->CfgUpdateSymbol(&cs);
}

int MT4Conn::tradeTransaction(TradeTransInfo* info, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	int res = RET_OK;
	if (RET_OK != (res = m_directInter->TradeTransaction(info)) && RET_OK_NONE != res)
	{
		if (RET_OK != (res = m_directInter->TradeTransaction(info)) && RET_OK_NONE != res)
		{
			err = m_directInter->ErrorDescription(res);
			Logger::getInstance()->error("TradeTransaction failed, order #{}, error {}", info->order, err);
			return res;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

int MT4Conn::updateTradeRecord(TradeRecord* info, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	int res = RET_OK;
	if (RET_OK != (res = m_directInter->AdmTradeRecordModify(info)) && RET_OK_NONE != res)
	{
		if (RET_OK != (res = m_directInter->AdmTradeRecordModify(info)) && RET_OK_NONE != res)
		{
			err = m_directInter->ErrorDescription(res);
			Logger::getInstance()->error("admTradeRecordModify failed, order #{}, error {}", info->order, err);
		}
	}
	return res;
}

int MT4Conn::tradesGetBySymbol(const std::string& symbol, std::vector<TradeRecord>& recordV, std::string& err)
{
	std::vector<std::string> groups;
	if (!getGroupNames(groups))
		return -1;
	std::lock_guard<std::mutex> lock(m_directMtx);
	int res = RET_OK;
	int size = 0;
	for (auto& g : groups)
	{
		TradeRecord* record = m_directInter->AdmTradesRequest(g.c_str(), 0, &size);
		if (record == nullptr)
			continue;
		else
		{
			for (int i = 0; i < size; i++)
			{
				recordV.push_back(record[i]);
			}
			m_directInter->MemFree(record);
		}

	}
	return 0;
}

int MT4Conn::tradesGetByTicket(const std::string tick, std::vector<TradeRecord>& recordV, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	int res = RET_OK;
	int size = 0;
	std::string ticket = "#" + tick;
	TradeRecord* record = m_directInter->AdmTradesRequest(ticket.c_str(), 0, &size);
	if (record == nullptr)
		return -1;
	else
	{
		for (int i = 0; i < size; i++)
		{
			recordV.push_back(record[i]);
		}
		m_directInter->MemFree(record);
	}

	return 0;
}

void MT4Conn::tradesRelease(TradeRecord*& record)
{
	m_directInter->MemFree(record);
}

int MT4Conn::chartInfoReq(const ChartInfo* chart, RateInfo*& ri, int& size, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	__time32_t timesign = 0;
	ri = m_directInter->ChartRequest(chart, &timesign, &size);
	if (ri != nullptr)
		return 0;
	else
		return -1;
}

void MT4Conn::releaseChartInfo(RateInfo*& chart)
{
	m_directInter->MemFree(chart);
}

int MT4Conn::chartInfoUpdate(const std::string symbol, const int period, int size, const RateInfo* rates, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	int res = m_directInter->ChartUpdate(symbol.c_str(), period, rates, &size);
	if (res != RET_OK)
		err = m_directInter->ErrorDescription(res);
	return res;
}

int MT4Conn::getUserRecord(int login, UserRecord& ur, std::string& err)
{
	std::lock_guard<std::mutex> lock(m_directMtx);
	const int logins[1] = { login };
	int total = sizeof(logins);
	UserRecord* tmpUr = m_directInter->UserRecordsRequest(logins, &total);
	if (NULL == tmpUr)
	{
		if (NULL == (tmpUr = m_directInter->UserRecordsRequest(logins, &total)))
		{
			return -1;
		}
		else
		{
			ur = tmpUr[0];
			m_directInter->MemFree(tmpUr);
			return 0;
		}
	}
	else
	{
		ur = tmpUr[0];
		m_directInter->MemFree(tmpUr);
		return 0;
	}
}


int MT4Conn::updateUserRecord(const UserRecord& ur, std::string& err)
{
	int code = m_directInter->UserRecordUpdate(&ur);
	if (code != RET_OK)
	{ 
		err = m_directInter->ErrorDescription(code);
		return code;
	}
	else
	{
		return code;
	}
}