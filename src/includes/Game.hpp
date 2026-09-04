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
        game.Locations.insert_or_assign(location.Name, location);
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

    // move to a new location
    void Move(Game::World &world, Game::Base &game, Location::Base &location, std::string next)
    {
        auto loaded = false;

        // check if area has been visited before
        if (Game::HasLocation(game, next))
        {
            // copy updates
            Game::Update(game, location);

            auto move = game.Locations[next];

            Location::Setup(location, move);

            loaded = true;
        }
        else if (Game::HasLocation(world, next))
        {
            auto move = world.Locations[next];

            loaded = Location::Load(location, move, world.ZipFile);
        }
        else
        {
            throw std::invalid_argument("LOCATION NOT FOUND!");
        }

        if (loaded)
        {
            game.Party.Location = std::string(next);

            // move party to origin
            if (SafeCast(location.Map.Origins.size()) > 0)
            {
                game.Party.X = location.Map.Origins[0].X;

                game.Party.Y = location.Map.Origins[0].Y;

                location.Map.Put(location.Map.Origins[0], Map::Object::PARTY, 1);
            }
            else
            {
                throw std::invalid_argument("LOCATION HAS NO ENTRY POINT!");
            }
        }
    }

    // current save
    Game::Base CurrentGame = Game::Base();

    // current world
    Game::World CurrentWorld = Game::World();

    void CheckTrigger(Graphics::Base &graphics, Scene::Base &scene, Game::World &world, Game::Base &game, Location::Base &location, Trigger::Base &trigger)
    {
        if (trigger.Type == Trigger::Type::EXIT)
        {
            if (!trigger.Activated)
            {
                if (!trigger.EncounterMessage.empty())
                {
                    Interface::MessageBox(graphics, scene, trigger.EncounterMessage, Color::Active);
                }

                trigger.Activated = true;
            }

            if (!trigger.Completed)
            {
                if (!trigger.CompletedMessage.empty())
                {
                    Interface::MessageBox(graphics, scene, trigger.CompletedMessage, Color::Active);
                }

                trigger.Completed = true;
            }

            if (SafeCast(trigger.Variables.size()) > 0)
            {
                Game::Move(world, game, location, trigger.Variables[0]);
            }
            else
            {
                throw std::invalid_argument("NEXT LOCATION UNDEFINED!");
            }
        }
        else if (!trigger.Activated)
        {
            Interface::MessageBox(graphics, scene, trigger.EncounterMessage, Color::Active);

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

            // send status message
            if (trigger.Completed)
            {
                Interface::MessageBox(graphics, scene, trigger.CompletedMessage, Color::Active);
            }
            else
            {
                Interface::MessageBox(graphics, scene, trigger.ActiveMessage, Color::Inactive);
            }
        }
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

                auto loot_id = -1;

                auto opponent_id = -1;

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
}