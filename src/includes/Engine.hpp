#pragma once

#include "Map.hpp"
#include "Move.hpp"
#include "Party.hpp"
#include "Random.hpp"

// functions and classes for handling gameplay logic
namespace BloodSwordRogue::Engine
{
    // queue element
    class ScoreElement
    {
    public:
        // character is PLAYER or NPC
        Character::ControlType Type = Character::ControlType::NONE;

        // character ID within the party
        int Id = -1;

        // score (used in ranking)
        int Score = 0;

        ScoreElement(Character::ControlType type, int id, int score) : Type(type), Id(id), Score(score) {}
    };

    // queue data type
    typedef std::vector<Engine::ScoreElement> Queue;

    // roll stages
    enum class RollStage
    {
        START,
        RESULT
    };

    // vulnerability scaler
    const int VulnerabilityScaler = 4;

    // RNG engine
    auto Random = Random::Base();

    auto Percentile = Random::Base();

    // dice type
    const int Dice = 6;

    // initialize RNG
    void InitializeRNG()
    {
        // fix integer RNG to 1D6
        Random.UniformIntDistribution(1, Engine::Dice);

        // fix floating point RNG to [0.0, 1.0)
        Random.UniformDistribution(0, 1.0);

        // fix integer RNG to 0-100
        Percentile.UniformIntDistribution(0, 100);

        // fix floating point RNG to [0.0, 1.0)
        Percentile.UniformDistribution(0, 1.0);
    }

    // stores individual roll results and sum
    class RollResult
    {
    public:
        std::vector<int> Rolls = {};

        int Sum = 0;

        RollResult(std::vector<int> rolls, int sum) : Rolls(rolls), Sum(sum) {}

        // empty result
        RollResult() {}
    };

    // roll number of dice and include modifiers
    Engine::RollResult Roll(int count, int modifier)
    {
        auto rolls = std::vector<int>();

        auto sum = 0;

        for (auto i = 0; i < count; i++)
        {
            rolls.push_back(Engine::Random.NextInt());

            sum += rolls.back();
        }

        sum += modifier;

        return Engine::RollResult(rolls, std::max(0, sum));
    }

    // roll number of dice
    Engine::RollResult Roll(int count)
    {
        return Engine::Roll(count, 0);
    }

    // total character score
    int Score(Character::Base &character, Attribute::Type attribute, bool in_battle = false, Item::Property weapon = Item::NONE)
    {
        auto score = 0;

        auto value = std::min(character.Value(attribute), character.Maximum(attribute));

        auto modifier = character.Modifier(attribute) + character.Modifiers(attribute);

        if (attribute == Attribute::Type::ARMOUR)
        {
            value = modifier;

            modifier = 0;
        }
        else if (attribute == Attribute::Type::FIGHTING_PROWESS)
        {
            if (in_battle)
            {
                if (character.IsPlayer() && !character.HasArmedWeapon() && character.Fight == Skills::NONE)
                {
                    modifier -= 2;
                }
                else if (character.IsEnemy() && character.Fight == Skills::Map("BROKEN WEAPON"))
                {
                    modifier -= 2;
                }

                if (character.HasStatus(Character::Status::BURNED))
                {
                    modifier -= 1;
                }

                if (character.HasStatus(Character::Status::FIGHTING_BLIND))
                {
                    value = 1;

                    modifier = 0;
                }
            }

            if (character.HasStatus(Character::Status::OSTEAL_WEAKNESS))
            {
                modifier -= 1;
            }
        }
        else if (attribute == Attribute::Type::AWARENESS)
        {
            if (character.HasStatus(Character::Status::OSTEAL_WEAKNESS))
            {
                modifier -= 1;
            }
        }
        else if (attribute == Attribute::Type::DAMAGE && (!character.HasArmedWeapon() || character.Fight == Skills::Map("BROKEN WEAPON")))
        {
            modifier -= 2;
        }

        if (character.HasArmedWeapon() && weapon != Item::NONE)
        {
            modifier += character.WeaponModifier(weapon, attribute);
        }

        score = value + modifier;

        return (in_battle && character.HasArmedWeapon() && weapon != Item::NONE) ? score : std::max(0, score);
    }

    // check if character is still alive (and not enthralled)
    bool IsAlive(Character::Base &character)
    {
        return Engine::Score(character, Attribute::Type::ENDURANCE) > 0;
    }

    // count characters in party who are alive
    int Count(Party::Base &party)
    {
        auto live = 0;

        for (auto character = 0; character < party.Count(); character++)
        {
            if (!Character::OtherClass(party[character].Class) || party[character].ControlType == Character::ControlType::NPC)
            {
                live += Engine::IsAlive(party[character]) ? 1 : 0;
            }
        }

        return live;
    }

    bool IsAlive(Party::Base &party)
    {
        return Engine::Count(party) > 0;
    }

    // count number of characters with IN BATTLE status in the party
    int InBattle(Party::Base &party)
    {
        auto in_battle = 0;

        for (auto character = 0; character < party.Count(); character++)
        {
            if (Engine::IsAlive(party[character]) && !Character::OtherClass(party[character].Class))
            {
                auto is_away = party[character].IsStatus(Character::Status::AWAY) && (party[character].Status[Character::Status::AWAY] < 0);

                auto is_paralyzed = party[character].IsStatus(Character::Status::PARALYZED);

                in_battle += (!is_away && !is_paralyzed) ? 1 : 0;
            }
        }

        return in_battle;
    }

    // count characters (control type) with specific [STATUS] in the party
    int Count(Party::Base &party, Character::ControlType control, Character::Status status)
    {
        auto count = 0;

        for (auto character = 0; character < party.Count(); character++)
        {
            auto status_count = party[character].ControlType == control && party[character].IsStatus(status);

            auto normal_count = party[character].ControlType == Character::ControlType::PLAYER && !Character::OtherClass(party[character].Class);

            auto enemy_count = party[character].ControlType == Character::ControlType::NPC;

            count += (status_count && (normal_count || enemy_count)) ? 1 : 0;
        }

        return count;
    }

    // counts live characters with skill
    int CountSkills(Party::Base &party, Skills::Type skill)
    {
        auto count = 0;

        for (int i = 0; i < party.Count(); i++)
        {
            count += (Engine::IsAlive(party[i]) && party[i].HasSkill(skill)) ? 1 : 0;
        }

        return count;
    }

    // counts number of particular items among live members in the party
    int CountItems(Party::Base &party, Item::Type item)
    {
        auto count = 0;

        for (int character = 0; character < party.Count(); character++)
        {
            count += (Engine::IsAlive(party[character]) && party[character].HasItemType(item)) ? party[character].CountItems(item) : 0;
        }

        return count;
    }

    // counts live characters without skill
    int Without(Party::Base &party, Skills::Type skill)
    {
        auto count = 0;

        for (int i = 0; i < party.Count(); i++)
        {
            count += (Engine::IsAlive(party[i]) && !party[i].HasSkill(skill)) ? 1 : 0;
        }

        return count;
    }

    // count quantities of an item within the party
    int Quantity(Party::Base &party, Item::Type item)
    {
        auto quantity = 0;

        for (auto i = 0; i < party.Count(); i++)
        {
            if (Engine::IsAlive(party[i]))
            {
                quantity += party[i].Quantity(item);
            }
        }

        return quantity;
    }

    // check if there is at least one character in the party still alive
    bool IsAlive(Party::Base &party, Character::ControlType control_type)
    {
        auto live = Engine::Count(party);

        auto enthralled = Engine::Count(party, control_type, Character::Status::ENTHRALLED);

        auto paralyzed = Engine::Count(party, control_type, Character::Status::PARALYZED);

        return live > 0 && live > enthralled && (live > paralyzed);
    }

    // check if the entire party is fleeing
    bool IsFleeing(Party::Base &party)
    {
        auto live = 0;

        auto fleeing = 0;

        for (auto character = 0; character < party.Count(); character++)
        {
            if (party[character].ControlType == Character::ControlType::PLAYER && Engine::IsAlive(party[character]) && !Character::OtherClass(party[character].Class))
            {
                fleeing += party[character].IsStatus(Character::Status::FLEEING) ? 1 : 0;

                // paralyzed players are "dead"
                live += !party[character].IsStatus(Character::Status::PARALYZED) ? 1 : 0;
            }
        }

        return live > 0 && fleeing > 0 && fleeing >= live;
    }

    // sort queue
    void Sort(Engine::Queue &queue, bool descending = true)
    {
        // sort based on direction
        if (descending)
        {
            std::sort(queue.begin(), queue.end(), [](ScoreElement &a, ScoreElement &b) -> bool
                      { return a.Score > b.Score; });
        }
        else
        {
            std::sort(queue.begin(), queue.end(), [](ScoreElement &a, ScoreElement &b) -> bool
                      { return a.Score < b.Score; });
        }
    }

    // check if character can be a target
    bool CanTarget(Character::Base &character, bool in_battle)
    {
        auto is_away = character.IsStatus(Character::Status::AWAY);

        auto is_paralyzed = character.IsStatus(Character::Status::PARALYZED);

        auto is_alive = Engine::IsAlive(character);

        auto battle = (in_battle && character.IsStatus(Character::Status::IN_BATTLE)) || !in_battle;

        return (is_alive && !is_away && !is_paralyzed && battle);
    }

    // count number of active combatants in the party
    int Combatants(Party::Base &party, bool in_battle = false)
    {
        auto live = 0;

        for (auto character = 0; character < party.Count(); character++)
        {
            live += (Engine::CanTarget(party[character], in_battle) && !party[character].IsStatus(Character::Status::EXCLUDED) && !Character::OtherClass(party[character].Class)) ? 1 : 0;
        }

        return live;
    }

    // return minimum value of an [ATTRIBUTE] in party
    int Min(Party::Base &party, Attribute::Type attribute, bool in_battle = false)
    {
        auto min_value = 255;

        if (attribute != Attribute::Type::NONE)
        {
            for (auto character = 0; character < party.Count(); character++)
            {
                if (Engine::CanTarget(party[character], in_battle))
                {
                    min_value = std::min(Engine::Score(party[character], attribute, in_battle), min_value);
                }
            }
        }
        else
        {
            min_value = 0;
        }

        return min_value;
    }

    // return maximum value of an [ATTRIBUTE] in party
    int Max(Party::Base &party, Attribute::Type attribute, bool in_battle = false)
    {
        auto max_value = 0;

        if (attribute != Attribute::Type::NONE)
        {
            for (auto character = 0; character < party.Count(); character++)
            {
                if (Engine::CanTarget(party[character], in_battle))
                {
                    max_value = std::max(Engine::Score(party[character], attribute, in_battle), max_value);
                }
            }
        }

        return max_value;
    }

    // build targetting queue
    void Build(Engine::Queue &queue, Party::Base &party, Attribute::Type attribute, Character::Status status = Character::Status::NONE, bool in_battle = false)
    {
        // add characters in party to queue
        for (auto i = 0; i < party.Count(); i++)
        {
            auto knocked_out = party[i].IsStatus(Character::Status::KNOCKED_OUT);

            if (Engine::CanTarget(party[i], in_battle) && (status == Character::Status::NONE || party[i].HasStatus(status)))
            {
                queue.push_back(ScoreElement(party[i].ControlType, i, knocked_out ? 1 : Engine::Score(party[i], attribute, in_battle)));
            }
        }
    }

    // creates queue (order sequence of characters with status ranked according to attribute score)
    Engine::Queue Build(Party::Base &party, Party::Base &other, Attribute::Type attribute, Character::Status status, bool in_battle = false, bool descending = true)
    {
        auto queue = Engine::Queue();

        // add characters in party to queue
        Engine::Build(queue, party, attribute, status, in_battle);

        // add characters from the other party to the queue
        Engine::Build(queue, other, attribute, status, in_battle);

        // sort queue
        Engine::Sort(queue, descending);

        return queue;
    }

    // build queue of characters sorted according to attribute score
    Engine::Queue Build(Party::Base &party, Attribute::Type attribute, Character::Status status, bool in_battle = false, bool descending = true)
    {
        auto empty = Party::Base();

        return Engine::Build(party, empty, attribute, status, in_battle, descending);
    }

    // build targetting queue
    void Build(Engine::Queue &queue, Party::Base &party, Attribute::Type attribute, bool in_battle = false)
    {
        return Engine::Build(queue, party, attribute, Character::Status::NONE, in_battle);
    }

    Engine::Queue Build(Party::Base &party, Attribute::Type attribute, Skills::Type skill, bool in_battle = false, bool descending = true)
    {
        auto queue = Engine::Queue();

        for (auto i = 0; i < party.Count(); i++)
        {
            auto knocked_out = party[i].IsStatus(Character::Status::KNOCKED_OUT);

            auto skilled = party[i].HasSkill(skill);

            if (Engine::CanTarget(party[i], in_battle) && skilled)
            {
                queue.push_back(ScoreElement(party[i].ControlType, i, knocked_out ? 1 : Engine::Score(party[i], attribute, in_battle)));
            }
        }

        return queue;
    }

    // creates queue (order sequence of characters ranked according to attribute score)
    Engine::Queue Build(Party::Base &party, Party::Base &other, Attribute::Type attribute, bool in_battle = false, bool descending = true)
    {
        auto queue = Engine::Queue();

        // add characters in party to queue
        Engine::Build(queue, party, attribute, in_battle);

        // add characters from the other party to the queue
        Engine::Build(queue, other, attribute, in_battle);

        // sort queue
        Engine::Sort(queue, descending);

        return queue;
    }

    // build queue of characters sorted according to attribute score
    Engine::Queue Build(Party::Base &party, Attribute::Type attribute, bool in_battle = false, bool descending = true)
    {
        auto empty = Party::Base();

        return Engine::Build(party, empty, attribute, in_battle, descending);
    }

    // search for a character in the queue
    Engine::Queue::iterator Find(Engine::Queue &queue, Character::ControlType control, int id)
    {
        auto found = queue.end();

        for (auto characters = queue.begin(); characters != queue.end(); characters++)
        {
            auto &character = *characters;

            if (character.Type == control && character.Id == id)
            {
                found = characters;

                break;
            }
        }

        return found;
    }

    // return the position of the character in the queue (-1 if character not in queue)
    int Order(Engine::Queue &queue, Character::ControlType &control, int id)
    {
        auto distance = -1;

        auto found = Engine::Find(queue, control, id);

        if (found != queue.end())
        {
            distance = SafeCast(std::distance(queue.begin(), found));
        }

        return distance;
    }

    // check if current character (in the queue) is player-controlled
    bool IsPlayer(Engine::Queue &queue, int &character)
    {
        return queue[character].Type == Character::ControlType::PLAYER;
    }

    // check if current character (in the queue) is non-player-controlled
    bool IsEnemy(Engine::Queue &queue, int &character)
    {
        return !Engine::IsPlayer(queue, character);
    }

    // move to next item in queue
    bool NextInQueue(Engine::Queue &queue, int &item)
    {
        auto first = false;

        item++;

        if (item >= SafeCast(queue.size()))
        {
            item = 0;

            first = true;
        }

        return first;
    }

    // move to previous item in queue
    void Previous(Engine::Queue &queue, int &item)
    {
        item--;

        if (item < 0)
        {
            item = SafeCast(queue.size()) - 1;
        }
    }

    // move character to end of the queue
    void MoveToEnd(Engine::Queue &queue, Character::ControlType control, int id)
    {
        auto search = Engine::Find(queue, control, id);

        if (search != queue.end())
        {
            std::rotate(search, std::next(search), queue.end());
        }
    }

    // move item to end of the queue
    void MoveToEnd(Engine::Queue &queue, int id)
    {
        if (id >= 0 && id < SafeCast(queue.size()) - 1)
        {
            auto search = queue.begin() + id;

            std::rotate(search, std::next(search), queue.end());
        }
    }

    // find character (control type, id) in map
    Point Location(Map::Base &map, Character::Base &character, int id)
    {
        Point location;

        if (character.ControlType == Character::ControlType::PLAYER)
        {
            location = map.Find(Map::Object::PLAYER, id);
        }
        else if (character.ControlType == Character::ControlType::NPC)
        {
            location = map.Find(Map::Object::ENEMY, id);
        }

        return location;
    }

    // find character in party, return index
    int Find(Party::Base &party, Character::Base &character)
    {
        return party.Index(character.Class);
    }

    // generic queue builder (based on distance / endurance)
    Engine::Queue Build(Map::Base &map, Party::Base &party, Point &src, bool in_battle, bool fight, bool ranged, bool move, bool spell, bool descending = false)
    {
        auto queue = Engine::Queue();

        for (auto i = 0; i < party.Count(); i++)
        {
            if (Engine::CanTarget(party[i], in_battle))
            {
                auto location = Engine::Location(map, party[i], i);

                auto distance = -1;

                if (move)
                {
                    auto path = Move::FindPath(map, src, location);

                    distance = Move::Count(map, path, map[src].IsEnemy());

                    if (SafeCast(path.Points.size()) == 0)
                    {
                        // path to target is blocked, set arbitrarily large distance
                        distance = 9999;
                    }

                    if (map[src].IsEnemy())
                    {
                        auto vulnerability = map.Free(location);

                        // add vulnerability score (more empty spaces, more vulnerable)
                        distance += (SafeCast(Map::Directions.size()) - vulnerability) * VulnerabilityScaler;
                    }
                }
                else
                {
                    distance = map.Distance(src, location);

                    if (map[src].IsEnemy())
                    {
                        auto action = std::string();

                        if (spell)
                        {
                            action = "SPELL";
                        }
                        else if (ranged)
                        {
                            action = "RANGED";
                        }
                        else
                        {
                            action = "FIGHT";
                        }
                    }
                }

                if (map.IsValid(location) && location != src)
                {
                    if ((move && distance > 0) || (ranged && distance > 1))
                    {
                        queue.push_back(Engine::ScoreElement(party[i].ControlType, i, distance));
                    }
                    else if ((!move && !ranged && fight && distance == 1) || (!move && !ranged && !fight && spell))
                    {
                        queue.push_back(Engine::ScoreElement(party[i].ControlType, i, Engine::Score(party[i], Attribute::Type::ENDURANCE, in_battle)));
                    }
                }
            }
        }

        Engine::Sort(queue, descending);

        return queue;
    }

    // build queue based on path to target (alternative to the distance-based (between src and dst) approach
    Engine::Queue MoveTargets(Map::Base &map, Party::Base &party, Point &src, bool in_battle, bool descending = false)
    {
        return Engine::Build(map, party, src, in_battle, false, false, true, false, descending);
    }

    // build shot targets
    Engine::Queue RangedTargets(Map::Base &map, Party::Base &party, Point &src, bool in_battle = false, bool descending = false)
    {
        return Engine::Build(map, party, src, in_battle, false, true, false, false, descending);
    }

    // build fight targets (adjacent)
    Engine::Queue FightTargets(Map::Base &map, Party::Base &party, Point &src, bool in_battle = false, bool descending = false)
    {
        return Engine::Build(map, party, src, in_battle, true, false, false, false, descending);
    }

    // build spell targets
    Engine::Queue SpellTargets(Map::Base &map, Party::Base &party, Point &src, bool in_battle = false, bool descending = false)
    {
        return Engine::Build(map, party, src, in_battle, false, false, false, true, descending);
    }

    // build targets sort by distance
    Engine::Queue AllTargets(Map::Base &map, Party::Base &party, Point &src, bool in_battle = false, bool descending = false)
    {
        auto queue = Engine::Queue();

        for (auto i = 0; i < party.Count(); i++)
        {
            if (Engine::CanTarget(party[i], in_battle))
            {
                auto location = Engine::Location(map, party[i], i);

                auto distance = map.Distance(src, location);

                queue.push_back(Engine::ScoreElement(party[i].ControlType, i, distance));
            }
        }

        Engine::Sort(queue, descending);

        return queue;
    }

    // build queue of preferred targets
    void Targets(Engine::Queue &queue, Map::Base &map, Party::Base &opponents, Character::Base &character, Point &src, bool in_party = true, bool in_battle = false, bool ranged = false, bool move = false, bool spell = false)
    {
        if (map.IsValid(src))
        {
            auto is_enemy = character.IsEnemy();

            auto id = map[src].Id;

            for (auto &target : character.Targets)
            {
                for (auto i = 0; i < opponents.Count(); i++)
                {
                    if ((opponents[i].Target == target) || (!in_party && target == Target::Map("PLAYER")))
                    {
                        auto add_target = true;

                        if (((in_party && id != i) || !in_party) && Engine::CanTarget(opponents[i], in_battle))
                        {
                            auto location = Engine::Location(map, opponents[i], i);

                            auto distance = Map::Unlimited;

                            auto prob = Engine::Percentile.NextInt();

                            if (move)
                            {
                                auto path = Move::FindPath(map, src, location, (in_party ? i : Map::NotFound));

                                distance = Move::Count(map, path, is_enemy, (in_party ? i : Map::Unlimited));

                                if (SafeCast(path.Points.size()) == 0)
                                {
                                    // path to target is blocked, set arbitrarily large distance
                                    distance = 9999;
                                }

                                if (is_enemy)
                                {
                                    auto vulnerability = map.Free(location);

                                    // add vulnerability score (more empty spaces, more vulnerable)
                                    distance += (SafeCast(Map::Directions.size()) - vulnerability) * VulnerabilityScaler;
                                }
                            }
                            else
                            {
                                distance = map.Distance(src, location);

                                auto action = std::string();

                                if (spell)
                                {
                                    action = "SPELL";
                                }
                                else if (ranged)
                                {
                                    action = "RANGED";
                                }
                                else
                                {
                                    action = "FIGHT";
                                }

                                if ((is_enemy && in_party && (character.TargetProbability > 0 && character.TargetProbability <= 100)) || (ranged && is_enemy && in_party))
                                {
                                    add_target = (prob < character.TargetProbability);
                                }
                            }

                            if (map.IsValid(location) && location != src)
                            {
                                // preserve scoring for future uses
                                if (spell || (!ranged && !move && distance == 1))
                                {
                                    queue.push_back(Engine::ScoreElement(opponents[i].ControlType, i, Engine::Score(opponents[i], Attribute::Type::ENDURANCE, in_battle)));
                                }
                                else if (((ranged && distance > 1) || (move && distance > 0)) && add_target)
                                {
                                    queue.push_back(Engine::ScoreElement(opponents[i].ControlType, i, distance));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // search preferred fight targets
    Engine::Queue FightTargets(Map::Base &map, Party::Base &party, Party::Base &opponents, Point &src, bool in_battle = false)
    {
        auto queue = Engine::Queue();

        if (map.IsValid(src))
        {
            auto id = map[src].Id;

            auto &character = party[id];

            // search within party
            Engine::Targets(queue, map, party, character, src, true, in_battle);

            // search within the other party
            Engine::Targets(queue, map, opponents, character, src, false, in_battle);
        }

        return queue;
    }

    // search preferred ranged targets
    Engine::Queue RangedTargets(Map::Base &map, Party::Base &mine, Party::Base &other, Point &src, bool in_battle = false)
    {
        auto queue = Engine::Queue();

        if (map.IsValid(src))
        {
            auto id = map[src].Id;

            auto &character = mine[id];

            // search within party
            Engine::Targets(queue, map, mine, character, src, true, in_battle, true);

            // search within the other party
            Engine::Targets(queue, map, other, character, src, false, in_battle, true);
        }

        return queue;
    }

    // search preferred move targets
    Engine::Queue MoveTargets(Map::Base &map, Party::Base &party, Party::Base &opponents, Point &src, bool in_battle = false)
    {
        auto queue = Engine::Queue();

        if (map.IsValid(src))
        {
            auto id = map[src].Id;

            auto &character = party[id];

            // search within party
            Engine::Targets(queue, map, party, character, src, true, in_battle, false, true);

            // search within the other party
            Engine::Targets(queue, map, opponents, character, src, false, in_battle, false, true);
        }

        // sort according to distance
        Engine::Sort(queue, false);

        return queue;
    }

    // check if control is a spell
    bool IsSpell(Controls::Type type)
    {
        return BloodSwordRogue::Has(Controls::Spells, type);
    }

    // character gains/loses endurance
    bool GainEndurance(Character::Base &character, int gain, bool in_battle = false)
    {
        auto endurance = Engine::Score(character, Attribute::Type::ENDURANCE, in_battle);

        endurance += gain;

        endurance = std::max(0, endurance);

        character.SetAttribute(Attribute::Type::ENDURANCE, endurance, 0);

        return Engine::IsAlive(character);
    }

    // character gains experience
    void GainExperience(Character::Base &character, int gain)
    {
        if (Engine::IsAlive(character))
        {
            character.Experience += gain;
        }
    }

    // party gains experience
    void GainExperience(Party::Base &party, int gain)
    {
        for (auto character = 0; character < party.Count(); character++)
        {
            Engine::GainExperience(party[character], gain);
        }
    }

    // modify character's [ATTRIBUTE] by adding gain (+/-)
    void ModifyAttribute(Character::Base &character, Attribute::Type attribute, int gain)
    {
        if ((gain > 0 || (gain < 0 && character.Value(attribute) > (1 - gain) && character.Maximum(attribute) > (1 - gain))) && attribute != Attribute::Type::NONE)
        {
            auto max_value = character.Maximum(attribute);

            auto value = character.Value(attribute);

            character.Maximum(attribute, max_value + gain);

            character.Value(attribute, value + gain);
        }
    }

    // modify party's [ATTRIBUTE] by adding gain (+/-)
    void ModifyAttribute(Party::Base &party, Attribute::Type attribute, int gain)
    {
        for (auto character = 0; character < party.Count(); character++)
        {
            if (Engine::IsAlive(party[character]))
            {
                Engine::ModifyAttribute(party[character], attribute, gain);
            }
        }
    }

    // set character ATTRIBUTE to maximum
    void MaximiseAttribute(Character::Base &character, Attribute::Type attribute)
    {
        if (attribute != Attribute::Type::NONE)
        {
            auto max_value = character.Maximum(attribute);

            character.Value(attribute, max_value);
        }
    }

    // set character ATTRIBUTE to maximum
    void MaximizeAttribute(Character::Base &character, Attribute::Type attribute)
    {
        Engine::MaximiseAttribute(character, attribute);
    }

    // set party's ATTRIBUTE to maximum
    void MaximiseAttribute(Party::Base &party, Attribute::Type attribute)
    {
        for (auto character = 0; character < party.Count(); character++)
        {
            Engine::MaximiseAttribute(party[character], attribute);
        }
    }

    // set party's ATTRIBUTE to maximum
    void MaximizeAttribute(Party::Base &party, Attribute::Type attribute)
    {
        Engine::MaximiseAttribute(party, attribute);
    }

    // change [FROM STATUS] to [TO STATUS]
    bool Transition(Character::Base &character, Character::Status from, Character::Status to)
    {
        auto update = false;

        if (character.IsStatus(from))
        {
            character.RemoveStatus(from);

            character.AddStatus(to);

            update = true;
        }

        return update;
    }

    // cooldown specific status
    bool CoolDown(Character::Base &character, Character::Status status)
    {
        auto update = false;

        if (character.HasStatus(status))
        {
            auto duration = character.Status[status];

            if (duration < 0)
            {
                duration = -1;
            }
            else if (duration > 0)
            {
                duration--;
            }

            if (duration != 0)
            {
                character.Status[status] = duration;
            }
            else
            {
                character.RemoveStatus(status);
            }

            update = true;
        }

        return update;
    }

    // cooldown status effects
    bool CoolDown(Character::Base &character)
    {
        auto update = false;

        auto active = SafeCast(character.Status.size());

        if (character.IsStatus(Character::Status::DEFENDING))
        {
            update = Engine::Transition(character, Character::Status::DEFENDING, Character::Status::DEFENDED);
        }
        else
        {
            auto status_types = std::vector<Character::Status>();

            // get all status
            for (auto &status : character.Status)
            {
                status_types.push_back(status.first);
            }

            // cooldown each status
            for (auto &status : status_types)
            {
                if (!BloodSwordRogue::In(Character::GlobalStatus, status))
                {
                    update |= Engine::CoolDown(character, status);
                }
            }

            if (active != SafeCast(character.Status.size()))
            {
                update = true;
            }
        }

        return update;
    }

    // check if character can flee
    bool CanFlee(Map::Base &map, Party::Base &party, int character)
    {
        auto flee = false;

        auto src = map.Find(Map::Object::PLAYER, character);

        if (!src.IsNone())
        {
            if (map[src].IsExit())
            {
                flee = true;
            }
            else
            {
                for (auto &direction : Map::Directions)
                {
                    auto neighbor = src + direction;

                    auto adjacent = false;

                    if (map.IsValid(neighbor) && map[neighbor].IsPlayer())
                    {
                        auto id = map[neighbor].Id;

                        adjacent = (Engine::IsAlive(party[id]) && party[id].IsStatus(Character::Status::FLEEING));
                    }

                    if ((map.IsValid(neighbor) && map[neighbor].Type == Map::Object::EXIT && !map[neighbor].IsEnemy() && !map[neighbor].IsTemporarilyBlocked()) || adjacent)
                    {
                        flee = true;

                        break;
                    }
                }
            }
        }

        return flee;
    }

    // check if character can shoot
    bool CanShoot(Character::Base &character, Skills::Type shot)
    {
        auto can_shoot = shot != Skills::NONE && character.HasSkill(shot) && Skills::IsRangedAttack(shot);

        if (shot == Skills::Map("ARCHERY"))
        {
            can_shoot &= (character.IsArmedCharged(Item::MapType("BOW"), Item::Requirements(Item::MapType("BOW"), true)) || character.IsArmedCharged(Item::MapType("MAGIC BOW"), Item::Requirements(Item::MapType("MAGIC BOW"), true)));
        }
        else if (shot == Skills::Map("SHURIKEN") && character.HasItemType(Item::MapType("LIMITED SHURIKEN")))
        {
            can_shoot &= character.IsArmedCharged(Item::MapType("LIMITED SHURIKEN"), Item::Requirements(Item::MapType("LIMITED SHURIKEN"), true));
        }

        return can_shoot;
    }

    // checks if a character can shoot and is properly armed
    bool CanShoot(Character::Base &character)
    {
        return Engine::CanShoot(character, character.Shoot);
    }

    // build a queue of shooters
    Engine::Queue Shooters(Party::Base &party, Attribute::Type attribute, bool in_battle = false, bool descending = false)
    {
        auto queue = Engine::Queue();

        // add characters in party to queue
        for (auto i = 0; i < party.Count(); i++)
        {
            auto knocked_out = party[i].IsStatus(Character::Status::KNOCKED_OUT);

            if (Engine::CanTarget(party[i], in_battle) && Engine::CanShoot(party[i]))
            {
                queue.push_back(ScoreElement(party[i].ControlType, i, knocked_out ? 1 : Engine::Score(party[i], attribute, in_battle)));
            }
        }

        Engine::Sort(queue, descending);

        return queue;
    }

    // generic reset
    void Cancel(Character::Base &character, Character::Status status)
    {
        if (character.IsStatus(status))
        {
            character.RemoveStatus(status);
        }
    }

    // remove select statuses from character
    void Cancel(Character::Base &character, std::vector<Character::Status> statuses)
    {
        for (auto &status : statuses)
        {
            Engine::Cancel(character, status);
        }
    }

    // remove character's FLEEING and DEFENDED status
    void ResetStatus(Character::Base &character)
    {
        Engine::Cancel(character, {Character::Status::FLEEING, Character::Status::DEFENDED});
    }

    // reset spells (complexity)
    void ResetSpells(Character::Base &character)
    {
        if (character.HasSkill(Skills::Map("SPELLS")))
        {
            character.ResetSpellComplexities();
        }
    }

    // reset all status and spells (complexity)
    void ResetStatusAndSpells(Character::Base &character)
    {
        Engine::ResetStatus(character);

        Engine::ResetSpells(character);
    }

    // reset combat status, remove IN COMBAT status
    void ResetCombatStatus(Party::Base &party)
    {
        for (auto i = 0; i < party.Count(); i++)
        {
            party[i].RemoveStatus(Character::Status::IN_COMBAT);
        }
    }

    // resets all status except enthralment and fleeing
    void ResetAll(Party::Base &party)
    {
        party.RemoveStatus({Character::Status::AWAY,
                      Character::Status::DEFENDING,
                      Character::Status::DEFENDED,
                      Character::Status::ENTANGLED,
                      Character::Status::FLEEING,
                      Character::Status::PARALYZED,
                      Character::Status::SLOW_MURDER,
                      Character::Status::BURNED,
                      Character::Status::INVISIBLE,
                      Character::Status::FIGHTING_BLIND,
                      Character::Status::STRONG,
                      Character::Status::IN_COMBAT,
                      Character::Status::IN_BATTLE,
                      Character::Status::MELEE,
                      Character::Status::RANGED});

        party.ResetSpells();
    }

    // kill all paralyzed characters (e.g. fleeing the battle, abandoning other character)
    void KillAllParalyzed(Party::Base &party)
    {
        for (auto i = 0; i < party.Count(); i++)
        {
            if (Engine::IsAlive(party[i]) && party[i].IsStatus(Character::Status::PARALYZED))
            {
                party[i].Value(Attribute::Type::ENDURANCE, 0);
            }
        }
    }

    // helper string function (character is not in party)
    std::string NotInParty(Character::Class &character)
    {
        return std::string(std::string("YOU DO NOT HAVE THE ")) + Character::ClassMapping[character] + std::string(" IN YOUR PARTY!");
    }

    // helper string function (character is dead)
    std::string IsDead(Character::Base &character)
    {
        return character.Name + " IS DEAD!";
    }

    // helper string function (not carrying the item)
    std::string NoItem(Item::Type &item)
    {
        return std::string("YOU DO NOT HAVE THE ") + Item::TypeMapping[item] + "!";
    }

    // helper string function (not enough quantities of the item)
    std::string NotEnough(Item::Type &item)
    {
        return std::string("YOU DO NOT HAVE ENOUGH ") + Item::TypeMapping[item] + "!";
    }

    // change string case using input transform function (functptr)
    // see: https://www.geeksforgeeks.org/function-pointer-in-cpp/
    std::string ChangeCase(std::string str, int (*funcptr)(int))
    {
        auto transformed = str;

        std::transform(transformed.begin(), transformed.end(), transformed.begin(), funcptr);

        return transformed;
    }

    // compares two strings
    bool Compare(std::string s1, std::string s2, int (*funcptr)(int))
    {
        Engine::ChangeCase(s1, funcptr);

        Engine::ChangeCase(s2, funcptr);

        return (s1.compare(s2) == 0);
    }

    // changes all of the characters in the string to upper case
    std::string ToUpper(std::string str)
    {
        return Engine::ChangeCase(str, ::toupper);
    }

    // changes all of the characters in the string to lower case
    std::string ToLower(std::string str)
    {
        return Engine::ChangeCase(str, ::tolower);
    }

    // parse color from string
    Uint32 Color(std::string color)
    {
        auto border = Color::Active;

        if (Engine::ToUpper(color) == "INACTIVE")
        {
            border = Color::Inactive;
        }
        else if (Engine::ToUpper(color) == "HIGHLIGHT")
        {
            border = Color::Highlight;
        }
        else if (Engine::ToUpper(color) == "BACKGROUND")
        {
            border = Color::Background;
        }
        else if (Engine::ToUpper(color) == "TRANSPARENT")
        {
            border = Color::Transparent;
        }

        return border;
    }

    // returns index (in party) of first live character
    int First(Party::Base &party)
    {
        auto first = -1;

        if (Engine::IsAlive(party))
        {
            for (auto i = 0; i < party.Count(); i++)
            {
                if (Engine::IsAlive(party[i]))
                {
                    first = i;

                    break;
                }
            }
        }

        return first;
    }

    // returns index (in party) of first character with STATUS
    int First(Party::Base &party, Character::Status status)
    {
        auto first = -1;

        if (Engine::IsAlive(party))
        {
            for (auto i = 0; i < party.Count(); i++)
            {
                if (Engine::IsAlive(party[i]) && party[i].HasStatus(status))
                {
                    first = i;

                    break;
                }
            }
        }

        return first;
    }

    // returns index (in party) of the last (battle order) live character
    int Last(Party::Base &party)
    {
        auto last = -1;

        if (Engine::IsAlive(party))
        {
            for (auto i = (party.Count() - 1); i >= 0; i--)
            {
                if (Engine::IsAlive(party[i]))
                {
                    last = i;

                    break;
                }
            }
        }

        return last;
    }

    // returns index (in party) of last (battle order) of character with STATUS
    int Last(Party::Base &party, Character::Status status)
    {
        auto last = -1;

        if (Engine::IsAlive(party))
        {
            for (auto i = (party.Count() - 1); i >= 0; i--)
            {
                if (Engine::IsAlive(party[i]) && party[i].HasStatus(status))
                {
                    last = i;

                    break;
                }
            }
        }

        return last;
    }

    // returns class of first live character in party
    Character::Class FirstClass(Party::Base &party)
    {
        auto character = Engine::First(party);

        return character != -1 ? party[character].Class : Character::Class::NONE;
    }

    // returns index (in party) of first character with STATUS
    Character::Class FirstClass(Party::Base &party, Character::Status status)
    {
        auto character = Engine::First(party, status);

        return character != -1 ? party[character].Class : Character::Class::NONE;
    }

    // returns class of the last (battle order) live character
    Character::Class LastClass(Party::Base &party)
    {
        auto character = Engine::Last(party);

        return character != -1 ? party[character].Class : Character::Class::NONE;
    }

    // returns index (in party) of last (battle order) of character with STATUS
    Character::Class LastClass(Party::Base &party, Character::Status status)
    {
        auto character = Engine::Last(party, status);

        return character != -1 ? party[character].Class : Character::Class::NONE;
    }

    // check if the character is not injured
    bool Healed(Character::Base &character)
    {
        // check if dead (counts as healed) or healed
        return !Engine::IsAlive(character) || (Engine::Score(character, Attribute::Type::ENDURANCE) >= character.Maximum(Attribute::Type::ENDURANCE));
    }

    // check if no one in the party is injured
    bool Healed(Party::Base &party)
    {
        auto healed = true;

        // check if everyone is healed
        for (auto i = 0; i < party.Count(); i++)
        {
            if (Engine::IsAlive(party[i]))
            {
                healed &= Engine::Healed(party[i]);
            }
        }

        return healed;
    }

    int FirstAsset(Items::Inventory &items)
    {
        auto first = -1;

        for (auto item = 0; item < SafeCast(items.size()); item++)
        {
            if (items[item].Asset != Asset::NONE)
            {
                first = item;

                break;
            }
        }

        return first;
    }

    // map a single token
    std::string MapToken(std::string variable, std::vector<std::string> tokens, std::string replacement)
    {
        auto mapped = variable;

        auto space = std::string(" ");

        for (auto &token : tokens)
        {
            auto front = token.substr(0, 1) == " ";

            auto search = mapped.find(token);

            if (search != std::string::npos)
            {
                if (front)
                {
                    mapped.replace(search, token.length(), std::string(space) + std::string(replacement));
                }
                else
                {
                    mapped.replace(search, token.length(), std::string(replacement) + std::string(space));
                }
            }
        }

        return mapped;
    }

    // replace instances of the [TOKEN] in [VARIABLE] with [REPLACEMENT]
    std::string MapToken(std::string variable, std::string token, std::string replacement)
    {
        auto tokens = std::vector<std::string>();

        auto space = std::string(" ");

        // generate instances of the token (upper case, lower case)
        tokens.push_back(space + Engine::ToLower(token));

        tokens.push_back(space + Engine::ToUpper(token));

        tokens.push_back(Engine::ToLower(token) + space);

        tokens.push_back(Engine::ToUpper(token) + space);

        return Engine::MapToken(variable, tokens, replacement);
    }

    // checks if a character can drop any item from their inventory
    bool CanDrop(Character::Base &character)
    {
        auto drop = false;

        if (Engine::IsAlive(character))
        {
            for (auto &item : character.Items)
            {
                if (!item.HasProperty(Item::MapProperty("CANNOT DROP")))
                {
                    drop = true;

                    break;
                }
            }
        }

        return drop;
    }

    // generate timestamp string
    std::string UtcTime(std::time_t time)
    {
        char time_string[std::size("yyyy-mm-dd HH:mm:ss")];

        std::strftime(std::data(time_string), std::size(time_string), "%F %T", std::localtime(&time));

        return std::string(time_string);
    }

    // get current time (string)
    std::string UtcTimeNow()
    {
        std::time_t time = std::time({});

        return Engine::UtcTime(time);
    }

    // convert a timepoint to time_t using system clock
    template <typename T>
    std::time_t ConvertTime(T timepoint)
    {
        using namespace std::chrono;

        auto system_clock_timepoint = time_point_cast<system_clock::duration>(timepoint - T::clock::now() + system_clock::now());

        return system_clock::to_time_t(system_clock_timepoint);
    }
}
