#pragma once

#include "Templates.hpp"

namespace BloodSwordRogue::Asset
{
    // asset missing or invalid
    int NONE = -1;

    // asset type id
    typedef int Type;

    // mapping of asset type ids to other types
    template <typename T>
    using Mapping = UnorderedMap<Asset::Type, T>;

    // lookup table for mapping T to asset type id
    template <typename T>
    using Lookup = UnorderedMap<T, Asset::Type>;

    // mapping of T to SDL sextures
    template <typename T>
    using TextureMap = UnorderedMap<T, SDL_Texture *>;

    // list of textures
    typedef std::vector<SDL_Texture *> TextureList;

    // asset list
    typedef std::vector<Asset::Type> List;

    // asset type to string mapping
    Asset::Mapping<std::string> TypeMapping = {};

    // get asset type id
    Asset::Type Map(std::string asset)
    {
        return BloodSwordRogue::Find(Asset::TypeMapping, asset, Asset::NONE);
    }

    // get asset type id
    Asset::Type Map(const char *asset)
    {
        return Asset::Map(std::string(asset));
    }

    // generate T -> asset type id mapping
    template <typename T>
    void MapTypes(Asset::Lookup<T> &types, ConstStrings<T> &assets)
    {
        types.clear();

        for (auto &asset : assets)
        {
            types[asset.first] = Asset::Map(std::string(asset.second));
        }
    }

    // generate T -> asset type id mapping
    template <typename T>
    void MapTypes(Asset::Lookup<T> &types, StringMap<T> &assets)
    {
        types.clear();

        for (auto &asset : assets)
        {
            types[asset.first] = Asset::Map(asset.second);
        }
    }

    // generate vector of asset type ids, must be a vector of type T
    void MapTypes(Asset::List &types, std::vector<const char *> &assets)
    {
        types.clear();

        for (auto &asset : assets)
        {
            auto asset_type = Asset::Map(asset);

            types.push_back(asset_type);
        }
    }
}
