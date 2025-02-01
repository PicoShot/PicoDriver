#pragma once
#include <Windows.h>
#include "../config/vars.h"
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

    static constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
    static constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
    static constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
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
        DeviceIoControl(
            vars::driverHandle,
            get_process_id,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytes_returned,
            nullptr
        );
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

    inline uintptr_t GetModuleBaseByName(const wchar_t* moduleName) {
        ModuleRequest request = {};
        request.process_id = reinterpret_cast<HANDLE>(vars::pid);
        wcscpy_s(request.module_name, moduleName);

        DWORD bytes_returned = 0;
        DeviceIoControl(
            vars::driverHandle,
            get_module_base,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytes_returned,
            nullptr
        );
        return reinterpret_cast<uintptr_t>(request.base_address);
    }

    template <typename T>
    __forceinline T Read(const std::uintptr_t addr) {
        alignas(T) T temp {};
        Request r{
            .process_id = reinterpret_cast<HANDLE>(vars::pid),
            .target = reinterpret_cast<PVOID>(addr),
            .buffer = &temp,
            .size = sizeof(T)
        };

        DeviceIoControl(vars::driverHandle, read, &r, sizeof(r),
            &r, sizeof(r), nullptr, nullptr);

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