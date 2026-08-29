#pragma once

#include "Templates.hpp"

namespace BloodSwordRogue::Controls
{
    const int NONE = -1;

    // control types
    typedef int Type;

    // list of control types
    typedef std::vector<Controls::Type> List;

    template <typename T>
    using Mapping = UnorderedMap<Controls::Type, T>;

    // control type not found
    const int NotFound = -1;

    // default selected control
    int Default = -1;

    // template for mapping control types to other types
    template <typename T>
    using Mapped = UnorderedMap<Controls::Type, T>;

    // mapping of control types to strings
    Controls::Mapping<std::string> TypeMapping = {};

    Controls::List Spells = {};

    // map control type from string
    Controls::Type MapType(std::string control)
    {
        return BloodSwordRogue::Find(Controls::TypeMapping, control, Controls::NONE);
    }

    // map control type from string
    Controls::Type MapType(const char *control)
    {
        return Controls::MapType(std::string(control));
    }
}
