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
        // 0 - item
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

    bool HasItems(Trigger::Base &trigger, Party::Base &party)
    {
        auto result = false;

        // variables
        // 0 .. N - item types
        if (Engine::IsAlive(party) && SafeCast(trigger.Variables.size()) > 1)
        {
            Items::List items = {};

            for (auto i = 0; i < SafeCast(trigger.Variables.size()); i++)
            {
                auto item = Item::MapType(trigger.Variables[i]);

                if (item != Item::NONE)
                {
                    items.push_back(item);
                }
            }

            if (SafeCast(items.size()) > 0)
            {
                if (trigger.Type == Trigger::Type::ANY_ITEMS)
                {
                    result = party.HasAnyItems(items);
                }
                else if (trigger.Type == Trigger::Type::ALL_ITEMS)
                {
                    result = party.HasAllItems(items);
                }
            }
        }

        return result;
    }
}
