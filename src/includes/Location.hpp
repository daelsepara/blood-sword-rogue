#pragma once

#include "Battlepits.hpp"
#include "Interface.hpp"
#include "Trigger.hpp"

// classes and functions to define rogue-like game locations
namespace BloodSwordRogue::Location
{
    // items in location
    class Loot
    {
    public:
        // location in map
        int X = -1;

        int Y = -1;

        // items in this location
        Items::Inventory Items = {};

        Point Location()
        {
            return Point(X, Y);
        }

        Loot() {}

        Loot(Point point) : X(point.X), Y(point.Y) {}

        nlohmann::json Data()
        {
            nlohmann::json data;

            data["X"] = this->X;

            data["Y"] = this->Y;

            data["items"] = Items::Data(this->Items);

            return data;
        }
    };

    // location base class
    class Base
    {
    public:
        // name
        std::string Name = std::string();

        // map
        Map::Base Map = Map::Base();

        // groups of enemies in current map
        std::vector<Party::Base> Opponents = {};

        // groups of items in current map
        std::vector<Location::Loot> Loot = {};

        // groups of triggers in current map
        std::vector<Trigger::Base> Triggers = {};

        Base() {}
    };

    // renumber remaining map stashes (loot)
    void RenumberLoot(Location::Base &location)
    {
        auto &map = location.Map;

        if (SafeCast(location.Loot.size()) > 0)
        {
            auto current = 1;

            // renumber remaining stashes
            for (auto &loot : location.Loot)
            {
                SDL_Log("[UPDATE LOOT %d] [NEW ID %d]", map[loot.Location()].Id, current);

                map[loot.Location()].Id = current;

                current++;
            }
        }
    }

    // renumber remaining map triggers
    void RenumberTriggers(Location::Base &location)
    {
        auto &map = location.Map;

        if (SafeCast(location.Triggers.size()) > 0)
        {
            auto current = 1;

            // renumber remaining triggers
            for (auto &trigger : location.Triggers)
            {
                SDL_Log("[UPDATE TRIGGER %d] [NEW ID %d]", map[trigger.Location()].Id, current);

                map[trigger.Location()].Id = current;

                current++;
            }
        }
    }

    // load location
    void Setup(Location::Base &location, nlohmann::json &data)
    {
        location.Name = !data["name"].is_null() ? std::string(data["name"]) : std::string();

        if (!data["map"].is_null())
        {
            location.Map.Setup(data["map"]);
        }

        location.Opponents.clear();

        if (!data["opponents"].is_null() && data["opponents"].is_array() && data["opponents"].size() > 0)
        {
            for (auto i = 0; i < SafeCast(data["opponents"].size()); i++)
            {
                auto opponent = Party::Base();

                opponent.Load(data["opponents"][i]);

                location.Opponents.push_back(opponent);
            }
        }

        location.Loot.clear();

        if (!data["loot"].is_null() && data["loot"].is_array() && data["loot"].size() > 0)
        {
            for (auto i = 0; i < SafeCast(data["loot"].size()); i++)
            {
                auto loot = Loot();

                loot.X = !data["loot"][i]["x"].is_null() ? int(data["loot"][i]["x"]) : -1;

                loot.Y = !data["loot"][i]["y"].is_null() ? int(data["loot"][i]["y"]) : -1;

                if (!data["loot"][i]["items"].is_null() && data["loot"][i]["items"].is_array() && data["loot"][i]["items"].size() > 0)
                {
                    loot.Items = Items::Load(data["loot"][i]["items"]);
                }

                location.Loot.push_back(loot);
            }
        }

        location.Triggers.clear();

        if (!data["triggers"].is_null() && data["triggers"].is_array() && data["triggers"].size() > 0)
        {
            for (auto i = 0; i < SafeCast(data["triggers"].size()); i++)
            {
                auto trigger = Trigger::Load(data["triggers"][i]);

                location.Triggers.push_back(trigger);
            }
        }
    }

    // renumber remaining map occupants
    void RenumberParties(Location::Base &location)
    {
        Interface::RenumberParties(location.Map, location.Opponents);
    }

    // generate location json data
    nlohmann::json Data(Location::Base &location)
    {
        nlohmann::json data;

        data["name"] = location.Name;

        data["map"] = location.Map.Data();

        nlohmann::json items;

        for (auto &loot : location.Loot)
        {
            items.push_back(loot.Data());
        }

        data["loot"] = items;

        nlohmann::json opponents;

        for (auto &opponent : location.Opponents)
        {
            opponents.push_back(Party::Data(opponent));
        }

        data["opponents"] = opponents;

        nlohmann::json triggers;

        for (auto &trigger : location.Triggers)
        {
            triggers.push_back(trigger.Data());
        }

        data["triggers"] = triggers;

        return data;
    }

    // copy location
    void Setup(Location::Base &location, Location::Base &source)
    {
        auto data = Location::Data(source);

        Location::Setup(location, data);
    }

    bool Load(Location::Base &location, const char *filename, const char *zip_file)
    {
        auto loaded = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, filename) : Read(filename);

        if (!json_file.empty())
        {
            auto data = nlohmann::json::parse(json_file);

            Location::Setup(location, data["location"]);

            loaded = true;
        }

        return loaded;
    }

    bool Load(Location::Base &location, std::string filename, std::string zip_file)
    {
        return Location::Load(location, filename.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    Location::Base Load(const char *filename, const char *zip_file)
    {
        auto location = Location::Base();

        Location::Load(location, filename, zip_file);

        return location;
    }

    Location::Base Load(std::string filename, std::string zip_file)
    {
        return Location::Load(filename.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    void Save(Location::Base &location, const char *filename)
    {
        nlohmann::json data;

        data["location"] = Location::Data(location);

        std::ofstream ofs(filename);

        if (ofs.is_open())
        {
            ofs << data.dump();

            ofs.close();
        }
    }
}
