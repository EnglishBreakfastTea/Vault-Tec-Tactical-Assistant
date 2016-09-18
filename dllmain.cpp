#include <cstdio>
#include <windows.h>
#include <fcntl.h>

#include "FOClient.h"
#include "mainLoop.h"

/* ParseMouse call at 0x4a822c:
 * 8B CE                 - mov ecx,esi
 * E8 7FF0FEFF           - call "FOnline 2.FOClient::ParseMouse" = 0x004972B0
 */

DWORD mainLoopInjAddress = 0x004A822C;
int mainLoopInjNopCount = 0;
extern "C" {
    void _stdcall mainLoopCWrapper(FOClient* client) { mainLoop(client); }
    void mainLoopInjCode(void);
}
__asm(
".globl _mainLoopInjCode\n"
"_mainLoopInjCode:\n"
    "pushad;"
    "pushfd;"
    "push ecx;" // FOClient* argument
    "call _mainLoopCWrapper@4;"
    "popfd;"
    "popad;"

    "push eax;"
    "mov eax, 0x004972B0;"
    "call eax;"
    "pop eax;"
    "ret;"
);

/*DWORD lMouseDownInjAddress = 0x0;
int lMouseDownInjNopCount = 0;
extern "C" { void lMouseDownInjCode(void); }
__asm(
".globl lMouseDownInjCode"
"_lMouseDownInjCode:"
"   ret"
);*/

void writeBytes(DWORD destAddr, void* patch, int numBytes)
{
    DWORD oldProtect = 0;
    auto destPtr = reinterpret_cast<void*>(destAddr);
    VirtualProtect(destPtr, numBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(destPtr, patch, numBytes);
    VirtualProtect(destPtr, numBytes, oldProtect, &oldProtect);
}

void cave(DWORD destAddr, void (*func)(void), int nopCount)
{
    static_assert(sizeof(DWORD) == 4, "DWORD != 4");

    BYTE patch[5] = {0xE8};
    DWORD offset = PtrToUlong(func) - destAddr - 5;
    memcpy(patch + 1, &offset, 4);

    writeBytes(destAddr, patch, 5);

    if (nopCount == 0) {
        return;
    }

    BYTE nops[255] = {0};
    memset(nops, 0x90, nopCount);

    writeBytes(destAddr + 5, nops, nopCount);
}

void createConsole()
{
    int hConHandle = 0;
    HANDLE lStdHandle = 0;
    FILE *fp = 0;

    // Allocate a console
    AllocConsole();

    // redirect unbuffered STDOUT to the console
    lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    // redirect unbuffered STDIN to the console
    lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
    fp = _fdopen(hConHandle, "r");
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);

    // redirect unbuffered STDERR to the console
    lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);
}

BOOL APIENTRY DllMain(HINSTANCE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        createConsole();
        cave(mainLoopInjAddress, mainLoopInjCode, mainLoopInjNopCount);
        //cave(lMouseDownInjAddress, lMouseDownInjCode, lMouseDownInjNopCount);
    }
}
