#pragma once

#include <string>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

class RedisClient
{
public:
	void init(std::string channel);
	bool connectToServer(std::string host, int port, std::string pass);
	void enqueue(std::string msg);
private:
	void publishMsg();
	void monitorConn();
private:
	void* m_redis;
	std::mutex m_mutex;
	std::condition_variable m_cond;

	std::mutex  m_mtxConn;
	std::string m_host;
	int m_port;
	std::string m_pass;
	std::thread m_monitorThd;


	std::string m_channel;
	std::queue<std::string> m_queMsg;
	std::mutex m_mtxMsg;
	std::condition_variable m_condMsg;
	bool m_stop = false;;
	std::thread m_thread;
};