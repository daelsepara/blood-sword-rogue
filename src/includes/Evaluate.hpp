#pragma once

#include "Engine.hpp"
#include "Location.hpp"

namespace BloodSwordRogue::Evaluate
{
    bool InParty(Trigger::Base &trigger, Party::Base &party)
    {
        auto result = false;

        // variables
        // 0 - player
        if (Engine::IsAlive(party) && SafeCast(trigger.Variables.size()) > 0)
        {
            if (party.HasCharacter(trigger.Variables[0]))
            {
                auto character = party.FindCharacter(trigger.Variables[0]);

                if (character >= 0 && character < party.Count())
                {
                    result = Engine::IsAlive(party[character]);
                }
            }
        }

        return result;
    }

    bool HasItem(Trigger::Base &trigger, Party::Base &party)
    {
        auto result = false;

        // variables
        // 0 - player
        if (Engine::IsAlive(party) && SafeCast(trigger.Variables.size()) > 0)
        {
            auto item = Item::MapType(trigger.Variables[0]);

            if (item != Item::NONE)
            {
                result = party.HasItemType(item);
            }
        }

        return result;
    }
}
