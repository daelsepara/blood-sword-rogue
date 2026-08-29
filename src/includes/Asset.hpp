#pragma once

#include "AssetTypes.hpp"
#include "Color.hpp"
#include "Primitives.hpp"

//====================================================================
// GRAPHICS: TEXTURE ASSETS
//====================================================================
namespace BloodSwordRogue::Asset
{
    // mapping of assets to their relative location
    Asset::Mapping<std::string> Locations = {};

    // mapping of asset types their respective loaded texture
    Asset::TextureMap<Asset::Type> Textures = {};

    // create texture from a file
    SDL_Texture *Create(SDL_Renderer *renderer, const char *path)
    {
        auto texture = IMG_LoadTexture(renderer, path);

        if (texture)
        {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

            SDL_SetTextureColorMod(texture, Color::R(Color::Active), Color::G(Color::Active), Color::B(Color::Active));
        }

        return texture;
    }

    // create texture from a file in zip archive
    SDL_Texture *Create(SDL_Renderer *renderer, const char *zip_file, const char *path)
    {
        auto texture = Texture(renderer, path, zip_file);

        if (texture)
        {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

            SDL_SetTextureColorMod(texture, Color::R(Color::Active), Color::G(Color::Active), Color::B(Color::Active));
        }

        return texture;
    }

    // re-create surface from asset and color adjust
    SDL_Surface *GetSurface(Asset::Type asset, Uint32 blur)
    {
        SDL_Surface *surface = nullptr;

        if (asset != Asset::NONE)
        {
            surface = Surface(Asset::Locations[asset].c_str());

            if (surface)
            {
                SDL_SetSurfaceColorMod(surface, Color::R(blur), Color::G(blur), Color::B(blur));
            }
        }

        return surface;
    }

    // reloads asset as an SDL surface and adjust color
    SDL_Surface *GetSurface(std::string asset, Uint32 blur)
    {
        auto asset_type = Asset::Map(asset);

        return Asset::GetSurface(asset_type, blur);
    }

    // reloads asset as an SDL surface and apply current palette
    SDL_Surface *GetSurface(Asset::Type asset)
    {
        return Asset::GetSurface(asset, Color::Active);
    }

    // reloads asset as an SDL surface and apply current palette
    SDL_Surface *GetSurface(std::string asset)
    {
        auto asset_type = Asset::Map(asset);

        return Asset::GetSurface(asset_type);
    }

    // unload all assets
    void Unload()
    {
        if (SafeCast(Asset::Textures.size()) > 0)
        {
            for (auto &texture : Asset::Textures)
            {
                Free(&texture.second);
            }

            Asset::Textures.clear();

            Asset::Locations.clear();
        }
    }

    // clear all lists
    void Clear()
    {
        Asset::Locations.clear();

        Asset::Textures.clear();

        Asset::TypeMapping.clear();
    }

    bool Generate(std::string &json_file, const char *zip_file, SDL_Renderer *renderer)
    {
        if (json_file.empty())
        {
            return false;
        }

        auto data = nlohmann::json::parse(json_file);

        auto is_zip = (zip_file != nullptr);

        if (!data["assets"].is_null() && data["assets"].is_array() && SafeCast(data["assets"].size()) > 0)
        {
            auto asset_type = 0;

            for (auto i = 0; i < SafeCast(data["assets"].size()); i++)
            {
                auto object = !data["assets"][i]["id"].is_null() ? std::string(data["assets"][i]["id"]) : std::string();

                auto path = !data["assets"][i]["path"].is_null() ? std::string(data["assets"][i]["path"]) : "";

                if (!path.empty() && !object.empty())
                {
                    if (renderer)
                    {
                        auto texture = is_zip ? Asset::Create(renderer, zip_file, path.c_str()) : Asset::Create(renderer, path.c_str());

                        if (texture)
                        {
                            Asset::Textures[asset_type] = texture;
                        }
                    }

                    // update asset location
                    Asset::Locations[asset_type] = std::string(path);

                    // update type mapping
                    Asset::TypeMapping[asset_type] = std::string(object);

                    asset_type++;
                }
            }
        }

        if (renderer)
        {
            return (!Asset::Locations.empty() && !Asset::Textures.empty() && (SafeCast(Asset::Textures.size()) == Asset::Locations.size()));
        }
        else
        {
            return !Locations.empty();
        }
    }

    // unloads all assets
    void FreeTextures()
    {
        Asset::Unload();

        Asset::Clear();
    }

    // load assets from a zip archive
    bool Load(std::string json_file, const char *zip_file, SDL_Renderer *renderer)
    {
        Asset::FreeTextures();

        return Generate(json_file, zip_file, renderer);
    }

    // load all assets and create textures
    bool Load(SDL_Renderer *renderer, const char *assets)
    {
        auto result = false;

        auto json_file = Read(assets);

        if (!json_file.empty())
        {
            result = Asset::Load(json_file, nullptr, renderer);
        }

#ifdef DEBUG
        if (result)
        {
            SDL_Log("[LOADED] %d Assets", SafeCast(Asset::Textures.size()));
        }
#endif

        return result;
    }

    // load assets and create textures
    bool Load(SDL_Renderer *renderer, std::string assets)
    {
        return Asset::Load(renderer, assets.c_str());
    }

    // load all assets from zip file and create textures
    bool Load(SDL_Renderer *renderer, const char *assets, const char *zip_file)
    {
        auto result = false;

        if (zip_file == nullptr)
        {
            result = Asset::Load(renderer, assets);
        }
        else
        {
            auto json_file = ZipFile::Read(zip_file, assets);

            if (!json_file.empty())
            {
                result = Asset::Load(json_file, zip_file, renderer);
            }
        }

        return result;
    }

    // load all assets from zip file and create textures
    bool Load(SDL_Renderer *renderer, std::string assets, std::string zip_file)
    {
        return Asset::Load(renderer, assets.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    // load asset locations
    bool Load(const char *assets)
    {
        auto result = false;

        auto json_file = Read(assets);

        if (!json_file.empty())
        {
            result = Asset::Generate(json_file, nullptr, nullptr);
        }

        return result;
    }

    // get texture associated with the asset type
    SDL_Texture *Get(Asset::Type asset)
    {
        SDL_Texture *texture = nullptr;

        if (Has(Asset::Textures, asset))
        {
            texture = Asset::Textures[asset];
        }

        return texture;
    }

    // get texture associated with the asset type string
    SDL_Texture *Get(const char *asset)
    {
        return Asset::Get(Asset::Map(asset));
    }

    // get texture associated with the asset type string
    SDL_Texture *Get(std::string asset)
    {
        return Asset::Get(Asset::Map(asset));
    }

    // get texture associated with the asset type and modulate the color
    SDL_Texture *Get(Asset::Type asset, Uint8 alpha)
    {
        auto texture = Asset::Get(asset);

        if (texture)
        {
            SDL_SetTextureColorMod(texture, alpha, alpha, alpha);
        }

        return texture;
    }

    // get texture associated with the asset type and modulate the color
    SDL_Texture *Get(Asset::Type asset, Uint32 color)
    {
        auto texture = Asset::Get(asset);

        if (texture)
        {
            SDL_SetTextureColorMod(texture, Color::R(color), Color::G(color), Color::B(color));
        }

        return texture;
    }

    // create a copy of the asset. must be de-allocated manually
    SDL_Texture *Copy(SDL_Renderer *renderer, Asset::Type asset)
    {
        SDL_Texture *texture = nullptr;

        if (Has(Asset::Locations, asset))
        {
            texture = Asset::Create(renderer, Asset::Locations[asset].c_str());
        }

        return texture;
    }

    // copy the texture associated with the asset and modulate the color. must be
    // de-allocated manually
    SDL_Texture *Copy(SDL_Renderer *renderer, Asset::Type asset, Uint32 color)
    {
        auto texture = Asset::Copy(renderer, asset);

        if (texture)
        {
            SDL_SetTextureColorMod(texture, Color::R(color), Color::G(color), Color::B(color));
        }

        return texture;
    }
}
