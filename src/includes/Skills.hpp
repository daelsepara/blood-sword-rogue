#pragma once

#include "AssetTypes.hpp"
#include "Utilities.hpp"

// functions for managing character skills
namespace BloodSwordRogue::Skills
{
    const int NONE = -1;

    // skill types
    typedef int Type;

    // skill type mapping template
    template <typename T>
    using Mapped = BloodSwordRogue::UnorderedMap<Skills::Type, T>;

    // skill type to string mapping
    BloodSwordRogue::StringMap<Skills::Type> TypeMapping = {};

    // list of skills
    typedef std::vector<Skills::Type> List;

    // battle skills
    Skills::List BattleSkills = {};

    // ranged attack skills
    Skills::List RangedAttack = {};

    // story skills
    Skills::List StorySkills = {};

    // story skills
    Skills::List AllSkills = {};

    // mapping of skills to asset type ids
    Asset::Lookup<Skills::Type> Assets = {};

    // mapping of skills to asset names
    BloodSwordRogue::StringMap<Skills::Type> AssetNames = {};

    // map all skill types to asset ids
    void MapAssets()
    {
        Asset::MapTypes(Skills::Assets, Skills::AssetNames);
    }

    // get skill type from string
    Skills::Type Map(std::string skill)
    {
        return BloodSwordRogue::Find(Skills::TypeMapping, skill, Skills::NONE);
    }

    // get skill type from string
    Skills::Type Map(const char *skill)
    {
        return Skills::Map(std::string(skill));
    }

    // is this skill in the list?
    bool In(Skills::List &list, Skills::Type skill)
    {
        return SafeCast(list.size()) > 0 && BloodSwordRogue::Has(list, skill);
    }

    // is this skill a battle skill?
    bool IsBattleSkill(Skills::Type skill)
    {
        return Skills::In(Skills::BattleSkills, skill);
    }

    // is this skill a ranged attack skill?
    bool IsRangedAttack(Skills::Type skill)
    {
        return Skills::In(Skills::RangedAttack, skill);
    }

    // is this skill a story skill?
    bool IsStorySkill(Skills::Type skill)
    {
        return Skills::In(Skills::StorySkills, skill);
    }

    // load list of skills from json data
    Skills::List Load(nlohmann::json &data)
    {
        auto skills = Skills::List();

        for (auto i = 0; i < SafeCast(data.size()); i++)
        {
            auto skill = !data[i].is_null() ? Skills::Map(std::string(data[i])) : Skills::NONE;

            if (skill != Skills::NONE)
            {
                skills.push_back(skill);
            }
        }

        return skills;
    }

    // generate json data from list of skills
    nlohmann::json Data(Skills::List &skills)
    {
        nlohmann::json data;

        for (auto &skill : skills)
        {
            if (skill != Skills::NONE)
            {
                data.push_back(Skills::TypeMapping[skill]);
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

        // load skills types
        LoadListMap(data, "skill-types", Skills::TypeMapping);

        // load battle skills
        LoadList(data, "battle-skills", BattleSkills, Map);

        // load ranged attack skills
        LoadList(data, "ranged-attacks", RangedAttack, Map);

        // load story skills
        LoadList(data, "story-skills", StorySkills, Map);

        // load skill asset mappoing
        LoadMapping(data, "skill-assets", AssetNames, Map);

        // load all (editable skills)
        LoadList(data, "all-skills", AllSkills, Map);

        // map skills to asset names
        MapAssets();

        // load target effects
        return !Skills::TypeMapping.empty();
    }

    // load skills from a zip archive
    bool Load(std::string skills, const char *zip_file)
    {
        Skills::TypeMapping.clear();

        Skills::BattleSkills.clear();

        Skills::RangedAttack.clear();

        Skills::StorySkills.clear();

        Skills::AssetNames.clear();

        auto result = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, skills) : Read(skills.c_str());

        if (!json_file.empty())
        {
            result = Generate(json_file);
        }

        return result;
    }

    // load skills
    bool Load(const char *skills)
    {
        return Load(skills, nullptr);
    }

    // load skills
    bool Load(std::string skills, std::string zip_file)
    {
        return zip_file.empty() ? Load(skills.c_str()) : Load(skills.c_str(), zip_file.c_str());
    }
}
