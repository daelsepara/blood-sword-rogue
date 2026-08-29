#include "includes/BloodSwordRogue.hpp"

// BloodSwordRogue Rogue
namespace BloodSwordRogue
{
    // main loop
    int Main()
    {
        auto return_code = 0;

        auto graphics = Graphics::Initialize("Blood Sword Rogue", "modules/default/images/icons/sword-wound.png");

        Interface::LoadSettings(graphics, "modules/default/settings.json");

        FontCache::Base TextCache = FontCache::Base();

        auto scene = Scene::Base();

        Interface::MessageBox(graphics, scene, "ASSETS LOADED: PRESS BUTTON TO EXIT", Color::Active);

        TextCache.Free();

        Interface::UnloadAssets();

        Graphics::Quit(graphics);

        return return_code;
    }
}

int main(int argc, char **argv)
{
    return BloodSwordRogue::Main();
}
