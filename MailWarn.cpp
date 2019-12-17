#include "MailWarn.h"
#include "Config.h"
#include "Logger.h"

MailWarn* MailWarn::m_self = nullptr;
std::mutex MailWarn::m_mtx;

MailWarn* MailWarn::getInstance()
{
	if (nullptr == m_self)
	{
		std::lock_guard<std::mutex> lgd(m_mtx);
		if (nullptr == m_self)
		{
			m_self = new MailWarn;
		}
	}
	return m_self;
}

MailWarn::~MailWarn()
{
}

MailWarn::MailWarn()
{
}

void MailWarn::SendWarnMail(const std::string &title, const std::string &content)
{
	if (_stricmp(Config::getInstance().getEmailConf().find("enable")->second.c_str(), "NO") == 0)
		return;

	std::string addr;
	for (auto &a : Config::getInstance().getEmailConf().find("addr")->second)
	{
		addr += a + ",";
	}
	addr = addr.substr(0,addr.find_last_of(','));

	std::string param = "email=" + addr + "&" + "title=" + title + "&" + \
		"content=" + Config::getInstance().getEmailConf().find("env")->second + ":" + content;

	CURLcode res = curl_post_req(Config::getInstance().getEmailConf().find("server")->second, param);

	if (res != CURLE_OK) {
		Logger::getInstance()->error("curl_easy_perform failed, error des:{}", std::string(curl_easy_strerror(res)));
	}
}

// http POST
CURLcode MailWarn::curl_post_req(const std::string &url, const std::string &postParams)
{
	// init curl
	CURL *curl = curl_easy_init();
	// res code
	CURLcode res;
	if (curl)
	{
		// set params
		curl_easy_setopt(curl, CURLOPT_POST, 1); // post req
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str()); // params
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postParams.size());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		// start req
		res = curl_easy_perform(curl);

		// release curl
		curl_easy_cleanup(curl);
	}

	return res;
}