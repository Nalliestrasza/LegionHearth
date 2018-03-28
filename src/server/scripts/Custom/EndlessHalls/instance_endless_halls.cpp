/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
This placeholder for the instance is needed for dungeon finding to be able
to give credit after the boss defined in lastEncounterDungeon is killed.
Without it, the party doing random dungeon won't get satchel of spoils and
gets instead the deserter debuff.
*/

#include "ScriptMgr.h"
#include "Creature.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "endless_halls.h"
#include "PhasingHandler.h"

ObjectData const creatureData[] =
{
    { 0,                     0 } // END
};

ObjectData const gameObjectData[] =
{
    { GO_RUNE_BLEU,             DATA_RUNE_BLEU},
    { GO_RUNE_ROUGE,            DATA_RUNE_ROUGE},
    { GO_RUNE_VERT,             DATA_RUNE_VERT},
    { GO_RUNE_JAUNE,            DATA_RUNE_JAUNE},
    { GO_RUNE_VIOLET,           DATA_RUNE_VIOLET},
    { GO_ORBE_BLEU,             DATA_ORBE_BLEU},
    { GO_ORBE_ROUGE,            DATA_ORBE_ROUGE},
    { GO_ORBE_VERT,             DATA_ORBE_VERT},
    { GO_ORBE_JAUNE,            DATA_ORBE_JAUNE},
    { GO_ORBE_VIOLET,           DATA_ORBE_VIOLET},
    { GO_TP_NORTH,              DATA_TP_NORTH },
    { GO_TP_SOUTH,              DATA_TP_SOUTH },
    { GO_TP_EAST,               DATA_TP_EAST },
    { GO_TP_WEST,               DATA_TP_WEST },
    { GO_ROCK_NORTH,            DATA_ROCK_NORTH },
    { GO_ROCK_SOUTH,            DATA_ROCK_SOUTH },
    { GO_ROCK_EAST,             DATA_ROCK_EAST },
    { GO_ROCK_WEST,             DATA_ROCK_WEST },
    { 0,                           0 } // END
};

class instance_endless_halls : public InstanceMapScript
{
public:
    instance_endless_halls() : InstanceMapScript(EndlessHallsScriptName, 1764) { }

    struct instance_endless_halls_InstanceMapScript : public InstanceScript
    {
        instance_endless_halls_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            LoadObjectData(creatureData, gameObjectData);

            BlueRuneState = GO_STATE_ACTIVE;
            RedRuneState = GO_STATE_ACTIVE;
            GreenRuneState = GO_STATE_ACTIVE;
            YellowRuneState = GO_STATE_ACTIVE;
            VioletRuneState = GO_STATE_ACTIVE;

            finished = false;
            reachFinal = false;

            actualPos = -1;

            runeBluePos = -1;
            runeRedPos = -1;
            runeGreenPos = -1;
            runeYellowPos = -1;
            runeVioletPos = -1;
            orbBluePos = -1;
            orbRedPos = -1;
            orbGreenPos = -1;
            orbYellowPos = -1;
            orbVioletPos = -1;

            // Create and initialize the maze
            mazeDefinition = generateMaze(10, 10, 4);

            // Assign orbs and runes pos
            mazeObjects();

            // Mask objects
            phaseMaze();

            // Time, used for tp at last
            canUseTime = false;
            canTpNow = time(0);
        }

        void OnPlayerEnter(Player* player) override
        {

            if (player->IsGameMaster())
                player->SetGameMaster(false);

            player->BindToInstance();

            if(!reachFinal)
                player->AddAura(SPELL_MUTE_PLAYER, player);

            phaseMaze();
            player->CastSpell(player, SPELL_HIDDEN_AURA_1SEC, true);
        }

        // genere maze from template
        int*** generateMaze(int col, int line, int walls)
        {
            int*** mazeDef = 0;
            mazeDef = new int**[col];
            uint8 pickrand = rand() % 10;
            std::string mazeinfo = mazeTemplate[pickrand];
            int index = 0;

            for (int h = 0; h < col; h++)
            {
                mazeDef[h] = new int*[line];

                for (int w = 0; w < line; w++)
                {
                    mazeDef[h][w] = new int[walls];
                    for (int k = 0; k < walls; k++)
                    {
                        mazeDef[h][w][k] = stoi(mazeinfo.substr(index, 1));
                        index++;
                    }
                }
            }

            return mazeDef;
        }

        // genere objets positions
        void mazeObjects()
        {
            std::vector<int> v{};

            while(v.size() < 10)
            {
                int randomnumber = rand() % 100;
                v.push_back(randomnumber);
                std::sort(v.begin(), v.end());
                auto last = std::unique(v.begin(), v.end());
                v.erase(last, v.end());
            }

            // randomize order
            std::random_shuffle(v.begin(), v.end());

            // Objects positions
            runeBluePos = v[0];
            runeRedPos = v[1];
            runeGreenPos = v[2];
            runeYellowPos = v[3];
            runeVioletPos = v[4];
            orbBluePos = v[5];
            orbRedPos = v[6];
            orbGreenPos = v[7];
            orbYellowPos = v[8];
            orbVioletPos = v[9];

            // player start at random val
            actualPos = rand() % 100;


            printf("Rune bleu : %i \n", runeBluePos);
            printf("Orbe bleu : %i \n", orbBluePos);
            printf("Rune rouge : %i \n", runeRedPos);
            printf("Orbe rouge : %i \n", orbRedPos);
            printf("Rune verte : %i \n", runeGreenPos);
            printf("Orbe verte : %i \n", orbGreenPos);
            printf("Rune jaune : %i \n", runeYellowPos);
            printf("Orbe jaune : %i \n", orbYellowPos);
            printf("Rune violet : %i \n", runeVioletPos);
            printf("Orbe violet : %i \n", orbVioletPos);
        }

        // Hide and show gameobjects pending maze location
        void phaseMaze()
        {
            // Shit coded, need to refactor
            // Runes

            if (GameObject* go = GetGameObject(DATA_RUNE_BLEU))
            {
                if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == runeBluePos && BlueRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != runeBluePos && BlueRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else
                    PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE))
            {
                if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == runeRedPos && RedRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);     
                else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != runeRedPos && RedRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else
                    PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_VERT))
            {
                if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == runeGreenPos && GreenRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != runeGreenPos && GreenRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else
                    PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE))
            {
                if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == runeYellowPos && YellowRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != runeYellowPos && YellowRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else
                    PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET))
            {
                if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == runeVioletPos && VioletRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != runeVioletPos && VioletRuneState != GO_STATE_READY)
                    PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                else
                    PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
            }

            // Rocks position
            int test = 0;
            int k = 0;
            int v = 0;
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    if (actualPos == test)
                    {
                        printf("Phasing actualPos : %i \n", actualPos);
                        k = i;
                        v = j;
                    }
                    test++;
                }
            }


            for (uint8 i = 0; i < OrbeCount; ++i)
            {
                // Orbes

                if (GameObject* go = instance->GetGameObject(BlueOrbsGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == orbBluePos && BlueRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != orbBluePos && BlueRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }

                if (GameObject* go = instance->GetGameObject(RedOrbsGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == orbRedPos && RedRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != orbRedPos && RedRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }

                if (GameObject* go = instance->GetGameObject(GreenOrbsGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == orbGreenPos && GreenRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != orbGreenPos && GreenRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }

                if (GameObject* go = instance->GetGameObject(YellowOrbsGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == orbYellowPos && YellowRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != orbYellowPos && YellowRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }

                if (GameObject* go = instance->GetGameObject(VioletOrbsGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos == orbVioletPos && VioletRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && actualPos != orbVioletPos && VioletRuneState != GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }

                // Rocks
                if (GameObject* go = instance->GetGameObject(NorthRocksGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][0] == 0)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][0] == 1)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }
                if (GameObject* go = instance->GetGameObject(SouthRocksGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][2] == 0)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][2] == 1)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }
                if (GameObject* go = instance->GetGameObject(EastRocksGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][1] == 0)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][1] == 1)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }
                if (GameObject* go = instance->GetGameObject(WestRocksGUIDs[i]))
                {
                    if (go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][3] == 0)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else if (!go->GetPhaseShift().HasPhase(PHASE_INVISIBLE) && mazeDefinition[k][v][3] == 1)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                }
            }
        }

        void poseMaze(uint32 data)
        {
            switch (data)
            {
            case SPELL_DIRECT_NORTH:
                actualPos = actualPos - 10;
                break;
            case SPELL_DIRECT_SOUTH:
                actualPos = actualPos + 10;
                break;
            case SPELL_DIRECT_EAST:
                actualPos = actualPos + 1;
                break;
            case SPELL_DIRECT_WEST:
                actualPos = actualPos - 1;
                break;
            default:
                break;
            }

            // Cheat or bug exploit
            if (actualPos > 99 || actualPos < 0)
                actualPos = rand() % 100;
        }
        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_ROCK_NORTH:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (NorthRocksGUIDs[i].IsEmpty())
                        {
                            NorthRocksGUIDs[i] = go->GetGUID();
                            PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;

                case GO_ROCK_SOUTH:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (SouthRocksGUIDs[i].IsEmpty())
                        {
                            SouthRocksGUIDs[i] = go->GetGUID();
                            PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;

                case GO_ROCK_EAST:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (EastRocksGUIDs[i].IsEmpty())
                        {
                            EastRocksGUIDs[i] = go->GetGUID();
                            PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;

                case GO_ROCK_WEST:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (WestRocksGUIDs[i].IsEmpty())
                        {
                            WestRocksGUIDs[i] = go->GetGUID();
                            PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;

                case GO_ORBE_BLEU:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (BlueOrbsGUIDs[i].IsEmpty())
                        {
                            BlueOrbsGUIDs[i] = go->GetGUID();
                            if (BlueRuneState == GO_STATE_READY)
                                PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            else
                                PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;
                case GO_ORBE_ROUGE:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (RedOrbsGUIDs[i].IsEmpty())
                        {
                            RedOrbsGUIDs[i] = go->GetGUID();
                            if (RedRuneState == GO_STATE_READY)
                                PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            else
                                PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;
                case GO_ORBE_VERT:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (GreenOrbsGUIDs[i].IsEmpty())
                        {
                            GreenOrbsGUIDs[i] = go->GetGUID();
                            if (GreenRuneState == GO_STATE_READY)
                                PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            else
                                PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;
                case GO_ORBE_JAUNE:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (YellowOrbsGUIDs[i].IsEmpty())
                        {
                            YellowOrbsGUIDs[i] = go->GetGUID();
                            if (YellowRuneState == GO_STATE_READY)
                                PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            else
                                PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;
                case GO_ORBE_VIOLET:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (VioletOrbsGUIDs[i].IsEmpty())
                        {
                            VioletOrbsGUIDs[i] = go->GetGUID();
                            if (VioletRuneState == GO_STATE_READY)
                                PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                            else
                                PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                            break;
                        }
                    break;
                case GO_RUNE_BLEU:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (BlueRuneState == GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                    break;
                case GO_RUNE_ROUGE:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (RedRuneState == GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                    break;
                case GO_RUNE_VERT:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (GreenRuneState == GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                    break;
                case GO_RUNE_JAUNE:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (YellowRuneState == GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                    break;
                case GO_RUNE_VIOLET:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (VioletRuneState == GO_STATE_READY)
                        PhasingHandler::AddPhase(go, PHASE_INVISIBLE, true);
                    else
                        PhasingHandler::RemovePhase(go, PHASE_INVISIBLE, true);
                    break;
                default:
                    break;
            }

            InstanceScript::OnGameObjectCreate(go);
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == DATA_DIRECTION)
            {
                poseMaze(data);
                phaseMaze();

                // Final room, with time check
                if (finished)
                {
                    canUseTime = true;
                    canTpNow = time(0);
                }
                    
            }
            else if (type == DATA_PICK_ORBE)
            {

                // please refactor this properly.
                // Set all the runes, not targetable.
                if (GameObject* go = GetGameObject(DATA_RUNE_BLEU)) go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE)) go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* go = GetGameObject(DATA_RUNE_VERT)) go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE)) go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET)) go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                switch (data)
                {
                    case GO_ORBE_BLEU:
                        if (GameObject* go = GetGameObject(DATA_RUNE_BLEU))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case GO_ORBE_ROUGE:
                        if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case GO_ORBE_VERT:
                        if (GameObject* go = GetGameObject(DATA_RUNE_VERT))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case GO_ORBE_JAUNE:
                        if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case GO_ORBE_VIOLET:
                        if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET))
                            go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                }
            }
            else if (type == DATA_PICK_RUNE)
            {
                switch (data)
                {
                    case GO_RUNE_BLEU:
                        BlueRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(BlueOrbsGUIDs[i]))
                                PhasingHandler::AddPhase(gob, PHASE_REMOVED, true);
                            else
                                PhasingHandler::RemovePhase(gob, PHASE_REMOVED, true);
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_BLEU))
                            PhasingHandler::AddPhase(go, PHASE_REMOVED, true);
                        else
                            PhasingHandler::RemovePhase(go, PHASE_REMOVED, true);
                        break;
                    case GO_RUNE_ROUGE: 
                        RedRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(RedOrbsGUIDs[i]))
                                PhasingHandler::AddPhase(gob, PHASE_REMOVED, true);
                            else
                                PhasingHandler::RemovePhase(gob, PHASE_REMOVED, true);
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE))
                            PhasingHandler::AddPhase(go, PHASE_REMOVED, true);
                        else
                            PhasingHandler::RemovePhase(go, PHASE_REMOVED, true);
                        break;
                    case GO_RUNE_VERT:
                        GreenRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(GreenOrbsGUIDs[i]))
                                PhasingHandler::AddPhase(gob, PHASE_REMOVED, true);
                            else
                                PhasingHandler::RemovePhase(gob, PHASE_REMOVED, true);
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_VERT))
                            PhasingHandler::AddPhase(go, PHASE_REMOVED, true);
                        else
                            PhasingHandler::RemovePhase(go, PHASE_REMOVED, true);
                        break;
                    case GO_RUNE_JAUNE:
                        YellowRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(YellowOrbsGUIDs[i]))
                                PhasingHandler::AddPhase(gob, PHASE_REMOVED, true);
                            else
                                PhasingHandler::RemovePhase(gob, PHASE_REMOVED, true);
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE))
                            PhasingHandler::AddPhase(go, PHASE_REMOVED, true);
                        else
                            PhasingHandler::RemovePhase(go, PHASE_REMOVED, true);
                        break;
                    case GO_RUNE_VIOLET:
                        VioletRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(VioletOrbsGUIDs[i]))
                                PhasingHandler::AddPhase(gob, PHASE_REMOVED, true);
                            else
                                PhasingHandler::RemovePhase(gob, PHASE_REMOVED, true);
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET))
                            PhasingHandler::AddPhase(go, PHASE_REMOVED, true);
                        else
                            PhasingHandler::RemovePhase(go, PHASE_REMOVED, true);
                        break;
                }
            }

            SaveToDB();
        }

        void WriteSaveDataMore(std::ostringstream& data) override
        {
            data << uint32(BlueRuneState) << ' ';
            data << uint32(RedRuneState) << ' ';
            data << uint32(GreenRuneState) << ' ';
            data << uint32(YellowRuneState) << ' ';
            data << uint32(VioletRuneState) << ' ';
        }

        void ReadSaveDataMore(std::istringstream& data) override
        {
            uint32 temp;

            data >> temp;
            BlueRuneState = GOState(temp);

            data >> temp;
            RedRuneState = GOState(temp);

            data >> temp;
            GreenRuneState = GOState(temp);

            data >> temp;
            YellowRuneState = GOState(temp);

            data >> temp;
            VioletRuneState = GOState(temp);
        }

        void Update(uint32 diff) override
        {
            

            if (BlueRuneState == GO_STATE_READY && RedRuneState == GO_STATE_READY && GreenRuneState == GO_STATE_READY && YellowRuneState == GO_STATE_READY && VioletRuneState == GO_STATE_READY && !finished)
                finished = true;

            if (canUseTime && difftime(time(0), canTpNow) > 2.001)
            {
                Map::PlayerList const& lPlayers = instance->GetPlayers();
                if (!lPlayers.isEmpty() && finished)
                {
                    for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                        if (Player* player = itr->GetSource())
                        {
                            // Maybe do this when player leave properly the dungeon and not when reaching the room.
                            //TODO : create portal in final room and teleport the player outta here with unaura all and other things
                            // Try to delete instance when leave
                            reachFinal = true;
                            player->TeleportTo(1764,-1748.07f,354.195f,116.615f,(float)M_PI); // Final room
                        }

                }
                canUseTime = false;
            }

            // Brutal
            Map::PlayerList const& lPlayers = instance->GetPlayers();

            if (!lPlayers.isEmpty())
            {
                for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                    if (Player* player = itr->GetSource())
                    {
                        if (player->IsGameMaster())
                            player->SetGameMaster(false);

                        player->SetSpeedRate(MOVE_RUN, 2.0f);
                        player->SetCanFly(false);
                    }
                        
            }
            // SaveToDB();
        }

    protected:

        GOState BlueRuneState;
        GOState RedRuneState;
        GOState GreenRuneState;
        GOState YellowRuneState;
        GOState VioletRuneState;

        // 2 gameobjects orbs in room
        // 2 rocks per position, so reuse o
        static uint8 const OrbeCount = 2;
        ObjectGuid BlueOrbsGUIDs[OrbeCount];
        ObjectGuid RedOrbsGUIDs[OrbeCount];
        ObjectGuid GreenOrbsGUIDs[OrbeCount];
        ObjectGuid YellowOrbsGUIDs[OrbeCount];
        ObjectGuid VioletOrbsGUIDs[OrbeCount];

        ObjectGuid NorthRocksGUIDs[OrbeCount];
        ObjectGuid SouthRocksGUIDs[OrbeCount];
        ObjectGuid EastRocksGUIDs[OrbeCount];
        ObjectGuid WestRocksGUIDs[OrbeCount];

        int*** mazeDefinition;
        int actualPos;

        int runeBluePos;
        int runeRedPos;
        int runeGreenPos;
        int runeYellowPos;
        int runeVioletPos;
        int orbBluePos;
        int orbRedPos;
        int orbGreenPos;
        int orbYellowPos;
        int orbVioletPos;

        bool finished;
        bool reachFinal;

        bool canUseTime;
        time_t canTpNow;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_endless_halls_InstanceMapScript(map);
    }
};

void AddSC_instance_endless_halls()
{
    new instance_endless_halls();
}
