#pragma once

#include "Primitives.hpp"
#include "Templates.hpp"

namespace BloodSwordRogue
{
    template <typename T>
    void LoadListMap(nlohmann::json data, const char *key, StringMap<T> &Map)
    {
        if (!data[key].is_null() && data[key].is_array() && SafeCast(data[key].size()) > 0)
        {
            auto type = 0;

            for (auto i = 0; i < SafeCast(data[key].size()); i++)
            {
                auto item = !data[key][i].is_null() ? std::string(data[key][i]) : std::string();

                if (!item.empty())
                {
#ifdef DEBUG
                    SDL_Log("[LOADED LIST MAP %s] [%d: %s]", key, type, item.c_str());
#endif
                    Map[type] = std::string(item);

                    type++;
                }
            }
        }
    }

    template <typename T>
    void LoadList(nlohmann::json data, const char *key, std::vector<T> &List, T (*func)(std::string))
    {
        if (!data[key].is_null() && data[key].is_array() && SafeCast(data[key].size()) > 0)
        {
            for (auto i = 0; i < SafeCast(data[key].size()); i++)
            {
                auto item = !data[key][i].is_null() ? std::string(data[key][i]) : std::string();

                if (!item.empty())
                {
#ifdef DEBUG
                    SDL_Log("[LOADED LIST %s] [%s]", key, item.c_str());
#endif
                    List.push_back(func(item));
                }
            }
        }
    }

    template <typename T>
    void LoadMapping(nlohmann::json data, const char *key, BloodSwordRogue::UnorderedMap<T, T> &Map, T (*func)(std::string))
    {
        if (!data[key].is_null() && data[key].is_object())
        {
            for (const auto &[k, v] : data[key].items())
            {
                auto map_key = std::string(k);

                auto map_val = std::string(v);
#ifdef DEBUG
                SDL_Log("[LOADED MAP %s] [%s : %s]", key, map_key.c_str(), map_val.c_str());
#endif
                Map[func(map_key)] = func(map_val);
            }
        }
    }

    template <typename T>
    void LoadMapping(nlohmann::json data, const char *key, StringMap<T> &Map, T (*func)(std::string))
    {
        if (!data[key].is_null() && data[key].is_object())
        {
            for (const auto &[k, v] : data[key].items())
            {
                auto map_key = std::string(k);

                auto map_val = std::string(v);
#ifdef DEBUG
                SDL_Log("[LOADED MAP %s] [%s : %s]", key, map_key.c_str(), map_val.c_str());
#endif
                Map[func(map_key)] = map_val;
            }
        }
    }

    // check if string is a number
    bool IsANumber(const std::string &variable)
    {
        // check if first character is a sign (-/+)
        auto offset = (SafeCast(variable.size()) > 1 && (variable[0] == '-' || variable[0] == '+') ? 1 : 0);

        // see: https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
        auto result = !variable.empty() && std::find_if(variable.begin() + offset, variable.end(), [](unsigned char c)
                                                        { return !std::isdigit(c); }) == variable.end();

        SDL_Log("[VARIABLE %s] IS %s A NUMBER", variable.c_str(), (!result ? "NOT" : ""));

        return result;
    }

    // trim whitespace from both ends of the string
    std::string Trim(std::string const &str, std::string const &whitespace = " \r\n\t\v\f")
    {
        auto start = str.find_first_not_of(whitespace);

        auto end = str.find_last_not_of(whitespace);

        return str.substr(start, end - start + 1);
    }
}
