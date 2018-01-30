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

#ifndef endless_halls_h__
#define endless_halls_h__

#include "CreatureAIImpl.h"

#define EndlessHallsScriptName "instance_endless_halls"
#define DataHeader "EH"

enum EHDataTypes
{
    DATA_RUNE_BLEU = 0,
    DATA_RUNE_ROUGE = 1,
    DATA_RUNE_VERT = 2,
    DATA_RUNE_JAUNE = 3,
    DATA_RUNE_VIOLET = 4,

    DATA_ORBE_BLEU = 5,
    DATA_ORBE_ROUGE = 6, 
    DATA_ORBE_VERT = 7, 
    DATA_ORBE_JAUNE = 8, 
    DATA_ORBE_VIOLET = 9,

    DATA_TP_NORTH = 10,
    DATA_TP_SOUTH = 11, 
    DATA_TP_EAST = 12,
    DATA_TP_WEST = 13,

    DATA_ROCK_NORTH = 14,
    DATA_ROCK_SOUTH = 15,
    DATA_ROCK_EAST = 16,
    DATA_ROCK_WEST = 17,

    DATA_PICK_RUNE = 18,
    DATA_PICK_ORBE = 19,
    DATA_DIRECTION = 20,
};

enum EHGameObjectsIds
{
    GO_RUNE_BLEU = 272215,
    GO_RUNE_ROUGE = 272217,
    GO_RUNE_VERT = 272216,
    GO_RUNE_JAUNE = 272218,
    GO_RUNE_VIOLET = 272219,

    GO_ORBE_BLEU = 272211,
    GO_ORBE_ROUGE = 272210,
    GO_ORBE_VERT = 272214,
    GO_ORBE_JAUNE = 272213,
    GO_ORBE_VIOLET = 272212,

    // Not blizzlike gobs, can't find the real entries
    GO_TP_NORTH = 5000,
    GO_TP_SOUTH = 5001,
    GO_TP_EAST = 5002,
    GO_TP_WEST = 5003,

    GO_ROCK_NORTH  = 5004,
    GO_ROCK_SOUTH  = 5005,
    GO_ROCK_EAST = 5006,
    GO_ROCK_WEST = 5007,
};

enum EHSpells
{
    SPELL_DIRECT_NORTH = 247350,
    SPELL_DIRECT_SOUTH = 247351,
    SPELL_DIRECT_EAST = 247352,
    SPELL_DIRECT_WEST = 247353,

    SPELL_DIRECT_BLACKOUT = 144149,

    SPELL_DIRECT_VICTORY = 247348,

    SPELL_BLUE_ORB = 247322,
    SPELL_RED_ORB = 247321,
    SPELL_GREEN_ORB = 247326,
    SPELL_YELLOW_ORB = 247323,
    SPELL_VIOLET_ORB = 247324,

    SPELL_MUTE_PLAYER = 1852,

    SPELL_HIDDEN_AURA_1SEC = 218611, // Swimmer Teleport
    SPELL_EXIT_MAZE = 247349, // Teleport out
    // Creer un gob de tp comme les autres et cast ce spell
    // Utiliser méthode OnCast, puis unaura le joueur, tp le joueur en extérieur et ensuite on détruit l'instance.

};

enum EHPhases
{
    PHASE_INVISIBLE = 175,
    PHASE_REMOVED = 176
};

enum EHMaze
{
    INSTANCE_MAZE_X = 0,
    INSTANCE_MAZE_Y = 0
};



template<typename AI>
inline AI* GetEndlessHallsAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, EndlessHallsScriptName);
}

#endif // endless_halls_h__
