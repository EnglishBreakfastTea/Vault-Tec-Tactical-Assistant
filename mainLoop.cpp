#include <cstdio>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/adapted/std_pair.hpp>

#include "State.h"
#include "HexAttack.h"
#include "Hotkeys.h"
#include "BackgroundJob.h"
#include "mainLoop.h"

void toggle1hex()
{
    auto name = "1hex";

    if (mouseHookInstalled(name)) {
        removeMouseHook(name);
    } else {
        installMouseHook(name, HexAttackHook);
    }
}

void toggle1hexv2()
{
    std::map<uint32_t, uint32_t> static patches;

    auto static getHeldItem = [](FOClient* client) -> Item* {
        auto player = client->playerCritter;
        if (!player) {
            return nullptr;
        }

        auto hand = player->hand;
        if (!hand) {
            return nullptr;
        }

        auto heldItem = hand->heldItem;
        if (!heldItem) {
            return nullptr;
        }

        return heldItem;
    };

    auto static HexPatchHook = [](FOClient* client) {
        auto heldItem = getHeldItem(client);
        if (!heldItem) {
            return false;
        }

        auto rangeIx = client->playerCritter->hand->mode & 0xF;
        auto rangeAddress = reinterpret_cast<uint32_t>(&heldItem->range[rangeIx]);
        if (patches.find(rangeAddress) != std::end(patches)) {
            // Patch already installed.
            return false;
        }

        // We patch the item's range, changing it to 1.
        patches[rangeAddress] = heldItem->range[rangeIx];
        heldItem->range[rangeIx] = 1;

        return false;
    };

    auto static HexUnpatchHook = [](FOClient* client) {
        auto heldItem = getHeldItem(client);
        if (!heldItem) {
            return false;
        }

        auto rangeIx = client->playerCritter->hand->mode & 0xF;
        auto rangeAddress = reinterpret_cast<uint32_t>(&heldItem->range[rangeIx]);
        auto patch = patches.find(rangeAddress);
        if (patch == std::end(patches)) {
            // No patch applied.
            return false;
        }

        // We remove the patch, changing the item's range to the original one.
        heldItem->range[rangeIx] = patch->second;
        patches.erase(patch);
        return false;
    };

    if (mouseHookInstalled("1hexpatch")) {
        assert(!mouseHookInstalled("1hexunpatch"));

        removeMouseHook("1hexpatch");
        installMouseHook("1hexunpatch", HexUnpatchHook);
    } else {
        removeMouseHook("1hexunpatch");
        installMouseHook("1hexpatch", HexPatchHook);
    }
}

void toggleCenter()
{
    auto name = "center";
    auto job = [](FOClient* client) {
        if (client->gameMode != IN_ENCOUNTER) {
            return;
        }
        client->center();
    };

    if (jobRunning(name)) {
        stopJob(name);
    } else {
        startJob(name, job);
    }
}

void mainLoop(FOClient* client)
{
    if (!state) {
        setup(state);
        installHotkeyHook({1, 1, 0, 'V'}, [](FOClient*) { printf("VTTA\n"); });
    }

    runJobs(client);

    auto msg = state->getMessage();

    if (!msg) {
        if (state->complexAction) {
            auto actionFinished = state->complexAction->frame(client);
            if (actionFinished) {
                printf("Complex action finished\n");
                state->complexAction = nullptr;
            }
        }

        return;
    }

    // Any received messages stop the current ComplexAction. For now...
    state->complexAction = nullptr;

    printf("received message: %s\n", msg->data());

    // TODO: smarter message handling dispatch.
    {
        uint32_t x, y;
        if (sscanf(msg->data(), "move %u %u", &x, &y) == 2) {
            printf("a move message\n");
            client->move(x, y);
            printf("called move\n");
            return;
        }
    }

    {
        if (*msg == "attack") {
            printf("attack message\n");

            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                return;
            }

            client->attack(critter->critterId);

            return;
        }
    }

    {
        if (*msg == "check near") {
            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                printf("no critter under mouse\n");
                return;
            }

            if (client->hexManager.playerNear(critter)) {
                printf("near!\n");
            } else {
                printf("not near\n");
            }

            return;
        }
    }

    {
        if (*msg == "1hex") {
            auto critter = client->hexManager.critterUnderMouse();
            if (!critter) {
                return;
            }

            state->complexAction = std::make_unique<HexAttack>(critter->critterId);

            return;
        }
    }

    {
        if (*msg == "toggle 1hex") {
            toggle1hex();

            return;
        }
    }

    {
        if (*msg == "center") {
            client->center();
            return;
        }
    }

    {
        using namespace boost::spirit;
        using qi::ascii::char_;
        using qi::ascii::space_type;
        using qi::ascii::space;
        using result_t = std::pair<std::pair<std::tuple<bool, bool, bool>, char>, std::string>;

        auto it = std::begin(*msg);
        auto end = std::end(*msg);
        result_t result;
        qi::rule<std::string::iterator, result_t, space_type> rule =
            "bind" >> lexeme[-(char_('c') ^ char_('a') ^ char_('s')) >> '_' >> char_]
                   >> lexeme['"' >> +(char_ - '"') >> '"'];

        qi::phrase_parse(it, end, rule, space, result);
        if (it == end) {
            bool ctrl, alt, shift;
            std::tie(ctrl, alt, shift) = result.first.first;
            auto ch = result.first.second;

            auto hotkey = Hotkey{ctrl, alt, shift, ch};
            auto command = result.second;

            if (command == "toggle 1hex") {
                installHotkeyHook(hotkey, [](FOClient*) { toggle1hex(); });
            }

            if (command == "toggle center") {
                installHotkeyHook(hotkey, [](FOClient*) { toggleCenter(); });
            }

            if (command == "center") {
                installHotkeyHook(hotkey, [](FOClient* client) { client->center(); });
            }

            if (command == "toggle 1hexv2") {
                installHotkeyHook(hotkey, [](FOClient*) { toggle1hexv2(); });
            }

            return;
        }
    }
}
