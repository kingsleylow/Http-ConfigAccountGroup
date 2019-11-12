#include <iostream>
#include "HttpServer.h"
#include "Logger.h"
#include "Config.h"
#include "MT4Module.h"
#include <iostream>
#include <Windows.h>
#define MY_DEBUG

#define SLEEP_TIME 1000
SERVICE_STATUS servicestatus;
SERVICE_STATUS_HANDLE hstatus;

void WINAPI CtrlHandler(DWORD request);
int start_service();
bool brun;

void WINAPI ServiceMain(int argc, char** argv)
{
	servicestatus.dwServiceType = SERVICE_WIN32;

	servicestatus.dwCurrentState = SERVICE_START_PENDING;

	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;

	servicestatus.dwWin32ExitCode = 0;

	servicestatus.dwServiceSpecificExitCode = 0;

	servicestatus.dwCheckPoint = 0;

	servicestatus.dwWaitHint = 0;

	hstatus = ::RegisterServiceCtrlHandler("DealerService", CtrlHandler);

	if (hstatus == 0)
	{
		return;
	}

	servicestatus.dwCurrentState = SERVICE_RUNNING;

	SetServiceStatus(hstatus, &servicestatus);

	if (0 == start_service()) {
		return;
	}

	while (!brun) {
		Sleep(SLEEP_TIME);
	}
}

void WINAPI CtrlHandler(DWORD request)
{

	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		brun = true;
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		brun = true;
		break;
	default:
		break;
	}

	SetServiceStatus(hstatus, &servicestatus);
}

int start_service()
{
	Config::getInstance().readConf("config/app.conf");
	MT4Conn mt4Conn;
	if (!mt4Conn.createConnToMT4())
	{
		Logger::getInstance()->error("connect to mt4 failed.");
		std::cout << "connect to mt4 failed." << std::endl;
		return -1;
	}
	else
	{
		Logger::getInstance()->info("connect to mt4 success.");
		std::cout << "connect to mt4 success." << std::endl;
	}

	HttpServer http;
	if (!http.initServerHttp())
	{
		Logger::getInstance()->error("http server init failed.");
		std::cout << "http server init failed." << std::endl;
		return -1;
	}
	else
	{
		Logger::getInstance()->info("http server init success.");
		std::cout << "http server init success." << std::endl;
	}
	http.setMT4Conn(&mt4Conn);
	mt4Conn.watchConntoMT4();
	std::cout << "running..." << std::endl;
	http.startServer();
	return 1;
}


int main(int argc, char *argv[]) {

#ifdef MY_DEBUG
	start_service();
	system("pause");
#else
	SERVICE_TABLE_ENTRY entrytable[2];

	entrytable[0].lpServiceName = "DealerService";

	entrytable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	entrytable[1].lpServiceName = NULL;

	entrytable[1].lpServiceProc = NULL;

	StartServiceCtrlDispatcher(entrytable);
#endif
	return 0;
}