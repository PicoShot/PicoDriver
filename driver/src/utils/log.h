#pragma once


#ifdef _DEBUG
#define LogInfo(...) DbgPrintEx(...)
#define LogError(...) DbgPrintEx(...)
#else
#define LogInfo(...)
#define LogError(...)
#endif