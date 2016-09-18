#ifndef _FOCLIENT_H
#define _FOCLIENT_H

#include <cstdint>

struct FOClient;

using FastTick = uint32_t (_stdcall *)();
using IsAction = uint32_t (_thiscall *)(FOClient*, uint32_t);
using SetAction = void (_thiscall *)(FOClient*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

uint32_t const FastTickAddr = 0x00538940;
uint32_t const IsActionAddr = 0x00472110;
uint32_t const SetActionAddr = 0x0048DDE0;

#endif //_FOCLIENT_H
