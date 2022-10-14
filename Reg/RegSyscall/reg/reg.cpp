#include <winternl.h>
#include "sycall.h"
#include <stdio.h>
#include <windows.h>
#include <iostream>
using namespace std;

int main()
{
	HANDLE createdKey = nullptr;
	UNICODE_STRING newKeyName;
	OBJECT_ATTRIBUTES attributes;
	WCHAR name[] = L"\\REGISTRY\\Users\\S-1-5-21-1644813252-2960985308-3220584533-1000\\SOFTWARE";
	NTSTATUS opStatus = NULL;
	ULONG keyDispositionValue = NULL;
	newKeyName.Buffer = name;
	newKeyName.Length = sizeof(name) - sizeof(WCHAR);
	newKeyName.MaximumLength = wcslen(name);
	InitializeObjectAttributes(&attributes, &newKeyName, 0x00000040L, NULL, NULL);
	opStatus = NtCreateKey(&createdKey, KEY_ALL_ACCESS, &attributes, 0, NULL, REG_OPTION_NON_VOLATILE, &keyDispositionValue);
	cout << GetLastError() << endl;
	if (opStatus!=0) {
		if (keyDispositionValue == REG_CREATED_NEW_KEY)
			cout << "New key created!\n";
		else if (keyDispositionValue == REG_OPENED_EXISTING_KEY)
			cout << "Key already exists!\n";
		if (!CloseHandle(createdKey))
			cout << "Error closing handle!";
	}

}