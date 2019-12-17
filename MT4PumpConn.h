#pragma once
#include "MT4ManagerAPI.h"
#include "Config.h"
#include <mutex>
#include <map>
#include <unordered_map>
#include <functional>

class PumpConn;
struct Args
{
	CManagerInterface* inter;
	PumpConn*          pump;

	Args& operator=(const Args& other)
	{
		if (this != &other)
		{
			this->inter = other.inter;
			this->pump = other.pump;

		}

		return *this;
	}
};

class PumpConn
{
public:
	~PumpConn();
	static PumpConn* getInstance();
public:
	bool init();
	bool createConnToMT4();
	
	bool getUserRecord(const int login, UserRecord& ur);
	bool getQuotes(std::string symbol, double& bid, double& ask);
	bool getConSymbol(std::string symbol, ConSymbol& cs);
	bool getSymbolInfo(std::string symbol, SymbolInfo& si);
	bool getConGroup(std::string group, ConGroup& cg);
	bool symbolAdd(std::string symbol);
	TradeRecord* tradesGetByLogin(const int login, std::string group, int& total);
private:
	PumpConn();
	PumpConn(const PumpConn& other) = default;
	PumpConn& operator=(const PumpConn& other) = default;

	static	void __stdcall NotifyCallBack(int code, int type, void* data, void *param);
	int mt4Conn(const char* host, CManagerInterface*& inter);
	bool mt4Login(const int login, const char* passwd, CManagerInterface*& inter);
	bool mt4Init(CManagerInterface*& inter);
	bool createConnToMT4(CManagerInterface*& inter);
	bool mt4Release();
	void monitorConn();

	void storeQuotes(SymbolInfo *si, int size);
	void storeConSymbol(ConSymbol *cs, int size);
	void storeConGroup(ConGroup* cg, int size);
private:
	std::map<CManagerInterface*, Args> m_inter;
	CManagerInterface* m_managerInter;
	CManagerInterface* m_managerInterBak;
	CManagerFactory    m_factoryInter;
	static std::mutex  m_mtxSendData;
	bool               m_stop;
	 
	static std::mutex     m_mtxSingleton;
	static PumpConn*      m_self;

	std::mutex                                  m_mtxQuotes;
	std::unordered_map<std::string, SymbolInfo> m_quotes;

	std::mutex                                  m_mtxSymbol;
	std::unordered_map<std::string, ConSymbol>  m_conSymbol;

	std::mutex                                  m_mtxGroup;
	std::unordered_map<std::string, ConGroup>   m_conGroup;

	std::thread                                 m_thdMonitor;
	std::mutex                                  m_mtxPump;
};

