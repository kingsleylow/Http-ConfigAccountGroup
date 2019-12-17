#pragma once
#include "MT4ManagerAPI.h"
#include <mutex>
#include <unordered_map>


enum ORDERSTATE { READY, PROCESSING, FAILED, SUCCESS };

class Common 
{
public:
	~Common();
	static Common* getInstance();
	std::string getReason(int reason);
	std::string getType(int type);
	std::string getCmd(int cmd);
	std::string getState(int state);
	std::string getTradeType(int tradeType);
	std::string getActivation(int activation);
	int64_t     getCurMilliSeconds();
	int64_t     getCurSeconds();

	std::string getProjectPath();
	//print info
	void PrintOrderRequest(const RequestInfo& info, std::string des = "");
	void PrintTradeRecord(const TradeRecord& record);
	void PrintTradeTransInfo(const TradeTransInfo& info, std::string des = "");

	std::string genFilterKey(const TradeRecord& record, int triggerType = 0);
	void deserializePluginTRFromJson(std::string json, TradeRecord& record, int& type);
	std::string serializeMT4TRToJson(int code, int type, const TradeRecord& record);
	void deserializeMT4TRFromJson(std::string json, TradeRecord& record, int& code, int& type);
	std::string MD5(std::string data);

	void  split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter=";");

	std::string serializeReqToJson(RequestInfo& qi, bool backup);
	void deserializeReqFromJson(std::string json, RequestInfo& qi, bool& backup);
private:
	Common();
	Common(Common& other) = delete;
	Common& operator()(Common& other) = delete;

private:
	static Common*     m_self;
	static std::mutex  m_mtxSingleton;

	std::unordered_map<int, std::string> m_type;
	std::unordered_map<int, std::string> m_reason;
	std::unordered_map<int, std::string> m_cmd;
	std::unordered_map<int, std::string> m_state;
	std::unordered_map<int, std::string> m_tradeType;
	std::unordered_map<int, std::string> m_activation;
};