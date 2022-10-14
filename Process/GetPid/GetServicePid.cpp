#include<windows.h>
#include<iostream>
#include <userenv.h>
#define UNLEN       256 

using namespace std;



/*
*通过服务名获取PID
*GetServicePid（
*	LPCWSTR ServiceName 具体的服务名
*）	 return DWORD PID 返回进程pid
*/

DWORD GetServicePid(LPCWSTR ServiceName)
{
	const SC_HANDLE controlManagerHandle = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
	if (nullptr == controlManagerHandle)
		throw runtime_error("Connecting to Service Control Manager failed");

	const SC_HANDLE serviceHandle = OpenServiceW(controlManagerHandle, ServiceName, SERVICE_QUERY_STATUS);
	CloseServiceHandle(controlManagerHandle);
	if (nullptr == serviceHandle)
		throw runtime_error("Opening service handle failed");

	SERVICE_STATUS_PROCESS procInfo;
	DWORD bytesNeeded;
	if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&procInfo), sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded))
	{
		CloseServiceHandle(serviceHandle);
		throw runtime_error("Querying service status failed");
	}

	CloseServiceHandle(serviceHandle);
	return procInfo.dwProcessId;
}
