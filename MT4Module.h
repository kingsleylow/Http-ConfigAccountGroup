#pragma once
#include "MT4ManagerAPI.h"
#include "Config.h"
#include "Utils.h"
#include <set>
#include <map>

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

	void watchConntoMT4();

	ConGroup getGroupCfg(const std::string& group);
	bool updateGroupSec(const std::string& group, const std::map<int, ConGroupSec>& cfgGroupSec,std::set<int> index);
	
	bool updateGroupSymbol(const std::string& group, const std::map<std::string, ConGroupMargin>& cfgGroupMargin);

	GroupCommon getGroupCommon(const std::string& group);
	bool updateGroupCommon(const std::string& group, const GroupCommon& common);

	bool getGroupNames(std::vector<std::string>& groups);
	int getSecuritiesNames(ConSymbolGroup securities[]); //return value:0 success. otherwise failed.

	GroupMargin getGroupMargin(const std::string& group);
	bool updateGroupMargin(const std::string group, const GroupMargin& margin);

	GroupArchive getGroupArchive(const std::string& group);
	bool updateGroupArchive(const std::string group, const GroupArchive& archive);

	GroupReport getGroupReport(const std::string& group);
	bool upateGroupReport(const std::string group, const GroupReport& report);

	GroupPermission getGroupPermission(const std::string& group);
	bool updateGroupPerssion(const std::string group, const GroupPermission& permission);

	bool updateAccounts(const std::string login, const AccountConfiguration& configuration);

	ConFeeder* getGlobalDatafeed(int& total);
	void releaseGlobalDatafeed(ConFeeder* feeder);
	ConAccess* getGlobalIPList(int& total);
	bool getGlobalSymbols(std::vector<ConSymbol>& symbols);
	bool getGlobalSymbols(std::string& symbol, ConSymbol& con);
	bool updateSymbolsSessions(const std::string& symbol, const ConSessions cs[7]);
	ConDataServer* getGlobalDCList(int& total);
	void releaseGlobalDCList(ConDataServer*);
	ConPluginParam* getGlobalPluginList(int& total);
	void releaseGlobalPluginList(ConPluginParam*);
	PerformanceInfo* getGlobalPerformance(const time_t from, int& total);
	void releaseGlobalPerformance(PerformanceInfo*);

	ConHoliday* getHoliday(int& total);
	void releaseHoliday(ConHoliday* ch);

	ConCommon getGlobalCommon();

	bool setSymbolSwap(std::string symbol, int swap_long, int swap_short, int swap_enable=-1,int swap_rollover3days =-1);
	bool updateOrderOpenPrice(int orderNo, double profit);
	bool getSymbolPrice(std::string symbol, double& bid, double& ask);
private:
	bool storeGroupsInfo();
	bool storeSymbolsInfo();
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
	int mt4Conn(const char* host, CManagerInterface* manager);

	/************************************************
    ** Authentication on the trading server using a manager account
    ** Arguments:
    **   login:  The login of the manager for connection.
    **   passwd:  The password of the manager for connection.
    ** Returns:
    **   true: success.
	**   false : failure.
    *************************************************/
	bool mt4Login(const int login, const char* passwd, CManagerInterface* manager);

	/************************************************
	** create interface of manager api
	** Arguments:
	**   mt4LibPath: MT4 lib's path
	** Returns:
	**   true: success
	**   false: failure
	*************************************************/
	bool mt4Init();

	bool heartBeat();
	bool mt4DirtIsConnected();

	bool createDirectConnToMT4(CManagerInterface* manager);
	bool createPumpConnToMT4(CManagerInterface* manager);

	static void __stdcall onPumpingFunc(int code, int type, void *data, void *param);
	bool switchToPumpMode(CManagerInterface* managerInter);

	bool getProfitConverRate(std::string symbol, double& conv);
private:
	CManagerInterface* m_directInter;
	CManagerInterface* m_pumpInter;
	CManagerFactory    m_factoryInter;
	std::map<std::string, ConGroup> m_GroupsInfo;
	std::map<std::string, ConSymbol> m_SymbolsInfo;
	std::map<std::string, std::array<double,2> > m_Quotes;
};

