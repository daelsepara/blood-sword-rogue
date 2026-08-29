#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_mouse.h>
#include <SDL_ttf.h>

#include "nlohmann/json.hpp"

#include "Constants.hpp"
#include "Files.hpp"
#include "ZipFileLibrary.hpp"

// primitive classes, functions, and constants used throughout the program
namespace BloodSwordRogue
{
    // convert to int
    template <typename T>
    int SafeCast(T size)
    {
        return static_cast<int>(size);
    }

    // cartesian coordinates
    class Point
    {
    public:
        // x, y
        int X = -1;

        int Y = -1;

        Point(int x, int y) : X(x), Y(y) {}

        Point() {}

        Point &operator*=(const Point &p)
        {
            this->X *= p.X;

            this->Y *= p.Y;

            return *this;
        }

        Point &operator+=(const Point &p)
        {
            this->X += p.X;

            this->Y += p.Y;

            return *this;
        }

        Point &operator-=(const Point &p)
        {
            this->X -= p.X;

            this->Y -= p.Y;

            return *this;
        }

        bool operator==(const Point &p)
        {
            return this->X == p.X && this->Y == p.Y;
        }

        bool operator!=(const Point &p)
        {
            return !(*this == p);
        }

        Point operator+(const Point &p)
        {
            return Point(this->X + p.X, this->Y + p.Y);
        }

        Point operator-(const Point &p)
        {
            return Point(this->X - p.X, this->Y - p.Y);
        }

        Point operator*(const Point &p)
        {
            return Point(this->X * p.X, this->Y * p.Y);
        }

        Point operator+(int p)
        {
            return Point(this->X + p, this->Y + p);
        }

        Point operator-(int p)
        {
            return *this + (-p);
        }

        Point operator*(int p)
        {
            return Point(this->X * p, this->Y * p);
        }

        Point operator/(const Point &p)
        {
            return Point(this->X / p.X, this->Y / p.Y);
        }

        Point operator/(int p)
        {
            return Point(this->X / p, this->Y / p);
        }

        // check if point is a valid map coordinate
        bool IsNone()
        {
            return *this == Point(-1, -1);
        }
    };

    // list of points
    typedef std::vector<Point> Points;

    // check if point is in list of points
    bool In(Points &points, Point point)
    {
        auto result = false;

        for (auto &item : points)
        {
            if (item == point)
            {
                result = true;

                break;
            }
        }

        return result;
    }

    // check if point is in list of points
    bool In(Points &points, int x, int y)
    {
        return In(points, Point(x, y));
    }

    // load an image as an SDL surface
    SDL_Surface *Surface(const char *image)
    {
        auto surface = IMG_Load(image);

        if (!surface)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_Error: %s", image, SDL_GetError());
        }

        return surface;
    }

    // load an image in a zip file as an SDL surface
    SDL_Surface *Surface(const char *image, const char *zip_file)
    {
        // read file from zip archive
        auto asset = ZipFile::Read(zip_file, image);

        // create a modifiable buffer
        auto buffer = asset.data();

        // create surface from memory buffer
        auto rw = SDL_RWFromMem((void *)buffer, SafeCast(asset.size()));

        if (!rw)
        {
            return nullptr;
        }

        // create surface and close SDL_RWops
        auto surface = IMG_Load_RW(rw, 1);

        asset.clear();

        return surface;
    }

    // load an image in a zip file as an SDL texture
    SDL_Texture *Texture(SDL_Renderer *renderer, const char *image, const char *zip_file)
    {
        // read file from zip archive
        auto asset = ZipFile::Read(zip_file, image);

        // create a modifiable buffer
        auto buffer = asset.data();

        // create surface from memory buffer
        auto rw = SDL_RWFromMem((void *)buffer, SafeCast(asset.size()));

        if (!rw)
        {
            return nullptr;
        }

        // create surface and close SDL_RWops
        auto texture = IMG_LoadTexture_RW(renderer, rw, 1);

        asset.clear();

        return texture;
    }

    // free surface
    void Free(SDL_Surface **surface)
    {
        if (*surface != nullptr)
        {
            SDL_FreeSurface(*surface);

            *surface = nullptr;
        }
    }

    // free texture
    void Free(SDL_Texture **texture)
    {
        if (*texture != nullptr)
        {
            SDL_DestroyTexture(*texture);

            *texture = nullptr;
        }
    }

    // free textures
    void Free(std::vector<SDL_Texture *> &textures)
    {
        if (!textures.empty())
        {
            for (auto &texture : textures)
            {
                Free(&texture);
            }

            textures.clear();
        }
    }

    // free resources in a map
    template <typename T, typename R>
    void Free(std::unordered_map<T, R> &resources)
    {
        if (!resources.empty())
        {
            for (auto &resource : resources)
            {
                Free(&resource.second);
            }

            resources.clear();
        }
    }

    // left pad string
    std::string LeftPad(std::string &str, int n)
    {
        if (SafeCast(str.size()) < n)
        {
            std::ostringstream oss;

            oss << std::setw(n) << std::left << str;

            return oss.str();
        }
        else
        {
            return str;
        }
    }

    // sanitize string by removing specific characters
    std::string CleanString(std::string text, const char *chars)
    {
        for (auto i = 0; i < strlen(chars); ++i)
        {
            text.erase(std::remove(text.begin(), text.end(), chars[i]), text.end());
        }

        return text;
    }

    // get size of texture
    Point Size(SDL_Texture *texture)
    {
        auto size = Point(0, 0);

        if (texture)
        {
            SDL_QueryTexture(texture, nullptr, nullptr, &size.X, &size.Y);
        }

        return size;
    }

    // get size of texture
    void Size(SDL_Texture *texture, int *texture_w, int *texture_h)
    {
        if (texture)
        {
            SDL_QueryTexture(texture, nullptr, nullptr, texture_w, texture_h);
        }
    }

    // get width of texture
    int Width(SDL_Texture *texture)
    {
        auto width = 0;

        Size(texture, &width, nullptr);

        return width;
    }

    // get max width of list of textures
    int Width(std::vector<SDL_Texture *> &textures)
    {
        auto width = 0;

        for (auto i = 0; i < SafeCast(textures.size()); i++)
        {
            width = std::max(width, Width(textures[i]));
        }

        return width;
    }

    // get height of texture
    int Height(SDL_Texture *texture)
    {
        auto height = 0;

        Size(texture, nullptr, &height);

        return height;
    }

    // get max height of list of textures
    int Height(std::vector<SDL_Texture *> &textures)
    {
        auto height = 0;

        for (auto i = 0; i < SafeCast(textures.size()); i++)
        {
            height = std::max(height, Height(textures[i]));
        }

        return height;
    }

    // reads entire file into string (for use with nlohmann json)
    std::string Read(const char *source)
    {
        std::ostringstream ss;

        std::ifstream input_file(source);

        auto file_content = std::string();

        if (input_file.good())
        {
            // read the file into the string stream
            ss << input_file.rdbuf();

            // convert to string
            file_content = ss.str();

            input_file.close();
        }

        return file_content;
    }
}
