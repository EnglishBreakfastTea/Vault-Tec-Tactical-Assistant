#ifndef _FOCLIENT_H
#define _FOCLIENT_H

#include <cstdint>

struct Item {
    uint32_t itemId;
};

struct Hand { // I don't know what this is, but it points to the held item, so I called it Hand.
    uint32_t handItemId; // ??
    Item* heldItem;
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
    uint32_t _padding[72];
    uint32_t playerCritterId; // could be inside FOClient
    // 73 * 4 = 292
};

struct FOClient {
    uint32_t _padding1[8]; // 8 * 4 = 32
    HexManager hexManager; // at 0x20
    // 32 + 292 = 324 = 81 * 4
    uint32_t _padding2[11685]; // 46740
    // 324 + 46740 = 47064 = 4 * 11766
    Critter* playerCritter; // at 0xb7d8
};

using FastTick = uint32_t (_stdcall *)();
using IsAction = uint32_t (_thiscall *)(FOClient*, uint32_t);
using SetAction = void (_thiscall *)(FOClient*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
using GetSmthPixel = void (_thiscall *)(HexManager*, uint32_t mouseX, uint32_t mouseY, Object*&, Critter*&);
using GetCritter = Critter* (_thiscall *)(HexManager*, uint32_t critterId);

uint32_t const FastTickAddr = 0x00538940;
uint32_t const IsActionAddr = 0x00472110;
uint32_t const SetActionAddr = 0x0048DDE0;
uint32_t const GetSmthPixelAddr = 0x00504C40;
uint32_t const GetCritterAddr = 0x004800C0;

uint32_t const MouseXAddr = 0x008520C4;
uint32_t const MouseYAddr = 0x008520C8;

#endif //_FOCLIENT_H
