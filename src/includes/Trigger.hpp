#pragma once

#include "Primitives.hpp"

namespace BloodSwordRogue::Trigger
{
    // trigger types
    enum class Type
    {
        NONE = -1,
        CHARACTER,
        ITEM,
        ANY_ITEM,
        ALL_ITEMS,
        VICTORY,
        EXIT
    };

    BloodSwordRogue::ConstStrings<Trigger::Type> TypeMapping = {
        {Trigger::Type::NONE, "NONE"},
        {Trigger::Type::CHARACTER, "CHARACTER"},
        {Trigger::Type::ITEM, "ITEM"},
        {Trigger::Type::ANY_ITEM, "ANY ITEM"},
        {Trigger::Type::ALL_ITEMS, "ALL ITEMS"},
        {Trigger::Type::VICTORY, "VICTORY"},
        {Trigger::Type::EXIT, "EXIT"}};

    // map string to trigger type
    Trigger::Type Map(const char *trigger)
    {
        return BloodSwordRogue::Find(Trigger::TypeMapping, trigger);
    }

    // map string to trigger type
    Trigger::Type Map(std::string trigger)
    {
        return Trigger::Map(trigger.c_str());
    }

    // triggers
    class Base
    {
    public:
        // trigger type
        Trigger::Type Type = Trigger::Type::NONE;

        // location in map
        int X = -1;

        int Y = -1;

        // message to display (upon encountering)
        std::string EncounterMessage = std::string();

        // message to display (when successfully completed)
        std::string CompletedMessage = std::string();

        // message to display (trigger has been activated but not completed)
        std::string ActiveMessage = std::string();

        // trigger variables
        std::vector<std::string> Variables = {};

        // trigger has been activated (encountered for the first time)
        bool Activated = false;

        // trigger has been resolved
        bool Completed = false;

        Point Location()
        {
            return Point(X, Y);
        }

        Base() {}

        nlohmann::json Data()
        {
            nlohmann::json data;

            data["type"] = std::string(Trigger::TypeMapping[this->Type]);

            data["x"] = this->X;

            data["y"] = this->Y;

            data["encounter_message"] = this->EncounterMessage;

            data["completed_message"] = this->CompletedMessage;

            data["active_message"] = this->ActiveMessage;

            data["activated"] = this->Activated;

            data["completed"] = this->Completed;

            nlohmann::json variables;

            for (auto variable : this->Variables)
            {
                variables.push_back(variable);
            }

            data["variables"] = variables;

            return data;
        }
    };

    Trigger::Base Load(nlohmann::json &data)
    {
        auto trigger = Trigger::Base();

        trigger.Type = !data["type"].is_null() ? Trigger::Map(std::string(data["type"])) : Trigger::Type::NONE;

        trigger.X = !data["x"].is_null() ? int(data["x"]) : 0;

        trigger.Y = !data["y"].is_null() ? int(data["y"]) : 0;

        trigger.EncounterMessage = !data["encounter_message"].is_null() ? std::string(data["encounter_message"]) : std::string();

        trigger.CompletedMessage = !data["completed_message"].is_null() ? std::string(data["completed_message"]) : std::string();

        trigger.ActiveMessage = !data["active_message"].is_null() ? std::string(data["active_message"]) : std::string();

        trigger.Activated = !data["activated"].is_null() ? data["activated"].get<bool>() : false;

        trigger.Completed = !data["completed"].is_null() ? data["completed"].get<bool>() : false;

        // set variables
        if (!data["variables"].is_null() && data["variables"].is_array() && SafeCast(data["variables"].size()) > 0)
        {
            auto variables = std::vector<std::string>();

            for (auto i = 0; i < SafeCast(data["variables"].size()); i++)
            {
                variables.push_back(std::string(data["variables"][i]));
            }

            trigger.Variables = variables;
        }

        return trigger;
    }
}
