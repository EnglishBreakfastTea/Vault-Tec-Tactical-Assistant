#ifndef _FOCLIENT_H
#define _FOCLIENT_H

#include <cstdint>

struct HexManager {};

struct FOClient {
    uint32_t _padding[8];
    HexManager hexManager; // at 0x20
};

struct Object; // from a Xander Root lying on the ground to a standing Wagon
struct Critter {
    uint32_t critterId;
};

using FastTick = uint32_t (_stdcall *)();
using IsAction = uint32_t (_thiscall *)(FOClient*, uint32_t);
using SetAction = void (_thiscall *)(FOClient*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
using GetSmthPixel = void (_thiscall *)(HexManager*, uint32_t mouseX, uint32_t mouseY, Object*&, Critter*&);

uint32_t const FastTickAddr = 0x00538940;
uint32_t const IsActionAddr = 0x00472110;
uint32_t const SetActionAddr = 0x0048DDE0;
uint32_t const GetSmthPixelAddr = 0x00504C40;

uint32_t const MouseXAddr = 0x008520C4;
uint32_t const MouseYAddr = 0x008520C8;

#endif //_FOCLIENT_H
