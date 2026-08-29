#pragma once

#include "FieldOfView.hpp"
#include "Random.hpp"

namespace BloodSwordRogue::Room
{
    const int None = -1;

    // room base class
    class Base
    {
    public:
        int X1 = -1;

        int Y1 = -1;

        int X2 = -1;

        int Y2 = -1;

        int Width = -1;

        int Height = -1;

        // return room's center coordinates
        Point Center()
        {
            auto x = this->X1 + this->Width / 2;

            auto y = this->Y1 + this->Height / 2;

            return Point(x, y);
        }

        // return room's inner area boundaries
        std::pair<Point, Point> Inner()
        {
            return {Point(this->X1 + 1, this->Y1 + 1), Point(this->X2 - 1, this->Y2 - 1)};
        }

        Base(int x, int y, int width, int height) : X1(x), Y1(y), Width(width), Height(height)
        {
            this->X2 = this->X1 + this->Width;

            this->Y2 = this->Y1 + this->Height;
        }

        // check if point is inside the room
        bool Inside(Point point)
        {
            return ((point.X > this->X1 && point.X < this->X2 - 1) && (point.Y > this->Y1 && point.Y < this->Y2 - 1));
        }

        // check if a room intersects another room
        bool Intersects(const Room::Base &room)
        {
            return (this->X1 <= room.X2 && this->X2 >= room.X1 && this->Y1 <= room.Y2 && this->Y2 >= room.Y1);
        }

        // check if room intersects any of the other rooms
        bool Intersects(const std::vector<Room::Base> &rooms)
        {
            auto result = false;

            for (auto &room : rooms)
            {
                result |= this->Intersects(room);

                if (result)
                {
                    break;
                }
            }

            return result;
        }

        bool Intersects(Point point, int width, int height, int gap = 0)
        {
            return (this->X1 <= (width + gap) && this->X2 >= (point.X - gap) && this->Y1 <= (height + gap) && this->Y2 >= (point.Y - gap));
        }
    };

    // check if point is inside any of the other rooms
    bool Inside(std::vector<Room::Base> &rooms, Room::Base &room, Point point)
    {
        auto result = room.Inside(point);

        for (auto &room : rooms)
        {
            result |= room.Inside(point);
        }

        return result;
    }
}

namespace BloodSwordRogue::Battlepits
{
    // create vertical tunnel between two points and add to list
    void VerticalTunnel(Points &points, int x, int y1, int y2)
    {
        auto delta = 0;

        if (y1 < y2)
        {
            delta = 1;
        }
        else if (y1 > y2)
        {
            delta = -1;
        }

        for (auto y = y1; y != y2; y += delta)
        {
            points.push_back(Point(x, y));
        }
    }

    // create horizontal tunnel between two points and add to list
    void HorizontalTunnel(Points &points, int x1, int x2, int y)
    {
        auto delta = 0;

        if (x1 < x2)
        {
            delta = 1;
        }
        else if (x1 > x2)
        {
            delta = -1;
        }

        for (auto x = x1; x != x2; x += delta)
        {
            points.push_back(Point(x, y));
        }
    }

    // create tunnels between two points
    Points CreateTunnels(Point start, Point end)
    {
        auto random = Random::Base();

        auto points = Points();

        if (random.NextDouble() < 0.5)
        {
            Battlepits::VerticalTunnel(points, start.X, start.Y, end.Y);

            Battlepits::HorizontalTunnel(points, start.X, end.X, end.Y);
        }
        else
        {
            Battlepits::HorizontalTunnel(points, start.X, end.X, start.Y);

            Battlepits::VerticalTunnel(points, end.X, start.Y, end.Y);
        }

        return points;
    }

    // randomly place room in the map
    void Place(Map::Base &map, Room::Base &room, int room_id)
    {
        // create walls around the room
        for (auto y = room.Y1; y < room.Y2; y++)
        {
            for (auto x = room.X1; x < room.X2; x++)
            {
                auto point = Point(x, y);

                if (room.Inside(point))
                {
                    // carve out inner room area
                    map[point].Type = Map::Object::PASSABLE;

                    map[point].Asset = Asset::NONE;
                }
                else if (map[point].Type != Map::Object::PASSABLE)
                {
                    // do not overwrite tunnels
                    map[point].Type = Map::Object::OBSTACLE;

                    map[point].Asset = Asset::Map("WALL");
                }
            }
        }
    }

    // clears the entire map
    void ClearMap(BloodSwordRogue::Map::Base &map, Asset::Type asset)
    {
        for (auto y = 0; y < map.Height; y++)
        {
            for (auto x = 0; x < map.Width; x++)
            {
                auto point = Point(x, y);

                map[point].Type = Map::Object::NONE;

                map[point].Occupant = Map::Object::NONE;

                map[point].Explored = false;

                map[point].Asset = asset;

                map[point].TemporaryAsset = Asset::NONE;

                map[point].Lifetime = Map::Unlimited;

                map[point].Id = Map::NotFound;
            }
        }
    }

    void ClearNone(BloodSwordRogue::Map::Base &map, Asset::Type asset = Asset::NONE)
    {
        for (auto y = 0; y < map.Height; y++)
        {
            for (auto x = 0; x < map.Width; x++)
            {
                auto point = Point(x, y);

                if (map[point].Type == Map::Object::NONE)
                {
                    map[point].Type = Map::Object::PASSABLE;

                    map[point].Asset = asset;
                }
            }
        }
    }

    // connect rooms with a tunnel
    void ConnectRooms(Map::Base &map, std::vector<Room::Base> &rooms, Room::Base &room, Room::Base &last)
    {
        auto tunnel = Battlepits::CreateTunnels(room.Center(), last.Center());

        for (auto &point : tunnel)
        {
            if (!Room::Inside(rooms, room, point))
            {
                // lay out tunnel but stop when it encounters //  an existing
                if (map[point].Type != Map::Object::PASSABLE)
                {
                    map[point].Type = Map::Object::PASSABLE;

                    map[point].Asset = Asset::NONE;
                }
                else
                {
                    break;
                }
            }
        }
    }

    // check if room placement is valid
    Points Available(Map::Base &map, std::vector<Room::Base> &rooms, int width, int height, int gap_size)
    {
        auto available = Points();

        for (auto y = 0; y <= map.Height - height; y++)
        {
            for (auto x = 0; x <= map.Width - width; x++)
            {
                auto point = Point(x, y);

                auto w = x + width;

                auto h = y + height;

                auto place = true;

                for (auto &room : rooms)
                {
                    place &= !room.Intersects(point, w, h, 0);

                    if (!place)
                    {
                        break;
                    }
                }

                if (place)
                {
                    available.push_back(point);
                }
            }
        }

        return available;
    }

    // create room with random dimensions
    Room::Base CreateRoom(Random::Base &random, Map::Base &map, Points &available, int width, int height)
    {
        auto point = available[random.NextInt(0, SafeCast(available.size()) - 1)];

        return Room::Base(point.X, point.Y, width, height);
    }

    // checker function type
    typedef bool (*Checker)(Map::Base &map, Point point);

    // check if map tile is empty
    bool Empty(Map::Base &map, Point point)
    {
        return (map[point].Type == Map::Object::NONE);
    }

    // check if map tile is blocked
    bool Blocked(Map::Base &map, Point point)
    {
        auto &tile = map[point];

        return tile.Type == Map::Object::NONE || (tile.Type != Map::Object::PASSABLE && tile.Type != Map::Object::ENEMY_PASSABLE);
    }

    // check if map tile is passable
    bool Passable(Map::Base &map, Point point)
    {
        auto &tile = map[point];

        return tile.Type == Map::Object::PASSABLE;
    }

    // assign walls to all tunnel tiles
    void SetTunnelWalls(Map::Base &map)
    {
        for (auto y = 1; y < map.Height - 1; y++)
        {
            for (auto x = 0; x < map.Width - 1; x++)
            {
                auto &tile = map[Point(x, y)];

                if (tile.Type == Map::Object::NONE)
                {
                    auto top = Passable(map, {x, y - 1});

                    auto left = Passable(map, {x - 1, y});

                    auto right = Passable(map, {x + 1, y});

                    auto bottom = Passable(map, {x, y + 1});

                    auto top_left = Passable(map, {x - 1, y - 1});

                    auto top_right = Passable(map, {x + 1, y - 1});

                    auto bottom_left = Passable(map, {x - 1, y + 1});

                    auto bottom_right = Passable(map, {x + 1, y + 1});

                    if (top || left || right || bottom || top_right || top_left || bottom_left || bottom_right)
                    {
                        tile.Asset = Asset::Map("WALL");

                        tile.Type = Map::Object::OBSTACLE;
                    }
                }
            }
        }
    }

    // generate battlepits map
    void Generate(Map::Base &map, std::vector<Room::Base> &rooms, int max_rooms, int min_size, int max_size, bool inner_tunnel, int gap_size)
    {
        // initialize RNG
        auto random = Random::Base();

        // clear rooms
        rooms.clear();

        // clear entire map
        Battlepits::ClearMap(map, Asset::NONE);

        for (auto r = 0; r < max_rooms; r++)
        {
            // ensure odd dimensions so midpoint is always at the exact center of the room
            auto width = random.NextInt(min_size, max_size) * 2 + 1;

            auto height = random.NextInt(min_size, max_size) * 2 + 1;

            if (r == 0)
            {
                // place starting room at the center
                auto room = Room::Base((map.Width - width) / 2, (map.Height - height) / 2, width, height);

                Battlepits::Place(map, room, 0);

                rooms.push_back(room);
            }
            else
            {
                auto available = Battlepits::Available(map, rooms, width, height, gap_size);

                std::shuffle(available.begin(), available.end(), random.Generator());

                if (SafeCast(available.size()) > 0)
                {
                    auto room = Battlepits::CreateRoom(random, map, available, width, height);

                    // place room
                    Battlepits::Place(map, room, SafeCast(rooms.size()));

                    rooms.push_back(room);
                }
            }
        }

        // connect rooms
        for (auto room_id = 1; room_id < SafeCast(rooms.size()); room_id++)
        {
            Battlepits::ConnectRooms(map, rooms, rooms[room_id], rooms[room_id - 1]);
        }

        Battlepits::SetTunnelWalls(map);

        Battlepits::ClearNone(map, Asset::NONE);
    }

    // generate battlepits map
    Map::Base Generate(int width, int height, int max_rooms, int min_size, int max_size, bool inner_tunnel, int gap_size)
    {
        auto map = Map::Base(width, height);

        auto rooms = std::vector<Room::Base>();

        Battlepits::Generate(map, rooms, max_rooms, min_size, max_size, inner_tunnel, gap_size);

        return map;
    }
}
