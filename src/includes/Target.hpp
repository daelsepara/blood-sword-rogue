#pragma once

#include "Utilities.hpp"

// for specific targetting
namespace BloodSwordRogue::Target
{
    // target not found
    const int NotFound = -1;

    const int NONE = -1;

    // target types
    typedef int Type;

    // list of target types
    typedef std::vector<Target::Type> List;

    // spell type mapping template
    template <typename T>
    using Mapped = BloodSwordRogue::UnorderedMap<Target::Type, T>;

    // mapping of target types to strings
    BloodSwordRogue::StringMap<Target::Type> Mapping = {};

    // map string to target type
    Target::Type Map(std::string target)
    {
        return BloodSwordRogue::Find(Target::Mapping, target, Target::NONE);
    }

    // map string to target type
    Target::Type Map(const char *target)
    {
        return Target::Map(std::string(target));
    }

    // load list of target types from json data
    Target::List LoadTargets(nlohmann::json &data)
    {
        auto targets = Target::List();

        for (auto i = 0; i < SafeCast(data.size()); i++)
        {
            auto target = !data[i].is_null() ? Target::Map(std::string(data[i])) : Target::NONE;

            if (target != Target::NONE)
            {
                targets.push_back(target);
            }
        }

        return targets;
    }

    // generate json data from list of target types
    nlohmann::json Data(Target::List &targets)
    {
        nlohmann::json data;

        for (auto &target : targets)
        {
            if (target != Target::NONE)
            {
                data.push_back(Target::Mapping[target]);
            }
        }

        return data;
    }

    bool Generate(std::string &json_file)
    {
        if (json_file.empty())
        {
            return false;
        }

        auto data = nlohmann::json::parse(json_file);

        // load item types
        LoadListMap(data, "target-types", Target::Mapping);

        return !Target::Mapping.empty();
    }

    // load target types from a zip archive
    bool Load(std::string target_types, const char *zip_file)
    {
        Target::Mapping.clear();

        auto result = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, target_types) : Read(target_types.c_str());

        if (!json_file.empty())
        {
            result = Generate(json_file);
        }

        return result;
    }

    // load target types from a file
    bool Load(std::string target_types, std::string zip_file)
    {
        return Load(target_types.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    // load target types
    bool Load(const char *target_types)
    {
        return Load(target_types, nullptr);
    }
}
