#include <Windows.h>
#include <iostream>
#include "logger.h"
#include "SDK/Engine_classes.hpp"
#include "SDK/KuroHotPatch_classes.hpp"
#include <filesystem>
namespace fs = std::filesystem;

bool CheckMountPak()
{
    SDK::UFunction* MountPakFunc = SDK::UKuroPakMountStatic::StaticClass()->GetFunction("KuroPakMountStatic", "MountPak");
    if (!MountPakFunc)
        return false;
    return true;
}

void unloadHook(HMODULE hModule) {
    Sleep(1000);// TIMING CHUẨN
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

bool EnsureFolderExists(const std::string& folderPath) {
    if (!fs::exists(folderPath)) {
        LOG_ERROR("chua co thu muc. tien thanh tao");
        if (fs::create_directories(folderPath))
        {
            LOG_SUCCESS("da tao thu muc");
            return true;
        }
        else
        {
            return false;
        }
    }
    return true; // đã tồn tại
}



bool ProcessPakFiles(const std::string& folderPath) {
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        LOG_ERROR("Thu muc khong ton tai hoac khong hop le.\n");
        return false;
    }

    int idCounter = 46; // để bừa cũng đc
    bool foundPak = false;
    Sleep(3000);// TIMING CHUẨN
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".pak") {
            foundPak = true;
            std::wstring wpath = entry.path().wstring();
            SDK::UKuroPakMountStatic::MountPak(wpath.c_str(), idCounter);
            LOG_SUCCESS("load pak: %ws", wpath.c_str());
            SDK::UKuroPakMountStatic::RemoveSha1Check(wpath.c_str());
            idCounter++;
        }
    }

    if (!foundPak) {
        LOG_ERROR("Khong tim thay file *.pak trong thu muc.\n");
        return false;
    }
    else
    {
        return true;
    }
}

std::string GetCurrentDllDirectory(HMODULE hModule) {
    char buffer[MAX_PATH];
    GetModuleFileNameA(hModule, buffer, MAX_PATH); // lấy đường dẫn DLL
    return fs::path(buffer).parent_path().string(); // lấy thư mục chứa DLL
}

DWORD MainThread(HMODULE Module)
{
    Logger::Init("Log");
    LOG_SUCCESS("github.com/Lai-Hoang/wuwa-viet-hoa");

    while (true)
    {
        if (CheckMountPak())
        {
            LOG_INFO("[+] Pak Load valid!\n");
            break; // hoặc tiếp tục xử lý
        }
        else
        {
            //LOG_INFO("[-] Waiting for objects...\n");
        }

        Sleep(100);
    }

    std::filesystem::path dllPath = GetCurrentDllDirectory(Module);
    dllPath /= "wuwaVietHoa";
    std::string pathVietHoa = dllPath.string();
    if (EnsureFolderExists(pathVietHoa)) {
        LOG_INFO("Thu Muc Da San Sang: %s \n", pathVietHoa.c_str());
    }
    else {
        LOG_ERROR("Khong The Tao Thu Muc: %s\n", pathVietHoa.c_str());
    }

    if (!ProcessPakFiles(pathVietHoa))
    {
        LOG_ERROR("Khong co file .Pak nao Hoac khong truy cap duoc thu muc.\n");
        LOG_WARNING("Thoat Game Sau 5s.\n");
        Sleep(5000);
        ExitProcess(1);
    }
    unloadHook(Module);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    }

    return TRUE;
}