#include "HttpServer.h"
#include "Logger.h"
#include "Config.h"
#include "Utils.h"
#include <map>
#include <unordered_map>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "SqliteClient.h"

HttpServer::HttpServer()
{
	m_mt4Conn = nullptr;

	m_httpMethod = { { EVHTTP_REQ_GET, "GET" },
	{ EVHTTP_REQ_POST , "POST" },
	{ EVHTTP_REQ_PUT , "PUT" },
	{ EVHTTP_REQ_DELETE , "DELETE" },
	{ EVHTTP_REQ_PATCH , "PATCH" },
	{ EVHTTP_REQ_OPTIONS , "OPTIONS" },
	{ EVHTTP_REQ_HEAD , "HEAD" },
	{ EVHTTP_REQ_TRACE , "TRACE" },
	{ EVHTTP_REQ_CONNECT , "CONNECT" } };


	std::string common = Config::getInstance().getHTTPConf().find("account-group-common")->second;
	std::string permissions = Config::getInstance().getHTTPConf().find("account-group-permissions")->second;
	std::string archiving = Config::getInstance().getHTTPConf().find("account-group-archiving")->second;
	std::string margins = Config::getInstance().getHTTPConf().find("account-group-margins")->second;
	std::string securities = Config::getInstance().getHTTPConf().find("account-group-securities")->second;
	std::string symbols = Config::getInstance().getHTTPConf().find("account-group-symbols")->second;
	std::string reports = Config::getInstance().getHTTPConf().find("account-group-reports")->second;
	std::string common_groups = Config::getInstance().getHTTPConf().find("common-account-groups")->second;
	std::string securities_auto = Config::getInstance().getHTTPConf().find("account-group-securities-auto")->second; 
	std::string securities_auto_get = securities_auto + "-get";
	std::string securities_auto_set = securities_auto + "-set";
	std::string accounts = Config::getInstance().getHTTPConf().find("account-configuration")->second;
	std::string global_datafeed = Config::getInstance().getHTTPConf().find("datafeed-configuration")->second;
	std::string global_common =  Config::getInstance().getHTTPConf().find("common-configuration")->second;
	std::string global_iplist =  Config::getInstance().getHTTPConf().find("ip-access-configuration")->second;
	std::string global_symbol_conf = Config::getInstance().getHTTPConf().find("symbol-configuration")->second;
	std::string global_dc = Config::getInstance().getHTTPConf().find("data-center-configuration")->second;
	std::string global_plugin =  Config::getInstance().getHTTPConf().find("plugin-configuration")->second;
	std::string global_performance =  Config::getInstance().getHTTPConf().find("performance-configuration")->second;
	std::string set_sessions = Config::getInstance().getHTTPConf().find("set-symbol-session")->second;
	std::string set_swap = Config::getInstance().getHTTPConf().find("set-swap")->second;
	std::string modify_open_price = Config::getInstance().getHTTPConf().find("modify-open_price")->second;
	std::string symbol_trade_mode = Config::getInstance().getHTTPConf().find("modify-symbol-trade-mode")->second;

	std::string con_symbol = Config::getInstance().getHTTPConf().find("consymbol")->second;
	std::string update_consymbol = Config::getInstance().getHTTPConf().find("update-consymbol")->second;
	std::string trade = Config::getInstance().getHTTPConf().find("trade")->second;
	std::string get_trade = Config::getInstance().getHTTPConf().find("get-trade")->second;
	std::string update_trade = Config::getInstance().getHTTPConf().find("update-trade")->second;
	std::string chart_req = Config::getInstance().getHTTPConf().find("chart-req")->second;
	std::string chart_update = Config::getInstance().getHTTPConf().find("chart-update")->second;
	std::string global_symbols_list = Config::getInstance().getHTTPConf().find("symbols-list")->second;       //symbols-list
	std::string get_holiday = Config::getInstance().getHTTPConf().find("get-holiday")->second;
	std::string common_securities = Config::getInstance().getHTTPConf().find("common-symbol-groups")->second; //securities-list

	std::string get_userrecord = Config::getInstance().getHTTPConf().find("get-user-record")->second;
	std::string update_userrecord = Config::getInstance().getHTTPConf().find("update-user-record")->second;
	m_uri = { {common, COMMON},
	{permissions, PERMISSIONS},
	{archiving, ARCHIVING},
	{margins, MARGINS},
	{securities, SECURITIES},
	{symbols, SYMBOLS},
	{reports, REPORTS},
	{common_groups, COMMON_GROUPS},
	{common_securities, COMMON_SECURITIES},
	{securities_auto_get, SECURITIES_AUTO_GET},
	{securities_auto_set, SECURITIES_AUTO_SET},
	{accounts, ACCOUNT_CONFIGUTATION},
	{global_datafeed, GLOBAL_DATAFEED},
	{global_common, GLOBAL_COMMON},
	{global_iplist, GLOBAL_IP},
	{global_symbols_list, GLOBAL_SYMBOLS_LIST},
	{global_symbol_conf, GLOBAL_SYMBOL},
	{global_dc, GLOBAL_DC},
	{global_plugin, GLOBAL_PLUGIN},
	{global_performance, GLOBAL_PERFORMANCE},
	{get_holiday, GET_HOLIDAY},
	{set_sessions, SET_SESSIONS} ,
	{set_swap, SET_SWAP},
	{modify_open_price, MODIFY_OPENPRICE},
	{symbol_trade_mode, SET_SYMBOL_TRADEMODE} ,
	{con_symbol, CON_SYMBOL},
	{update_consymbol, UPDATE_CONSYMBOL},
	{trade, TRADE},
	{get_trade, GET_TRADE},
	{update_trade, UPDATE_TRADE},
	{chart_update, CHART_UPDATE},
	{chart_req, CHART_REQ},
	{get_userrecord, GET_USERRECORD},
	{update_userrecord, UPDATE_USERRECORD} };
}

HttpServer::~HttpServer()
{

}

void *HttpServer::my_zeroing_malloc(size_t howmuch)
{
	return calloc(1, howmuch);
}

bool HttpServer::initServerHttp()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	evthread_use_windows_threads();

	m_evBase = event_base_new();
	if (m_evBase == nullptr)
	{
		Logger::getInstance()->error("Couldn't create an event_base: exiting");
		return false;
	}

	m_http = evhttp_new(m_evBase);
	if (nullptr == m_http)
	{
		Logger::getInstance()->error("Couldn't create evhttp: exiting");
		return false;
	}

	evhttp_set_gencb(m_http, HttpServer::cbFunc, this);

	std::string port = Config::getInstance().getHTTPConf().find("port")->second;
	struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(m_http, "0.0.0.0", std::stoi(port));
	if (nullptr == handle)
	{
		Logger::getInstance()->error("Couldn't bind to port {}. Exiting.", port);
		return false;
	}
	return true;
}

bool HttpServer::serverSetupCerts(SSL_CTX* ctx, const char* certificate_chain, const char* private_key)
{
	Logger::getInstance()->info("Loading certificate chain from {} and private key from {}", certificate_chain, private_key);
	if (1 != SSL_CTX_use_certificate_chain_file(ctx, certificate_chain))
	{
		Logger::getInstance()->error("SSL_CTX_use_certificate_chain_file failed.");
		return false;
	}
	if (1 != SSL_CTX_use_PrivateKey_file(ctx, private_key, SSL_FILETYPE_PEM))
	{
		Logger::getInstance()->error("SSL_CTX_use_PrivateKey_file failed.");
		return false;
	}
	if (1 != SSL_CTX_check_private_key(ctx))
	{
		Logger::getInstance()->error("SSL_CTX_check_private_key failed.");
		return false;
	}
	return true;
}

void HttpServer::loginCb(evhttp_request* req, void* arg)
{
	const char* uri = evhttp_request_get_uri(req);

	if (EVHTTP_REQ_GET == evhttp_request_get_command(req))
	{
		evbuffer* buf = evbuffer_new();
		if (nullptr == buf)
			return;
		evbuffer_add_printf(buf, "Requested: %s\n", uri);
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		return;
	}

	if (EVHTTP_REQ_POST == evhttp_request_get_command(req))
	{
		evhttp_send_reply(req, 200, "OK", NULL);
		Logger::getInstance()->info("Got a POST request for <{}>", uri);
		return;
	}

	evhttp_uri *decoded = evhttp_uri_parse(uri);
	if (nullptr == decoded)
	{
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	evbuffer* buf = evhttp_request_get_input_buffer(req);
	evbuffer_add(buf, "", 1);
	char* payload = (char*)evbuffer_pullup(buf, -1);
	int post_data_len = evbuffer_get_length(buf);
	Logger::getInstance()->info("[post_data:len{}]={}",post_data_len, payload);

	evhttp_add_header(evhttp_request_get_output_headers(req), "Server", "HttpSignature");
	evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain");
	evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");

	evbuffer* evb = evbuffer_new();
	evbuffer_add_printf(evb, "%s", "hello world");
	evhttp_send_reply(req, HTTP_OK, "OK", evb);

	if (decoded)
		evhttp_uri_free(decoded);
	if (evb)
		evbuffer_free(evb);
}

bufferevent* HttpServer::bevCb(event_base* base, void* arg)
{
	SSL_CTX* ctx = (SSL_CTX*)arg;
	bufferevent* r = bufferevent_openssl_socket_new(base, -1, SSL_new(ctx), BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
	return r;
}

void HttpServer::setMT4Conn(MT4Conn* conn)
{
	m_mt4Conn = conn;
}

void HttpServer::cbFunc(struct evhttp_request *req, void *args)
{
	HttpServer* pSelf = (HttpServer *)args;
	evbuffer* evb = nullptr;

	std::string strMessage;
	std::map<std::string, std::string> mapParams;
	//evhttp_cmd_type eRequestType;

	std::string uri;
	evhttp_cmd_type method;

	std::map<std::string, std::string> uriArgs;
	std::string body;


	int res = 0;
	int mt4res = 0;
	std::string response = "";
	if (pSelf->parseReq(req, method, uri, uriArgs, body))
	{
		res = pSelf->handleReq(method, uri, uriArgs, body, response, mt4res);
	}

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> w(sb);
	w.StartObject();

	w.Key("IsSucc");
	w.Int(res);

	w.Key("RetCode");
	w.Int(mt4res);

	w.Key("Result");
	if (response.empty())
	{
		/*w.StartObject();
		w.EndObject();*/
		w.String("");
	}
	else if(response.find("{") != std::string::npos 
		|| response.find("[") != std::string::npos)
	{
		w.RawValue(response.c_str(), response.length(), rapidjson::Type::kObjectType);
	}
	else
	{
		w.String(response.c_str());
	}
	

	w.EndObject();

	std::string resp = sb.GetString();

	//设置返回消息
	evb = evbuffer_new();
	auto oh = evhttp_request_get_output_headers(req);
	evhttp_add_header(oh, "Content-type", "application/json; charset=UTF-8");
	evhttp_add_header(oh, "Connection", "Close");
	evhttp_add_header(oh, "Content-length", std::to_string(resp.size()).c_str());

	evbuffer_add_printf(evb, "%s", resp.c_str());
	//Logger::getInstance()->info("response [{}]", resp);

	switch (res)
	{
	case STATUS::OK:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->info("response ok.");
	}
	break;
	case BAD_URL:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->error("response: bad url！");
	}
	break;
	case BAD_METHOD:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->error("response: method not allowed！");
	}
	break;
	case PARAM_INVALID:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->error("response: bad request！");
	}
	break;
	case SERVER_ERROR:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->error("response: internal server error！");
	}
	break;
	default:
	{
		evhttp_send_reply(req, 200, "OK", evb);
		Logger::getInstance()->error("response: internal server error!");
	}
	break;
	}

	if (evb)
	{
		evbuffer_free(evb);
	}
}

int  HttpServer::startServer()
{
	Logger::getInstance()->info("http server start...");
	int exitCode = event_base_dispatch(m_evBase);
	if (exitCode < 0)
	{
		Logger::getInstance()->error("event_base_dispath exit occure error.");
	}
	evhttp_free(m_http);
	event_base_free(m_evBase);

	m_http = nullptr;
	m_evBase = nullptr;
	return exitCode;
}

int HttpServer::stopServer()
{
	return event_base_loopbreak(m_evBase);
}

bool HttpServer::parseReq(struct evhttp_request* req, evhttp_cmd_type& method, std::string& uri, std::map<std::string, std::string>& uriArgs, std::string& body)
{
	evkeyvalq querykvs;
	struct evhttp_uri* pDecodedUrl = nullptr;
	char *pDecodedPath = nullptr;
	evbuffer *evbuf = nullptr;
	bool bRes = true;
	do
	{
		method = evhttp_request_get_command(req);

		const char *pUri = evhttp_request_get_uri(req);

		pDecodedUrl = evhttp_uri_parse(pUri);
		if (!pDecodedUrl)
		{
			evhttp_send_error(req, HTTP_BADREQUEST, 0);
			bRes = false;
			break;
		}
		const char *pPath = evhttp_uri_get_path(pDecodedUrl);
		if (nullptr == pPath)
		{
			pPath = "/";
		}

		uri = pPath;

		pDecodedPath = evhttp_uridecode(pPath, 0, nullptr);
		if (!pDecodedPath)
		{
			evhttp_send_error(req, 404, 0);
			bRes = false;
			break;
		}

		querykvs.tqh_first = nullptr;
		const char *pQueryStr = evhttp_uri_get_query(pDecodedUrl);
		if (pQueryStr)
		{
			evhttp_parse_query_str(pQueryStr, &querykvs);
		}
		for (evkeyval *it = querykvs.tqh_first; it != nullptr; it = it->next.tqe_next)
		{
			uriArgs[it->key] = it->value;
		}
		//clear memory
		evhttp_clear_headers(&querykvs);

		evbuf = evhttp_request_get_input_buffer(req);
		while (evbuffer_get_length(evbuf) > 0)
		{
			char cbuf[1024];
			memset(cbuf, 0, sizeof(cbuf));
			int n = evbuffer_remove(evbuf, cbuf, sizeof(cbuf) - 1);
			if (n > 0)
			{
				body.append(cbuf, n);
			}
		}
		Logger::getInstance()->info("request [method:{}, uri:{}, body:{}]", m_httpMethod[method], pUri, body);
	} while (0);

	if (pDecodedUrl)
	{
		evhttp_uri_free(pDecodedUrl);
	}
	if (pDecodedPath)
	{
		free(pDecodedPath);
	}
	return bRes;
}

int HttpServer::handleReq(const evhttp_cmd_type& method, const std::string& uri, const std::map<std::string, std::string>& uriArgs, const std::string& body, std::string& response, int& mt4res)
{
	int res = OK;
	if (method == EVHTTP_REQ_POST)
	{
		switch (m_uri[uri])
		{
		case COMMON:
			res = setGroupCommon(body, response, mt4res);
			break;
		case PERMISSIONS:
			res = setGroupPermission(body, response, mt4res);
			break;
		case ARCHIVING:
			res = setGroupArchive(body, response, mt4res);
			break;
		case MARGINS:
			res = setGroupMargins(body, response, mt4res);
			break;
		case SECURITIES:
			res = setGroupSecurities(body, response, mt4res);
			break;
		case SYMBOLS:
			res = setGroupSymbols(body, response, mt4res);
			break;
		case REPORTS:
			res = setGroupReport(body, response, mt4res);
			break;
		case ACCOUNT_CONFIGUTATION:
			res = setAccount(body, response, mt4res);
			break;
		case GLOBAL_SYMBOL:
			res = getGlobalSymbol(body, response, mt4res);
			break;
		case GLOBAL_PERFORMANCE:
			res = getGlobalPerformance(body, response, mt4res);
			break;
		case SET_SESSIONS:
			res = setSessions(body, response, mt4res);
			break;
		case SET_SWAP:
			res = setSwap(body, response, mt4res);
			break;
		case MODIFY_OPENPRICE:
			res = modifyOpenPrice(body, response, mt4res);
			break;
		case UPDATE_CONSYMBOL:
			res = updateConSymbol(body, response, mt4res);
			break;
		case TRADE:
			res = tradeTransaction(body, response, mt4res);
			break;
		case UPDATE_TRADE:
			res = updateTransaction(body, response, mt4res);
			break;
		case CHART_REQ:
			res = chartReq(body, response, mt4res);
			break;
		case CHART_UPDATE:
			res = chartUpdate(body, response, mt4res);
			break;
		case UPDATE_USERRECORD:
			res = updateUserRecord(body, response, mt4res);
			break;
		default:
			//response = R"( {"bad url or request method not right." } )";
			res = BAD_URL;
			break;
		}
	}
	else if (method == EVHTTP_REQ_GET)
	{
		switch (m_uri[uri])
		{
		case COMMON:
			res = getGroupCommon(uriArgs, response, mt4res);
			break;
		case PERMISSIONS:
			res = getGroupPermission(uriArgs, response, mt4res);
			break;
		case ARCHIVING:
			res = getGroupArchive(uriArgs, response, mt4res);
			break;
		case MARGINS:
			res = getGroupMargins(uriArgs, response, mt4res);
			break;
		case SECURITIES:
			res = getGroupSecurities(uriArgs, response, mt4res);
			break;
		case SYMBOLS:
			res = getGroupSymbols(uriArgs, response, mt4res);
			break;
		case REPORTS:
			res = getGroupReport(uriArgs, response, mt4res);
			break;
		case COMMON_GROUPS:
			res = getGroupsNames(response, mt4res);
			break;
		case COMMON_SECURITIES:
			res = getSecuritiesNames(response, mt4res);
			break;
		case SECURITIES_AUTO_GET:  //get
			res = getAllGroupsSecurities(response, mt4res);
			break;
		case SECURITIES_AUTO_SET:  //set
			res = setAllGroupsSecurities(response, mt4res);
			break;
		case GLOBAL_DATAFEED:
			res = getGlobalDatafeed(response, mt4res);
			break;
		case GLOBAL_COMMON:
			res = getGlobalCommon(response, mt4res);
			break;
		case GLOBAL_IP:
			res = getGlobalIPlist(response, mt4res);
			break;
		case GLOBAL_SYMBOLS_LIST:
			res = getGlobalSymbolsList(response, mt4res);
			break;
		case GLOBAL_DC:
			res = getGlobalDC(response, mt4res);
			break;
		case GLOBAL_PLUGIN:
			res = getGlobalPlugin(response, mt4res);
			break;
		case GET_HOLIDAY:
			res = getHoliday(response, mt4res);
			break;
		case SET_SYMBOL_TRADEMODE:
			res = setConSymbolTradeMode(uriArgs, response, mt4res);
			break;
		case CON_SYMBOL:
			res = getConSymbol(uriArgs, response, mt4res);
			break;
		case GET_TRADE:
			res = getTrades(uriArgs, response, mt4res);
			break;
		case GET_USERRECORD:
			res = getUserRecord(uriArgs, response, mt4res);
			break;
		default:
			//response = R"( { "bad url or request method not right." })";
			res = BAD_URL;
			break;
		}
	}
	else
	{
		//response = R"( { "not support the request method, only can ues get and post for now."})";
		res = BAD_METHOD;
	}

	return res;
}

int HttpServer::setGroupSecurities(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	std::map<int, ConGroupSec> sec;
	std::set<int> index;
	std::string group;
	std::string err;
	if (Utils::getInstance().parseFromJsonToSec(body, sec, index, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupSec(group, sec, index, err)))
		{
			Logger::getInstance()->info("update group securities success.");
			//response = R"( { "update group securities success."} )";
		}
		else if (-1 == mt4res)
		{
			res = SERVER_ERROR;
			//response = R"({ "server internal error." })";
		}
		else
		{
			//response = "{\" " + err + "\" }";
			res = 0;
		}
	}
	else
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupCommon(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	GroupCommon groupCommon;
	std::string group;
	std::string err;
	if (Utils::getInstance().parseFromJsonToCommon(body, groupCommon, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupCommon(group, groupCommon, err)))
		{
			Logger::getInstance()->info("update group common success.");
			//response = R"( { "update group common success."} )";
		}
		else if(-1 == mt4res)
		{
			res = SERVER_ERROR;
			//response = R"({ "server internal error." })";
		}
		else
		{
			//response = "{\" " + err + "\" }";
			res = 0;
		}
	}
	else
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupSymbols(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	std::map<std::string, ConGroupMargin> margins;
	std::string group;
	std::string err;
	if (Utils::getInstance().parseFromJsonToSymbol(body, margins, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupSymbol(group, margins, err)))
		{
			Logger::getInstance()->info("update group securities success.");
			//response = R"( { "update group securities success."} )";
		}
		else if (-1 == mt4res)
		{
			res = SERVER_ERROR;
			//response = R"({ "server internal error." })";
		}
		else
		{
			//response = "{ \"" + err + "\" }";
			res = 0;
		}
	}
	else
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::getGroupCommon(const std::map<std::string, std::string>& uriArgs,std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
		return res;
	}
	GroupCommon groupComm;//default constructor
	groupComm = m_mt4Conn->getGroupCommon(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromCommonToJson(uriArgs.at("Group"), groupComm, response))
	{
		Logger::getInstance()->info("serialize group common success.");
	}
	else
	{
		//response = R"( {"serialize group common failed." })";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupSecurities(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConGroup conGroup = { 0 };
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
		return res;
	}
	conGroup = m_mt4Conn->getGroupCfg(uriArgs.at("Group"));
	int size = sizeof(conGroup.secgroups) / sizeof(ConGroupSec);
	if (Utils::getInstance().parseFromSecToJson(conGroup.group, conGroup.secgroups, size, response))
	{
		Logger::getInstance()->info("serialize group securities success.");
	}
	else
	{
		//response = R"( {"serialize group securities failed." })";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupSymbols(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConGroup conGroup = { 0 };
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"( { "param invalid.please check and try again." })";
		res = PARAM_INVALID;
		return res;
	}
	conGroup = m_mt4Conn->getGroupCfg(uriArgs.at("Group"));
	int size = conGroup.secmargins_total;
	if (Utils::getInstance().parseFromSymbolToJson(conGroup.group, conGroup.secmargins, size, response))
	{
		Logger::getInstance()->info("serialize group margins success.");
	}
	else
	{
		//response = R"( { "serialize group margins failed." })";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupsNames(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	std::vector<std::string> common_groups;
	if (m_mt4Conn->getGroupNames(common_groups))
	{
		if (Utils::getInstance().parseFromCommonGroupsToJson(common_groups, response))
		{
			Logger::getInstance()->info("serialize groups success.");;
		}
		else
		{
			//response = R"({ "serialize groups failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"({"get groups failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getSecuritiesNames(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConSymbolGroup securities[MAX_SEC_GROUPS] = { 0 };
	if (0 == (mt4res = m_mt4Conn->getSecuritiesNames(securities, response)))
	{
		int size = sizeof(securities) / sizeof(ConSymbolGroup);
		if (Utils::getInstance().parseFromCommmonSecuritesToJson(securities, size, response))
		{
			Logger::getInstance()->info("serialize securities success.");
		}
		else
		{
			//response = R"({ "serialize securities failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = "{ \"" + response + "\" }"; 
		res = SERVER_ERROR;
		mt4res = 0;
	}
	return res;
}

int HttpServer::getGroupMargins(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupMargin margin;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}
	margin = m_mt4Conn->getGroupMargin(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromMarginToJson(uriArgs.at("Group"), margin,  response))
	{
		Logger::getInstance()->info("serialize group margins success.");
	}
	else
	{
		//response = R"({"serialize group margins failed."})";
		res = SERVER_ERROR;
	}
	return res;
}


int HttpServer::getGroupArchive(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupArchive archive;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}
	archive = m_mt4Conn->getGroupArchive(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromArchiveToJson(uriArgs.at("Group"), archive, response))
	{
		Logger::getInstance()->info("serialize group archive success.");
	}
	else
	{
		//response = R"({"serialize group archive failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupReport(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupReport report;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}
	report = m_mt4Conn->getGroupReport(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromReportToJson(uriArgs.at("Group"), report, response))
	{
		//Logger::getInstance()->info("serialize group report success.");
	}
	else
	{
		//response = R"({ "serialize group report failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupPermission(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupPermission permission;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}
	permission = m_mt4Conn->getGroupPermission(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromPermissionToJson(uriArgs.at("Group"), permission, response))
	{
		Logger::getInstance()->info("serialize group permission success.");
	}
	else
	{
		//response = R"({"serialize group permission failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setAccount(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	AccountConfiguration configuration;
	std::string login;
	if (Utils::getInstance().parseFromJsonToAccuntConfiguration(body, configuration, login))
	{
		if (0 == (mt4res = m_mt4Conn->updateAccounts(login, configuration, response)))
		{
			Logger::getInstance()->info("update account configuration success.");
			//response = R"({"update account configuration success."})";
		}
		else
		{
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupMargins(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupMargin margins;
	std::string group;
	if (Utils::getInstance().parseFromJsonToMargin(body, margins, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupMargin(group, margins, response)))
		{
			Logger::getInstance()->info("update group margin success.");
			//response = R"({"update group margin success."})";
		}
		else if(-1 == mt4res)
		{
			//response = R"({"update group margin failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"( {"param invalid.please check and try again."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupArchive(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupArchive archive;
	std::string group;
	if (Utils::getInstance().parseFromJsonToArchive(body, archive, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupArchive(group, archive, response)))
		{
			Logger::getInstance()->info("update group archive success.");
			//response = R"({"update group archive success."})";
		}
		else if(-1 == mt4res)
		{
			//response = R"({"update group archive failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupReport(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupReport report;
	std::string group;
	if (Utils::getInstance().parseFromJsonToReport(body, report, group))
	{
		if (0 == (mt4res = m_mt4Conn->upateGroupReport(group, report, response)))
		{
			Logger::getInstance()->info("update group report success.");
			//response = R"({"update group report success."})";
		}
		else if(-1 == mt4res)
		{
			//response = R"({"update group report failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupPermission(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	GroupPermission permission;
	std::string group;
	if (Utils::getInstance().parseFromJsonToPermission(body, permission, group))
	{
		if (0 == (mt4res = m_mt4Conn->updateGroupPerssion(group, permission, response)))
		{
			Logger::getInstance()->info("update group permission success.");
			//response = R"({"update group permission success."})";
		}
		else if(-1 == mt4res)
		{
			//response = R"({"update group permission failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::getAllGroupsSecurities(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	std::map<int, std::string> securitiesMap;
	ConSymbolGroup securities[MAX_SEC_GROUPS] = { 0 };
	if (0 == (mt4res = m_mt4Conn->getSecuritiesNames(securities, response)))
	{
		int size = sizeof(securities) / sizeof(ConSymbolGroup);
		for (int i = 0; i < size; i++)
		{
			securitiesMap[i] = securities[i].name;
		}
	}
	else
	{
		//response = "{" + response + "}";
		return res;
	}

	std::vector<std::string> common_groups;
	if (m_mt4Conn->getGroupNames(common_groups))
	{
		for (auto &group : common_groups)
		{
			ConGroup conGroup = { 0 };
			conGroup = m_mt4Conn->getGroupCfg(group);
			int size = sizeof(conGroup.secgroups) / sizeof(ConGroupSec);
			for (int i = 0; i < size; i++)
			{
				if (!SqliteClient::getInstance().add(i, securitiesMap[i], conGroup))
				{
					//response = R"({"operate failed."})";
					return SERVER_ERROR;
				}
			}
		}
		//response = R"({"operate success."})";
		return OK;
	}
	else
	{
		//response = R"({"operate failed."})";
		return SERVER_ERROR;
	}
}

int HttpServer::setAllGroupsSecurities(std::string& response, int& mt4res)
{
	//std::unordered_map<std::string, std::vector<std::string>> records;
	std::vector<std::vector<std::string>> records;
	if(SqliteClient::getInstance().getAllRecords(records))
	{
		for (auto &r : records)
		{
			ConGroupSec cs = {0};
			cs.show = std::stoi(r.at(3));
			cs.trade = std::stoi(r.at(4));
			cs.confirmation = std::stoi(r.at(5));
			cs.execution = std::stoi(r.at(6));
			cs.spread_diff = std::stoi(r.at(7));
			cs.freemargin_mode = std::stoi(r.at(8));
			cs.ie_deviation = std::stoi(r.at(9));
			cs.ie_quick_mode = std::stoi(r.at(10));
			cs.lot_min = std::stoi(r.at(11));
			cs.lot_max = std::stoi(r.at(12));
			cs.lot_step = std::stoi(r.at(13));
			cs.comm_base = std::stod(r.at(14));
			cs.comm_type = std::stoi(r.at(15));
			cs.comm_lots = std::stoi(r.at(16));
			cs.comm_agent = std::stoi(r.at(17));
			cs.comm_agent_type = std::stoi(r.at(18));
			cs.comm_tax = std::stod(r.at(19));
			cs.comm_agent_lots = std::stoi(r.at(20));
			cs.trade_rights = std::stoi(r.at(21));
			cs.autocloseout_mode = std::stoi(r.at(22));

			std::map<int, ConGroupSec> sec;
			sec[std::stoi(r.at(0))] = cs;
			std::set<int> sIndex;
			sIndex.insert(std::stoi(r.at(0)));

			std::string err;
			if (0 == m_mt4Conn->updateGroupSec(r.at(2), sec, sIndex, err))
			{
				Logger::getInstance()->info("update group securities success.");
			}
			else
			{
				Logger::getInstance()->info("update group securities failed. {}", err);
			}
		}
		//response = R"({"operate success."})";
		return OK;
	}
	else
	{
		//response = R"({"operate failed."})";
		return SERVER_ERROR;
	}
}


void HttpServer::readdb(ConGroupSec value,int index, std::string group, void* self)
{
	HttpServer* pThis = (HttpServer*)self;
	std::map<int, ConGroupSec> sec;
	sec[index] = value;
	std::set<int> sIndex;
	sIndex.insert(index);

	std::string err;
	if (0 == pThis->m_mt4Conn->updateGroupSec(group, sec, sIndex, err))
	{
		Logger::getInstance()->info("update group securities success.");
	}
	else
	{
		Logger::getInstance()->info("update group securities failed. {}", err);
	}
}


int HttpServer::getGlobalDatafeed(std::string& response, int& mt4res)
{
	mt4res = 0;
	int res = 0;
	int total = 0;
	ConFeeder* feeder = m_mt4Conn->getGlobalDatafeed(total);

	if (Utils::getInstance().parseFromDatafeedToJson(feeder, total, response))
	{
		Logger::getInstance()->info("serialize datafeeder success.");
		//response = R"({"get datafeeder list success."})";
	}
	else
	{
		//response = R"({"serialize datafeeder failed."})";
		res = SERVER_ERROR;
	}
	m_mt4Conn->releaseGlobalDatafeed(feeder);
	return res;
}
int HttpServer::getGlobalCommon(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConCommon common = m_mt4Conn->getGlobalCommon();
	if (Utils::getInstance().parseFromGlobalCommonToJson(common, response))
	{
		Logger::getInstance()->info("serialize global common success");
		//response = R"({"get global common success.})";
	}
	else
	{
		//response = R"({serialize global common failed."})";
		res = SERVER_ERROR;
	}
	return res;
}
int HttpServer::getGlobalIPlist(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	int total = 0;
	ConAccess* ip = m_mt4Conn->getGlobalIPList(total);
	if (Utils::getInstance().parseFromIPListToJson(ip, total, response))
	{
		Logger::getInstance()->info("serialize global ip-list success.");
		//response = R"({"get access list success."})";
	}
	else
	{
		//response = R"({"serialize global ip-list failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGlobalSymbolsList(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	std::vector<ConSymbol> symbols;
	if (0 == (mt4res = m_mt4Conn->getGlobalSymbols(symbols, response)))
	{
		if (Utils::getInstance().parseFromSymbolsListToJson(symbols, response))
		{
			Logger::getInstance()->info("serialize symbol list success.");
		}
		else
		{
			Logger::getInstance()->info("serialize symbol list failed.");
			//response = R"({"derialize symbol list failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("mt4 get symbols info failed.");
		//response = R"({"mt4 get symbols info failed."})";
		res = SERVER_ERROR;
		return res;
	}
	return res;
}
int HttpServer::getGlobalSymbol(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	std::string symbol;
	if (!Utils::getInstance().parseFromJsonToSymbol(body, symbol))
	{
		//response = R"({"invalid param."})";
		res = PARAM_INVALID;
		return res;
	}
	ConSymbol con = {};
	if (0 != (mt4res = m_mt4Conn->getGlobalSymbols(symbol, con, response)) && Utils::getInstance().parseFromSymbolToJson(con, response))
	{
		Logger::getInstance()->info("serial symbol conf success.");
	}
	else
	{
		Logger::getInstance()->error("get symbol conf failed.");
		//response = R"({"get symbol conf failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGlobalDC(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	int total = 0;
	ConDataServer* dc = m_mt4Conn->getGlobalDCList(total);
	if (NULL == dc)
	{
		Logger::getInstance()->error("dc list get failed");
		//response = R"({"dc list get failed."})";
		res = SERVER_ERROR;
	}
	else
	{
		if (Utils::getInstance().parseFromDCListToJson(dc, total, response))
		{
			Logger::getInstance()->info("dc list serialize success.");
		}
		else
		{
			Logger::getInstance()->error("dc list serialize failed.");
			//response = R"({"dc list derialize failed."})";
			res = SERVER_ERROR;
		}
		m_mt4Conn->releaseGlobalDCList(dc);
	}
	return res;
}
int HttpServer::getGlobalPlugin(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	int total = 0;
	ConPluginParam* cp = m_mt4Conn->getGlobalPluginList(total);
	if (NULL == cp)
	{
		Logger::getInstance()->error("cp list get failed.");
		//response = R"({plugin list get failed."})";
		res = SERVER_ERROR;
	}
	else
	{
		if (Utils::getInstance().parseFromPluginListToJson(cp, total, response))
		{
			Logger::getInstance()->info("cp list serialize success.");
		}
		else
		{
			Logger::getInstance()->error("cp list serialize failed.");
			//response = R"({"plugin list derialize failed."})";
			res = SERVER_ERROR;
		}
		m_mt4Conn->releaseGlobalPluginList(cp);
	}
	return res;
}
int HttpServer::getGlobalPerformance(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	unsigned int from = 0;
	if (!Utils::getInstance().parseFromJsonToPerformance(body, from))
	{
		//response = R"({"invalid param."})";
		res = PARAM_INVALID;
		return res;
	}
	PerformanceInfo* pi = nullptr;
	int total = 0;
	if (NULL != (pi = m_mt4Conn->getGlobalPerformance(from, total)) && Utils::getInstance().parseFromPerformanceToJson(pi, total, response))
	{
		Logger::getInstance()->info("serial performance conf success.");
	}
	else
	{
		Logger::getInstance()->error("get performance conf failed.");
		//response = R"({"get performance conf failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getHoliday(std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConHoliday* ch = nullptr;
	int total = 0;
	if (NULL != (ch = m_mt4Conn->getHoliday(total)) &&
		Utils::getInstance().parseFromHolidayToJson(ch, total, response))
	{
		Logger::getInstance()->info("serial conholiday success.");
		m_mt4Conn->releaseHoliday(ch);
	}
	else
	{
		Logger::getInstance()->error("get holiday conf failed.");
		//response = R"({"get holiday conf failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setSessions(const std::string& body,std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	ConSessions css[7] = {};
	std::string symbol;
	if (!body.empty() && Utils::getInstance().parseFromJsonToSession(body, css, symbol))
	{
		if (0 == (mt4res = m_mt4Conn->updateSymbolsSessions(symbol, css, response)))
		{
			Logger::getInstance()->info("update symbol sessions success.");
			//response = R"({"update symbol sessions success."})";
		}
		else if(-1 == mt4res)
		{
			Logger::getInstance()->info("update symbol sessions failed.");
			//response = R"({"update symbol sessions failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize sessions failed.");
		//response = R"({"unserialize sessions failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setSwap(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	std::string symbol;
	int swap_long = 0;
	int swap_short = 0;
	int swap_enable = 0;
	int swap_rollover = 0;
	if (!body.empty() && Utils::getInstance().parseFromJsonToSwap(body, symbol, swap_long, swap_short, swap_enable, swap_rollover))
	{
		if (0 != (mt4res = m_mt4Conn->setSymbolSwap(response, symbol, swap_long, swap_short, swap_enable, swap_rollover)))
		{
			Logger::getInstance()->info("update symbol swap success.");
			//response = R"({"update symbol swap success."})";
		}
		else if(-1 == mt4res)
		{
			Logger::getInstance()->info("update symbol swap failed.");
			//response = R"({"update symbol swap failed."})";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize swap failed.");
		//response = R"({"unserialize swap failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::modifyOpenPrice(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	int orderNo = 0;
	double profit = 0;
	if (!body.empty() && Utils::getInstance().parseFromJsonToOpenPrice(body, orderNo, profit))
	{
		if (0 != (mt4res = m_mt4Conn->updateOrderOpenPrice(orderNo, profit, response)))
		{
			Logger::getInstance()->info("update order open_price success.");
			//response = R"({"update order open_price success."})";
		}
		else if(-1 == mt4res)
		{
			Logger::getInstance()->info("update order open_price failed.");
			//response = R"({"update order open_price failed."})";
			res = SERVER_ERROR;
		}
		else
		{
			//response = "{\"" + response + "\"}";
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize order open_price failed.");
		//response = R"({"unserialize order open_price failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setConSymbolTradeMode(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	if (uriArgs.find("symbol") == uriArgs.end() || uriArgs.find("tradeMode") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}

	if (0 == (mt4res = m_mt4Conn->setSymbolTradeMode(uriArgs.at("symbol"), std::stoi(uriArgs.at("tradeMode")), response)))
	{
		Logger::getInstance()->info("serialize group permission success.");
		//response = R"({"get group permission success."})";
	}
	else if(-1 == mt4res)
	{
		//response = R"({"serialize group permission failed."})";
		res = SERVER_ERROR;
	}
	else
	{
		//response = "{\"" + response + "\"}";
	}
	
	return res;
}

int HttpServer::getConSymbol(const std::map<std::string, std::string>& uriArgs, std::string& response, int &mt4res)
{
	int res = 0;
	mt4res = 0;
	if (uriArgs.find("symbol") == uriArgs.end())
	{
		//response = R"({"param invalid.please check and try again."})";
		res = PARAM_INVALID;
		return res;
	}

	ConSymbol cs = {0};
	if (0 == (mt4res = m_mt4Conn->getConSymbol(uriArgs.at("symbol"), cs, response)))
	{
		Logger::getInstance()->info("serialize group permission success.");
		response = Utils::getInstance().serializeConSymbolToJson(0, 0, cs);
	}
	else
	{
		//response = R"({"param invalid."})";
	}

	return res;
}

int HttpServer::updateConSymbol(const std::string& body, std::string& response, int& mt4res)
{
	ConSymbol cs = {0};
	mt4res = 0;
	int res = 0;
	if (!body.empty() && Utils::getInstance().unSerializeConSymbolToJson(body,cs))
	{
		if (0 == (mt4res = m_mt4Conn->updateConSymbol(cs)))
		{
			Logger::getInstance()->info("update consymbol success.");
			//response = R"({"update order open_price success."})";
		}
		else
		{
			Logger::getInstance()->info("update consymbol failed.");
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize consymbol failed.");
		//response = R"({"unserialize consymbol failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::tradeTransaction(const std::string& body,std::string& response, int &mt4res)
{
	TradeTransInfo tti = { 0 };
	int res = 0;
	mt4res = 0;
	if (!body.empty() && Utils::getInstance().unSerializeTradeTransInfo(body, tti))
	{
		if (0 == (mt4res = m_mt4Conn->tradeTransaction(&tti, response)))
		{
			Logger::getInstance()->info("trade transaction success.");
			//response = R"({"trade transaction success."})";
		}
		else
		{
			Logger::getInstance()->info("trade transaction failed.");
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize tradetraninfo failed.");
		//response = R"({"unserialize tradetraninfo failed."})";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::updateTransaction(const std::string& body, std::string& response, int &mt4res)
{
	TradeRecord tri = { 0 };
	int res = 0;
	mt4res = 0;
	if (!body.empty() && Utils::getInstance().unSerializeTradeRecord(body, tri))
	{
		if (0 == (mt4res = m_mt4Conn->updateTradeRecord(&tri, response)))
		{
			Logger::getInstance()->info("update traderecord success.");
			//response = R"({"update traderecord success."})";
		}
		else
		{
			Logger::getInstance()->info("update traderecord failed.");
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize traderecord failed.");
		//response = R"({"unserialize traderecord failed."})";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::chartReq(const std::string& body, std::string& response, int &mt4res)
{
	RateInfo* rateInfo = nullptr;
	int size = 0;
	mt4res = 0;
	ChartInfo ci = {0};
	int res = 0;
	if (!body.empty() && Utils::getInstance().unSerializeChartInfo(body, ci))
	{
		if (0 == (mt4res = m_mt4Conn->chartInfoReq(&ci, rateInfo, size, response)))
		{
			Logger::getInstance()->info("get chart info success.");
			response = Utils::getInstance().serializeRateInfo(rateInfo, size);
			m_mt4Conn->releaseChartInfo(rateInfo);
		}
		else if(-1 == mt4res)
		{
			Logger::getInstance()->info("get chart info failed.");
			//response = R"( { "get chart info failed." } )";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize charinfo failed.");
		//response = R"( {"unserialize chartinfo failed."} )";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::chartUpdate(const std::string& body, std::string& response, int &mt4Res)
{
	RateInfo* rateInfo = nullptr;
	int size = 0;
	int period = 0;
	std::string symbol;
	
	int res = 0;
	if (!body.empty() && Utils::getInstance().unSerializeChartupdate(body, symbol, period, size, rateInfo))
	{
		if (0 == (mt4Res = m_mt4Conn->chartInfoUpdate(symbol,period, size, rateInfo, response)))
		{
			Logger::getInstance()->info("update chart info success.");
			//response = R"({ "update chart info success ."})";
		}
		else
		{
			Logger::getInstance()->info("update chart info failed.");
			res = 0;
		}
		delete []rateInfo;
	}
	else
	{
		Logger::getInstance()->error("unserialize charinfo failed.");
		//response = R"({ "unserialize chartinfo failed."})";
		res = PARAM_INVALID;
	}
	return res;
}


int HttpServer::getTrades(const std::map<std::string, std::string> uriArgs, std::string& response, int &mt4res)
{
	int res = 0;
	mt4res = 0;
	if (uriArgs.find("ticket") == uriArgs.end() || uriArgs.at("ticket").empty())
	{
		Logger::getInstance()->error("param invalid.please check and try again.");
		res = PARAM_INVALID;
		return res;
	}
	
	std::vector<TradeRecord> record;
	int size = 0;
	if (0 == (mt4res = m_mt4Conn->tradesGetByTicket(uriArgs.at("ticket"), record, response)))
	{
		if(Utils::getInstance().serializeTradeRecord(record, response))
		{
			return res;
		}
		else
		{
			Logger::getInstance()->error("serialize trade record failed.");
			res = SERVER_ERROR;
			return res;
		}
	}
	else
	{
		Logger::getInstance()->error("get record by symbol failed.");
	}
	return res;
}

int HttpServer::getUserRecord(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res)
{
	int res = 0;
	mt4res = 0;
	if (uriArgs.find("login") == uriArgs.end() || uriArgs.at("login").empty())
	{
		Logger::getInstance()->error("param invalid.please check and try again.");
		res = PARAM_INVALID;
		return res;
	}

	UserRecord ur = {0};
	int size = 0;
	if (0 == (mt4res = m_mt4Conn->getUserRecord(std::stoi(uriArgs.at("login")), ur, response)))
	{
		if (Utils::getInstance().serializeUserRecord(ur, response))
		{
			return res;
		}
		else
		{
			Logger::getInstance()->error("serialize user record failed.");
			res = SERVER_ERROR;
			return res;
		}
	}
	else
	{
		Logger::getInstance()->error("get user record by login failed.");
	}
	return res;
}

int HttpServer::updateUserRecord(const std::string& body, std::string& response, int& mt4res)
{
	int res = 0;
	UserRecord ur = {0};
	if (!body.empty() && Utils::getInstance().unSerializeUserRecord(body, ur))
	{
		if (0 == (mt4res = m_mt4Conn->updateUserRecord(ur, response)))
		{
			Logger::getInstance()->info("update user record success.");
			//response = R"({ "update chart info success ."})";
		}
		else
		{
			Logger::getInstance()->info("update user record failed.");
			res = 0;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize user record failed.");
		//response = R"({ "unserialize chartinfo failed."})";
		res = PARAM_INVALID;
	}
	return res;
}