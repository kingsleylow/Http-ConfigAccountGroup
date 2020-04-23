#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <functional>
#include <thread>

using CallBack = std::function<int(std::string& data, void* param)>;
class MemCache
{
public:
	MemCache() = default;
	~MemCache() = default;

public:
	void init(CallBack cb, void* param);
	bool add(std::string data);
	void startEventLoop();
	void stopEventLoop();

private:
	CallBack                  m_fCb;
	void*                     m_fArgs;
	std::queue<std::string>   m_Cache;
	std::mutex                m_mtxCache;
	std::condition_variable   m_condCache;
	bool                      m_stop = false;
	std::thread               m_thread;
};