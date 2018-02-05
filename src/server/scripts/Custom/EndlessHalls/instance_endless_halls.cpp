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
            my3DArray = create3DArray(10, 10, 4);

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
            player->SetGameMaster(false); 
            player->BindToInstance();

            if(!reachFinal)
                player->AddAura(SPELL_MUTE_PLAYER, player);

            phaseMaze();
            player->CastSpell(player, SPELL_HIDDEN_AURA_1SEC, true);
        }

        // genere maze from template
        int*** create3DArray(int col, int line, int four)
        {
            // Differents mazes, maybe i should make a randomiser later.
            std::string mazesStr[10] = {
                "0110010101010111010101010001011001010011110000110110100101100101010110010110100100101010101001101001010001010011110000111010101010101100011101010011111000011010111010011110000110100010110010110110101111000011110000111010101001101001101010000100110100011010101010101100000111000011011001010101101110101100001101100101100111100011010010011100001111001001011000111000110001010101010111010101010110011000",
                "0110000101100001011001010101010101110011101000101110001110100110010100011010101011001011101011001001110000110110100110100010101011000101010101011011101000101010101011000011011001010101100110101110100111100001101010000110010100111010110000111100001111000101100100101100100101101001011011010011011000111110011100011010001011100011100010101010101010100110100110101000110001011001110010011100110101011001",
                "0110010100010110010101010101011100110010111001010011110000110010011010011100100110100110100101101001101011000101010100111000110000111100010111010011011001011011011000111010011001010101100110100110100110101100100110100110010100011010101000101110010101011101100101100101100111001011101001000011011001011001010000110110100110100110100110100110011101011001101000101000110001011001100011000101010111011001",
                "0110010101110101010101010101001101100011101000101010011001110101000110101010101011001011101010101000011001011001101010000110100110101100010110010110001111000011101001101001011001010011101011000101100110101100010111010001101010100110001100101000011000110110010110111010101010101010011010011100100101101001110010011100101110100110010100111000011001110101000110101100110100011100010110011100010101011001",
                "0110001101100011011001110011011001110011101011001001101010101010100010101010101011100101001110001010101001101001101010101100001111000101100111001101001110101000011010010110001100100110001110001100001111000101100110101010101011000101010110110010011000011100101111000101001101001001101011000111010111010101000110100110001111000011100001100101010100111100100110100100110101011001010001011101010101011001",
                "0110010100110110011100110110001101100011111000111010100010101010100011101001101010101010110001011001110000111100001110101000110000110010011000111100001110101010011001011001101010101010011010011000101010100010011011011001110010010110001110101010111010010110010100110110100111001011101011000101100100101100100101100001101010100010011001011101011100011110010110011000110011010101000111000101110101010001",
                "0110010101010101001101100101001101000011101001000011011011011001001011000011101011100011111010010110001111000101110110111000101011100001101011000101010100111000011010011000011010010100011100111100001111100101010111010111010110011100000110101100001101100011100001100101010100111010011010011010110001011001001001101001101011000011101001100101001110101100001110100100100111001101000110001100010111011001",
                "0110010101010101010100110100011101010011101001000101001100101010011010010110100110100110001110101100100110100100110100111010101010101110000101101101010100111010101010101010111001011101010100111010100011001001101011000011011000111000110000110010011010010110100110101010011000111010101010100010111000011010110010011010101011101001110010010110100101100011110010111100010101010101100101001001110001011001",
                "0110010101010101010100110100010101110011110000110010011000111010011000111010101001001001111010011010110010011100100110100110001110100110110101010101011100011010101010101010110000110100011110010110100110101100110100011100001110100010101000101010001001100101010110011100101110101010101010101100010101010011011010011100101111001011010001110011101010000110001110100100110101011001100011000101100111001001",
                "0110011101010101001101100101001101100011101011000101001111001001011010011010101010100110000111000011011010010100101110101010101001100011101011000101010110011010110010011010110010010100011100110110100101100101100101000101011110011000110000111010011001010101001111000011011001011001110011010101001111000011110011010101001101100101001111000001110000110110001110101100000111000101010101011001100011001001"
            };

            int*** array3D = 0;
            array3D = new int**[col];
            uint8 pickrand = rand() % 10;
            std::string mazeinfo = mazesStr[pickrand];
            int index = 0;

            for (int h = 0; h < col; h++)
            {
                array3D[h] = new int*[line];

                for (int w = 0; w < line; w++)
                {
                    array3D[h][w] = new int[four];
                    for (int k = 0; k < four; k++)
                    {
                        array3D[h][w][k] = stoi(mazeinfo.substr(index, 1));
                        index++;
                    }
                }
            }

            return array3D;
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


            printf("\n Rune bleu : %i", runeBluePos);
            printf("\n Orbe bleu : %i", orbBluePos);
            printf("\n Rune rouge : %i", runeRedPos);
            printf("\n Orbe rouge : %i", orbRedPos);
            printf("\n Rune verte : %i", runeGreenPos);
            printf("\n Orbe verte : %i", orbGreenPos);
            printf("\n Rune jaune : %i", runeYellowPos);
            printf("\n Orbe jaune : %i", orbYellowPos);
            printf("\n Rune violet : %i", runeVioletPos);
            printf("\n Orbe violet : %i", orbVioletPos);
        }

        // Hide and show gameobjects pending maze location
        void phaseMaze()
        {
            // Shit coded, need to refactor
            // Runes

            if (GameObject* go = GetGameObject(DATA_RUNE_BLEU))
            {
                if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == runeBluePos && BlueRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != runeBluePos && BlueRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE))
            {
                if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == runeRedPos && RedRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));     
                else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != runeRedPos && RedRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_VERT))
            {
                if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == runeGreenPos && GreenRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != runeGreenPos && GreenRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE))
            {
                if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == runeYellowPos && YellowRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != runeYellowPos && YellowRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
            }

            if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET))
            {
                if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == runeVioletPos && VioletRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != runeVioletPos && VioletRuneState != GO_STATE_READY)
                    go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
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
                        printf("\n Phasing actualPos : %i", actualPos);
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
                    if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == orbBluePos && BlueRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != orbBluePos && BlueRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }

                if (GameObject* go = instance->GetGameObject(RedOrbsGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == orbRedPos && RedRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != orbRedPos && RedRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }

                if (GameObject* go = instance->GetGameObject(GreenOrbsGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == orbGreenPos && GreenRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != orbGreenPos && GreenRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }

                if (GameObject* go = instance->GetGameObject(YellowOrbsGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == orbYellowPos && YellowRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != orbYellowPos && YellowRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }

                if (GameObject* go = instance->GetGameObject(VioletOrbsGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && actualPos == orbVioletPos && VioletRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && actualPos != orbVioletPos && VioletRuneState != GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }

                // Rocks
                if (GameObject* go = instance->GetGameObject(NorthRocksGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][0] == 0)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][0] == 1)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }
                if (GameObject* go = instance->GetGameObject(SouthRocksGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][2] == 0)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][2] == 1)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }
                if (GameObject* go = instance->GetGameObject(EastRocksGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][1] == 0)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][1] == 1)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                }
                if (GameObject* go = instance->GetGameObject(WestRocksGUIDs[i]))
                {
                    if (go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][3] == 0)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    else if (!go->IsInPhase(PHASE_INVISIBLE) && my3DArray[k][v][3] == 1)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
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
                            go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;

                case GO_ROCK_SOUTH:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (SouthRocksGUIDs[i].IsEmpty())
                        {
                            SouthRocksGUIDs[i] = go->GetGUID();
                            go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;

                case GO_ROCK_EAST:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (EastRocksGUIDs[i].IsEmpty())
                        {
                            EastRocksGUIDs[i] = go->GetGUID();
                            go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;

                case GO_ROCK_WEST:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (WestRocksGUIDs[i].IsEmpty())
                        {
                            WestRocksGUIDs[i] = go->GetGUID();
                            go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;

                case GO_ORBE_BLEU:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (BlueOrbsGUIDs[i].IsEmpty())
                        {
                            BlueOrbsGUIDs[i] = go->GetGUID();
                            if (BlueRuneState == GO_STATE_READY)
                                go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;
                case GO_ORBE_ROUGE:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (RedOrbsGUIDs[i].IsEmpty())
                        {
                            RedOrbsGUIDs[i] = go->GetGUID();
                            if (RedRuneState == GO_STATE_READY)
                                go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;
                case GO_ORBE_VERT:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (GreenOrbsGUIDs[i].IsEmpty())
                        {
                            GreenOrbsGUIDs[i] = go->GetGUID();
                            if (GreenRuneState == GO_STATE_READY)
                                go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;
                case GO_ORBE_JAUNE:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (YellowOrbsGUIDs[i].IsEmpty())
                        {
                            YellowOrbsGUIDs[i] = go->GetGUID();
                            if (YellowRuneState == GO_STATE_READY)
                                go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;
                case GO_ORBE_VIOLET:
                    for (uint8 i = 0; i < OrbeCount; ++i)
                        if (VioletOrbsGUIDs[i].IsEmpty())
                        {
                            VioletOrbsGUIDs[i] = go->GetGUID();
                            if (VioletRuneState == GO_STATE_READY)
                                go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                            break;
                        }
                    break;
                case GO_RUNE_BLEU:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (BlueRuneState == GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    break;
                case GO_RUNE_ROUGE:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (RedRuneState == GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    break;
                case GO_RUNE_VERT:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (GreenRuneState == GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    break;
                case GO_RUNE_JAUNE:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (YellowRuneState == GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
                    break;
                case GO_RUNE_VIOLET:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    if (VioletRuneState == GO_STATE_READY)
                        go->SetInPhase(PHASE_INVISIBLE, true, !go->IsInPhase(PHASE_INVISIBLE));
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
                                gob->SetInPhase(PHASE_REMOVED, true, !gob->IsInPhase(PHASE_REMOVED));
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_BLEU))
                            go->SetInPhase(PHASE_REMOVED, true, !go->IsInPhase(PHASE_REMOVED));
                        break;
                    case GO_RUNE_ROUGE: 
                        RedRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(RedOrbsGUIDs[i]))
                                gob->SetInPhase(PHASE_REMOVED, true, !gob->IsInPhase(PHASE_REMOVED));
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_ROUGE))
                            go->SetInPhase(PHASE_REMOVED, true, !go->IsInPhase(PHASE_REMOVED));
                        break;
                    case GO_RUNE_VERT:
                        GreenRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(GreenOrbsGUIDs[i]))
                                gob->SetInPhase(PHASE_REMOVED, true, !gob->IsInPhase(PHASE_REMOVED));
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_VERT))
                            go->SetInPhase(PHASE_REMOVED, true, !go->IsInPhase(PHASE_REMOVED));
                        break;
                    case GO_RUNE_JAUNE:
                        YellowRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(YellowOrbsGUIDs[i]))
                                gob->SetInPhase(PHASE_REMOVED, true, !gob->IsInPhase(PHASE_REMOVED));
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_JAUNE))
                            go->SetInPhase(PHASE_REMOVED, true, !go->IsInPhase(PHASE_REMOVED));
                        break;
                    case GO_RUNE_VIOLET:
                        VioletRuneState = GO_STATE_READY;
                        for (uint8 i = 0; i < OrbeCount; ++i)
                        {
                            if (GameObject* gob = instance->GetGameObject(VioletOrbsGUIDs[i]))
                                gob->SetInPhase(PHASE_REMOVED, true, !gob->IsInPhase(PHASE_REMOVED));
                        }
                        if (GameObject* go = GetGameObject(DATA_RUNE_VIOLET))
                            go->SetInPhase(PHASE_REMOVED, true, !go->IsInPhase(PHASE_REMOVED));
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

        int*** my3DArray;
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
