#include <cstdio>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/adapted/std_pair.hpp>

#include "State.h"
#include "HexAttack.h"
#include "Hotkeys.h"
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

void mainLoop(FOClient* client)
{
    if (!state) {
        setup(state);
        installHotkeyHook({1, 1, 0, 'V'}, [](FOClient*) { printf("VTTA\n"); });
    }

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

            if (command == "center") {
                installHotkeyHook(hotkey, [](FOClient* client) { client->center(); });
            }

            return;
        }
    }
}
