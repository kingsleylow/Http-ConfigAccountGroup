#pragma once
#include "MT4ManagerAPI.h"
#include "Config.h"
#include "Utils.h"
#include <set>
#include <map>
#include "MemCache.h"
#include "RedisClient.h"
#include "MT4DirectConn.h"
#include "MT4PumpConn.h"

//using URI = enum { COMMON, PERMISSIONS, ARCHIVING, MARGINS, SECURITIES, SYMBOLS, REPORTS };
using pumpCallBack = void(__stdcall *)(int code, int type, void *data, void *param);
class MT4Conn
{
public:
	MT4Conn();
	~MT4Conn();

public:
	/************************************************
	** Checks the state of connection to a trading server.
	** Arguments:
	**   none.
	** Returns:
	**   false: disconnected.
	**   true : connected
	*************************************************/

	bool createConnToMT4();
	/************************************************
	** create interface of manager api
	** Arguments:
	**   mt4LibPath: MT4 lib's path
	** Returns:
	**   true: success
	**   false: failure
	*************************************************/
	bool mt4Init();
	void watchDirectConntoMT4();
	void watchPumpConnToMT4();

	ConGroup getGroupCfg(const std::string& group);
	int updateGroupSec(const std::string& group, const std::map<int, ConGroupSec>& cfgGroupSec,std::set<int> index, std::string& err);
	
	int updateGroupSymbol(const std::string& group, const std::map<std::string, ConGroupMargin>& cfgGroupMargin, std::string& err);

	GroupCommon getGroupCommon(const std::string& group);
	int updateGroupCommon(const std::string& group, const GroupCommon& common, std::string& err);

	bool getGroupNames(std::vector<std::string>& groups);
	int getSecuritiesNames(ConSymbolGroup securities[], std::string& err); //return value:0 success. otherwise failed.

	GroupMargin getGroupMargin(const std::string& group);
	int updateGroupMargin(const std::string group, const GroupMargin& margin, std::string& err);

	GroupArchive getGroupArchive(const std::string& group);
	int updateGroupArchive(const std::string group, const GroupArchive& archive, std::string& err);

	GroupReport getGroupReport(const std::string& group);
	int upateGroupReport(const std::string group, const GroupReport& report, std::string& err);

	GroupPermission getGroupPermission(const std::string& group);
	int updateGroupPerssion(const std::string group, const GroupPermission& permission, std::string& err);

	int updateAccounts(const std::string login, const AccountConfiguration& configuration, std::string& err);

	ConFeeder* getGlobalDatafeed(int& total);
	void releaseGlobalDatafeed(ConFeeder* feeder);
	ConAccess* getGlobalIPList(int& total);
	int getGlobalSymbols(std::vector<ConSymbol>& symbols, std::string& err);
	int getGlobalSymbols(std::string& symbol, ConSymbol& con, std::string& err);
	int updateSymbolsSessions(const std::string& symbol, const ConSessions cs[7], std::string& err);
	ConDataServer* getGlobalDCList(int& total);
	void releaseGlobalDCList(ConDataServer*);
	ConPluginParam* getGlobalPluginList(int& total);
	void releaseGlobalPluginList(ConPluginParam*);
	PerformanceInfo* getGlobalPerformance(const time_t from, int& total);
	void releaseGlobalPerformance(PerformanceInfo*);

	ConHoliday* getHoliday(int& total);
	void releaseHoliday(ConHoliday* ch);

	ConCommon getGlobalCommon();

	int setSymbolSwap(std::string& err, std::string symbol, int swap_long, int swap_short, int swap_enable=-1,int swap_rollover3days =-1);
	int updateOrderOpenPrice(int orderNo, double profit, std::string& err);
	bool getSymbolPrice(std::string symbol, double& bid, double& ask);

	int setSymbolTradeMode(std::string symbol, int mode, std::string& err);
	int getConSymbol(std::string symbol, ConSymbol& cs, std::string& err);
	int updateConSymbol(const ConSymbol cs);

	int tradeTransaction(TradeTransInfo* info, std::string& err);
	int updateTradeRecord(TradeRecord* info, std::string& err);
	int tradesGetBySymbol(const std::string& symbol, std::vector<TradeRecord>& record, std::string& err);
	int tradesGetByTicket(const std::string tick, std::vector<TradeRecord>& record, std::string& err);
	void tradesRelease(TradeRecord*& record);

	int chartInfoReq(const ChartInfo* chart, RateInfo*& ri, int& size, std::string& err);
	void releaseChartInfo(RateInfo*& chart);

	int chartInfoUpdate(const std::string symbol, const int period, int size, const RateInfo* rates, std::string& err);

	int getUserRecord(int login, UserRecord& ur, std::string& err);
	int updateUserRecord(const UserRecord& ur, std::string& err);
private:
	bool storeGroupsInfo(void* data, int type);
	bool storeSymbolsInfo(void* data, int type);
	bool storeUserInfo(void* data, int type);
	bool storePrices();
	/************************************************
	** Connection to a trading server
	** Arguments:
	**   host: trading server's ip and port, format like "localhost:443".
	** Returns:
	**   RET_OK(0): success
	**   RET_ERROR(2): Common error.
    **   RET_INVALID_DATA(3): Invalid information.
    **   RET_TECH_PROBLEM(4): Technical errors on the server.
    **   RET_OLD_VERSION(5): Old terminal version.
    **   RET_NO_CONNECT(6): No connection.
    **   RET_NOT_ENOUGH_RIGHTS(7): Not enough permissions to perform the operation.
    **   RET_TOO_FREQUENT(8): Too frequent requests.
	**   RET_MALFUNCTION(9): Operation cannot be completed.
	**   RET_GENERATE_KEY(10): Key generation is required.
    **   RET_SECURITY_SESSION(11): Connection using extended authentication.
	*************************************************/
	int mt4Conn(const char* host, CManagerInterface*& manager);

	/************************************************
    ** Authentication on the trading server using a manager account
    ** Arguments:
    **   login:  The login of the manager for connection.
    **   passwd:  The password of the manager for connection.
    ** Returns:
    **   true: success.
	**   false : failure.
    *************************************************/
	bool mt4Login(const int login, const char* passwd, CManagerInterface*& manager);

	bool heartBeat();
	bool mt4DirtIsConnected();

	bool createDirectConnToMT4(CManagerInterface*& manager);
	bool createPumpConnToMT4(CManagerInterface*& manager);

	static void __stdcall onPumpingFunc(int code, int type, void *data, void *param);
	bool switchToPumpMode(CManagerInterface*& managerInter);

	bool getProfitConverRate(std::string symbol, double& conv);

	int  pumpEvent(std::string& data, void* param);

	void expireCache();
	void forwardEvent(std::vector<std::string> data, int msgtype, int mt4type);
private:
	std::mutex         m_directMtx;
	CManagerInterface* m_directInter;
	CManagerInterface* m_pumpInter;
	CManagerFactory    m_factoryInter;
	std::map<std::string, ConGroup> m_GroupsInfo;
	std::map<std::string, ConSymbol> m_SymbolsInfo;
	std::map<std::string, std::array<double,2> > m_Quotes;
	MemCache      m_memCache;
	RedisClient   m_redis;

	std::mutex                    m_pumpCacheMtx;
	std::map<std::string, time_t> m_pumpCache;
	std::thread                   m_tdExpire;
	bool                          m_stop = false;
	std::unordered_map<std::string, std::string> m_groupCache;


	std::vector<std::thread>      m_monitorConn;
};

