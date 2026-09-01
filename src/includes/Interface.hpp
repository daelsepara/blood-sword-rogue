#pragma once

#include "Input.hpp"
#include "Party.hpp"

namespace BloodSwordRogue::Interface
{
    //====================================================================
    // MODULE BASE CLASS AND GLOBALS
    //====================================================================

    // struct to represent a game module
    class Module
    {
    public:
        // unique id of module
        std::string Id = std::string();

        // title of module
        std::string Title = std::string();

        // settings file
        std::string SettingsFile = std::string();

        // zip archive
        std::string ZipFile = std::string();

        // module is in a zipped archive
        bool Zipped = false;
    };

    // list of available modules
    std::vector<Module> Modules = {};

    //====================================================================
    // SETTINGS GLOBALS
    //====================================================================

    // path to settings file
    std::string SettingsFile = std::string();

    // zip archive
    std::string ZipFile = std::string();

    // module is a zipped archive
    bool Zipped = false;

    // game settings (json object)
    nlohmann::json Settings;

    //====================================================================
    // CLASS ASSET MAPPINGS
    //====================================================================

    // character class to asset type mapping
    Asset::Lookup<Character::Class> ClassAssets = {};

    // character class to asset name mapping
    BloodSwordRogue::ConstStrings<Character::Class> ClassAssetsNames = {
        {Character::Class::WARRIOR, "WARRIOR"},
        {Character::Class::TRICKSTER, "TRICKSTER"},
        {Character::Class::SAGE, "SAGE"},
        {Character::Class::ENCHANTER, "ENCHANTER"}};

    //====================================================================
    // NUMBERS
    //====================================================================

    // number assets ids
    Asset::List Numbers = {};

    // number asset names
    std::vector<const char *> NumbersNames = {
        "ZERO",
        "ONE",
        "TWO",
        "THREE",
        "FOUR",
        "FIVE",
        "SIX",
        "SEVEN",
        "EIGHT",
        "NINE"};

    //====================================================================
    // DICE
    //====================================================================

    // dice asset ids
    Asset::List Dice = {};

    // dice asset names
    std::vector<const char *> DiceAssets = {
        "DICE1",
        "DICE2",
        "DICE3",
        "DICE4",
        "DICE5",
        "DICE6"};

    // dice textures
    Asset::TextureList DiceTextures = {};

    void InitializeDice()
    {
        // initialize dice asset ids
        Asset::MapTypes(Interface::Dice, Interface::DiceAssets);

        // clear texture list
        Interface::DiceTextures = Asset::TextureList(SafeCast(Interface::Dice.size()));

        // generate texture list
        for (auto dice = 0; dice < SafeCast(Interface::Dice.size()); dice++)
        {
            Interface::DiceTextures[dice] = Asset::Get(Interface::Dice[dice]);
        }
    }

    //====================================================================
    // UNLOAD TEXTURES
    //====================================================================

    // unload all textures and assets
    void UnloadTextures()
    {
        Asset::Unload();
    }

    // unload sound, fonts, texture assets
    void UnloadAssets()
    {
        // unload fonts
        Fonts::Free();

        // unload all textures
        Interface::UnloadTextures();

        // close all gamepads
        Input::CloseGamePads();
    }

    //====================================================================
    // LOAD TEXTURES
    //====================================================================

    // load all textures
    void LoadTextures(Graphics::Base &graphics)
    {
        // load all assets, initialize asset type ids
        if (Zipped)
        {
            Asset::Load(graphics.Renderer, Settings["assets"], Interface::ZipFile);
        }
        else
        {
            Asset::Load(graphics.Renderer, Settings["assets"]);
        }

        // initialize attribute to asset mapping
        Attribute::MapAssets();

        // initialize skill to asset mapping
        Skills::MapAssets();

        // initialize spell to asset mapping
        Spells::MapAssets();

        // initialize dice assets
        Interface::InitializeDice();

        // initialize number assets
        Asset::MapTypes(Interface::Numbers, Interface::NumbersNames);

        // initialize character class to asset mapping
        Asset::MapTypes(Interface::ClassAssets, Interface::ClassAssetsNames);
    }

    // switch texture and reload all textures
    void ReloadTextures(Graphics::Base &graphics)
    {
        // unload all textures
        Interface::UnloadTextures();

        // re-load textures
        Interface::LoadTextures(graphics);
    }

    //====================================================================
    // INITIALIZE SETTINGS
    //====================================================================

    // initialize settings from json
    void Initialize(nlohmann::json &data)
    {
        if (!data["settings"].is_null() && data["settings"].is_object() && SafeCast(data["settings"].size()) > 0)
        {
            Interface::Settings = data["settings"];
        }
    }

    // initialize settings from file or zip file
    void Initialize(const char *settings, const char *zip_file)
    {
        auto ifs = zip_file != nullptr ? ZipFile::Read(zip_file, settings) : Read(settings);

        if (!ifs.empty())
        {
            auto data = nlohmann::json::parse(ifs);

            Interface::Initialize(data);

            ifs.clear();
        }
    }

    //====================================================================
    // LOAD SETTINGS
    //====================================================================

    // load settings from file
    void LoadSettings(Graphics::Base &graphics, std::string settings_file, std::string zip_file)
    {
        // create directories
        Files::CreateDirectories();

        // initialize settings
        Interface::Initialize(settings_file.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());

        // initialize gamepads
        Input::InitializeGamePads();

        // load textures
        Interface::LoadTextures(graphics);

        // load control types
        Controls::Load(Interface::Settings["controls"], zip_file);

        // load fonts
        Fonts::Load(Interface::Settings["fonts"], zip_file);

        // load targets
        Target::Load(Interface::Settings["targets"], zip_file);

        // load item properties
        Item::Load(Interface::Settings["item-properties"], zip_file);

        // load skills
        Skills::Load(Interface::Settings["skills"], zip_file);

        // load item defaults
        Items::Load(Interface::Settings["items"], zip_file);
    }

    void LoadSettings(Graphics::Base &graphics, std::string settings_file)
    {
        LoadSettings(graphics, settings_file, std::string());
    }

    //====================================================================
    // RELOAD SETTINGS
    //====================================================================

    // reload settings
    void ReloadSettings(Graphics::Base &graphics, std::string settings_file, std::string zip_file)
    {
        // unload all assets (texture, sound, fonts)
        Interface::UnloadAssets();

        // reload all setings
        Interface::LoadSettings(graphics, settings_file, zip_file);
    }

    // reload settings
    void ReloadSettings(Graphics::Base &graphics, std::string settings_file)
    {
        Interface::ReloadSettings(graphics, settings_file, std::string());
    }

    //====================================================================
    // MESSAGE BOX ROUTINES
    //====================================================================

    // draws a message box on screen
    void MessageBox(Graphics::Base &graphics, Graphics::Scenery scenes, Point offset, int width, int height, SDL_Texture *message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        auto box = Scene::Base();

        auto pad = BloodSwordRogue::Pad * 2;

        if (message)
        {
            auto texture_w = 0;

            auto texture_h = 0;

            BloodSwordRogue::Size(message, &texture_w, &texture_h);

            auto box_w = std::max(256 - pad * 2, texture_w) + pad * 2;

            auto box_h = std::max(texture_h, BloodSwordRogue::TileSize * 2 - pad * 2) + pad * 2;

            auto location_box = offset + (Point(width, height) - Point(box_w, box_h)) / 2;

            auto location_txt = offset + (Point(width, height) - Point(texture_w, box_h)) / 2;

            auto confirm = location_box + Point(box_w / 2 - BloodSwordRogue::HalfTile, texture_h + pad * 2);

            auto input = Controls::User();

            box.Add(Scene::Element(location_box, box_w, box_h, background, border, border_size));

            box.VerifyAndAdd(Scene::Element(message, location_txt + Point(0, pad)));

            box.VerifyAndAdd(Scene::Element(Asset::Get("CONFIRM"), confirm));

            box.Add(Controls::Base(Controls::MapType("CONFIRM"), 0, 0, 0, 0, 0, confirm.X, confirm.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, highlight));

            while (true)
            {
                Graphics::Scenery scenery = scenes;

                scenery.push_back(box);

                input = Input::WaitForInput(graphics, scenery, box.Controls, input, true, blur);

                if (Input::Check(input))
                {
                    if (input.Type == Controls::MapType("CONFIRM"))
                    {
                        break;
                    }
                }
            }
        }
    }

    // draws a message box over a scene
    void MessageBox(Graphics::Base &graphics, Graphics::Scenery scenes, SDL_Texture *message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        Interface::MessageBox(graphics, scenes, Point(0, 0), graphics.Width, graphics.Height, message, background, border, border_size, highlight, blur);
    }

    // draws a message box over a scene
    void MessageBox(Graphics::Base &graphics, Graphics::Scenery scenes, Graphics::RichText message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        auto texture = Graphics::CreateText(graphics, message.Text.c_str(), message.Font, message.Color, message.Style);

        if (texture)
        {
            Interface::MessageBox(graphics, scenes, texture, background, border, border_size, highlight, blur);

            BloodSwordRogue::Free(&texture);
        }
    }

    // generic message box
    void MessageBox(Graphics::Base &graphics, Graphics::Scenery scenery, std::string message, Uint32 border)
    {
        auto texture = Graphics::CreateText(graphics, message.c_str(), Fonts::Normal, Color::S(Color::Active), TTF_STYLE_NORMAL);

        if (texture)
        {
            Interface::MessageBox(graphics, scenery, texture, Color::Background, border, BloodSwordRogue::Border, border == Color::Active ? Color::Highlight : Color::Active, true);

            BloodSwordRogue::Free(&texture);
        }
    }

    // generic message box
    void MessageBox(Graphics::Base &graphics, Scene::Base &scene, std::string message, Uint32 border)
    {
        Graphics::Scenery scenes = {scene};

        Interface::MessageBox(graphics, scenes, message, border);
    }

    //====================================================================
    // CONFIRMATION (YES/NO) BOX ROUTINES
    //====================================================================

    // draws a confirmation message box on screen
    bool Confirm(Graphics::Base &graphics, Graphics::Scenery scenes, Point offset, int width, int height, SDL_Texture *message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        auto result = false;

        auto box = Scene::Base();

        auto pad = 16;

        if (message)
        {
            auto texture_w = 0;

            auto texture_h = 0;

            BloodSwordRogue::Size(message, &texture_w, &texture_h);

            auto box_w = std::max(texture_w + pad * 2, BloodSwordRogue::Width(message) + ((BloodSwordRogue::TileSize + BloodSwordRogue::Pad) * 2));

            auto box_h = texture_h + pad * 3 + BloodSwordRogue::TileSize;

            auto location = offset + (Point(width, height) - Point(box_w, box_h)) / 2;

            auto message_x = offset.X + (width - texture_w) / 2;

            auto confirm = location + Point(box_w / 2 - BloodSwordRogue::TileSize - pad, texture_h + pad * 2);

            auto input = Controls::User();

            box.Add(Scene::Element(location, box_w, box_h, background, border, border_size));

            box.VerifyAndAdd(Scene::Element(message, Point(message_x, location.Y + pad)));

            box.VerifyAndAdd(Scene::Element(Asset::Get("CONFIRM"), confirm));

            box.VerifyAndAdd(Scene::Element(Asset::Get("CANCEL"), confirm + Point(TileSize + pad * 2, 0)));

            box.Add(Controls::Base(Controls::MapType("CONFIRM"), 0, 0, 1, 0, 0, confirm.X, confirm.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, highlight));

            box.Add(Controls::Base(Controls::MapType("CANCEL"), 1, 0, 1, 1, 1, confirm.X + BloodSwordRogue::TileSize + pad * 2, confirm.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, highlight));

            while (true)
            {
                Graphics::Scenery scenery = scenes;

                scenery.push_back(box);

                input = Input::WaitForInput(graphics, scenery, box.Controls, input, blur);

                if (Input::Check(input))
                {
                    if (input.Type == Controls::MapType("CONFIRM"))
                    {
                        result = true;

                        break;
                    }
                    else if (input.Type == Controls::MapType("CANCEL"))
                    {
                        result = false;

                        break;
                    }
                }
            }
        }

        return result;
    }

    // draws a confirm message box over a scene
    bool Confirm(Graphics::Base &graphics, Graphics::Scenery scenes, SDL_Texture *message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        return Interface::Confirm(graphics, scenes, Point(0, 0), graphics.Width, graphics.Height, message, background, border, border_size, highlight, blur);
    }

    // draws a confirm message box over a scene
    bool Confirm(Graphics::Base &graphics, Graphics::Scenery scenes, Graphics::RichText message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        auto result = false;

        auto texture = Graphics::CreateText(graphics, message.Text.c_str(), message.Font, message.Color, message.Style);

        if (texture)
        {
            result = Interface::Confirm(graphics, scenes, texture, background, border, border_size, highlight, blur);

            BloodSwordRogue::Free(&texture);
        }

        return result;
    }

    // show confirm dialog window
    bool Confirm(Graphics::Base &graphics, Graphics::Scenery scenes, std::string message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        return Interface::Confirm(graphics, scenes, Graphics::RichText(message.c_str(), Fonts::Normal, Color::Active, TTF_STYLE_NORMAL, 0), background, border, border_size, highlight, blur);
    }

    // show confirm dialog window
    bool Confirm(Graphics::Base &graphics, Scene::Base &scene, std::string message, Uint32 background, Uint32 border, int border_size, Uint32 highlight, bool blur = true)
    {
        Graphics::Scenery scenes = {scene};

        return Interface::Confirm(graphics, scenes, message, background, border, border_size, highlight, blur);
    }

    // adds text to scene using cache
    void AddText(Scene::Base &scene, FontCache::Base &cache, std::string &text, int x, int y)
    {
        if (cache.Has(text))
        {
            scene.VerifyAndAdd(Scene::Element(cache[text].Texture, Point(x, y)));
        }
        else
        {
            for (auto c = 0; c < SafeCast(text.size()); c++)
            {
                auto glyph = cache[std::string(1, text[c])];

                scene.VerifyAndAdd(Scene::Element(glyph.Texture, Point(x, y)));

                x += glyph.Width;
            }
        }
    }

    int IconList(Graphics::Base &graphics, Graphics::Scenery scenes, Asset::List &assets, std::vector<std::string> captions_text = {})
    {
        auto selected = -1;

        if (assets.empty())
        {
            return selected;
        }

        auto items = SafeCast(assets.size());

        auto width = (items + 1) * (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile) + BloodSwordRogue::HalfTile;

        auto height = BloodSwordRogue::TileSize * 2;

        auto box = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

        auto has_captions = SafeCast(captions_text.size()) > 0 && (captions_text.size() == assets.size());

        Asset::TextureList captions = has_captions ? Graphics::CreateText(graphics, Graphics::GenerateTextList(captions_text, Fonts::Caption, Color::Active, 0)) : Asset::TextureList();

        auto input = Controls::User();

        auto done = false;

        // pre-calculate possible caption position adjustments
        auto bx = box.X - BloodSwordRogue::Border;

        auto b2 = BloodSwordRogue::Border * 2;

        auto bw = box.X + width + BloodSwordRogue::Border;

        while (!done)
        {
            auto scene = Scene::Base();

            // icon list
            scene.Add(Scene::Element(box.X - BloodSwordRogue::Border, box.Y - BloodSwordRogue::Border, width + BloodSwordRogue::Border * 2, height + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            for (auto i = 0; i < items; i++)
            {
                auto point = Point(box.X + BloodSwordRogue::HalfTile + i * (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile), box.Y + BloodSwordRogue::HalfTile);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(assets[i]), point));

                auto left = i > 0 ? i - 1 : i;

                auto right = i < items ? i + 1 : i;

                scene.Add(Controls::Base(Controls::MapType("SELECT"), i, left, right, i, i, point.X, point.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));

                if (has_captions && (input.Current == i))
                {
                    auto caption = i;

                    auto caption_width = BloodSwordRogue::Width(captions[caption]);

                    // center caption
                    auto center = (BloodSwordRogue::TileSize - caption_width) / 2;

                    auto pc = point.X + center;

                    auto pcw = (pc + caption_width);

                    // adjust if caption spills over panel borders
                    if ((pc < bx) && i == 0)
                    {
                        center += (bx - pc + b2);
                    }
                    else if (pcw > bw)
                    {
                        center -= (pcw - bw + b2);
                    }

                    scene.VerifyAndAdd(Scene::Element(captions[caption], point.X + center, point.Y + BloodSwordRogue::TileSize + 2));
                }
            }

            auto back_id = SafeCast(scene.Controls.size());

            auto back = Point(box.X + BloodSwordRogue::HalfTile + items * (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile), box.Y + BloodSwordRogue::HalfTile);

            scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("BACK")), back));

            scene.Add(Controls::Base(Controls::MapType("BACK"), back_id, back_id - 1, back_id, back_id, back_id, back.X, back.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));

            Graphics::Scenery scenery = scenes;

            scenes.push_back(scene);

            input = Input::WaitForInput(graphics, scenery, scene.Controls, input, true);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("BACK"))
                {
                    done = true;
                }
                else if (input.Type == Controls::MapType("SELECT"))
                {
                    selected = input.Current;

                    done = true;
                }

                input.Current = -1;

                input.Selected = false;
            }
        }

        if (has_captions)
        {
            BloodSwordRogue::Free(captions);
        }

        return selected;
    }

    // select icon from a list
    int IconList(Graphics::Base &graphics, Scene::Base &background, Asset::List &assets, std::vector<std::string> captions_text = {})
    {
        Graphics::Scenery scenes = {background};

        return Interface::IconList(graphics, scenes, assets, captions_text);
    }

    // select icon from a grid
    int IconGrid(Graphics::Base &graphics, Graphics::Scenery scenes, Asset::List &assets, int width, int height, std::vector<std::string> captions_text = {})
    {
        auto selected = -1;

        if (assets.empty())
        {
            return selected;
        }

        auto box = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

        // number of icon columns
        auto limit_x = (width / (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile));

        // number of icon rows
        auto limit_y = ((height - BloodSwordRogue::TileSize) / (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile));

        auto items = SafeCast(assets.size());

        auto page_size = limit_x * limit_y;

        auto offset = 0;

        auto has_captions = SafeCast(captions_text.size()) > 0 && (captions_text.size() == assets.size());

        Asset::TextureList captions = has_captions ? Graphics::CreateText(graphics, Graphics::GenerateTextList(captions_text, Fonts::Caption, Color::Active, 0)) : Asset::TextureList();

        auto input = Controls::User();

        auto done = false;

        // pre-calculate possible caption position adjustments
        auto bx = box.X - BloodSwordRogue::Border;

        auto b2 = BloodSwordRogue::Border * 2;

        auto bw = box.X + width + BloodSwordRogue::Border;

        while (!done)
        {
            auto scene = Scene::Base();

            // icon grid
            scene.Add(Scene::Element(box.X - BloodSwordRogue::Border, box.Y - BloodSwordRogue::Border, width + BloodSwordRogue::Border * 2, height + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            auto page_count = 0;

            for (auto y = 0; y < limit_y; y++)
            {
                for (auto x = 0; x < limit_x; x++)
                {
                    auto id = (y * limit_x + x);

                    auto index = offset + id;

                    if (index >= 0 && index < items)
                    {
                        auto point = Point(box.X + BloodSwordRogue::HalfTile + x * (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile), box.Y + BloodSwordRogue::HalfTile + y * (BloodSwordRogue::TileSize + BloodSwordRogue::HalfTile));

                        scene.VerifyAndAdd(Scene::Element(Asset::Get(assets[index]), point));

                        auto left = x > 0 ? id - 1 : id;

                        auto right = (x < limit_x - 1) && (index < items - 1) ? id + 1 : id;

                        auto up = y > 0 ? id - limit_x : id;

                        auto down = (y < limit_y - 1) && (index + limit_x < items) ? id + limit_x : id;

                        scene.Add(Controls::Base(Controls::MapType("SELECT"), id, left, right, up, down, point.X, point.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));

                        page_count++;

                        if (has_captions && ((input.Current + offset) == index))
                        {
                            auto caption = index;

                            auto caption_width = BloodSwordRogue::Width(captions[caption]);

                            // center caption
                            auto center = (BloodSwordRogue::TileSize - caption_width) / 2;

                            auto pc = point.X + center;

                            auto pcw = (pc + caption_width);

                            // adjust if caption spills over panel borders
                            if ((pc < bx) && x == 0)
                            {
                                center += (bx - pc + b2);
                            }
                            else if (pcw > bw)
                            {
                                center -= (pcw - bw + b2);
                            }

                            scene.VerifyAndAdd(Scene::Element(captions[caption], point.X + center, point.Y + BloodSwordRogue::TileSize + 2));
                        }
                    }
                }
            }

            auto has_prev = offset > 0;

            auto has_next = items > (offset + page_count);

            // check if there are previous items
            if (has_prev)
            {
                auto prev_id = scene.Controls.size();

                auto prev = Point(box.X + width - (BloodSwordRogue::TileSize + BloodSwordRogue::Border) * (has_next ? 3 : 2) + BloodSwordRogue::Border, box.Y + height - BloodSwordRogue::TileSize + BloodSwordRogue::Border);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("LEFT")), prev));

                scene.Add(Controls::Base(Controls::MapType("LEFT"), prev_id, prev_id, prev_id + 1, prev_id, prev_id, prev.X, prev.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));
            }

            // check if there are more items
            if (has_next)
            {
                auto next_id = scene.Controls.size();

                auto next = Point(box.X + width - (BloodSwordRogue::TileSize + BloodSwordRogue::Border) * 2 + BloodSwordRogue::Border, box.Y + height - BloodSwordRogue::TileSize + BloodSwordRogue::Border);

                scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("RIGHT")), next));

                scene.Add(Controls::Base(Controls::MapType("RIGHT"), next_id, has_prev ? next_id - 1 : next_id, next_id + 1, next_id, next_id, next.X, next.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));
            }

            auto back_id = scene.Controls.size();

            auto back = Point(box.X + width - BloodSwordRogue::TileSize + BloodSwordRogue::Border, box.Y + height - BloodSwordRogue::TileSize + BloodSwordRogue::Border);

            scene.VerifyAndAdd(Scene::Element(Asset::Get(Asset::Map("BACK")), back));

            scene.Add(Controls::Base(Controls::MapType("BACK"), back_id, has_next || has_prev ? back_id - 1 : back_id, back_id, back_id, back_id, back.X, back.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Highlight));

            if (input.Type == Controls::MapType("LEFT"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("LEFT"));
            }
            else if (input.Type == Controls::MapType("RIGHT"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("RIGHT"));
            }

            Graphics::Scenery scenery = scenes;

            scenery.push_back(scene);

            input = Input::WaitForInput(graphics, scenery, scene.Controls, input, true);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("BACK"))
                {
                    done = true;
                }
                else if (input.Type == Controls::MapType("LEFT"))
                {
                    offset -= page_size;
                }
                else if (input.Type == Controls::MapType("RIGHT"))
                {
                    offset += page_size;
                }
                else if (input.Type == Controls::MapType("SELECT"))
                {
                    selected = offset + input.Current;

                    done = true;
                }

                input.Current = -1;

                input.Selected = false;
            }
        }

        if (has_captions)
        {
            BloodSwordRogue::Free(captions);
        }

        return selected;
    }

    int IconGrid(Graphics::Base &graphics, Scene::Base &background, Asset::List &assets, int width, int height, std::vector<std::string> captions_text = {})
    {
        Graphics::Scenery scenes = {background};

        return Interface::IconGrid(graphics, scenes, assets, width, height, captions_text);
    }

    // scroll up on texture
    void TextUp(Scene::Base &overlay, Controls::User &input, Controls::Type control, bool &up, int &offset, int texture_h, int text_h, int speed)
    {
        if (text_h < texture_h)
        {
            offset -= speed;

            if (offset < 0)
            {
                offset = 0;
            }

            up = true;
        }

        Controls::Select(input, overlay.Controls, control);
    }

    // scroll down on texture
    void TextDown(Scene::Base &overlay, Controls::User &input, Controls::Type control, bool &down, int &offset, int texture_h, int text_h, int speed)
    {
        if (text_h < texture_h)
        {
            offset += speed;

            if (offset > (texture_h - text_h))
            {
                offset = texture_h - text_h;
            }

            down = true;
        }

        Controls::Select(input, overlay.Controls, control);
    }

    // add scrollable texture (inside a box background) to scene
    void AddScrollableTextureBox(Scene::Base &scene, int x, int y, int width, int height, Uint32 bg_color, Uint32 border, int border_size, SDL_Texture *texture, int texture_h, int text_x, int text_y, int text_h, int offset, int controls_x, int controls_y, Asset::Type asset, Asset::Type left, Asset::Type right, int scroll_speed)
    {
        auto id = SafeCast(scene.Controls.size());

        // texture box panel (pad box to make it bigger)
        scene.Add(Scene::Element(Point(x, y) - BloodSwordRogue::TileSize, width + BloodSwordRogue::TileSize * 2, height + BloodSwordRogue::TileSize * 2, bg_color, border, border_size));

        // texture
        scene.VerifyAndAdd(Scene::Element(texture, text_x, text_y, text_h, offset));

        // scroll up (left)
        scene.VerifyAndAdd(Scene::Element(Asset::Get(left), controls_x, controls_y));

        scene.Add(Controls::Base(Controls::MapType("LEFT"), id, id, id + 1, id, id, controls_x, controls_y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

        if (offset <= 0)
        {
            // blur button
            scene.Add(Scene::Element(controls_x, controls_y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Blur));
        }

        id++;

        // close texturebox
        scene.VerifyAndAdd(Scene::Element(Asset::Get(asset), controls_x + BloodSwordRogue::TileSize + BloodSwordRogue::Pad, controls_y));

        scene.Add(Controls::Base(Controls::MapType("CONFIRM"), id, id - 1, id + 1, id, id, controls_x + BloodSwordRogue::TileSize + BloodSwordRogue::Pad, controls_y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

        id++;

        // scroll down (right)
        scene.VerifyAndAdd(Scene::Element(Asset::Get(right), controls_x + 128 + 16, controls_y));

        scene.Add(Controls::Base(Controls::MapType("RIGHT"), id, id - 1, id, id, id, controls_x + 128 + 16, controls_y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

        if (text_h >= texture_h || (offset + scroll_speed) > (texture_h - text_h))
        {
            // blur button
            scene.Add(Scene::Element(controls_x + 128 + 16, controls_y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Blur));
        }
    }

    // draws a scrollable image box
    void ScrollableImageBox(Graphics::Base &graphics, Scene::Base &background, SDL_Texture *texture, int width, int height, int x, int y, Uint32 bg_color, Uint32 border, int border_size, Uint32 highlight, Asset::Type asset, Asset::Type left, Asset::Type right, bool blur = true, int offset = 0)
    {
        if (texture)
        {
            auto text_h = std::min(height - (BloodSwordRogue::TileSize + 24), BloodSwordRogue::Height(texture));

            auto texture_h = BloodSwordRogue::Height(texture);

            auto text_x = x + BloodSwordRogue::Pad;

            auto text_y = y + BloodSwordRogue::Pad;

            auto input = Controls::User();

            auto controls_x = x + (width - 208) / 2;

            auto controls_y = y + height - BloodSwordRogue::Pad;

            auto scroll_speed = BloodSwordRogue::ScrollSpeed;

            auto done = false;

            while (!done)
            {
                auto scene = Scene::Base();

                Interface::AddScrollableTextureBox(scene, x, y, width, height, bg_color, border, border_size, texture, texture_h, text_x, text_y, text_h, offset, controls_x, controls_y, asset, left, right, scroll_speed);

                input = Input::WaitForInput(graphics, {background, scene}, scene.Controls, input, blur);

                if (Input::Validate(input))
                {
                    if (input.Type == Controls::MapType("LEFT") || input.Up)
                    {
                        Interface::TextUp(scene, input, Controls::MapType("LEFT"), input.Up, offset, texture_h, text_h, scroll_speed);
                    }
                    else if (input.Type == Controls::MapType("RIGHT") || input.Down)
                    {
                        Interface::TextDown(scene, input, Controls::MapType("RIGHT"), input.Down, offset, texture_h, text_h, scroll_speed);
                    }
                    else if (input.Type == Controls::MapType("CONFIRM"))
                    {
                        done = true;
                    }

                    input.Selected = false;
                }
            }
        }
    }

    // show scaled version of map
    void ShowMap(Graphics::Base &graphics, Scene::Base &background, Map::Base &map, bool hide = false)
    {
        auto scale = Point(16, 16);

        auto surface = Graphics::CreateSurface(map.Width * scale.X, map.Height * scale.Y);

        if (surface)
        {
            SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 255));

            SDL_Rect rect;

            rect.w = scale.X;

            rect.h = scale.Y;

            for (auto y = 0; y < map.Height; y++)
            {
                for (auto x = 0; x < map.Width; x++)
                {
                    SDL_Surface *surface_asset = nullptr;

                    auto &tile = map[Point(x, y)];

                    rect.x = x * scale.X;

                    rect.y = y * scale.Y;

                    if (hide)
                    {
                        if ((tile.Occupant == Map::Object::PARTY) || (tile.Occupant == Map::Object::PLAYER))
                        {
                            surface_asset = Asset::GetSurface("WHITE SPACE", Color::Active);
                        }
                        else if (tile.Explored && tile.Asset != Asset::NONE)
                        {
                            surface_asset = Asset::GetSurface(tile.Asset);
                        }
                        else if (!tile.Explored)
                        {
                            surface_asset = Graphics::CreateSurface(scale.X, scale.Y);

                            SDL_FillRect(surface_asset, nullptr, SDL_MapRGBA(surface_asset->format, Color::R(Color::Inactive), Color::G(Color::Inactive), Color::B(Color::Inactive), 255));
                        }
                    }
                    else if (tile.IsOccupied())
                    {
                        if (tile.Occupant == Map::Object::ENEMIES)
                        {
                            surface_asset = Asset::GetSurface("CHARACTER", Color::Highlight);
                        }
                        else if (tile.Occupant == Map::Object::ITEMS)
                        {
                            surface_asset = Asset::GetSurface("ITEMS", Color::Highlight);
                        }
                        else if ((tile.Occupant == Map::Object::PARTY) || (tile.Occupant == Map::Object::PLAYER))
                        {
                            surface_asset = Asset::GetSurface("CHARACTER", Color::Active);
                        }
                    }
                    else if (tile.Asset != Asset::NONE)
                    {
                        surface_asset = Asset::GetSurface(tile.Asset);
                    }

                    if (surface_asset)
                    {
                        Graphics::RenderAssetScaled(surface, surface_asset, rect);

                        BloodSwordRogue::Free(&surface_asset);
                    }
                }
            }

            auto texture = SDL_CreateTextureFromSurface(graphics.Renderer, surface);

            if (texture)
            {
                auto width = BloodSwordRogue::Width(texture) + 16;

                auto height = std::min(400, BloodSwordRogue::Height(texture) + BloodSwordRogue::TileSize + 24);

                auto x = (graphics.Width - width) / 2;

                auto y = (graphics.Height - height) / 2;

                // calculate offset to center current location
                auto text_h = std::min(height - (BloodSwordRogue::TileSize + 24), BloodSwordRogue::Height(texture));

                auto loc = map.ViewY * scale.Y + scale.Y / 2;

                auto offset = 0;

                if (loc > text_h / 2)
                {
                    offset = (loc - text_h / 2);
                }

                offset = std::min(std::max(0, offset), BloodSwordRogue::Height(texture) - text_h);

                Interface::ScrollableImageBox(graphics, background, texture, width, height, x, y, Color::Background, Color::Active, BloodSwordRogue::Border, Color::Active, Asset::Map("CONFIRM"), Asset::Map("UP"), Asset::Map("DOWN"), true, offset);

                BloodSwordRogue::Free(&texture);
            }

            BloodSwordRogue::Free(&surface);
        }
    }

    // add character to party in map location
    void AddCharacter(Graphics::Base &graphics, Scene::Base &scene, std::vector<Party::Base> &parties, Character::Base &character, Map::Base &map, Map::Object object, Point point, int max_characters)
    {
        if (!map.IsValid(point))
        {
            SDL_Log("[INVALID LOCATION] (%d, %d)", point.X, point.Y);

            return;
        }

        auto &tile = map[point];

        if (tile.IsOccupied() && tile.Id != Map::NotFound)
        {
            auto id = tile.Id - 1;

            if (!(id >= 0 && id < parties.size()) || parties[id].Count() >= max_characters)
            {
                Interface::MessageBox(graphics, scene, "CANNOT ADD MORE TO THE PARTY", Color::Highlight);

                return;
            }
        }

        if (tile.Type == Map::Object::PASSABLE || tile.Type == Map::Object::ENEMY_PASSABLE)
        {
            if (tile.IsOccupied() && tile.Id != Map::NotFound)
            {
                // add character to existing party
                auto id = tile.Id - 1;

                parties[id].Add(character);

                Interface::MessageBox(graphics, scene, character.Name + std::string(" ADD TO PARTY"), Color::Active);

                SDL_Log("[ADDED TO PARTY %d] [ADD %s]", id + 1, character.Name.c_str());
            }
            else
            {
                auto party = Party::Base();

                party.Add(character);

                party.X = point.X;

                party.Y = point.Y;

                auto id = SafeCast(parties.size()) + 1;

                parties.push_back(party);

                tile.Id = id;

                tile.Occupant = object;

                Interface::MessageBox(graphics, scene, std::string("CREATED PARTY WITH ") + character.Name, Color::Active);

                SDL_Log("[CREATE PARTY %d] [ADD %s]", id, character.Name.c_str());
            }
        }
    }

    // renumber remaining map occupants
    void RenumberParties(Map::Base &map, std::vector<Party::Base> &parties)
    {
        if (SafeCast(parties.size()) > 0)
        {
            auto current = 1;

            // renumber remaining opponents
            for (auto &party : parties)
            {
                SDL_Log("[UPDATE PARTY %d] [NEW ID %d]", map[party.Origin()].Id, current);

                map[party.Origin()].Id = current;

                current++;
            }
        }
    }

    // remove enemy from party (in selected map location)
    void RemoveCharacter(Graphics::Base &graphics, Scene::Base &scene, Map::Base &map, std::vector<Party::Base> &parties, Point &point, int selected)
    {
        if (!map.IsValid(point))
        {
            SDL_Log("[INVALID LOCATION] (%d, %d)", point.X, point.Y);

            return;
        }

        auto &tile = map[point];

        if (tile.Id > 0 && tile.Id <= SafeCast(parties.size()))
        {
            auto id = tile.Id - 1;

            auto &party = parties[id];

            if (selected >= 0 && selected < party.Count())
            {
                SDL_Log("[UPDATE PARTY %d] [REMOVED %s]", id + 1, party[selected].Name.c_str());

                party.RemoveCharacter(selected);
            }

            if (party.Count() <= 0)
            {
                SDL_Log("[REMOVED PARTY %d]", id + 1);

                parties.erase(parties.begin() + id);

                // remove party from current location
                tile.Id = Map::NotFound;

                tile.Occupant = Map::Object::NONE;

                Interface::RenumberParties(map, parties);
            }
        }
        else
        {
            Interface::MessageBox(graphics, scene, "CANNOT REMOVE FROM THE PARTY", Color::Highlight);
        }
    }

    // get text from user input (popup interface)
    std::string TextBoxInput(Graphics::Base &graphics, Graphics::Scenery scenes, Point location, std::string question, std::string start_text, Uint32 question_color, Uint32 input_color, int input_limit, int box_w, int box_h, int wrap, Uint32 border = Color::Active, Uint32 box_bg = Color::Background, int border_size = BloodSwordRogue::Border, bool allow_empty = false, bool blur = true)
    {
        auto message = Graphics::CreateText(graphics, question.c_str(), Fonts::Normal, Color::S(question_color), TTF_STYLE_NORMAL, 0);

        auto input_text = std::string();

        if (message)
        {
            SDL_Texture *texture = nullptr;

            auto input = Controls::User();

            auto pad = 16;

            input.TextLimit = input_limit;

            input.SetText(start_text);

            if (SafeCast(input.TextInput.size()) > 0)
            {
                texture = Graphics::CreateText(graphics, input.TextInput.c_str(), Fonts::Normal, Color::S(input_color), TTF_STYLE_NORMAL, wrap);
            }

            // setup text input mode
            input.Text = true;

            // cursor blink
            auto blink = false;

            // enable text input events
            Input::StartTextInput();

            while (true)
            {
                auto box = Scene::Base();

                box.Add(Scene::Element(location, box_w, box_h, box_bg, border, border_size));

                box.VerifyAndAdd(Scene::Element(message, location + Point(pad, pad)));

                if (texture)
                {
                    auto threshold = (box_h - BloodSwordRogue::TileSize - pad);

                    auto offset = 0;

                    if (BloodSwordRogue::Height(texture) > threshold)
                    {
                        offset = BloodSwordRogue::Height(texture) - threshold;

                        box.VerifyAndAdd(Scene::Element(texture, location.X + pad, location.Y + BloodSwordRogue::TileSize, threshold, offset));
                    }
                    else
                    {
                        box.VerifyAndAdd(Scene::Element(texture, location + Point(pad, BloodSwordRogue::TileSize)));
                    }
                }

                blink = !blink;

                auto scenery = scenes;

                scenery.push_back(box);

                input = Input::WaitForText(graphics, scenery, box.Controls, input, blur, BloodSwordRogue::StandardDelay);

                if (input.Selected && (allow_empty || (!allow_empty && SafeCast(input.TextInput.size()) > 0)))
                {
                    break;
                }
                else
                {
                    BloodSwordRogue::Free(&texture);

                    auto text_input = input.TextInput + (blink ? std::string("_") : std::string(" "));

                    texture = Graphics::CreateText(graphics, text_input.c_str(), Fonts::Normal, Color::S(input_color), TTF_STYLE_NORMAL, wrap);
                }
            }

            // disable text input events
            Input::StopTextInput();

            BloodSwordRogue::Free(&texture);

            BloodSwordRogue::Free(&message);

            input_text = std::string(input.TextInput);
        }

        return input_text;
    }

    // get text from user input (popup interface)
    std::string TextInput(Graphics::Base &graphics, Graphics::Scenery scenes, Point location, std::string question, std::string start_text, Uint32 question_color, Uint32 input_color, int input_limit, int box_w, int box_h, Uint32 border = Color::Active, Uint32 box_bg = Color::Background, int border_size = BloodSwordRogue::Border, bool blur = true)
    {
        auto message = Graphics::CreateText(graphics, question.c_str(), Fonts::Normal, Color::S(question_color), TTF_STYLE_NORMAL, 0);

        auto input_text = std::string();

        if (message)
        {
            auto cursor = Graphics::CreateText(graphics, "_", Fonts::Normal, Color::S(Color::Highlight), TTF_STYLE_NORMAL, 0);

            SDL_Texture *texture = nullptr;

            auto input = Controls::User();

            auto pad = 16;

            input.TextLimit = input_limit;

            input.SetText(start_text);

            input.RefreshText = (SafeCast(input.TextInput.size()) > 0);

            if (SafeCast(input.TextInput.size()) > 0)
            {
                texture = Graphics::CreateText(graphics, input.TextInput.c_str(), Fonts::Normal, Color::S(input_color), TTF_STYLE_NORMAL, 0);
            }

            // setup text input mode
            input.Text = true;

            // cursor blink
            auto blink = false;

            // enable text input events
            Input::StartTextInput();

            while (true)
            {
                auto box = Scene::Base();

                box.Add(Scene::Element(location, box_w, box_h, box_bg, border, border_size));

                box.VerifyAndAdd(Scene::Element(message, location + Point(pad, pad)));

                if (texture)
                {
                    box.VerifyAndAdd(Scene::Element(texture, location + Point(pad, BloodSwordRogue::TileSize)));
                }

                // add blinking cursor
                if (blink && SafeCast(input.TextInput.size()) < input.TextLimit)
                {
                    auto pad_cursor = texture ? BloodSwordRogue::Width(texture) : 0;

                    box.VerifyAndAdd(Scene::Element(cursor, location + Point(pad + pad_cursor, BloodSwordRogue::TileSize)));
                }

                blink = !blink;

                auto scenery = scenes;

                scenery.push_back(box);

                input = Input::WaitForText(graphics, scenery, box.Controls, input, blur, BloodSwordRogue::StandardDelay);

                if (input.RefreshText)
                {
                    BloodSwordRogue::Free(&texture);

                    if (SafeCast(input.TextInput.size()) > 0)
                    {
                        texture = Graphics::CreateText(graphics, input.TextInput.c_str(), Fonts::Normal, Color::S(input_color), TTF_STYLE_NORMAL, 0);
                    }
                }
                else if (input.Selected && SafeCast(input.TextInput.size()) > 0)
                {
                    break;
                }
            }

            // disable text input events
            Input::StopTextInput();

            BloodSwordRogue::Free(&texture);

            BloodSwordRogue::Free(&cursor);

            BloodSwordRogue::Free(&message);

            input_text = std::string(input.TextInput);
        }

        return input_text;
    }

    // get text from user input (popup interface)
    std::string TextInput(Graphics::Base &graphics, Graphics::Scenery scenes, std::string question, int input_limit, int box_w, int box_h, bool blur = true)
    {
        auto location = (Point(graphics.Width, graphics.Height) - Point(box_w, box_h)) / 2;

        return Interface::TextInput(graphics, scenes, location, question, "", Color::Inactive, Color::Active, input_limit, box_w, box_h, Color::Active, Color::Background, BloodSwordRogue::Border, blur);
    }

    std::string TextInput(Graphics::Base &graphics, Graphics::Scenery scenes, std::string question, int box_w, int box_h, bool blur = true)
    {
        return Interface::TextInput(graphics, scenes, question, 20, box_w, box_h, blur);
    }

    // get text from user input (popup interface)
    std::string TextInput(Graphics::Base &graphics, Graphics::Scenery scenes, std::string question, bool blur = true)
    {
        auto box_w = (SafeCast(question.size()) > 16) ? (SafeCast(question.size()) * 16) : (320);

        return Interface::TextInput(graphics, scenes, question, box_w, (128 - BloodSwordRogue::Pad), blur);
    }

    std::string TextInput(Graphics::Base &graphics, Graphics::Scenery scenes, std::string question, std::string start_text, int input_limit, bool blur = true)
    {
        auto box_w = (SafeCast(question.size()) > 16) ? (SafeCast(question.size()) * 16) : (320);

        auto box_h = (128 - BloodSwordRogue::Pad);

        auto location = (Point(graphics.Width, graphics.Height) - Point(box_w, box_h)) / 2;

        return Interface::TextInput(graphics, scenes, location, question, start_text, Color::Inactive, Color::Active, input_limit, box_w, box_h, Color::Active, Color::Background, BloodSwordRogue::Border, blur);
    }

    // return a filename or new one from the list
    std::string FilesList(Graphics::Base &graphics, Graphics::Scenery scenes, std::string path, int width, int height, Asset::Type asset, Controls::Type action)
    {
        auto filename = std::string();

        std::vector<std::string> files = {};

        auto limit = 5;

        auto text_height = 0;

        auto text_width = 0;

        auto max_text_height = 0;

        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            auto extension = entry.path().extension();

            if (extension != std::string(".json"))
            {
                continue;
            }

            // List files in directory
            auto file_entry = entry.path().stem();

            files.push_back(file_entry);

            Graphics::Estimate(Fonts::Normal, file_entry.c_str(), &text_width, &text_height);

            width = std::max(width, text_width + BloodSwordRogue::TileSize);

            max_text_height = std::max(max_text_height, text_height);

            height = std::max(height, (max_text_height + BloodSwordRogue::Pad) * limit + BloodSwordRogue::TileSize * 2);

            // List files in directory
            SDL_Log("[DIRECTORY %s] [%s]", path.c_str(), file_entry.c_str());
        }

        auto items = SafeCast(files.size());

        auto done = false;

        auto input = Controls::User();

        auto offset = 0;

        auto selected = -1;

        while (!done)
        {
            auto scene = Scene::Base();

            Asset::TextureList assets = {};

            auto box = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

            scene.Add(Scene::Element(box.X - BloodSwordRogue::Border, box.Y - BloodSwordRogue::Border, width + BloodSwordRogue::Border * 2, height + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            for (auto i = 0; i < limit; i++)
            {
                if ((offset + i) >= 0 && (offset + i) < items)
                {
                    auto color = selected == (offset + i) ? Color::S(Color::Highlight) : (input.Current == i ? Color::S(Color::Active) : Color::S(Color::Inactive));

                    assets.push_back(Graphics::CreateText(graphics, files[offset + i].c_str(), Fonts::Normal, color, TTF_STYLE_NORMAL));

                    auto loc = Point(box.X + BloodSwordRogue::Pad, box.Y + i * (max_text_height + BloodSwordRogue::Pad) + BloodSwordRogue::Pad);

                    scene.VerifyAndAdd(Scene::Element(assets.back(), loc.X + 4, loc.Y));

                    auto up = i > 0 ? i - 1 : i;

                    auto down = i + 1;

                    scene.Add(Controls::Base(Controls::MapType("SELECT"), i, i, i, up, down, loc.X - BloodSwordRogue::HalfTile / 2, loc.Y - 4, width + BloodSwordRogue::HalfTile / 2, max_text_height, Color::Transparent));
                }
            }

            auto x_offset = (BloodSwordRogue::TileSize + BloodSwordRogue::Pad);

            auto controls = 0;

            // add main control
            auto control_id = SafeCast(scene.Controls.size());

            auto control = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

            scene.Add(Scene::Element(Asset::Get(asset), control));

            scene.Add(Controls::Base(action, control_id, controls > 0 ? control_id - 1 : control_id, control_id, control_id - (controls + 1), control_id, control.X, control.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            controls++;

            if (offset + limit < items)
            {
                auto down_id = SafeCast(scene.Controls.size());

                auto down = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

                scene.Add(Scene::Element(Asset::Get(Asset::Map("DOWN")), down));

                scene.Add(Controls::Base(Controls::MapType("DOWN"), down_id, controls > 0 ? down_id - 1 : down_id, down_id + 1, down_id - (controls + 1), down_id, down.X, down.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

                controls++;
            }

            if (offset > 0)
            {
                auto up_id = SafeCast(scene.Controls.size());

                auto up = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

                scene.Add(Scene::Element(Asset::Get(Asset::Map("UP")), up));

                scene.Add(Controls::Base(Controls::MapType("UP"), up_id, controls > 0 ? up_id - 1 : up_id, up_id + 1, up_id - (controls + 1), up_id, up.X, up.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

                controls++;
            }

            auto back_id = SafeCast(scene.Controls.size());

            auto back = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

            scene.Add(Scene::Element(Asset::Get(Asset::Map("BACK")), back));

            scene.Add(Controls::Base(Controls::MapType("BACK"), back_id, controls > 0 ? back_id - 1 : back_id, back_id, back_id - (controls + 1), back_id, back.X, back.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            // retain focus on scroll controls
            if (input.Type == Controls::MapType("UP"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("UP"));
            }
            else if (input.Type == Controls::MapType("DOWN"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("DOWN"));
            }

            auto scenery = scenes;

            scenes.push_back(scene);

            input = Input::WaitForInput(graphics, scenery, scene.Controls, input);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("BACK"))
                {
                    selected = -1;

                    filename = std::string();

                    done = true;
                }
                else if (input.Type == Controls::MapType("UP"))
                {
                    if (offset > 0)
                    {
                        offset--;
                    }
                }
                else if (input.Type == Controls::MapType("DOWN"))
                {
                    if (offset + limit < items)
                    {
                        offset++;
                    }
                }
                else if (input.Type == Controls::MapType("SELECT"))
                {
                    if (selected == (input.Current + offset))
                    {
                        selected = -1;
                    }
                    else
                    {
                        selected = input.Current + offset;
                    }
                }
                else if (input.Type == action)
                {
                    if (action == Controls::MapType("LOAD"))
                    {
                        if (selected >= 0 && selected < items)
                        {
                            filename = std::string(files[selected]);
                        }

                        done = true;
                    }
                    else if (action == Controls::MapType("SAVE"))
                    {
                        if (selected >= 0 && selected < items)
                        {
                            auto file = std::string(files[selected]);

                            auto confirm = std::string("OVERWRITE ") + file + std::string("?");

                            if (Interface::Confirm(graphics, scenery, confirm, Color::Background, Color::Highlight, BloodSwordRogue::Border, Color::Active, true))
                            {
                                filename = std::string(file);

                                done = true;
                            }
                        }
                        else
                        {
                            auto question = std::string("CREATE NEW FILE");

                            auto file = BloodSwordRogue::Trim(Interface::TextInput(graphics, scenery, question, "", 20, true));

                            if (SafeCast(file.size()) > 0)
                            {
                                filename = std::string(file);

                                done = true;
                            }
                        }
                    }
                }
            }

            BloodSwordRogue::Free(assets);
        }

        return filename;
    }

    // generic text list
    int TextList(Graphics::Base &graphics, Graphics::Scenery scenes, std::vector<std::string> &text_list, int width, int height, Asset::Type asset, Controls::Type action, int selected = -1)
    {
        auto text_height = 0;

        auto text_width = 0;

        // estimate size of text
        TTF_SizeText(Fonts::Normal, "M", &text_width, &text_height);

        auto max_text_height = text_height;

        auto items = SafeCast(text_list.size());

        auto limit = 5;

        for (const auto &entry : text_list)
        {
            Graphics::Estimate(Fonts::Normal, entry.c_str(), &text_width, &text_height);

            width = std::max(width, text_width + BloodSwordRogue::TileSize);

            max_text_height = std::max(max_text_height, text_height);

            height = std::max(height, (max_text_height + BloodSwordRogue::Pad) * limit + BloodSwordRogue::TileSize * 2);
        }

        auto done = false;

        auto input = Controls::User();

        auto offset = 0;

        // clip selected
        selected = std::min(std::max(-1, selected), items - 1);

        while (!done)
        {
            auto scene = Scene::Base();

            Asset::TextureList assets = {};

            auto box = Point((graphics.Width - width) / 2, (graphics.Height - height) / 2);

            scene.Add(Scene::Element(box.X - BloodSwordRogue::Border, box.Y - BloodSwordRogue::Border, width + BloodSwordRogue::Border * 2, height + BloodSwordRogue::Border * 2, Color::Background, Color::Active, BloodSwordRogue::Border));

            for (auto i = 0; i < limit; i++)
            {
                if ((offset + i) >= 0 && (offset + i) < items)
                {
                    auto color = selected == (offset + i) ? Color::S(Color::Highlight) : (input.Current == i ? Color::S(Color::Active) : Color::S(Color::Inactive));

                    assets.push_back(Graphics::CreateText(graphics, text_list[offset + i].c_str(), Fonts::Normal, color, TTF_STYLE_NORMAL));

                    auto loc = Point(box.X + BloodSwordRogue::Pad, box.Y + i * (max_text_height + BloodSwordRogue::Pad) + BloodSwordRogue::Pad);

                    scene.VerifyAndAdd(Scene::Element(assets.back(), loc.X + 4, loc.Y));

                    auto up = i > 0 ? i - 1 : i;

                    auto down = i + 1;

                    scene.Add(Controls::Base(Controls::MapType("SELECT"), i, i, i, up, down, loc.X - BloodSwordRogue::HalfTile / 2, loc.Y - 4, width + BloodSwordRogue::HalfTile / 2, max_text_height, Color::Transparent));
                }
            }

            auto x_offset = (BloodSwordRogue::TileSize + BloodSwordRogue::Pad);

            auto controls = 0;

            // add scroll controls
            if (offset + limit < items)
            {
                auto down_id = SafeCast(scene.Controls.size());

                auto down = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

                scene.Add(Scene::Element(Asset::Get(Asset::Map("DOWN")), down));

                scene.Add(Controls::Base(Controls::MapType("DOWN"), down_id, controls > 0 ? down_id - 1 : down_id, down_id + 1, down_id - (controls + 1), down_id, down.X, down.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

                controls++;
            }

            if (offset > 0)
            {
                auto up_id = SafeCast(scene.Controls.size());

                auto up = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

                scene.Add(Scene::Element(Asset::Get(Asset::Map("UP")), up));

                scene.Add(Controls::Base(Controls::MapType("UP"), up_id, controls > 0 ? up_id - 1 : up_id, up_id + 1, up_id - (controls + 1), up_id, up.X, up.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

                controls++;
            }

            // add action control
            auto action_id = SafeCast(scene.Controls.size());

            auto action_pos = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

            scene.Add(Scene::Element(Asset::Get(asset), action_pos));

            scene.Add(Controls::Base(action, action_id, controls > 0 ? action_id - 1 : action_id, action_id + 1, action_id - (controls + 1), action_id, action_pos.X, action_pos.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            controls++;

            // add back button
            auto back_id = SafeCast(scene.Controls.size());

            auto back = Point(box.X + controls * x_offset + BloodSwordRogue::Pad, box.Y + height - BloodSwordRogue::Pad - BloodSwordRogue::TileSize);

            scene.Add(Scene::Element(Asset::Get(Asset::Map("BACK")), back));

            scene.Add(Controls::Base(Controls::MapType("BACK"), back_id, controls > 0 ? back_id - 1 : back_id, back_id, back_id - (controls + 1), back_id, back.X, back.Y, BloodSwordRogue::TileSize, BloodSwordRogue::TileSize, Color::Active));

            // retain focus on scroll controls
            if (input.Type == Controls::MapType("UP"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("UP"));
            }
            else if (input.Type == Controls::MapType("DOWN"))
            {
                input.Current = Controls::Find(scene.Controls, Controls::MapType("DOWN"));
            }

            auto scenery = scenes;

            scenes.push_back(scene);

            input = Input::WaitForInput(graphics, scenery, scene.Controls, input);

            if (Input::Check(input))
            {
                if (input.Type == Controls::MapType("BACK"))
                {
                    selected = -1;

                    done = true;
                }
                else if (input.Type == Controls::MapType("UP"))
                {
                    if (offset > 0)
                    {
                        offset--;
                    }
                }
                else if (input.Type == Controls::MapType("DOWN"))
                {
                    if (offset + limit < items)
                    {
                        offset++;
                    }
                }
                else if (input.Type == Controls::MapType("SELECT"))
                {
                    if (selected == (input.Current + offset))
                    {
                        selected = -1;
                    }
                    else
                    {
                        selected = input.Current + offset;
                    }
                }
                else if (input.Type == action)
                {
                    if (selected >= 0 && selected < items)
                    {
                        done = true;
                    }
                }
            }

            BloodSwordRogue::Free(assets);
        }

        return selected;
    }
}