#pragma once

#include "Utilities.hpp"

// item types
namespace BloodSwordRogue::Item
{
    const int NONE = -1;

    // item types
    typedef int Type;

    // item property
    typedef int Property;

    // item target effect types
    typedef int TargetEffect;

    // mapping of item types to strings
    BloodSwordRogue::StringMap<Item::Type> TypeMapping = {};

    // item properties
    BloodSwordRogue::StringMap<Item::Property> PropertyMapping = {};

    // item type mapping template
    template <typename T>
    using Mapped = BloodSwordRogue::UnorderedMap<Item::Type, T>;

    // melee weapon (range) requirements
    Item::Mapped<Item::Type> MeleeRequirements = {};

    // ranged weapon (ammunition) requirements
    Item::Mapped<Item::Type> RangedRequirements = {};

    // container requirements
    Item::Mapped<Item::Type> StorageRequirements = {};

    // list of item properties
    typedef std::vector<Item::Property> Properties;

    // list of item target effects
    typedef std::vector<Item::TargetEffect> TargetEffects;

    // invisible properties (hide from item description)
    Item::Properties Invisible = {};

    // map string to item type
    Item::Type MapType(std::string item)
    {
        return BloodSwordRogue::Find(Item::TypeMapping, item, Item::NONE);
    }

    // map string to item type
    Item::Type MapType(const char *item)
    {
        return Item::MapType(std::string(item));
    }

    // map string to item property
    Item::Property MapProperty(std::string property)
    {
        return BloodSwordRogue::Find(Item::PropertyMapping, property, Item::NONE);
    }

    // map string to item property
    Item::Property MapProperty(const char *property)
    {
        return Item::MapProperty(std::string(property));
    }

    // this property is invisible
    bool IsInvisible(Item::Property property)
    {
        return property != Item::NONE && BloodSwordRogue::Has(Item::Invisible, property);
    }

    // mapping of item target effects to strings
    BloodSwordRogue::StringMap<Item::TargetEffect> TargetEffectMapping = {};

    // map string to item target effect
    Item::TargetEffect MapTargetEffect(std::string target_effect)
    {
        return BloodSwordRogue::Find(Item::TargetEffectMapping, target_effect, Item::NONE);
    }

    // map string to item target effect
    Item::TargetEffect MapTargetEffect(const char *target_effect)
    {
        return Item::MapTargetEffect(std::string(target_effect));
    }

    bool Generate(std::string &json_file)
    {
        if (json_file.empty())
        {
            return false;
        }

        auto data = nlohmann::json::parse(json_file);

        // load item types
        LoadListMap(data, "item-types", Item::TypeMapping);

        // load melee requirements
        LoadMapping(data, "item-melee", Item::MeleeRequirements, MapType);

        // load ranged requirements
        LoadMapping(data, "item-ranged", Item::RangedRequirements, MapType);

        // load storage requirements
        LoadMapping(data, "item-storage", Item::StorageRequirements, MapType);

        // load item properties
        LoadListMap(data, "item-properties", Item::PropertyMapping);

        // load invisible properties
        LoadList(data, "invisible-properties", Item::Invisible, MapProperty);

        // load target effects
        LoadListMap(data, "target-effects", Item::TargetEffectMapping);

        return !Item::TypeMapping.empty() && !Item::PropertyMapping.empty();
    }

    // load item properties from a zip archive
    bool Load(std::string item_properties, const char *zip_file)
    {
        Item::TypeMapping.clear();

        Item::PropertyMapping.clear();

        Item::TargetEffectMapping.clear();

        Item::MeleeRequirements.clear();

        Item::RangedRequirements.clear();

        Item::StorageRequirements.clear();

        Item::Invisible.clear();

        auto result = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, item_properties) : Read(item_properties.c_str());

        if (!json_file.empty())
        {
            result = Generate(json_file);
        }

        return result;
    }

    // load item properties
    bool Load(std::string item_properties, std::string zip_file)
    {
        return Load(item_properties, zip_file.empty() ? nullptr : zip_file.c_str());
    }

    // load item properties
    bool Load(const char *item_properties)
    {
        return Load(std::string(item_properties), nullptr);
    }
}
