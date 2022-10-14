#include<windows.h>
#include<iostream>
#define UNLEN       256 

using namespace std;

BOOL changeReg() {
	//check reg 
	HKEY hRoot = HKEY_LOCAL_MACHINE;
	WCHAR szSubKey[UNLEN] = L"SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps";
	HKEY hKey;

	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	LONG lRet = RegCreateKeyExW(hRoot, szSubKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (lRet == ERROR_SUCCESS)
	{
		wcout << L"reg create ok!" << endl;
	}

	DWORD type = 2;
	lRet = RegSetValueExW(hKey, L"DumpType", 0, REG_DWORD, (BYTE*)&type, sizeof(type));
	if (lRet == ERROR_SUCCESS)
	{
		wcout << L"reg set ok!" << endl;
		return TRUE;
	}
	RegCloseKey(hKey);
	return FALSE;
}