#pragma once
#include <string>
#include <set>
#include <vector>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include "MT4ManagerAPI.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "openssl/md5.h"

struct GroupCommon
{
	std::string group = "";
	int enable = 0;
	int otp_mode = 0;
	std::string company = "";
	std::string support_page = "";
	double default_deposit = 0.0;
	std::string currency = "";
	int default_leverage = 0;
	double interestrate = 0.0;
};

struct GroupMargin
{
	int margin_call = 0;
	int margin_stopout = 0;
	int margin_type = 0;
	int margin_mode = 0;
	double credit = 0.0;
	int stopout_skip_hedged = 0;
	int hedge_large_leg = 0;
};

struct GroupArchive
{
	int archive_period = 0;
	int archive_max_balance = 0;
	int archive_pending_period = 0;
};

struct GroupReport
{
	int reports = 0;
	std::string smtp_server = "";
	std::string template_path ="";
	std::string smtp_login ="";
	std::string smtp_passwd ="";
	std::string support_email = "";
	int copies = 0;
	std::string signature = "";
};

struct GroupPermission
{
	int timeout = 0;
	int news = 0;
	int news_language[8] = {0};
	int news_language_total = 0;
	int maxsecurities = 0;

	int use_swap = 0;
	int rights = 0;                      // Flags of group permissions
	int check_ie_prices = 0;             // Check the prices of request in the Instant Execution mode
	int maxpositions = 0;                // Limit of open positions
	int close_reopen = 0;                // Partial closure mode
	int hedge_prohibited = 0;            // Hedging not allowed
	int close_fifo = 0;                  // Enable closing by FIFO rule
	int unused_rights[2] = {0};          // A reserved field
	std::string securities_hash = "";    // Internal data
};

struct AccountConfiguration
{
	std::string password = "";
	int enable_change_password = 0;
};


class Utils
{
public:
	static Utils& getInstance();

	void  split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter);
	std::string getProjectPath();

	const std::string& getCmdDes(int cmd);
	const std::string& getStateDes(int state);
	const std::string& getReasonDes(int reason);
	const std::string& getActivationDes(int activation);

	bool parseFromJsonToSec(const std::string json, std::map<int,ConGroupSec>& sec, std::set<int>& index, std::string& group);
	bool parseFromSecToJson(const std::string& group, const ConGroupSec sec[],const int size, std::string& json);

	//actually ConGroupMargin means symbol's settings
	bool parseFromJsonToSymbol(const std::string json, std::map<std::string, ConGroupMargin>& margins, std::string& group);
	bool parseFromSymbolToJson(const std::string& group, const ConGroupMargin margins[],const int size, std::string& json);

	bool parseFromJsonToCommon(const std::string json, GroupCommon& common, std::string& group);
	bool parseFromCommonToJson(const std::string& group, const GroupCommon common, std::string& json);

	bool parseFromJsonToMargin(const std::string json, GroupMargin& margin, std::string& group);
	bool parseFromMarginToJson(const std::string& group, const GroupMargin& margin, std::string& json);

	bool parseFromCommmonSecuritesToJson(const ConSymbolGroup securites[],const int size, std::string& json);
	bool parseFromCommonGroupsToJson(const std::vector<std::string>& groups, std::string& json);

	bool parseFromArchiveToJson(const std::string& group, const GroupArchive& archive, std::string& json);
	bool parseFromJsonToArchive(const std::string json, GroupArchive& archive, std::string& group);

	bool parseFromReportToJson(const std::string& group, const GroupReport& report, std::string& json);
	bool parseFromJsonToReport(const std::string json, GroupReport& report, std::string& group);

	bool parseFromPermissionToJson(const std::string& group, const GroupPermission& permission, std::string& json);
	bool parseFromJsonToPermission(const std::string json, GroupPermission& permission, std::string& group);

	bool parseFromJsonToAccuntConfiguration(const std::string& json, AccountConfiguration& configuration, std::string& login);

	bool parseFromDatafeedToJson(const ConFeeder[], const int size, std::string& json);
	bool parseFromGlobalCommonToJson(const ConCommon&, std::string& json);
	bool parseFromIPListToJson(const ConAccess[], const int size, std::string& json);
	bool parseFromSymbolsListToJson(const std::vector<ConSymbol>&, std::string& json);
	bool parseFromSymbolToJson(const ConSymbol& symbol, std::string& json);
	bool parseFromJsonToSymbol(const std::string& json, std::string& symbol);
	bool parseFromDCListToJson(const ConDataServer[], const int size, std::string& json);
	bool parseFromPluginListToJson(const ConPluginParam[], const int size, std::string& json);
	bool parseFromPerformanceToJson(const PerformanceInfo[], const int size, std::string& json);
	bool parseFromJsonToPerformance(const std::string& json, unsigned int &seconds);

	bool parseFromHolidayToJson(const ConHoliday[], const int size, std::string& json);
	bool parseFromJsonToSession(const std::string& json, ConSessions(&)[7], std::string& symbol);

	bool parseFromJsonToSwap(const std::string& body, std::string& symbol, int& swap_long, int& swap_short, int& swap_enable, int& swap_rollover);
	bool parseFromJsonToOpenPrice(const std::string& body, int& orderNo , double& profit);

	std::string serializeConSymbolToJson(int code, int type, const ConSymbol& cs);
	bool unSerializeConSymbolToJson(const std::string& body, ConSymbol& cs);

	bool unSerializeTradeTransInfo(const std::string& body, TradeTransInfo& tti);
	bool unSerializeTradeRecord(const std::string& body, TradeRecord& tri);
	bool serializeTradeRecord(std::vector<TradeRecord> record, std::string& out);

	bool unSerializeChartInfo(const std::string& body, ChartInfo& ci);
	std::string serializeRateInfo(RateInfo* ri, int size);

	bool unSerializeChartupdate(const std::string& body, std::string& symbol, int& period, int& size, RateInfo*& rates);

	std::string serializePumpSymbols(const std::vector<std::string> symbols);
	std::string serializePumpUsers(const std::vector<std::string> users);
	std::string serializePumpGroups(const std::vector<std::string> groups);
	std::string serializeConGroup(const ConGroup& group);
	std::string serializeConGroupMargin(const ConGroupMargin& margin);
	std::string serializeConGroupSec(const  ConGroupSec& sec);

	std::string Md5(std::string data);
private:
	Utils();
	~Utils() = default;

	Utils(const Utils& other) = default;
	Utils& operator= (const Utils& other) = default;

	bool addInt(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, int& value);
	bool addInt64(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, long& value);
	bool addDouble(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, double& value);
	bool addString(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, std::string& value);
	bool addString(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, char* value, int len);
	bool addIntArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, int* value);
	bool addDoubleArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, double* value);
	bool addConSession(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, ConSessions* session, int size);
	bool addRateInfoArray(rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &obj, std::string key, RateInfo*& rates, int size);
	
	bool intToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, int value);
	bool doubleToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, double value);
	bool stringToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, std::string value);
	bool charArrToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string key, char* value);
private:
	static Utils* m_self;
	static std::mutex m_mtx;

	std::unordered_map<int, std::string> m_cmd;
	std::unordered_map<int, std::string> m_state;
	std::unordered_map<int, std::string> m_reason;
	std::unordered_map<int, std::string> m_activation;
};
