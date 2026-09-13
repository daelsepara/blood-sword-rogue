#pragma once

#include "Evaluate.hpp"
#include "Models.hpp"

namespace BloodSwordRogue::Game
{
    class World
    {
    public:
        std::string ZipFile = std::string();

        std::string Start = std::string();

        BloodSwordRogue::UnorderedMap<std::string, std::string> Locations = {};

        void Add(std::string name, std::string path)
        {
            this->Locations.insert_or_assign(name, path);
        }

        World() {}
    };

    class Base
    {
    public:
        std::string Name = std::string();

        Party::Base Party = Party::Base();

        BloodSwordRogue::UnorderedMap<std::string, Location::Base> Locations = {};

        Base() {}
    };

    // update / add visited location to game
    void Update(Game::Base &game, Location::Base location)
    {
        if (!location.Name.empty())
        {
            game.Locations.insert_or_assign(location.Name, location);
        }
    }

    // update party state in game
    void Copy(Game::Base &game, Party::Base party)
    {
        game.Party = Party::Base();

        game.Party.Module = std::string(party.Module);

        game.Party.Location = std::string(party.Location);

        game.Party.X = party.X;

        game.Party.Y = party.Y;

        game.Party.FieldOfView = party.FieldOfView;

        for (auto i = 0; i < SafeCast(party.Count()); i++)
        {
            game.Party.Add(party[i]);
        }

        game.Party.Variables = party.Variables;
    }

    bool HasLocation(Game::Base &game, std::string location)
    {
        return BloodSwordRogue::Has(game.Locations, location);
    }

    bool HasLocation(Game::World &world, std::string location)
    {
        return BloodSwordRogue::Has(world.Locations, location);
    }

    // load game world
    Game::World LoadWorld(const char *filename, const char *zip_file)
    {
        auto world = Game::World();

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, filename) : Read(filename);

        if (!json_file.empty())
        {
            auto data = nlohmann::json::parse(json_file);

            world.ZipFile = zip_file != nullptr ? std::string(zip_file) : std::string();

            world.Start = !data["start"].is_null() && data["start"].is_string() ? Engine::ToUpper(std::string(data["start"])) : std::string();

            if (!data["locations"].is_null() && data["locations"].is_object())
            {
                for (auto &location : data["locations"].items())
                {
                    auto name = Engine::ToUpper(std::string(location.key()));

                    auto path = std::string(location.value());

                    world.Add(name, path);
                }
            }
        }

        SDL_Log("[LOAD WORLD] [START %s] [LOCATIONS %d]", world.Start.c_str(), SafeCast(world.Locations.size()));

        return world;
    }

    Game::World LoadWorld(std::string filename, std::string zip_file)
    {
        return Game::LoadWorld(filename.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    Game::World LoadWorld(const char *filename)
    {
        return Game::LoadWorld(filename, nullptr);
    }

    Game::World LoadWorld(std::string filename)
    {
        return Game::LoadWorld(filename.c_str(), nullptr);
    }

    void ResetMapView(Graphics::Base &graphics, Map::Base &map, int width, int height)
    {
        // set edit window dimensions with spaces for map controls
        map.ViewX = width;

        map.ViewY = height;

        // set map offsets within the edit window
        map.X = (map.Width - map.ViewX) / 2;

        map.Y = (map.Height - map.ViewY) / 2;

        // set edit window positions on screen
        map.DrawX = (graphics.Width - (map.ViewX * map.TileSize)) / 2;

        map.DrawY = (graphics.Height - (map.ViewY * map.TileSize)) / 2;
    }

    // refresh map view
    void RefreshMapView(Graphics::Base &graphics, Map::Base &map, int &width, int &height)
    {
        width = std::min(map.ViewX, graphics.Width / map.TileSize - 2);

        height = std::min(map.ViewY, graphics.Height / map.TileSize - 5);

        Game::ResetMapView(graphics, map, width, height);
    }

    // refresh map view
    void RefreshMapView(Graphics::Base &graphics, Map::Base &map)
    {
        auto width = 0;

        auto height = 0;

        Game::RefreshMapView(graphics, map, width, height);
    }

    // leave location
    void Leave(Game::Base &game, Location::Base &location)
    {
        if (location.Map.IsValid(game.Party.Origin()))
        {
            auto &tile = location.Map[game.Party.Origin()];

            tile.Id = Map::NotFound;

            tile.Occupant = Map::Object::NONE;

            // record party's last position
            if (SafeCast(location.Map.Spawn.size() > 0))
            {
                location.Map.Spawn[0] = game.Party.Origin();
            }
            else
            {
                location.Map.Spawn.push_back(game.Party.Origin());
            }
        }
    }

    // move to a new location
    bool Move(Game::World &world, Game::Base &game, Location::Base &location, std::string next)
    {
        auto loaded = false;

        // check if area has been visited before
        if (Game::HasLocation(game, next))
        {
            SDL_Log("[MOVE %s] [VISITED]", next.c_str());

            Game::Leave(game, location);

            // copy updates
            Game::Update(game, location);

            auto move = game.Locations[next];

            Location::Setup(location, move);

            loaded = true;
        }
        else if (Game::HasLocation(world, next))
        {
            SDL_Log("[MOVE %s] [NEW]", next.c_str());

            Game::Leave(game, location);

            // copy updates
            Game::Update(game, location);

            auto move = world.Locations[next];

            loaded = Location::Load(location, move, world.ZipFile);
        }
        else
        {
            throw std::invalid_argument("LOCATION NOT FOUND!");
        }

        return loaded;
    }

    // exit to another area
    void Exit(Game::World &world, Game::Base &game, Location::Base &location, std::string next)
    {
        auto loaded = Game::Move(world, game, location, next);

        if (loaded)
        {
            game.Party.Location = std::string(next);

            if (SafeCast(location.Map.Spawn.size()) > 0)
            {
                auto x = location.Map.Spawn[0].X;

                auto y = location.Map.Spawn[0].Y;

                game.Party.X = x;

                game.Party.Y = y;

                location.Map.Put(Point(x, y), Map::Object::PARTY, Map::Party);
            }
            else
            {
                throw std::invalid_argument("ENTRY POINT INVALID!");
            }
        }
    }

    // move to a new location (start at location's origin point)
    void Travel(Game::World &world, Game::Base &game, Location::Base &location, std::string next)
    {
        auto loaded = Game::Move(world, game, location, next);

        if (loaded)
        {
            game.Party.Location = std::string(next);

            // move party to origin
            if (SafeCast(location.Map.Origins.size()) > 0)
            {
                game.Party.X = location.Map.Origins[0].X;

                game.Party.Y = location.Map.Origins[0].Y;

                location.Map.Put(location.Map.Origins[0], Map::Object::PARTY, Map::Party);
            }
            else
            {
                throw std::invalid_argument("LOCATION HAS NO ENTRY POINT!");
            }
        }
    }

    Models::Update CheckTrigger(Graphics::Base &graphics, Graphics::Scenery scenes, Game::World &world, Game::Base &game, Location::Base &location, Trigger::Base &trigger)
    {
        Models::Update update = {false, false, false};

        if (trigger.Type == Trigger::Type::NONE)
        {
            throw std::invalid_argument("TRIGGER TYPE NOT DEFINED!");
        }

        if (trigger.Type == Trigger::Type::TRAVEL)
        {
            if (!trigger.Activated)
            {
                if (!trigger.EncounterMessage.empty())
                {
                    Interface::MessageBox(graphics, scenes, trigger.EncounterMessage, Color::Active);
                }

                trigger.Activated = true;
            }
            else if (!trigger.ActiveMessage.empty())
            {
                Interface::MessageBox(graphics, scenes, trigger.ActiveMessage, Color::Active);
            }

            if (SafeCast(trigger.Variables.size()) > 0)
            {
                Game::Travel(world, game, location, trigger.Variables[0]);

                Game::RefreshMapView(graphics, location.Map);

                update.Scene = true;
            }
            else
            {
                throw std::invalid_argument("NEXT DESTINATION UNDEFINED!");
            }
        }
        else if (trigger.Type == Trigger::Type::EXIT)
        {
            if (SafeCast(trigger.Variables.size()) > 0)
            {
                auto next = trigger.Variables[0];

                Game::Exit(world, game, location, next);

                Game::RefreshMapView(graphics, location.Map);

                update.Scene = true;
            }
            else
            {
                throw std::invalid_argument("CANNOT MOVE TO NEXT LOCATION!");
            }
        }
        else if (!trigger.Activated)
        {
            Interface::MessageBox(graphics, scenes, trigger.EncounterMessage, Color::Active);

            trigger.Activated = true;
        }
        else if (!trigger.Completed)
        {
            // check trigger conditions
            if (trigger.Type == Trigger::Type::CHARACTER)
            {
                trigger.Completed = Evaluate::InParty(trigger, game.Party);
            }
            else if (trigger.Type == Trigger::Type::ITEM)
            {
                trigger.Completed = Evaluate::HasItem(trigger, game.Party);
            }
            else if (trigger.Type == Trigger::Type::ANY_ITEMS || trigger.Type == Trigger::Type::ALL_ITEMS)
            {
                trigger.Completed = Evaluate::HasItems(trigger, game.Party);
            }

            // send status message
            if (trigger.Completed)
            {
                if (!trigger.CompletedMessage.empty())
                {
                    Interface::MessageBox(graphics, scenes, trigger.CompletedMessage, Color::Active);
                }
            }
            else
            {
                if (!trigger.ActiveMessage.empty())
                {
                    Interface::MessageBox(graphics, scenes, trigger.ActiveMessage, Color::Inactive);
                }
            }
        }

        return update;
    }

    // render location map and contents
    void RenderLocation(Scene::Base &scene, Location::Base &location, Party::Base &party, FieldOfView::Method method, bool sight = true)
    {
        auto &map = location.Map;

        // set fog color
        auto fog = Color::O(Color::Active, 0x14);

        // get leading character's awareness
        auto first = Engine::First(party);

        // set field of view radius
        auto radius = Engine::IsAlive(party) ? party[first].Value(Attribute::Type::AWARENESS) / 2 : 0;

        // calculate field of view
        auto view = FieldOfView::Compute(map, party.Origin(), radius, method);

        // offset for FoV illumination
        auto fov_offset = BloodSwordRogue::Pad;

        // size of FoV illumination
        auto fov_size = BloodSwordRogue::TileSize - fov_offset * 2;

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

                auto visible = BloodSwordRogue::In(view, x, y);

                auto loot_id = Map::NotFound;

                auto opponent_id = Map::NotFound;

                if (visible || tile.Explored)
                {
                    if (tile.IsOccupied())
                    {
                        switch (tile.Occupant)
                        {
                        case Map::Object::PARTY:

                            if (Engine::IsAlive(party) && tile.Id == Map::Party)
                            {
                                auto first = Engine::First(party);

                                auto &player = party[first];

                                if (Engine::IsAlive(player))
                                {
                                    scene.VerifyAndAdd(Scene::Element(Asset::Get(player.Asset), screen));
                                }
                            }

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

                            break;

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
                    else if (visible && sight)
                    {
                        // show field of view
                        scene.Add(Scene::Element(screen.X + fov_offset, screen.Y + fov_offset, fov_size, fov_size, Color::O(Color::Highlight, 0x20)));
                    }
                }

                if (visible)
                {
                    // mark tile as explored
                    tile.Explored = true;
                }
                else if (tile.Explored)
                {
                    // blur tiles
                    scene.Add(Scene::Element(screen.X, screen.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Blur));
                }
                else
                {
                    // fog
                    scene.Add(Scene::Element(screen.X, screen.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, fog));
                }

                auto type = Controls::NONE;

                if (tile.Explored && (tile.IsPassable() || tile.Type == Map::Object::TRIGGER || tile.Occupant == Map::Object::ITEMS || tile.Occupant == Map::Object::ENEMIES))
                {
                    type = Controls::MapType("MOVE");
                }
                else if (party.Origin() == Point(x, y))
                {
                    type = Controls::MapType("PARTY");
                }

                scene.Add(Controls::Base(type, id, id, id, id, id, screen.X, screen.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight, x, y));

                id++;
            }
        }
    }

    // center map on src (point)
    void Center(Location::Base &location, Point src)
    {
        location.Map.X = src.X - (location.Map.ViewX) / 2 + 1;

        location.Map.Y = src.Y - (location.Map.ViewY) / 2 + 1;

        location.Map.CheckBounds();
    }

    // center map on entity (map object type, id)
    void Center(Location::Base &location, Map::Object entity, int id)
    {
        auto src = location.Map.Find(entity, id);

        Game::Center(location, src);
    }

    // check if tile is blocked for movement
    bool Blocked(Location::Base &location, Point point)
    {
        auto &tile = location.Map[point];

        auto triggered = (tile.Occupant == Map::Object::TRIGGER);

        auto items = (tile.IsOccupied() && tile.Occupant == Map::Object::ITEMS);

        auto blockers = (tile.IsOccupied() && tile.Occupant != Map::Object::ITEMS && tile.Occupant != Map::Object::TRIGGER);

        // additional checks
        if (items)
        {
            auto loot = Location::FindLoot(location, point);

            if (loot >= 0 && loot < SafeCast(location.Loot.size()))
            {
                items &= (SafeCast(location.Loot[loot].Items.size()) > 0);
            }
            else
            {
                items = false;
            }
        }

        if (triggered)
        {
            triggered &= (Location::FindTrigger(location, point) != Map::NotFound);
        }

        if (items)
        {
            SDL_Log("[BLOCKED] [(%d, %d) %s]", point.X, point.Y, "ITEMS");
        }
        else if (triggered)
        {
            SDL_Log("[BLOCKED] [(%d, %d) %s]", point.X, point.Y, "TRIGGER");
        }

        return (items || blockers || triggered || tile.IsBlocked() || !tile.IsPassable());
    }

    // check if party can move to location (point)
    bool Move(Game::Base &game, Location::Base &location, Point point)
    {
        auto moved = !Game::Blocked(location, point);

        auto &party = game.Party;

        if (moved)
        {
            auto from = party.Origin();

            auto &origin = location.Map[from];

            auto &destination = location.Map[point];

            origin.Occupant = Map::Object::NONE;

            origin.Id = Map::NotFound;

            destination.Occupant = Map::Object::PARTY;

            destination.Id = Map::Party;

            party.X = point.X;

            party.Y = point.Y;
        }

        return moved;
    }

    // remove loot from map
    void RemoveLoot(Graphics::Base &graphics, Graphics::Scenery scenes, Game::Base &game, Location::Base &location, Point point)
    {
        auto loot = Location::FindLoot(location, point);

        if (loot >= 0 && loot < SafeCast(location.Loot.size()) && location.Map.IsValid(point))
        {
            auto &tile = location.Map[point];

            tile.Id = Map::NotFound;

            tile.Occupant = Map::Object::NONE;

            location.Loot.erase(location.Loot.begin() + loot);

            Location::RenumberLoot(location);
        }
    }

    // returns the plural for the item
    std::string GetPlural(Item::Type item)
    {
        auto plural = std::string();

        for (auto i = 0; i < SafeCast(Interface::ItemsWithQuantities.size()); i++)
        {
            if (Item::MapType(Interface::ItemsWithQuantities[i]) == item)
            {
                plural = std::string(Interface::ItemPlurals[i]);

                break;
            }
        }

        return plural;
    }

    // character takes item from inventory
    void TakeItem(Graphics::Base &graphics, Graphics::Scenery scenes, Game::Base &game, Character::Base &character, Items::Inventory &items, int item)
    {
        if (item >= 0 && item < SafeCast(items.size()))
        {
            if (character.TotalEncumbrance() + items[item].Encumbrance > character.EncumbranceLimit)
            {
                std::string encumbrance = std::string("INVENTORY FULL!");

                Interface::MessageBox(graphics, scenes, encumbrance, Color::Highlight);
            }
            else
            {
                auto container = Item::Container(items[item].Type);

                // check if item needs storage
                if (container != Item::NONE)
                {
                    if (character.HasItemType(container))
                    {
                        // add to quantity
                        auto added = character.Add(items[item].Type, items[item].Quantity);

                        if (added)
                        {
                            std::string plural = std::string(" ") + Game::GetPlural(items[item].Type);

                            std::string taken = std::to_string(items[item].Quantity) + plural + std::string(" TAKEN");

                            Interface::MessageBox(graphics, scenes, taken, Color::Active);

                            items.erase(items.begin() + item);
                        }
                        else
                        {
                            std::string cannot = std::string("CANNOT TAKE THE ") + items[item].Name + std::string("!");

                            Interface::MessageBox(graphics, scenes, cannot, Color::Highlight);
                        }
                    }
                    else
                    {
                        // missing container
                        std::string missing = std::string("NO ") + Item::TypeMapping[container] + std::string(" TO STORE THE ") + Game::GetPlural(items[item].Type) + "!";

                        Interface::MessageBox(graphics, scenes, missing, Color::Highlight);
                    }
                }
                else
                {
                    // take item
                    std::string taken = items[item].Name + std::string(" TAKEN");

                    Interface::MessageBox(graphics, scenes, taken, Color::Active);

                    character.Items.push_back(items[item]);

                    items.erase(items.begin() + item);
                }
            }
        }
    }

    // view items
    void ViewItems(Graphics::Base &graphics, Graphics::Scenery scenes, Game::Base &game, Items::Inventory &items)
    {
        Asset::List assets = {
            Asset::Map("MAGNIFYING GLASS"),
            Asset::Map("USE")};

        Controls::List actions = {
            Controls::MapType("VIEW"),
            Controls::MapType("TAKE")};

        Interface::Strings captions = {
            "VIEW",
            "TAKE"};

        while (true && SafeCast(items.size()) > 0)
        {
            auto item = Interface::SelectItem(graphics, scenes, items);

            if (item >= 0 && item < SafeCast(items.size()))
            {
                auto action = Interface::IconList(graphics, scenes, assets, captions);

                if (action >= 0 && action < SafeCast(actions.size()))
                {
                    if (actions[action] == Controls::MapType("VIEW"))
                    {
                        Interface::ViewItem(graphics, scenes, items[item], false);
                    }
                    else if (actions[action] == Controls::MapType("TAKE"))
                    {
                        auto character = Interface::SelectCharacter(graphics, scenes, game.Party);

                        if (character >= 0 && character < game.Party.Count() && Engine::IsAlive(game.Party[character]))
                        {
                            Game::TakeItem(graphics, scenes, game, game.Party[character], items, item);
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    // equip item
    void EquipItem(Graphics::Base &graphics, Graphics::Scenery &scenes, Game::Base &game, int character, int item)
    {
        Asset::List melee_assets = {
            Asset::Map("PRIMARY"),
            Asset::Map("SECONDARY")};

        Item::Properties melee_types = {
            Item::MapProperty("PRIMARY"),
            Item::MapProperty("SECONDARY")};

        Interface::Strings melee_captions = {
            "PRIMARY",
            "SECONDARY"};

        auto &items = game.Party[character].Items;

        if (item < 0 || item >= SafeCast(items.size()))
        {
            return;
        }

        auto can_equip = true;

        for (auto &requirement : Interface::ItemSkills)
        {
            if (items[item].Type == requirement.first && !game.Party[character].HasSkill(requirement.second))
            {
                can_equip = false;

                break;
            }
        }

        if (!can_equip)
        {
            std::string cannot_equip = std::string("CANNOT EQUIP ") + items[item].Name;

            Interface::MessageBox(graphics, scenes, cannot_equip, Color::Highlight);

            return;
        }

        if (items[item].HasProperty(Item::MapProperty("WEAPON")))
        {
            auto weapon_type = Item::MapProperty("PRIMARY");

            if (game.Party[character].HasSkill(Skills::Map("AMBIDEXTERITY")) && !items[item].HasProperty(Item::MapProperty("RANGED")))
            {
                auto weapon = Interface::IconList(graphics, scenes, melee_assets, melee_captions);

                if (weapon >= 0 && weapon < SafeCast(melee_types.size()))
                {
                    weapon_type = melee_types[weapon];
                }
                else
                {
                    weapon_type = Item::NONE;
                }
            }
            else if (items[item].HasProperty(Item::MapProperty("RANGED")))
            {
                weapon_type = Item::MapProperty("RANGED");
            }

            if (weapon_type != Item::NONE)
            {
                auto equipped = game.Party[character].EquippedWeapon(weapon_type);

                if (equipped >= 0 && equipped < SafeCast(items.size()))
                {
                    items[equipped].RemoveProperty(Item::MapProperty("EQUIPPED"));

                    if (weapon_type != Item::MapProperty("RANGED"))
                    {
                        items[equipped].RemoveProperty(weapon_type);
                    }
                }

                items[item].AddProperty(Item::MapProperty("EQUIPPED"));

                if (weapon_type != Item::MapProperty("RANGED"))
                {
                    items[item].AddProperty(weapon_type);
                }
            }
        }
        else if (items[item].HasProperty(Item::MapProperty("ARMOUR")))
        {
            auto equipped = game.Party[character].EquippedArmour();

            if (equipped >= 0 && equipped < SafeCast(items.size()))
            {
                items[equipped].RemoveProperty(Item::MapProperty("EQUIPPED"));
            }

            items[item].AddProperty(Item::MapProperty("EQUIPPED"));
        }
        else if (items[item].HasProperty(Item::MapProperty("ACCESSORY")))
        {
            items[item].AddProperty(Item::MapProperty("EQUIPPED"));
        }
    }

    // unequip item
    void UnequipItem(Items::Inventory &items, int item)
    {
        items[item].RemoveProperty(Item::MapProperty("EQUIPPED"));

        if (items[item].HasProperty(Item::MapProperty("PRIMARY")))
        {
            items[item].RemoveProperty(Item::MapProperty("PRIMARY"));
        }
        else if (items[item].HasProperty(Item::MapProperty("SECONDARY")))
        {
            items[item].RemoveProperty(Item::MapProperty("SECONDARY"));
        }
    }

    // view items
    void ViewItems(Graphics::Base &graphics, Graphics::Scenery scenes, Game::Base &game, Location::Base &location, int character)
    {
        Asset::List assets = {
            Asset::Map("MAGNIFYING GLASS"),
            Asset::Map("USE"),
            Asset::Map("TRADE"),
            Asset::Map("CANCEL")};

        Controls::List actions = {
            Controls::MapType("VIEW"),
            Controls::MapType("USE"),
            Controls::MapType("TRADE"),
            Controls::MapType("DROP")};

        Interface::Strings captions = {
            "VIEW",
            "USE",
            "TRADE",
            "DROP"};

        Asset::List gear_assets = {
            Asset::Map("MAGNIFYING GLASS"),
            Asset::Map("GEAR"),
            Asset::Map("TRADE"),
            Asset::Map("CANCEL")};

        Controls::List gear_actions = {
            Controls::MapType("VIEW"),
            Controls::MapType("EQUIP"),
            Controls::MapType("TRADE"),
            Controls::MapType("DROP")};

        Interface::Strings gear_captions = {
            "VIEW",
            "EQUIP",
            "TRADE",
            "DROP"};

        if (character >= 0 && character < game.Party.Count())
        {
            auto &items = game.Party[character].Items;

            while (true && SafeCast(items.size()) > 0)
            {
                auto item = Interface::SelectItem(graphics, scenes, game.Party[character]);

                if (item >= 0 && item < SafeCast(items.size()))
                {
                    auto action = -1;

                    auto action_limit = SafeCast(actions.size());

                    auto gear = false;

                    while (true)
                    {
                        if (items[item].HasProperty(Item::MapProperty("WEAPON")) || items[item].HasProperty(Item::MapProperty("ARMOUR")) || items[item].HasProperty(Item::MapProperty("ACCESSORY")))
                        {
                            gear = true;

                            gear_assets[1] = items[item].HasProperty(Item::MapProperty("EQUIPPED")) ? Asset::Map("EQUIPPED") : Asset::Map("GEAR");

                            gear_actions[1] = items[item].HasProperty(Item::MapProperty("EQUIPPED")) ? Controls::MapType("UNEQUIP") : Controls::MapType("EQUIP");

                            gear_captions[1] = items[item].HasProperty(Item::MapProperty("EQUIPPED")) ? "UNEQUIP" : "EQUIP";

                            action_limit = SafeCast(gear_actions.size());

                            action = Interface::IconList(graphics, scenes, gear_assets, gear_captions);
                        }
                        else
                        {
                            action = Interface::IconList(graphics, scenes, assets, captions);
                        }

                        if (action >= 0 && action < action_limit)
                        {
                            if ((!gear && actions[action] == Controls::MapType("VIEW")) || (gear && gear_actions[action] == Controls::MapType("VIEW")))
                            {
                                Interface::ViewItem(graphics, scenes, items[item], false);
                            }
                            else if (!gear && actions[action] == Controls::MapType("USE"))
                            {
                            }
                            else if (gear && gear_actions[action] == Controls::MapType("EQUIP"))
                            {
                                Game::EquipItem(graphics, scenes, game, character, item);
                            }
                            else if (gear && gear_actions[action] == Controls::MapType("UNEQUIP"))
                            {
                                Game::UnequipItem(items, item);
                            }
                            else if ((!gear && actions[action] == Controls::MapType("TRADE")) || (gear && gear_actions[action] == Controls::MapType("TRADE")))
                            {
                            }
                            else if ((!gear && actions[action] == Controls::MapType("DROP")) || (gear && gear_actions[action] == Controls::MapType("DROP")))
                            {
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    Models::Update Menu(Graphics::Base &graphics, Graphics::Scenery scenes, Game::Base &game, Location::Base &location, Point point)
    {
        Models::Update update = {false, false, false};

        Asset::List assets = {
            Asset::Map("ITEMS"),
            Asset::Map("MAP"),
            Asset::Map("EXIT")};

        Controls::List actions = {
            Controls::MapType("ITEMS"),
            Controls::MapType("MAP"),
            Controls::MapType("EXIT")};

        Interface::Strings captions = {
            "INVENTORY",
            "VIEW MAP",
            "QUIT GAME"};

        while (true)
        {
            auto selected = Interface::IconList(graphics, scenes, assets, captions);

            if (selected >= 0 && selected < SafeCast(actions.size()))
            {
                if (actions[selected] == Controls::MapType("ITEMS"))
                {
                    while (true)
                    {
                        auto character = Interface::SelectCharacter(graphics, scenes, game.Party);

                        if (character >= 0 && character < game.Party.Count())
                        {
                            Game::ViewItems(graphics, scenes, game, location, character);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else if (actions[selected] == Controls::MapType("MAP"))
                {
                    Interface::ShowMap(graphics, scenes, location.Map, true);
                }
                else if (actions[selected] == Controls::MapType("EXIT"))
                {
                    if (Interface::Confirm(graphics, scenes, "ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                    {
                        update.Quit = true;

                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }

        return update;
    }

    // handle tile interaction (items/enemies)
    Models::Update Handle(Graphics::Base &graphics, Graphics::Scenery scenes, Game::World &world, Game::Base &game, Location::Base &location, Point point)
    {
        Models::Update update = {false, false, false};

        auto &tile = location.Map[point];

        if (tile.IsOccupied())
        {
            if (tile.Occupant == Map::Object::ITEMS)
            {
                auto loot = Location::FindLoot(location, point);

                if (loot >= 0 && loot < SafeCast(location.Loot.size()))
                {
                    auto &items = location.Loot[loot].Items;

                    Game::ViewItems(graphics, scenes, game, items);

                    if (SafeCast(items.size()) <= 0)
                    {
                        Game::RemoveLoot(graphics, scenes, game, location, point);
                    }

                    update.Scene = true;

                    update.Party = true;

                    Input::Clear();
                }
            }
            else if (tile.Occupant == Map::Object::ENEMIES)
            {
                auto enemy = Location::FindOpponents(location, point);

                if (enemy >= 0 && enemy < SafeCast(location.Opponents.size()))
                {
                    update.Scene = true;

                    update.Party = true;

                    Input::Clear();
                }
            }
            else if (tile.Occupant == Map::Object::TRIGGER)
            {
                // handle trigger
                auto id = Location::FindTrigger(location, point);

                if (id >= 0 && id < SafeCast(location.Triggers.size()))
                {
                    auto &trigger = location.Triggers[id];

                    update = Game::CheckTrigger(graphics, scenes, world, game, location, trigger);
                }
            }
        }

        return update;
    }

    // process party actions
    Models::Update Actions(Graphics::Base &graphics, Graphics::Scenery background, Game::World &world, Game::Base &game, Location::Base &location, Point point, Controls::List &input_buffer)
    {
        Models::Update result = {false, false};

        if (Game::Blocked(location, point))
        {
            // clear input buffer
            input_buffer.clear();

            result = Game::Handle(graphics, background, world, game, location, point);
        }
        else if (Game::Move(game, location, point))
        {
            result.Scene = true;
        }

        return result;
    }

    // update scene for location mode
    Scene::Base UpdateScene(Graphics::Base &graphics, Game::Base &game, Location::Base &location, FieldOfView::Method method, bool animating)
    {
        auto scene = Scene::Base();

        auto panel_w = graphics.Width / BloodSwordRogue::TileSize - 2;

        auto panel_h = graphics.Height / BloodSwordRogue::TileSize - 5;

        auto panel_x = (graphics.Width - panel_w * BloodSwordRogue::TileSize) / 2;

        auto panel_y = (graphics.Height - panel_h * BloodSwordRogue::TileSize) / 2;

        // map panel
        scene.Add(Scene::Element(panel_x - BloodSwordRogue::Border, panel_y - BloodSwordRogue::Border, panel_w * BloodSwordRogue::TileSize + BloodSwordRogue::Border * 2, panel_h * BloodSwordRogue::TileSize + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

        Game::Center(location, Map::Object::PARTY, Map::Party);

        Game::RenderLocation(scene, location, game.Party, method, false);

        return scene;
    }

    void Setup()
    {
        // additional setup
    }

    void Main(Graphics::Base &graphics)
    {
        Game::Setup();

        FontCache::Base TextCache = FontCache::Base();

        TextCache.Create(graphics.Renderer, Fonts::Normal, "0123456789(),", Color::S(Color::Active), TTF_STYLE_NORMAL);

        // set FOV algorithm
        auto method = FieldOfView::Map(Engine::ToUpper(Interface::Settings["fov"]));

        auto game = Game::Base();

        auto world = Game::LoadWorld("modules/default/world.json");

        auto location = Location::Base();

        // add all characters to party
        for (auto character_class : Character::All)
        {
            auto character = Generate::Character(character_class, 2);

            game.Party.Add(character);
        }

        int width = std::min(graphics.Width / BloodSwordRogue::TileSize - 2, 32);

        int height = std::min(graphics.Height / BloodSwordRogue::TileSize - 5, 32);

        Game::Travel(world, game, location, world.Start);

        Game::RefreshMapView(graphics, location.Map, width, height);

        auto input_buffer = Controls::List();

        auto animating = false;

        Models::Update update = {true, false, false};

        auto scene = Scene::Base();

        auto input = Controls::User();

        auto done = false;

        // coordinates
        SDL_Texture *location_name = nullptr;

        auto location_size = Point();

        auto prev_location = std::string();

        while (!done)
        {
            if (update.Scene || animating)
            {
                scene = Game::UpdateScene(graphics, game, location, method, false);

                update.Scene = false;
            }

            // top panel
            scene.Add(Scene::Element(BloodSwordRogue::Border, BloodSwordRogue::Border, graphics.Width - BloodSwordRogue::Border * 2, BloodSwordRogue::TileSize * 2 - BloodSwordRogue::Border * 2, Color::Background, Color::Inactive, BloodSwordRogue::Border));

            if (!(prev_location == location.Name.c_str()))
            {
                BloodSwordRogue::Free(&location_name);

                location_name = Graphics::CreateText(graphics, (location.Name + std::string(" ")).c_str(), Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

                location_size = BloodSwordRogue::Size(location_name);

                prev_location = std::string(location.Name);
            }

            auto coordinates = "(" + std::to_string(game.Party.X) + "," + std::to_string(game.Party.Y) + ")";

            scene.Add(Scene::Element(location_name, Point(BloodSwordRogue::HalfTile, (location.Map.TileSize * 2 - location_size.Y) / 2)));

            Interface::AddText(scene, TextCache, coordinates, BloodSwordRogue::HalfTile + location_size.X, (location.Map.TileSize * 2 - location_size.Y) / 2);

            // bottom panel
            scene.Add(Scene::Element(BloodSwordRogue::Border, graphics.Height - BloodSwordRogue::TileSize * 2 + BloodSwordRogue::Border, graphics.Width - BloodSwordRogue::Border * 2, BloodSwordRogue::TileSize * 2 - BloodSwordRogue::Border * 2, Color::Background, Color::Inactive, BloodSwordRogue::Border));

            // show members of the party
            for (auto i = 0; i < SafeCast(game.Party.Count()); i++)
            {
                auto point = Point(BloodSwordRogue::HalfTile + i * (location.Map.TileSize + BloodSwordRogue::Pad), graphics.Height - location.Map.TileSize - BloodSwordRogue::HalfTile);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(game.Party[i].Asset), point));

                if (!Engine::IsAlive(game.Party[i]))
                {
                    scene.Add(Scene::Element(point.X, point.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Blur));
                }
            }

            Graphics::Scenery scenes = {scene};

            if (!animating)
            {
                auto input = Input::RogueInput(graphics, scenes);

                auto prev = game.Party.Origin();

                // check for buffered input
                if (input_buffer.size() > 0)
                {
                    input.Selected = true;

                    input.Type = input_buffer.front();

                    input_buffer.erase(input_buffer.begin());

                    SDL_Delay(BloodSwordRogue::StandardDelay);
                }

                if (Input::Check(input))
                {
                    auto point = game.Party.Origin();

                    if (input.Type == Controls::MapType("MENU"))
                    {
                        update = Game::Menu(graphics, scenes, game, location, point);

                        done = update.Quit;
                    }
                    else if (input.Type == Controls::MapType("MAP"))
                    {
                        Interface::ShowMap(graphics, scenes, location.Map, true);
                    }
                    else if (input.Type == Controls::MapType("UP"))
                    {
                        if (point.Y > 0)
                        {
                            point.Y--;

                            update = Game::Actions(graphics, scenes, world, game, location, point, input_buffer);
                        }
                    }
                    else if (input.Type == Controls::MapType("DOWN"))
                    {
                        if (point.Y < location.Map.Height - 1)
                        {
                            point.Y++;

                            update = Game::Actions(graphics, scenes, world, game, location, point, input_buffer);
                        }
                    }
                    else if (input.Type == Controls::MapType("LEFT"))
                    {
                        if (point.X > 0)
                        {
                            point.X--;

                            update = Game::Actions(graphics, scenes, world, game, location, point, input_buffer);
                        }
                    }
                    else if (input.Type == Controls::MapType("RIGHT"))
                    {
                        if (point.X < location.Map.Width - 1)
                        {
                            point.X++;

                            update = Game::Actions(graphics, scenes, world, game, location, point, input_buffer);
                        }
                    }
                    else if (input.Type == Controls::MapType("EXIT"))
                    {
                        done = Interface::Confirm(graphics, scenes, "ARE YOU SURE?", Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true);
                    }

                    // trigger event on movement
                    if (prev != game.Party.Origin())
                    {
                    }

                    input.Selected = false;
                }
            }
        }

        BloodSwordRogue::Free(&location_name);

        TextCache.Free();
    }
}