#include <ntifs.h>
#include <ntddk.h>

#include "mouse/mouse.h"
#include "process/Process.h"
#include "memory/memory.h"
#include "utils/defs.h"
#include "utils/log.h"

namespace driver
{
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

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
            status = STATUS_INVALID_DEVICE_STATE;
            return status;;
        }

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

            ExAcquireResourceExclusiveLite(&process_lock, TRUE);
            PEPROCESS old_process = target_process;
            status = PsLookupProcessByProcessId(kernelRequest.process_id, &target_process);
            if (old_process) ObDereferenceObject(old_process);
            ExReleaseResourceLite(&process_lock);
            break;
        }

        case codes::read:
        {
            auto userRequest = static_cast<Request*>(stack_irp->Parameters.DeviceIoControl.Type3InputBuffer);
            Request kernelRequest;

            status = memory::IsAddressValid(userRequest, sizeof(Request), FALSE);
            if (!NT_SUCCESS(status)) break;

            RtlCopyMemory(&kernelRequest, userRequest, sizeof(Request));

            status = memory::Read(
                kernelRequest.target,
                kernelRequest.buffer,
                kernelRequest.size,
                &kernelRequest.return_size
            );
            break;
        }

        case codes::write:
        {
            auto userRequest = static_cast<Request*>(stack_irp->Parameters.DeviceIoControl.Type3InputBuffer);
            Request kernelRequest;

            status = memory::IsAddressValid(userRequest, sizeof(Request), FALSE);
            if (!NT_SUCCESS(status)) break;

            RtlCopyMemory(&kernelRequest, userRequest, sizeof(Request));

            status = memory::Write(
                kernelRequest.target,
                kernelRequest.buffer,
                kernelRequest.size,
                &kernelRequest.return_size
            );
            break;
        }

        case codes::get_process_id:
        {
            auto request = static_cast<ProcessRequest*>(irp->AssociatedIrp.SystemBuffer);
            if (request)
            {
                status = Process::GetProcessId(request);
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
                status = Process::GetModuleBase(request);
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

    ExDeleteResourceLite(&process_lock);
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

    ExInitializeResourceLite(&process_lock);


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