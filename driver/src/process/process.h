#pragma once
#include <ntdef.h>
#include "../utils/log.h"

namespace Process
{
    NTSTATUS GetModuleBase(ModuleRequest* request) {
        LogInfo("[+] GetModuleBase called\n");
        if (!request || !request->module_name[0]) {
            return STATUS_INVALID_PARAMETER;
        }

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
                PPEB_LDR_DATA ldr = peb->Ldr;

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

    NTSTATUS GetProcessId(ProcessRequest* request)
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

        LogInfo((DPFLTR_IHVAUDIO_ID, DPFLTR_INFO_LEVEL, "%wZ\n", &processName));

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
}
