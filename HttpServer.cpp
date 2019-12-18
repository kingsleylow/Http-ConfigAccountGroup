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
	std::string common_securities = Config::getInstance().getHTTPConf().find("common-symbol-groups")->second;
	std::string securities_auto = Config::getInstance().getHTTPConf().find("account-group-securities-auto")->second;

	std::string securities_auto_get = securities_auto + "-get";
	std::string securities_auto_set = securities_auto + "-set";

	std::string accounts = Config::getInstance().getHTTPConf().find("account-configuration")->second;

	std::string global_datafeed = Config::getInstance().getHTTPConf().find("datafeed-configuration")->second;
	std::string global_common =  Config::getInstance().getHTTPConf().find("common-configuration")->second;
	std::string global_iplist =  Config::getInstance().getHTTPConf().find("ip-access-configuration")->second;
	std::string global_symbols_list =  Config::getInstance().getHTTPConf().find("symbols-list")->second;
	std::string global_symbol_conf = Config::getInstance().getHTTPConf().find("symbol-configuration")->second;
	std::string global_dc = Config::getInstance().getHTTPConf().find("data-center-configuration")->second;
	std::string global_plugin =  Config::getInstance().getHTTPConf().find("plugin-configuration")->second;
	std::string global_performance =  Config::getInstance().getHTTPConf().find("performance-configuration")->second;

	std::string get_holiday = Config::getInstance().getHTTPConf().find("get-holiday")->second;
	std::string set_sessions = Config::getInstance().getHTTPConf().find("set-symbol-session")->second;
	std::string set_swap = Config::getInstance().getHTTPConf().find("set-swap")->second;
	std::string modify_open_price = Config::getInstance().getHTTPConf().find("modify-open_price")->second;
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
	{modify_open_price, MODIFY_OPENPRICE} };
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

	//CRYPTO_set_mem_functions(HttpServer::my_zeroing_malloc, realloc, free);
	//SSL_library_init();
	//SSL_load_error_strings();
	//OpenSSL_add_all_algorithms();
	//Logger::getInstance()->info("Using OpenSSL version [{}], libevent version [{}]", SSLeay_version(SSLEAY_VERSION), event_get_version());

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

	//m_ctx = SSL_CTX_new(SSLv23_server_method());
	//SSL_CTX_set_options(m_ctx, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE |SSL_OP_NO_SSLv2);

	//m_ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	//if (nullptr == m_ecdh)
	//{
	//	Logger::getInstance()->error("EC_KEY_new_by_curve_name failed.");
	//	return false;
	//}

	//if (1 != SSL_CTX_set_tmp_ecdh(m_ctx, m_ecdh))
	//{
	//	Logger::getInstance()->error("SSL_CTX_set_tmp_ecdh failed.");
	//	return false;
	//}

	//std::string path = Config::getInstance().getExePath();
	//std::string certificate_chain = path + Config::getInstance().getHTTPConf().find("certificate")->second;
	//std::string private_key = path + Config::getInstance().getHTTPConf().find("private_key")->second;
	//
	//if (!serverSetupCerts(m_ctx, certificate_chain.c_str(), private_key.c_str()))
	//{
	//	Logger::getInstance()->error("setup certificate failed.exit...");
	//	return false;
	//}

	//evhttp_set_bevcb(m_http, bevCb, m_ctx);

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
	std::string des;
	std::string response = "";
	if (pSelf->parseReq(req, method, uri, uriArgs, body))
	{
		res = pSelf->handleReq(method, uri, uriArgs, body, des, response);
	}
	else
	{
		des = R"("http service parse failed")";
	}

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> w(sb);
	w.StartObject();

	w.Key("code");
	w.Int(res);

	w.Key("description");
	w.String(des.c_str());

	w.Key("response");
	if (response.empty())
	{
		w.StartObject();
		w.EndObject();
	}
	else
	{
		w.RawValue(response.c_str(), response.length(), rapidjson::Type::kObjectType);
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
	Logger::getInstance()->info("response [{}]", resp);

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
		evhttp_send_reply(req, 404, "Not Found", evb);
		Logger::getInstance()->error("response: bad url！");
	}
	break;
	case BAD_METHOD:
	{
		evhttp_send_reply(req, 405, "Method Not Allowed", evb);
		Logger::getInstance()->error("response: method not allowed！");
	}
	break;
	case PARAM_INVALID:
	{
		evhttp_send_reply(req, 400, "Bad Request", evb);
		Logger::getInstance()->error("response: bad request！");
	}
	break;
	case SERVER_ERROR:
	{
		evhttp_send_reply(req, 500, "Internal Server Error", evb);
		Logger::getInstance()->error("response: internal server error！");
	}
	break;
	default:
	{
		evhttp_send_reply(req, 500, "Internal Server Error", evb);
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

int HttpServer::handleReq(const evhttp_cmd_type& method, const std::string& uri, const std::map<std::string, std::string>& uriArgs, const std::string& body,std::string& des, std::string& response)
{
	int res = OK;
	if (method == EVHTTP_REQ_POST)
	{
		switch (m_uri[uri])
		{
		case COMMON:
			res = setGroupCommon(body, des, response);
			break;
		case PERMISSIONS:
			res = setGroupPermission(body, des, response);
			break;
		case ARCHIVING:
			res = setGroupArchive(body, des, response);
			break;
		case MARGINS:
			res = setGroupMargins(body, des, response);
			break;
		case SECURITIES:
			res = setGroupSecurities(body, des, response);
			break;
		case SYMBOLS:
			res = setGroupSymbols(body, des, response);
			break;
		case REPORTS:
			res = setGroupReport(body, des, response);
			break;
		case ACCOUNT_CONFIGUTATION:
			res = setAccount(body, des, response);
			break;
		case GLOBAL_SYMBOL:
			res = getGlobalSymbol(body, des, response);
			break;
		case GLOBAL_PERFORMANCE:
			res = getGlobalPerformance(body, des, response);
			break;
		case SET_SESSIONS:
			res = setSessions(body, des, response);
			break;
		case SET_SWAP:
			res = setSwap(body, des, response);
			break;
		case MODIFY_OPENPRICE:
			res = modifyOpenPrice(body, des, response);
			break;
		default:
			des = "bad url or request method not right";
			res = BAD_URL;
			break;
		}
	}
	else if (method == EVHTTP_REQ_GET)
	{
		switch (m_uri[uri])
		{
		case COMMON:
			res = getGroupCommon(uriArgs,des, response);
			break;
		case PERMISSIONS:
			res = getGroupPermission(uriArgs, des, response);
			break;
		case ARCHIVING:
			res = getGroupArchive(uriArgs,des, response);
			break;
		case MARGINS:
			res = getGroupMargins(uriArgs,des, response);
			break;
		case SECURITIES:
			res = getGroupSecurities(uriArgs,des, response);
			break;
		case SYMBOLS:
			res = getGroupSymbols(uriArgs, des, response);
			break;
		case REPORTS:
			res = getGroupReport(uriArgs,des, response);
			break;
		case COMMON_GROUPS:
			res = getGroupsNames(des, response);
			break;
		case COMMON_SECURITIES:
			res = getSecuritiesNames(des, response);
			break;
		case SECURITIES_AUTO_GET:  //get
			res = getAllGroupsSecurities(des, response);
			break;
		case SECURITIES_AUTO_SET:  //set
			res = setAllGroupsSecurities(des, response);
			break;
		case GLOBAL_DATAFEED:
			res = getGlobalDatafeed(des, response);
			break;
		case GLOBAL_COMMON:
			res = getGlobalCommon(des, response);
			break;
		case GLOBAL_IP:
			res = getGlobalIPlist(des, response);
			break;
		case GLOBAL_SYMBOLS_LIST:
			res = getGlobalSymbolsList(des, response);
			break;
		case GLOBAL_DC:
			res = getGlobalDC(des, response);
			break;
		case GLOBAL_PLUGIN:
			res = getGlobalPlugin(des, response);
			break;
		case GET_HOLIDAY:
			res = getHoliday(des, response);
			break;
		default:
			des = "bad url or request method not right";
			res = BAD_URL;
			break;
		}
	}
	else
	{
		response = "not support the request method, only can ues get and post for now.";
		res = BAD_METHOD;
	}

	return res;
}

int HttpServer::setGroupSecurities(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	std::map<int, ConGroupSec> sec;
	std::set<int> index;
	std::string group;
	if (Utils::getInstance().parseFromJsonToSec(body, sec, index, group))
	{
		if (m_mt4Conn->updateGroupSec(group, sec, index))
		{
			Logger::getInstance()->info("update group securities success.");
			des = "update group securities success.";
		}
		else
		{
			des = "update group securities failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupCommon(const std::string& body,std::string& des, std::string& response)
{
	int res = 0;
	GroupCommon groupCommon;
	std::string group;
	if (Utils::getInstance().parseFromJsonToCommon(body, groupCommon, group))
	{
		if (m_mt4Conn->updateGroupCommon(group, groupCommon))
		{
			Logger::getInstance()->info("update group common success.");
			des = "update group common success.";
		}
		else
		{
			des = "update group common failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupSymbols(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	std::map<std::string, ConGroupMargin> margins;
	std::string group;
	if (Utils::getInstance().parseFromJsonToSymbol(body, margins, group))
	{
		if (m_mt4Conn->updateGroupSymbol(group, margins))
		{
			Logger::getInstance()->info("update group securities success.");
			des = "update group securities success.";
		}
		else
		{
			des = "update group securities failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::getGroupCommon(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	GroupCommon groupComm;//default constructor
	groupComm = m_mt4Conn->getGroupCommon(uriArgs.at("Group"));
	/*if (groupComm.group.empty())
	{
		des = "Group is not existed, pls check.";
		res = PARAM_INVALID;
		return res;
	}*/
	if (Utils::getInstance().parseFromCommonToJson(uriArgs.at("Group"), groupComm, response))
	{
		Logger::getInstance()->info("serialize group common success.");
		des = "get group common success.";
	}
	else
	{
		des = "serialize group common failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupSecurities(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	ConGroup conGroup = { 0 };
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	conGroup = m_mt4Conn->getGroupCfg(uriArgs.at("Group"));
	int size = sizeof(conGroup.secgroups) / sizeof(ConGroupSec);
	if (Utils::getInstance().parseFromSecToJson(conGroup.group, conGroup.secgroups, size, response))
	{
		Logger::getInstance()->info("serialize group securities success.");
		des = "get group securities success.";
	}
	else
	{
		des = "serialize group securities failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupSymbols(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	ConGroup conGroup = { 0 };
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	conGroup = m_mt4Conn->getGroupCfg(uriArgs.at("Group"));
	int size = conGroup.secmargins_total;
	if (Utils::getInstance().parseFromSymbolToJson(conGroup.group, conGroup.secmargins, size, response))
	{
		Logger::getInstance()->info("serialize group margins success.");
		des = "get group symbols success.";
	}
	else
	{
		des = "serialize group margins failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupsNames(std::string& des, std::string& response)
{
	int res = 0;
	std::vector<std::string> common_groups;
	if (m_mt4Conn->getGroupNames(common_groups))
	{
		if (Utils::getInstance().parseFromCommonGroupsToJson(common_groups, response))
		{
			Logger::getInstance()->info("serialize groups success.");;
			des = "get group names success.";
		}
		else
		{
			des = "serialize groups failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "get groups failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getSecuritiesNames(std::string& des, std::string& response)
{
	int res = 0;
	ConSymbolGroup securities[MAX_SEC_GROUPS] = { 0 };
	if (0 == m_mt4Conn->getSecuritiesNames(securities))
	{
		int size = sizeof(securities) / sizeof(ConSymbolGroup);
		if (Utils::getInstance().parseFromCommmonSecuritesToJson(securities, size, response))
		{
			Logger::getInstance()->info("serialize securities success.");
			des = "get sucurities name success.";
		}
		else
		{
			des = "serialize securities failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "get securites failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupMargins(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	GroupMargin margin;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	margin = m_mt4Conn->getGroupMargin(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromMarginToJson(uriArgs.at("Group"), margin,  response))
	{
		Logger::getInstance()->info("serialize group margins success.");
		des = "get group margin success.";
	}
	else
	{
		des = "serialize group margins failed.";
		res = SERVER_ERROR;
	}
	return res;
}


int HttpServer::getGroupArchive(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	GroupArchive archive;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	archive = m_mt4Conn->getGroupArchive(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromArchiveToJson(uriArgs.at("Group"), archive, response))
	{
		Logger::getInstance()->info("serialize group archive success.");
		des = "get group archive success.";
	}
	else
	{
		des = "serialize group archive failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupReport(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	GroupReport report;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	report = m_mt4Conn->getGroupReport(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromReportToJson(uriArgs.at("Group"), report, response))
	{
		Logger::getInstance()->info("serialize group report success.");
		des = "get group report success.";
	}
	else
	{
		des = "serialize group report failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGroupPermission(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response)
{
	int res = 0;
	GroupPermission permission;
	if (uriArgs.find("Group") == uriArgs.end())
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
		return res;
	}
	permission = m_mt4Conn->getGroupPermission(uriArgs.at("Group"));

	if (Utils::getInstance().parseFromPermissionToJson(uriArgs.at("Group"), permission, response))
	{
		Logger::getInstance()->info("serialize group permission success.");
		des = "get group permission success.";
	}
	else
	{
		des = "serialize group permission failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setAccount(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	AccountConfiguration configuration;
	std::string login;
	if (Utils::getInstance().parseFromJsonToAccuntConfiguration(body, configuration, login))
	{
		if (m_mt4Conn->updateAccounts(login, configuration))
		{
			Logger::getInstance()->info("update account configuration success.");
			des = "update account configuration success.";
		}
		else
		{
			des = "update account configuration failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupMargins(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	GroupMargin margins;
	std::string group;
	if (Utils::getInstance().parseFromJsonToMargin(body, margins, group))
	{
		if (m_mt4Conn->updateGroupMargin(group, margins))
		{
			Logger::getInstance()->info("update group margin success.");
			des = "update group margin success.";
		}
		else
		{
			des = "update group margin failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupArchive(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	GroupArchive archive;
	std::string group;
	if (Utils::getInstance().parseFromJsonToArchive(body, archive, group))
	{
		if (m_mt4Conn->updateGroupArchive(group, archive))
		{
			Logger::getInstance()->info("update group archive success.");
			des = "update group archive success.";
		}
		else
		{
			des = "update group archive failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupReport(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	GroupReport report;
	std::string group;
	if (Utils::getInstance().parseFromJsonToReport(body, report, group))
	{
		if (m_mt4Conn->upateGroupReport(group, report))
		{
			Logger::getInstance()->info("update group report success.");
			des = "update group report success.";
		}
		else
		{
			des = "update group report failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::setGroupPermission(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	GroupPermission permission;
	std::string group;
	if (Utils::getInstance().parseFromJsonToPermission(body, permission, group))
	{
		if (m_mt4Conn->updateGroupPerssion(group, permission))
		{
			Logger::getInstance()->info("update group permission success.");
			des = "update group permission success.";
		}
		else
		{
			des = "update group permission failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		des = "param invalid.please check and try again.";
		res = PARAM_INVALID;
	}
	return res;
}

int HttpServer::getAllGroupsSecurities(std::string& des, std::string& response)
{
	std::map<int, std::string> securitiesMap;
	ConSymbolGroup securities[MAX_SEC_GROUPS] = { 0 };
	if (0 == m_mt4Conn->getSecuritiesNames(securities))
	{
		int size = sizeof(securities) / sizeof(ConSymbolGroup);
		for (int i = 0; i < size; i++)
		{
			securitiesMap[i] = securities[i].name;
		}
	}
	else
	{
		des = "operate failed.";
		return SERVER_ERROR;
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
					des = "operate failed.";
					return SERVER_ERROR;
				}
			}
		}
		des = "operate success";
		return OK;
	}
	else
	{
		des = "operate failed.";
		return SERVER_ERROR;
	}
}

int HttpServer::setAllGroupsSecurities(std::string& des, std::string& response)
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

			if (m_mt4Conn->updateGroupSec(r.at(2), sec, sIndex))
			{
				Logger::getInstance()->info("update group securities success.");
			}
			else
			{
				Logger::getInstance()->info("update group securities failed.");
			}
		}
		des = "operate success";
		return OK;
	}
	else
	{
		des = "operate failed.";
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

	if (pThis->m_mt4Conn->updateGroupSec(group, sec, sIndex))
	{
		Logger::getInstance()->info("update group securities success.");
	}
	else
	{
		Logger::getInstance()->info("update group securities failed.");
	}
}


int HttpServer::getGlobalDatafeed(std::string& des, std::string& response)
{
	int res = 0;
	int total = 0;
	ConFeeder* feeder = m_mt4Conn->getGlobalDatafeed(total);

	if (Utils::getInstance().parseFromDatafeedToJson(feeder, total, response))
	{
		Logger::getInstance()->info("serialize datafeeder success.");
		des = "get datafeeder list success.";
	}
	else
	{
		des = "serialize datafeeder failed.";
		res = SERVER_ERROR;
	}
	m_mt4Conn->releaseGlobalDatafeed(feeder);
	return res;
}
int HttpServer::getGlobalCommon(std::string& des, std::string& response)
{
	int res = 0;
	ConCommon common = m_mt4Conn->getGlobalCommon();
	if (Utils::getInstance().parseFromGlobalCommonToJson(common, response))
	{
		Logger::getInstance()->info("serialize global common success");
		des = "get global common success.";
	}
	else
	{
		des = "serialize global common failed.";
		res = SERVER_ERROR;
	}
	return res;
}
int HttpServer::getGlobalIPlist(std::string& des, std::string& response)
{
	int res = 0;
	int total = 0;
	ConAccess* ip = m_mt4Conn->getGlobalIPList(total);
	if (Utils::getInstance().parseFromIPListToJson(ip, total, response))
	{
		Logger::getInstance()->info("serialize global ip-list success.");
		des = "get access list success.";
	}
	else
	{
		des = "serialize global ip-list failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGlobalSymbolsList(std::string& des, std::string& response)
{
	int res = 0;
	std::vector<ConSymbol> symbols;
	if (!m_mt4Conn->getGlobalSymbols(symbols))
	{
		Logger::getInstance()->error("mt4 get symbols info failed.");
		des = "mt4 get symbols info failed";
		res = SERVER_ERROR;
		return res;
	}
	else
	{
		if (Utils::getInstance().parseFromSymbolsListToJson(symbols, response))
		{
			Logger::getInstance()->info("serialize symbol list success.");
			des = "get symbol list succes.";
		}
		else
		{
			Logger::getInstance()->info("serialize symbol list failed.");
			des = "derialize symbol list failed.";
			res = SERVER_ERROR;
		}
	}
	return res;
}
int HttpServer::getGlobalSymbol(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	std::string symbol;
	if (!Utils::getInstance().parseFromJsonToSymbol(body, symbol))
	{
		des = "invalid param";
		res = PARAM_INVALID;
		return res;
	}
	ConSymbol con = {};
	if (m_mt4Conn->getGlobalSymbols(symbol, con) && Utils::getInstance().parseFromSymbolToJson(con, response))
	{
		Logger::getInstance()->info("serial symbol conf success.");
		des = "get symbol conf success.";
	}
	else
	{
		Logger::getInstance()->error("get symbol conf failed.");
		des = "get symbol conf failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getGlobalDC(std::string& des, std::string& response)
{
	int res = 0;
	int total = 0;
	ConDataServer* dc = m_mt4Conn->getGlobalDCList(total);
	if (NULL == dc)
	{
		Logger::getInstance()->error("dc list get failed");
		des = "dc list get failed.";
		res = SERVER_ERROR;
	}
	else
	{
		if (Utils::getInstance().parseFromDCListToJson(dc, total, response))
		{
			Logger::getInstance()->info("dc list serialize success.");
			des = "get dc list success.";
		}
		else
		{
			Logger::getInstance()->error("dc list serialize failed.");
			des = "dc list derialize failed.";
			res = SERVER_ERROR;
		}
		m_mt4Conn->releaseGlobalDCList(dc);
	}
	return res;
}
int HttpServer::getGlobalPlugin(std::string& des, std::string& response)
{
	int res = 0;
	int total = 0;
	ConPluginParam* cp = m_mt4Conn->getGlobalPluginList(total);
	if (NULL == cp)
	{
		Logger::getInstance()->error("cp list get failed.");
		des = "plugin list get failed.";
		res = SERVER_ERROR;
	}
	else
	{
		if (Utils::getInstance().parseFromPluginListToJson(cp, total, response))
		{
			Logger::getInstance()->info("cp list serialize success.");
			des = "get plugin list success.";
		}
		else
		{
			Logger::getInstance()->error("cp list serialize failed.");
			des = "plugin list derialize failed.";
			res = SERVER_ERROR;
		}
		m_mt4Conn->releaseGlobalPluginList(cp);
	}
	return res;
}
int HttpServer::getGlobalPerformance(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	unsigned int from = 0;
	if (!Utils::getInstance().parseFromJsonToPerformance(body, from))
	{
		des = "invalid param";
		res = PARAM_INVALID;
		return res;
	}
	PerformanceInfo* pi = nullptr;
	int total = 0;
	if (NULL != (pi = m_mt4Conn->getGlobalPerformance(from, total)) && Utils::getInstance().parseFromPerformanceToJson(pi, total, response))
	{
		Logger::getInstance()->info("serial performance conf success.");
		des = "get performance conf success.";
	}
	else
	{
		Logger::getInstance()->error("get performance conf failed.");
		des = "get performance conf failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::getHoliday(std::string& des, std::string& response)
{
	int res = 0;
	ConHoliday* ch = nullptr;
	int total = 0;
	if (NULL != (ch = m_mt4Conn->getHoliday(total)) &&
		Utils::getInstance().parseFromHolidayToJson(ch, total, response))
	{
		Logger::getInstance()->info("serial conholiday success.");
		des = "get holiday conf success.";
		m_mt4Conn->releaseHoliday(ch);
	}
	else
	{
		Logger::getInstance()->error("get holiday conf failed.");
		des = "get holiday conf failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setSessions(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	ConSessions css[7] = {};
	std::string symbol;
	if (!body.empty() && Utils::getInstance().parseFromJsonToSession(body, css, symbol))
	{
		if (m_mt4Conn->updateSymbolsSessions(symbol, css))
		{
			Logger::getInstance()->info("update symbol sessions success.");
			des = "update symbol sessions success.";
		}
		else
		{
			Logger::getInstance()->info("update symbol sessions failed.");
			des = "update symbol sessions failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize sessions failed.");
		des = "unserialize sessions failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::setSwap(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	std::string symbol;
	int swap_long = 0;
	int swap_short = 0;
	int swap_enable = 0;
	int swap_rollover = 0;
	if (!body.empty() && Utils::getInstance().parseFromJsonToSwap(body, symbol, swap_long, swap_short, swap_enable, swap_rollover))
	{
		if (m_mt4Conn->setSymbolSwap(symbol, swap_long, swap_short, swap_enable, swap_rollover))
		{
			Logger::getInstance()->info("update symbol swap success.");
			des = "update symbol swap success.";
		}
		else
		{
			Logger::getInstance()->info("update symbol swap failed.");
			des = "update symbol swap failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize swap failed.");
		des = "unserialize swap failed.";
		res = SERVER_ERROR;
	}
	return res;
}

int HttpServer::modifyOpenPrice(const std::string& body, std::string& des, std::string& response)
{
	int res = 0;
	int orderNo = 0;
	double profit = 0;
	if (!body.empty() && Utils::getInstance().parseFromJsonToOpenPrice(body, orderNo, profit))
	{
		if (m_mt4Conn->updateOrderOpenPrice(orderNo, profit))
		{
			Logger::getInstance()->info("update order open_price success.");
			des = "update order open_price success.";
		}
		else
		{
			Logger::getInstance()->info("update order open_price failed.");
			des = "update order open_price failed.";
			res = SERVER_ERROR;
		}
	}
	else
	{
		Logger::getInstance()->error("unserialize order open_price failed.");
		des = "unserialize order open_price failed.";
		res = SERVER_ERROR;
	}
	return res;
}