#include <cstdio>
#include <windows.h>
#include <fcntl.h>

#include "FOClient.h"
#include "mainLoop.h"
#include "LeftMouseHook.h"
#include "InterfaceOverlay.h"
#include "Hotkeys.h"
#include "State.h"

/* We insert a call to our code (mainLoop) before the call to ParseMouse in MainLoop. */

DWORD mainLoopInjAddress = 0x494B22;
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
"mov eax, 0x00484190;" // FOClient::ParseMouse
"call eax;"
"pop eax;"
"ret;"
);

/* We insert a call to our code (lMouseDown) before the call to GameLMouseDown in ParseMouse.
 * The code may decide for the call to GameLMouseDown to not be performed. */

DWORD lMouseDownInjAddress = 0x484DD5;
int lMouseDownInjNopCount = 0;
extern "C" {
    /* Returns 1 if the GameLMouseDown call should be performed, 0 otherwise. */
    DWORD _stdcall lMouseDownCWrapper(FOClient* client) { return !lMouseDown(client); }
    void lMouseDownInjCode(void);
}
__asm(
".globl _lMouseDownInjCode\n"
"_lMouseDownInjCode:\n"
"push ebp;"
"mov ebp, esp;"
"sub esp, 4;"
"pushad;"
"pushfd;"
"push ecx;" // FOClient*
"call _lMouseDownCWrapper@4;"
"mov [ebp-4], eax;" // get the returned value
"popfd;"
"popad;"

"push eax;"
"mov eax, [ebp-4];"
"cmp eax, 0;"
"pop eax;"
"je vtta_lmd_end;"

"push eax;"
"mov eax, 0x004C2450;" // FOClient::GameLMouseDown
"call eax;"
"pop eax;"

"vtta_lmd_end:"
"mov esp, ebp;"
"pop ebp;"
"ret;"
);

/* We insert a call to our code (drawIface) before the call to RunPrepared in DrawIfaceLayer. */

DWORD drawIfaceInjAddress = 0x456432;
int drawIfaceInjNopCount = 0;
extern "C" {
    void _stdcall drawIfaceCWrapper(FOClient* client) { drawIface(client); }
    void drawIfaceInjCode(void);
}
__asm(
".globl _drawIfaceInjCode\n"
"_drawIfaceInjCode:\n"
"pushad;"
"pushfd;"
"push esi;" // FOClient*
"call _drawIfaceCWrapper@4;"
"popfd;"
"popad;"

"push eax;"
"mov eax, 0x005085E0;" //Script::RunPrepared
"call eax;"
"pop eax;"
"ret;"
);

/* We insert a call to our code (parseKeyboard) before the call to ParseKeyboard in MainLoop. */

DWORD parseKeyboardInjAddress = 0x494B1B;
int parseKeyboardInjNopCount = 0;
extern "C" {
    void _stdcall parseKeyboardCWrapper(FOClient* client) { parseKeyboard(client); }
    void parseKeyboardInjCode(void);
}
__asm(
".globl _parseKeyboardInjCode\n"
"_parseKeyboardInjCode:\n"
"pushad;"
"pushfd;"
"push ecx;" // FOClient*
"call _parseKeyboardCWrapper@4;"
"popfd;"
"popad;"

"push eax;"
"mov eax, 0x00483580;" // FOClient::ParseKeyboard
"call eax;"
"pop eax;"
"ret;"
);

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
        cave(lMouseDownInjAddress, lMouseDownInjCode, lMouseDownInjNopCount);
        cave(drawIfaceInjAddress, drawIfaceInjCode, drawIfaceInjNopCount);
        cave(parseKeyboardInjAddress, parseKeyboardInjCode, parseKeyboardInjNopCount);
    }

    if (reason == DLL_PROCESS_DETACH) {
        state = nullptr;
    }
}
