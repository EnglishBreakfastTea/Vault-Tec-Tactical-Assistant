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
    uint8_t mode;
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

    uint32_t _padding1[8]; // 8 * 4 = 32
    HexManager hexManager; // at 0x20
    // 32 + 292 = 324 = 81 * 4
    uint32_t _padding2[11685]; // 46740
    // 324 + 46740 = 47064 = 4 * 11766
    Critter* playerCritter; // at 0xb7d8
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

/* Constant in-game addresses. */
extern uint32_t* const mouseX;
extern uint32_t* const mouseY;

#endif //_FOCLIENT_H
