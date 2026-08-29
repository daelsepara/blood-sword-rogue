#pragma once

#include "Primitives.hpp"
#include "Templates.hpp"

// simple implementation of a font (glyph) cache
namespace BloodSwordRogue::FontCache
{
    class Glyph
    {
    public:
        SDL_Texture *Texture = nullptr;

        int Width = -1;

        int Height = -1;

        Glyph(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color text_color, int style)
        {
            // set style
            TTF_SetFontStyle(font, style);

            auto surface = TTF_RenderUTF8_Blended(font, text, text_color);

            if (surface)
            {
                // create texture
                this->Texture = SDL_CreateTextureFromSurface(renderer, surface);

                if (this->Texture)
                {
                    // get texture dimensions
                    BloodSwordRogue::Size(this->Texture, &this->Width, &this->Height);
                }

                BloodSwordRogue::Free(&surface);
            }
        }

        Glyph() {}

        void Free()
        {
            if (this->Texture)
            {
                BloodSwordRogue::Free(&this->Texture);
            }
        }
    };

    FontCache::Glyph Null = FontCache::Glyph();

    class Base
    {
    public:
        BloodSwordRogue::UnorderedMap<std::string, FontCache::Glyph> Glyphs = {};

        Base() {}

        bool Has(std::string &text)
        {
            return BloodSwordRogue::Has(this->Glyphs, text);
        }

        FontCache::Glyph &operator[](std::string chr)
        {
            return this->Has(chr) ? this->Glyphs[chr] : FontCache::Null;
        }

        SDL_Texture *Texture(std::string chr)
        {
            return this->Has(chr) ? this->Glyphs[chr].Texture : nullptr;
        }

        void Free()
        {
            for (auto &item : this->Glyphs)
            {
                auto glyph = item.second;

                glyph.Free();
            }

            this->Glyphs.clear();
        }

        void CreateGlyph(SDL_Renderer *renderer, TTF_Font *font, std::string &text, SDL_Color text_color, int style)
        {
            // ensure that there are no duplicates
            if (!this->Has(text))
            {
                this->Glyphs[text] = FontCache::Glyph(renderer, font, text.c_str(), text_color, style);
            }
        }

        // add character to cache
        void Add(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color text_color, int style)
        {
            for (auto c = 0; c < std::strlen(text); c++)
            {
                auto chr = std::string(1, text[c]);

                this->CreateGlyph(renderer, font, chr, text_color, style);
            }
        }

        // create texture cache of individual characters
        void Create(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color text_color, int style)
        {
            this->Free();

            this->Add(renderer, font, text, text_color, style);
        }

        // add words to cache
        void Add(SDL_Renderer *renderer, TTF_Font *font, std::vector<std::string> &collection, SDL_Color text_color, int style)
        {
            for (auto &text : collection)
            {
                this->CreateGlyph(renderer, font, text, text_color, style);
            }
        }

        // create cache of words
        void Create(SDL_Renderer *renderer, TTF_Font *font, std::vector<std::string> &collection, SDL_Color text_color, int style)
        {
            this->Free();

            this->Add(renderer, font, collection, text_color, style);
        }
    };
}
