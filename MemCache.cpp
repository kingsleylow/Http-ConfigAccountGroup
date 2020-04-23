#include "MemCache.h"


bool MemCache::add(std::string data)
{
	std::unique_lock<std::mutex> lock(m_mtxCache);
	if (data.empty())
		return false;
	m_Cache.push(data);
	m_condCache.notify_one();
	return true;
}

void MemCache::startEventLoop()
{
	m_thread = std::thread([this] {
		while (true)
		{
			std::string data;
			{
				std::unique_lock<std::mutex> lock(m_mtxCache);
				m_condCache.wait(lock, [this] {return this->m_stop || !this->m_Cache.empty(); });
				if (m_stop && m_Cache.empty())
					break;
				data = m_Cache.front();
				m_Cache.pop();
			}
			m_fCb(data, m_fArgs);
		}});
	m_thread.detach();
}

void MemCache::stopEventLoop()
{
	m_stop = true;
}

void MemCache::init(CallBack cb, void* param)
{
	m_stop = false;
	m_fCb = cb;
	m_fArgs = param;
}