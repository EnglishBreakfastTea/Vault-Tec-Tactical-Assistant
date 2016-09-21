#include <cstdio>
#include <windows.h>
#include <fcntl.h>

#include "FOClient.h"
#include "mainLoop.h"
#include "LeftMouseHook.h"

/* ParseMouse call at 0x4a822c:
 * 8B CE                 - mov ecx,esi
 * E8 7FF0FEFF           - call "FOnline 2.FOClient::ParseMouse" = 0x004972B0
 */

/* We insert a call to our code (mainLoop) before the call to ParseMouse. */

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

/* GameLMouseDown call at 0x497f02:
 * 8B CE                 - mov ecx,esi
 * E8 C9DE0300           - call "FOnline 2.FOClient::GameLMouseDown" = 0x004D5DD0
 */

/* We insert a call to our code (lMouseDown) before the call to GameLMouseDown.
 * The code may decide for the call to GameLMouseDown to not be performed. */

DWORD lMouseDownInjAddress = 0x497f02;
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
"mov eax, 0x004D5DD0;"
"call eax;"
"pop eax;"

"vtta_lmd_end:"
"mov esp, ebp;"
"pop ebp;"
"ret;"
);

/* RunPrepared call at 0x468ec2:
 * C6 05 9CC38300 01     - mov byte ptr ["FOnline 2.FOClient::SpritesCanDraw"],01
 * E8 01FA0A00           - call "FOnline 2.Script::SetArgUInt"
 * 83 C4 04              - add esp,04
 * E8 192B0B00           - call "FOnline 2.Script::RunPrepared" = 0x0051B9E0
 */

/* We insert a call to our code (drawIface) before the call to RunPrepared. */

void drawIface(FOClient* client)
{
    if (client->gameMode == IN_ENCOUNTER) {
        drawString("VTTA", 100, 100, 100, 100, GREEN, NORMAL, BORDER);
    }
}

DWORD drawIfaceInjAddress = 0x468ec2;
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
"mov eax, 0x0051B9E0;"
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
    }
}
