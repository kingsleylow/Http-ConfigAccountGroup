#include "Utils.h"
#include <typeinfo>

Utils* Utils::m_self = nullptr;
std::mutex Utils::m_mtx;

Utils& Utils::getInstance()
{
	if (m_self == nullptr)
	{
		std::lock_guard<std::mutex> lgd(m_mtx);
		if (m_self == nullptr)
			m_self = new Utils();
	}
	return *m_self;
}

Utils::Utils()
{
	m_state[TS_OPEN_NORMAL] = "TS_OPEN_NORMAL";     //open
	m_state[TS_CLOSED_NORMAL] = "TS_CLOSED_NORMAL"; //close
	m_state[TS_CLOSED_PART] = "TS_CLOSED_PART";     //partially close
	m_state[TS_CLOSED_BY] = "TS_CLOSED_BY";         //close by an opposite order
	m_state[TS_DELETED] = "TS_DELETED";             //delete
	
	m_cmd[OP_BUY] = "OP_BUY";
	m_cmd[OP_SELL] = "OP_SELL";
	m_cmd[OP_BUY_LIMIT] = "OP_BUY_LIMIT";
	m_cmd[OP_SELL_LIMIT] = "OP_SELL_LIMIT";
	m_cmd[OP_BUY_STOP] = "OP_BUY_STOP";
	m_cmd[OP_SELL_STOP] = "OP_SELL_STOP";

	m_reason[TR_REASON_CLIENT] = "TR_REASON_CLIENT";
	m_reason[TR_REASON_DEALER] = "TR_REASON_DEALER";
	m_reason[TR_REASON_API] = "TR_REASON_API";
	m_reason[TR_REASON_MOBILE] = "TR_REASON_MOBILE";
	m_reason[TR_REASON_WEB] = "TR_REASON_WEB";

	m_activation[ACTIVATION_NONE] = "ACTIVATION_NONE";
	m_activation[ACTIVATION_SL] = "ACTIVATION_SL";
	m_activation[ACTIVATION_TP] = "ACTIVATION_TP";
	m_activation[ACTIVATION_PENDING] = "ACTIVATION_PENDING";
	m_activation[ACTIVATION_STOPOUT] = "ACTIVATION_STOPOUT";
	m_activation[ACTIVATION_SL_ROLLBACK] = "ACTIVATION_SL_ROLLBACK";
	m_activation[ACTIVATION_TP_ROLLBACK] = "ACTIVATION_TP_ROLLBACK";
	m_activation[ACTIVATION_PENDING_ROLLBACK] = "ACTIVATION_PENDING_ROLLBACK";
	m_activation[ACTIVATION_STOPOUT_ROLLBACK] = "ACTIVATION_STOPOUT_ROLLBACK";
}

void  Utils::split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter)
{
	int begin = 0;
	int end = in.find(delimeter);
	std::string tmp = in.substr(0, end);
	out.push_back(tmp);
	while (end != std::string::npos)
	{
		begin = end + 1;
		end = in.find(delimeter, end + 1);
		tmp = in.substr(begin, end - begin);
		out.push_back(tmp);
	}
}


std::string Utils::getProjectPath()
{
	std::string path;
	char p[MAX_PATH] = { 0 };
	if (GetModuleFileName(0, p, MAX_PATH))
	{
		path = p;
		path = path.substr(0, path.rfind("\\"));
		path += "\\";
	}
	return std::move(path);
}


const std::string& Utils::getCmdDes(int cmd)
{
	return m_cmd[cmd];
}
const std::string& Utils::getStateDes(int state)
{
	return m_state[state];
}
const std::string& Utils::getReasonDes(int reason)
{
	return m_reason[reason];
}
const std::string& Utils::getActivationDes(int activation)
{
	return m_activation[activation];
}


bool Utils::parseFromJsonToSec(const std::string json, std::map<int, ConGroupSec>& sec, std::set<int>& index, std::string& group)
{
	using namespace rapidjson;
	Document doc;
	
	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupSec") && doc["GroupSec"].IsArray())
	{
		for (auto& v: doc["GroupSec"].GetArray())
		{
			if (v.HasMember("index") && v["index"].IsInt())
			{
				index.insert(v["index"].GetInt());
			}
			else
			{
				return false;
			}

			ConGroupSec tmp = { 0 };
			sec[v["index"].GetInt()] = tmp;

			if (addInt(v, "show", sec[v["index"].GetInt()].show) &&
				addInt(v, "trade", sec[v["index"].GetInt()].trade) &&
				addInt(v, "execution", sec[v["index"].GetInt()].execution) &&
				addDouble(v, "comm_base", sec[v["index"].GetInt()].comm_base) &&
				addInt(v, "comm_type", sec[v["index"].GetInt()].comm_type) &&
				addInt(v, "comm_lots", sec[v["index"].GetInt()].comm_lots) &&
				addDouble(v, "comm_agent", sec[v["index"].GetInt()].comm_agent) &&
				addInt(v, "comm_agent_type", sec[v["index"].GetInt()].comm_agent_type) &&
				addInt(v, "spread_diff", sec[v["index"].GetInt()].spread_diff) &&
				addInt(v, "lot_min", sec[v["index"].GetInt()].lot_min) &&
				addInt(v, "lot_max", sec[v["index"].GetInt()].lot_max) &&
				addInt(v, "lot_step", sec[v["index"].GetInt()].lot_step) &&
				addInt(v, "ie_deviation", sec[v["index"].GetInt()].ie_deviation) &&
				addInt(v, "confirmation", sec[v["index"].GetInt()].confirmation) &&
				addInt(v, "trade_rights", sec[v["index"].GetInt()].trade_rights) &&
				addInt(v, "ie_quick_mode", sec[v["index"].GetInt()].ie_quick_mode) &&
				addInt(v, "autocloseout_mode", sec[v["index"].GetInt()].autocloseout_mode) &&
				addDouble(v, "comm_tax", sec[v["index"].GetInt()].comm_tax) &&
				addInt(v, "comm_agent_lots", sec[v["index"].GetInt()].comm_agent_lots) &&
				addInt(v, "freemargin_mode", sec[v["index"].GetInt()].freemargin_mode))
			{
				
			}
			else
			{
				return false;
			}
		}
	}
	else
		return false;

	return true;
}

bool Utils::parseFromSecToJson(const std::string& group,const ConGroupSec sec[],const int size, std::string& json)
{
	if (sec == nullptr)
		return false;

	using namespace rapidjson;
	
	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupSec");
	w.StartArray();

	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("index");
		w.Int(i);

		w.Key("show");
		w.Int(sec[i].show);

		w.Key("trade");
		w.Int(sec[i].trade);

		w.Key("execution");
		w.Int(sec[i].execution);

		w.Key("comm_base");
		w.Double(sec[i].comm_base);

		w.Key("comm_type");
		w.Int(sec[i].comm_type);

		w.Key("comm_lots");
		w.Int(sec[i].comm_lots);

		w.Key("comm_agent");
		w.Double(sec[i].comm_agent);

		w.Key("comm_agent_type");
		w.Int(sec[i].comm_agent_type);

		w.Key("spread_diff");
		w.Int(sec[i].spread_diff);

		w.Key("lot_min");
		w.Int(sec[i].lot_min);

		w.Key("lot_max");
		w.Int(sec[i].lot_max);

		w.Key("lot_step");
		w.Int(sec[i].lot_step);

		w.Key("ie_deviation");
		w.Int(sec[i].ie_deviation);

		w.Key("confirmation");
		w.Int(sec[i].confirmation);

		w.Key("trade_rights");
		w.Int(sec[i].trade_rights);

		w.Key("ie_quick_mode");
		w.Int(sec[i].ie_quick_mode);

		w.Key("autocloseout_mode");
		w.Int(sec[i].autocloseout_mode);

		w.Key("comm_tax");
		w.Double(sec[i].comm_tax);

		w.Key("comm_agent_lots");
		w.Int(sec[i].comm_agent_lots);

		w.Key("freemargin_mode");
		w.Int(sec[i].freemargin_mode);
		w.EndObject();
	}
	w.EndArray();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}


bool Utils::parseFromJsonToSymbol(const std::string json, std::map<std::string, ConGroupMargin>& margins, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupSymbol") && doc["GroupSymbol"].IsArray())
	{
		for (auto& v : doc["GroupSymbol"].GetArray())
		{
			ConGroupMargin tmp = { 0 };

			if (v.HasMember("symbol") && v["symbol"].IsString())
			{
				memcpy(tmp.symbol, v["symbol"].GetString(), v["symbol"].Size());
				margins[v["symbol"].GetString()] = tmp;
			}
			else
			{
				return false;
			}

			if (addDouble(v, "swap_short", margins[v["swap_short"].GetString()].swap_short) &&
				addDouble(v, "swap_long", margins[v["swap_long"].GetString()].swap_long) &&
				addDouble(v, "margin_divider", margins[v["margin_divider"].GetString()].margin_divider))
			{

			}
			else
			{
				return false;
			}
		}
	}
	else
		return false;

	return true;
}

bool Utils::parseFromSymbolToJson(const std::string& group, const ConGroupMargin margins[],const int size, std::string& json)
{
	if (margins == nullptr)
		return false;

	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupSymbol");
	w.StartArray();

	for (int i = 0; i < size; i++)
	{
		w.StartObject();

		w.Key("symbol");
		w.String(margins[i].symbol);

		w.Key("swap_short");
		w.Double(margins[i].swap_short);

		w.Key("swap_long");
		w.Double(margins[i].swap_long);

		w.Key("margin_divider");
		w.Double(margins[i].margin_divider);

		w.EndObject();
	}

	w.EndArray();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}

bool Utils::parseFromJsonToCommon(const std::string json, GroupCommon& common, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupCommon") && doc["GroupCommon"].IsObject())
	{
		Value o = doc["GroupCommon"].GetObject();

		if (addString(o, "group", common.group) &&
			addInt(o, "enable", common.enable) &&
			addInt(o, "otp_mode", common.otp_mode) &&
			addString(o, "company", common.company) &&
			addString(o, "support_page", common.support_page) &&
			addDouble(o, "default_deposit", common.default_deposit) &&
			addString(o, "currency", common.currency) &&
			addInt(o, "default_leverage", common.default_leverage) &&
			addDouble(o, "interestrate", common.interestrate))
		{

		}
		else
		{
			return false;
		}

	}
	else
		return false;

	return true;
}

bool Utils::parseFromCommonToJson(const std::string& group, const GroupCommon common, std::string& json)
{
	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupCommon");
	w.StartObject();

	w.Key("group");
	w.String(common.group.c_str());

	w.Key("enable");
	w.Int(common.enable);

	w.Key("otp_mode");
	w.Int(common.otp_mode);

	w.Key("company");
	w.String(common.company.c_str());

	w.Key("support_page");
	w.String(common.support_page.c_str());

	w.Key("default_deposit");
	w.Double(common.default_deposit);

	w.Key("currency");
	w.String(common.currency.c_str());

	w.Key("default_leverage");
	w.Int(common.default_leverage);

	w.Key("interestrate");
	w.Double(common.interestrate);

	w.EndObject();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}


bool Utils::parseFromJsonToMargin(const std::string json, GroupMargin& margin, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupMargin") && doc["GroupMargin"].IsObject())
	{
		Value o = doc["GroupMargin"].GetObject();

		if (addInt(o, "margin_call", margin.margin_call) &&
			addInt(o, "margin_stopout", margin.margin_stopout) &&
			addInt(o, "margin_type", margin.margin_type) &&
			addInt(o, "margin_mode", margin.margin_mode) &&
			addDouble(o, "credit", margin.credit) &&
			addInt(o, "stopout_skip_hedged", margin.stopout_skip_hedged) &&
			addInt(o, "hedge_largeleg", margin.hedge_large_leg))
		{

		}
		else
		{
			return false;
		}

	}
	else
		return false;

	return true;
}


bool Utils::parseFromMarginToJson(const std::string& group, const GroupMargin& margin, std::string& json)
{
	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupMargin");
	w.StartObject();

	w.Key("margin_call");
	w.Int(margin.margin_call);

	w.Key("margin_stopout");
	w.Int(margin.margin_stopout);

	w.Key("margin_type");
	w.Int(margin.margin_type);

	w.Key("margin_mode");
	w.Int(margin.margin_mode);

	w.Key("credit");
	w.Double(margin.credit);

	w.Key("stopout_skip_hedged");
	w.Double(margin.stopout_skip_hedged);

	w.Key("hedge_largeleg");
	w.Int(margin.hedge_large_leg);

	w.EndObject();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}

bool Utils::parseFromArchiveToJson(const std::string& group, const GroupArchive& archive, std::string& json)
{
	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupArchive");
	w.StartObject();

	w.Key("archive_period");
	w.Int(archive.archive_period);

	w.Key("archive_max_balance");
	w.Int(archive.archive_max_balance);

	w.Key("archive_pending_period");
	w.Int(archive.archive_pending_period);

	w.EndObject();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}
bool Utils::parseFromJsonToArchive(const std::string json, GroupArchive& archive, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupArchive") && doc["GroupArchive"].IsObject())
	{
		Value o = doc["GroupArchive"].GetObject();

		if (addInt(o, "archive_period", archive.archive_period) &&
			addInt(o, "archive_max_balance", archive.archive_max_balance) &&
			addInt(o, "archive_pending_period", archive.archive_pending_period))
		{

		}
		else
		{
			return false;
		}

	}
	else
		return false;

	return true;
}

bool Utils::parseFromReportToJson(const std::string& group, const GroupReport& report, std::string& json)
{
	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupReport");
	w.StartObject();

	w.Key("reports");
	w.Int(report.reports);

	w.Key("copies");
	w.Int(report.copies);

	w.Key("smtp_server");
	w.String(report.smtp_server.c_str());

	w.Key("template_path");
	w.String(report.template_path.c_str());

	w.Key("smtp_login");
	w.String(report.smtp_login.c_str());

	w.Key("smtp_passwd");
	w.String(report.smtp_passwd.c_str());

	w.Key("support_email");
	w.String(report.support_email.c_str());

	w.Key("signature");
	w.String(report.signature.c_str());

	w.EndObject();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}
bool Utils::parseFromJsonToReport(const std::string json, GroupReport& report, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupReport") && doc["GroupReport"].IsObject())
	{
		Value o = doc["GroupReport"].GetObject();

		if (addInt(o, "reports", report.reports) &&
			addString(o, "smtp_server", report.smtp_server) &&
			addString(o, "template_path", report.template_path) &&
			addString(o, "smtp_login", report.smtp_login) &&
			addString(o, "smtp_passwd", report.smtp_passwd) &&
			addString(o, "support_email", report.support_email) &&
			addString(o, "signature", report.signature) &&
			addInt(o, "copies", report.copies))
		{

		}
		else
		{
			return false;
		}

	}
	else
		return false;

	return true;
}

bool Utils::parseFromPermissionToJson(const std::string& group, const GroupPermission& permission, std::string& json)
{
	using namespace rapidjson;

	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("Group");
	w.String(group.c_str());

	w.Key("GroupPermission");
	w.StartObject();

	w.Key("timeout");
	w.Int(permission.timeout);

	w.Key("news");
	w.Int(permission.news);

	w.Key("news_language");
	w.StartArray();
	for (int i = 0; i < 8; i++)
	{
		w.Int(permission.news_language[i]);
	}
	w.EndArray();

	w.Key("news_language_total");
	w.Int(permission.news_language_total);

	w.Key("maxsecurities");
	w.Int(permission.maxsecurities);

	w.Key("use_swap");
	w.Int(permission.use_swap);

	w.Key("rights");
	w.Int(permission.rights);

	w.Key("check_ie_prices");
	w.Int(permission.check_ie_prices);

	w.Key("maxpositions");
	w.Int(permission.maxpositions);

	w.Key("close_reopen");
	w.Int(permission.close_reopen);

	w.Key("hedge_prohibited");
	w.Int(permission.hedge_prohibited);

	w.Key("close_fifo");
	w.Int(permission.close_fifo);

	w.Key("unused_rights");
	w.StartArray();
	for (int i = 0; i < 2; i++)
	{
		w.Int(permission.unused_rights[i]);
	}
	w.EndArray();

	w.Key("securities_hash");
	w.String(permission.securities_hash.c_str());

	w.EndObject();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}
bool Utils::parseFromJsonToPermission(const std::string json, GroupPermission& permission, std::string& group)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("Group") && doc["Group"].IsString())
	{
		group = doc["Group"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("GroupPermission") && doc["GroupPermission"].IsObject())
	{
		Value o = doc["GroupPermission"].GetObject();

		int timeout = 0;
		int news = 0;
		int news_language[8] = { 0 };
		int news_language_total = 0;
		int maxsecurities = 0;

		int use_swap = 0;
		int rights = 0;                      // Flags of group permissions
		int check_ie_prices = 0;             // Check the prices of request in the Instant Execution mode
		int maxpositions = 0;                // Limit of open positions
		int close_reopen = 0;                // Partial closure mode
		int hedge_prohibited = 0;            // Hedging not allowed
		int close_fifo = 0;                  // Enable closing by FIFO rule
		int unused_rights[2] = { 0 };          // A reserved field
		std::string securities_hash = "";    // Internal data
		if(addInt(o, "timeout", permission.timeout) && 
			addInt(o, "news", permission.news) &&
			addInt(o, "news_language_total", permission.news_language_total)&&
			addInt(o, "maxsecurities", permission.maxsecurities) &&
			addInt(o, "use_swap", permission.use_swap) &&
			addInt(o, "rights", permission.rights) &&
			addInt(o, "check_ie_prices", permission.check_ie_prices) &&
			addInt(o, "maxpositions", permission.maxpositions) &&
			addInt(o, "close_reopen", permission.close_reopen) &&
			addInt(o, "hedge_prohibited", permission.hedge_prohibited) &&
			addInt(o, "close_fifo", permission.close_fifo) &&
			addString(o, "securities_hash", permission.securities_hash) &&
			addIntArray(o, "news_language", permission.news_language) &&
			addIntArray(o, "unused_rights", permission.unused_rights))
		{

		}
		else
		{
			return false;
		}
	}
	else
		return false;

	return true;
}

bool Utils::parseFromCommmonSecuritesToJson(const ConSymbolGroup securites[], const int size, std::string& json)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);

	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();

		w.Key("name");
		w.String(securites[i].name);

		w.Key("description");
		w.String(securites[i].description);

		w.EndObject();
	}
	w.EndArray();

	json = strBuf.GetString();

	return true;
}

bool Utils::parseFromCommonGroupsToJson(const std::vector<std::string>& groups, std::string& json)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> w(strBuf);
	w.StartObject();

	w.Key("total");
	w.Int(groups.size());

	w.Key("groups");
	w.StartArray();

	for (auto &v : groups)
	{
		w.String(v.c_str());
	}

	w.EndArray();

	w.EndObject();

	json = strBuf.GetString();

	return true;
}

bool Utils::addInt64(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, long& value)
{
	if (key.empty())
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsInt64())
	{
		value = obj[key.c_str()].GetInt64();
		return true;
	}
	else
		return false;
}

bool Utils::addInt(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, int& value)
{
	if (key.empty())
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsInt())
	{
		value = obj[key.c_str()].GetInt();
		return true;
	}
	else
		return false;
}
bool Utils::addDouble(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, double& value)
{
	if (key.empty())
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsDouble())
	{
		value = obj[key.c_str()].GetDouble();
		return true;
	}
	else if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsInt64())
	{
		value = obj[key.c_str()].GetInt64();
		return true;
	}
	else
		return false;
}
bool Utils::addString(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, std::string& value)
{
	if (key.empty())
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsString())
	{
		value = obj[key.c_str()].GetString();
		return true;
	}
	else
		return false;
}

bool Utils::addString(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, char* value, int len)
{
	if (key.empty() || value == nullptr)
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsString())
	{
		if (obj[key.c_str()].GetStringLength() > len)
			return false;
		memcpy(value, obj[key.c_str()].GetString(), obj[key.c_str()].GetStringLength());
	}
	else
		return false;

	return true;
}

bool Utils::addIntArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, int* value)
{
	assert(value);
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsArray())
	{
		int size = obj[key.c_str()].Size();
		auto a = obj[key.c_str()].GetArray();
		for (int i = 0; i < size; i++)
			value[i] = a[i].GetInt();
		return true;
	}
	else
		return false;
}

bool Utils::addDoubleArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, double* value)
{
	assert(value);
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsArray())
	{
		int size = obj[key.c_str()].Size();
		auto a = obj[key.c_str()].GetArray();
		for (int i = 0; i < size; i++)
			value[i] = a[i].GetDouble();
		return true;
	}
	else
		return false;
}


bool Utils::addConSession(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, ConSessions* session, int size)
{
	if (session == nullptr)
		return false;
	if (obj.HasMember(key.c_str()) && obj[key.c_str()].IsArray())
	{
		auto arr = obj[key.c_str()].GetArray();
		for (int i = 0; i < size; i++)
		{
			if (arr[i].HasMember("quote") && arr[i].IsArray())
			{
				auto quoteArr = arr[i].GetArray();
				for (int j = 0; j < 3; j++)
				{
					if (quoteArr[j].HasMember("open_hour") && quoteArr[j].IsInt())
						session[i].quote[j].open_hour = quoteArr[j].GetInt();
					else
						return false;

					if (quoteArr[j].HasMember("open_min") && quoteArr[j].IsInt())
						session[i].quote[j].open_min = quoteArr[j].GetInt();
					else
						return false;

					if (quoteArr[j].HasMember("close_hour") && quoteArr[j].IsInt())
						session[i].quote[j].close_hour = quoteArr[j].GetInt();
					else
						return false;

					if (quoteArr[j].HasMember("close_min") && quoteArr[j].IsInt())
						session[i].quote[j].close_min = quoteArr[j].GetInt();
					else
						return false;
				}
			}
			else
				return false;
		}
	}
	else
		return false;

	return true;
}

bool Utils::addRateInfoArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &arr, std::string key, RateInfo*& rates, int size)
{
	rates = new RateInfo[size];
	memset(rates, 0, sizeof(RateInfo)*size);
	if (!arr.IsArray() || arr.Size() != size)
		return false;
	for (int i = 0; i < size; i++)
	{
		auto& obj = arr[i];
		if (addInt(obj, "ctm", (int&)rates[i].ctm) &&
			addInt(obj, "open", rates[i].open) &&
			addInt(obj, "high", rates[i].high) &&
			addInt(obj, "low", rates[i].low) &&
			addInt(obj, "close", rates[i].close) &&
			addDouble(obj, "volume", rates[i].vol))
			return true;
		else
			return false;
	}
	return true;
}

bool Utils::intToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, int value)
{
	w.Key(key.c_str());
	w.Int(value);
	return true;
}
bool Utils::doubleToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, double value)
{
	w.Key(key.c_str());
	w.Double(value);
	return true;
}
bool Utils::stringToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, std::string value)
{
	w.Key(key.c_str());
	w.String(value.c_str());
	return true;
}
bool Utils::charArrToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, char* value)
{
	w.Key(key.c_str());
	w.String(value);
	return true;
}

bool Utils::parseFromJsonToAccuntConfiguration(const std::string& json, AccountConfiguration& configuration, std::string& login)
{
	using namespace rapidjson;
	Document doc;

	if (doc.Parse(json.c_str()).HasParseError())
	{
		return false;
	}

	if (doc.HasMember("login") && doc["login"].IsString())
	{
		login = doc["login"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("password") && doc["password"].IsString())
	{
		configuration.password = doc["password"].GetString();
	}
	else
	{
		return false;
	}

	if (doc.HasMember("enable_change_password") && doc["enable_change_password"].IsInt())
	{
		configuration.enable_change_password = doc["enable_change_password"].GetInt();
	}
	else
	{
		return false;
	}
	return true;
}

bool Utils::parseFromDatafeedToJson(const ConFeeder feeder[], const int size, std::string& json)
{
	if (nullptr == feeder || 0 == size)
		return false;
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	
	w.StartObject();
	w.Key("datafeed");
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("name");
		w.String(feeder[i].name);
		w.Key("file");
		w.String(feeder[i].file);
		w.Key("server");
		w.String(feeder[i].server);
		w.Key("login");
		w.String(feeder[i].login);
		w.Key("pass");
		w.String(feeder[i].pass);
		w.Key("keywords");
		w.String(feeder[i].keywords);
		w.Key("enable");
		w.Int(feeder[i].enable);
		w.Key("mode");
		w.Int(feeder[i].mode);
		w.Key("timeout");
		w.Int(feeder[i].timeout);
		w.Key("timeout_reconnect");
		w.Int(feeder[i].timeout_reconnect);
		w.Key("timeout_sleep");
		w.Int(feeder[i].timeout_sleep);
		w.Key("attemps_sleep");
		w.Int(feeder[i].attemps_sleep);
		w.Key("news_langid");
		w.Int(feeder[i].news_langid);
		w.EndObject();
	}
	w.EndArray();
	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromGlobalCommonToJson(const ConCommon& common, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("owner");
	w.String(common.owner);
	w.Key("name");
	w.String(common.name);
	w.Key("address");
	w.Uint(common.address);
	w.Key("port");
	w.Int(common.port);
	w.Key("timeout");
	w.Uint(common.timeout);
	w.Key("enable_demo");
	w.Int(common.typeofdemo);
	w.Key("timeofdemo");
	w.Int(common.timeofdemo);
	w.Key("daylightcorrection");
	w.Int(common.daylightcorrection);
	w.Key("timezone_real");
	w.Int(common.timezone_real);
	w.Key("timezone");
	w.Int(common.timezone);
	w.Key("timesync");
	w.String(common.timesync);

	w.Key("minclient");
	w.Int(common.minclient);
	w.Key("minapi");
	w.Int(common.minapi);
	w.Key("feeder_timeout");
	w.Uint(common.feeder_timeout);
	w.Key("keepemails");
	w.Int(common.keepemails);
	w.Key("endhour");
	w.Int(common.endhour);
	w.Key("endminute");
	w.Int(common.endminute);

	w.Key("optimization_time");
	w.Int(common.optimization_time);
	w.Key("optimization_lasttime");
	w.Int(common.optimization_lasttime);
	w.Key("optimization_counter");
	w.Int(common.optimization_counter);

	w.Key("antiflood");
	w.Int(common.antiflood);
	w.Key("floodcontrol");
	w.Int(common.floodcontrol);
	w.Key("liveupdate_mode");
	w.Int(common.liveupdate_mode);

	w.Key("lastorder");
	w.Int(common.lastorder);
	w.Key("lastlogin");
	w.Int(common.lastlogin);
	w.Key("lostlogin");
	w.Int(common.lostlogin);

	w.Key("rollovers_mode");
	w.Int(common.rollovers_mode);

	w.Key("path_database");
	w.String(common.path_database);
	w.Key("path_history");
	w.String(common.path_history);
	w.Key("path_log");
	w.String(common.path_log);

	w.Key("overnight_last_day");
	w.Uint(common.overnight_last_day);
	w.Key("overnight_last_time");
	w.Uint(common.overnight_last_time);
	w.Key("overnight_prev_time");
	w.Uint(common.overnight_prev_time);

	w.Key("overmonth_last_month");
	w.Uint(common.overmonth_last_month);

	w.Key("adapters");
	w.String(common.adapters);
	w.Key("bind_adresses");
	w.StartArray();
	for (int i = 0; i < 8; i++)
	{
		w.Uint(common.bind_adresses[i]);
	}
	w.EndArray();
	w.Key("server_version");
	w.Int(common.server_version);
	w.Key("server_build");
	w.Int(common.server_build);
	w.Key("web_adresses");
	w.StartArray();
	for (int i = 0; i < 8; i++)
	{
		w.Uint(common.web_adresses[i]);
	}
	w.EndArray();
	w.Key("statement_mode");
	w.Int(common.statement_mode);
	w.Key("monthly_state_mode");
	w.Int(common.monthly_state_mode);
	w.Key("keepticks");
	w.Int(common.keepticks);
	w.Key("statement_weekend");
	w.Int(common.statement_weekend);
	w.Key("last_activate");
	w.Uint(common.last_activate);
	w.Key("stop_last");
	w.Uint(common.stop_last);
	w.Key("stop_delay");
	w.Int(common.stop_delay);
	w.Key("stop_reason");
	w.Int(common.stop_reason);
	w.Key("account_url");
	w.String(common.account_url);
	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromIPListToJson(const ConAccess ip[], const int size, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("Access-List");
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("action");
		w.Int(ip[i].action);
		w.Key("from");
		w.Uint(ip[i].from);
		w.Key("to");
		w.Uint(ip[i].to);
		w.Key("comment");
		w.String(ip[i].comment);
		w.EndObject();
	}
	w.EndArray();
	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromSymbolsListToJson(const std::vector<ConSymbol>& symbols, std::string& json)
{
	if (symbols.empty())
		return false;
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartArray();
	for (auto& s : symbols)
	{
		w.String(s.symbol);
	}
	w.EndArray();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromSymbolToJson(const ConSymbol& symbol, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("symbol");
	w.String(symbol.symbol);
	w.Key("description");
	w.String(symbol.description);
	w.Key("source");
	w.String(symbol.source);
	w.Key("currency");
	w.String(symbol.currency);
	w.Key("type");
	w.Int(symbol.type);
	w.Key("digits");
	w.Int(symbol.digits);
	w.Key("trade");
	w.Int(symbol.trade);

	w.Key("background_color");
	w.Int(symbol.background_color);
	w.Key("count");
	w.Int(symbol.count);
	w.Key("count_original");
	w.Int(symbol.count_original);

	w.Key("realtime");
	w.Int(symbol.realtime);
	w.Key("starting");
	w.Uint(symbol.starting);
	w.Key("expiration");
	w.Uint(symbol.expiration);

	w.Key("sessions");
	w.StartArray();
	for(int i = 0; i < 7; i++)
	{
		w.StartObject();
		w.Key("quote_session");
		w.StartArray();
		for (int j = 0; j < 3; j++)
		{
			w.StartObject();
			w.Key("open_hour");
			w.Int(symbol.sessions[i].quote[j].open_hour);
			w.Key("open_min");
			w.Int(symbol.sessions[i].quote[j].open_min);
			w.Key("close_hour");
			w.Int(symbol.sessions[i].quote[j].close_hour);
			w.Key("close_min");
			w.Int(symbol.sessions[i].quote[j].close_min);
			w.EndObject();
		}
		w.EndArray();

		w.Key("trade_session");
		w.StartArray();
		for (int j = 0; j < 3; j++)
		{
			w.StartObject();
			w.Key("open_hour");
			w.Int(symbol.sessions[i].trade[j].open_hour);
			w.Key("open_min");
			w.Int(symbol.sessions[i].trade[j].open_min);
			w.Key("close_hour");
			w.Int(symbol.sessions[i].trade[j].close_hour);
			w.Key("close_min");
			w.Int(symbol.sessions[i].trade[j].close_min);
			w.EndObject();
		}
		w.EndArray();
		w.EndObject();
	}
	w.EndArray();

	w.Key("profit_mode");
	w.Int(symbol.profit_mode);

	w.Key("filter");
	w.Int(symbol.filter);

	w.Key("filter_counter");
	w.Int(symbol.filter_counter);

	w.Key("filter_limit");
	w.Double(symbol.filter_limit);

	w.Key("filter_smoothing");
	w.Int(symbol.filter_smoothing);

	w.Key("logging");
	w.Int(symbol.logging);

	w.Key("spread");
	w.Int(symbol.spread);

	w.Key("spread_balance");
	w.Int(symbol.spread_balance);

	w.Key("exemode");
	w.Int(symbol.exemode);

	w.Key("swap_enable");
	w.Int(symbol.swap_enable);

	w.Key("swap_type");
	w.Int(symbol.swap_type);

	w.Key("swap_long");
	w.Double(symbol.swap_long);

	w.Key("swap_short");
	w.Double(symbol.swap_short);

	w.Key("swap_rollover3days");
	w.Int(symbol.swap_rollover3days);

	w.Key("contract_size");
	w.Double(symbol.contract_size);

	w.Key("tick_value");
	w.Double(symbol.tick_value);

	w.Key("tick_size");
	w.Double(symbol.tick_size);
	//
	w.Key("stops_level");
	w.Int(symbol.stops_level);

	w.Key("gtc_pendings");
	w.Int(symbol.gtc_pendings);

	w.Key("margin_mode");
	w.Int(symbol.margin_mode);

	w.Key("margin_initial");
	w.Double(symbol.margin_initial);

	w.Key("margin_maintenance");
	w.Double(symbol.margin_maintenance);

	w.Key("margin_hedged");
	w.Double(symbol.margin_hedged);

	w.Key("margin_divider");
	w.Double(symbol.margin_divider);

	w.Key("point");
	w.Double(symbol.point);

	w.Key("multiply");
	w.Double(symbol.multiply);

	w.Key("bid_tickvalue");
	w.Double(symbol.bid_tickvalue);

	w.Key("ask_tickvalue");
	w.Double(symbol.ask_tickvalue);

	w.Key("long_only");
	w.Int(symbol.long_only);

	w.Key("instant_max_volume");
	w.Int(symbol.instant_max_volume);

	w.Key("margin_currency");
	w.String(symbol.margin_currency);

	w.Key("freeze_level");
	w.Int(symbol.freeze_level);

	w.Key("margin_hedged_strong");
	w.Int(symbol.margin_hedged_strong);

	w.Key("value_date");
	w.Uint(symbol.value_date);

	w.Key("quotes_delay");
	w.Int(symbol.quotes_delay);

	w.Key("swap_openprice");
	w.Int(symbol.swap_openprice);

	w.Key("swap_variation_margin");
	w.Int(symbol.swap_variation_margin);

	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromJsonToSymbol(const std::string& json, std::string& symbol)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
		return false;
	if (d.HasMember("symbol") && d["symbol"].IsString())
	{
		symbol = d["symbol"].GetString();
		return true;
	}
	else
		return false;
}

bool Utils::parseFromDCListToJson(const ConDataServer dc[], const int size, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("DC-List");
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("server");
		w.String(dc[i].server);

		w.Key("ip");
		w.Uint(dc[i].ip);

		w.Key("description");
		w.String(dc[i].description);

		w.Key("isproxy");
		w.Int(dc[i].isproxy);

		w.Key("priority");
		w.Int(dc[i].priority);

		w.Key("loading");
		w.Uint(dc[i].loading);

		w.Key("ip_internal");
		w.Uint(dc[i].ip_internal);

		w.EndObject();
	}
	w.EndArray();
	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromPluginListToJson(const ConPluginParam cp[], const int size, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("Plugin-List");
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("plugin");
		w.StartObject();
		w.Key("file");
		w.String(cp[i].plugin.file);

		w.Key("info");
		w.StartObject();
		w.Key("name");
		w.String(cp[i].plugin.info.name);
		w.Key("version");
		w.Uint(cp[i].plugin.info.version);
		w.Key("copyright");
		w.String(cp[i].plugin.info.copyright);
		w.EndObject();

		w.Key("enabled");
		w.Int(cp[i].plugin.enabled);

		w.Key("configurable");
		w.Int(cp[i].plugin.configurable);

		w.Key("manager_access");
		w.Int(cp[i].plugin.manager_access);

		w.EndObject();
		w.Key("params-total");
		w.Int(cp[i].total);
		w.Key("params");
		w.StartArray();
		for (int j = 0; j < cp[i].total; j++)
		{
			w.StartObject();
			w.Key("name");
			w.String(cp[i].params[j].name);
			w.Key("value");
			w.String(cp[i].params[j].value);
			w.EndObject();
		}
		w.EndArray();
		w.EndObject();
	}
	w.EndArray();
	w.EndObject();

	json = sb.GetString();
	return true;
}

bool Utils::parseFromPerformanceToJson(const PerformanceInfo pi[], const int size, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartObject();
	w.Key("Performance");
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("ctm");
		w.Uint(pi[i].ctm);
		w.Key("users");
		w.Int(pi[i].users);
		w.Key("cpu");
		w.Int(pi[i].cpu);
		w.Key("freemem");
		w.Int(pi[i].freemem);
		w.Key("network");
		w.Int(pi[i].network);
		w.Key("sockets");
		w.Int(pi[i].sockets);
		w.EndObject();
	}
	w.EndArray();
	w.EndObject();
	json = sb.GetString();
	return true;
}
bool Utils::parseFromJsonToPerformance(const std::string& json, unsigned int &seconds)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
		return false;
	if (d.HasMember("from") && d["from"].IsUint())
	{
		seconds = d["from"].GetUint();
		return true;
	}
	else
		return false;
}

bool Utils::parseFromHolidayToJson(const ConHoliday ch[], const int size, std::string& json)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("year");
		w.Int(ch[i].year);
		w.Key("month");
		w.Int(ch[i].month);
		w.Key("day");
		w.Int(ch[i].day);
		w.Key("from");
		w.Int(ch[i].from);
		w.Key("to");
		w.Int(ch[i].to);
		w.Key("symbol");
		w.String(ch[i].symbol);
		w.Key("description");
		w.String(ch[i].description);
		w.Key("enable");
		w.Int(ch[i].enable);
		w.EndObject();
	}
	w.EndArray();
	json = sb.GetString();
	return true;
}

bool Utils::parseFromJsonToSession(const std::string& json, ConSessions (&css)[7], std::string& symbol)
{
	if (nullptr == css)
		return false;
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
		return false;
	if (d.HasMember("symbol") && d["symbol"].IsString())
	{
		symbol = d["symbol"].GetString();
	}
	else
	{
		return false;
	}
		
	if (d.HasMember("sessions") && d["sessions"].IsArray())
	{
		//for (auto& s : d["sessions"].GetArray())
		for(int i = 0; i < 7; i++)
		{
			if (d["sessions"].GetArray()[i].IsObject())
			{
				if (d["sessions"].GetArray()[i].HasMember("quote_session") && d["sessions"].GetArray()[i]["quote_session"].IsArray())
				{
					for (int j = 0; j < 3; j++)
					{
						if (d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].HasMember("open_hour"))
						{
							css[i].quote[j].open_hour = d["sessions"].GetArray()[i]["quote_session"].GetArray()[j]["open_hour"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].HasMember("open_min"))
						{
							css[i].quote[j].open_min = d["sessions"].GetArray()[i]["quote_session"].GetArray()[j]["open_min"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].HasMember("close_hour"))
						{
							css[i].quote[j].close_hour = d["sessions"].GetArray()[i]["quote_session"].GetArray()[j]["close_hour"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["quote_session"].GetArray()[j].HasMember("close_min"))
						{
							css[i].quote[j].close_min = d["sessions"].GetArray()[i]["quote_session"].GetArray()[j]["close_min"].GetInt();
						}
					}
				}
				else
				{
					return false;
				}
				if (d["sessions"].GetArray()[i].HasMember("trade_session") && d["sessions"].GetArray()[i]["trade_session"].IsArray())
				{
					for (int j = 0; j < 3; j++)
					{
						if (d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].HasMember("open_hour"))
						{
							css[i].trade[j].open_hour = d["sessions"].GetArray()[i]["trade_session"].GetArray()[j]["open_hour"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].HasMember("open_min"))
						{
							css[i].trade[j].open_min = d["sessions"].GetArray()[i]["trade_session"].GetArray()[j]["open_min"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].HasMember("close_hour"))
						{
							css[i].trade[j].close_hour = d["sessions"].GetArray()[i]["trade_session"].GetArray()[j]["close_hour"].GetInt();
						}
						if (d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].IsObject() && d["sessions"].GetArray()[i]["trade_session"].GetArray()[j].HasMember("close_min"))
						{
							css[i].trade[j].close_min = d["sessions"].GetArray()[i]["trade_session"].GetArray()[j]["close_min"].GetInt();
						}
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	else
		return false;
}

bool Utils::parseFromJsonToSwap(const std::string& body, std::string& symbol, int& swap_long, int& swap_short, int& swap_enable, int& swap_rollover)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;
	if (addString(d, "symbol", symbol) &&
		addInt(d, "swap_long", swap_long) &&
		addInt(d, "swap_short", swap_short) &&
		addInt(d, "swap_enable", swap_enable) &&
		addInt(d, "swap_rollover3days", swap_rollover))
		return true;
	else
		return false;
}

bool Utils::parseFromJsonToOpenPrice(const std::string& body, int& orderNo, double& profit)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;
	if (d.HasMember("orderNo") && d["orderNo"].IsString())
	{
		orderNo = std::stoi(d["orderNo"].GetString());
	}
	else
	{
		return false;
	}

	if (d.HasMember("profit") && d["profit"].IsString())
	{
		profit = std::stod(d["profit"].GetString());
	}
	else
	{
		return false;
	}
	return true;
}

std::string Utils::serializeConSymbolToJson(int code, int type, const ConSymbol& cs)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();

	w.Key("symbol");
	w.String(cs.symbol);

	w.Key("description");
	w.String(cs.description);

	w.Key("source");
	w.String(cs.source);

	w.Key("currency");
	w.String(cs.currency);

	w.Key("type");
	w.Int(cs.type);

	w.Key("digits");
	w.Int(cs.digits);

	w.Key("trade");
	w.Int(cs.trade);

	w.Key("background_color");
	w.Int(cs.background_color);

	w.Key("count"); //position
	w.Int(cs.count);

	w.Key("count_original");
	w.Int(cs.count_original);

	w.Key("realtime");
	w.Int(cs.realtime);

	w.Key("starting");
	w.Int(cs.starting);

	w.Key("expiration");
	w.Int(cs.expiration);

	w.Key("sessions");
	w.StartArray();
	for (int i = 0; i < 7; i++)
	{
		w.StartObject();
		w.Key("quote");
		w.StartArray();
		for (int j = 0; j < 3; j++)
		{
			w.StartObject();
			w.Key("open_hour");
			w.Int(cs.sessions[i].quote[j].open_hour);
			w.Key("open_min");
			w.Int(cs.sessions[i].quote[j].open_min);
			w.Key("close_hour");
			w.Int(cs.sessions[i].quote[j].close_hour);
			w.Key("close_min");
			w.Int(cs.sessions[i].quote[j].close_min);
			w.EndObject();
		}
		w.EndArray();

		w.Key("trade");
		w.StartArray();
		for (int j = 0; j < 3; j++)
		{
			w.StartObject();
			w.Key("open_hour");
			w.Int(cs.sessions[i].trade[j].open_hour);
			w.Key("open_min");
			w.Int(cs.sessions[i].trade[j].open_min);
			w.Key("close_hour");
			w.Int(cs.sessions[i].trade[j].close_hour);
			w.Key("close_min");
			w.Int(cs.sessions[i].trade[j].close_min);
			w.EndObject();
		}
		w.EndArray();
		w.EndObject();
	}
	w.EndArray();

	w.Key("profit_mode");
	w.Int(cs.profit_mode);

	w.Key("filter");
	w.Int(cs.filter);

	w.Key("filter_counter");
	w.Int(cs.filter_counter);

	w.Key("filter_limit");
	w.Double(cs.filter_limit);

	w.Key("filter_smoothing");
	w.Int(cs.filter_smoothing);

	w.Key("logging");
	w.Int(cs.logging);

	w.Key("spread");
	w.Int(cs.spread);

	w.Key("spread_balance");
	w.Int(cs.spread_balance);

	w.Key("exemode");
	w.Int(cs.exemode);

	w.Key("swap_enable");
	w.Int(cs.swap_enable);

	w.Key("swap_type");
	w.Int(cs.swap_type);

	w.Key("swap_long");
	w.Int64(cs.swap_long);

	w.Key("swap_short");
	w.Int64(cs.swap_short);

	w.Key("swap_rollover3days");
	w.Int(cs.swap_rollover3days);

	w.Key("contract_size");
	w.Double(cs.contract_size);

	w.Key("tick_value");
	w.Double(cs.tick_value);

	w.Key("tick_size");
	w.Double(cs.tick_size);

	w.Key("stops_level");
	w.Int(cs.stops_level);

	w.Key("gtc_pendings");
	w.Int(cs.gtc_pendings);

	w.Key("margin_mode");
	w.Int(cs.margin_mode);

	w.Key("margin_initial");
	w.Double(cs.margin_initial);

	w.Key("margin_maintenance");
	w.Double(cs.margin_maintenance);

	w.Key("margin_hedged");
	w.Double(cs.margin_hedged);

	w.Key("margin_divider");
	w.Double(cs.margin_divider);

	w.Key("point");
	w.Double(cs.point);

	w.Key("multiply");
	w.Double(cs.multiply);

	w.Key("bid_tickvalue");
	w.Double(cs.bid_tickvalue);

	w.Key("ask_tickvalue");
	w.Double(cs.ask_tickvalue);

	w.Key("long_only");
	w.Int(cs.long_only);

	w.Key("instant_max_volume");
	w.Int(cs.instant_max_volume);

	w.Key("margin_currency");
	w.String(cs.margin_currency);

	w.Key("freeze_level");
	w.Int(cs.freeze_level);

	w.Key("margin_hedged_strong");
	w.Int(cs.margin_hedged_strong);

	w.Key("value_date");
	w.Int(cs.value_date);

	w.Key("quote_delay");
	w.Int(cs.quotes_delay);

	w.Key("swap_openprice");
	w.Int(cs.swap_openprice);

	w.Key("swap_variation_margin");
	w.Int(cs.swap_variation_margin);

	w.EndObject();

	return sb.GetString();
}

bool Utils::unSerializeConSymbolToJson(const std::string& body, ConSymbol& cs)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;

	if (addString(d, "symbol", cs.symbol, 12) &&
		addString(d, "description", cs.description, 64) &&
		addString(d, "source", cs.source, 12) &&
		addString(d, "currency", cs.currency, 12) &&
		addInt(d, "type", cs.type) &&
		addInt(d, "digits", cs.digits) &&
		addInt(d, "trade", cs.trade) &&
		addInt(d, "background_color", (int&)cs.background_color) &&
		addInt(d, "count", cs.count) &&
		addInt(d, "count_original", cs.count_original) &&
		addInt(d, "realtime", cs.realtime) &&
		addInt(d, "starting", (int&)cs.starting) &&
		addInt(d, "expiration", (int&)cs.expiration) &&
		addInt(d, "profit_mode", cs.profit_mode) &&
		addInt(d, "filter", cs.filter) &&
		addInt(d, "filter_counter", cs.filter_counter) &&
		addDouble(d, "filter_limit", cs.filter_limit) &&
		addInt(d, "filter_smoothing", cs.filter_smoothing) &&
		addInt(d, "logging", cs.logging) &&
		addInt(d, "spread", cs.spread) &&
		addInt(d, "spread_balance", cs.spread_balance) &&
		addInt(d, "exemode", cs.exemode) &&
		addInt(d, "swap_enable", cs.swap_enable) &&
		addInt(d, "swap_type", cs.swap_type) &&
		addDouble(d, "swap_long", cs.swap_long) &&
		addDouble(d, "swap_short", cs.swap_short) &&
		addInt(d, "swap_rollover3days", cs.swap_rollover3days) &&
		addDouble(d, "contract_size", cs.contract_size) &&
		addDouble(d, "tick_value", cs.tick_size) &&
		addDouble(d, "tick_size", cs.tick_size) &&
		addInt(d, "stop_level", cs.stops_level) &&
		addInt(d, "gtc_pending", cs.gtc_pendings) &&
		addInt(d, "margin_mode", cs.margin_mode) &&
		addDouble(d, "margin_initial", cs.margin_initial) &&
		addDouble(d, "margin_maintenance", cs.margin_maintenance) &&
		addDouble(d, "margin_hedged", cs.margin_hedged) &&
		addDouble(d, "margin_divider", cs.margin_divider) &&
		addDouble(d, "point", cs.point) &&
		addDouble(d, "multiply", cs.multiply) &&
		addDouble(d, "bid_tickvalue", cs.bid_tickvalue) &&
		addDouble(d, "ask_tickvalue", cs.ask_tickvalue) &&
		addInt(d, "long_only", cs.long_only) &&
		addInt(d, "instant_max_volume", cs.instant_max_volume) &&
		addString(d, "margin_currency", cs.margin_currency, 12) &&
		addInt(d, "freeze_level", cs.freeze_level) &&
		addInt(d, "margin_hedged_strong", cs.margin_hedged_strong) &&
		addInt(d, "swap_openprice", cs.swap_openprice) &&
		addInt(d, "swap_variation_margin", cs.swap_variation_margin) &&
		addConSession(d, "sessions", cs.sessions, 7)
		)
		return true;
	else
		return false;
}

bool Utils::unSerializeTradeTransInfo(const std::string& body, TradeTransInfo& tti)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;

	if (addInt(d, "type", (int&)tti.type) &&
		addInt(d, "flags", (int&)tti.flags) &&
		addInt(d, "cmd", (int&)tti.cmd) &&
		addInt(d, "order", tti.order) &&
		addInt(d, "orderby", tti.orderby) &&
		addString(d, "symbol", tti.symbol, 12) &&
		addInt(d, "volume", tti.volume) &&
		addDouble(d, "price", tti.price) &&
		addDouble(d, "sl", tti.sl) &&
		addDouble(d, "tp", tti.tp) &&
		addInt(d, "ie_deviation", tti.ie_deviation) &&
		addString(d, "comment", tti.comment, 32) &&
		addInt(d, "expiration", (int&)tti.expiration)/* &&
		addInt(d, "crc", tti.crc)*/)
		return true;
	else
		return false;
}

bool Utils::unSerializeTradeRecord(const std::string& body, TradeRecord& tri)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;

	if (addInt(d, "order", tri.order) &&
		addInt(d, "login", tri.login) &&
		addString(d, "symbol", tri.symbol, 12) &&
		addInt(d, "digits", tri.digits) &&
		addInt(d, "cmd", tri.cmd) &&
		addInt(d, "volume", tri.volume) &&
		addInt(d, "gw_volume", tri.gw_volume) &&
		addInt(d, "gw_order", tri.gw_order) &&
		addInt(d, "gw_open_price", (int&)tri.gw_open_price) &&
		addInt(d, "gw_close_price", (int&)tri.gw_close_price) &&
		addInt(d, "open_time", (int&)tri.open_time) &&
		addInt(d, "close_time", (int&)tri.close_time) &&
		addInt(d, "state", tri.state) &&
		addDouble(d, "open_price", tri.open_price) &&
		addDouble(d, "sl", tri.sl) &&
		addDouble(d, "tp", tri.tp) &&
		addInt(d, "expiration", (int&)tri.expiration) &&
		addInt(d, "reason", (int&)tri.reason) &&
		addDoubleArray(d, "conv_rates", tri.conv_rates) &&
		addDouble(d, "commission", tri.commission) &&
		addDouble(d, "commission_agent", tri.commission_agent) &&
		addDouble(d, "storage", tri.storage) &&
		addDouble(d, "close_price", tri.close_price) &&
		addDouble(d, "profit", tri.profit) &&
		addDouble(d, "taxes", tri.taxes) &&
		addInt(d, "magic", tri.magic) &&
		addString(d, "comment", tri.comment, 32) &&
		addInt(d, "activation", tri.activation) &&
		addDouble(d, "margin_rate", tri.margin_rate) &&
		addInt(d, "timestamp", (int&)tri.timestamp))
		return true;
	else
		return false;
}

bool Utils::unSerializeChartInfo(const std::string& body, ChartInfo& ci)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;

	if (addInt(d, "period", ci.period) &&
		addInt(d, "start", (int&)ci.start) &&
		addInt(d, "end", (int&)ci.end) &&
		addInt(d, "timesign", (int&)ci.timesign) &&
		addInt(d, "mode", ci.mode) &&
		addString(d, "symbol", ci.symbol, 12))
		return true;
	else
		return false;
}

std::string Utils::serializeRateInfo(RateInfo* ri, int size)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);

	w.StartArray();
	for (int i = 0; i < size; i++)
	{
		w.StartObject();
		w.Key("ctm");
		w.Int(ri[i].ctm);

		w.Key("open");
		w.Int(ri[i].open);

		w.Key("high");
		w.Int(ri[i].high);

		w.Key("low");
		w.Int(ri[i].low);

		w.Key("close");
		w.Int(ri[i].close);

		w.Key("volume");
		w.Double(ri[i].vol);
		w.EndObject();
	}
	w.EndArray();

	return sb.GetString();
}

bool Utils::unSerializeChartupdate(const std::string& body, std::string& symbol, int& period, int& size, RateInfo*& rates)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(body.c_str()).HasParseError())
		return false;

	if (!addInt(d, "period", period) ||
		!addInt(d, "count", size) ||
		!addString(d, "symbol", symbol))
		return false;
	else
	{
		if (!d.HasMember("rate") || !d["rate"].IsArray())
			return false;
		if (addRateInfoArray(d["rate"], "rate", rates, size))
			return true;
		else
			return false;
	}
}

bool Utils::serializeTradeRecord(std::vector<TradeRecord> record, std::string& out)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	int size = record.size();
	w.StartObject();
	for (int i = 0; i < size; i++)
	{
		w.Key("order");
		w.Int(record[i].order);
		w.Key("login");
		w.Int(record[i].login);
		w.Key("symbol");
		w.String(record[i].symbol);
		w.Key("digits");
		w.Int(record[i].digits);
		w.Key("cmd");
		w.Int(record[i].cmd);
		w.Key("volume");
		w.Int(record[i].volume);
		w.Key("open_time");
		w.Int(record[i].open_time);
		w.Key("close_time");
		w.Int(record[i].close_time);
		w.Key("open_price");
		w.Double(record[i].open_price);
		w.Key("close_price");
		w.Double(record[i].close_price);
		w.Key("sl");
		w.Double(record[i].sl);
		w.Key("tp");
		w.Double(record[i].tp);
		w.Key("expiration");
		w.Int(record[i].expiration);
		w.Key("reason");
		w.Int(record[i].reason);
		w.Key("conv_rates");
		w.StartArray();
		w.Double(record[i].conv_rates[0]);
		w.Double(record[i].conv_rates[1]);
		w.EndArray();
		w.Key("commission");
		w.Double(record[i].commission);
		w.Key("commission_agent");
		w.Double(record[i].commission_agent);
		w.Key("storage");
		w.Double(record[i].storage);
		w.Key("profit");
		w.Double(record[i].profit);
		w.Key("taxes");
		w.Double(record[i].taxes);
		w.Key("magic");
		w.Int(record[i].magic);
		w.Key("activation");
		w.Int(record[i].activation);
		w.Key("gw_order");
		w.Int(record[i].gw_order);
		w.Key("gw_volume");
		w.Int(record[i].gw_volume);
		w.Key("gw_open_price");
		w.Int(record[i].gw_open_price);
		w.Key("gw_close_price");
		w.Int(record[i].gw_close_price);
		w.Key("timestamp");
		w.Int(record[i].timestamp);
		w.Key("comment");
		w.String(record[i].comment);
		w.Key("margin_rate");
		w.Double(record[i].margin_rate);
		w.Key("state");
		w.Int(record[i].state);
	}
	w.EndObject();;

	out = sb.GetString();
	return true;
}

std::string Utils::serializePumpSymbols(const std::vector<std::string> symbols)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	w.Key("type");
	w.Int(0);
	w.Key("comment");
	w.String("symbols");
	w.Key("content");
	w.StartArray();
	for (auto& s : symbols)
	{
		w.String(s.c_str());
	}
	w.EndArray();
	w.EndObject();
	return sb.GetString();
}

std::string Utils::serializePumpUsers(const std::vector<std::string> users)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	w.Key("type");
	w.Int(1);
	w.Key("comment");
	w.String("users");
	w.Key("content");
	w.StartArray();
	for (auto& u : users)
	{
		w.String(u.c_str());
	}
	w.EndArray();
	w.EndObject();
	return sb.GetString();
}

std::string Utils::serializePumpGroups(const std::vector<std::string> groups)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	w.Key("type");
	w.Int(2);
	w.Key("comment");
	w.String("groups");
	w.Key("content");
	w.StartArray();
	for (auto& g : groups)
	{
		w.String(g.c_str());
	}
	w.EndArray();
	w.EndObject();
	return sb.GetString();
}

std::string Utils::serializeConGroup(const ConGroup& group)
{
	//w.RawValue(response.c_str(), response.length(), rapidjson::Type::kObjectType);
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	charArrToJson(w, "group", (char*)group.group);
	intToJson(w, "enable", group.enable);
	intToJson(w, "timeout", group.timeout);
	intToJson(w, "otp_mode", group.otp_mode);

	charArrToJson(w, "company", (char*)group.company);
	charArrToJson(w, "signature", (char*)group.signature);
	charArrToJson(w, "support_page", (char*)group.support_page);
	charArrToJson(w, "smtp_server", (char*)(group.smtp_server));
	charArrToJson(w, "smtp_login", (char*)(group.smtp_login));
	charArrToJson(w, "smtp_password", (char*)(group.smtp_password));
	charArrToJson(w, "support_email", (char*)(group.support_email));
	charArrToJson(w, "templates", (char*)(group.templates));
	intToJson(w, "copies", group.copies);
	intToJson(w, "reports", group.reports);
	intToJson(w, "default_leverage", group.default_leverage);
	doubleToJson(w, "default_deposit", group.default_deposit);
	intToJson(w, "maxsecurities", group.maxsecurities);
	w.Key("ConGroupSec");
	w.StartArray();
	for (int i = 0; i < MAX_SEC_GROUPS; i++)
	{
		std::string sec = serializeConGroupSec(group.secgroups[i]);
		w.RawValue(sec.c_str(), sec.length(), rapidjson::Type::kObjectType);
	}
	w.EndArray();
	w.Key("ConGroupMargin");
	w.StartArray();
	for (int i = 0; i < MAX_SEC_GROPS_MARGIN; i++)
	{
		std::string margin = serializeConGroupMargin(group.secmargins[i]);
		w.RawValue(margin.c_str(), margin.length(), rapidjson::Type::kObjectType);
	}
	w.EndArray();
	intToJson(w, "secmargins_total", group.secmargins_total);
	charArrToJson(w, "currency", (char*)(group.currency));
	doubleToJson(w, "credit", group.credit);
	intToJson(w, "margin_call", group.margin_call);
	intToJson(w, "margin_mode", group.margin_mode);
	intToJson(w, "margin_stopout", group.margin_stopout);
	doubleToJson(w, "interestrate", group.interestrate);
	intToJson(w, "use_swap", group.use_swap);

	intToJson(w, "news", group.news);
	intToJson(w, "rights", group.rights);
	intToJson(w, "check_ie_prices", group.check_ie_prices);
	intToJson(w, "maxpositions", group.maxpositions);
	intToJson(w, "close_reopen", group.close_reopen);
	intToJson(w, "hedge_prohibited", group.hedge_prohibited);
	intToJson(w, "close_fifo", group.close_fifo);
	intToJson(w, "hedge_largeleg", group.hedge_largeleg);
	//charArrToJson(w, "securities_hash", (char*)group.securities_hash);
	intToJson(w, "margin_type", group.margin_type);
	intToJson(w, "archive_period", group.archive_period);
	intToJson(w, "archive_max_balance", group.archive_max_balance);
	intToJson(w, "stopout_skip_hedged", group.stopout_skip_hedged);
	intToJson(w, "archive_pending_period", group.archive_pending_period);

	w.EndObject();
	return sb.GetString();
}

std::string Utils::serializeConGroupMargin(const ConGroupMargin& margin)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	charArrToJson(w, "symbol", (char*)(margin.symbol));
	doubleToJson(w, "swap_long", margin.swap_long);
	doubleToJson(w, "swap_short", margin.swap_short);
	doubleToJson(w, "margin_divider", margin.margin_divider);
	w.EndObject();
	return sb.GetString();
}
std::string Utils::serializeConGroupSec(const  ConGroupSec& sec)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	intToJson(w, "show", sec.show);
	intToJson(w, "trade", sec.trade);
	intToJson(w, "execution", sec.execution);
	doubleToJson(w, "comm_base", sec.comm_base);
	intToJson(w, "comm_tye", sec.comm_type);
	intToJson(w, "comm_lots", sec.comm_lots);
	doubleToJson(w, "comm_agent", sec.comm_agent);
	intToJson(w, "comm_agent_type", sec.comm_agent_type);

	intToJson(w, "spread_diff", sec.spread_diff);
	intToJson(w, "lot_min", sec.lot_min);
	intToJson(w, "lot_max", sec.lot_max);
	intToJson(w, "lot_step", sec.lot_step);

	intToJson(w, "ie_deviation", sec.ie_deviation);
	intToJson(w, "confirmation", sec.confirmation);
	intToJson(w, "trade_rights", sec.trade_rights);
	intToJson(w, "ie_quick_mode", sec.ie_quick_mode);
	intToJson(w, "autocloseout_mode", sec.autocloseout_mode);
	doubleToJson(w, "comm_tax", sec.comm_tax);
	intToJson(w, "comm_agent_lots", sec.comm_agent_lots);
	w.EndObject();
	return sb.GetString();
}

std::string Utils::Md5(std::string data)
{
	MD5_CTX c;
	unsigned char md5_hex[MD5_DIGEST_LENGTH] = { 0 };
	char md5_str[MD5_DIGEST_LENGTH * 2 + 1] = {0};
	MD5_Init(&c);
	MD5_Update(&c, data.c_str(), data.size());
	MD5_Final(md5_hex, &c);

	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		sprintf(md5_str + i * 2, "%02X", md5_hex[i]);
	}
	md5_str[MD5_DIGEST_LENGTH * 2] = '\0';

	return md5_str;
}