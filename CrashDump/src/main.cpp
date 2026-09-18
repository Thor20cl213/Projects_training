#include <windows.h>
#include <dbghelp.h>
#include <iostream>

#pragma comment(lib, "dbghelp.lib")

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException)
{
    HANDLE hFile = CreateFile(
        "crash.dmp",
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpWithFullMemory,
            &dumpInfo,
            nullptr,
            nullptr
        );

        CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
    SetUnhandledExceptionFilter(CrashHandler);

    int* p = nullptr;
    *p = 42; // 💥

    return 0;
}
