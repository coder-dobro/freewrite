#include <Windows.h>
#include <TlHelp32.h>

class Hackprocess
{
public:

	PROCESSENTRY32 gameProcess;
	HANDLE HandleProcess;
	HWND HWNDCsgo;
	DWORD dwordClient;
	DWORD dwordEngine;
	DWORD dwordOverlay;
	DWORD dwordVGui;
	DWORD dwordServer;
	DWORD dwordLibCef;
	DWORD dwordSteam;
	DWORD FindProcessName(const char *ProcessName, PROCESSENTRY32 *pEntry)
	{
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;        if (!Process32First(hSnapshot, &ProcessEntry))
		{
			CloseHandle(hSnapshot);
			return 0;
		}
		do {
			if (!_strcmpi(ProcessEntry.szExeFile, ProcessName))
			{
				memcpy((void *)pEntry, (void *)&ProcessEntry, sizeof(PROCESSENTRY32));
				CloseHandle(hSnapshot);
				return ProcessEntry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &ProcessEntry));
		CloseHandle(hSnapshot);
		return 0;
	}

	DWORD getThreadByProcess(DWORD DwordProcess)
	{
		THREADENTRY32 ThreadEntry;
		ThreadEntry.dwSize = sizeof(THREADENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

		if (!Thread32First(hSnapshot, &ThreadEntry)) { CloseHandle(hSnapshot); return 0; }

		do {
			if (ThreadEntry.th32OwnerProcessID == DwordProcess)
			{
				CloseHandle(hSnapshot);
				return ThreadEntry.th32ThreadID;
			}
		} while (Thread32Next(hSnapshot, &ThreadEntry));
		CloseHandle(hSnapshot);
		return 0;
	}

	DWORD GetModuleNamePointer(LPSTR LPSTRModuleName, DWORD DwordProcessId)
	{
		MODULEENTRY32 lpModuleEntry = { 0 };
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, DwordProcessId);
		if (!hSnapShot)
			return NULL;
		lpModuleEntry.dwSize = sizeof(lpModuleEntry);
		BOOL RunModule = Module32First(hSnapShot, &lpModuleEntry);
		while (RunModule)
		{
			if (!strcmp(lpModuleEntry.szModule, LPSTRModuleName))
			{
				CloseHandle(hSnapShot);
				return (DWORD)lpModuleEntry.modBaseAddr;
			}
			RunModule = Module32Next(hSnapShot, &lpModuleEntry);
		}
		CloseHandle(hSnapShot);
		return NULL;
	}

	void runSetDebugPrivs()
	{
		HANDLE HandleProcess = GetCurrentProcess(), HandleToken;
		TOKEN_PRIVILEGES priv;
		LUID LUID;
		OpenProcessToken(HandleProcess, TOKEN_ADJUST_PRIVILEGES, &HandleToken);
		LookupPrivilegeValue(0, "seDebugPrivilege", &LUID);
		priv.PrivilegeCount = 1;
		priv.Privileges[0].Luid = LUID;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(HandleToken, false, &priv, 0, 0, 0);
		CloseHandle(HandleToken);
		CloseHandle(HandleProcess);
		return;
	}

	void memoryType()
	{
		runSetDebugPrivs();
		while (!FindProcessName("csgo.exe", &gameProcess)) Sleep(12);
		while (!(getThreadByProcess(gameProcess.th32ProcessID))) Sleep(12);
		HandleProcess = OpenProcess(PROCESS_ALL_ACCESS, false, gameProcess.th32ProcessID);
		while (dwordClient == 0x0) dwordClient = GetModuleNamePointer("client.dll", gameProcess.th32ProcessID);
		while (dwordEngine == 0x0) dwordEngine = GetModuleNamePointer("engine.dll", gameProcess.th32ProcessID);
		while (dwordVGui == 0x0) dwordVGui = GetModuleNamePointer("vguimatsurface.dll", gameProcess.th32ProcessID);
		while (dwordServer == 0x0)dwordServer = GetModuleNamePointer("server.dll", gameProcess.th32ProcessID);
		HWNDCsgo = FindWindow(NULL, "Counter-Strike: Global Offensive");
		return;
	}
};

extern Hackprocess CSSource;