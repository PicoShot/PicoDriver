#pragma once

namespace memory
{
    NTSTATUS IsAddressValid(PVOID address, SIZE_T size, BOOLEAN isWrite) {
        __try {
            if (isWrite) {
                ProbeForWrite(address, size, __alignof(UCHAR));
            }
            else {
                ProbeForRead(address, size, __alignof(UCHAR));
            }
            return STATUS_SUCCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    NTSTATUS ValidateAndGetProcess(PEPROCESS* outProcess) {
        if (!outProcess) return STATUS_INVALID_PARAMETER;

        ExAcquireResourceSharedLite(&process_lock, TRUE);
        PEPROCESS current_target = target_process;
        if (current_target) {
            ObReferenceObject(current_target);
            *outProcess = current_target;
        }
        ExReleaseResourceLite(&process_lock);

        return current_target ? STATUS_SUCCESS : STATUS_PROCESS_NOT_IN_JOB;
    }

    NTSTATUS Read(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesTransferred) {
        PEPROCESS targetProc;
        NTSTATUS status;

        if (!targetAddress || !buffer || !size) {
            return STATUS_INVALID_PARAMETER;
        }

        status = IsAddressValid(buffer, size, TRUE);
        if (!NT_SUCCESS(status)) return status;

        status = ValidateAndGetProcess(&targetProc);
        if (!NT_SUCCESS(status)) return status;

        status = MmCopyVirtualMemory(
            targetProc,
            targetAddress,
            PsGetCurrentProcess(),
            buffer,
            size,
            KernelMode,
            bytesTransferred
        );

        ObDereferenceObject(targetProc);
        return status;
    }

    NTSTATUS Write(PVOID targetAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesTransferred) {
        PEPROCESS targetProc;
        NTSTATUS status;

        if (!targetAddress || !buffer || !size) {
            return STATUS_INVALID_PARAMETER;
        }

        status = IsAddressValid(buffer, size, FALSE);
        if (!NT_SUCCESS(status)) return status;

        status = ValidateAndGetProcess(&targetProc);
        if (!NT_SUCCESS(status)) return status;

        status = MmCopyVirtualMemory(
            PsGetCurrentProcess(),
            buffer,
            targetProc,
            targetAddress,
            size,
            KernelMode,
            bytesTransferred
        );

        ObDereferenceObject(targetProc);
        return status;
    }
}
