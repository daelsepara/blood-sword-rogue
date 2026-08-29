#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

// filesystem namespace alias
namespace fs = std::filesystem;

// files
namespace BloodSwordRogue::Files
{
    std::string ConvertString(std::wstring &wstr)
    {
        // calculate required buffer size
        size_t len = wcstombs(nullptr, wstr.c_str(), 0) + 1;

        // allocate buffer
        char *buffer = new char[len];

        // perform conversion
        wcstombs(buffer, wstr.c_str(), len);

        // create std::string
        std::string str(buffer);

        // clean up
        delete[] buffer;

        return str;
    }

    // patform-dependent function for returning user document directory
    std::string GetUserPath()
    {
#if defined(_WIN32)
        PWSTR PathString;

        SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &PathString);

        std::wstring WindowsPath(PathString);

        CoTaskMemFree(PathString);

        auto UserPath = Interface::ConvertString(WindowsPath);
#else
        const char *HomeDirectory = nullptr;

        if ((HomeDirectory = getenv("HOME")) == nullptr)
        {
            HomeDirectory = getpwuid(getuid())->pw_dir;
        }

        auto UserPath = std::string(HomeDirectory) + "/Documents";
#endif

        return UserPath;
    }

    std::string GetMainPath()
    {
        auto DocumentsPath = std::string("/Blood Sword Rogue");

        return Files::GetUserPath() + std::string(DocumentsPath);
    }

    // create subdirectory
    void CreateDirectories(std::string directory)
    {
        fs::create_directories(Files::GetMainPath() + directory);
    }

    // create directories
    void CreateDirectories()
    {
        auto directories = std::vector<std::string>({"/Saved Games", "/Locations"});

        for (auto &directory : directories)
        {
            Files::CreateDirectories(directory);
        }
    }
}