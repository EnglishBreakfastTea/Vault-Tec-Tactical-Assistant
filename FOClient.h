#ifndef _FOCLIENT_H
#define _FOCLIENT_H

#include <cstdint>
#include <string>

/* Structures and functions mapping to the FOnline client code. Plus some high-level wrappers. */

struct Item {
    uint32_t itemId;
};

struct Hand { // I don't know what this is, but it points to the held item, so I called it Hand.
    uint32_t handItemId; // Don't know how I should call it.
    Item* heldItem;
    // 4 * 2 = 8
    uint32_t _padding[10]; // 40
    // 8 + 40 = 48 = 4 * 12
    uint8_t mode; // at 0x30
};

struct Object; // from a Xander Root lying on the ground to a standing Wagon

struct Critter {
    uint32_t critterId;
    uint16_t _padding1;
    uint16_t x;
    uint16_t y;
    uint16_t _padding2;
    // 4 + 2 + 2 + 2 + 2 = 12 = 4 * 3
    uint32_t _padding3[1064]; // 4256
    // 4256 + 12 = 4268 = 4 * 1067
    Hand* hand;
};

struct HexManager {
    /* Returns the critter currently pointed by the mouse cursor or nullptr if there is none. */
    Critter* critterUnderMouse();

    /* Check if the distance between the player and the given critter is <= 1 hex (can be 0 if critter = player). */
    bool playerNear(Critter*);

    uint32_t _padding[72];
    uint32_t playerCritterId; // could be inside FOClient
    // 73 * 4 = 292
};

struct FOClient {
    /* Move the player to the given position. */
    void move(uint32_t x, uint32_t y);

    /* Attack the given critter using the in-game attack mode (e.g. burst) */
    void attack(uint32_t critterId);

    uint32_t _padding1[8]; // * 4 = 32
    HexManager hexManager; // at 0x20
    // 292 + 32 = 324 = 81 * 4
    uint32_t _padding2[62]; // * 4 = 248
    uint32_t gameMode; // at 0x23c; menu/world map/encounter/loading/etc.(?)
    uint32_t mouseMode; // at 0x240; click/move/attack/use on etc.
    // 4 + 4 + 248 + 324 = 580 = 145 * 4
    uint32_t _padding3[11621]; // * 4 = 46484
    Critter* playerCritter; // at 0xb7d8
    // 4 + 46484 + 580 = 47068 = 11767 * 4
};

struct FOWindow {
    uint32_t _padding[41]; // * 4 = 164
    uint32_t windowActive; // at 0xa4
    // 4 + 164 = 168 = 4 * 42
};

/* String structure mimicking the one passed to DrawText. */
struct DrawTextString {
    uint32_t _padding[6];
    const char* text; // at 0x18
};

/* Check if the distance between two critters is 1 hex. */
bool crittersNeighbours(Critter*, Critter*);

/* Wrapper for Global_DrawText using std::string for passing the text and ignoring the return value. */
void drawString(std::string, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                uint32_t color, uint32_t font, uint32_t flags);

/* Functions for accessing global structures. */
extern FOWindow* mainWindow();

/* In-game functions. */
using FastTick = uint32_t (_stdcall *)();
using IsAction = uint32_t (_thiscall *)(FOClient*, uint32_t);
using SetAction = void (_thiscall *)(FOClient*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
using GetSmthPixel = void (_thiscall *)(HexManager*, uint32_t mouseX, uint32_t mouseY, Object*&, Critter*&);
using GetCritter = Critter* (_thiscall *)(HexManager*, uint32_t critterId);
using Global_DrawText = uint32_t (_cdecl *)(DrawTextString*, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                            uint32_t color, uint32_t font, uint32_t flags);

extern FastTick fastTick;
extern IsAction isAction;
extern SetAction setAction;
extern GetSmthPixel getSmthPixel;
extern GetCritter getCritter;
extern Global_DrawText global_drawText;

/* Game modes. */
extern uint32_t const IN_MENU;
extern uint32_t const IN_ENCOUNTER;
extern uint32_t const ON_WORLD_MAP;
extern uint32_t const LOADING;

/* Mouse modes. */
extern uint32_t const MOUSE_POINT;
extern uint32_t const MOUSE_MOVE;
extern uint32_t const MOUSE_USE_ON;
extern uint32_t const MOUSE_ATTACK;
extern uint32_t const MOUSE_SKILL;
extern uint32_t const MOUSE_WAIT;
extern uint32_t const MOUSE_INVENTORY;

/* Flags passed to drawText. */
extern uint32_t const HALIGN_MIDDLE;
extern uint32_t const VALIGN_MIDDLE;
extern uint32_t const HALIGN_RIGHT;
extern uint32_t const VALIGN_RIGHT;
extern uint32_t const VALIGN_RIGHT;
extern uint32_t const BORDER;

/* Common colors passed to drawText. */
extern uint32_t const GREEN;
extern uint32_t const GRAY;

/* Common fonts passed to drawText. */
extern uint32_t const NORMAL; // Don't know how to call it.

/* Constant in-game pointers. */
extern uint32_t* const mouseX;
extern uint32_t* const mouseY;

#endif //_FOCLIENT_H
