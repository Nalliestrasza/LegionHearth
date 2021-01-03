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

#ifndef SC_PRECOMPILED_H
#define SC_PRECOMPILED_H

#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Unit.h"
#include "GameObject.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "SpellScript.h"

#include "SpellAuraEffects.h"
#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "Language.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "SpellAuras.h"
#include "TargetedMovementGenerator.h"
#include "WeatherMgr.h"
#include "Player.h"
#include "Pet.h"
#include "LFG.h"
#include "GroupMgr.h"
#include "MMapFactory.h"
#include "DisableMgr.h"
#include "SpellHistory.h"
#include "MiscPackets.h"
#include "Transport.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "WorldSession.h"



#ifdef _WIN32
#include <windows.h>
#endif

#endif

class Player_perm : public PlayerScript //Dis no respect sneaky case
{
public:
	Player_perm() : PlayerScript("Player_perm") {}
	void OnLogin(Player* player, bool firstLogin)
	{

        //Select Customs
        QueryResult getDisplay = WorldDatabase.PQuery("SELECT displayId, scale, skybox from player_custom WHERE guid = %u", player->GetGUID().GetCounter());
        if (!getDisplay)
            return;

        Field* field = getDisplay->Fetch();

        uint32 display_id = field[0].GetUInt32();
        float scale = field[1].GetFloat();
        uint32 skyboxId = field[2].GetUInt32();
        uint32 map = player->GetMapId();
		uint32 mapCache = player->GetMapId();


        if (display_id != 0)
        {
            player->SetDisplayId(display_id);
        }

        if (scale != 1)
        {
            player->SetObjectScale(scale);
        }

        if (skyboxId != 0)
        {
            if (map > MAP_CUSTOM_PHASE)
            {
       
                if (player->GetMapId() >= MAP_CUSTOM_PHASE)
                {
                    if (MapEntry const* entry = sMapStore.AssertEntry(map))
                        map = entry->ParentMapID;
                }

                uint32 lightId = DB2Manager::GetMapLightId(map);

                WorldPacket data(SMSG_OVERRIDE_LIGHT, 12);
                data << lightId;
                data << skyboxId;
                data << 200;

                sWorld->SendMapMessage(mapCache, WorldPackets::Misc::OverrideLight(int32(lightId), int32(200), int32(skyboxId)).Write());

            }
            else
            {
                uint32 lightId = DB2Manager::GetMapLightId(map);

                WorldPacket data(SMSG_OVERRIDE_LIGHT, 12);
                data << lightId;
                data << skyboxId;
                data << 200;

                player->GetSession()->SendPacket(&data, true);
            }
           
        }
	}

};

class Player_panda : public PlayerScript
{
public:
    Player_panda() : PlayerScript("Player_panda") {}
    void OnLogin(Player* player, bool firstLogin)
    {
        if (player->getRace() == RACE_PANDAREN_NEUTRAL)
        {
            // A new Panda join the world ! Fabulous...
            if (firstLogin)
            {
                player->ShowNeutralPlayerFactionSelectUI();
       

            }
        }
   
    }
};



// Add some script here
void AddSC_legionhearth()
{
	new Player_perm();
    new Player_panda();
}
