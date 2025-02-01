#include <ntifs.h>
#include <ntddk.h>

#include "mouse/mouse.h"
#include "utils/defs.h"
#include "utils/log.h"

namespace driver
{
    ERESOURCE process_lock;

    namespace codes
    {
        constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
        constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
        constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_NEITHER, FILE_SPECIAL_ACCESS);
        constexpr ULONG get_process_id = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x699, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG get_module_base = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x700, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG mouse_move = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }

    struct Request
    {
        HANDLE process_id;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
        SIZE_T return_size;
    };

    struct ProcessRequest
    {
        WCHAR process_name[260];
        HANDLE process_id;
    };

    struct ModuleRequest {
        HANDLE process_id;
        WCHAR module_name[260];
        PVOID base_address;
    };

    struct MouseRequest {
        long x;
        long y;
        unsigned short button_flags;
    };

    NTSTATUS GetModuleBase(ModuleRequest* request) {
        LogInfo("[+] GetModuleBase called\n");
        if (!request || !request->module_name[0]) {
            return STATUS_INVALID_PARAMETER;
        }

        PEPROCESS target_process = nullptr;
        NTSTATUS status = PsLookupProcessByProcessId(request->process_id, &target_process);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        __try {
            PPEB peb = PsGetProcessPeb(target_process);
            if (!peb) {
                ObDereferenceObject(target_process);
                return STATUS_UNSUCCESSFUL;
            }

            KAPC_STATE apc_state;
            KeStackAttachProcess(target_process, &apc_state);

            UNICODE_STRING targetModule;
            RtlInitUnicodeString(&targetModule, request->module_name);

            request->base_address = nullptr;

            if (peb && peb->Ldr) {
                PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)peb->Ldr;

                for (PLIST_ENTRY list = ldr->InLoadOrderModuleList.Flink;
                    list != &ldr->InLoadOrderModuleList;
                    list = list->Flink) {

                    PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

                    if (RtlEqualUnicodeString(&entry->BaseDllName, &targetModule, TRUE)) {
                        request->base_address = entry->DllBase;
                        break;
                    }
                }
            }

            KeUnstackDetachProcess(&apc_state);
            ObDereferenceObject(target_process);

            return request->base_address ? STATUS_SUCCESS : STATUS_NOT_FOUND;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject(target_process);
            return STATUS_ACCESS_VIOLATION;
        }
    }

    NTSTATUS GetProcessIdByName(ProcessRequest* request)
    {
        LogInfo("[+] GetProcessIdByName called\n");

        if (!request || !request->process_name[0])
        {
            LogError("[-] Invalid request parameters\n");
            return STATUS_INVALID_PARAMETER;
        }

        UNICODE_STRING processName;
        RtlInitUnicodeString(&processName, request->process_name);
        LogInfo("[+] Searching for process: ");
        // Note: Using %wZ for UNICODE_STRING
        KdPrintEx((DPFLTR_IHVAUDIO_ID, DPFLTR_INFO_LEVEL, "%wZ\n", &processName));

        ULONG buffer_size = 0;
        PVOID buffer = nullptr;
        NTSTATUS status;

        status = ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &buffer_size);
        if (status != STATUS_INFO_LENGTH_MISMATCH)
        {
            LogError("[-] Initial ZwQuerySystemInformation failed\n");
            return STATUS_UNSUCCESSFUL;
        }

        buffer_size += 4096;
        buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, buffer_size, 'PrID');
        if (!buffer)
        {
            LogError("[-] Failed to allocate memory\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwQuerySystemInformation(SystemProcessInformation, buffer, buffer_size, nullptr);
        if (!NT_SUCCESS(status))
        {
            LogError("[-] Failed to query system information\n");
            ExFreePoolWithTag(buffer, 'PrID');
            return status;
        }

        PSYSTEM_PROCESS_INFORMATION curr = (PSYSTEM_PROCESS_INFORMATION)buffer;
        request->process_id = nullptr;

        while (TRUE)
        {
            if (curr->ImageName.Buffer && curr->ImageName.Length > 0)
            {
                LogInfo("[+] Checking process: ");
                KdPrintEx((DPFLTR_IHVAUDIO_ID, DPFLTR_INFO_LEVEL, "%wZ\n", &curr->ImageName));

                if (RtlEqualUnicodeString(&curr->ImageName, &processName, TRUE))
                {
                    request->process_id = curr->UniqueProcessId;
                    LogInfo("[+] Process found!\n");
                    break;
                }
            }

            if (curr->NextEntryOffset == 0)
                break;

            curr = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)curr + curr->NextEntryOffset);
        }

        ExFreePoolWithTag(buffer, 'PrID');

        if (request->process_id)
        {
            LogInfo("[+] Process ID found successfully\n");
            return STATUS_SUCCESS;
        }
        else
        {
            LogError("[-] Process not found\n");
            return STATUS_NOT_FOUND;
        }
    }

    NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        LogInfo("[+] Device control called.\n");

        NTSTATUS status = STATUS_UNSUCCESSFUL;
        PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

        if (!stack_irp)
        {
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return status;
        }

        static PEPROCESS target_process = nullptr;
        const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;

        switch (control_code)
        {
        case codes::attach:
        {
            auto userRequest = static_cast<Request*>(stack_irp->Parameters.DeviceIoControl.Type3InputBuffer);

            __try {
                ProbeForRead(userRequest, sizeof(Request), __alignof(Request));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                break;
            }

            Request kernelRequest;
            RtlCopyMemory(&kernelRequest, userRequest, sizeof(Request));

            ExAcquireResourceExclusiveLite(&driver::process_lock, TRUE);
            PEPROCESS old_process = target_process;
            status = PsLookupProcessByProcessId(kernelRequest.process_id, &target_process);
            if (old_process) ObDereferenceObject(old_process);
            ExReleaseResourceLite(&driver::process_lock);
            break;
        }

        case codes::read:
        {
            auto userRequest = static_cast<Request*>(stack_irp->Parameters.DeviceIoControl.Type3InputBuffer);

            __try {
                ProbeForRead(userRequest, sizeof(Request), __alignof(Request));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                break;
            }

            Request kernelRequest;
            RtlCopyMemory(&kernelRequest, userRequest, sizeof(Request));

            __try {
                ProbeForWrite(kernelRequest.buffer, kernelRequest.size, __alignof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                break;
            }

            ExAcquireResourceSharedLite(&driver::process_lock, TRUE);
            PEPROCESS current_target = target_process;
            if (current_target) ObReferenceObject(current_target);
            ExReleaseResourceLite(&driver::process_lock);

            if (current_target) {
                status = MmCopyVirtualMemory(
                    current_target, kernelRequest.target,
                    PsGetCurrentProcess(), kernelRequest.buffer,
                    kernelRequest.size, KernelMode, &kernelRequest.return_size
                );
                ObDereferenceObject(current_target);
            }

            break;
        }

        case codes::write:
        {
            auto userRequest = static_cast<Request*>(stack_irp->Parameters.DeviceIoControl.Type3InputBuffer);

            __try {
                ProbeForRead(userRequest, sizeof(Request), __alignof(Request));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                break;
            }

            Request kernelRequest;
            RtlCopyMemory(&kernelRequest, userRequest, sizeof(Request));

            __try {
                ProbeForRead(kernelRequest.buffer, kernelRequest.size, __alignof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                break;
            }

            ExAcquireResourceSharedLite(&driver::process_lock, TRUE);
            PEPROCESS current_target = target_process;
            if (current_target) ObReferenceObject(current_target);
            ExReleaseResourceLite(&driver::process_lock);

            if (current_target) {
                status = MmCopyVirtualMemory(
                    PsGetCurrentProcess(), kernelRequest.buffer,
                    current_target, kernelRequest.target,
                    kernelRequest.size, KernelMode, &kernelRequest.return_size
                );
                ObDereferenceObject(current_target);
            }

            break;
        }

        case codes::get_process_id:
        {
            auto request = reinterpret_cast<ProcessRequest*>(irp->AssociatedIrp.SystemBuffer);
            if (request)
            {
                status = GetProcessIdByName(request);
                irp->IoStatus.Information = sizeof(ProcessRequest);
            }
        }
        break;

        case codes::get_module_base:
        {
            auto request = reinterpret_cast<ModuleRequest*>(irp->AssociatedIrp.SystemBuffer);
            if (request)
            {
                LogInfo("[+] Getting module base address\n");
                status = GetModuleBase(request);
                irp->IoStatus.Information = sizeof(ModuleRequest);
            }
        }
        break;

        case codes::mouse_move:
        {
            auto request = reinterpret_cast<MouseRequest*>(irp->AssociatedIrp.SystemBuffer);

            if (request)
            {
                LogInfo("[+] Moving mouse\n");
                mouse_move(request->x, request->y, request->button_flags);
                irp->IoStatus.Information = 0;
            }

        }
        break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            irp->IoStatus.Information = 0;
            break;
        }

        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return status;
    }
}

VOID DriverUnload(PDRIVER_OBJECT driver_object)
{
    LogInfo("[+] Unloading PicoDriver...\n");

    ExDeleteResourceLite(&driver::process_lock);
    UNICODE_STRING symbolic_link = {};
    RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\PicoDriver");
    IoDeleteSymbolicLink(&symbolic_link);

    if (driver_object->DeviceObject)
    {
        IoDeleteDevice(driver_object->DeviceObject);
    }


    LogInfo("[+] PicoDriver unloaded successfully.\n");
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
    UNREFERENCED_PARAMETER(registry_path);

    ExInitializeResourceLite(&driver::process_lock);


    UNICODE_STRING device_name = {};
    RtlInitUnicodeString(&device_name, L"\\Device\\PicoDriver");

    PDEVICE_OBJECT device_object = nullptr;
    NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

    if (!NT_SUCCESS(status))
    {
        LogError("[-] Failed to create driver device.\n");
        return status;
    }

    LogInfo("[+] Driver device successfully created.\n");

    UNICODE_STRING symbolic_link = {};
    RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\PicoDriver");

    status = IoCreateSymbolicLink(&symbolic_link, &device_name);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(device_object);
        LogError("[-] Failed to establish symbolic link.\n");
        return status;
    }

    LogInfo("[+] Driver symbolic link successfully established.\n");

    driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
    driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;
    driver_object->DriverUnload = DriverUnload;

    ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);
    LogInfo("[+] Driver Initializing successfully.\n");

    return status;
}

NTSTATUS DriverEntry()
{
    LogInfo("[+] hello from pico driver\n");

    UNICODE_STRING driver_name = {};
    RtlInitUnicodeString(&driver_name, L"\\Driver\\PicoDriver");

    return IoCreateDriver(&driver_name, &driver_main);
}