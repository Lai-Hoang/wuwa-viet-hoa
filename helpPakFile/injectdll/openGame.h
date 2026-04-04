#pragma once
#include "pch.h"

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}


std::optional<std::string> SelectDirectory(const char* title)
{
	auto currPath = std::filesystem::current_path();

	if (!SUCCEEDED(CoInitialize(nullptr)))
		return {};

	IFileDialog* pfd;
	if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		return {};

	const size_t titleSize = strlen(title) + 1;
	wchar_t* wcTitle = new wchar_t[titleSize];
	mbstowcs(wcTitle, title, titleSize);

	DWORD dwOptions;
	IShellItem* psi;
	if (!SUCCEEDED(pfd->GetOptions(&dwOptions)) ||
		!SUCCEEDED(pfd->SetOptions(dwOptions | FOS_PICKFOLDERS)) ||
		!SUCCEEDED(pfd->SetTitle(wcTitle)) ||
		!SUCCEEDED(pfd->Show(NULL)) ||
		!SUCCEEDED(pfd->GetResult(&psi)))
	{
		pfd->Release();
		return {};
	}

	WCHAR* folderName;
	if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &folderName)))
	{
		pfd->Release();
		psi->Release();
		return {};
	}

	pfd->Release();
	psi->Release();

	std::filesystem::current_path(currPath);

	std::u16string u16(reinterpret_cast<const char16_t*>(folderName));
	return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
}

std::optional<std::string> SelectFile(const char* filter, const char* title)
{
	auto currPath = std::filesystem::current_path();

	// common dialog box structure, setting all fields to 0 is important
	OPENFILENAME ofn = { 0 };
	TCHAR szFile[260] = { 0 };

	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.lpstrTitle = title;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	std::optional<std::string> result = {};
	if (GetOpenFileName(&ofn) == TRUE)
		result = std::string(szFile);

	std::filesystem::current_path(currPath);
	return result;
}

std::optional<std::string> GetOrSelectPath(CSimpleIni& ini, const char* section, const char* name, const char* friendName, const char* filter)
{

	auto savedPath = ini.GetValue(section, name);
	if (savedPath != nullptr)
		return std::string(savedPath);

	//LOG_DEBUG("%s path not found. Please point to it manually.", friendName);
	printf("duong dan %s khong ton tai. Vui long tim va chon Client-Win64-Shipping.exe\n", friendName);

	auto titleStr = string_format("Select %s", friendName);
	auto selectedPath = filter == nullptr ? SelectDirectory(titleStr.c_str()) : SelectFile(filter, titleStr.c_str());
	if (!selectedPath)
		return {};

	ini.SetValue(section, name, selectedPath->c_str());
	return selectedPath;
}



bool OpenGameProcess(HANDLE* phProcess, HANDLE* phThread, const char* additionalParam)
{
	HANDLE hToken;
	BOOL TokenRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

	if (!TokenRet)
	{
		//LOG_LAST_ERROR("Privilege escalation failed!");
		printf("Privilege escalation failed!\n");
		return false;
	}

	auto filePath = GetOrSelectPath(ini, "Inject", "wuwapath", "wuwa path", "Executable\0Client-Win64-Shipping.exe;\0");

	if (!filePath)
	{
		killLoader();
	}
	auto commandline = ini.GetValue("Inject", "WuwaCommandLine");
	if (!commandline)
	{
		ini.SetValue("Inject", "WuwaCommandLine", "");
		ini.SaveFile("cfg.ini");
	}

	// 橡钼屦屐, 耋耱怏弪 腓 镟疣戾蝠 ?觐眙桡?	
	bool useAdditionalParamExists = ini.KeyExists("Inject", "UseAdditionalParam");
	bool useSkipSplashExists = ini.KeyExists("Inject", "useSkipSplash");

	if (!useAdditionalParamExists) {
		ini.SetBoolValue("Inject", "UseAdditionalParam", false);
		ini.SaveFile("cfg.ini");
	}
	if (!useSkipSplashExists) {
		ini.SetBoolValue("Inject", "useSkipSplash", false);
		ini.SaveFile("cfg.ini");
	}

	// 阻蜞屐 镟疣戾蝠 UseAdditionalParam
	bool useAdditionalParam = ini.GetBoolValue("Inject", "UseAdditionalParam", false);
	bool useSkipSplash = ini.GetBoolValue("Inject", "UseSkipSplash", false);

	std::string newCommandLine;
	if (commandline != nullptr) {
		newCommandLine = commandline;
	}

	if (useAdditionalParam && additionalParam != nullptr) {
		if (!newCommandLine.empty()) {
			newCommandLine += " ";
		}
		newCommandLine += additionalParam;
		if (useSkipSplash)
		{
			newCommandLine += " -SkipSplash";
		}
		//std::cout << "Launching the program with an additional parameter: " << additionalParam << std::endl;
	}
	else if (useAdditionalParam && additionalParam != nullptr || commandline != nullptr && !newCommandLine.empty())
	{
		std::cout << "Chay chuong trinh voi mot vai tham so bo sung: " << newCommandLine << std::endl;
	}
	else {
		std::cout << "Chay chuong trinh khong co tham so bo sung" << std::endl;
	}

	LPSTR lpstr = newCommandLine.empty() ? nullptr : const_cast<LPSTR>(newCommandLine.c_str());

	//LPSTR lpstr = commandline == nullptr ? nullptr : const_cast<LPSTR>(commandline);
	if (!filePath)
		return false;

	DWORD pid = FindProcessId("explorer.exe");
	if (pid == 0)
	{
		//LOG_ERROR("Can't find 'explorer' pid!");
		printf("Can't find 'explorer' pid!");
		return false;
	}

	std::string CurrentDirectory = filePath.value();
	int pos = CurrentDirectory.rfind("\\", CurrentDirectory.length());
	CurrentDirectory = CurrentDirectory.substr(0, pos);

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	SIZE_T lpsize = 0;
	InitializeProcThreadAttributeList(NULL, 1, 0, &lpsize);

	char* temp = new char[lpsize];
	LPPROC_THREAD_ATTRIBUTE_LIST AttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)temp;
	InitializeProcThreadAttributeList(AttributeList, 1, 0, &lpsize);
	if (!UpdateProcThreadAttribute(AttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
		&handle, sizeof(HANDLE), NULL, NULL))
	{
		//LOG_WARNING("UpdateProcThreadAttribute failed ! (%d).\n", GetLastError());
		printf("UpdateProcThreadAttribute failed ! (%d).\n", GetLastError());
	}

	STARTUPINFOEXA si{};
	si.StartupInfo.cb = sizeof(si);
	si.lpAttributeList = AttributeList;

	PROCESS_INFORMATION pi{};
	/*BOOL result = CreateProcessAsUserA(hToken, const_cast<LPSTR>(filePath->data()), lpstr,
		0, 0, 0, EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED, 0,
		(LPSTR)CurrentDirectory.data(), (LPSTARTUPINFOA)&si, &pi);*/
	BOOL result = CreateProcessAsUserA(
		hToken,
		const_cast<LPSTR>(filePath->data()),
		(std::string("\"") + *filePath + "\" " + (lpstr ? lpstr : "")).data(),
		nullptr, nullptr, FALSE,
		EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
		nullptr,
		(LPSTR)CurrentDirectory.data(),
		(LPSTARTUPINFOA)&si,
		&pi
	);

	bool isOpened = result;
	if (isOpened)
	{
		ini.SaveFile("cfg.ini");
		*phThread = pi.hThread;
		*phProcess = pi.hProcess;
	}
	else
	{
		//LOG_LAST_ERROR("Failed to create game process.");
		printf("Khong the tao tien trinh tro choi.\n");
		//LOG_ERROR("If you have problem with Client-Win64-Shipping.exe path. You can change it manually in cfg.ini.");
		printf("Neu ban gap van de voi duong dan Client-Win64-Shipping.exe ban co the thay doi thu cong trong cfg.ini.\n");
	}

	DeleteProcThreadAttributeList(AttributeList);
	delete[] temp;
	return isOpened;
}