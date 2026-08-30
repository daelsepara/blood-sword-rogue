#pragma once

#include "Generator.hpp"

// functions for managing the party of characters
namespace BloodSwordRogue::Party
{
    // list of characters
    typedef std::vector<Character::Base> List;

    // party base class
    class Base
    {
    private:
        // members in the party
        Party::List Members = {};

    public:
        // current module loaded
        std::string Module = "DEFAULT";

        // variables
        BloodSwordRogue::UnorderedMap<std::string, std::string> Variables = {};

        std::string Location = std::string();

        // location on map: x, y
        int X = -1;

        int Y = -1;

        // default field of view radius
        int FieldOfView = 3;

        Base() {};

        Base(Party::List members) : Members(members) {}

        Base(Character::Base character) : Members(Party::List({character})) {}

        // add character to party
        void Add(Character::Base &character)
        {
            this->Members.push_back(character);
        }

        // number of characters in the party
        int Count()
        {
            return SafeCast(this->Members.size());
        }

        // access party member by index
        Character::Base &operator[](int index)
        {
            if (index < 0 || index > this->Count() - 1)
            {
                auto missing = "CHARACTER: " + std::to_string(index) + " NOT IN PARTY!";

                throw std::invalid_argument(missing);
            }

            return this->Members[index];
        }

        // check if a specific character class is present in party
        Party::List::iterator FindCharacter(Character::Class character_class)
        {
            auto found = this->Members.end();

            for (auto character = this->Members.begin(); character != this->Members.end(); character++)
            {
                if (character->Class == character_class)
                {
                    found = character;

                    break;
                }
            }

            return found;
        }

        // check if party has a member of a specific character class
        bool HasClass(Character::Class character_class)
        {
            return this->FindCharacter(character_class) != this->Members.end();
        }

        // party has characters that are not in standard list (WARRIOR, TRICKSTER, SAGE, ENCHANTER)
        bool HasOthers()
        {
            auto found = false;

            for (auto character = 0; character < SafeCast(this->Members.size()); character++)
            {
                found |= Character::OtherClass(this->Members[character].Class);

                if (found)
                {
                    break;
                }
            }

            return found;
        }

        // returns index of character in party
        int Index(Character::Class character_class)
        {
            auto index = -1;

            for (auto i = 0; i < SafeCast(this->Members.size()); i++)
            {
                if (this->Members[i].Class == character_class)
                {
                    index = i;

                    break;
                }
            }

            return index;
        }

        // find index of party member that is a valid target
        int FindTarget(Target::Type target)
        {
            auto found = -1;

            for (auto i = 0; i < SafeCast(this->Members.size()); i++)
            {
                if (this->Members[i].Target == target)
                {
                    found = i;

                    break;
                }
            }

            return found;
        }

        // check if anyone in the party has this status
        int FindStatus(Character::Status status)
        {
            auto found = -1;

            for (auto i = 0; i < SafeCast(this->Members.size()); i++)
            {
                if (this->Members[i].HasStatus(status))
                {
                    found = i;

                    break;
                }
            }

            return found;
        }

        // check if party has valid target
        bool HasTarget(Target::Type target)
        {
            return (this->FindTarget(target) != -1);
        }

        // search by name
        int FindCharacter(std::string name)
        {
            auto found = -1;

            for (auto i = 0; i < SafeCast(this->Members.size()); i++)
            {
                if ((this->Members[i].Name) == name.c_str())
                {
                    found = i;

                    break;
                }
            }

            return found;
        }

        // has a character with NAME
        bool HasCharacter(std::string name)
        {
            return (this->FindCharacter(name) != -1);
        }

        // check if party has an item of type
        bool HasItemType(Item::Type item)
        {
            auto result = false;

            for (auto character = this->Members.begin(); character != this->Members.end(); character++)
            {
                if (character->HasItemType(item))
                {
                    result = true;

                    break;
                }
            }

            return result;
        }

        // check if party has all items in list
        bool HasAllItems(Items::List items)
        {
            auto result = true;

            for (auto &item : items)
            {
                result &= this->HasItemType(item);

                if (!result)
                {
                    break;
                }
            }

            return result;
        }

        // check if party has charged item of type (charge) with enough quantity
        bool HasChargedItem(Item::Type item, Item::Type charge, int quantity)
        {
            auto result = false;

            for (auto character = this->Members.begin(); character != this->Members.end(); character++)
            {
                if (character->HasChargedItem(item, charge, quantity))
                {
                    result = true;

                    break;
                }
            }

            return result;
        }

        // charge / discharge item
        bool AddCharge(Item::Type item, Item::Type charge, int quantity)
        {
            auto result = false;

            for (auto character = this->Members.begin(); character != this->Members.end(); character++)
            {
                if (character->HasChargedItem(item, charge, quantity < 0 ? -quantity : 0))
                {
                    character->AddCharge(item, charge, quantity);
                }
            }

            return result;
        }

        // check if anyone in the party has this status
        bool HasStatus(Character::Status status)
        {
            return (this->FindStatus(status) != -1);
        }

        // access party by character class
        Character::Base &operator[](Character::Class character_class)
        {
            if (!this->HasClass(character_class))
            {
                auto missing = std::string("CHARACTER: ") + Character::ClassMapping[character_class] + std::string(" NOT IN PARTY!");

                throw std::invalid_argument(missing);
            }

            return (*this)[SafeCast(std::distance(this->Members.begin(), this->FindCharacter(character_class)))];
        }

        // remove character from party (based on index)
        void RemoveCharacter(int index)
        {
            if (index >= 0 && index < this->Count())
            {
                this->Members.erase(this->Members.begin() + index);
            }
        }

        // remove item from everyone
        void RemoveItem(Item::Type item)
        {
            if (item != Item::NONE)
            {
                for (auto character = this->Members.begin(); character != this->Members.end(); character++)
                {
                    if (character->HasItemType(item))
                    {
                        character->RemoveItem(item);
                    }
                }
            }
        }

        // clear party of all members
        void Clear()
        {
            this->Members.clear();
        }

        // clear party of all status
        void ClearStatus()
        {
            for (auto i = 0; i < SafeCast(this->Members.size()); i++)
            {
                this->Members[i].Status.clear();
            }
        }

        // remove character from party (based on character class)
        void RemoveCharacter(Character::Class character_class)
        {
            auto found = -1;

            if (!this->HasClass(character_class))
            {
                auto missing = std::string("CHARACTER: ") + Character::ClassMapping[character_class] + std::string(" NOT IN PARTY!");

                throw std::invalid_argument(missing);
            }

            for (auto i = 0; i < this->Count(); i++)
            {
                auto member = (*this)[i];

                if (member.Class == character_class)
                {
                    found = i;

                    break;
                }
            }

            if (found >= 0 && found < this->Count())
            {
                this->RemoveCharacter(found);
            }
        }

        // add status to entire party
        void AddStatus(Character::Status status)
        {
            for (auto i = 0; i < this->Count(); i++)
            {
                this->Members[i].AddStatus(status);
            }
        }

        // add these statuses to party
        void AddStatus(std::vector<Character::Status> statuses)
        {
            for (auto i = 0; i < this->Count(); i++)
            {
                for (auto &status : statuses)
                {
                    this->Members[i].AddStatus(status);
                }
            }
        }

        // remove status from the entire party
        void RemoveStatus(Character::Status status)
        {
            for (auto i = 0; i < this->Count(); i++)
            {
                this->Members[i].RemoveStatus(status);
            }
        }

        // remove list of status from party
        void RemoveStatus(std::vector<Character::Status> statuses)
        {
            for (auto i = 0; i < this->Count(); i++)
            {
                for (auto &status : statuses)
                {
                    this->Members[i].RemoveStatus(status);
                }
            }
        }

        // reset spell complexities
        void ResetSpells()
        {
            for (auto i = 0; i < this->Count(); i++)
            {
                if (this->Members[i].HasSkill(Skills::Map("SPELLS")))
                {
                    this->Members[i].ResetSpellComplexities();
                }
            }
        }

        // get current location as point
        Point Origin()
        {
            return Point(this->X, this->Y);
        }

        // load party from json data
        void Load(nlohmann::json &data)
        {
            // set module
            this->Module = !data["module"].is_null() ? std::string(data["module"]) : "DEFAULT";

            // load party members
            this->Clear();

            if (!data["members"].is_null() && data["members"].is_array() && SafeCast(data["members"].size()) > 0)
            {
                for (auto i = 0; i < SafeCast(data["members"].size()); i++)
                {
                    auto character = Character::Load(data["members"][i]);

                    this->Add(character);
                }
            }

            this->Variables.clear();

            if (!data["variables"].is_null() && data["variables"].is_object())
            {
                for (auto &[key, val] : data["variables"].items())
                {
                    auto variable = std::string(key);

                    this->Variables[key] = std::string(val);
                }
            }

            this->Location = !data["location"].is_null() ? std::string(data["location"]) : std::string();

            this->X = !data["x"].is_null() ? int(data["x"]) : -1;

            this->Y = !data["y"].is_null() ? int(data["y"]) : -1;

            this->FieldOfView = !data["fov"].is_null() ? int(data["fov"]) : 3;
        }

        // check if string is a number
        bool IsANumber(const std::string &variable)
        {
            return BloodSwordRogue::IsANumber(variable);
        }

        // check if variable is in internal table
        bool IsPresent(std::string variable)
        {
            auto result = !variable.empty() && BloodSwordRogue::Has(this->Variables, variable);

            SDL_Log("[VARIABLE %s] IS %sSENT", variable.c_str(), (result ? "PRE" : "AB"));

            return result;
        }

        // get value of the variable stored in internal table (and/or return literal value)
        std::string GetValue(std::string variable)
        {
            auto value = std::string();

            if (!variable.empty())
            {
                if (variable == "PARTY")
                {
                    value = std::to_string(this->Count());

                    SDL_Log("[PARTY] ---> %s", value.c_str());
                }
                else if (this->IsPresent(variable))
                {
                    value = std::string(this->Variables[variable]);

                    SDL_Log("[VARIABLE %s] ---> %s", variable.c_str(), value.c_str());
                }
                else
                {
                    SDL_Log("[LITERAL] %s", variable.c_str());

                    // may be a number or a string literal
                    value = std::string(variable);
                }
            }

            return value;
        }

        // set variable
        void SetValue(std::string variable, std::string value)
        {
            if (!variable.empty())
            {
                if (!this->IsANumber(variable))
                {
                    this->Variables[variable] = std::string(value);

                    SDL_Log("[VARIABLE %s] <--- %s", variable.c_str(), value.c_str());
                }
            }
        }

        // set numerical value of a variable
        void SetNumber(std::string variable, int value)
        {
            if (!variable.empty())
            {
                if (!this->IsANumber(variable))
                {
                    this->Variables[variable] = std::to_string(value);

                    SDL_Log("[VARIABLE %s] <--- %d", variable.c_str(), value);
                }
            }
        }

        // get numeric value of a variable
        int GetNumber(std::string variable)
        {
            auto value = 0;

            if (!variable.empty())
            {
                if (variable == "PARTY")
                {
                    value = this->Count();
                }
                else if (this->IsANumber(variable))
                {
                    value = std::stoi(variable, nullptr, 10);
                }
                else
                {
                    auto search = this->GetValue(variable);

                    value = (!search.empty() && this->IsANumber(search)) ? std::stoi(search, nullptr, 10) : 0;
                }
            }

            SDL_Log("[NUMBER %s] ---> %d", variable.c_str(), value);

            return value;
        }

        // check if item is in the list
        bool IsValid(std::vector<std::string> list, std::string item)
        {
            auto result = BloodSwordRogue::Has(list, item);

            SDL_Log("[CHECK %s] IS %s", item.c_str(), (result ? "VALID" : "INVALID"));

            return result;
        }

        // math operations
        void Math(std::string operation, std::string first, std::string second, bool clamp = true)
        {
            // first = (first) (operation) (second)
            if (!operation.empty() && !first.empty() && !this->IsANumber(first) && !second.empty())
            {
                // check if operation is valid
                if (this->IsValid({"+", "-", "*"}, operation))
                {
                    if (!this->IsPresent(first) && !this->IsANumber(first))
                    {
                        // initialize first variable if not present
                        this->SetNumber(first, 0);
                    }

                    if ((!this->IsANumber(first) || this->IsANumber(this->GetValue(first))) && (this->IsANumber(second) || this->IsANumber(this->GetValue(second))))
                    {
                        auto value_first = this->GetNumber(first);

                        auto value_second = this->GetNumber(second);

                        if (operation == "+")
                        {
                            value_first += value_second;
                        }
                        else if (operation == "-")
                        {
                            value_first -= value_second;
                        }
                        else if (operation == "*")
                        {
                            value_first *= value_second;
                        }

                        value_first = clamp ? std::max(0, value_first) : value_first;

                        SDL_Log("[MATH] %s %s %s = %d", first.c_str(), operation.c_str(), second.c_str(), value_first);

                        // set variable
                        this->SetNumber(first, value_first);
                    }
                }
            }
        }

        // logical operations (on non-numeric)
        bool Is(std::string operation, std::string first, std::string second)
        {
            auto result = false;

            // (first) (logical operiation) (second)
            if (!operation.empty() && !first.empty() && !second.empty())
            {
                // check if operation is valid
                if (this->IsValid({"=", "!=", "<>"}, operation))
                {
                    auto value_first = std::string(this->GetValue(first));

                    auto value_second = std::string(this->GetValue(second));

                    if (operation == "=")
                    {
                        result = (value_first == value_second.c_str());
                    }
                    else if (operation == "!=" || operation == "<>")
                    {
                        result = !(value_first == value_second.c_str());
                    }

                    SDL_Log("[IF] %s %s %s IS %s", first.c_str(), operation.c_str(), second.c_str(), (result ? "TRUE" : "FALSE"));
                }
            }

            return result;
        }

        // logical operations
        bool If(std::string operation, std::string first, std::string second)
        {
            auto result = false;

            // (first) (logical operiation) (second)
            if (!operation.empty() && !first.empty() && !second.empty())
            {
                // check if operation is valid
                if (this->IsValid({"=", "!=", "<>", "<", ">", "<=", ">="}, operation))
                {
                    // check if both are numbers and/or resolve to numbers
                    if ((!this->IsANumber(this->GetValue(first)) && !this->IsANumber(first)) || (!this->IsANumber(this->GetValue(second)) && !this->IsANumber(second)))
                    {
                        return this->Is(operation, first, second);
                    }

                    auto value_first = this->GetNumber(first);

                    auto value_second = this->GetNumber(second);

                    if (operation == "=")
                    {
                        result = (value_first == value_second);
                    }
                    else if (operation == "!=" || operation == "<>")
                    {
                        result = (value_first != value_second);
                    }
                    else if (operation == "<")
                    {
                        result = (value_first < value_second);
                    }
                    else if (operation == "<=")
                    {
                        result = (value_first <= value_second);
                    }
                    else if (operation == ">")
                    {
                        result = (value_first > value_second);
                    }
                    else if (operation == ">=")
                    {
                        result = (value_first > value_second);
                    }

                    SDL_Log("[IF] %s %s %s IS %s", first.c_str(), operation.c_str(), second.c_str(), (result ? "TRUE" : "FALSE"));
                }
            }

            return result;
        }

        // erase variable
        void RemoveVariable(std::string variable)
        {
            if (this->IsPresent(variable))
            {
                this->Variables.erase(variable);

                if (!this->IsPresent(variable))
                {
                    SDL_Log("[VARIABLE %s] REMOVED", variable.c_str());
                }
            }
        }
    };

    // other characters (in the book that can be added to the party)
    Party::Base Characters = Party::Base();

    // initialize party from json data
    Party::Base Initialize(nlohmann::json &data)
    {
        auto party = Party::Base();

        if (!data.is_null())
        {
            party.Load(data);
        }

        return party;
    }

    // get party data as json
    nlohmann::json Data(Party::Base &party)
    {
        nlohmann::json data;

        data["module"] = !party.Module.empty() ? party.Module : "DEFAULT";

        if (party.Count() > 0)
        {
            nlohmann::json members;

            for (auto character = 0; character < party.Count(); character++)
            {
                auto character_data = Character::Data(party[character]);

                members.push_back(character_data);
            }

            data["members"] = members;
        }

        if (SafeCast(party.Variables.size()) > 0)
        {
            nlohmann::json variables;

            for (auto &variable : party.Variables)
            {
                variables.emplace(variable.first, variable.second);
            }

            data["variables"] = variables;
        }

        data["location"] = party.Location;

        data["x"] = party.X;

        data["y"] = party.Y;

        data["fov"] = party.FieldOfView;

        return data;
    }

    // save party to file
    void Save(Party::Base &party, const char *filename, const char *name)
    {
        nlohmann::json data;

        data.emplace(std::string(name), Party::Data(party));

        std::ofstream ifs(filename);

        if (ifs.is_open())
        {
            ifs << data.dump();

            ifs.close();
        }
    }

    // load party from zip archive
    Party::Base Load(const char *filename, const char *name, const char *zip_file)
    {
        auto party = Party::Base();

        auto ifs = zip_file != nullptr ? ZipFile::Read(zip_file, filename) : Read(filename);

        if (!ifs.empty())
        {
            auto data = nlohmann::json::parse(ifs);

            party = Party::Initialize(data[std::string(name)]);

            ifs.clear();

            SDL_Log("[LOADED] %d characters", party.Count());
        }

        return party;
    }

    // load named party from file
    Party::Base Load(std::string filename, std::string name)
    {
        return Party::Load(filename.c_str(), name.c_str(), nullptr);
    }

    // load named party from zip archive
    Party::Base Load(std::string filename, std::string name, std::string zip_file)
    {
        return Party::Load(filename.c_str(), name.c_str(), zip_file.empty() ? nullptr : zip_file.c_str());
    }
}
