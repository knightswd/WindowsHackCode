#include<windows.h>
#include<iostream>
#include <userenv.h>
#include <ntdef.h>
#define UNLEN       256 

using namespace std;
/*
* 通过文件路径来获取进程pid
*GetPidUsingFilePath(
*	LPCWSTR processBinaryPath具体进程路径
*)	return DWORD PID 返回进程pid
*/

DWORD GetPidUsingFilePath(LPCWSTR processBinaryPath) {
	DWORD retPid = 0;
	IO_STATUS_BLOCK iosb;
	HANDLE hFile;
	PFILE_PROCESS_IDS_USING_FILE_INFORMATION pfpiufi = NULL;
	int FileProcessIdsUsingFileInformation = 47;
	ULONG pfpiufiLen = 0;
	PULONG_PTR processIdListPtr = NULL;
	NTSTATUS status = 0;
	pNtQueryInformationFile NtQueryInformationFile = (pNtQueryInformationFile)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQueryInformationFile");
	hFile = CreateFile(processBinaryPath, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		pfpiufiLen = 8192;
		pfpiufi = (PFILE_PROCESS_IDS_USING_FILE_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pfpiufiLen);
		status = NtQueryInformationFile(hFile, &iosb, pfpiufi, pfpiufiLen, (FILE_INFORMATION_CLASS)FileProcessIdsUsingFileInformation);
		while (status == STATUS_INFO_LENGTH_MISMATCH) {
			pfpiufiLen = pfpiufiLen + 8192;
			pfpiufi = (PFILE_PROCESS_IDS_USING_FILE_INFORMATION)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pfpiufi, pfpiufiLen);
			status = NtQueryInformationFile(hFile, &iosb, pfpiufi, pfpiufiLen, (FILE_INFORMATION_CLASS)FileProcessIdsUsingFileInformation);
		}
		processIdListPtr = pfpiufi->ProcessIdList;
		// we return only the first pid, it's usually the right one
		if (pfpiufi->NumberOfProcessIdsInList >= 1)
			retPid = *processIdListPtr;
		HeapFree(GetProcessHeap(), 0, pfpiufi);
		CloseHandle(hFile);
	}
	return retPid;
}