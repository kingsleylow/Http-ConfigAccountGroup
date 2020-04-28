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
                  GLOBAL_DATAFEED, GLOBAL_COMMON, GLOBAL_IP, GLOBAL_SYMBOLS_LIST,GLOBAL_SYMBOL, GLOBAL_DC, GLOBAL_PLUGIN, GLOBAL_PERFORMANCE, GET_HOLIDAY, SET_SESSIONS, SET_SWAP, MODIFY_OPENPRICE, 
	SET_SYMBOL_TRADEMODE, CON_SYMBOL, UPDATE_CONSYMBOL, TRADE,GET_TRADE, UPDATE_TRADE, CHART_REQ, CHART_UPDATE, UPDATE_USERRECORD, GET_USERRECORD
};

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
	int  handleReq(const evhttp_cmd_type& method, const std::string& uri, const std::map<std::string, std::string>& uriArgs, const std::string& body, std::string& response,int& mt4res);

	int getGroupsNames(std::string& response, int& mt4res);
	int getSecuritiesNames(std::string& response, int& mt4res);

	int setGroupSecurities(const std::string& body, std::string& response, int& mt4res);
	int getGroupSecurities(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);

	int setGroupCommon(const std::string& body, std::string& response, int& mt4res);
	int getGroupCommon(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);

	int setGroupSymbols(const std::string& body, std::string& response, int& mt4res);
	int getGroupSymbols(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);

	int getGroupMargins(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);
	int setGroupMargins(const std::string& body, std::string& response, int& mt4res);

	int getGroupArchive(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);
	int setGroupArchive(const std::string& body,  std::string& response, int& mt4res);

	int getGroupReport(const std::map<std::string, std::string>& uriArgs,std::string& response, int& mt4res);
	int setGroupReport(const std::string& body, std::string& response, int& mt4res);

	int getGroupPermission(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);
	int setGroupPermission(const std::string& body, std::string& response, int& mt4res);


	int getAllGroupsSecurities(std::string& response, int& mt4res);  //out
	int setAllGroupsSecurities(std::string& response, int& mt4res);

	int getGlobalDatafeed(std::string& response, int& mt4res);
	int getGlobalCommon(std::string& response, int& mt4res);
	int getGlobalIPlist(std::string& response, int& mt4res);
	int getGlobalSymbolsList(std::string& response, int& mt4res);
	int getGlobalSymbol(const std::string& body, std::string& response, int& mt4res);
	int getGlobalDC(std::string& response, int& mt4res);
	int getGlobalPlugin(std::string& response, int& mt4res);
	int getGlobalPerformance(const std::string& body, std::string& response, int& mt4res);

	int setAccount(const std::string& body, std::string& response, int& mt4res);

	int getHoliday(std::string& response, int& mt4res);
	int setSessions(const std::string& body, std::string& response, int& mt4res);

	int setSwap(const std::string& body, std::string& response, int& mt4res);

	int modifyOpenPrice(const std::string& body, std::string& response, int& mt4res);
	
	static void readdb(ConGroupSec, int index, std::string, void*);
	int setConSymbolTradeMode(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);

	int getConSymbol(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);
	int updateConSymbol(const std::string& body, std::string& response, int& mt4res);

	int tradeTransaction(const std::string& body, std::string& response, int& mt4res);
	int updateTransaction(const std::string& body, std::string& response, int& mt4res);
	int getTrades(const std::map<std::string, std::string> uriArgs, std::string& response, int &mt4Res);

	int chartReq(const std::string& body, std::string& response, int& mt4res);
	int chartUpdate(const std::string& body, std::string& response, int& mt4res);

	int getUserRecord(const std::map<std::string, std::string>& uriArgs, std::string& response, int& mt4res);
	int updateUserRecord(const std::string& body, std::string& response, int& mt4res);
private:
	struct event_base* m_evBase;
	struct evhttp* m_http;
	SSL_CTX* m_ctx;
	EC_KEY* m_ecdh;
	std::map<evhttp_cmd_type, std::string> m_httpMethod;
	std::map<std::string, URI> m_uri;
	MT4Conn* m_mt4Conn;
};