#pragma once
#include <ntifs.h>

typedef unsigned char BYTE;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void (*PPS_POST_PROCESS_INIT_ROUTINE)(void);

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID Reserved4[3];
    PVOID AtlThunkSListPtr;
    PVOID Reserved5;
    ULONG Reserved6;
    PVOID Reserved7;
    ULONG Reserved8;
    ULONG AtlThunkSListPtr32;
    PVOID Reserved9[45];
    BYTE Reserved10[96];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved11[128];
    PVOID Reserved12[1];
    ULONG SessionId;
} PEB, * PPEB;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    BYTE Reserved1[48];
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    PVOID Reserved2;
    ULONG HandleCount;
    ULONG SessionId;
    PVOID Reserved3;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG Reserved4;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    PVOID Reserved5;
    SIZE_T QuotaPagedPoolUsage;
    PVOID Reserved6;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved7[6];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;


#define KeMRaiseIrql(a, b) *(b) = KfRaiseIrql(a)
#define KM_POOL_TAG 'kdd'

typedef int BOOL;
typedef unsigned int DWORD;
typedef ULONG_PTR QWORD;

#pragma warning(disable : 4201)
typedef struct _MOUSE_INPUT_DATA {
    USHORT UnitId;
    USHORT Flags;
    union {
        ULONG Buttons;
        struct {
            USHORT ButtonFlags;
            USHORT ButtonData;
        };
    };
    ULONG RawButtons;
    LONG LastX;
    LONG LastY;
    ULONG ExtraInformation;
} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;

typedef VOID(*MouseClassServiceCallbackFn)(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);

typedef struct _MOUSE_OBJECT {
    PDEVICE_OBJECT mouse_device;
    MouseClassServiceCallbackFn service_callback;
    BOOL use_mouse;
} MOUSE_OBJECT, * PMOUSE_OBJECT;


typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;


MOUSE_OBJECT gMouseObject;

extern "C" {

    // assembly

    QWORD _KeAcquireSpinLockAtDpcLevel;
    QWORD _KeReleaseSpinLockFromDpcLevel;
    QWORD _IofCompleteRequest;
    QWORD _IoReleaseRemoveLockEx;

    // system calls

    NTSYSCALLAPI POBJECT_TYPE* IoDriverObjectType;
    NTSYSCALLAPI NTSTATUS ObReferenceObjectByName(__in PUNICODE_STRING ObjectName, __in ULONG Attributes, __in_opt PACCESS_STATE AccessState, __in_opt ACCESS_MASK DesiredAccess, __in POBJECT_TYPE ObjectType,__in KPROCESSOR_MODE AccessMode, __inout_opt PVOID ParseContext, __out PVOID* Object);

    // mouse service

    VOID MouseClassServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);

    // kernel api 

    NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
    NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T RetunSize);
    NTKERNELAPI NTSTATUS ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
    NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);
    NTKERNELAPI PPEB PsGetProcessPeb(PEPROCESS Process);
}