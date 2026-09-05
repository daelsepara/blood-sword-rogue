#pragma once

#include "Attribute.hpp"
#include "ItemProperties.hpp"
#include "Target.hpp"
#include "ZipFileLibrary.hpp"

// item (classes and functions)
namespace BloodSwordRogue::Item
{
    // unlimited quantity
    const int Unlimited = -1;

    // not found constant
    const int NotFound = -1;

    // item damage modifier
    struct Damage
    {
        int Value = 0;

        int Modifier = 0;

        bool IgnoreArmour = false;

        Damage() {}

        Damage(int value, int modifier) : Value(value), Modifier(modifier) {}

        Damage(int value, int modifier, bool ignore_armour) : Value(value), Modifier(modifier), IgnoreArmour(ignore_armour) {}
    };

    // item base class
    class Base
    {
    public:
        // attributes and modifiers
        BloodSwordRogue::IntegerMap<Attribute::Type> Attributes = {};

        // item properties
        Item::Properties Properties = {};

        // item type
        Item::Type Type = Item::NONE;

        // kind of items that can be stored in this container  (e.g. Money, Arrow)
        Item::Type Contains = Item::NONE;

        // amount/number of the items above it currently contains
        int Quantity = 0;

        // maximum number of items it can contain
        int Limit = Item::Unlimited;

        // item asset
        Asset::Type Asset = Asset::NONE;

        // item name
        std::string Name = std::string();

        // flag to check if it's revealed (i.e. with the SAGE)
        bool Revealed = false;

        // flag to see if it drops when used (e.g. thrown)
        bool Drops = false;

        // for encumbrance checks
        int Encumbrance = 1;

        // for specific targetting (damage rolls/modifiers)
        Target::Mapped<Item::Damage> DamageTypes = {};

        // modifiers (+/- damage rolls/modifiers)
        Target::Mapped<Item::Damage> DamageModifiers = {};

        // for specific targetting effects
        Target::Mapped<Item::TargetEffect> TargetEffects = {};

        Base() {}

        Base(const char *name,
             Item::Type type,
             BloodSwordRogue::IntegerMap<Attribute::Type> attributes,
             Item::Properties properties,
             Item::Type contains,
             int quantity,
             int limit,
             int asset) : Attributes(attributes),
                          Properties(properties),
                          Type(type),
                          Contains(contains),
                          Quantity(quantity),
                          Limit(limit),
                          Asset(asset),
                          Name(name) {}

        Base(const char *name,
             Item::Type type,
             BloodSwordRogue::IntegerMap<Attribute::Type> attributes,
             Item::Properties properties,
             int quantity,
             int limit,
             int asset) : Attributes(attributes),
                          Properties(properties),
                          Type(type),
                          Quantity(quantity),
                          Limit(limit),
                          Asset(asset),
                          Name(name) {}

        Base(const char *name,
             Item::Type type,
             Item::Properties properties,
             Item::Type contains,
             int quantity,
             int limit,
             Asset::Type asset) : Properties(properties),
                                  Type(type),
                                  Contains(contains),
                                  Quantity(quantity),
                                  Limit(limit),
                                  Asset(asset),
                                  Name(name) {}

        // check if item has this property
        bool HasProperty(Item::Property property)
        {
            return BloodSwordRogue::Found(this->Properties, property);
        }

        // this item contains a type of item and of sufficient quantity
        bool HasQuantity(Item::Type type, int quantity)
        {
            return (this->HasProperty(Item::MapProperty("CONTAINER")) && (this->Contains == type) && (this->Quantity >= quantity) && ((this->Limit != Item::Unlimited && quantity >= 1) || this->Limit == Item::Unlimited));
        }

        // check if item is of this type
        bool IsType(Item::Type item)
        {
            return (this->Type == item);
        }

        // is a charged item
        bool IsCharged(Item::Type item, Item::Type charge, int quantity)
        {
            return (this->HasProperty(Item::MapProperty("CONTAINER")) && this->IsType(item) && (this->Contains == charge) && (this->Quantity >= quantity) && ((this->Limit != Item::Unlimited && quantity >= 1) || this->Limit == Item::Unlimited));
        }

        // check if item has this attribute
        bool HasAttribute(Attribute::Type attribute)
        {
            return BloodSwordRogue::Has(this->Attributes, attribute);
        }

        // item has target-specific effect
        bool HasTargetEffect(Target::Type target)
        {
            return BloodSwordRogue::Has(this->TargetEffects, target);
        }

        // set target-specific effect
        void SetTargetEffect(Target::Type target, Item::TargetEffect effect)
        {
            this->TargetEffects.insert_or_assign(target, effect);
        }

        // add target-specific effect
        bool AddTargetEffect(Target::Type target, Item::TargetEffect effect)
        {
            if (!this->HasTargetEffect(target))
            {
                this->SetTargetEffect(target, effect);
            }

            return this->HasTargetEffect(target);
        }

        // remove target effect
        bool RemoveTargetEffect(Target::Type target)
        {
            if (this->HasTargetEffect(target))
            {
                this->TargetEffects.erase(target);
            }

            return !this->HasTargetEffect(target);
        }

        // item has target-specific damage type
        bool HasDamageType(Target::Type target)
        {
            return BloodSwordRogue::Has(this->DamageTypes, target);
        }

        // set damage type
        void SetDamageType(Target::Type target, Item::Damage damage)
        {
            this->DamageTypes.insert_or_assign(target, damage);
        }

        // add damage type
        bool AddDamageType(Target::Type target, Item::Damage damage)
        {
            if (!this->HasDamageType(target))
            {
                this->SetDamageType(target, damage);
            }

            return this->HasDamageType(target);
        }

        // remove damage type
        bool RemoveDamageType(Target::Type target)
        {
            if (this->HasDamageType(target))
            {
                this->DamageTypes.erase(target);
            }

            return !this->HasDamageType(target);
        }

        // item has target-specific damage modifiers
        bool HasDamageModifier(Target::Type target)
        {
            return BloodSwordRogue::Has(this->DamageModifiers, target);
        }

        // set damage modifier
        void SetDamageModifier(Target::Type target, Item::Damage damage)
        {
            this->DamageModifiers.insert_or_assign(target, damage);
        }

        // add damage modifier
        bool AddDamageModifier(Target::Type target, Item::Damage damage)
        {
            if (!this->HasDamageModifier(target))
            {
                this->SetDamageModifier(target, damage);
            }

            return this->HasDamageModifier(target);
        }

        // remove damage modifer
        bool RemoveDamageModifier(Target::Type target)
        {
            if (this->HasDamageModifier(target))
            {
                this->DamageModifiers.erase(target);
            }

            return !this->HasDamageModifier(target);
        }

        // item has all of the properties
        bool HasAll(Item::Properties properties)
        {
            auto has = true;

            for (auto &property : properties)
            {
                has &= this->HasProperty(property);
            }

            return has;
        }

        // item has any of the properties
        bool HasAny(Item::Properties properties)
        {
            auto has = false;

            for (auto &property : properties)
            {
                has |= this->HasProperty(property);
            }

            return has;
        }

        // check if this item contains this type of object
        bool ContainsItem(Item::Type item)
        {
            return this->HasQuantity(item, 1);
        }

        // add item (of quantity) to this container
        bool Add(Item::Type item, int quantity)
        {
            auto result = false;

            if (this->HasProperty(Item::MapProperty("CONTAINER")) && (this->Contains == item) && ((this->Limit == Item::Unlimited) || (((this->Quantity + quantity) <= this->Limit) && ((this->Quantity + quantity) >= 0))))
            {
                this->Quantity += quantity;

                // minimum
                this->Quantity = std::max(0, this->Quantity);

                if (this->Limit != Item::Unlimited)
                {
                    // maximum
                    this->Quantity = std::min(this->Quantity, this->Limit);
                }

                result = true;
            }

            return result;
        }

        // is the item charged? or does it have sufficient charge?
        bool IsCharged(Item::Type charge, int quantity)
        {
            return (this->Contains == charge && this->Quantity >= quantity);
        }

        // return attribute-modifier value of this item, if any
        int Modifier(Attribute::Type attribute)
        {
            auto modifier = 0;

            if (this->HasAttribute(attribute))
            {
                modifier = this->Attributes[attribute];
            }

            return modifier;
        }

        // set attribute-modifier value of this item
        void SetAttribute(Attribute::Type attribute, int modifier)
        {
            this->Attributes.insert_or_assign(attribute, modifier);
        }

        // add attribute to item
        bool AddAttribute(Attribute::Type attribute, int modifier)
        {
            auto result = !this->HasAttribute(attribute);

            if (result)
            {
                this->SetAttribute(attribute, modifier);
            }

            return this->HasAttribute(attribute);
        }

        // return attribute-modifier value of this item (with specific property), if any
        int Modifier(Attribute::Type attribute, Item::Property property)
        {
            auto modifier = 0;

            if (this->HasAttribute(attribute) && this->HasProperty(property))
            {
                modifier = this->Attributes[attribute];
            }

            return modifier;
        }

        // remove a quantity of the item from this container
        bool Remove(Item::Type type, int quantity)
        {
            auto result = false;

            if (this->HasQuantity(type, quantity))
            {
                result = this->Add(type, -quantity);
            }

            return result;
        }

        // remove one unit of this item
        bool Remove(Item::Type type)
        {
            return this->Remove(type, 1);
        }

        // remove item property, e.g. remove 'EQUIPPED' property
        bool RemoveProperty(Item::Property property)
        {
            auto result = this->HasProperty(property);

            if (result)
            {
                auto found = BloodSwordRogue::Find(this->Properties, property);

                if (found != this->Properties.end())
                {
                    this->Properties.erase(found);
                }
            }

            return !this->HasProperty(property);
        }

        // add property to item
        bool AddProperty(Item::Property property)
        {
            auto result = !this->HasProperty(property);

            if (result)
            {
                this->Properties.push_back(property);
            }

            return this->HasProperty(property);
        }

        // add properties to item
        bool AddProperties(Item::Properties properties)
        {
            auto result = false;

            for (auto &property : properties)
            {
                result |= this->AddProperty(property);
            }

            return result;
        }

        // remove item property, e.g. remove 'FIGHTING PROWESS' attribute, i.e. it will no longer modify
        // the bearer's FIGHTING PROWESS attribute
        bool RemoveAttribute(Attribute::Type attribute)
        {
            auto result = this->HasAttribute(attribute);

            if (result)
            {
                this->Attributes.erase(attribute);
            }

            return result;
        }

        // reveal item description
        void Reveal()
        {
            this->Revealed = true;
        }

        // hide item description
        void Hide()
        {
            this->Revealed = false;
        }

        // generrate string description with stats
        std::string String(bool newline = false)
        {
            auto item_string = this->Name;

            if (this->Type != Item::MapType("GOLD"))
            {
                if (newline)
                {
                    item_string += "\n";
                }
                else
                {
                    item_string += " ";
                }

                auto visible = 0;

                for (auto &property : this->Properties)
                {
                    visible += !Item::IsInvisible(property) ? 1 : 0;
                }

                if (SafeCast(this->Attributes.size()) > 0 || (SafeCast(this->Properties.size()) > 0 && visible > 0) || this->Quantity > 1 || this->Encumbrance > 1)
                {
                    auto stats = 0;

                    item_string += "(";

                    if (SafeCast(this->Properties.size()) > 0)
                    {
                        for (auto &property : this->Properties)
                        {
                            if (!Item::IsInvisible(property))
                            {
                                if (stats > 0)
                                {
                                    item_string += ", ";
                                }

                                item_string += Item::PropertyMapping[property];

                                stats++;
                            }
                        }
                    }

                    if (this->HasProperty(Item::MapProperty("CONTAINER")) && this->Contains != Item::NONE)
                    {
                        if (stats > 0)
                        {
                            item_string += ", ";
                        }

                        item_string += Item::TypeMapping[this->Contains] + ": " + std::to_string(this->Quantity);

                        stats++;
                    }

                    if (SafeCast(this->Attributes.size()) > 0)
                    {
                        for (auto &attribute : this->Attributes)
                        {
                            if (stats > 0)
                            {
                                item_string += ", ";
                            }

                            item_string += std::string(Attribute::Abbreviations[attribute.first]) + ": " + std::to_string(attribute.second);

                            stats++;
                        }
                    }

                    if ((SafeCast(this->Properties.size()) == 0 || !this->HasProperty(Item::MapProperty("CONTAINER"))) && this->Quantity > 1)
                    {
                        if (stats > 0)
                        {
                            item_string += ", ";
                        }

                        item_string += "QUANTITY: " + std::to_string(this->Quantity);
                    }

                    if (this->Encumbrance > 1)
                    {
                        if (stats > 0)
                        {
                            item_string += ", ";
                        }

                        item_string += "ENCUMBRANCE: " + std::to_string(this->Encumbrance);
                    }

                    item_string += ")";
                }
            }
            else
            {
                // GOLD pieces
                item_string = std::to_string(this->Quantity) + " " + item_string;
            }

            return item_string;
        }
    };

    // return requirement (e.g. ammo/charge) for this item
    Item::Type Requirements(Item::Type item, bool ranged = false)
    {
        auto requirement = Item::NONE;

        if (ranged)
        {
            requirement = BloodSwordRogue::Find(Item::RangedRequirements, item, Item::NONE);
        }
        else
        {
            requirement = BloodSwordRogue::Find(Item::MeleeRequirements, item, Item::NONE);
        }

        return requirement;
    }

    // get container for item
    Item::Type Container(Item::Type item)
    {
        auto container = BloodSwordRogue::Find(Item::StorageRequirements, item, Item::NONE);

        return container != Item::NONE ? container : item;
    }

    // load item attributes from json data
    BloodSwordRogue::IntegerMap<Attribute::Type> LoadAttributes(nlohmann::json &data)
    {
        BloodSwordRogue::IntegerMap<Attribute::Type> attributes = {};

        for (auto &[key, value] : data.items())
        {
            auto attribute = Attribute::MapAttribute(std::string(key));

            if (attribute != Attribute::Type::NONE)
            {
                attributes[attribute] = int(value);
            }
        }

        return attributes;
    }

    // load item properties from json data
    Item::Properties LoadProperties(nlohmann::json &data)
    {
        auto properties = Item::Properties();

        for (auto i = 0; i < SafeCast(data.size()); i++)
        {
            properties.push_back(Item::MapProperty(std::string(data[i])));
        }

        return properties;
    }

    // load item from json data
    Item::Base Load(nlohmann::json &data)
    {
        auto item = Item::Base();

        if (!data["attributes"].is_null() && data["attributes"].is_object())
        {
            item.Attributes = Item::LoadAttributes(data["attributes"]);
        }

        if (!data["properties"].is_null() && data["properties"].is_array() && SafeCast(data["properties"].size()) > 0)
        {
            item.Properties = Item::LoadProperties(data["properties"]);
        }

        item.Type = !data["type"].is_null() ? Item::MapType(std::string(data["type"])) : Item::NONE;

        item.Contains = !data["contains"].is_null() ? Item::MapType(std::string(data["contains"])) : Item::NONE;

        item.Quantity = !data["quantity"].is_null() ? int(data["quantity"]) : 0;

        item.Encumbrance = !data["encumbrance"].is_null() ? int(data["encumbrance"]) : 1;

        item.Limit = !data["limit"].is_null() ? int(data["limit"]) : Item::Unlimited;

        item.Asset = !data["asset"].is_null() ? Asset::Map(data["asset"]) : Asset::NONE;

        item.Name = !data["name"].is_null() ? std::string(data["name"]) : std::string();

        if (!data["damage-types"].is_null() && data["damage-types"].is_object())
        {
            item.DamageTypes.clear();

            for (auto &[key, val] : data["damage-types"].items())
            {
                if (val.is_object())
                {
                    auto target = Target::Map(std::string(key));

                    auto damage = !val["damage"].is_null() ? int(val["damage"]) : 0;

                    auto modifier = !val["modifier"].is_null() ? int(val["modifier"]) : 0;

                    auto ignore_armour = (!val["ignore_armour"].is_null() && val["ignore_armour"].is_boolean()) ? val["ignore_armour"].get<bool>() : false;

                    if (target != Target::NONE)
                    {
                        item.DamageTypes[target] = Item::Damage(damage, modifier, ignore_armour);
                    }
                }
            }
        }

        if (!data["damage-modifiers"].is_null() && data["damage-modifiers"].is_object())
        {
            item.DamageModifiers.clear();

            for (auto &[key, val] : data["damage-modifiers"].items())
            {
                if (val.is_object())
                {
                    auto target = Target::Map(std::string(key));

                    auto damage = !val["damage"].is_null() ? int(val["damage"]) : 0;

                    auto modifier = !val["modifier"].is_null() ? int(val["modifier"]) : 0;

                    auto ignore_armour = (!val["ignore_armour"].is_null() && val["ignore_armour"].is_boolean()) ? val["ignore_armour"].get<bool>() : false;

                    if (target != Target::NONE)
                    {
                        item.DamageModifiers[target] = Item::Damage(damage, modifier, ignore_armour);
                    }
                }
            }
        }

        if (!data["target-effects"].is_null() && data["target-effects"].is_object())
        {
            item.TargetEffects.clear();

            for (auto &[key, val] : data["target-effects"].items())
            {
                auto target = Target::Map(std::string(key));

                auto effect = Item::MapTargetEffect(std::string(val));

                if (target != Target::NONE && effect != Item::NONE)
                {
                    item.TargetEffects[target] = effect;
                }
            }
        }

        // check whether or not description has been revealed
        item.Revealed = (!data["revealed"].is_null() && data["revealed"].is_boolean()) ? data["revealed"].get<bool>() : false;

        item.Drops = (!data["drops"].is_null() && data["drops"].is_boolean()) ? data["drops"].get<bool>() : false;

        return item;
    }
}

// functions and classes for handling items or group/list  of items
namespace BloodSwordRogue::Items
{
    // default stats/properties for in-game items
    Item::Mapped<Item::Base> Defaults = {};

    // collection of items
    typedef std::vector<Item::Base> Inventory;

    // list of item types
    typedef std::vector<Item::Type> List;

    // constant for unlimited quantity
    const int Unlimited = -1;

    // load inventory from json data
    Items::Inventory Load(nlohmann::json &data)
    {
        auto items = Items::Inventory();

        for (auto i = 0; i < SafeCast(data.size()); i++)
        {
            auto item = Item::Load(data[i]);

            if (item.Type == Item::NONE)
            {
                SDL_Log("[ITEM %s] [NOT LOADED] [MISSING TYPE %s]", std::string(data[i]["name"]).c_str(), std::string(data[i]["type"]).c_str());
            }
            else if (SafeCast(item.Name.size()) > 0)
            {
                items.push_back(item);
            }
        }

        return items;
    }

    // generate inventory json data
    nlohmann::json Data(Items::Inventory &items)
    {
        nlohmann::json data;

        for (auto &item : items)
        {
            nlohmann::json row;

            if (SafeCast(item.Attributes.size()) > 0)
            {
                nlohmann::json attributes;

                for (auto &attribute : item.Attributes)
                {
                    auto attribute_name = std::string(Attribute::TypeMapping[attribute.first]);

                    attributes.emplace(attribute_name, attribute.second);
                }

                row["attributes"] = attributes;
            }

            if (SafeCast(item.Properties.size()) > 0)
            {
                nlohmann::json properties;

                for (auto &property : item.Properties)
                {
                    properties.push_back(std::string(Item::PropertyMapping[property]));
                }

                row["properties"] = properties;
            }

            row["name"] = item.Name;

            row["asset"] = Asset::TypeMapping[item.Asset];

            row["contains"] = Item::TypeMapping[item.Contains];

            row["type"] = Item::TypeMapping[item.Type];

            row["quantity"] = item.Quantity;

            row["encumbrance"] = item.Encumbrance;

            row["revealed"] = item.Revealed;

            if (SafeCast(item.DamageTypes.size()) > 0)
            {
                nlohmann::json damage_types;

                for (auto &damage : item.DamageTypes)
                {
                    nlohmann::json damage_type;

                    auto target = Target::Mapping[damage.first];

                    auto value = damage.second.Value;

                    auto modifier = damage.second.Modifier;

                    damage_type.emplace("value", value);

                    damage_type.emplace("modifier", modifier);

                    damage_types.emplace(target, damage_type);
                }

                row["damage-types"] = damage_types;
            }

            if (SafeCast(item.TargetEffects.size()) > 0)
            {
                nlohmann::json target_effects;

                for (auto &targets : item.TargetEffects)
                {
                    nlohmann::json target_effect;

                    auto target = Target::Mapping[targets.first];

                    auto effect = std::string(Item::TargetEffectMapping[targets.second]);

                    target_effects.emplace(target, effect);
                }

                if (SafeCast(target_effects.size()) > 0)
                {
                    row["target-effects"] = target_effects;
                }
            }

            data.push_back(row);
        }

        return data;
    }

    // load items from json data
    void Initialize(nlohmann::json &data)
    {
        // clear global map
        Items::Defaults.clear();

        if (!data["items"].is_null() && data["items"].is_array() && SafeCast(data["items"].size()) > 0)
        {
            auto items = Items::Load(data["items"]);

            for (auto &item : items)
            {
                if (item.Type != Item::NONE)
                {
                    Items::Defaults.insert({item.Type, item});
                }
            }

#ifdef DEBUG
            SDL_Log("[LOADED] %d items", SafeCast(Items::Defaults.size()));
#endif
        }
    }

    // load items from file
    void Load(const char *items, const char *zip_file)
    {
        auto ifs = zip_file != nullptr ? ZipFile::Read(zip_file, items) : Read(items);

        if (!ifs.empty())
        {
            auto data = nlohmann::json::parse(ifs);

            Items::Initialize(data);
        }
    }

    // load items from file
    void Load(std::string filename, std::string zip_file)
    {
        Items::Load(filename.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    // load items from file
    void Load(const char *items)
    {
        Load(items, nullptr);
    }

    // load items from file
    void Load(std::string filename)
    {
        Items::Load(filename.c_str());
    }

    // this item has a default settings
    bool Found(Item::Type item)
    {
        return BloodSwordRogue::Has(Items::Defaults, item);
    }

    // find item which contains type
    Items::Inventory::iterator FindItems(Items::Inventory &items, Item::Type container, Item::Type type)
    {
        auto result = items.end();

        for (auto item = items.begin(); item != items.end(); item++)
        {
            if (item->Type == container && item->HasProperty(Item::MapProperty("CONTAINER")) && item->Contains == type)
            {
                result = item;

                break;
            }
        }

        return result;
    }

    // has any item with all the properties
    Items::Inventory::iterator FindItemProperties(Items::Inventory &items, Item::Properties properties)
    {
        auto result = items.end();

        for (auto item = items.begin(); item != items.end(); item++)
        {
            auto has = item->HasAll(properties);

            if (has)
            {
                result = item;

                break;
            }
        }

        return result;
    }

    // has any item with all the types
    Items::Inventory::iterator FindTypes(Items::Inventory &items, Item::Type type)
    {
        auto result = items.end();

        for (auto item = items.begin(); item != items.end(); item++)
        {
            auto has = (*item).Type == type;

            if (has)
            {
                result = item;

                break;
            }
        }

        return result;
    }

    // add item to inventory
    void Add(Items::Inventory &items, Item::Base item)
    {
        if (item.Type != Item::NONE && SafeCast(item.Name.size()) > 0)
        {
            auto is_container = false;

            auto container = Item::NONE;

            // check if container
            for (auto i = 0; i < SafeCast(items.size()); i++)
            {
                if (items[i].HasProperty(Item::MapProperty("CONTAINER")) && items[i].Contains == item.Type)
                {
                    is_container = true;

                    container = items[i].Type;

                    break;
                }
            }

            if (is_container && container != Item::NONE)
            {
                auto found = Items::FindItems(items, container, item.Type);

                if (found != items.end())
                {
                    (*found).Quantity += item.Quantity;

                    (*found).Quantity = std::min(0, (*found).Quantity);

                    if ((*found).Limit != Item::Unlimited)
                    {
                        (*found).Quantity = std::max((*found).Quantity, (*found).Limit);
                    }
                }
            }
            else
            {
                items.push_back(item);
            }
        }
    }

    // check if item is in the list
    bool Included(Items::List list, Item::Type item)
    {
        auto included = false;

        if (item != Item::NONE)
        {
            for (auto i = 0; i < SafeCast(list.size()); i++)
            {
                if (list[i] == item)
                {
                    included = true;

                    break;
                }
            }
        }

        return included;
    }
}
