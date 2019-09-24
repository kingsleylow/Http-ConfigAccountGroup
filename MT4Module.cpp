#include "MT4Module.h"
#include "Logger.h"
#include <chrono>
#include <thread>

MT4Conn::MT4Conn()
{
}


MT4Conn::~MT4Conn()
{
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
	return true;
}

bool MT4Conn::mt4Login(const int login, const char* passwd, CManagerInterface* manager)
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
		
	return true;
}

int MT4Conn::mt4Conn(const char* host, CManagerInterface* manager)
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
		break;
	case PUMP_UPDATE_SYMBOLS:
		pThis->storeSymbolsInfo();
		break;
	case PUMP_UPDATE_GROUPS:
		pThis->storeGroupsInfo();
		break;
	case PUMP_UPDATE_USERS:
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

bool MT4Conn::storeGroupsInfo()
{
	int total = 0;
	bool res = true;
	ConGroup* groupInfo = m_pumpInter->GroupsGet(&total);
	if (total)
	{
		for (int i = 0; i < total; i++)
		{
			m_GroupsInfo[groupInfo[i].group] = groupInfo[i];
		}
	}
	else
	{
		res = false;
	}
	m_pumpInter->MemFree(groupInfo);
	return res;
}

bool MT4Conn::storeSymbolsInfo()
{
	int total = 0;
	ConSymbol* symbols = m_pumpInter->SymbolsGetAll(&total);
	if (NULL == symbols)
		return false;
	for (int i = 0; i < total; i++)
	{
		m_SymbolsInfo[symbols[i].symbol] = symbols[i];
	}
	m_pumpInter->MemFree(symbols);
	return true;
}

bool MT4Conn::switchToPumpMode(CManagerInterface* managerInter)
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
		if (m_directInter->Ping() == RET_OK)
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

bool MT4Conn::createDirectConnToMT4(CManagerInterface* manager)
{
	if (mt4Login(std::stod(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), m_directInter))
		return true;
	else
		return false;
}
bool MT4Conn::createPumpConnToMT4(CManagerInterface* manager)
{
	if (m_pumpInter->IsConnected())
	{
		m_pumpInter->Disconnect();
	}
	m_pumpInter->Release();
	m_pumpInter = nullptr;
	if (nullptr == (m_pumpInter = m_factoryInter.Create(ManAPIVersion)))
		return false;


	if (mt4Login(std::stod(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), m_pumpInter) &&
		switchToPumpMode(m_pumpInter))
		return true;
	else
		return false;
}

bool MT4Conn::createConnToMT4()
{
	if (!mt4Init())
		return false;
	if (mt4Login(std::stod(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
		Config::getInstance().getMT4ConnConf().find(std::move("passwd"))->second.c_str(), m_directInter) &&
		mt4Login(std::stod(Config::getInstance().getMT4ConnConf().find(std::move("login"))->second),
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

void MT4Conn::watchConntoMT4()
{
	std::thread t([&]()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));

			if (!mt4DirtIsConnected())
			{
				Logger::getInstance()->warn("disconnected to mt4. will try to connect to mt4.");
				if (!createConnToMT4())
				{
					Logger::getInstance()->info("connect to mt4 failed.");
				}
			}

			if (!m_pumpInter->IsConnected())
			{
				if (!createPumpConnToMT4(m_pumpInter))
				{
					Logger::getInstance()->info("connect to mt4 failed.");
				}
			}
		}
	});
	t.detach();
}

ConGroup MT4Conn::getGroupCfg(const std::string& group)
{
	ConGroup ret = { 0 };
	ret = m_GroupsInfo[group];
	/*int total = 0;
	ConGroup* cfgGroup = nullptr;
	ConGroup ret = { 0 };
	
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
					if (group.compare(cfgGroup[i].group) == 0)
						ret = cfgGroup[i];
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

	m_directInter->MemFree(cfgGroup);*/
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

int MT4Conn::getSecuritiesNames(ConSymbolGroup securities[])
{
	int res = 0;
	int tryTimes = 0;
	do
	{
		if (mt4DirtIsConnected())
		{
			res = m_directInter->CfgRequestSymbolGroup(securities);
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

	return res;
}


bool MT4Conn::updateGroupSec(const std::string& group,const std::map<int, ConGroupSec>& cfgGroupSec, std::set<int> index)
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
		return false;
	}	

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group securities failed.{}, group:{},index:{}",
			m_directInter->ErrorDescription(res), group, tmpIndex);
		return false;
	}
	return true;
}


bool MT4Conn::updateGroupSymbol(const std::string& group, const std::map<std::string, ConGroupMargin>& cfgGroupMargin)
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
		return false;
	}

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group margins failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
	return true;
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

bool MT4Conn::updateGroupCommon(const std::string& group, const GroupCommon& common)
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
		return false;
	}
	

	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group common failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
	return true;
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

bool MT4Conn::updateGroupMargin(const std::string group, const GroupMargin& margin)
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
		return false;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group margin failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
	return true;
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
bool MT4Conn::updateGroupArchive(const std::string group, const GroupArchive& archive)
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
		return false;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group margin failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
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

bool MT4Conn::upateGroupReport(const std::string group, const GroupReport& report)
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
		return false;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group margin failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
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

bool MT4Conn::updateGroupPerssion(const std::string group, const GroupPermission& permission)
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
		return false;
	}


	int res = 0;
	if (RET_OK != (res = m_directInter->CfgUpdateGroup(&cfgGroup)))
	{
		Logger::getInstance()->error("update group margin failed.{}", m_directInter->ErrorDescription(res));
		return false;
	}
}

bool MT4Conn::updateAccounts(const std::string login, const AccountConfiguration& configuration)
{
	if (login.empty())
		return false;
	int _login = std::stoi(login);
	bool res = true;
	UserRecord ur = {0};
	int ret = m_pumpInter->UserRecordGet(_login, &ur);
	if (ret == RET_OK)
	{
		strncpy(ur.password, configuration.password.c_str(), configuration.password.length()+1);
		ur.enable_change_password = configuration.enable_change_password;
		if (RET_OK != (ret = m_directInter->UserRecordUpdate(&ur)))
		{
			Logger::getInstance()->info("error :{}", m_directInter->ErrorDescription(ret));
			res = false;
		}
		if (RET_OK != (ret = m_directInter->UserPasswordSet(_login, configuration.password.c_str(), 0, 0)))
		{
			Logger::getInstance()->info("error :{}", m_directInter->ErrorDescription(ret));
			res = false;
		}
	}
	else
	{
		Logger::getInstance()->info("error :{}", m_pumpInter->ErrorDescription(ret));
		res = false;
	}
	return res;
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

bool MT4Conn::getGlobalSymbols(std::vector<ConSymbol>& symbols)
{
	if (m_SymbolsInfo.empty())
		return false;
	for (auto& s : m_SymbolsInfo)
	{
		symbols.push_back(s.second);
	}	
	return true;
}

bool  MT4Conn::getGlobalSymbols(std::string& symbol, ConSymbol& con)
{
	if (m_SymbolsInfo.empty() || m_SymbolsInfo.find(symbol) == m_SymbolsInfo.end())
		return false;
	else
	{
		con = m_SymbolsInfo[symbol];
		return true;
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