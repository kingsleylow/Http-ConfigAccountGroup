#include "RedisClient.h"
#include "Logger.h"
#include <iostream>
#include <cpp_redis/cpp_redis>

void RedisClient::init(std::string channel)
{
	m_channel = channel;
	m_thread = std::thread(&RedisClient::publishMsg, this);
	m_thread.detach();
	m_redis = new(cpp_redis::client);
}

bool RedisClient::connectToServer(std::string host, int port, std::string pass)
{
	m_host = host;
	m_port = port;
	m_pass = pass;

	auto lamda = [this](const std::string& host, std::size_t port, cpp_redis::client::connect_state status)
	{
		if (status == cpp_redis::client::connect_state::failed)
		{
			Logger::getInstance()->error("client disconnected from {}:{}", host, port);
			this->m_cond.notify_all();
			return false;
		}
		else if (status == cpp_redis::client::connect_state::ok)
		{
			Logger::getInstance()->info("client connect to {}:{} success.", host, port);
			return true;
		}
		else
		{
			return false;
		}
	};

	try
	{
		std::lock_guard<std::mutex> lock(m_mtxConn);
		((cpp_redis::client*)m_redis)->connect(host, port, lamda, 2000, 5, 500);

		auto reply = ((cpp_redis::client*)m_redis)->auth(pass);
		((cpp_redis::client*)m_redis)->commit();
		bool ok = reply.get().ok();
		if (!ok)
		{
			Logger::getInstance()->error("auth to redis failed.");
			return false;
		}
	}
	catch (std::exception& e)
	{
		Logger::getInstance()->error("redis connect {} failed. {}", host, e.what());
		return false;
	}

	m_monitorThd = std::thread(&RedisClient::monitorConn, this);
	m_monitorThd.detach();
	return true;
}

void RedisClient::publishMsg()
{
	std::string data;
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(m_mtxMsg);
			m_condMsg.wait(lock, [this] {return m_stop || !m_queMsg.empty(); });
			if (m_stop)
				break;
			data = m_queMsg.front();
			m_queMsg.pop();
		}
		try
		{
			std::lock_guard<std::mutex> lock(m_mtxConn);
			auto reply = ((cpp_redis::client*)m_redis)->publish(m_channel, data);
			((cpp_redis::client*)m_redis)->commit();
			bool ok = reply.get().ok();
			if (!ok)
			{
				Logger::getInstance()->error("send to redis error.");
			}
		}
		catch(std::exception& e)
		{
			Logger::getInstance()->error("{}", e.what());
		}
	}
	
}


void RedisClient::enqueue(std::string msg)
{
	std::lock_guard<std::mutex> lock(m_mtxMsg);
	m_queMsg.push(msg);
	m_condMsg.notify_one();
}

void RedisClient::monitorConn()
{
	while (true)
	{
		if (m_stop)
			break;

		{
			std::lock_guard<std::mutex> lock(m_mtxConn);
			if (!((cpp_redis::client*)m_redis)->is_connected())
			{
				connectToServer(m_host, m_port, m_pass);
			}
		}
		
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
}