#pragma once
#include "pch.h"

void killLoader()
{
	HANDLE currentProcess = GetCurrentProcess();
	// Kết thúc quá trình với mã lỗi 0 (thành công)
	TerminateProcess(currentProcess, 0);
	system("taskkill /f /im Client-Win64-Shipping.exe");
}

int FindProcessId(const std::string& processName)
{
	int pid = -1;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);

	if (Process32First(snapshot, &process))
	{
		do
		{
			if (std::string(process.szExeFile) == processName)
			{
				pid = process.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);

	return pid;
}

void WaitForCloseProcess(const std::string& processName)
{
	int pid = FindProcessId(processName);
	if (pid == -1)
		return;

	std::cout << "Da tim thay tien trinh '" << processName << "' Dang cho dong..." << std::endl;

//#ifdef _DEBUG
//	std::stringstream stream;
//	stream << "taskkill /F /T /IM " << processName;
//	int retval = system(stream.str().c_str());
//
//	std::cout << "Trying to kill process." << std::endl;
//#endif

	HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	DWORD exitCode = 0;
	while (hProc && (GetExitCodeProcess(hProc, &exitCode), exitCode == STILL_ACTIVE)) {
		Sleep(1000);
	}

	if (hProc != NULL)
		CloseHandle(hProc);
}