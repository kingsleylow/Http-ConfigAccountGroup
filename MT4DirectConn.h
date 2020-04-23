#pragma once
#include "MT4ManagerAPI.h"
#include "Config.h"
#include <mutex>
#include <map>
#include <unordered_map>

class DirectConn
{
public:
	~DirectConn();
	static DirectConn* getInstance();
public:
	bool init();
	bool createConnToMT4();
	
	bool getConSymbol(std::string symbol, ConSymbol& cs);
	bool getConGroup(std::string group, ConGroup& cg);
	bool marginLevelRequest(int login, MarginLevel& ml);
	bool getUserRecord(int login, UserRecord& ur);
	TradeRecord* TradesRequest(std::string loginList, int open_only, int& total);
	void tradesRelease(TradeRecord* tr);
	bool tradeTransaction(TradeTransInfo* info, int &res);
	bool setConSymbolTradeMode(std::string& symbol, int mode);
private:
	DirectConn();
	DirectConn(const DirectConn& other) = default;
	DirectConn& operator=(const DirectConn& other) = default;

	bool mt4IsConnected();
	void monitorConn();

	int mt4Conn(const char* host, CManagerInterface*& inter);
	bool mt4Login(const int login, const char* passwd, CManagerInterface*& inter);
	bool mt4Init(CManagerInterface*& inter);
	bool createConnToMT4(CManagerInterface*& inter);
	bool mt4Release();

	bool storeConSymbol();
	bool storeConGroup();
private:
	CManagerInterface* m_managerInter;
	CManagerInterface* m_managerInterBak;
	CManagerFactory    m_factoryInter;
	static std::mutex  m_mtxSendData;
	bool               m_stop;

	static std::mutex     m_mtxSingleton;
	static DirectConn*    m_self;

	std::thread           m_thdMonitor;
	std::mutex            m_mtxDirect;

	std::mutex m_mtxSymbols;
	std::mutex m_mtxGroups;
	std::unordered_map<std::string, ConSymbol> m_symbols;
	std::unordered_map<std::string, ConGroup>  m_groups;
};