#pragma once
#include "event2/http.h"
#include "event2/event.h"
#include "event2/thread.h"
#include "event2/keyvalq_struct.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/bufferevent_ssl.h"
#include "event2/util.h"
#include <string>
#include <map>
#include "MT4Module.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"

using STATUS = enum { OK, BAD_URL, BAD_METHOD, PARAM_INVALID, SERVER_ERROR};
using URI = enum {COMMON = 1, PERMISSIONS, ARCHIVING, MARGINS, SECURITIES, SYMBOLS, REPORTS, COMMON_GROUPS, COMMON_SECURITIES, SECURITIES_AUTO_GET, SECURITIES_AUTO_SET, ACCOUNT_CONFIGUTATION,
                  GLOBAL_DATAFEED, GLOBAL_COMMON, GLOBAL_IP, GLOBAL_SYMBOLS_LIST,GLOBAL_SYMBOL, GLOBAL_DC, GLOBAL_PLUGIN, GLOBAL_PERFORMANCE, GET_HOLIDAY, SET_SESSIONS, SET_SWAP, MODIFY_OPENPRICE};

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

	bool initServerHttp();
	int  stopServer();
	int  startServer();

	void setMT4Conn(MT4Conn* conn);

private:
	static void *my_zeroing_malloc(size_t howmuch);
	static void cbFunc(struct evhttp_request *, void *args);
	bool serverSetupCerts(SSL_CTX* ctx, const char* certificate_chain, const char* private_key);
	void loginCb(struct evhttp_request* req, void* arg);
	static bufferevent* bevCb(event_base* base, void* arg);

	bool parseReq(struct evhttp_request* req, evhttp_cmd_type& method, std::string& uri, std::map<std::string, std::string>& uriArgs , std::string& body);
	int  handleReq(const evhttp_cmd_type& method, const std::string& uri, const std::map<std::string, std::string>& uriArgs, const std::string& body,std::string& des, std::string& response);

	int getGroupsNames(std::string& des, std::string& response);
	int getSecuritiesNames(std::string& des, std::string& response);

	int setGroupSecurities(const std::string& body, std::string& des, std::string& response);
	int getGroupSecurities(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);

	int setGroupCommon(const std::string& body,std::string& des, std::string& response);
	int getGroupCommon(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);

	int setGroupSymbols(const std::string& body, std::string& des, std::string& response);
	int getGroupSymbols(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);

	int getGroupMargins(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);
	int setGroupMargins(const std::string& body, std::string& des, std::string& response);

	int getGroupArchive(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);
	int setGroupArchive(const std::string& body, std::string& des, std::string& response);

	int getGroupReport(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);
	int setGroupReport(const std::string& body, std::string& des, std::string& response);

	int getGroupPermission(const std::map<std::string, std::string>& uriArgs, std::string& des, std::string& response);
	int setGroupPermission(const std::string& body, std::string& des, std::string& response);


	int getAllGroupsSecurities(std::string& des, std::string& response);  //out
	int setAllGroupsSecurities(std::string& des, std::string& response);

	int getGlobalDatafeed(std::string& des, std::string& response);
	int getGlobalCommon(std::string& des, std::string& response);
	int getGlobalIPlist(std::string& des, std::string& response);
	int getGlobalSymbolsList(std::string& des, std::string& response);
	int getGlobalSymbol(const std::string& body, std::string& des, std::string& response);
	int getGlobalDC(std::string& des, std::string& response);
	int getGlobalPlugin(std::string& des, std::string& response);
	int getGlobalPerformance(const std::string& body, std::string& des, std::string& response);

	int setAccount(const std::string& body, std::string& des, std::string& response);

	int getHoliday(std::string& des, std::string& response);
	int setSessions(const std::string& body, std::string& des, std::string& response);

	int setSwap(const std::string& body, std::string& des, std::string& response);

	int modifyOpenPrice(const std::string& body, std::string& des, std::string& response);
	
	static void readdb(ConGroupSec, int index, std::string, void*);
private:
	struct event_base* m_evBase;
	struct evhttp* m_http;
	SSL_CTX* m_ctx;
	EC_KEY* m_ecdh;
	std::map<evhttp_cmd_type, std::string> m_httpMethod;
	std::map<std::string, URI> m_uri;
	MT4Conn* m_mt4Conn;
};