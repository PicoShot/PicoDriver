#pragma once
#include <Windows.h>
#include "vars.h"
#include <string>

#include "xorstr.hpp"

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1

#define MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

namespace driver
{
    namespace cache {
        inline HANDLE driverHandle = nullptr;
        inline DWORD processId = 0;
    }

    namespace scanner
	{
        struct Pattern {
            std::vector<int> pattern;
            std::string mask;
        };
    }

    namespace debug
	{
        inline bool enableLogging = true;
    }

    static constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG get_process_id = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x699, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG get_module_base = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x700, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG mouse_move = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);


#pragma pack(push, 8)
    struct Request {
        HANDLE process_id;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
        SIZE_T return_size;
    };

    struct ProcessRequest {
        alignas(8) WCHAR process_name[260];
        HANDLE process_id;
    };

    struct ModuleRequest {
        HANDLE process_id;
        alignas(8) WCHAR module_name[260];
        PVOID base_address;
    };

    struct MouseRequest {
        long x;
        long y;
        unsigned short button_flags;
    };
#pragma pack(pop)

    inline bool CreateDriverHandle() {
        cache::driverHandle = CreateFile(xorstr_(L"\\\\.\\PicoDriver"),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (cache::driverHandle == INVALID_HANDLE_VALUE) {
            return false;
        }
        vars::driverHandle = cache::driverHandle;
        return true;
    }

    inline DWORD GetProcessIdByName(const wchar_t* processName) {
        if (!vars::driverHandle || processName == nullptr) {
            return 0;
        }
        ProcessRequest request = {};
        wcscpy_s(request.process_name, processName);
        DWORD bytes_returned = 0;
        bool success = DeviceIoControl(
            vars::driverHandle,
            get_process_id,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytes_returned,
            nullptr
        );
        if (!success) {
            DWORD error = GetLastError();
            printf(xorstr_("GetProcessIdByName failed:\n"));
            printf(xorstr_("- Error code: %d\n"), error);
            return 0;
        }
        return (DWORD)(DWORD_PTR)request.process_id;
    }

    inline bool AttachToProcess()
    {
        CreateDriverHandle();
        vars::pid = GetProcessIdByName(xorstr_(L"cs2.exe"));
        Request r;

        r.process_id = reinterpret_cast<HANDLE>(static_cast<UINT_PTR>(vars::pid));

        return DeviceIoControl(vars::driverHandle, attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
    }

    inline scanner::Pattern ParsePattern(const char* pattern) {
	    scanner::Pattern result;
        char* start = const_cast<char*>(pattern);
        char* end = const_cast<char*>(pattern) + strlen(pattern);

        for (char* current = start; current < end; ++current) {
            if (*current == '?') {
                result.pattern.push_back(-1);
                result.mask.push_back('?');
            }
            else if (*current == ' ') {
                continue;
            }
            else {
                char byte = (char)strtoul(std::string(current, 2).c_str(), nullptr, 16);
                result.pattern.push_back(byte);
                result.mask.push_back('x');
                ++current;
            }
        }
        return result;
    }

    inline bool DataCompare(const uint8_t* data, const scanner::Pattern& pattern) {
        for (size_t i = 0; i < pattern.pattern.size(); i++) {
            if (pattern.mask[i] == '?' || pattern.mask[i] != 'x')
                continue;
            if (data[i] != static_cast<uint8_t>(pattern.pattern[i]))
                return false;
        }
        return true;
    }

    inline uintptr_t GetModuleBaseByName(const wchar_t* moduleName) {
        if (!vars::driverHandle) {
            printf(xorstr_("[-] Invalid driverHandle for GetModuleBase\n"));
            return 0;
        }

        if (!moduleName) {
            printf(xorstr_("[-] Invalid moduleName for GetModuleBase\n"));
            return 0;
        }

        if (!vars::pid) {
            printf(xorstr_("[-] Invalid pid for GetModuleBase\n"));
            return 0;
        }

        ModuleRequest request = {};
        request.process_id = reinterpret_cast<HANDLE>(vars::pid);
        wcscpy_s(request.module_name, moduleName);

        DWORD bytes_returned = 0;
        bool success = DeviceIoControl(
            vars::driverHandle,
            get_module_base,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytes_returned,
            nullptr
        );

        if (!success) {
            DWORD error = GetLastError();
            printf(xorstr_("[-] GetModuleBase failed:\n"));
            printf(xorstr_("  - Error code: %d\n"), error);
            printf(xorstr_("  - Module: %ws\n"), moduleName);
            return 0;
        }

        if (!request.base_address) {
            printf(xorstr_("[-] Module not found: %ws\n"), moduleName);
            return 0;
        }
        return reinterpret_cast<uintptr_t>(request.base_address);
    }

    template <typename T>
    __forceinline T Read(const std::uintptr_t addr) {
        if (addr == 0) {
            printf("[Read Failed] Null address\n");
            return T{};
        }

        alignas(T) T temp {};
        Request r{
            .process_id = reinterpret_cast<HANDLE>(vars::pid),
            .target = reinterpret_cast<PVOID>(addr),
            .buffer = &temp,
            .size = sizeof(T)
        };

        BOOL result = DeviceIoControl(vars::driverHandle, read, &r, sizeof(r),
            &r, sizeof(r), nullptr, nullptr);

        if (!result) {
            printf("[Read Failed] Address: 0x%llX, Size: %zu, Error: %lu\n",
                addr, sizeof(T), GetLastError());
            return T{};
        }

        // Optional: Add success logging for specific addresses you want to monitor
        // printf("[Read Success] Address: 0x%llX, Size: %zu\n", addr, sizeof(T));

        return temp;
    }

    template <typename T>
    __forceinline void Write(const std::uintptr_t addr, const T& value) {
        if (addr == 0) return;

        Request r{
            .target = reinterpret_cast<PVOID>(addr),
            .buffer = const_cast<void*>(static_cast<const void*>(&value)),
            .size = sizeof(T)
        };

        DeviceIoControl(vars::driverHandle, write, &r, sizeof(r),
            &r, sizeof(r), nullptr, nullptr);
    }

    inline uintptr_t FindPattern(uintptr_t start, size_t size, const char* pattern) {
        if (!start || !size || !pattern) {
            if (debug::enableLogging) printf("[-] Invalid parameters for pattern scan\n");
            return 0;
        }

        auto parsed = ParsePattern(pattern);
        if (parsed.pattern.empty()) {
            if (debug::enableLogging) printf("[-] Failed to parse pattern\n");
            return 0;
        }

        std::vector<uint8_t> buffer(size);

        Request r{
            .process_id = reinterpret_cast<HANDLE>(vars::pid),
            .target = reinterpret_cast<PVOID>(start),
            .buffer = buffer.data(),
            .size = size
        };

        if (!DeviceIoControl(vars::driverHandle, read, &r, sizeof(r),
            &r, sizeof(r), nullptr, nullptr)) {
            if (debug::enableLogging) printf("[-] Failed to read memory region\n");
            return 0;
        }

        if (debug::enableLogging) {
            printf("[+] Scanning memory region:\n");
            printf("    Base: 0x%llx\n", start);
            printf("    Size: 0x%llx\n", size);
            printf("    Pattern size: %zu\n", parsed.pattern.size());
        }

        for (size_t i = 0; i < size - parsed.pattern.size(); i++) {
            if (DataCompare(buffer.data() + i, parsed)) {
                if (debug::enableLogging) printf("[+] Pattern found at offset: 0x%llx\n", i);
                return start + i;
            }
        }

        if (debug::enableLogging) printf("[-] Pattern not found\n");
        return 0;
    }

    inline uintptr_t FindPatternInModule(const wchar_t* moduleName, const char* pattern) {
        uintptr_t moduleBase = GetModuleBaseByName(moduleName);
        if (!moduleBase) {
            if (debug::enableLogging) printf("[-] Failed to find module: %ws\n", moduleName);
            return 0;
        }

        // Read module headers
        IMAGE_DOS_HEADER dosHeader = Read<IMAGE_DOS_HEADER>(moduleBase);
        if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
            if (debug::enableLogging) printf("[-] Invalid DOS header\n");
            return 0;
        }

        IMAGE_NT_HEADERS ntHeaders = Read<IMAGE_NT_HEADERS>(moduleBase + dosHeader.e_lfanew);
        if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
            if (debug::enableLogging) printf("[-] Invalid NT headers\n");
            return 0;
        }

        size_t moduleSize = ntHeaders.OptionalHeader.SizeOfImage;
        if (debug::enableLogging) {
            printf("[+] Module info:\n");
            printf("    Base: 0x%llx\n", moduleBase);
            printf("    Size: 0x%llx\n", moduleSize);
        }

        return FindPattern(moduleBase, moduleSize, pattern);
    }

    inline bool IsValidPtr(uintptr_t ptr) {
        if (!ptr || ptr < 0x10000) return false;
        return true;
    }

    inline static std::string ReadString(const std::uintptr_t addr, size_t maxLength = 256) {
        if (addr == 0) return {};

        constexpr size_t STACK_BUFFER_SIZE = 256;
        if (maxLength <= STACK_BUFFER_SIZE) {
            char stackBuffer[STACK_BUFFER_SIZE];
            Request r{
                .process_id = reinterpret_cast<HANDLE>(vars::pid),
                .target = reinterpret_cast<PVOID>(addr),
                .buffer = stackBuffer,
                .size = maxLength
            };

            if (DeviceIoControl(vars::driverHandle, read, &r, sizeof(r),
                &r, sizeof(r), nullptr, nullptr)) {
                stackBuffer[maxLength - 1] = '\0';
                return stackBuffer;
            }
            return {};
        }

        std::vector<char> buffer(maxLength);
        Request r{
            .process_id = reinterpret_cast<HANDLE>(vars::pid),
            .target = reinterpret_cast<PVOID>(addr),
            .buffer = buffer.data(),
            .size = maxLength
        };

        if (DeviceIoControl(vars::driverHandle, read, &r, sizeof(r),
            &r, sizeof(r), nullptr, nullptr)) {
            buffer[maxLength - 1] = '\0';
            return buffer.data();
        }
        return {};
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = MOUSE_MOVE_ABSOLUTE) {
        static MouseRequest request;

        if (!vars::driverHandle) return false;

        request.x = x;
        request.y = y;
        request.button_flags = flags;

        return DeviceIoControl(
            vars::driverHandle,
            mouse_move,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            nullptr,
            nullptr
        );
    }
    
}