#include<windows.h>
#include<iostream>
#define UNLEN       256 

using namespace std;

/*
*设置进程权限
*setPrivilege（
*	HANDLE hToken 目标进程句柄 ，
*	LPCWSTR name  设置的进程权限名
*	return BOOL TRUE成功/FALSE失败
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