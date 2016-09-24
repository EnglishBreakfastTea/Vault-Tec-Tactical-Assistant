#include <windows.h>
#include <stdio.h>

struct AllocRAII {
    AllocRAII(HANDLE proc, size_t len)
        : mem{VirtualAllocEx(proc, NULL, len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)}, len{len}, proc{proc}
    {
    }

    ~AllocRAII()
    {
        release();
    }

    void release()
    {
        if (mem != NULL)
            VirtualFreeEx(proc, mem, len, MEM_RELEASE);

        mem = NULL;
    }

    void* mem;

private:
    size_t len;
    HANDLE proc;
};

void Inject(HANDLE process, const char* dllPath)
{
    auto kernel32 = GetModuleHandle("kernel32.dll");

    auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "LoadLibraryA"));

    auto dllPathMem = AllocRAII(process, strlen(dllPath) + 1);
    if (dllPathMem.mem == NULL) {
        printf("Error D: VirtualAllocEx");
        return;
    }

    WriteProcessMemory(process, dllPathMem.mem, dllPath, strlen(dllPath) + 1, NULL);

    auto thread = CreateRemoteThread(process, NULL, 0, loadLibrary, dllPathMem.mem, 0, NULL);
    if (thread == NULL) {
        printf("Error D: CreateRemoteThread");
        return;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPTSTR cmdLine, int cmdShow)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    BOOL result = FALSE;

    char exePath[MAX_PATH + 1] = {0};
    char workingDir[MAX_PATH + 1] = {0};
    char dllPath[MAX_PATH + 1] = {0};

    GetCurrentDirectory(MAX_PATH, workingDir);
    snprintf(exePath, MAX_PATH, "\"%s\\FOnline.exe\"", workingDir);
    snprintf(dllPath, MAX_PATH, "libVaultTecTacticalAssistant.dll");

    si.cb = sizeof(STARTUPINFO);

    result = CreateProcess(NULL, exePath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, workingDir, &si, &pi);
    if (!result) {
        MessageBox(0, "Error D:", "Error", MB_ICONERROR);
        return -1;
    }

    Inject(pi.hProcess, dllPath);

    ResumeThread(pi.hThread);

    return 0;
}

