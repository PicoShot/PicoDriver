#pragma once


#ifdef _DEBUG
#define LogError(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-] - " __VA_ARGS__)
#define LogInfo(...) DbgPrintEx((DPFLTR_IHVAUDIO_ID, DPFLTR_INFO_LEVEL, "[+] - " __VA_ARGS__));
#else
#define LogError(...)
#define LogInfo(...)
#endif