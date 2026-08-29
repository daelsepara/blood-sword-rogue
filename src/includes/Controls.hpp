#pragma once

#include "ControlTypes.hpp"
#include "Utilities.hpp"
#include "ZipFileLibrary.hpp"

namespace BloodSwordRogue::Controls
{
    // base control class
    class Base
    {
    public:
        // type of control. used in distinguishing selected control in user input
        Controls::Type Type = Controls::NONE;

        // ID
        int Id = -1;

        // ID of control to the left of this control, equal to ID if none
        int Left = -1;

        // ID of control to the right of this control, equal to ID if none
        int Right = -1;

        // ID of control above this control, equal to ID if none
        int Up = -1;

        // ID of control below this control, equal to ID if none
        int Down = -1;

        // X location on the screen (part of this control's hitbox definition)
        int X = 0;

        // Y location on the screen (part of this control's hitbox definition)
        int Y = 0;

        // width of this control's hitbox
        int W = 0;

        // height of this control's hitbox
        int H = 0;

        // border size (when highligted)
        int Pixels = Pixel;

        // control is on the map
        bool OnMap = false;

        // location on the map
        Point Map = Point(-1, -1);

        // color of border when hightlighted
        Uint32 Highlight;

        Base(Controls::Type type,
             int id, int left, int right, int up, int down,
             int x, int y, int w, int h,
             int highlight) : Type(type),
                              Id(id), Left(left), Right(right), Up(up), Down(down),
                              X(x), Y(y), W(w), H(h),
                              Highlight(highlight) {}

        Base(Controls::Type type,
             int id, int left, int right, int up, int down,
             int x, int y, int w, int h, int highlight,
             int mapx, int mapy) : Base(type, id, left, right, up, down, x, y, w, h, highlight)
        {
            this->OnMap = true;

            this->Map = Point(mapx, mapy);
        }

        Base(Controls::Type type,
             int id, int left, int right, int up, int down,
             Point p, int w, int h, int highlight,
             Point map) : Base(type,
                               id, left, right, up, down,
                               p.X, p.Y, w, h, highlight,
                               map.X, map.Y) {}

        Base() {}
    };

    // user input base class
    class User
    {
    public:
        // type of the control currently in focus
        Controls::Type Type = Controls::NONE;

        // ID of the control currently in focus, -1 if none
        int Current = -1;

        // the control has been explicitly selected
        // i.e. through a mouse button click, pressing a button on the gamepad,
        // or via the RETURN key (on the keyboard)
        bool Selected = false;

        // a scroll up event
        bool Up = false;

        // a scroll down event
        bool Down = false;

        // control is currently being held down (e.g. mouse left button held over the control, etc.)
        bool Hold = false;

        // quit/exit/terminate event
        bool Quit = false;

        // blink curser
        bool Blink = false;

        // text event
        bool Text = false;

        // flag to indicate a re-render text
        bool RefreshText = false;

        // input text
        std::string TextInput;

        // Character limit
        int TextLimit = 20;

        User(Controls::Type type,
             int current,
             bool selected,
             bool up,
             bool down,
             bool hold) : Type(type),
                          Current(current),
                          Selected(selected),
                          Up(up),
                          Down(down),
                          Hold(hold) {}

        User(int current,
             bool selected,
             bool up,
             bool down) : Current(current), Selected(selected), Up(up), Down(down) {}

        User(int current, bool selected) : Current(current), Selected(selected) {}

        User(int current) : Current(current) {}

        User() { this->Current = Controls::Default; }

        // set input text
        void SetText(const char *text)
        {
            this->TextInput = std::string(text);
        }

        // set input text
        void SetText(std::string text)
        {
            this->TextInput = std::string(text);
        }

        // clear input text
        void ClearText()
        {
            this->TextInput.clear();
        }

        // clear input
        void Clear()
        {
            this->Type = Controls::NONE;

            this->Selected = false;

            this->Current = -1;
        }
    };

    // list of controls
    typedef std::vector<Controls::Base> Collection;

    // find if control is present in the list
    int Find(Controls::Collection &controls, Controls::Type type)
    {
        auto result = -1;

        for (auto &control : controls)
        {
            if (control.Type == type)
            {
                result = control.Id;

                break;
            }
        }

        return result;
    }

    // find control type in the list
    int Find(Controls::List &controls, Controls::Type type)
    {
        auto result = -1;

        for (auto id = 0; id < SafeCast(controls.size()); id++)
        {
            if (controls[id] == type)
            {
                result = id;

                break;
            }
        }

        return result;
    }

    // select control from list of controls
    void Select(Controls::User &input, Controls::Collection &controls, Controls::Type control)
    {
        input.Current = Controls::Find(controls, control);

        if (input.Current != -1)
        {
            input.Type = control;
        }

        input.Selected = false;
    }

    // select control from list of controls
    void Select(Controls::User &input, Controls::List &controls, Controls::Type control)
    {
        input.Current = Controls::Find(controls, control);

        if (input.Current != -1)
        {
            input.Type = control;
        }

        input.Selected = false;
    }

    bool Generate(std::string &json_file)
    {
        if (json_file.empty())
        {
            return false;
        }

        auto data = nlohmann::json::parse(json_file);

        // load control types
        LoadListMap(data, "control-types", Controls::TypeMapping);

        // load spells controls
        LoadList(data, "controls-spells", Controls::Spells, Controls::MapType);

        return !Controls::TypeMapping.empty();
    }

    // load control types from a zip archive
    bool Load(std::string controls, const char *zip_file)
    {
        Controls::TypeMapping.clear();

        auto result = false;

        auto json_file = zip_file != nullptr ? ZipFile::Read(zip_file, controls) : Read(controls.c_str());

        if (!json_file.empty())
        {
            result = Generate(json_file);
        }

        return result;
    }

    // load control types
    bool Load(const char *controls)
    {
        return Load(controls, nullptr);
    }

    // load control types from a file
    bool Load(std::string controls, std::string zip_file)
    {
        return Load(controls.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }

    bool Load(std::string controls)
    {
        return Load(controls, std::string());
    }
}
