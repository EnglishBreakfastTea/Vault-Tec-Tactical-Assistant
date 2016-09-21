#include "FOClient.h"

#include <cstdio>
#include <cassert>
#include <algorithm>
#include <array>

void FOClient::move(uint32_t x, uint32_t y)
{
    auto time = fastTick();
    auto moving = isAction(this, 1);
    auto ACTION_MOVE = 1u;
    auto run = 1u;
    // reverse x, y?
    setAction(this, ACTION_MOVE, x, y, run, 0, !moving, time);
}

void FOClient::attack(uint32_t critterId)
{
    auto ACTION_ATTACK = 6u;

    if (!playerCritter) {
        printf("no player critter!\n");
        return;
    }

    auto hand = playerCritter->hand;
    if (!hand) {
        printf("what, no hand\n");
        return;
    }

    auto mode = hand->mode;

    auto heldItem = hand->heldItem;
    if (!heldItem) {
        // an empty hand should also be an item
        printf("no hand item\n");
        return;
    }

    setAction(this, ACTION_ATTACK, hand->handItemId, heldItem->itemId, 2, critterId, mode, 0);
}

Critter* HexManager::critterUnderMouse()
{
    Object* obj = nullptr;
    Critter* critter = nullptr;
    getSmthPixel(this, *mouseX, *mouseY, obj, critter);

    if (!critter) {
        return nullptr;
    }

    return critter;
}

bool crittersNeighbours(Critter* critter1, Critter* critter2)
{
    assert(critter1);
    assert(critter2);

    // A critter will always be near itself.
    auto static evenXDeltas = std::array<std::pair<int, int>, 7>
            {{{0, 0}, {1,0}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
    auto static oddXDeltas = std::array<std::pair<int, int>, 7>
            {{{0, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {0, 1}, {1, 0}}};

    if (critter1->x % 2 == 0) {
        return std::any_of(std::begin(evenXDeltas), std::end(evenXDeltas), [critter1, critter2](auto delta) {
            return critter1->x + delta.first == critter2->x
                   && critter1->y + delta.second == critter2->y;
        });
    }

    return std::any_of(std::begin(oddXDeltas), std::end(oddXDeltas), [critter1, critter2](auto delta) {
        return critter1->x + delta.first == critter2->x
               && critter1->y + delta.second == critter2->y;
    });
}

bool HexManager::playerNear(Critter* critter)
{
    assert(critter);

    auto playerCritter = getCritter(this, playerCritterId);
    return crittersNeighbours(critter, playerCritter);
}

void drawString(std::string str, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                uint32_t color, uint32_t font, uint32_t flags)
{
    DrawTextString _str{{0}, str.data()};
    global_drawText(&_str, x, y, width, height, color, font, flags);
}

uint32_t constexpr FastTickAddr = 0x00538940;
uint32_t constexpr IsActionAddr = 0x00472110;
uint32_t constexpr SetActionAddr = 0x0048DDE0;
uint32_t constexpr GetSmthPixelAddr = 0x00504C40;
uint32_t constexpr GetCritterAddr = 0x004800C0;
uint32_t constexpr DrawTextAddr = 0x004763E0;

uint32_t constexpr MouseXAddr = 0x008520C4;
uint32_t constexpr MouseYAddr = 0x008520C8;

FastTick fastTick = reinterpret_cast<FastTick>(FastTickAddr);
IsAction isAction = reinterpret_cast<IsAction>(IsActionAddr);
SetAction setAction = reinterpret_cast<SetAction>(SetActionAddr);
GetSmthPixel getSmthPixel = reinterpret_cast<GetSmthPixel>(GetSmthPixelAddr);
GetCritter getCritter = reinterpret_cast<GetCritter>(GetCritterAddr);
Global_DrawText global_drawText = reinterpret_cast<Global_DrawText>(DrawTextAddr);

uint32_t const IN_MENU = 1;
uint32_t const IN_ENCOUNTER = 5;
uint32_t const ON_WORLD_MAP = 6;
uint32_t const LOADING = 7;

uint32_t const MOUSE_POINT = 0;
uint32_t const MOUSE_MOVE = 1;
uint32_t const MOUSE_USE_ON = 2;
uint32_t const MOUSE_ATTACK = 3;
uint32_t const MOUSE_SKILL = 4;
uint32_t const MOUSE_WAIT = 5;
uint32_t const MOUSE_INVENTORY = 6;

uint32_t const HALIGN_MIDDLE = 4;
uint32_t const VALIGN_MIDDLE = 8;
uint32_t const HALIGN_RIGHT = 16;
uint32_t const VALIGN_RIGHT = 32;
uint32_t const BORDER = 512;

uint32_t const GREEN = 0xFF00AA00;
uint32_t const GRAY =  0xFFADADB9;

uint32_t const NORMAL = 5;

uint32_t* const mouseX = reinterpret_cast<uint32_t*>(MouseXAddr);
uint32_t* const mouseY = reinterpret_cast<uint32_t*>(MouseYAddr);
