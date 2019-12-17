#pragma once

#include "curl/curl.h"
#include <mutex>

class MailWarn
{
public:
	~MailWarn();
	static MailWarn* getInstance();

	void SendWarnMail(const std::string &title, const std::string &content);
private:
	MailWarn();
	MailWarn(const MailWarn& other)= delete;
	MailWarn(MailWarn&& other) = delete;

	MailWarn& operator=(const MailWarn& other) = delete;
	MailWarn& operator=(MailWarn&& other) = delete;

	CURLcode curl_post_req(const std::string &url, const std::string &postParams);

private:
	static MailWarn* m_self;
	static std::mutex m_mtx;
};