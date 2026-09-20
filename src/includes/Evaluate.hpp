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

    bool AttributeTest(Graphics::Base &graphics, Graphics::Scenery scenes, Trigger::Base &trigger, Party::Base &party)
    {
        auto result = false;

        // variables
        // 0 - player/select
        // 1 - attribute
        // 2 - additional roll
        // 3 - additional modifier
        if (Engine::IsAlive(party) && SafeCast(trigger.Variables.size()) >= 2)
        {
            auto character = Interface::SelectCharacter(graphics, scenes, party, trigger.Variables[0]);

            if (character >= 0 && character < party.Count() && Engine::IsAlive(party[character]))
            {
                auto attribute = Attribute::MapAttribute(trigger.Variables[1]);

                if (attribute != Attribute::Type::NONE)
                {
                    auto roll = SafeCast(trigger.Variables.size() > 2) ? std::stoi(BloodSwordRogue::Trim(trigger.Variables[2]), nullptr, 10) : 0;

                    auto modifier = SafeCast(trigger.Variables.size() > 3) ? std::stoi(BloodSwordRogue::Trim(trigger.Variables[3]), nullptr, 10) : 0;

                    result = Interface::AttributeTest(graphics, scenes, party[character], attribute, roll, modifier);
                }
            }
        }

        return result;
    }
}
