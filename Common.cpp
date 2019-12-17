#include "Common.h"
#include "Logger.h"
#include <chrono>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "openssl/md5.h"
#include <sstream>
#include <iomanip>

Common* Common::m_self = nullptr;
std::mutex Common::m_mtxSingleton;

Common* Common::getInstance()
{
	if (nullptr == m_self)
	{
		std::lock_guard<std::mutex> lgd(m_mtxSingleton);
		if (nullptr == m_self)
		{
			m_self = new Common;
		}
	}
	return m_self;
}

Common::Common()
{
	m_reason[TR_REASON_CLIENT] = "TR_REASON_CLIENT";
	m_reason[TR_REASON_EXPERT] = "TR_REASON_EXPERT";
	m_reason[TR_REASON_DEALER] = "TR_REASON_DEALER";
	m_reason[TR_REASON_SIGNAL] = "TR_REASON_SIGNAL";
	m_reason[TR_REASON_GATEWAY] = "TR_REASON_GATEWAY";
	m_reason[TR_REASON_MOBILE] = "TR_REASON_MOBILE";
	m_reason[TR_REASON_WEB] = "TR_REASON_WEB";
	m_reason[TR_REASON_API] = "TR_REASON_API";

	m_type[TRANS_ADD] = "TRANS_ADD";
	m_type[TRANS_DELETE] = "TRANS_DELETE";
	m_type[TRANS_UPDATE] = "TRANS_UPDATE";
	m_type[TRANS_CHANGE_GRP] = "TRANS_CHANGE_GRP";

	m_cmd[OP_BUY] = "OP_BUY";
	m_cmd[OP_SELL] = "OP_SELL";
	m_cmd[OP_BUY_LIMIT] = "OP_BUY_LIMIT";
	m_cmd[OP_SELL_LIMIT] = "OP_SELL_LIMIT";
	m_cmd[OP_BUY_STOP] = "OP_BUY_STOP";
	m_cmd[OP_SELL_STOP] = "OP_SELL_STOP";
	m_cmd[OP_BALANCE] = "OP_BALANCE";
	m_cmd[OP_CREDIT] = "OP_CREDIT";

	m_state[TS_OPEN_NORMAL] = "TS_OPEN_NORMAL";
	m_state[TS_OPEN_REMAND] = "TS_OPEN_REMAND";
	m_state[TS_OPEN_RESTORED] = "TS_OPEN_RESTORED";
	m_state[TS_CLOSED_NORMAL] = "TS_CLOSED_NORMAL";
	m_state[TS_CLOSED_PART] = "TS_CLOSED_PART";
	m_state[TS_CLOSED_BY] = "TS_CLOSED_BY";
	m_state[TS_DELETED] = "TS_DELETED";

	m_tradeType[TT_PRICES_GET] = "TT_PRICES_GET";
	m_tradeType[TT_PRICES_REQUOTE] = "TT_PRICES_REQUOTE";
	m_tradeType[TT_ORDER_IE_OPEN] = "TT_ORDER_IE_OPEN";
	m_tradeType[TT_ORDER_REQ_OPEN] = "TT_ORDER_REQ_OPEN";
	m_tradeType[TT_ORDER_MK_OPEN] = "TT_ORDER_MK_OPEN";
	m_tradeType[TT_ORDER_PENDING_OPEN] = "TT_ORDER_PENDING_OPEN";
	m_tradeType[TT_ORDER_IE_CLOSE] = "TT_ORDER_IE_CLOSE";
	m_tradeType[TT_ORDER_REQ_CLOSE] = "TT_ORDER_REQ_CLOSE";
	m_tradeType[TT_ORDER_MK_CLOSE] = "TT_ORDER_MK_CLOSE";

	m_tradeType[TT_ORDER_MODIFY] = "TT_ORDER_MODIFY";
	m_tradeType[TT_ORDER_DELETE] = "TT_ORDER_DELETE";
	m_tradeType[TT_ORDER_CLOSE_BY] = "TT_ORDER_CLOSE_BY";
	m_tradeType[TT_ORDER_CLOSE_ALL] = "TT_ORDER_CLOSE_ALL";

	m_tradeType[TT_BR_ORDER_OPEN] = "TT_BR_ORDER_OPEN";
	m_tradeType[TT_BR_ORDER_CLOSE] = "TT_BR_ORDER_CLOSE";
	m_tradeType[TT_BR_ORDER_DELETE] = "TT_BR_ORDER_DELETE";
	m_tradeType[TT_BR_ORDER_CLOSE_BY] = "TT_BR_ORDER_CLOSE_BY";
	m_tradeType[TT_BR_ORDER_CLOSE_ALL] = "TT_BR_ORDER_CLOSE_ALL";
	m_tradeType[TT_BR_ORDER_MODIFY] = "TT_BR_ORDER_MODIFY";
	m_tradeType[TT_BR_ORDER_ACTIVATE] = "TT_BR_ORDER_ACTIVATE";
	m_tradeType[TT_BR_ORDER_COMMENT] = "TT_BR_ORDER_COMMENT";
	m_tradeType[TT_BR_BALANCE] = "TT_BR_BALANCE";

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

Common::~Common()
{

}

std::string Common::getReason(int reason)
{
	return m_reason[reason];
}
std::string Common::getType(int type)
{
	return m_type[type];
}
std::string Common::getCmd(int cmd)
{
	return m_cmd[cmd];
}
std::string Common::getState(int state)
{
	return m_state[state];
}
std::string Common::getTradeType(int tradeType)
{
	return m_tradeType[tradeType];
}
std::string Common::getActivation(int activation)
{
	return m_activation[activation];
}

int64_t Common::getCurMilliSeconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
}
int64_t Common::getCurSeconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock().now().time_since_epoch()).count();
}

void Common::PrintOrderRequest(const RequestInfo& req, std::string des)
{
	Logger::getInstance()->info("{} RequestInfo--id:{}, status:{}, time:{}, manager:{}, login:{}, group:{}, balance:{}, bid:{}, ask:{},\
trade--type:{}, flags:{}, cmd:{}, order:{}, orderby:{}, symbol:{}, volume:{}, price:{}, sl:{}, tp:{}, comment:{}, expiration:{}",
des, req.id, (int)req.status, req.time, req.manager, req.login, req.group, req.balance, req.prices[0], req.prices[1],
m_tradeType[req.trade.type], (int)req.trade.flags, m_cmd[req.trade.cmd], req.trade.order, req.trade.orderby, req.trade.symbol, req.trade.volume,
req.trade.price, req.trade.sl, req.trade.tp, req.trade.comment, req.trade.expiration);
}
void Common::PrintTradeRecord(const TradeRecord& record)
{
	Logger::getInstance()->info("TradeRecord--order:{},login:{}, symbol:{},digits:{}, cmd:{},volume:{},open_time:{},state:{},open_price:{}, \
		sl:{}, tp:{}, close_time:{}, expiration:{}, reason:{}, commission:{}, storage:{}, close_price:{},\
profit:{}, comment:{}, activation:{}, margin_rate:{}, timestamp:{}", record.order, record.login, record.symbol,
record.digits, m_cmd[record.cmd], record.volume, record.open_time, m_state[record.state], record.open_price, record.sl, record.tp,
record.close_time, record.expiration, m_reason[record.reason], record.commission, record.storage, record.close_price,
record.profit, record.comment, m_activation[record.activation], record.margin_rate, record.timestamp);
}
void Common::PrintTradeTransInfo(const TradeTransInfo& info, std::string des)
{
	Logger::getInstance()->info("{} TradeTransInfo--type:{}, flags:{}, cmd:{}, order:{}, orderby:{}, symbol:{}, volumne:{}, price:{}, sl:{}, tp:{}, ie_deviation:{}, comment:{}, expiration:{}", des,
		m_tradeType[info.type], (int)info.flags, m_cmd[info.cmd], info.order, info.orderby, info.symbol, info.volume, info.price, info.sl, info.tp, info.ie_deviation, info.comment, info.expiration);
}

std::string Common::genFilterKey(const TradeRecord& record, int triggerType)
{
	std::string key;
	if (0 == triggerType)
		key = std::to_string(record.order) + ":" + std::to_string(record.cmd)+ std::to_string(record.activation) +  std::to_string(record.state);
	else if(1 == triggerType || 2 == triggerType || 4 == triggerType)
		key = std::to_string(record.order) + ":" + std::to_string(record.cmd) + std::to_string(triggerType) + std::to_string(TS_CLOSED_NORMAL);

	return std::move(key);
}

void Common::deserializePluginTRFromJson(std::string json, TradeRecord& record, int& type)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
	{
		Logger::getInstance()->error("json parse error ,{}", json);
		return;
	}

	auto addInt = [&](Document &obj, char* key, int& value)
	{
		if (obj.HasMember(key) && obj[key].IsInt())
		{
			value = obj[key].GetInt();
		}
	};

	auto addDouble = [&](Document &obj, char* key, double& value)
	{
		if (obj.HasMember(key) && obj[key].IsDouble())
		{
			value = obj[key].GetDouble();
		}
	};

	auto addString = [&](Document &obj, char* key, char* value)
	{
		if (obj.HasMember(key) && obj[key].IsString())
		{
			memcpy(value, obj[key].GetString(), obj[key].GetStringLength());
		}
	};

	auto addInt32 = [&](Document &obj, char* key, __time32_t& value)
	{
		if (obj.HasMember(key) && obj[key].IsInt())
		{
			value = obj[key].GetInt();
		}
	};

	addInt(d, "order", record.order);
	addInt(d, "login", record.login);
	addString(d, "symbol", record.symbol);
	addInt(d, "digits", record.digits);
	addInt(d, "cmd", record.cmd);
	addInt(d, "volume", record.volume);
	addInt32(d, "open_time", record.open_time);
	addInt(d, "state", record.state);
	addDouble(d, "open_price", record.open_price);
	addDouble(d, "sl", record.sl);
	addDouble(d, "tp", record.tp);
	addInt32(d, "close_time", record.close_time);
	addInt32(d, "expiration", record.expiration);
	addDouble(d, "close_price", record.close_price);
	addString(d, "comment", record.comment);
	addInt(d, "activation", record.activation);
	addInt32(d, "timestamp", record.timestamp);
	addInt(d, "trigger", type);
}

std::string Common::serializeMT4TRToJson(int code, int type, const TradeRecord& record)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	w.Key("code");
	w.Int(code);
	w.Key("type");
	w.Int(type);

	w.Key("order");
	w.Int(record.order);

	w.Key("login");
	w.Int(record.login);

	w.Key("symbol");
	w.String(record.symbol);

	w.Key("digits");
	w.Int(record.digits);

	w.Key("cmd");
	w.Int(record.cmd);

	w.Key("volume");
	w.Int(record.volume);

	w.Key("open_time");
	w.Int(record.open_time);
	w.Key("open_price");
	w.Double(record.open_price);

	w.Key("close_time");
	w.Int(record.close_time);
	w.Key("close_price");
	w.Double(record.close_price);

	w.Key("state");
	w.Int(record.state);

	w.Key("sl");
	w.Double(record.sl);
	w.Key("tp");
	w.Double(record.tp);

	w.Key("expiration");
	w.Int(record.expiration);
	w.Key("reason");
	w.Int(record.reason);
	w.Key("timestamp");
	w.Int(record.timestamp);

	w.Key("profit");
	w.Double(record.profit);
	w.Key("storage");//swap
	w.Double(record.storage);
	w.Key("commission");
	w.Double(record.commission);
	w.Key("taxes");
	w.Double(record.taxes);

	w.Key("activation");
	w.Int(record.activation);

	w.Key("margin_rate");
	w.Double(record.margin_rate);
	w.Key("comment");
	w.String(record.comment);
	w.EndObject();

	return sb.GetString();
}

void Common::deserializeMT4TRFromJson(std::string json, TradeRecord& record, int& code, int& type)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
	{
		Logger::getInstance()->error("json parse error ,{}", json);
		return;
	}

	auto addInt = [&](Document &obj, char* key, int& value)
	{
		if (obj.HasMember(key) && obj[key].IsInt())
		{
			value = obj[key].GetInt();
		}
	};

	auto addDouble = [&](Document &obj, char* key, double& value)
	{
		if (obj.HasMember(key) && obj[key].IsDouble())
		{
			value = obj[key].GetDouble();
		}
	};

	auto addString = [&](Document &obj, char* key, char* value)
	{
		if (obj.HasMember(key) && obj[key].IsString())
		{
			memcpy(value, obj[key].GetString(), obj[key].GetStringLength());
		}
	};

	auto addInt32 = [&](Document &obj, char* key, __time32_t& value)
	{
		if (obj.HasMember(key) && obj[key].IsInt())
		{
			value = obj[key].GetInt();
		}
	};

    auto addInt8 = [&](Document &obj, char* key, char& value)
	{
		if (obj.HasMember(key) && obj[key].IsInt())
		{
			value = obj[key].GetInt();
		}
	};

	addInt(d, "order", record.order);
	addInt(d, "login", record.login);
	addString(d, "symbol", record.symbol);
	addInt(d, "digits", record.digits);
	addInt(d, "cmd", record.cmd);
	addInt(d, "volume", record.volume);
	addInt32(d, "open_time", record.open_time);
	addInt(d, "state", record.state);
	addDouble(d, "open_price", record.open_price);
	addDouble(d, "sl", record.sl);
	addDouble(d, "tp", record.tp);
	addInt32(d, "close_time", record.close_time);
	addInt32(d, "expiration", record.expiration);
	addDouble(d, "close_price", record.close_price);
	addString(d, "comment", record.comment);
	addInt(d, "activation", record.activation);
	addInt32(d, "timestamp", record.timestamp);
	addDouble(d, "margin_rate", record.margin_rate);
	addInt8(d, "reason", record.reason);
	addDouble(d, "profit", record.profit);
	addDouble(d, "storage", record.storage);
	addDouble(d, "commission", record.commission);
	addDouble(d, "taxes", record.taxes);
	addInt(d, "type", type);
	addInt(d, "code", code);
}

std::string Common::serializeReqToJson(RequestInfo& qi, bool backup)
{
	using namespace rapidjson;
	StringBuffer sb;
	Writer<StringBuffer> w(sb);
	w.StartObject();
	w.Key("backup");
	w.Bool(backup);
	w.Key("id");
	w.Int(qi.id);
	w.Key("status");
	w.Int(qi.status);
	w.Key("time");
	w.Int(qi.time);
	w.Key("manager");
	w.Int(qi.manager);
	w.Key("login");
	w.Int(qi.login);
	w.Key("group");
	w.String(qi.group);
	w.Key("balance");
	w.Double(qi.balance);
	w.Key("credit");
	w.Double(qi.credit);
	w.Key("bid");
	w.Double(qi.prices[0]);
	w.Key("ask");
	w.Double(qi.prices[1]);
	w.Key("trade");
	w.StartObject();
	w.Key("type");
	w.Int(qi.trade.type);
	w.Key("flags");
	w.Int(qi.trade.flags);
	w.Key("cmd");
	w.Int(qi.trade.cmd);
	w.Key("order");
	w.Int(qi.trade.order);
	w.Key("orderby");
	w.Int(qi.trade.orderby);
	w.Key("symbol");
	w.String(qi.trade.symbol);
	w.Key("volume");
	w.Int(qi.trade.volume);
	w.Key("price");
	w.Double(qi.trade.price);
	w.Key("sl");
	w.Double(qi.trade.sl);
	w.Key("tp");
	w.Double(qi.trade.tp);
	w.Key("comment");
	w.String(qi.trade.comment);
	w.Key("expiration");
	w.Int(qi.trade.expiration);
	w.EndObject();
	w.EndObject();
	return sb.GetString();
}

void Common::deserializeReqFromJson(std::string json, RequestInfo& qi, bool& backup)
{
	using namespace rapidjson;
	Document d;
	if (d.Parse(json.c_str()).HasParseError())
	{
		Logger::getInstance()->error("json parse error ,{}", json);
		return;
	}
	if (d.HasMember("backup") && d["backup"].IsBool())
	{
		backup = d["backup"].GetBool();
	}
	if (d.HasMember("id") && d["id"].IsInt())
	{
		qi.id = d["id"].GetInt();
	}
	if (d.HasMember("statue") && d["status"].IsInt())
	{
		qi.status = d["status"].GetInt();
	}
	if (d.HasMember("time") && d["time"].IsInt())
	{
		qi.time = d["time"].GetInt();
	}
	if (d.HasMember("manager") && d["manager"].IsInt())
	{
		qi.manager = d["manager"].GetInt();
	}
	if (d.HasMember("login") && d["login"].IsInt())
	{
		qi.login = d["login"].GetInt();
	}
	if (d.HasMember("group") && d["group"].IsString())
	{
		memcpy(qi.group, d["group"].GetString(), d["group"].GetStringLength());
	}
	if (d.HasMember("balance") && d["balance"].IsDouble())
	{
		qi.balance = d["balance"].GetDouble();
	}
	if (d.HasMember("credit") && d["credit"].IsDouble())
	{
		qi.credit = d["credit"].GetDouble();
	}
	if (d.HasMember("bid") && d["bid"].IsDouble())
	{
		qi.prices[0] = d["bid"].GetDouble();
	}
	if (d.HasMember("ask") && d["ask"].IsDouble())
	{
		qi.prices[1] = d["ask"].GetDouble();
	}
	if (d.HasMember("trade") && d["trade"].IsObject())
	{
		Value o = d["trade"].GetObjectA();
		if (o.HasMember("type") && o["type"].IsInt())
		{
			qi.trade.type = o["type"].GetInt();
		}
		if (o.HasMember("flags") && o["flags"].IsInt())
		{
			qi.trade.flags = o["flags"].GetInt();
		}
		if (o.HasMember("cmd") && o["cmd"].IsInt())
		{
			qi.trade.cmd = o["cmd"].GetInt();
		}
		if (o.HasMember("order") && o["order"].IsInt())
		{
			qi.trade.order = o["order"].GetInt();
		}
		if (o.HasMember("orderby") && o["orderby"].IsInt())
		{
			qi.trade.orderby = o["orderby"].GetInt();
		}
		if (o.HasMember("symbol") && o["symbol"].IsString())
		{
			memcpy(qi.trade.symbol, o["symbol"].GetString(), o["symbol"].GetStringLength());
		}
		if (o.HasMember("volume") && o["volume"].IsInt())
		{
			qi.trade.volume = o["volume"].GetInt();
		}
		if (o.HasMember("price") && o["price"].IsInt())
		{
			qi.trade.price = o["price"].GetDouble();
		}
		if (o.HasMember("sl") && o["sl"].IsDouble())
		{
			qi.trade.sl = o["sl"].GetDouble();
		}
		if (o.HasMember("tp") && o["tp"].IsDouble())
		{
			qi.trade.tp = o["tp"].GetDouble();
		}
		if (o.HasMember("comment") && o["comment"].IsString())
		{
			memcpy(qi.trade.comment, o["comment"].GetString(), o["comment"].GetStringLength());
		}
		if (o.HasMember("expiration") && o["expiration"].IsInt())
		{
			qi.trade.expiration = o["expiration"].GetInt();
		}
	}
}

std::string Common::MD5(std::string data)
{
	MD5_CTX c;
	unsigned char md5[17] = { 0 };
	MD5_Init(&c);
	MD5_Update(&c, data.c_str(), data.size());
	MD5_Final(md5, &c);

	char buf[33] = {0};
	char tmp[3] = {0};
	for (int i = 0; i < 16; i++)
	{
		sprintf_s(tmp,3 , "%02X", md5[i]);
		strcat_s(buf,strlen(tmp)+1, tmp);
	}
	return buf;
}

void  Common::split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter)
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


std::string Common::getProjectPath()
{
	std::string path;
	char p[MAX_PATH] = { 0 };
	if (GetModuleFileName(0, p, MAX_PATH))
	{
		path = p;
		path = path.substr(0, path.rfind("\\"));
		path += "\\";
	}
	return path;
}