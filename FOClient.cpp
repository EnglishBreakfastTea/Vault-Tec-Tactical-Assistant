#include "FOClient.h"

#include <cstdio>
#include <cassert>
#include <algorithm>
#include <array>

/* Move the player to the given position. */
void FOClient::move(uint32_t x, uint32_t y)
{
    auto time = fastTick();
    auto moving = isAction(this, 1);
    auto ACTION_MOVE = 1u;
    auto run = 1u;
    // reverse x, y?
    setAction(this, ACTION_MOVE, x, y, run, 0, !moving, time);
}

/* Attack the given critter using the in-game attack mode (e.g. burst) */
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
        return std::any_of(std::begin(evenXDeltas), std::end(evenXDeltas), [=](auto delta) {
            return critter1->x + delta.first == critter2->x
                   && critter1->y + delta.second == critter2->y;
        });
    }

    return std::any_of(std::begin(oddXDeltas), std::end(oddXDeltas), [=](auto delta) {
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

uint32_t constexpr FastTickAddr = 0x00538940;
uint32_t constexpr IsActionAddr = 0x00472110;
uint32_t constexpr SetActionAddr = 0x0048DDE0;
uint32_t constexpr GetSmthPixelAddr = 0x00504C40;
uint32_t constexpr GetCritterAddr = 0x004800C0;

uint32_t constexpr MouseXAddr = 0x008520C4;
uint32_t constexpr MouseYAddr = 0x008520C8;

FastTick fastTick = reinterpret_cast<FastTick>(FastTickAddr);
IsAction isAction = reinterpret_cast<IsAction>(IsActionAddr);
SetAction setAction = reinterpret_cast<SetAction>(SetActionAddr);
GetSmthPixel getSmthPixel = reinterpret_cast<GetSmthPixel>(GetSmthPixelAddr);
GetCritter getCritter = reinterpret_cast<GetCritter>(GetCritterAddr);

uint32_t* const mouseX = reinterpret_cast<uint32_t*>(MouseXAddr);
uint32_t* const mouseY = reinterpret_cast<uint32_t*>(MouseYAddr);
