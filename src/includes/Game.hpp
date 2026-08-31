#pragma once

#include "Evaluate.hpp"
#include "Models.hpp"

namespace BloodSwordRogue::Game
{
    class World
    {
    public:
        std::string ZipFile = std::string();

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

    void Save(Game::Base &game, Location::Base location)
    {
        game.Locations.insert_or_assign(location.Name, location);
    }

    void Save(Game::Base &game, Party::Base party)
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

    // move to a new location
    void Move(Game::World &world, Game::Base &game, Location::Base &location, Party::Base &party, std::string next)
    {
        auto loaded = false;

        // check if area has been visited before
        if (Game::HasLocation(game, next))
        {
            // copy updates
            Game::Save(game, location);

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
            party.Location = std::string(next);

            // move party to origin
            if (SafeCast(location.Map.Origins.size()) > 0)
            {
                party.X = location.Map.Origins[0].X;

                party.Y = location.Map.Origins[0].Y;

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

    Game::World CurrentWorld = Game::World();

    void CheckTrigger(Graphics::Base &graphics, Scene::Base &scene, Location::Base &location, Party::Base &party, Trigger::Base &trigger)
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
                Game::Move(CurrentWorld, CurrentGame, location, party, trigger.Variables[0]);
            }
            else
            {
                throw std::invalid_argument("NEXT LOCATION UNDEFIND!");
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
                trigger.Completed = Evaluate::InParty(trigger, party);
            }
            else if (trigger.Type == Trigger::Type::ITEM)
            {
                trigger.Completed = Evaluate::HasItem(trigger, party);
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

    // check if there is loot at this location
    int FindLoot(Location::Base &location, Point point)
    {
        auto &loot = location.Loot;

        auto found = -1;

        for (auto id = 0; id < SafeCast(loot.size()); id++)
        {
            if (loot[id].Location() == point)
            {
                found = id;

                break;
            }
        }

        return found;
    }

    // check if there is an opponent party at this location
    int FindOpponents(Location::Base &location, Point point)
    {
        auto &opponents = location.Opponents;

        auto found = -1;

        for (auto id = 0; id < SafeCast(opponents.size()); id++)
        {
            if (opponents[id].Origin() == point)
            {
                found = id;

                break;
            }
        }

        return found;
    }

    // check if there is a trigger at this location
    int FindTrigger(Location::Base &location, Point point)
    {
        auto found = -1;

        auto &triggers = location.Triggers;

        for (auto id = 0; id < SafeCast(triggers.size()); id++)
        {
            if (triggers[id].Location() == point)
            {
                found = id;

                break;
            }
        }

        return found;
    }

    // render battlepits
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

                            opponent_id = Game::FindOpponents(location, Point(x, y));

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

                            loot_id = Game::FindLoot(location, Point(x, y));

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