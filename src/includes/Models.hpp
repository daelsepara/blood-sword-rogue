#pragma once

namespace BloodSwordRogue::Models
{
    // update flags struct
    struct Update
    {
        bool Scene = false;

        bool Party = false;

        bool Quit = false;
    };
}
