#include <windows.h>
#include <vector>
#include <cstdint>
namespace Interceptor {
    // Lưu trữ byte gốc để khôi phục sau này
    inline uint8_t original_bytes[12];
    inline uintptr_t target_addr = 0;
    const size_t trampoline_size = 12;

    enum class Error { Success, MemoryProtectionFailed };

    inline Error replace(uintptr_t address, void* replacement) {
        target_addr = address;

        DWORD oldProtect;

        // Sao lưu 12 byte gốc trước khi sửa đổi
        memcpy(original_bytes, (void*)address, trampoline_size);

        // Thay đổi quyền truy cập bộ nhớ để ghi (Read/Write/Execute)
        if (!VirtualProtect((LPVOID)address, trampoline_size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return Error::MemoryProtectionFailed;
        }

        uint8_t* location = (uint8_t*)address;

        // MOV RAX, [64-bit address]
        location[0] = 0x48;
        location[1] = 0xB8;
        *(uint64_t*)(location + 2) = (uint64_t)replacement;

        // PUSH RAX; RET
        location[10] = 0x50;
        location[11] = 0xC3;

        // Khôi phục lại quyền bộ nhớ (Read/Execute)
        VirtualProtect((LPVOID)address, trampoline_size, oldProtect, &oldProtect);
        return Error::Success;
    }
    inline void restore() {
        if (target_addr == 0) return;
        DWORD oldProtect;
        VirtualProtect((LPVOID)target_addr, trampoline_size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)target_addr, original_bytes, trampoline_size); // Trả lại trạng thái ban đầu
        VirtualProtect((LPVOID)target_addr, trampoline_size, oldProtect, &oldProtect);
    }
}