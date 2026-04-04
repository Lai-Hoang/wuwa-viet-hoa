#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include "interceptor.hpp"
#include "logger.h"

// Con trỏ hàm gốc trong kernelbase.dll để gọi sau khi chặn
typedef HANDLE(WINAPI* PCreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

PCreateFileW CreateFileW_Original = nullptr;


namespace SigGate {
    const std::wstring ext = L".sig";
    const std::wstring ue_p_suffix = L"WindowsNoEditor_P.sig";
    const std::wstring ue_suffix = L"WindowsNoEditor.sig";

    // Hàm Callback sẽ được gọi thay cho CreateFileW
    HANDLE WINAPI callback(
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
    ) {
        std::wstring file_name(lpFileName);

        // Logic kiểm tra đuôi file .sig nhưng loại trừ các file của Unreal Engine
        if (file_name.size() >= 4 && file_name.substr(file_name.size() - 4) == ext) {
            if (file_name.find(ue_suffix) == std::wstring::npos &&
                file_name.find(ue_p_suffix) == std::wstring::npos) {

                LOG_SUCCESS("Hit CreateFileW: %ls - Suspending thread!\n", lpFileName);
                // Dừng luồng hiện tại nếu khớp điều kiện
                SuspendThread(GetCurrentThread());
            }
        }

        // Gọi hàm CreateFileW gốc để game tiếp tục hoạt động
        return CreateFileW_Original(
            lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition,
            dwFlagsAndAttributes, hTemplateFile
        );
    }
}

bool IsMyGameWindow(HWND hwnd) {
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid); // Lấy PID của cửa sổ tìm được
    return windowPid == GetCurrentProcessId(); // So sánh với PID của chính mình
}

void onAttach(HMODULE Module) {
    Logger::Init("wuwaVietHoa");
    AllocConsole();
    //FILE* f;
    //freopen_s(&f, "CONOUT$", "w", stdout);

    LOG_INFO("Dang Cho wuwaVietHoa: https://github.com/Lai-Hoang/wuwa-viet-hoa\n");

    // Lấy địa chỉ hàm CreateFileW từ kernelbase.dll và kernel32.dll
    HMODULE hKernelBase = GetModuleHandleW(L"kernelbase.dll");
    CreateFileW_Original = (PCreateFileW)GetProcAddress(hKernelBase, "CreateFileW");

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    uintptr_t create_file_w_addr = (uintptr_t)GetProcAddress(hKernel32, "CreateFileW");

    // Thực hiện chặn hàm (Intercept/Hook)
    if (Interceptor::replace(create_file_w_addr, (void*)SigGate::callback) != Interceptor::Error::Success) {
        LOG_ERROR("Failed to intercept CreateFileW!\n");
        exit(1);
    }
    LOG_SUCCESS("Thanh Cong.!\n");
    
    /*while (!FindWindowA("UnrealWindow", 0)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }*/
    HWND targetHwnd = NULL;

    // Vòng lặp đợi cho đến khi tìm đúng cửa sổ của tiến trình này
    while (true) {
        // Tìm cửa sổ có lớp là UnrealWindow
        targetHwnd = FindWindowA("UnrealWindow", NULL);

        if (targetHwnd != NULL) {
            // Nếu tìm thấy, kiểm tra xem có phải của game mình đang inject không
            if (IsMyGameWindow(targetHwnd)) {
                break; // Đúng cửa sổ của game này, thoát vòng lặp
            }
        }

        // Nếu không phải hoặc chưa thấy, đợi 100ms rồi tìm lại
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    LOG_WARNING("Vui Long Nhan X O Bang nay de TAT GAME"); // nhấn X ở bảng này để tắt game.
    Interceptor::restore();
    //FreeConsole();
    FreeLibraryAndExitThread(Module, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)onAttach, hModule, 0, 0);
    }
    return TRUE;
}