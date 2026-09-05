#include "includes/BloodSwordRogue.hpp"

// BloodSwordRogue Map Maker
namespace BloodSwordRogue::MapMaker
{
    enum class Mode
    {
        EDIT,
        ERASE,
        FILL,
        CLEAR
    };

    enum class Function
    {
        TILE,
        ORIGIN,
        TRIGGER,
        LOOT,
        ENEMY,
        RESIZE,
        GENERATE
    };

    BloodSwordRogue::StringMap<MapMaker::Mode> ModeText = {
        {Mode::EDIT, "EDIT"},
        {Mode::ERASE, "ERASE"},
        {Mode::FILL, "FILL"},
        {Mode::CLEAR, "CLEAR"}};

    Asset::List Assets = {};

    Asset::List Numbers = {};

    BloodSwordRogue::UnorderedMap<Target::Type, Character::Base> Roster = {};

    Asset::List RosterAssets = {};

    std::vector<std::string> RosterCaptions = {};

    Target::List RosterTargets = {};

    Asset::List ItemAssets = {};

    Items::List ItemTypes = {};

    std::vector<std::string> ItemCaptions = {};

    std::vector<std::string> ItemsWithQuantities = {"ARROW", "FOOD", "SHURIKEN", "GOLD", "QUIVER", "POUCH", "STEEL SCEPTRE", "LIMITED SHURIKEN"};

    std::vector<std::string> ItemPlurals = {"ARROWS", "FOOD PORTIONS", "SHURIKENS", "GOLD PIECES", "ARROWS", "GOLD PIECES", "CHARGES", "SHURIKEN"};

    std::vector<std::string> ItemsAssets = {"ARROWS", "FOOD", "SHURIKEN", "MONEY", "ARROWS", "MONEY", "POWER LIGHTNING", "SHURIKEN"};

    std::vector<std::string> TriggerTypes = {};

    BloodSwordRogue::UnorderedMap<Attribute::Type, Asset::Type> AttributeAssets = {};

    BloodSwordRogue::UnorderedMap<Item::Property, Asset::Type> PropertyAssets = {};

    const int MaxEnemies = 5;

    const int MaxItems = 10;

    // generic number setter
    int SetValue(Graphics::Base &graphics, Graphics::Scenery &scenery, std::string asset, int value, int min_value, int max_value)
    {
        return Interface::SetValue(graphics, scenery, MapMaker::Numbers, asset, value, min_value, max_value);
    }

    // generic number setter
    int SetValue(Graphics::Base &graphics, Graphics::Scenery &scenery, std::string asset, int value)
    {
        return Interface::SetValue(graphics, scenery, MapMaker::Numbers, asset, value);
    }

    // renders the current map
    void RenderMap(Graphics::Base &graphics, Scene::Base &scene, Location::Base &location)
    {
        auto &map = location.Map;

        auto items_default = Asset::Get("ITEMS");

        // control id for map tiles
        auto id = 0;

        for (auto y = map.Y; y < map.Y + map.ViewY; y++)
        {
            for (auto x = map.X; x < map.X + map.ViewX; x++)
            {
                auto offset = Point(x - map.X, y - map.Y);

                auto &tile = map[Point(x, y)];

                auto screen = Point(map.DrawX, map.DrawY) + offset * map.TileSize;

                auto loot_id = -1;

                auto opponent_id = -1;

                // add occupant
                if (tile.IsOccupied())
                {
                    switch (tile.Occupant)
                    {
                    case Map::Object::PARTY:

                        scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("CHARACTER")), screen));

                        break;

                    case Map::Object::ENEMIES:

                        opponent_id = Location::FindOpponents(location, Point(x, y));

                        if (opponent_id >= 0 && opponent_id < SafeCast(location.Opponents.size()) && SafeCast(location.Opponents.size()) > 0)
                        {
                            if (Engine::IsAlive(location.Opponents[opponent_id]))
                            {
                                auto first = Engine::First(location.Opponents[opponent_id]);

                                auto &enemy = location.Opponents[opponent_id][first];

                                if (Engine::IsAlive(enemy))
                                {
                                    scene.VerifyAndAdd(Scene::Element(Asset::Get(enemy.Asset), screen));
                                }
                            }
                        }

                        break;

                    case Map::Object::TEMPORARY_OBSTACLE:

                        if (tile.Lifetime > 0 && tile.TemporaryAsset != Asset::NONE)
                        {
                            scene.VerifyAndAdd(Scene::Element(Asset::Get(tile.TemporaryAsset), screen));
                        }
                        else if (tile.Asset != Asset::NONE)
                        {
                            scene.VerifyAndAdd(Scene::Element(Asset::Get(tile.Asset), screen));
                        }

                        break;

                    case Map::Object::ITEMS:

                        loot_id = Location::FindLoot(location, Point(x, y));

                        if (loot_id >= 0 && loot_id < SafeCast(location.Loot.size()) && SafeCast(location.Loot.size()) > 0)
                        {
                            auto &loot = location.Loot[loot_id];

                            if (SafeCast(loot.Items.size()) > 0)
                            {
                                auto first = Engine::FirstAsset(loot.Items);

                                if (first != Item::NotFound)
                                {
                                    auto &item = loot.Items[first];

                                    scene.VerifyAndAdd(Scene::Element(Asset::Get(item.Asset), screen));
                                }
                                else
                                {
                                    scene.VerifyAndAdd(Scene::Element(items_default, screen));
                                }
                            }
                        }

                    case Map::Object::TRIGGER:

                        if (tile.Asset != Asset::NONE)
                        {
                            scene.VerifyAndAdd(Scene::Element(Asset::Get(tile.Asset), screen));
                        }

                        break;

                    default:

                        break;
                    }
                }
                else if (tile.Asset != Asset::NONE)
                {
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(tile.Asset), screen));
                }

                if (tile.Type != Map::Object::PASSABLE)
                {
                    // indicate that the tile is not passable
                    scene.Add(Scene::Element(screen.X + 4, screen.Y + 4, map.TileSize - 8, map.TileSize - 8, Color::Transparent, Color::Highlight, 2));
                }

                if (tile.Occupant == Map::Object::TRIGGER)
                {
                    // indicate that the tile is a trigger point
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("SELECT TRANSPARENT RED")), screen));
                }

                // check if point is marked as an origin (party)
                if (SafeCast(map.Origins.size()) > 0)
                {
                    if (Point(x, y) == map.Origins[0])
                    {
                        scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("SELECT TRANSPARENT BLUE")), screen));
                    }
                }

                auto type = Controls::MapType("SELECT");

                scene.Add(Controls::Base(type, id, id, id, id, id, screen.X, screen.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active, x, y));

                id++;
            }
        }
    }

    void LoadRoster(nlohmann::json &data, const char *key)
    {
        MapMaker::Roster.clear();

        MapMaker::RosterAssets.clear();

        MapMaker::RosterCaptions.clear();

        MapMaker::RosterTargets.clear();

        if (!data[key].is_null() && data[key].is_object())
        {
            for (const auto &[k, v] : data[key].items())
            {
                auto target = Target::Map(std::string(k));

                if (target != Target::NONE)
                {
                    MapMaker::Roster[target] = Character::Load(v);

                    if (!Engine::IsAlive(MapMaker::Roster[target]))
                    {
                        MapMaker::Roster.erase(target);
                    }
                    else
                    {
                        SDL_Log("[LOADED ROSTER %s] [%s]", key, MapMaker::Roster[target].Name.c_str());
                    }
                }
            }
        }

        // generate roster assets and captions
        if (!MapMaker::Roster.empty())
        {
            for (auto &roster : MapMaker::Roster)
            {
                auto enemy = roster.second;

                if (enemy.Asset != Asset::NONE)
                {
                    MapMaker::RosterAssets.push_back(enemy.Asset);

                    MapMaker::RosterCaptions.push_back(enemy.Name);

                    MapMaker::RosterTargets.push_back(roster.first);
                }
            }
        }
    }

    void SetupItems()
    {
        MapMaker::ItemAssets.clear();

        MapMaker::ItemTypes.clear();

        MapMaker::ItemCaptions.clear();

        for (auto &item : Items::Defaults)
        {
            auto type = item.first;

            auto asset = item.second.Asset;

            if (type != Item::NONE && asset != Asset::NONE)
            {
                MapMaker::ItemAssets.push_back(asset);

                MapMaker::ItemTypes.push_back(type);

                MapMaker::ItemCaptions.push_back(item.second.Name);
            }
        }

        MapMaker::Numbers.clear();

        MapMaker::Numbers = {
            Asset::Map("ZERO"),
            Asset::Map("ONE"),
            Asset::Map("TWO"),
            Asset::Map("THREE"),
            Asset::Map("FOUR"),
            Asset::Map("FIVE"),
            Asset::Map("SIX"),
            Asset::Map("SEVEN"),
            Asset::Map("EIGHT"),
            Asset::Map("NINE")};

        MapMaker::AttributeAssets.clear();

        MapMaker::AttributeAssets = {
            {Attribute::Type::FIGHTING_PROWESS, Asset::Map("FIGHT")},
            {Attribute::Type::AWARENESS, Asset::Map("BRAIN")},
            {Attribute::Type::PSYCHIC_ABILITY, Asset::Map("CALL TO MIND")},
            {Attribute::Type::ENDURANCE, Asset::Map("ENDURANCE")},
            {Attribute::Type::DAMAGE, Asset::Map("BLOOD")},
            {Attribute::Type::ARMOUR, Asset::Map("LAYERED ARMOUR")}};

        MapMaker::PropertyAssets.clear();

        MapMaker::PropertyAssets = {
            {Item::MapProperty("EQUIPPED"), Asset::Map("EQUIPPED")},
            {Item::MapProperty("WEAPON"), Asset::Map("WEAPON")},
            {Item::MapProperty("ARMOUR"), Asset::Map("ARMOUR")},
            {Item::MapProperty("ACCESSORY"), Asset::Map("ACCESSORY")},
            {Item::MapProperty("MELEE"), Asset::Map("MELEE")},
            {Item::MapProperty("RANGED"), Asset::Map("RANGED")},
            {Item::MapProperty("RUSTY"), Asset::Map("RUSTY")},
            {Item::MapProperty("BROKEN"), Asset::Map("BROKEN")},
            {Item::MapProperty("POISONED"), Asset::Map("POISONED")},
            {Item::MapProperty("CURSED"), Asset::Map("CURSED")},
            {Item::MapProperty("RESURRECTION"), Asset::Map("RESURRECTION")},
            {Item::MapProperty("EDIBLE"), Asset::Map("EDIBLE")},
            {Item::MapProperty("PRIMARY"), Asset::Map("PRIMARY")},
            {Item::MapProperty("SECONDARY"), Asset::Map("SECONDARY")},
            {Item::MapProperty("INVISIBLE"), Asset::Map("INVISIBLE")},
            {Item::MapProperty("CANNOT DROP"), Asset::Map("CANNOT DROP")},
            {Item::MapProperty("CANNOT TRADE"), Asset::Map("CANNOT TRADE")},
            {Item::MapProperty("CONTAINER"), Asset::Map("CONTAINER")},
            {Item::MapProperty("READABLE"), Asset::Map("READABLE")},
            {Item::MapProperty("LIQUID"), Asset::Map("LIQUID")},
            {Item::MapProperty("ALL RANGES"), Asset::Map("ALL RANGES")},
            {Item::MapProperty("REQUIRES TARGET"), Asset::Map("REQUIRES TARGET")},
            {Item::MapProperty("COMBAT"), Asset::Map("COMBAT")}};

        MapMaker::TriggerTypes.clear();

        for (auto &trigger : Trigger::TypeMapping)
        {
            if (trigger.first != Trigger::Type::NONE)
            {
                MapMaker::TriggerTypes.push_back(std::string(trigger.second));
            }
        }
    }

    // load map maker settings json string
    bool Load(std::string &json_file)
    {
        if (json_file.empty())
        {
            return false;
        }

        auto data = nlohmann::json::parse(json_file);

        LoadList(data, "map-assets", MapMaker::Assets, Asset::Map);

        LoadRoster(data, "enemies");

        SetupItems();

        return !MapMaker::Assets.empty() && MapMaker::Roster.size() > 0;
    }

    // load map maker settings from a zip archive
    bool Load(std::string map_maker, const char *zip_file)
    {
        MapMaker::Assets.clear();

        auto result = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, map_maker) : Read(map_maker.c_str());

        if (!json_file.empty())
        {
            result = Load(json_file);
        }

        return result;
    }

    // load map maker settings
    bool Load(const char *map_maker)
    {
        return Load(map_maker, nullptr);
    }

    // load map maker settings
    bool Load(std::string map_maker, std::string zip_file)
    {
        return zip_file.empty() ? Load(map_maker.c_str()) : Load(map_maker.c_str(), zip_file.c_str());
    }

    // check if this is an item that requires a quantity to be set
    void CheckQuantity(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item)
    {
        for (auto i = 0; i < SafeCast(MapMaker::ItemsWithQuantities.size()); i++)
        {
            auto item_quantity = MapMaker::ItemsWithQuantities[i];

            if (Item::MapType(item_quantity) == item.Type)
            {
                auto question = std::string("HOW MANY ") + MapMaker::ItemPlurals[i] + std::string(" TO ADD?");

                auto quantity = BloodSwordRogue::Trim(Interface::TextInput(graphics, scenes, question, true));

                if (BloodSwordRogue::IsANumber(quantity))
                {
                    item.Quantity = std::stoi(quantity, nullptr, 10);

                    // clamp quantity to a reasonable range
                    item.Quantity = std::min(std::max(0, item.Quantity), item.Limit != Item::Unlimited ? item.Limit : 99);

                    if (item.Contains != Item::NONE)
                    {
                        SDL_Log("[UPDATE %s] [QUANTITY %d]", Item::TypeMapping[item.Contains].c_str(), item.Quantity);
                    }
                    else
                    {
                        SDL_Log("[UPDATE %s] [QUANTITY %d]", Item::TypeMapping[item.Type].c_str(), item.Quantity);
                    }
                }

                break;
            }
        }
    }

    // check if we need to modify the quantity for this item
    void CheckQuantity(Graphics::Base &graphics, Scene::Base &scene, Item::Base &item)
    {
        Graphics::Scenery scenes = {scene};

        MapMaker::CheckQuantity(graphics, scenes, item);
    }

    // select character skill
    Skills::Type SelectSkill(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        auto selected_skill = Skills::NONE;

        Asset::List assets = {};

        Skills::List skills = {};

        std::vector<std::string> captions = {};

        for (auto &skill : character.Skills)
        {
            assets.push_back(Skills::Assets[skill]);

            captions.push_back(Skills::TypeMapping[skill]);

            skills.push_back(skill);
        }

        if (SafeCast(skills.size()) > 0)
        {
            auto selected = Interface::IconGrid(graphics, scenes, assets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, captions);

            if (selected >= 0 && selected < SafeCast(skills.size()))
            {
                selected_skill = skills[selected];
            }
        }

        return selected_skill;
    }

    // add skill to character
    void AddSkill(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        Asset::List assets = {};

        Skills::List skills = {};

        std::vector<std::string> captions = {};

        for (auto skill : Skills::AllSkills)
        {
            if (!character.HasSkill(skill))
            {
                assets.push_back(Skills::Assets[skill]);

                captions.push_back(Skills::TypeMapping[skill]);

                skills.push_back(skill);
            }
        }

        if (SafeCast(skills.size()) > 0)
        {
            while (true)
            {
                auto selected = Interface::IconGrid(graphics, scenes, assets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, captions);

                if (selected >= 0 && selected < SafeCast(skills.size()))
                {
                    if (character.HasSkill(skills[selected]))
                    {
                        Interface::MessageBox(graphics, scenes, character.Name + std::string(" ALREADY HAS THE ") + Skills::TypeMapping[skills[selected]] + std::string(" SKILL"), Color::Highlight);
                    }
                    else
                    {
                        character.AddSkill(skills[selected]);

                        Interface::MessageBox(graphics, scenes, character.Name + std::string(" GAINS THE ") + Skills::TypeMapping[skills[selected]] + std::string(" SKILL"), Color::Active);
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    // remove skill from character
    void RemoveSkill(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        while (true && SafeCast(character.Skills.size()) > 0)
        {
            auto selected = MapMaker::SelectSkill(graphics, scenes, character);

            if (selected != Skills::NONE)
            {
                character.RemoveSkill(selected);

                Interface::MessageBox(graphics, scenes, character.Name + std::string(" LOSES THE ") + Skills::TypeMapping[selected] + std::string(" SKILL"), Color::Active);
            }
            else
            {
                break;
            }
        }
    }

    // generic add item
    void AddItem(Graphics::Base &graphics, Graphics::Scenery &scenes, Items::Inventory &items, Item::Base &item)
    {
        MapMaker::CheckQuantity(graphics, scenes, item);

        items.push_back(item);

        Interface::MessageBox(graphics, scenes, std::string("ADDED ") + item.Name, Color::Active);
    }

    // add item to character
    void AddItem(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        if (character.TotalEncumbrance() >= character.EncumbranceLimit)
        {
            Interface::MessageBox(graphics, scenes, character.Name + std::string(" REACHED THE ENCUMBRANCE LIMIT!"), Color::Highlight);

            return;
        }

        while (true && character.TotalEncumbrance() < character.EncumbranceLimit)
        {
            auto selected = Interface::IconGrid(graphics, scenes, MapMaker::ItemAssets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, MapMaker::ItemCaptions);

            if (selected >= 0 && selected < SafeCast(MapMaker::ItemTypes.size()))
            {
                auto item = Items::Defaults[MapMaker::ItemTypes[selected]];

                if ((character.TotalEncumbrance() + item.Encumbrance) <= character.EncumbranceLimit)
                {
                    MapMaker::AddItem(graphics, scenes, character.Items, item);
                }
                else
                {
                    Interface::MessageBox(graphics, scenes, character.Name + std::string(" REACHED THE ENCUMBRANCE LIMIT!"), Color::Highlight);
                }
            }
            else
            {
                break;
            }
        }
    }

    // item details
    void ViewItem(Graphics::Base &graphics, Graphics::Scenery scenes, Item::Base &item)
    {
        if (item.Asset == Asset::NONE || item.Type == Item::NONE)
        {
            return;
        }

        Asset::List assets = {};

        std::vector<std::string> captions = {};

        assets.push_back(item.Asset);

        captions.push_back(Item::TypeMapping[item.Type]);

        assets.push_back(Asset::Map("WEIGHT"));

        captions.push_back(std::string("ENCUMBRANCE: ") + std::to_string(item.Encumbrance));

        // show this item's quantity or any it contains
        for (auto i = 0; i < SafeCast(MapMaker::ItemsWithQuantities.size()); i++)
        {
            auto item_quantity = MapMaker::ItemsWithQuantities[i];

            if (Item::MapType(item_quantity) == item.Type)
            {
                auto asset_quantity = Asset::Map(MapMaker::ItemsAssets[i]);

                assets.push_back(asset_quantity);

                if (assets[0] == asset_quantity)
                {
                    assets[0] = Asset::Map("ITEMS");
                }

                captions.push_back(std::to_string(item.Quantity) + std::string(" ") + MapMaker::ItemPlurals[i]);
            }
        }

        // show item attributes
        for (auto attribute : item.Attributes)
        {
            assets.push_back(AttributeAssets[attribute.first]);

            auto caption = Attribute::TypeMapping[attribute.first] + std::string(": ") + std::to_string(item.Modifier(attribute.first));

            captions.push_back(caption);
        }

        // show item properties
        for (auto &properties : MapMaker::PropertyAssets)
        {
            auto property = properties.first;

            if (item.HasProperty(property))
            {
                assets.push_back(properties.second);

                captions.push_back(Item::PropertyMapping[property]);
            }
        }

        Interface::IconGrid(graphics, scenes, assets, BloodSwordRogue::TileSize * 8, BloodSwordRogue::TileSize * 6, captions);
    }

    // edit item attributes
    void EditAttributes(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item)
    {
        auto done = false;

        Controls::List actions = {
            Controls::MapType("EDIT"),
            Controls::MapType("ADD"),
            Controls::MapType("REMOVE")};

        Asset::List assets = {
            Asset::Map("GEARS"),
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL")};

        Asset::List attribute_assets = {};

        std::vector<std::string> attribute_captions = {};

        std::vector<Attribute::Type> item_attributes = {};

        for (auto &attribute : MapMaker::AttributeAssets)
        {
            attribute_assets.push_back(attribute.second);

            attribute_captions.push_back(Attribute::TypeMapping[attribute.first]);

            item_attributes.push_back(attribute.first);
        }

        while (!done)
        {
            std::vector<std::string> attributes = {};

            for (auto attribute : item.Attributes)
            {
                attributes.push_back(Attribute::TypeMapping[attribute.first]);
            }

            auto action = Interface::TextAction(graphics, scenes, attributes, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, actions, assets);

            if (action.Action == Controls::MapType("ADD"))
            {
                auto selected = Interface::IconGrid(graphics, scenes, attribute_assets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, attribute_captions);

                if (selected >= 0 && selected < SafeCast(item_attributes.size()))
                {
                    auto attribute = item_attributes[selected];

                    auto asset = MapMaker::AttributeAssets[attribute];

                    if (!item.HasAttribute(attribute))
                    {
                        auto modifier = MapMaker::SetValue(graphics, scenes, Asset::TypeMapping[asset], 0, Attribute::MinModifier, Attribute::MaxModifier);

                        if (modifier >= Attribute::MinModifier && modifier <= Attribute::MaxModifier)
                        {
                            item.AddAttribute(attribute, modifier);
                        }
                    }
                    else
                    {
                        auto modifier = MapMaker::SetValue(graphics, scenes, Asset::TypeMapping[asset], item.Attributes[attribute], Attribute::MinModifier, Attribute::MaxModifier);

                        if (modifier >= Attribute::MinModifier && modifier <= Attribute::MaxModifier)
                        {
                            item.SetAttribute(attribute, modifier);
                        }
                    }
                }
            }
            else if (action.Action == Controls::MapType("EDIT"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(attributes.size()))
                {
                    auto attribute = Attribute::MapAttribute(attributes[action.Selected]);

                    auto asset = MapMaker::AttributeAssets[attribute];

                    auto modifier = MapMaker::SetValue(graphics, scenes, Asset::TypeMapping[asset], item.Attributes[attribute], Attribute::MinModifier, Attribute::MaxModifier);

                    if (modifier >= Attribute::MinModifier && modifier <= Attribute::MaxModifier)
                    {
                        item.SetAttribute(attribute, modifier);
                    }
                }
            }
            else if (action.Action == Controls::MapType("REMOVE"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(attributes.size()))
                {
                    auto attribute = Attribute::MapAttribute(attributes[action.Selected]);

                    item.RemoveAttribute(attribute);
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // edit item properties
    void EditProperties(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item)
    {
        auto done = false;

        Controls::List actions = {
            Controls::MapType("ADD"),
            Controls::MapType("REMOVE")};

        Asset::List assets = {
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL")};

        Asset::List property_assets = {};

        std::vector<std::string> property_captions = {};

        Item::Properties item_properties = {};

        for (auto &property : MapMaker::PropertyAssets)
        {
            property_assets.push_back(property.second);

            property_captions.push_back(Item::PropertyMapping[property.first]);

            item_properties.push_back(property.first);
        }

        while (!done)
        {
            std::vector<std::string> properties = {};

            for (auto property : item.Properties)
            {
                properties.push_back(Item::PropertyMapping[property]);
            }

            auto action = Interface::TextAction(graphics, scenes, properties, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, actions, assets);

            if (action.Action == Controls::MapType("ADD"))
            {
                auto property = Interface::IconGrid(graphics, scenes, property_assets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, property_captions);

                if (property >= 0 && property < SafeCast(item_properties.size()))
                {
                    item.AddProperty(item_properties[property]);
                }
            }
            else if (action.Action == Controls::MapType("REMOVE"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(properties.size()))
                {
                    auto property = Item::MapProperty(properties[action.Selected]);

                    if (item.HasProperty(property))
                    {
                        item.RemoveProperty(property);
                    }
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // edit item target-specific effects
    void EditTargetEffects(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item)
    {
        auto done = false;

        Controls::List actions = {
            Controls::MapType("EDIT"),
            Controls::MapType("ADD"),
            Controls::MapType("REMOVE")};

        Asset::List assets = {
            Asset::Map("GEARS"),
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL")};

        std::vector<std::string> target_types = {};

        std::vector<std::string> effect_types = {};

        for (auto &target_type : Target::Mapping)
        {
            target_types.push_back(target_type.second);
        }

        for (auto &target_effect : Item::TargetEffectMapping)
        {
            effect_types.push_back(target_effect.second);
        }

        while (!done)
        {
            std::vector<std::string> targets = {};

            for (auto target_effect : item.TargetEffects)
            {
                targets.push_back(Target::Mapping[target_effect.first]);
            }

            auto action = Interface::TextAction(graphics, scenes, targets, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, actions, assets);

            if (action.Action == Controls::MapType("ADD"))
            {
                auto target = Interface::TextList(graphics, scenes, target_types, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, Asset::Map("TARGETING"), Controls::MapType("CONFIRM"));

                if (target >= 0 && target < SafeCast(target_types.size()))
                {
                    auto target_type = Target::Map(target_types[target]);

                    if (!item.HasTargetEffect(target_type))
                    {
                        auto target_effect = Interface::TextList(graphics, scenes, effect_types, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, Asset::Map("TARGETING"), Controls::MapType("CONFIRM"));

                        if (target_effect >= 0 && target_effect < SafeCast(effect_types.size()))
                        {
                            item.AddTargetEffect(target_type, Item::MapTargetEffect(effect_types[target_effect]));
                        }
                    }
                    else
                    {
                        auto selected = -1;

                        for (auto effect_type = 0; effect_type < SafeCast(effect_types.size()); effect_type++)
                        {
                            auto effect = Item::TargetEffectMapping[item.TargetEffects[target_type]];

                            if (effect_types[effect_type] == effect.c_str())
                            {
                                selected = effect_type;

                                break;
                            }
                        }

                        auto target_effect = Interface::TextList(graphics, scenes, effect_types, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, Asset::Map("CONFIRM"), Controls::MapType("CONFIRM"), selected);

                        if (target_effect >= 0 && target_effect < SafeCast(effect_types.size()))
                        {
                            item.SetTargetEffect(target_type, Item::MapTargetEffect(effect_types[target_effect]));
                        }
                    }
                }
            }
            else if (action.Action == Controls::MapType("EDIT"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(targets.size()))
                {
                    auto target_type = Target::Map(targets[action.Selected]);

                    auto selected = -1;

                    for (auto effect_type = 0; effect_type < SafeCast(effect_types.size()); effect_type++)
                    {
                        auto effect = Item::TargetEffectMapping[item.TargetEffects[target_type]];

                        if (effect_types[effect_type] == effect.c_str())
                        {
                            selected = effect_type;

                            break;
                        }
                    }

                    auto target_effect = Interface::TextList(graphics, scenes, effect_types, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, Asset::Map("CONFIRM"), Controls::MapType("CONFIRM"), selected);

                    if (target_effect >= 0 && target_effect < SafeCast(effect_types.size()))
                    {
                        item.SetTargetEffect(target_type, Item::MapTargetEffect(effect_types[target_effect]));
                    }
                }
            }
            else if (action.Action == Controls::MapType("REMOVE"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(targets.size()))
                {
                    auto target_type = Target::Map(targets[action.Selected]);

                    item.RemoveTargetEffect(target_type);
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // update damage
    void UpdateDamage(Graphics::Base &graphics, Graphics::Scenery &scenes, Asset::List &damage_assets, Controls::List &damage_controls, std::vector<std::string> &damage_captions, Item::Damage &damage_type)
    {
        while (true)
        {
            damage_assets[2] = damage_type.IgnoreArmour ? Asset::Map("PLAIN CIRCLE") : Asset::Map("CIRCLE");

            auto selected = Interface::IconList(graphics, scenes, damage_assets, damage_captions);

            if (selected >= 0 && selected < SafeCast(damage_controls.size()))
            {
                if (damage_controls[selected] == Controls::MapType("DAMAGE"))
                {
                    damage_type.Value = Interface::SetValue(graphics, scenes, Interface::Numbers, "BLOOD", damage_type.Value, 0, 12);
                }
                else if (damage_controls[selected] == Controls::MapType("MODIFIER"))
                {
                    damage_type.Modifier = Interface::SetValue(graphics, scenes, Interface::Numbers, "PLUS", damage_type.Modifier, -5, 5);
                }
                else if (damage_controls[selected] == Controls::MapType("ARMOUR"))
                {
                    damage_type.IgnoreArmour = !damage_type.IgnoreArmour;
                }
            }
            else
            {
                break;
            }
        }
    }

    // edit target specific damage modifiers
    void EditDamage(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item, bool modifier)
    {
        auto done = false;

        Controls::List actions = {
            Controls::MapType("EDIT"),
            Controls::MapType("ADD"),
            Controls::MapType("REMOVE")};

        Asset::List assets = {
            Asset::Map("GEARS"),
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL")};

        std::vector<std::string> target_types = {};

        for (auto &target_type : Target::Mapping)
        {
            target_types.push_back(target_type.second);
        }

        Asset::List damage_assets = {
            MapMaker::AttributeAssets[Attribute::Type::DAMAGE],
            Asset::Map("PLUS"),
            Asset::Map("CIRCLE"),
        };

        Controls::List damage_controls = {
            Controls::MapType("DAMAGE"),
            Controls::MapType("MODIFIER"),
            Controls::MapType("ARMOUR"),
        };

        std::vector<std::string> damage_captions = {
            "DAMAGE VALUE",
            "DAMAGE MODIFIER",
            "TOGGLE IGNORE ARMOUR"};

        while (!done)
        {
            std::vector<std::string> targets = {};

            if (!modifier)
            {
                for (auto target : item.DamageTypes)
                {
                    targets.push_back(Target::Mapping[target.first]);
                }
            }
            else
            {
                for (auto target : item.DamageModifiers)
                {
                    targets.push_back(Target::Mapping[target.first]);
                }
            }

            auto action = Interface::TextAction(graphics, scenes, targets, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, actions, assets);

            if (action.Action == Controls::MapType("ADD"))
            {
                auto selected = Interface::TextList(graphics, scenes, target_types, BloodSwordRogue::TileSize * 6, BloodSwordRogue::TileSize * 4, Asset::Map("TARGETING"), Controls::MapType("CONFIRM"));

                if (selected >= 0 && selected < SafeCast(target_types.size()))
                {
                    auto target = Target::Map(target_types[selected]);

                    auto exist = !modifier ? item.HasDamageType(target) : item.HasDamageModifier(target);

                    if (!exist)
                    {
                        auto damage = Item::Damage(0, 0, false);

                        MapMaker::UpdateDamage(graphics, scenes, damage_assets, damage_controls, damage_captions, damage);

                        if (!modifier)
                        {
                            item.AddDamageType(target, damage);
                        }
                        else
                        {
                            item.AddDamageModifier(target, damage);
                        }
                    }
                    else
                    {
                        auto damage = !modifier ? item.DamageTypes[target] : item.DamageModifiers[target];

                        MapMaker::UpdateDamage(graphics, scenes, damage_assets, damage_controls, damage_captions, damage);

                        if (!modifier)
                        {
                            item.SetDamageType(target, damage);
                        }
                        else
                        {
                            item.SetDamageModifier(target, damage);
                        }
                    }
                }
            }
            else if (action.Action == Controls::MapType("EDIT"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(targets.size()))
                {
                    auto target = Target::Map(targets[action.Selected]);

                    auto damage = !modifier ? item.DamageTypes[target] : item.DamageModifiers[target];

                    MapMaker::UpdateDamage(graphics, scenes, damage_assets, damage_controls, damage_captions, damage);

                    if (!modifier)
                    {
                        item.SetDamageType(target, damage);
                    }
                    else
                    {
                        item.SetDamageModifier(target, damage);
                    }
                }
            }
            else if (action.Action == Controls::MapType("REMOVE"))
            {
                if (action.Selected >= 0 && action.Selected < SafeCast(targets.size()))
                {
                    auto target = Target::Map(targets[action.Selected]);

                    if (!modifier)
                    {
                        if (item.HasDamageType(target))
                        {
                            item.RemoveDamageType(target);
                        }
                    }
                    else
                    {
                        if (item.HasDamageModifier(target))
                        {
                            item.RemoveDamageModifier(target);
                        }
                    }
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // edit item
    void EditItem(Graphics::Base &graphics, Graphics::Scenery &scenes, Item::Base &item)
    {
        auto done = false;

        Asset::List object_assets = {
            Asset::Map("MAGNIFYING GLASS"),
            Asset::Map("ONE"),
            Asset::Map("VERTICAL FLIP"),
            Asset::Map("WEIGHT"),
            Asset::Map("GEARS"),
            Asset::Map("IDENTIFY"),
            Asset::Map("BLOOD"),
            Asset::Map("BLOODY SWORD"),
            Asset::Map("TARGETING")};

        Asset::List object_controls = {
            Controls::MapType("VIEW"),
            Controls::MapType("QUANTITY"),
            Controls::MapType("LIMIT"),
            Controls::MapType("ENCUMBRANCE"),
            Controls::MapType("ATTRIBUTES"),
            Controls::MapType("PROPERTIES"),
            Controls::MapType("DAMAGE TYPE"),
            Controls::MapType("DAMAGE MODIFIER"),
            Controls::MapType("TARGET")};

        std::vector<std::string> object_captions = {
            "VIEW",
            "QUANTITY",
            "LIMIT",
            "ENCUMBRANCE",
            "ATTRIBUTES",
            "PROPERTIES",
            "DAMAGE TYPES",
            "DAMAGE MODIFIERS",
            "TARGET EFFECTS"};

        while (!done)
        {
            auto selected = Interface::IconList(graphics, scenes, object_assets, object_captions);

            if (selected >= 0 && selected < SafeCast(object_controls.size()))
            {
                if (object_controls[selected] == Controls::MapType("VIEW"))
                {
                    MapMaker::ViewItem(graphics, scenes, item);
                }
                else if (object_controls[selected] == Controls::MapType("QUANTITY"))
                {
                    for (auto i = 0; i < SafeCast(MapMaker::ItemsWithQuantities.size()); i++)
                    {
                        auto item_quantity = MapMaker::ItemsWithQuantities[i];

                        if (Item::MapType(item_quantity) == item.Type)
                        {
                            auto quantity = MapMaker::SetValue(graphics, scenes, MapMaker::ItemsAssets[i], item.Quantity, 0, item.Limit != Item::Unlimited ? item.Limit : 99);

                            if (quantity >= 0 && quantity < 100)
                            {
                                item.Quantity = quantity;
                            }

                            break;
                        }
                    }
                }
                else if (object_controls[selected] == Controls::MapType("LIMIT"))
                {
                    for (auto i = 0; i < SafeCast(MapMaker::ItemsWithQuantities.size()); i++)
                    {
                        auto item_limit = MapMaker::ItemsWithQuantities[i];

                        if (Item::MapType(item_limit) == item.Type)
                        {
                            auto limit = MapMaker::SetValue(graphics, scenes, MapMaker::ItemsAssets[i], item.Limit, -1, 99);

                            if (limit >= -1 && limit < 100)
                            {
                                item.Limit = limit;
                            }

                            break;
                        }
                    }
                }
                else if (object_controls[selected] == Controls::MapType("ENCUMBRANCE"))
                {
                    auto encumbrance = MapMaker::SetValue(graphics, scenes, "WEIGHT", item.Encumbrance, 0, 99);

                    if (encumbrance >= 0 && encumbrance < 100)
                    {
                        item.Encumbrance = encumbrance;
                    }
                }
                else if (object_controls[selected] == Controls::MapType("ATTRIBUTES"))
                {
                    MapMaker::EditAttributes(graphics, scenes, item);
                }
                else if (object_controls[selected] == Controls::MapType("PROPERTIES"))
                {
                    MapMaker::EditProperties(graphics, scenes, item);
                }
                else if (object_controls[selected] == Controls::MapType("TARGET"))
                {
                    MapMaker::EditTargetEffects(graphics, scenes, item);
                }
                else if (object_controls[selected] == Controls::MapType("DAMAGE TYPE"))
                {
                    MapMaker::EditDamage(graphics, scenes, item, false);
                }
                else if (object_controls[selected] == Controls::MapType("DAMAGE MODIFIER"))
                {
                    MapMaker::EditDamage(graphics, scenes, item, true);
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // select item
    int SelectItem(Graphics::Base &graphics, Graphics::Scenery scenes, Items::Inventory &items)
    {
        auto selected = Item::NONE;

        Asset::List assets = {};

        std::vector<std::string> captions = {};

        for (auto &item : items)
        {
            assets.push_back(item.Asset);

            std::string caption = item.Name;

            if (item.Contains != Item::NONE && item.Quantity > 0)
            {
                caption += std::string(": ") + std::to_string(item.Quantity);
            }

            captions.push_back(caption);
        }

        if (SafeCast(assets.size()) > 0)
        {
            selected = Interface::IconGrid(graphics, scenes, assets, BloodSwordRogue::TileSize * 12, BloodSwordRogue::TileSize * 5, captions);
        }

        return selected;
    }

    // select item
    int SelectItem(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        return MapMaker::SelectItem(graphics, scenes, character.Items);
    }

    // view items
    void ViewItems(Graphics::Base &graphics, Graphics::Scenery scenes, Items::Inventory &items)
    {
        while (true)
        {
            auto selected = MapMaker::SelectItem(graphics, scenes, items);

            if (selected >= 0 && selected < SafeCast(items.size()))
            {
                // edit item details
                MapMaker::EditItem(graphics, scenes, items[selected]);
            }
            else
            {
                break;
            }
        }
    }

    // view items on this character
    void ViewItems(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        MapMaker::ViewItems(graphics, scenes, character.Items);
    }

    // remove item from character
    void RemoveItem(Graphics::Base &graphics, Graphics::Scenery scenes, Character::Base &character)
    {
        while (true && SafeCast(character.Items.size() > 0))
        {
            auto selected = MapMaker::SelectItem(graphics, scenes, character);

            if (selected >= 0 && selected < SafeCast(character.Items.size()))
            {
                character.Items.erase(character.Items.begin() + selected);
            }
            else
            {
                break;
            }
        }
    }

    // increase modifier
    void IncModifier(Character::Base &character, Attribute::Type attribute, int maximum)
    {
        auto value = character.Modifier(attribute);

        if (value < maximum)
        {
            character.Modifier(attribute, value + 1);
        }
    }

    // decrease modifier
    void DecModifier(Character::Base &character, Attribute::Type attribute, int minimum)
    {
        auto value = character.Modifier(attribute);

        if (value > minimum)
        {
            character.Modifier(attribute, value - 1);
        }
    }

    // increments attribute
    void IncAttribute(Character::Base &character, Attribute::Type attribute, int maximum)
    {
        auto value = character.Value(attribute);

        if (value < maximum)
        {
            if ((value + 1) > character.Maximum(attribute))
            {
                character.Maximum(attribute, value + 1);
            }

            character.Value(attribute, value + 1);
        }
    }

    // decrease attribute
    void DecAttribute(Character::Base &character, Attribute::Type attribute, int minimum)
    {
        auto value = character.Value(attribute);

        if (value > minimum)
        {
            character.Value(attribute, value - 1);
        }

        if (attribute == Attribute::Type::ENDURANCE)
        {
            character.Maximum(attribute, character.Value(attribute));
        }
    }

    // edit character attributes
    void EditCharacter(Graphics::Base &graphics, Scene::Base &background, Character::Base &character)
    {
        auto done = false;

        Asset::List object_assets = {
            Asset::Map("MAGNIFYING GLASS"),
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL")};

        Asset::List object_controls = {
            Controls::MapType("VIEW"),
            Controls::MapType("CONFIRM"),
            Controls::MapType("CANCEL")};

        std::vector<std::string> item_captions = {
            "VIEW ITEMS",
            "ADD ITEM",
            "REMOVE ITEM"};

        std::vector<std::string> skill_captions = {
            "VIEW SKILLS",
            "ADD SKILL",
            "REMOVE SKILL"};

        auto tile = BloodSwordRogue::TileSize;

        auto width = tile * 16;

        auto height = tile * 10;

        auto box = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

        auto input = Controls::User();

        while (!done)
        {
            auto scene = Scene::Base();

            // icon grid
            scene.Add(Scene::Element(box.X - BloodSwordRogue::Border, box.Y - BloodSwordRogue::Border, width + BloodSwordRogue::Border * 2, height + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            // add character asset
            if (character.Asset != Asset::NONE)
            {
                scene.VerifyAndAdd(Scene::Element(Asset::Get(character.Asset), Point(box.X + tile, box.Y + tile)));
            }
            else
            {
                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("CHARACTER")), Point(box.X + tile, box.Y + tile)));
            }

            scene.Add(Controls::Base(Controls::MapType("NAME"), 0, 0, 1, 0, 0, box.X + tile, box.Y + tile, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            // edit skills
            scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("CHECKBOX TREE")), Point(box.X + tile * 2, box.Y + tile)));

            scene.Add(Controls::Base(Controls::MapType("SKILLS"), 1, 0, 2, 1, 1, box.X + tile * 2, box.Y + tile, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            // edit items
            scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("ITEMS")), Point(box.X + tile * 3, box.Y + tile)));

            scene.Add(Controls::Base(Controls::MapType("ITEMS"), 2, 1, 2, 2, 2, box.X + tile * 3, box.Y + tile, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            // fighting prowess
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::FIGHTING_PROWESS], character.Value(Attribute::Type::FIGHTING_PROWESS), box.X + tile, box.Y + tile * 3, "FPR+", "FPR-");

            // fighting prowess modifiers
            Interface::RenderValue(scene, MapMaker::Numbers, Asset::Map("PLUS"), character.Modifier(Attribute::Type::FIGHTING_PROWESS), box.X + tile * 6, box.Y + tile * 3, "FPR MOD+", "FPR MOD-");

            // awareness
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::AWARENESS], character.Value(Attribute::Type::AWARENESS), box.X + tile, box.Y + tile * 4, "AWR+", "AWR-");

            // awareness modifiers
            Interface::RenderValue(scene, MapMaker::Numbers, Asset::Map("PLUS"), character.Modifier(Attribute::Type::AWARENESS), box.X + tile * 6, box.Y + tile * 4, "AWR MOD+", "AWR MOD-");

            // psychic ability
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::PSYCHIC_ABILITY], character.Value(Attribute::Type::PSYCHIC_ABILITY), box.X + tile, box.Y + tile * 5, "PSY+", "PSY-");

            // psychic ability modifiers
            Interface::RenderValue(scene, MapMaker::Numbers, Asset::Map("PLUS"), character.Modifier(Attribute::Type::PSYCHIC_ABILITY), box.X + tile * 6, box.Y + tile * 5, "PSY MOD+", "PSY MOD-");

            // damage
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::DAMAGE], character.Value(Attribute::Type::DAMAGE), box.X + tile, box.Y + tile * 6, "DMG+", "DMG-");

            // damage modifiers
            Interface::RenderValue(scene, MapMaker::Numbers, Asset::Map("PLUS"), character.Modifier(Attribute::Type::DAMAGE), box.X + tile * 6, box.Y + tile * 6, "DMG MOD+", "DMG MOD-");

            // endurance
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::ENDURANCE], character.Value(Attribute::Type::ENDURANCE), box.X + tile, box.Y + tile * 7, "END+", "END-");

            // encumbrance limit
            Interface::RenderValue(scene, MapMaker::Numbers, Asset::Map("WEIGHT"), character.EncumbranceLimit, box.X + tile * 6, box.Y + tile * 7, "WT+", "WT-");

            // armour
            Interface::RenderValue(scene, MapMaker::Numbers, MapMaker::AttributeAssets[Attribute::Type::ARMOUR], character.Modifier(Attribute::Type::ARMOUR), box.X + tile, box.Y + tile * 8, "ARM+", "ARM-");

            auto back_id = scene.Controls.size();

            auto back = Point(box.X + width - BloodSwordRogue::TileSize + BloodSwordRogue::Border, box.Y + height - BloodSwordRogue::TileSize + BloodSwordRogue::Border);

            scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("BACK")), back));

            scene.Add(Controls::Base(Controls::MapType("BACK"), back_id, back_id, back_id, back_id, back_id, back.X, back.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));

            input = Input::WaitForInput(graphics, background, scene, input, true, true);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("BACK"))
                {
                    done = true;
                }
                else if (input.Type == Controls::MapType("FPR+"))
                {
                    MapMaker::IncAttribute(character, Attribute::Type::FIGHTING_PROWESS, Attribute::MaxValue);
                }
                else if (input.Type == Controls::MapType("FPR-"))
                {
                    MapMaker::DecAttribute(character, Attribute::Type::FIGHTING_PROWESS, Attribute::MinValue);
                }
                else if (input.Type == Controls::MapType("AWR+"))
                {
                    MapMaker::IncAttribute(character, Attribute::Type::AWARENESS, Attribute::MaxValue);
                }
                else if (input.Type == Controls::MapType("AWR-"))
                {
                    MapMaker::DecAttribute(character, Attribute::Type::AWARENESS, Attribute::MinValue);
                }
                else if (input.Type == Controls::MapType("PSY+"))
                {
                    MapMaker::IncAttribute(character, Attribute::Type::PSYCHIC_ABILITY, Attribute::MaxValue);
                }
                else if (input.Type == Controls::MapType("PSY-"))
                {
                    MapMaker::DecAttribute(character, Attribute::Type::PSYCHIC_ABILITY, Attribute::MinValue);
                }
                else if (input.Type == Controls::MapType("END+"))
                {
                    MapMaker::IncAttribute(character, Attribute::Type::ENDURANCE, 99);
                }
                else if (input.Type == Controls::MapType("END-"))
                {
                    MapMaker::DecAttribute(character, Attribute::Type::ENDURANCE, Attribute::MinValue);
                }
                else if (input.Type == Controls::MapType("DMG+"))
                {
                    MapMaker::IncAttribute(character, Attribute::Type::DAMAGE, Attribute::MaxValue);
                }
                else if (input.Type == Controls::MapType("DMG-"))
                {
                    MapMaker::DecAttribute(character, Attribute::Type::DAMAGE, Attribute::MinValue);
                }
                else if (input.Type == Controls::MapType("ARM+"))
                {
                    MapMaker::IncModifier(character, Attribute::Type::ARMOUR, Attribute::MaxModifier);
                }
                else if (input.Type == Controls::MapType("ARM-"))
                {
                    MapMaker::DecModifier(character, Attribute::Type::ARMOUR, 0);
                }
                else if (input.Type == Controls::MapType("FPR MOD+"))
                {
                    MapMaker::IncModifier(character, Attribute::Type::FIGHTING_PROWESS, Attribute::MaxModifier);
                }
                else if (input.Type == Controls::MapType("FPR MOD-"))
                {
                    MapMaker::DecModifier(character, Attribute::Type::FIGHTING_PROWESS, Attribute::MinModifier);
                }
                else if (input.Type == Controls::MapType("AWR MOD+"))
                {
                    MapMaker::IncModifier(character, Attribute::Type::AWARENESS, Attribute::MaxModifier);
                }
                else if (input.Type == Controls::MapType("AWR MOD-"))
                {
                    MapMaker::DecModifier(character, Attribute::Type::AWARENESS, Attribute::MinModifier);
                }
                else if (input.Type == Controls::MapType("PSY MOD+"))
                {
                    MapMaker::IncModifier(character, Attribute::Type::PSYCHIC_ABILITY, Attribute::MaxModifier);
                }
                else if (input.Type == Controls::MapType("PSY MOD-"))
                {
                    MapMaker::DecModifier(character, Attribute::Type::PSYCHIC_ABILITY, Attribute::MinModifier);
                }
                else if (input.Type == Controls::MapType("DMG MOD+"))
                {
                    MapMaker::IncModifier(character, Attribute::Type::DAMAGE, Attribute::MaxModifier);
                }
                else if (input.Type == Controls::MapType("DMG MOD-"))
                {
                    MapMaker::DecModifier(character, Attribute::Type::DAMAGE, Attribute::MinModifier);
                }
                else if (input.Type == Controls::MapType("WT+"))
                {
                    if (character.EncumbranceLimit < 99)
                    {
                        character.EncumbranceLimit++;
                    }
                }
                else if (input.Type == Controls::MapType("WT-"))
                {
                    if (character.EncumbranceLimit > character.TotalEncumbrance() && character.EncumbranceLimit > 1)
                    {
                        character.EncumbranceLimit--;
                    }
                }
                else if (input.Type == Controls::MapType("NAME"))
                {
                    auto question = std::string("EDIT NAME");

                    auto name = BloodSwordRogue::Trim(Interface::TextInput(graphics, {background, scene}, question, character.Name, 20, true));

                    if (SafeCast(name.size()) > 0)
                    {
                        character.Name = std::string(name);
                    }
                }
                else if (input.Type == Controls::MapType("SKILLS"))
                {
                    auto selected = Interface::IconList(graphics, {background, scene}, object_assets, skill_captions);

                    if (selected >= 0 && selected < SafeCast(object_controls.size()))
                    {
                        if (object_controls[selected] == Controls::MapType("VIEW") && SafeCast(character.Skills.size() > 0))
                        {
                            MapMaker::SelectSkill(graphics, {background, scene}, character);
                        }
                        else if (object_controls[selected] == Controls::MapType("CONFIRM"))
                        {
                            MapMaker::AddSkill(graphics, {background, scene}, character);
                        }
                        else if (object_controls[selected] == Controls::MapType("CANCEL"))
                        {
                            MapMaker::RemoveSkill(graphics, {background, scene}, character);
                        }
                    }
                }
                else if (input.Type == Controls::MapType("ITEMS"))
                {
                    auto selected = Interface::IconList(graphics, {background, scene}, object_assets, item_captions);

                    if (selected >= 0 && selected < SafeCast(object_controls.size()))
                    {
                        if (object_controls[selected] == Controls::MapType("VIEW") && SafeCast(character.Items.size() > 0))
                        {
                            MapMaker::ViewItems(graphics, {background, scene}, character);
                        }
                        else if (object_controls[selected] == Controls::MapType("CONFIRM"))
                        {
                            MapMaker::AddItem(graphics, {background, scene}, character);
                        }
                        else if (object_controls[selected] == Controls::MapType("CANCEL"))
                        {
                            MapMaker::RemoveItem(graphics, {background, scene}, character);
                        }
                    }
                }

                input.Selected = false;
            }
        }
    }

    // edit party at map location
    void EditEnemy(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.Id > 0 && tile.Id <= SafeCast(location.Opponents.size()))
        {
            Asset::List assets = {};

            std::vector<std::string> captions = {};

            auto id = tile.Id - 1;

            auto &party = location.Opponents[id];

            auto selected = 0;

            if (party.Count() > 1)
            {
                for (auto i = 0; i < party.Count(); i++)
                {
                    assets.push_back(party[i].Asset);

                    captions.push_back(std::string("EDIT ") + party[i].Name);
                }

                selected = Interface::IconList(graphics, scene, assets, captions);
            }

            // TODO: edit enemy in party
            if (selected >= 0 && selected < party.Count())
            {
                SDL_Log("[UPDATE PARTY %d] [EDITING %s]", id + 1, party[selected].Name.c_str());

                MapMaker::EditCharacter(graphics, scene, party[selected]);
            }
        }
    }

    // add enemy to existing party or create new one at map location
    void AddEnemy(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.Type == Map::Object::PASSABLE || tile.Type == Map::Object::ENEMY_PASSABLE)
        {
            while (true)
            {
                auto selected = Interface::IconGrid(graphics, scene, MapMaker::RosterAssets, (map.ViewX + 1) * map.TileSize, (map.ViewY + 1) * map.TileSize + BloodSwordRogue::HalfTile, MapMaker::RosterCaptions);

                if (selected >= 0 && selected < SafeCast(MapMaker::RosterTargets.size()))
                {
                    auto enemy = MapMaker::Roster[MapMaker::RosterTargets[selected]];

                    Interface::AddCharacter(graphics, scene, location.Opponents, enemy, map, Map::Object::ENEMIES, point, MapMaker::MaxEnemies);
                }
                else
                {
                    break;
                }
            }
        }
    }

    // remove enemy from party (in selected map location)
    void RemoveEnemy(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.Id > 0 && tile.Id <= SafeCast(location.Opponents.size()))
        {
            auto id = tile.Id - 1;

            auto &party = location.Opponents[id];

            auto selected = 0;

            auto threshold = 1;

            while (true || party.Count() > 0)
            {
                if (party.Count() > threshold)
                {
                    Asset::List assets = {};

                    std::vector<std::string> captions = {};

                    for (auto i = 0; i < party.Count(); i++)
                    {
                        assets.push_back(party[i].Asset);

                        captions.push_back(std::string("REMOVE ") + party[i].Name);
                    }

                    selected = Interface::IconList(graphics, scene, assets, captions);
                }

                if (selected >= 0 && selected < party.Count())
                {
                    Interface::RemoveCharacter(graphics, scene, map, location.Opponents, point, selected);

                    selected = -1;

                    threshold = 0;
                }
                else
                {
                    break;
                }
            }
        }
    }

    // add item to existing loot or create new loot at map location
    void AddLoot(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.IsOccupied() && tile.Id != Map::NotFound)
        {
            auto id = tile.Id - 1;

            if (location.Loot[id].Items.size() >= MapMaker::MaxItems)
            {
                Interface::MessageBox(graphics, scene, "CANNOT ADD MORE TO THE STASH", Color::Highlight);

                return;
            }
        }

        if (tile.Type == Map::Object::PASSABLE || tile.Type == Map::Object::ENEMY_PASSABLE)
        {
            while (true)
            {
                auto selected = Interface::IconGrid(graphics, scene, MapMaker::ItemAssets, (map.ViewX + 1) * map.TileSize, (map.ViewY + 1) * map.TileSize + BloodSwordRogue::HalfTile, MapMaker::ItemCaptions);

                if (selected >= 0 && selected < SafeCast(MapMaker::ItemTypes.size()))
                {
                    if (tile.IsOccupied() && tile.Id != Map::NotFound)
                    {
                        // add item to existing loot
                        auto id = tile.Id - 1;

                        auto item = Items::Defaults[MapMaker::ItemTypes[selected]];

                        Graphics::Scenery scenes = {scene};

                        MapMaker::AddItem(graphics, scenes, location.Loot[id].Items, item);

                        SDL_Log("[ADDED TO LOOT %d] [ADD %s]", id + 1, item.Name.c_str());
                    }
                    else
                    {
                        auto loot = Location::Loot();

                        auto item = Items::Defaults[MapMaker::ItemTypes[selected]];

                        Graphics::Scenery scenes = {scene};

                        MapMaker::AddItem(graphics, scenes, loot.Items, item);

                        loot.X = point.X;

                        loot.Y = point.Y;

                        auto id = SafeCast(location.Loot.size()) + 1;

                        location.Loot.push_back(loot);

                        tile.Id = id;

                        tile.Occupant = Map::Object::ITEMS;

                        SDL_Log("[CREATE LOOT %d] [ADD %s]", id, item.Name.c_str());
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    // remove item from loot (in map selected map location)
    void RemoveLoot(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.Id > 0 && tile.Id <= SafeCast(location.Loot.size()))
        {
            Asset::List assets = {};

            std::vector<std::string> captions = {};

            auto id = tile.Id - 1;

            auto &items = location.Loot[id].Items;

            auto selected = 0;

            if (SafeCast(items.size()) > 1)
            {
                for (auto i = 0; i < SafeCast(items.size()); i++)
                {
                    assets.push_back(items[i].Asset);

                    captions.push_back(std::string("REMOVE ") + items[i].Name);
                }

                selected = Interface::IconList(graphics, scene, assets, captions);
            }

            if (selected >= 0 && selected < SafeCast(items.size()))
            {
                SDL_Log("[UPDATE LOOT %d] [REMOVED %s]", id + 1, items[selected].Name.c_str());

                items.erase(items.begin() + selected);
            }

            if (SafeCast(items.size()) <= 0)
            {
                SDL_Log("[REMOVED LOOT %d]", id + 1);

                location.Loot.erase(location.Loot.begin() + id);

                // remove loot from current location
                tile.Id = Map::NotFound;

                tile.Occupant = Map::Object::NONE;

                Location::RenumberLoot(location);
            }
        }
    }

    // Limited to viewing loot at map location (no adding/removing quantities/attributes/properties)
    void EditLoot(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (tile.Id > 0 && tile.Id <= SafeCast(location.Loot.size()))
        {
            auto id = tile.Id - 1;

            auto &loot = location.Loot[id];

            Graphics::Scenery scenes = {scene};

            if (SafeCast(loot.Items.size()) > 1)
            {
                MapMaker::ViewItems(graphics, scenes, loot.Items);
            }
            else
            {
                MapMaker::EditItem(graphics, scenes, loot.Items[0]);
            }
        }
    }

    // check if next point is a valid target for flood fill
    bool IsValidTarget(Map::Base &map, Asset::Type valid_asset, Map::Object valid_type, Point next)
    {
        return (map.IsValid(next) && map[next].Asset == valid_asset && map[next].Type == valid_type);
    }

    // check run of valid targets in specified direction
    void CheckRun(Map::Base &map, Asset::Type valid_asset, Map::Object valid_type, Point point, int dir, int &start, int &end)
    {
        if (std::abs(dir) != 1)
        {
            return;
        }

        start = point.X;

        end = point.X;

        while (point.X >= 0 && point.X < map.Width)
        {
            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, point))
            {
                end = point.X;

                point.X += dir;
            }
            else
            {
                break;
            }
        }
    }

    // fill line in specified direction
    int FillDirection(Map::Base &map, Points &points, Asset::Type valid_asset, Map::Object valid_type, Asset::Type asset, Map::Object type, Point next, int dir)
    {
        auto fills = 0;

        if (std::abs(dir) != 1)
        {
            return fills;
        }

        auto top_start = next.X;

        auto top_end = next.X;

        auto bot_start = next.X;

        auto bot_end = next.X;

        while (true)
        {
            map[next].Asset = asset;

            map[next].Type = type;

            fills++;

            auto top = Point(next.X, next.Y - 1);

            // check if upper row is a valid target (and not yet in queue)
            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, top))
            {
                // check top run
                if ((dir < 0 && next.X <= top_end) || (dir > 0 && next.X >= top_end))
                {
                    if (!BloodSwordRogue::In(points, top))
                    {
                        points.push_back(top);

                        MapMaker::CheckRun(map, valid_asset, valid_type, top, dir, top_start, top_end);
                    }
                }
            }

            // check if lower is a valid target (and not yet in queue)
            auto bot = Point(next.X, next.Y + 1);

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, bot))
            {
                // check bottom run
                if ((dir < 0 && next.X <= bot_end) || (dir > 0 && next.X >= bot_end))
                {
                    if (!BloodSwordRogue::In(points, bot))
                    {
                        points.push_back(bot);

                        MapMaker::CheckRun(map, valid_asset, valid_type, bot, dir, bot_start, bot_end);
                    }
                }
            }

            // fill in current direction as long as next point is a valid target
            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, Point(next.X + dir, next.Y)))
            {
                next = Point(next.X + dir, next.Y);
            }
            else
            {
                // stop fill
                break;
            }
        }

        return fills;
    }

    // fill entire line
    int FillLine(Map::Base &map, Points &points, Asset::Type valid_asset, Map::Object valid_type, Asset::Type asset, Map::Object type, Point point)
    {
        auto fills = 0;

        // check if "line" has been filled before
        if (!MapMaker::IsValidTarget(map, valid_asset, valid_type, point))
        {
            return fills;
        }

        auto next = point;

        // fill left
        fills += MapMaker::FillDirection(map, points, valid_asset, valid_type, asset, type, next, -1);

        // check if we can fill to right
        if (!MapMaker::IsValidTarget(map, valid_asset, valid_type, Point(point.X + 1, point.Y)))
        {
            return fills;
        }

        // set fill-right starting point
        next = Point(point.X + 1, point.Y);

        // fill right
        fills += MapMaker::FillDirection(map, points, valid_asset, valid_type, asset, type, next, +1);

        return fills;
    }

    // fill areas on the map with the specified asset
    void FloodFill(Map::Base &map, Asset::Type asset, Map::Object type, Point point)
    {
        Points points = {};

        if (!map.IsValid(point))
        {
            return;
        }

        auto valid_asset = map[point].Asset;

        auto valid_type = map[point].Type;

        auto passes = 0;

        auto fills = 0;

        points.push_back(point);

        while (SafeCast(points.size()) > 0)
        {
            // pop one point from the stack
            auto next = points.back();

            points.pop_back();

            // fill line
            fills += MapMaker::FillLine(map, points, valid_asset, valid_type, asset, type, next);

            passes++;
        }

        SDL_Log("[FLOOD FILL] [%d PASSES] [%d POINTS]", passes, fills);
    }

    void FloodFillV2(Map::Base &map, Asset::Type asset, Map::Object type, Point point)
    {
        if (!map.IsValid(point))
        {
            return;
        }

        std::vector<std::vector<bool>> visited(map.Height, std::vector<bool>(map.Width, false));

        Points points = {};

        auto valid_asset = map[point].Asset;

        auto valid_type = map[point].Type;

        auto passes = 0;

        auto fills = 0;

        points.push_back(point);

        while (SafeCast(points.size()) > 0)
        {
            // pop one point from the stack
            auto next = points.back();

            points.pop_back();

            visited[next.Y][next.X] = true;

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, next))
            {
                map[next].Asset = asset;

                map[next].Type = type;

                fills++;
            }

            auto left = Point(next.X - 1, next.Y);

            auto right = Point(next.X + 1, next.Y);

            auto up = Point(next.X, next.Y - 1);

            auto down = Point(next.X, next.Y + 1);

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, left) && !visited[left.Y][left.X])
            {
                map[left].Asset = asset;

                map[left].Type = type;

                fills++;

                points.push_back(left);
            }

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, right) && !visited[right.Y][right.X])
            {
                map[right].Asset = asset;

                map[right].Type = type;

                fills++;

                points.push_back(right);
            }

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, up) && !visited[up.Y][up.X])
            {
                map[up].Asset = asset;

                map[up].Type = type;

                fills++;

                points.push_back(up);
            }

            if (MapMaker::IsValidTarget(map, valid_asset, valid_type, down) && !visited[down.Y][down.X])
            {
                map[down].Asset = asset;

                map[down].Type = type;

                fills++;

                points.push_back(down);
            }

            passes++;
        }

        SDL_Log("[FLOOD FILL] [%d PASSES] [%d POINTS]", passes, fills);
    }

    void ResetMapView(Graphics::Base &graphics, Map::Base &map, int tiles_w, int tiles_h)
    {
        // set edit window dimensions with spaces for map controls
        map.ViewX = tiles_w;

        map.ViewY = tiles_h;

        // set map offsets within the edit window
        map.X = (map.Width - map.ViewX) / 2;

        map.Y = (map.Height - map.ViewY) / 2;

        // set edit window positions on screen
        map.DrawX = (graphics.Width - (map.ViewX * map.TileSize)) / 2;

        map.DrawY = (graphics.Height - (map.ViewY * map.TileSize)) / 2;
    }

    void RefreshMapView(Graphics::Base &graphics, Map::Base &map, int &TilesW, int &TilesH)
    {
        TilesW = std::min(map.ViewX, graphics.Width / map.TileSize - 4);

        TilesH = std::min(map.ViewY, graphics.Height / map.TileSize - 7);

        MapMaker::ResetMapView(graphics, map, TilesW, TilesH);
    }

    void MapSettings(Graphics::Base &graphics, Graphics::Scenery scenes, Location::Base &location, Function &function)
    {
        std::vector<Asset::Type> assets = {};

        std::vector<Controls::Type> controls = {};

        std::vector<std::string> captions = {};

        int width = (graphics.Width - 4 * BloodSwordRogue::TileSize);

        int height = (graphics.Height - 7 * BloodSwordRogue::TileSize);

        // setup icon grid
        assets.push_back(Asset::Map("MAP"));

        controls.push_back(Controls::MapType("MAP"));

        captions.push_back(std::string("EDIT NAME"));

        assets.push_back(Asset::Map("SELECT"));

        controls.push_back(Controls::MapType("SELECT"));

        captions.push_back(std::string("ADD PARTY ORIGIN"));

        assets.push_back(Asset::Map("HORIZONTAL FLIP"));

        controls.push_back(Controls::MapType("RESIZE WIDTH"));

        captions.push_back(std::string("RESIZE WIDTH"));

        assets.push_back(Asset::Map("VERTICAL FLIP"));

        controls.push_back(Controls::MapType("RESIZE HEIGHT"));

        captions.push_back(std::string("RESIZE LENGTH"));

        assets.push_back(Asset::Map("MAZE"));

        controls.push_back(Controls::MapType("MAZE"));

        captions.push_back(std::string("GENERATE MAZE"));

        assets.push_back(Asset::Map("CAVE"));

        controls.push_back(Controls::MapType("CAVE"));

        captions.push_back(std::string("GENERATE BATTLEPITS"));

        auto done = false;

        while (!done)
        {
            auto grid = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

            auto selected = Interface::IconGrid(graphics, scenes, assets, width, height, captions);

            if (selected >= 0 && selected < SafeCast(controls.size()))
            {
                if (controls[selected] == Controls::MapType("MAP"))
                {
                    auto question = std::string("EDIT NAME");

                    auto name = BloodSwordRogue::Trim(Interface::TextInput(graphics, scenes, question, location.Name, 20, true));

                    if (SafeCast(name.size()) > 0)
                    {
                        location.Name = std::string(name);
                    }
                }
                else if (controls[selected] == Controls::MapType("SELECT"))
                {
                    function = Function::ORIGIN;

                    done = true;
                }
                else if (controls[selected] == Controls::MapType("RESIZE WIDTH"))
                {
                    auto new_width = MapMaker::SetValue(graphics, scenes, "HORIZONTAL FLIP", location.Map.Width);

                    auto diff = std::abs(new_width - location.Map.Width);

                    auto columns = diff / 2;

                    auto remainder = diff % 2;

                    if (new_width < location.Map.Width)
                    {
                        location.Map.RemoveColumnsLeft(columns);

                        location.Map.RemoveColumnsRight(columns + remainder);

                        // move locations
                        if (SafeCast(location.Map.Origins.size()) > 0)
                        {
                            location.Map.Origins[0].X -= columns;

                            if (!(location.Map.Origins[0].X >= 0 && location.Map.Origins[0].X < location.Map.Width))
                            {
                                location.Map.Origins.clear();
                            }
                        }

                        std::vector<Party::Base> parties = {};

                        for (auto opponent = 0; opponent < SafeCast(location.Opponents.size()); opponent++)
                        {
                            location.Opponents[opponent].X -= columns;

                            if (location.Opponents[opponent].X >= 0 && location.Opponents[opponent].X < location.Map.Width)
                            {
                                parties.push_back(location.Opponents[opponent]);
                            }
                        }

                        location.Opponents = parties;

                        Location::RenumberParties(location);

                        std::vector<Location::Loot> bags = {};

                        for (auto loot = 0; loot < SafeCast(location.Loot.size()); loot++)
                        {
                            location.Loot[loot].X -= columns;

                            if (location.Loot[loot].X >= 0 && location.Loot[loot].X < location.Map.Width)
                            {
                                bags.push_back(location.Loot[loot]);
                            }
                        }

                        location.Loot = bags;

                        Location::RenumberLoot(location);

                        std::vector<Trigger::Base> triggers = {};

                        for (auto trigger = 0; trigger < SafeCast(location.Triggers.size()); trigger++)
                        {
                            location.Triggers[trigger].X -= columns;

                            if (location.Triggers[trigger].X >= 0 && location.Triggers[trigger].X < location.Map.Width)
                            {
                                triggers.push_back(location.Triggers[trigger]);
                            }
                        }

                        location.Triggers = triggers;

                        // renumber triggers
                        Location::RenumberTriggers(location);

                        location.Map.ViewX = location.Map.Width;

                        function = Function::RESIZE;

                        done = true;
                    }
                    else if (new_width > location.Map.Width)
                    {
                        location.Map.AddColumnsLeft(columns);

                        location.Map.AddColumnsRight(columns + remainder);

                        // move locations
                        if (SafeCast(location.Map.Origins.size()) > 0)
                        {
                            location.Map.Origins[0].X += columns;
                        }

                        for (auto &opponent : location.Opponents)
                        {
                            opponent.X += columns;
                        }

                        for (auto &loot : location.Loot)
                        {
                            loot.X += columns;
                        }

                        for (auto &trigger : location.Triggers)
                        {
                            trigger.X += columns;
                        }

                        location.Map.ViewX = location.Map.Width;

                        function = Function::RESIZE;

                        done = true;
                    }
                }
                else if (controls[selected] == Controls::MapType("RESIZE HEIGHT"))
                {
                    auto new_height = MapMaker::SetValue(graphics, scenes, "VERTICAL FLIP", location.Map.Height);

                    auto diff = std::abs((new_height - location.Map.Height));

                    auto rows = diff / 2;

                    auto remainder = diff % 2;

                    if (new_height < location.Map.Height)
                    {
                        location.Map.RemoveRowsTop(rows);

                        location.Map.RemoveRowsBottom(rows + remainder);

                        // move locations
                        if (SafeCast(location.Map.Origins.size()) > 0)
                        {
                            location.Map.Origins[0].Y -= rows;

                            if (!(location.Map.Origins[0].Y >= 0 && location.Map.Origins[0].Y < location.Map.Height))
                            {
                                location.Map.Origins.clear();
                            }
                        }

                        std::vector<Party::Base> parties = {};

                        for (auto opponent = 0; opponent < SafeCast(location.Opponents.size()); opponent++)
                        {
                            location.Opponents[opponent].Y -= rows;

                            if (location.Opponents[opponent].Y >= 0 && location.Opponents[opponent].Y < location.Map.Height)
                            {
                                parties.push_back(location.Opponents[opponent]);
                            }
                        }

                        location.Opponents = parties;

                        Location::RenumberParties(location);

                        std::vector<Location::Loot> bags = {};

                        for (auto loot = 0; loot < SafeCast(location.Loot.size()); loot++)
                        {
                            location.Loot[loot].Y -= rows;

                            if (location.Loot[loot].Y >= 0 && location.Loot[loot].Y < location.Map.Height)
                            {
                                bags.push_back(location.Loot[loot]);
                            }
                        }

                        location.Loot = bags;

                        Location::RenumberLoot(location);

                        std::vector<Trigger::Base> triggers = {};

                        for (auto trigger = 0; trigger < SafeCast(location.Triggers.size()); trigger++)
                        {
                            location.Triggers[trigger].Y -= rows;

                            if (location.Triggers[trigger].Y >= 0 && location.Triggers[trigger].Y < location.Map.Height)
                            {
                                triggers.push_back(location.Triggers[trigger]);
                            }
                        }

                        location.Triggers = triggers;

                        // renumber triggers
                        Location::RenumberTriggers(location);

                        location.Map.ViewY = location.Map.Height;

                        function = Function::RESIZE;

                        done = true;
                    }
                    else if (new_height > location.Map.Height)
                    {
                        location.Map.AddRowsTop(rows);

                        location.Map.AddRowsBottom(rows + remainder);

                        // move locations
                        if (SafeCast(location.Map.Origins.size()) > 0)
                        {
                            location.Map.Origins[0].Y += rows;
                        }

                        for (auto &opponent : location.Opponents)
                        {
                            opponent.Y += rows;
                        }

                        for (auto &loot : location.Loot)
                        {
                            loot.Y += rows;
                        }

                        for (auto &trigger : location.Triggers)
                        {
                            trigger.Y += rows;
                        }

                        location.Map.ViewY = location.Map.Height;

                        function = Function::RESIZE;

                        done = true;
                    }
                }
                else if (controls[selected] == Controls::MapType("CAVE"))
                {
                    if (Interface::Confirm(graphics, scenes, "THIS CLEARS THE MAP - ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                    {
                        location.Triggers.clear();

                        location.Loot.clear();

                        location.Opponents.clear();

                        auto max_rooms = std::max(location.Map.Width, location.Map.Height) * 2 / 5;

                        location.Map = Battlepits::Generate(location.Map.Width, location.Map.Height, max_rooms, 2, 3, false, 0);

                        function = Function::GENERATE;

                        done = true;
                    }
                }
                else if (controls[selected] == Controls::MapType("MAZE"))
                {
                    if (Interface::Confirm(graphics, scenes, "THIS CLEARS THE MAP - ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                    {
                        location.Triggers.clear();

                        location.Loot.clear();

                        location.Opponents.clear();

                        Maze::Generate(location.Map, location.Map.Width, location.Map.Height);

                        function = Function::GENERATE;

                        done = true;
                    }
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // helper function
    void EditTriggerMessage(Graphics::Base &graphics, Graphics::Scenery &scenes, Map::Base &map, std::string question, std::string &message)
    {
        message = Interface::TextBoxInput(graphics, scenes, Point(map.DrawX - BloodSwordRogue::Border, map.DrawY - BloodSwordRogue::Border), question, message, Color::Inactive, Color::Active, 1000, map.ViewX * map.TileSize + BloodSwordRogue::Border * 2, map.ViewY * map.TileSize + BloodSwordRogue::Border * 2, map.ViewX * map.TileSize, Color::Active, Color::Background, BloodSwordRogue::Border, true, true);
    }

    // edit trigger
    void EditTrigger(Graphics::Base &graphics, Graphics::Scenery &scenes, Map::Base &map, Trigger::Base &trigger)
    {
        Asset::List assets = {
            Asset::Map("CHECKBOX TREE"),
            Asset::Map("IDENTIFY"),
            Asset::Map("IDENTIFY"),
            Asset::Map("IDENTIFY"),
            Asset::Map("IDENTIFY"),
            Asset::Map("CONFIRM")};

        std::vector<std::string> captions = {
            "SELECT TRIGGER TYPE",
            "EDIT ENCOUNTER MESSAGE",
            "EDIT ACTIVE MESSAGE",
            "EDIT COMPLETION MESSAGE",
            "EDIT VARIABLES",
            "DONE"};

        auto done = false;

        while (!done)
        {
            auto selected = Interface::IconList(graphics, scenes, assets, captions);

            if (selected >= 0 && selected < SafeCast(captions.size()))
            {
                if (selected == 0)
                {
                    // set trigger type
                    auto select = -1;

                    if (trigger.Type != Trigger::Type::NONE)
                    {
                        for (auto i = 0; i < SafeCast(MapMaker::TriggerTypes.size()); i++)
                        {
                            if (trigger.Type == Trigger::Map(MapMaker::TriggerTypes[i]))
                            {
                                select = i;

                                break;
                            }
                        }
                    }

                    auto type = Interface::TextList(graphics, scenes, MapMaker::TriggerTypes, map.TileSize * 6, map.TileSize * 4, Asset::Map("CONFIRM"), Controls::MapType("CONFIRM"), select);

                    if (type >= 0 && type < SafeCast(MapMaker::TriggerTypes.size()))
                    {
                        trigger.Type = Trigger::Map(MapMaker::TriggerTypes[type]);
                    }
                    else
                    {
                        trigger.Type = Trigger::Type::NONE;
                    }
                }
                else if (selected == 1)
                {
                    // edit encounter message
                    MapMaker::EditTriggerMessage(graphics, scenes, map, "EDIT ENCOUNTER MESSAGE", trigger.EncounterMessage);
                }
                else if (selected == 2)
                {
                    // edit active message
                    MapMaker::EditTriggerMessage(graphics, scenes, map, "EDIT ACTIVE MESSAGE", trigger.ActiveMessage);
                }
                else if (selected == 3)
                {
                    // edit completion message
                    MapMaker::EditTriggerMessage(graphics, scenes, map, "EDIT COMPLETION MESSAGE", trigger.CompletedMessage);
                }
                else if (selected == 4)
                {
                    // edit variables
                    trigger.Variables = Interface::GetTextList(graphics, scenes, trigger.Variables, map.TileSize * 6, map.TileSize * 4);
                }
                else if (selected == 5)
                {
                    // check if trigger type is set
                    if (trigger.Type != Trigger::Type::NONE)
                    {
                        done = true;
                    }
                    else
                    {
                        Interface::MessageBox(graphics, scenes.back(), "TRIGGER TYPE NOT SET", Color::Highlight);
                    }
                }
            }
            else
            {
                // check if trigger type is set
                if (trigger.Type == Trigger::Type::NONE)
                {
                    Interface::MessageBox(graphics, scenes.back(), "TRIGGER TYPE NOT SET", Color::Highlight);
                }

                done = true;
            }
        }
    }

    void AddTrigger(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, Map::Tile &tile, Point &point, Location::Base &location)
    {
        if (!map.IsValid(point))
        {
            SDL_Log("[INVALID LOCATION] (%d, %d)", point.X, point.Y);

            return;
        }

        if ((tile.Type == Map::Object::PASSABLE || tile.Type == Map::Object::ENEMY_PASSABLE) && !tile.IsOccupied())
        {
            Graphics::Scenery scenes = {scene};

            auto id = tile.Id - 1;

            if (tile.Id == Map::NotFound)
            {
                auto trigger = Trigger::Base();

                MapMaker::EditTrigger(graphics, scenes, map, trigger);

                if (trigger.Type != Trigger::Type::NONE)
                {
                    trigger.X = point.X;

                    trigger.Y = point.Y;

                    location.Triggers.push_back(trigger);

                    tile.Occupant = Map::Object::TRIGGER;

                    tile.Id = SafeCast(location.Triggers.size());
                }
            }
            else if (id >= 0 && id < location.Triggers.size())
            {
                MapMaker::EditTrigger(graphics, scenes, map, location.Triggers[id]);
            }
            else
            {
                Interface::MessageBox(graphics, scene, "TRIGGER NOT FOUND", Color::Highlight);
            }
        }
        else
        {
            Interface::MessageBox(graphics, scene, "CANNOT ADD TRIGGER", Color::Highlight);

            return;
        }
    }

    // export map to .png image file
    void Export(Map::Base &map, const char *image_file)
    {
        SDL_Rect rect;

        auto offset = 16;

        rect.w = map.Width * map.TileSize + offset * 2;

        rect.h = map.Height * map.TileSize + offset * 2;

        auto surface = Graphics::CreateSurface(rect.w, rect.h);

        if (surface)
        {
            // fill entire map
            SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

            for (auto y = 0; y < map.Height; y++)
            {
                for (auto x = 0; x < map.Width; x++)
                {
                    SDL_Surface *surface_asset = nullptr;

                    auto &tile = map[Point(x, y)];

                    if (tile.Asset != Asset::NONE)
                    {
                        surface_asset = Asset::GetSurface(tile.Asset);
                    }
                    else
                    {
                        // fill empty space
                        surface_asset = Graphics::CreateSurface(map.TileSize, map.TileSize);

                        SDL_FillRect(surface_asset, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 255));
                    }

                    rect.x = x * map.TileSize + offset;

                    rect.y = y * map.TileSize + offset;

                    Graphics::RenderAssetThenFree(surface, surface_asset, rect);
                }
            }

            IMG_SavePNG(surface, image_file);

            SDL_FreeSurface(surface);
        }
    }

    void Main(Graphics::Base &graphics)
    {
        FontCache::Base TextCache = FontCache::Base();

        TextCache.Create(graphics.Renderer, Fonts::Normal, "0123456789(),", Color::S(Color::Active), TTF_STYLE_NORMAL);

        auto location = Location::Base();

        auto &map = location.Map;

        map.Initialize(32, 32);

        // get max edit window dimensions
        int TilesW = std::min(graphics.Width / map.TileSize - 4, 32);

        int TilesH = std::min(graphics.Height / map.TileSize - 7, 32);

        MapMaker::ResetMapView(graphics, map, TilesW, TilesH);

        auto done = false;

        auto input = Controls::User();

        auto mode = Mode::EDIT;

        auto function = Function::TILE;

        bool passable = false;

        Asset::Type asset = Asset::NONE;

        std::vector<std::string> controls_list = {
            "EDIT",
            "TILE",
            "ENEMY",
            "ITEMS",
            "TRIGGER",
            "TOGGLE",
            "CANCEL",
            "FILL",
            "CLEAR",
            "SETTINGS",
            "MAP",
            "NEW",
            "LOAD",
            "SAVE",
            "EXPORT",
            "EXIT"};

        std::vector<std::string> controls_assets = {
            "EDIT",
            "EMPTY SPACE",
            "ENEMY",
            "ITEMS",
            "TIME",
            "CIRCLE",
            "CANCEL",
            "PAINT ROLLER",
            "CROSS MARK",
            "GEARS",
            "MAP",
            "SELECT",
            "LOAD",
            "SAVE",
            "HEXES",
            "EXIT"};

        std::vector<std::string> controls_captions = {
            "EDIT MODE",
            "SET TILE",
            "ADD ENEMY",
            "ADD ITEM",
            "ADD TRIGGER",
            "TOGGLE PASSABLE",
            "REMOVE",
            "FILL AREA",
            "CLEAR AREA",
            "MAP SETTINGS",
            "PREVIEW",
            "NEW MAP",
            "LOAD LOCATION",
            "SAVE LOCATION",
            "EXPORT TO PNG",
            "QUIT"};

        // generate caption textures for edit controls
        auto captions = Graphics::CreateText(graphics, Graphics::GenerateTextList(controls_captions, Fonts::Caption, Color::Active, 0));

        // generate captions for tile icon grid
        std::vector<std::string> icon_captions = {};

        for (auto &asset : MapMaker::Assets)
        {
            icon_captions.push_back(Asset::TypeMapping[asset]);
        }

        // set correct asset on passable toggle
        controls_assets[5] = std::string(passable ? "CIRCLE" : "PLAIN CIRCLE");

        // add/remove/edit enemies/items
        Asset::List object_assets = {
            Asset::Map("CONFIRM"),
            Asset::Map("CANCEL"),
            Asset::Map("GEARS")};

        Asset::List object_controls = {
            Controls::MapType("CONFIRM"),
            Controls::MapType("CANCEL"),
            Controls::MapType("SETTINGS")};

        std::vector<std::string> object_captions = {
            "ADD",
            "REMOVE",
            "EDIT"};

        SDL_Texture *trigger_asset = nullptr;

        while (!done)
        {
            auto scene = Scene::Base();

            // map panel
            scene.Add(Scene::Element(map.DrawX - BloodSwordRogue::Border, map.DrawY - BloodSwordRogue::Border, map.ViewX * map.TileSize + BloodSwordRogue::Border * 2, map.ViewY * map.TileSize + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            MapMaker::RenderMap(graphics, scene, location);

            // top panel
            scene.Add(Scene::Element(BloodSwordRogue::Border, BloodSwordRogue::Border, graphics.Width - BloodSwordRogue::Border * 2, map.TileSize * 2 - BloodSwordRogue::Border * 2, Color::Background, Color::Inactive, BloodSwordRogue::Border));

            // mode text
            auto mode_text = std::string("MODE: ") + MapMaker::ModeText[mode];

            auto mode_asset = Graphics::CreateText(graphics, mode_text.c_str(), Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

            auto mode_size = BloodSwordRogue::Size(mode_asset);

            scene.Add(Scene::Element(mode_asset, Point(BloodSwordRogue::HalfTile, (map.TileSize * 2 - mode_size.Y) / 2)));

            // add tile text
            auto add_text = mode == Mode::FILL ? std::string("FILL: ") : std::string(" ADD: ");

            auto add_asset = Graphics::CreateText(graphics, add_text.c_str(), Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

            auto add_size = BloodSwordRogue::Size(add_asset);

            if (mode == Mode::EDIT || mode == Mode::FILL)
            {
                scene.Add(Scene::Element(add_asset, Point(BloodSwordRogue::HalfTile * 6, (map.TileSize * 2 - add_size.Y) / 2)));

                auto add_point = Point(BloodSwordRogue::HalfTile * 6 + add_size.X, BloodSwordRogue::HalfTile);

                if (function == Function::TILE)
                {
                    if (asset != Asset::NONE)
                    {
                        scene.VerifyAndAdd(Scene::Element(Asset::Get(asset), add_point));

                        if (!passable)
                        {
                            scene.Add(Scene::Element(add_point.X + 4, add_point.Y + 4, map.TileSize - 8, map.TileSize - 8, Color::Transparent, Color::Highlight, 2));
                        }
                    }
                    else
                    {
                        scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map(controls_assets[1].c_str())), add_point));
                    }
                }
                else if (function == Function::ENEMY && mode == Mode::EDIT)
                {
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map(controls_assets[2].c_str())), add_point));
                }
                else if (function == Function::LOOT && mode == Mode::EDIT)
                {
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map(controls_assets[3].c_str())), add_point));
                }
                else if (function == Function::TRIGGER && mode == Mode::EDIT)
                {
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map(controls_assets[4].c_str())), add_point));
                }
                else if (function == Function::ORIGIN && mode == Mode::EDIT)
                {
                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("SELECT TRANSPARENT BLUE")), add_point));
                }
            }

            // coordinates text
            auto coordinates_asset = Graphics::CreateText(graphics, "TILE: ", Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

            auto coordinates_size = BloodSwordRogue::Size(coordinates_asset);

            if (input.Current >= 0 && input.Current < scene.Controls.size() && scene.Controls[input.Current].OnMap)
            {
                auto origin = scene.Controls[input.Current].Map;

                auto coordinates = "(" + std::to_string(origin.X) + "," + std::to_string(origin.Y) + ")";

                auto mode_offset = (mode == Mode::EDIT || mode == Mode::FILL) ? 12 : 6;

                scene.Add(Scene::Element(coordinates_asset, Point(BloodSwordRogue::HalfTile * mode_offset, (map.TileSize * 2 - coordinates_size.Y) / 2)));

                Interface::AddText(scene, TextCache, coordinates, BloodSwordRogue::HalfTile * mode_offset + coordinates_size.X, (map.TileSize * 2 - coordinates_size.Y) / 2);
            }

            // map offsets text
            auto map_offsets_asset = Graphics::CreateText(graphics, "MAP OFFSETS: ", Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

            auto map_offsets_size = BloodSwordRogue::Size(map_offsets_asset);

            auto map_offsets = "(" + std::to_string(map.X) + "," + std::to_string(map.Y) + ")";

            auto map_offset = 18;

            scene.Add(Scene::Element(map_offsets_asset, Point(BloodSwordRogue::HalfTile * map_offset, (map.TileSize * 2 - map_offsets_size.Y) / 2)));

            Interface::AddText(scene, TextCache, map_offsets, BloodSwordRogue::HalfTile * map_offset + map_offsets_size.X, (map.TileSize * 2 - map_offsets_size.Y) / 2);

            // map dimensions text
            auto map_dimensions_asset = Graphics::CreateText(graphics, " DIMENSIONS: ", Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

            auto map_dimensions_size = BloodSwordRogue::Size(map_dimensions_asset);

            auto map_dimensions = "(" + std::to_string(map.Width) + "," + std::to_string(map.Height) + ")";

            auto map_dimension = 26;

            scene.Add(Scene::Element(map_dimensions_asset, Point(BloodSwordRogue::HalfTile * map_dimension, (map.TileSize * 2 - map_dimensions_size.Y) / 2)));

            Interface::AddText(scene, TextCache, map_dimensions, BloodSwordRogue::HalfTile * map_dimension + map_dimensions_size.X, (map.TileSize * 2 - map_dimensions_size.Y) / 2);

            // map controls (up)
            if (map.Y > 0)
            {
                auto id = scene.Controls.size();

                auto map_up = Point((graphics.Width - map.TileSize) / 2, (map.DrawY - map.TileSize - BloodSwordRogue::Border * 3));

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("UP")), map_up));

                scene.Add(Controls::Base(Controls::MapType("MAP UP"), id, id, id, id, id, map_up.X, map_up.Y, map.TileSize, map.TileSize, Color::Highlight));
            }

            // map controls (left)
            if (map.X > 0)
            {
                auto id = scene.Controls.size();

                auto map_left = Point((map.DrawX - map.TileSize - BloodSwordRogue::Border * 3), (graphics.Height - map.TileSize) / 2);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("LEFT")), map_left));

                scene.Add(Controls::Base(Controls::MapType("MAP LEFT"), id, id, id, id, id, map_left.X, map_left.Y, map.TileSize, map.TileSize, Color::Highlight));
            }

            // map controls (down)
            if (map.Y + map.ViewY < map.Height)
            {
                auto id = scene.Controls.size();

                auto map_down = Point((graphics.Width - map.TileSize) / 2, (map.DrawY + map.ViewY * map.TileSize + BloodSwordRogue::Border * 3));

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("DOWN")), map_down));

                scene.Add(Controls::Base(Controls::MapType("MAP DOWN"), id, id, id, id, id, map_down.X, map_down.Y, map.TileSize, map.TileSize, Color::Highlight));
            }

            // map controls (right)
            if (map.X + map.ViewX < map.Width)
            {
                auto id = scene.Controls.size();

                auto map_right = Point((map.DrawX + map.ViewX * map.TileSize + BloodSwordRogue::Border * 3), (graphics.Height - map.TileSize) / 2);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("RIGHT")), map_right));

                scene.Add(Controls::Base(Controls::MapType("MAP RIGHT"), id, id, id, id, id, map_right.X, map_right.Y, map.TileSize, map.TileSize, Color::Highlight));
            }

            // check if we need to flush controls on map view changes
            if (input.Type == Controls::MapType("MAP LEFT"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("MAP LEFT"));
            }
            else if (input.Type == Controls::MapType("MAP RIGHT"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("MAP RIGHT"));
            }
            else if (input.Type == Controls::MapType("MAP UP"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("MAP UP"));
            }
            else if (input.Type == Controls::MapType("MAP DOWN"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("MAP DOWN"));
            }

            // bottom panel
            scene.Add(Scene::Element(BloodSwordRogue::Border, graphics.Height - map.TileSize * 2 + BloodSwordRogue::Border, graphics.Width - BloodSwordRogue::Border * 2, map.TileSize * 2 - BloodSwordRogue::Border * 2, Color::Background, Color::Inactive, BloodSwordRogue::Border));

            if (Input::IsValid(scene, input) && scene.Controls[input.Current].OnMap && map[scene.Controls[input.Current].Map].IsOccupied())
            {
                // render occupants
                auto &tile = map[scene.Controls[input.Current].Map];

                auto id = tile.Id - 1;

                if (tile.Occupant == Map::Object::ENEMIES)
                {
                    auto party = location.Opponents[id];

                    for (auto i = 0; i < SafeCast(party.Count()); i++)
                    {
                        auto point = Point(BloodSwordRogue::HalfTile + i * (map.TileSize + BloodSwordRogue::Pad), graphics.Height - map.TileSize - BloodSwordRogue::HalfTile);

                        scene.VerifyAndAdd(Scene::Element(Asset::Get(party[i].Asset), point));
                    }
                }
                else if (tile.Occupant == Map::Object::ITEMS)
                {
                    auto loot = location.Loot[id].Items;

                    for (auto i = 0; i < SafeCast(loot.size()); i++)
                    {
                        auto point = Point(BloodSwordRogue::HalfTile + i * (map.TileSize + BloodSwordRogue::Pad), graphics.Height - map.TileSize - BloodSwordRogue::HalfTile);

                        scene.VerifyAndAdd(Scene::Element(Asset::Get(loot[i].Asset), point));
                    }
                }
                else if (tile.Occupant == Map::Object::TRIGGER)
                {
                    if (trigger_asset != nullptr)
                    {
                        BloodSwordRogue::Free(&trigger_asset);
                    }

                    if (id >= 0 && id < SafeCast(location.Triggers.size()))
                    {
                        auto &trigger = location.Triggers[id];

                        std::string trigger_text = std::string();

                        trigger_text += std::string("TRIGGER TYPE: ") + std::string(Trigger::TypeMapping[trigger.Type]) + (SafeCast(trigger.Variables.size()) > 0 ? (std::string(" (") + std::to_string(SafeCast(trigger.Variables.size())) + std::string(" VARIABLES)")) : std::string()) + std::string("\n");

                        trigger_text += std::string("   ENCOUNTER: ") + trigger.EncounterMessage.substr(0, 75) + (trigger.EncounterMessage.size() > 40 ? std::string("...\n") : std::string("\n"));

                        trigger_text += std::string("      ACTIVE: ") + trigger.ActiveMessage.substr(0, 75) + (trigger.ActiveMessage.size() > 40 ? std::string("...\n") : std::string("\n"));

                        trigger_text += std::string("    COMPLETE: ") + trigger.CompletedMessage.substr(0, 75) + (trigger.CompletedMessage.size() > 40 ? std::string("...\n") : std::string());

                        auto trigger_asset = Graphics::CreateText(graphics, trigger_text.c_str(), Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

                        auto point = Point(BloodSwordRogue::HalfTile, graphics.Height - map.TileSize - BloodSwordRogue::HalfTile);

                        scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("TIME")), point));

                        scene.VerifyAndAdd(Scene::Element(trigger_asset, point + Point(map.TileSize + BloodSwordRogue::Pad, -TTF_FontHeight(Fonts::Normal) / 2 - BloodSwordRogue::Pad)));
                    }
                }
            }
            else
            {
                // add edit controls
                auto id = scene.Controls.size();

                for (auto i = 0; i < SafeCast(controls_list.size()); i++)
                {
                    auto point = Point(BloodSwordRogue::HalfTile + i * (map.TileSize + BloodSwordRogue::Pad), graphics.Height - map.TileSize - BloodSwordRogue::HalfTile);

                    scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map(controls_assets[i].c_str())), point));

                    scene.Add(Controls::Base(Controls::MapType(controls_list[i].c_str()), id, i > 0 ? id - 1 : id, i < (SafeCast(controls_list.size()) - 1) ? id + 1 : id, id, id, point.X, point.Y, map.TileSize, map.TileSize, Color::Highlight));

                    id++;
                }

                // render edit button captions
                auto edit = Controls::Find(scene.Controls, Controls::MapType(controls_list[0].c_str()));

                if (Input::IsValid(scene, input) && input.Current >= edit && input.Current < (edit + SafeCast(controls_list.size())))
                {
                    auto caption = input.Current - edit;

                    auto &control = scene.Controls[input.Current];

                    // center caption
                    auto center = (control.W - BloodSwordRogue::Width(captions[caption])) / 2;

                    if ((control.X + center < (BloodSwordRogue::HalfTile * 2)) && input.Current == edit)
                    {
                        center = 0;
                    }

                    scene.VerifyAndAdd(Scene::Element(captions[caption], control.X + center, control.Y + control.H + 2));
                }
            }

            input = Input::WaitForInput(graphics, scene, input);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("SELECT"))
                {
                    if (input.Current >= 0 && input.Current < scene.Controls.size() && scene.Controls[input.Current].OnMap)
                    {
                        auto point = scene.Controls[input.Current].Map;

                        auto &tile = map[point];

                        if (mode == Mode::EDIT)
                        {
                            if (tile.IsOccupied())
                            {
                                if (function == Function::ENEMY && tile.Occupant == Map::Object::ENEMIES)
                                {
                                    auto selected = Interface::IconList(graphics, scene, object_assets, object_captions);

                                    if (selected >= 0 && selected < SafeCast(object_controls.size()))
                                    {
                                        if (object_controls[selected] == Controls::MapType("CONFIRM"))
                                        {
                                            MapMaker::AddEnemy(graphics, scene, map, tile, point, location);
                                        }
                                        else if (object_controls[selected] == Controls::MapType("CANCEL"))
                                        {
                                            MapMaker::RemoveEnemy(graphics, scene, map, tile, point, location);
                                        }
                                        else if (object_controls[selected] == Controls::MapType("SETTINGS"))
                                        {
                                            MapMaker::EditEnemy(graphics, scene, map, tile, point, location);
                                        }
                                    }
                                }
                                else if (function == Function::LOOT && tile.Occupant == Map::Object::ITEMS)
                                {
                                    auto selected = Interface::IconList(graphics, scene, object_assets, object_captions);

                                    if (selected >= 0 && selected < SafeCast(object_controls.size()))
                                    {
                                        if (object_controls[selected] == Controls::MapType("CONFIRM"))
                                        {
                                            MapMaker::AddLoot(graphics, scene, map, tile, point, location);
                                        }
                                        else if (object_controls[selected] == Controls::MapType("CANCEL"))
                                        {
                                            MapMaker::RemoveLoot(graphics, scene, map, tile, point, location);
                                        }
                                        else if (object_controls[selected] == Controls::MapType("SETTINGS"))
                                        {
                                            MapMaker::EditLoot(graphics, scene, map, tile, point, location);
                                        }
                                    }
                                }
                                else if (function == Function::TRIGGER && tile.Occupant == Map::Object::TRIGGER)
                                {
                                    auto id = tile.Id - 1;

                                    if (id >= 0 && id < SafeCast(location.Triggers.size()))
                                    {
                                        Graphics::Scenery scenes = {scene};

                                        MapMaker::EditTrigger(graphics, scenes, map, location.Triggers[id]);
                                    }
                                    else
                                    {
                                        Interface::MessageBox(graphics, scene, "TRIGGER NOT FOUND", Color::Highlight);
                                    }
                                }
                            }
                            else if (function == Function::TILE && asset != Asset::NONE)
                            {
                                tile.Asset = asset;

                                if (passable)
                                {
                                    tile.Type = Map::Object::PASSABLE;
                                }
                                else
                                {
                                    tile.Type = Map::Object::OBSTACLE;
                                }
                            }
                            else if (function == Function::ENEMY)
                            {
                                MapMaker::AddEnemy(graphics, scene, map, tile, point, location);
                            }
                            else if (function == Function::LOOT)
                            {
                                MapMaker::AddLoot(graphics, scene, map, tile, point, location);
                            }
                            else if (function == Function::ORIGIN && tile.Type == Map::Object::PASSABLE)
                            {
                                if (SafeCast(map.Origins.size()) > 0)
                                {
                                    map.Origins.clear();
                                }

                                map.Origins.push_back(point);
                            }
                            else if (function == Function::TRIGGER && (tile.Type == Map::Object::PASSABLE || tile.Type == Map::Object::ENEMY_PASSABLE))
                            {
                                MapMaker::AddTrigger(graphics, scene, map, tile, point, location);
                            }
                        }
                        else if (mode == Mode::ERASE)
                        {
                            if (!tile.IsOccupied())
                            {
                                tile.Asset = Asset::NONE;

                                tile.Type = Map::Object::PASSABLE;
                            }
                            else if (tile.IsOccupied())
                            {
                                if (tile.Occupant == Map::Object::ENEMIES)
                                {
                                    MapMaker::RemoveEnemy(graphics, scene, map, tile, point, location);
                                }
                                else if (tile.Occupant == Map::Object::ITEMS)
                                {
                                    MapMaker::RemoveLoot(graphics, scene, map, tile, point, location);
                                }
                                else if (tile.Occupant == Map::Object::TRIGGER)
                                {
                                    // remove trigger
                                    auto id = tile.Id - 1;

                                    if (id >= 0 && id < SafeCast(location.Triggers.size()))
                                    {
                                        location.Triggers.erase(location.Triggers.begin() + id);

                                        tile.Id = Map::NotFound;

                                        tile.Occupant = Map::Object::NONE;

                                        Location::RenumberTriggers(location);
                                    }
                                }
                            }
                        }
                        else if (mode == Mode::FILL && function == Function::TILE && asset != Asset::NONE)
                        {
                            auto type = passable ? Map::Object::PASSABLE : Map::Object::OBSTACLE;

                            if (map[point].Asset != asset || map[point].Type != type)
                            {
                                MapMaker::FloodFill(map, asset, type, point);
                            }
                        }
                        else if (mode == Mode::CLEAR)
                        {
                            if (tile.Asset != Asset::NONE || tile.Type != Map::Object::PASSABLE)
                            {
                                MapMaker::FloodFill(map, Asset::NONE, Map::Object::PASSABLE, point);
                            }
                        }
                    }
                }
                else if (input.Type == Controls::MapType("EDIT"))
                {
                    mode = Mode::EDIT;
                }
                else if (input.Type == Controls::MapType("TILE"))
                {
                    mode = (mode == Mode::EDIT || mode == Mode::FILL) ? mode : Mode::EDIT;

                    function = Function::TILE;

                    auto selected = Interface::IconGrid(graphics, scene, MapMaker::Assets, (map.ViewX + 1) * map.TileSize, (map.ViewY + 1) * map.TileSize + BloodSwordRogue::HalfTile, icon_captions);

                    if (selected >= 0 && selected < SafeCast(MapMaker::Assets.size()))
                    {
                        asset = MapMaker::Assets[selected];

                        // change asset in menu
                        controls_assets[1] = std::string(Asset::TypeMapping[asset]);
                    }
                }
                else if (input.Type == Controls::MapType("MAP UP"))
                {
                    if (map.Y > 0)
                    {
                        map.Y--;
                    }
                }
                else if (input.Type == Controls::MapType("MAP LEFT"))
                {
                    if (map.X > 0)
                    {
                        map.X--;
                    }
                }
                else if (input.Type == Controls::MapType("MAP RIGHT"))
                {
                    if (map.X + map.ViewX < map.Width)
                    {
                        map.X++;
                    }
                }
                else if (input.Type == Controls::MapType("MAP DOWN"))
                {
                    if (map.Y + map.ViewY < map.Height)
                    {
                        map.Y++;
                    }
                }
                else if (input.Type == Controls::MapType("ENEMY"))
                {
                    mode = Mode::EDIT;

                    function = Function::ENEMY;
                }
                else if (input.Type == Controls::MapType("ITEMS"))
                {
                    mode = Mode::EDIT;

                    function = Function::LOOT;
                }
                else if (input.Type == Controls::MapType("TRIGGER"))
                {
                    function = Function::TRIGGER;
                }
                else if (input.Type == Controls::MapType("TOGGLE"))
                {
                    passable = !passable;

                    controls_assets[5] = std::string(passable ? "CIRCLE" : "PLAIN CIRCLE");
                }
                else if (input.Type == Controls::MapType("CANCEL"))
                {
                    mode = Mode::ERASE;
                }
                else if (input.Type == Controls::MapType("FILL"))
                {
                    mode = Mode::FILL;

                    function = Function::TILE;
                }
                else if (input.Type == Controls::MapType("CLEAR"))
                {
                    mode = Mode::CLEAR;
                }
                else if (input.Type == Controls::MapType("SETTINGS"))
                {
                    Graphics::Scenery scenes = {scene};

                    MapMaker::MapSettings(graphics, scenes, location, function);

                    if (function == Function::ORIGIN)
                    {
                        mode = Mode::EDIT;
                    }
                    else if (function == Function::RESIZE)
                    {
                        MapMaker::RefreshMapView(graphics, map, TilesW, TilesH);

                        Interface::MessageBox(graphics, scene, std::string("MAP RESIZED"), Color::Active);

                        mode = Mode::EDIT;

                        function = Function::TILE;
                    }
                    else if (function == Function::GENERATE)
                    {
                        MapMaker::RefreshMapView(graphics, map, TilesW, TilesH);

                        Interface::MessageBox(graphics, scene, std::string("NEW MAP GENERATED"), Color::Active);

                        mode = Mode::EDIT;

                        function = Function::TILE;
                    }
                }
                else if (input.Type == Controls::MapType("MAP"))
                {
                    Interface::ShowMap(graphics, scene, map);
                }
                else if (input.Type == Controls::MapType("NEW"))
                {
                    if (Interface::Confirm(graphics, scene, "THIS CLEARS THE MAP - ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                    {
                        location.Triggers.clear();

                        location.Loot.clear();

                        location.Opponents.clear();

                        map.Initialize(map.Width, map.Height);

                        MapMaker::RefreshMapView(graphics, map, TilesW, TilesH);
                    }
                }
                else if (input.Type == Controls::MapType("LOAD"))
                {
                    if (Interface::Confirm(graphics, scene, "THIS CLEARS THE MAP - ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                    {
                        auto path = Files::GetMainPath() + std::string("/Locations");

                        Graphics::Scenery scenes = {scene};

                        auto extension = std::string(".json");

                        auto filename = Interface::FilesList(graphics, scenes, path, extension, map.TileSize * 6, map.TileSize * 4, Asset::Map("LOAD"), Controls::MapType("LOAD"));

                        if (!filename.empty())
                        {
                            auto src = path + std::string("/") + filename + extension;

                            SDL_Log("[LOAD AREA] [FILE %s]", filename.c_str());

                            auto json_file = Read(src.c_str());

                            if (!json_file.empty())
                            {
                                auto data = nlohmann::json::parse(json_file);

                                Location::Setup(location, data["location"]);

                                MapMaker::RefreshMapView(graphics, map, TilesW, TilesH);

                                auto loaded = (!location.Name.empty() ? location.Name : std::string("MAP")) + std::string(" LOADED");

                                Interface::MessageBox(graphics, scene, loaded, Color::Active);
                            }
                        }
                    }
                }
                else if (input.Type == Controls::MapType("SAVE"))
                {
                    auto path = Files::GetMainPath() + std::string("/Locations");

                    Graphics::Scenery scenes = {scene};

                    auto extension = std::string(".json");

                    auto filename = Interface::FilesList(graphics, scenes, path, extension, map.TileSize * 6, map.TileSize * 4, Asset::Map("SAVE"), Controls::MapType("SAVE"));

                    if (!filename.empty())
                    {
                        auto dst = path + std::string("/") + filename + extension;

                        SDL_Log("[SAVE AREA] [FILE %s]", filename.c_str());

                        Location::Save(location, dst.c_str());

                        auto saved = (!location.Name.empty() ? location.Name : std::string("MAP")) + std::string(" SAVED");

                        Interface::MessageBox(graphics, scene, saved, Color::Active);
                    }
                }
                else if (input.Type == Controls::MapType("EXPORT"))
                {
                    auto path = Files::GetMainPath() + std::string("/Locations/Exports");

                    Graphics::Scenery scenes = {scene};

                    auto extension = std::string(".png");

                    auto filename = Interface::FilesList(graphics, scenes, path, extension, map.TileSize * 6, map.TileSize * 4, Asset::Map("SAVE"), Controls::MapType("SAVE"));

                    if (!filename.empty())
                    {
                        auto dst = path + std::string("/") + filename + extension;

                        SDL_Log("[EXPORT AREA] [FILE %s]", filename.c_str());

                        MapMaker::Export(location.Map, dst.c_str());

                        auto saved = (!location.Name.empty() ? location.Name : std::string("MAP")) + std::string(" EXPORTED");

                        Interface::MessageBox(graphics, scene, saved, Color::Active);
                    }
                }
                else if (input.Type == Controls::MapType("EXIT"))
                {
                    done = Interface::Confirm(graphics, scene, "ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true);
                }
            }

            BloodSwordRogue::Free(&map_dimensions_asset);

            BloodSwordRogue::Free(&map_offsets_asset);

            BloodSwordRogue::Free(&coordinates_asset);

            BloodSwordRogue::Free(&mode_asset);

            BloodSwordRogue::Free(&add_asset);

            BloodSwordRogue::Free(&trigger_asset);
        }

        BloodSwordRogue::Free(captions);

        TextCache.Free();
    }

    // main loop
    int Start()
    {
        auto return_code = 0;

        auto graphics = Graphics::Initialize("BloodSword Rogue: Map Maker", "modules/default/images/icons/sword-wound.png");

        Interface::LoadSettings(graphics, "modules/default/settings.json");

        MapMaker::Load("modules/default/map-maker.json");

        MapMaker::Main(graphics);

        Interface::UnloadAssets();

        Graphics::Quit(graphics);

        return return_code;
    }
}

int main(int argc, char **argv)
{
    return BloodSwordRogue::MapMaker::Start();
}
