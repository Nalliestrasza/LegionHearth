/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

/* ScriptData
Name: gobject_commandscript
%Complete: 100
Comment: All gobject related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameEventMgr.h"
#include "GameObject.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "PoolMgr.h"
#include "RBAC.h"
#include "WorldSession.h"
#include <sstream>

class gobject_commandscript : public CommandScript
{
public:
    gobject_commandscript() : CommandScript("gobject_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> gobjectAddCommandTable =
        {
            { "temp", rbac::RBAC_PERM_COMMAND_GOBJECT_ADD_TEMP, false, &HandleGameObjectAddTempCommand,   "" },
            { "",     rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, &HandleGameObjectAddCommand,       "" },
        };
        static std::vector<ChatCommand> gobjectSetCommandTable =
        {
            { "phase", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_PHASE, false, &HandleGameObjectSetPhaseCommand,  "" },
            { "state", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_STATE, false, &HandleGameObjectSetStateCommand,  "" },
            { "scale", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_SCALE, false, &HandleGameObjectSetScaleCommand,  "" },
        };
        static std::vector<ChatCommand> gobjectDuplicationCommandTable =
        {
            { "create", rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_MANAGE ,  false, &HandleGameObjectDuplicationCreateCommand,   "" },
            { "add",    rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_SPAWN,    false, &HandleGameObjectDuplicationAddCommand,      "" },
            { "target", rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_READ,     false, &HandleGameObjectDuplicationTargetCommand,   "" },
            { "delete", rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_SPAWN,    false, &HandleGameObjectDuplicationDeleteCommand,   "" },
            { "private",rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_MANAGE ,  false, &HandleGameObjectDuplicationPrivateCommand,  "" },
            { "remove", rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_MANAGE ,  false, &HandleGameObjectDuplicationRemoveCommand,   "" },
        };
        static std::vector<ChatCommand> gobjectCommandTable =
        {
            { "activate",   rbac::RBAC_PERM_COMMAND_GOBJECT_ACTIVATE, false, &HandleGameObjectActivateCommand,  ""       },
            { "delete",     rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,   false, &HandleGameObjectDeleteCommand,    ""       },
            { "info",       rbac::RBAC_PERM_COMMAND_GOBJECT_INFO,     false, &HandleGameObjectInfoCommand,      ""       },
            { "move",       rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,     false, &HandleGameObjectMoveCommand,      ""       },
            { "near",       rbac::RBAC_PERM_COMMAND_GOBJECT_NEAR,     false, &HandleGameObjectNearCommand,      ""       },
            { "target",     rbac::RBAC_PERM_COMMAND_GOBJECT_TARGET,   false, &HandleGameObjectTargetCommand,    ""       },
            { "rotate",     rbac::RBAC_PERM_COMMAND_GOBJECT_TURN,     false, &HandleGameObjectTurnCommand,      ""       },
            { "add",        rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, NULL,            "", gobjectAddCommandTable },
            { "set",        rbac::RBAC_PERM_COMMAND_GOBJECT_SET,      false, NULL,            "", gobjectSetCommandTable },
            { "dupplicate", rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, NULL,    "", gobjectDuplicationCommandTable },
            { "raz",        rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,   false, &HandleGameRazCommand,            ""        },
            { "doodad",     rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,   false, &HandleGameDoodadCommand,         ""        },

        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gobject", rbac::RBAC_PERM_COMMAND_GOBJECT, false, NULL, "", gobjectCommandTable },
        };
        return commandTable;
    }

    static bool HandleGameObjectActivateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32_t const autoCloseTime = object->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;

        // Activate
        object->SetLootState(GO_READY);
        object->UseDoorOrButton(autoCloseTime, false, handler->GetSession()->GetPlayer());

        handler->PSendSysMessage("Object activated!");

        return true;
    }

    //spawn go
    static bool HandleGameObjectAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        if (!id)
            return false;

        uint32 objectId = atoul(id);
        if (!objectId)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "%s a .gob add %d", handler->GetSession()->GetPlayer()->GetName().c_str(), objectId);

        char* spawntimeSecs = strtok(NULL, " ");

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);
        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            // report to DB errors log as in loading case
            TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", objectId, objectInfo->type, objectInfo->displayId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_HAVE_INVALID_DATA, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();

        GameObject* object = GameObject::CreateGameObject(objectInfo->entry, map, *player, QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.0f, 0.0f), 255, GO_STATE_READY);
        if (!object)
            return false;

        PhasingHandler::InheritPhaseShift(object, player);

        if (spawntimeSecs)
        {
            int32 value = atoi(spawntimeSecs);
            object->SetRespawnTime(value);
        }

        // fill the gameobject data and save to the db
        object->SaveToDB(map->GetId(), { map->GetDifficultyID() });
        ObjectGuid::LowType spawnId = object->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        // this will generate a new guid if the object is in an instance
        object = GameObject::CreateGameObjectFromDB(spawnId, map);
        if (!object)
            return false;

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGOData(spawnId)));

        // Log
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        uint64 spawnerGuid = player->GetSession()->GetPlayer()->GetGUID().GetCounter();

        WorldDatabasePreparedStatement* gobInfo = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_LOG);
        gobInfo->setUInt64(0, spawnId);
        gobInfo->setUInt32(1, spawnerAccountId);
        gobInfo->setUInt64(2, spawnerGuid);
        WorldDatabase.Execute(gobInfo);

        handler->PSendSysMessage(LANG_GAMEOBJECT_ADD, objectId, objectInfo->name.c_str(), std::to_string(spawnId).c_str(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
        return true;

    }

    // add go, temp only
    static bool HandleGameObjectAddTempCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* spawntime = strtok(NULL, " ");
        uint32 spawntm = 300;

        if (spawntime)
            spawntm = atoul(spawntime);

        uint32 objectId = atoul(id);

        if (!sObjectMgr->GetGameObjectTemplate(objectId))
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->SummonGameObject(objectId, *player, QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.0f, 0.0f), spawntm);

        return true;
    }

    static bool HandleGameObjectTargetCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        QueryResult result;
        GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();

        if (*args)
        {
            // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
            char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
            if (!id)
                return false;

            uint32 objectId = atoul(id);

            if (objectId)
                result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
                player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), objectId);
            else
            {
                std::string name = id;
                WorldDatabase.EscapeString(name);
                result = WorldDatabase.PQuery(
                    "SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
                    "FROM gameobject LEFT JOIN gameobject_template ON gameobject_template.entry = gameobject.id WHERE map = %i AND name LIKE '%%%s%%' ORDER BY order_ ASC LIMIT 1",
                    player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), name.c_str());
            }
        }
        else
        {
            std::ostringstream eventFilter;
            eventFilter << " AND (eventEntry IS NULL ";
            bool initString = true;

            for (GameEventMgr::ActiveEvents::const_iterator itr = activeEventsList.begin(); itr != activeEventsList.end(); ++itr)
            {
                if (initString)
                {
                    eventFilter  <<  "OR eventEntry IN (" << *itr;
                    initString = false;
                }
                else
                    eventFilter << ',' << *itr;
            }

            if (!initString)
                eventFilter << "))";
            else
                eventFilter << ')';

            result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, "
                "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
                "LEFT OUTER JOIN game_event_gameobject on gameobject.guid = game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 10",
                handler->GetSession()->GetPlayer()->GetPositionX(), handler->GetSession()->GetPlayer()->GetPositionY(), handler->GetSession()->GetPlayer()->GetPositionZ(),
                handler->GetSession()->GetPlayer()->GetMapId(), eventFilter.str().c_str());
            
            
        }

        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
            return true;
        }

        bool found = false;
        float x, y, z, o;
        ObjectGuid::LowType guidLow;
        uint32 id, phaseId, phaseGroup, spawnerAccountId;
        uint64 spawnerPlayerId;
        uint16 mapId;
        uint32 poolId;

        do
        {
            

            Field* fields = result->Fetch();
            guidLow =       fields[0].GetUInt64();
            id =            fields[1].GetUInt32();
            x =             fields[2].GetFloat();
            y =             fields[3].GetFloat();
            z =             fields[4].GetFloat();
            o =             fields[5].GetFloat();
            mapId =         fields[6].GetUInt16();
            phaseId =       fields[7].GetUInt32();
            phaseGroup =    fields[8].GetUInt32();
            poolId =  sPoolMgr->IsPartOfAPool<GameObject>(guidLow);
            if (!poolId || sPoolMgr->IsSpawnedObject<GameObject>(guidLow))
                found = true;
        } while (result->NextRow() && !found);

        if (!found)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
            return false;
        }

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(id);

        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
            return false;
        }

        GameObject* target = handler->GetObjectFromPlayerMapByDbGuid(guidLow);


        // log info
        QueryResult logResult = WorldDatabase.PQuery("SELECT spawnerAccountId, spawnerPlayerId from gameobject_log WHERE guid = %u", guidLow);       

        if (!logResult)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL, std::to_string(guidLow).c_str(), objectInfo->name.c_str(), std::to_string(guidLow).c_str(), id, x, y, z, mapId, o, phaseId, phaseGroup);
        }
        else
        {
            Field* log = logResult->Fetch();
            spawnerAccountId = log[0].GetUInt32();
            spawnerPlayerId = log[1].GetUInt64();

            handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL, std::to_string(guidLow).c_str(), objectInfo->name.c_str(), std::to_string(guidLow).c_str(), id, x, y, z, mapId, o, phaseId, phaseGroup);
            handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL_PID, spawnerPlayerId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL_ACCID, spawnerAccountId);

            QueryResult getName = CharacterDatabase.PQuery("SELECT name FROM characters WHERE guid = %u", spawnerPlayerId);
            
            if (!getName)
            {
                handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL_DELETE_PNAME);
            }
            else
            {
				Field* fields = getName->Fetch();
				std::string spawnerName;
				spawnerName = fields[0].GetString();
                handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL_PNAME, spawnerName);
            }
        }
        
      

        if (target)
        {
            int32 curRespawnDelay = int32(target->GetRespawnTimeEx() - time(NULL));
            if (curRespawnDelay < 0)
                curRespawnDelay = 0;

            std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay, true);
            std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

            handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        }
        return true;
    }

    //delete object by selection or guid
    static bool HandleGameObjectDeleteCommand(ChatHandler* handler, char const* args)
    {
        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid ownerGuid = object->GetOwnerGUID();
        if (!ownerGuid.IsEmpty())
        {
            Unit* owner = ObjectAccessor::GetUnit(*handler->GetSession()->GetPlayer(), ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
            {
                handler->PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, ownerGuid.ToString().c_str(), object->GetGUID().ToString().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            owner->RemoveGameObject(object, false);
        }

        object->SetRespawnTime(0);                                 // not save respawn time
        object->Delete();
        object->DeleteFromDB();

        //Del from gameobject_raz
        WorldDatabasePreparedStatement* gobLog = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_LOG);
        gobLog->setUInt64(0, guidLow);
        WorldDatabase.Execute(gobLog);

        handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, std::to_string(guidLow).c_str());

        return true;
    }

    //turn selected object
    static bool HandleGameObjectTurnCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* orientation = strtok(NULL, " ");
        float oz = 0.f, oy = 0.f, ox = 0.f;

        if (orientation)
        {
            oz = float(atof(orientation));

            orientation = strtok(NULL, " ");
            if (orientation)
            {
                oy = float(atof(orientation));
                orientation = strtok(NULL, " ");
                if (orientation)
                    ox = float(atof(orientation));
            }
        }
        else
        {
            Player* player = handler->GetSession()->GetPlayer();
            oz = player->GetOrientation();
        }

        // LegionHearth conversion
        double toRad = (M_PI / 180);

        Map* map = object->GetMap();

        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), oz);
        //object->SetWorldRotationAngles(oz, oy, ox);
        object->SetWorldRotationAngles(oz * toRad, oy * toRad, ox * toRad);
        object->SaveToDB();

        // Generate a completely new spawn with new guid
        // 3.3.5a client caches recently deleted objects and brings them back to life
        // when CreateObject block for this guid is received again
        // however it entirely skips parsing that block and only uses already known location
        object->Delete();

        object = GameObject::CreateGameObjectFromDB(guidLow, map);
        if (!object)
            return false;

        handler->PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, std::to_string(object->GetSpawnId()).c_str(), object->GetGOInfo()->name.c_str(), object->GetGUID().ToString().c_str(), object->GetOrientation());
        return true;

    }

    //move selected object
    static bool HandleGameObjectMoveCommand(ChatHandler* handler, char const* args)
    {
			// number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
			char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
			if (!id)
				return false;

			ObjectGuid::LowType guidLow = atoull(id);
			if (!guidLow)
				return false;

			GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
			if (!object)
			{
				handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
				handler->SetSentErrorMessage(true);
				return false;
			}

			char* toX = strtok(NULL, " ");
			char* toY = strtok(NULL, " ");
			char* toZ = strtok(NULL, " ");

			float x, y, z;
			if (!toX)
			{
				Player* player = handler->GetSession()->GetPlayer();
				player->GetPosition(x, y, z);
			}
			else
			{
				if (!toY || !toZ)
					return false;

				x = (float)atof(toX);
				y = (float)atof(toY);
				z = (float)atof(toZ);

				if (!MapManager::IsValidMapCoord(object->GetMapId(), x, y, z))
				{
					handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, object->GetMapId());
					handler->SetSentErrorMessage(true);
					return false;
				}
			}

            Map* map = object->GetMap();

            object->Relocate(x, y, z, object->GetOrientation());
            object->SaveToDB();

            // Generate a completely new spawn with new guid
            // 3.3.5a client caches recently deleted objects and brings them back to life
            // when CreateObject block for this guid is received again
            // however it entirely skips parsing that block and only uses already known location

            object->Delete();

            object = GameObject::CreateGameObjectFromDB(guidLow, map);
            if (!object)
                return false;

            handler->PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, std::to_string(object->GetSpawnId()).c_str(), object->GetGOInfo()->name.c_str(), object->GetGUID().ToString().c_str());
            return true;
	}
       

    //set phasemask for selected object
    static bool HandleGameObjectSetPhaseCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        /*// number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* phase = strtok (NULL, " ");
        uint32 phaseMask = phase ? atoul(phase) : 0;
        if (phaseMask == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        object->SetPhaseMask(phaseMask, true);
        object->SaveToDB();*/
        return true;
    }

    static bool HandleGameObjectNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt32(3, player->GetMapId());
        stmt->setFloat(4, player->GetPositionX());
        stmt->setFloat(5, player->GetPositionY());
        stmt->setFloat(6, player->GetPositionZ());
        stmt->setFloat(7, distance * distance);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

                if (!gameObjectInfo)
                    continue;

                handler->PSendSysMessage(LANG_GO_LIST_CHAT, std::to_string(guid).c_str(), entry, std::to_string(guid).c_str(), gameObjectInfo->name.c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE, distance, count);
        return true;
    }

    //show info of gameobject
    static bool HandleGameObjectInfoCommand(ChatHandler* handler, char const* args)
    {
        uint32 entry = 0;
        uint32 type = 0;
        uint32 displayId = 0;
        std::string name;
        uint32 lootId = 0;

        if (!*args)
            return false;

        char* param1 = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        if (!param1)
            return false;

        if (strcmp(param1, "guid") == 0)
        {
            char* tail = strtok(nullptr, "");
            char* cValue = handler->extractKeyFromLink(tail, "Hgameobject");
            if (!cValue)
                return false;
            ObjectGuid::LowType guidLow = atoull(cValue);
            const GameObjectData* data = sObjectMgr->GetGOData(guidLow);
            if (!data)
                return false;
            entry = data->id;
        }
        else
        {
            entry = atoul(param1);
        }

        GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!gameObjectInfo)
            return false;

        type = gameObjectInfo->type;
        displayId = gameObjectInfo->displayId;
        name = gameObjectInfo->name;
        lootId = gameObjectInfo->GetLootId();

        handler->PSendSysMessage(LANG_GOINFO_ENTRY, entry);
        handler->PSendSysMessage(LANG_GOINFO_TYPE, type);
        handler->PSendSysMessage(LANG_GOINFO_LOOTID, lootId);
        handler->PSendSysMessage(LANG_GOINFO_DISPLAYID, displayId);
        handler->PSendSysMessage(LANG_GOINFO_NAME, name.c_str());
        handler->PSendSysMessage(LANG_GOINFO_SIZE, gameObjectInfo->size);

        if (GameObjectTemplateAddon const* addon = sObjectMgr->GetGameObjectTemplateAddon(entry))
            handler->PSendSysMessage(LANG_GOINFO_ADDON, addon->faction, addon->flags);

        if (GameObjectDisplayInfoEntry const* modelInfo = sGameObjectDisplayInfoStore.LookupEntry(displayId))
            handler->PSendSysMessage(LANG_GOINFO_MODEL, modelInfo->GeoBoxMax.X, modelInfo->GeoBoxMax.Y, modelInfo->GeoBoxMax.Z, modelInfo->GeoBoxMin.X, modelInfo->GeoBoxMin.Y, modelInfo->GeoBoxMin.Z);

        return true;
    }

    static bool HandleGameObjectSetStateCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* type = strtok(NULL, " ");
        if (!type)
            return false;

        int32 objectType = atoi(type);
        if (objectType < 0)
        {
            if (objectType == -1)
                object->SendGameObjectDespawn();
            else if (objectType == -2)
                return false;
            return true;
        }

        char* state = strtok(NULL, " ");
        if (!state)
            return false;

        int32 objectState = atoi(state);

        switch (objectType)
        {
            case 0:
                object->SetGoState(GOState(objectState));
                break;
            case 1:
                object->SetGoType(GameobjectTypes(objectState));
                break;
            case 2:
                object->SetGoArtKit(objectState);
                break;
            case 3:
                object->SetGoAnimProgress(objectState);
                break;
            case 4:
                object->SendCustomAnim(objectState);
                break;
            case 5:
                if (objectState < 0 || objectState > GO_DESTRUCTIBLE_REBUILDING)
                    return false;

                object->SetDestructibleState(GameObjectDestructibleState(objectState));
                break;
            default:
                break;
        }

        handler->PSendSysMessage("Set gobject type %d state %d", objectType, objectState);
        return true;
    }

    static bool HandleGameObjectSetScaleCommand(ChatHandler* handler, char const* args)
    {
        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }
            
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* scale_temp = strtok(NULL, " ");
        if (!scale_temp)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        float scale = atof(scale_temp);

        if (scale <= 0.0f)
        {
            scale = object->GetGOInfo()->size;
            const_cast<GameObjectData*>(object->GetGOData())->size = -1.0f;
        }
        else
        {
            const_cast<GameObjectData*>(object->GetGOData())->size = scale;
        }


        Map* map = object->GetMap();

        object->SetObjectScale(scale);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        object->SaveToDB();

        // Generate a completely new spawn with new guid
        // 3.3.5a client caches recently deleted objects and brings them back to life
        // when CreateObject block for this guid is received again
        // however it entirely skips parsing that block and only uses already known location
        object->Delete();

        object = GameObject::CreateGameObjectFromDB(guidLow, map);
        if (!object)
            return false;

        handler->PSendSysMessage("Set %s scale to %f", object->GetGUID().ToString(), scale);
        return true;

    }

    static bool HandleGameDoodadCommand(ChatHandler* handler, char const* args)
    {

        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        char const* boolArg = strtok(NULL, "");
        if (!boolArg)
            return false;


        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        bool resultBool = true;

        if (strncmp(boolArg, "on", 3) == 0)
        {
            resultBool = true;
        }
        else if (strncmp(boolArg, "off", 4) == 0) {
            resultBool = false;
        }
        else
        {
            handler->SendSysMessage(LANG_USE_BOL);
            handler->SetSentErrorMessage(true);
            return false;
        }

        const_cast<GameObjectData*>(object->GetGOData())->hasDoodads = resultBool;

        Map* map = object->GetMap();

        object->SetDoodads(resultBool);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        object->SaveToDB();

        // Generate a completely new spawn with new guid
        // 3.3.5a client caches recently deleted objects and brings them back to life
        // when CreateObject block for this guid is received again
        // however it entirely skips parsing that block and only uses already known location
        object->Delete();

        object = GameObject::CreateGameObjectFromDB(guidLow, map);
        if (!object)
            return false;

        handler->PSendSysMessage("Doodad for object %s %s \n", object->GetGUID().ToString().c_str(), object->HasDoodads() ? "ON" : "OFF");
    }

    static bool HandleGameRazCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        QueryResult getGuid = WorldDatabase.PQuery("SELECT guid from gameobject_log WHERE spawnerAccountId = %u", player->GetSession()->GetAccountId());
        if (getGuid)
        {
            do {
                
                Field* field = getGuid->Fetch();
                uint64 guidLow = field[0].GetUInt64();

                if (guidLow == 0)
                    return false;

                GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                object->SetRespawnTime(0);                                 // not save respawn time
                object->Delete();
                object->DeleteFromDB();

                WorldDatabasePreparedStatement* del = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_LOG);
                del->setUInt64(0, guidLow);
                WorldDatabase.Execute(del);

                handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, std::to_string(guidLow).c_str());

            } while (getGuid->NextRow());
        }
        else
        {
            return false;
        }

        return true;
    }

    static bool HandleGameObjectDuplicationCreateCommand(ChatHandler* handler, char const* args)
    {
        //1  COMMANDS PARAMETERS

        // Player
        Player* player = handler->GetSession()->GetPlayer();
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();

        //first parameter : gob guid

        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = atoull(id);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        //second parameter : range
        char* scale_temp = strtok(NULL, " ");
        if (!scale_temp)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 range = atoul(scale_temp);

        // Security
        if (range > 150)
        {
            handler->SendSysMessage(LANG_DUPPLICATION_CREATE_RANGE_ERR_FAR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        //third parameter : name
        std::string dupName = "";
        char const* targetName = strtok(NULL, "");
        if (targetName && targetName != NULL)
            dupName = targetName;
        else
            dupName = "Dupplication" + player->GetName();

        if (dupName.size() > 50)
        {
            handler->SendSysMessage(LANG_DUPPLICATION_CREATE_NAME_LONG);
            handler->SetSentErrorMessage(true);
            return false;
        }



        //2 DOODADS
        // auto-increment
        WorldDatabasePreparedStatement* stmtmax = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE_MAX_ID);
        PreparedQueryResult resultmax = WorldDatabase.Query(stmtmax);
        uint32 tId = resultmax->Fetch()->GetUInt32();
        ++tId;


        // Doodads next to reference object
        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST_ADVANCED);
        stmt->setFloat(0, object->GetPositionX());
        stmt->setFloat(1, object->GetPositionY());
        stmt->setFloat(2, object->GetPositionZ());
        stmt->setUInt32(3, object->GetMapId());
        stmt->setFloat(4, object->GetPositionX());
        stmt->setFloat(5, object->GetPositionY());
        stmt->setFloat(6, object->GetPositionZ());
        stmt->setUInt32(7, range * range);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            //Security
            if (result->GetRowCount() > (uint64)1000)
            {
                handler->SendSysMessage(LANG_DUPPLICATION_CREATE_RANGE_ERR_DOODADS);
                handler->SetSentErrorMessage(true);
                return false;
            }

            //test query
            WorldDatabasePreparedStatement* insertdup = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_TEMPLATE);
            insertdup->setUInt32(0, tId);
            insertdup->setString(1, dupName);
            insertdup->setUInt32(2, spawnerAccountId);
            insertdup->setFloat(3, object->GetEntry());
            insertdup->setFloat(4, object->GetPositionZ());
            insertdup->setFloat(5, object->GetOrientation());
            insertdup->setFloat(6, sObjectMgr->GetGOData(guidLow)->size);
            insertdup->setString(7, player->GetName());
            WorldDatabase.Execute(insertdup);

            do
            {
                Field* fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();
                float o = fields[6].GetFloat();
                float size = fields[7].GetFloat();
                float rotation0 = fields[8].GetFloat();
                float rotation1 = fields[9].GetFloat();
                float rotation2 = fields[10].GetFloat();
                float rotation3 = fields[11].GetFloat();

                // Don't add the reference object to doodad list
                if (guidLow != guid)
                {
                    WorldDatabasePreparedStatement* insertdood = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_DOODADS);
                    insertdood->setUInt32(0, tId);
                    insertdood->setUInt32(1, entry);
                    insertdood->setFloat(2, x - object->GetPositionX());
                    insertdood->setFloat(3, y - object->GetPositionY());
                    insertdood->setFloat(4, z - object->GetPositionZ());
                    insertdood->setFloat(5, o - object->GetOrientation());
                    insertdood->setFloat(6, size);
                    insertdood->setFloat(7, rotation0);
                    insertdood->setFloat(8, rotation1);
                    insertdood->setFloat(9, rotation2);
                    insertdood->setFloat(10, rotation3);
                    insertdood->setFloat(11, sqrt(pow(x - object->GetPositionX(), 2) + pow(y - object->GetPositionY(), 2)));
                    insertdood->setFloat(12, std::atan2(x - object->GetPositionX(), y - object->GetPositionY()));
                    WorldDatabase.Execute(insertdood);
                }
                
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_DUPPLICATION_CREATE_SUCCESS, tId, dupName.c_str(), player->GetName());
        return true;
    }

    static bool HandleGameObjectDuplicationAddCommand(ChatHandler* handler, char const* args)
    {

        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Player
        Player* player = handler->GetSession()->GetPlayer();
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        uint64 spawnerGuid = player->GetSession()->GetPlayer()->GetGUID().GetCounter();

        //first parameter : dupplicate template id
        if (!*args)
            return false;

        char const* pId = strtok((char*)args, " ");
        uint32 dupId = atoi(pId);

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE);
        stmt->setUInt32(0, dupId);
        PreparedQueryResult duppTemplate = WorldDatabase.Query(stmt);

        if (!duppTemplate)
        {
            handler->PSendSysMessage(LANG_DUPPLICATION_ADD_ERROR, dupId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Field* dupfields = duppTemplate->Fetch();
        std::string dupName = dupfields[0].GetString();
        uint32 refEntry = dupfields[1].GetUInt32();
        float refPosZ = dupfields[2].GetFloat();
        float refOrientation = dupfields[3].GetFloat();
        float refSize = dupfields[4].GetFloat();

        uint8 isPriv = dupfields[5].GetUInt8();
        uint32 accountId = dupfields[6].GetUInt32();

        if (isPriv == 1 && accountId != spawnerAccountId)
        {
            handler->PSendSysMessage(LANG_DUPPLICATION_ADD_PRIVATE, dupId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Create Reference Copy Object where the player is

        // Création de l'objet "object" 
        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(refEntry);

        // on définit ici la position, etc... du gameobject ainsi que son entry. 
        GameObject* object = GameObject::CreateGameObject(objectInfo->entry, player->GetMap(), *player, QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.0f, 0.0f), 255, GO_STATE_READY);

        // sans ça mon serveur copiait l'intégrité de la map 0 (royaume de l'est) :lmaofam: 
        PhasingHandler::InheritPhaseShift(object, player);

        // on récup le scale 
        object->SetObjectScale(refSize);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        object->SaveToDB(player->GetMap()->GetId(), { player->GetMap()->GetDifficultyID() });

        ObjectGuid::LowType spawnId = object->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance. 
        // this is required to avoid weird behavior and memory leaks 
        delete object;

        object = GameObject::CreateGameObjectFromDB(spawnId, player->GetMap());

        sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGOData(spawnId)));

        // Dupplication infos (for delete command)
        // Guid Max select
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_GUID_MAX_ID);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        uint32 dupplicationGuid = result->Fetch()->GetUInt32();

        // Create point
        WorldDatabasePreparedStatement* duppGuid = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_GUID);
        duppGuid->setUInt32(0, dupplicationGuid + 1);
        duppGuid->setUInt32(1, dupId);
        duppGuid->setUInt64(2, spawnId);
        duppGuid->setString(3, dupName);
        duppGuid->setString(4, player->GetName());
        duppGuid->setFloat(5, player->GetPositionX());
        duppGuid->setFloat(6, player->GetPositionY());
        duppGuid->setFloat(7, player->GetPositionZ());
        duppGuid->setUInt16(8, player->GetMapId());
        WorldDatabase.Execute(duppGuid);

        // Log
        WorldDatabasePreparedStatement* gobInfo = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_LOG_DUPPLICATION_GUID);
        gobInfo->setUInt64(0, spawnId);
        gobInfo->setUInt32(1, spawnerAccountId);
        gobInfo->setUInt64(2, spawnerGuid);
        gobInfo->setUInt32(3, dupplicationGuid + 1);
        WorldDatabase.Execute(gobInfo);


        WorldDatabasePreparedStatement* stmt2 = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_DOODADS);
        stmt2->setUInt32(0, dupId);
        PreparedQueryResult duppDoodads = WorldDatabase.Query(stmt2);
        if (duppDoodads)
        {
            do
            {
                Field* fields = duppDoodads->Fetch();
                uint32 doodEntry = fields[0].GetUInt32();
                float doodDiffX = fields[1].GetFloat();
                float doodDiffY = fields[2].GetFloat();
                float doodDiffZ = fields[3].GetFloat();
                float doodDiffO = fields[4].GetFloat();
                float doodSize = fields[5].GetFloat();
                float doodRotX = fields[6].GetFloat();
                float doodRotY = fields[7].GetFloat();
                float doodRotZ = fields[8].GetFloat();
                float doodRotW = fields[9].GetFloat();
                float doodDist = fields[10].GetFloat();
                float doodAngle = fields[11].GetFloat();

                // Check if object exists !
                GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(doodEntry);
                if (!objectInfo)
                    continue;

                // New angle calcul
                float angle = (M_PI / 2) - (doodAngle - (player->GetOrientation() - refOrientation));

                // PosX and PosY calcul
                float doodPosX = player->GetPositionX() + (doodDist * cos(angle));
                float doodPosY = player->GetPositionY() + (doodDist * sin(angle));
                float doodPosZ = doodDiffZ  + player->GetPositionZ();
                float doodOrientation = doodDiffO + player->GetOrientation();

                // Rotation
                float yaw, pitch, roll;
                QuaternionData(doodRotX, doodRotY, doodRotZ, doodRotW).toEulerAnglesZYX(yaw, pitch, roll);

                // Create Object

                // Création de l'objet "object" 
                GameObjectTemplate const* objectInfoDood = sObjectMgr->GetGameObjectTemplate(doodEntry);

                // on définit ici la position, etc... du gameobject ainsi que son entry.
                GameObject* object2 = GameObject::CreateGameObject(objectInfoDood->entry, player->GetMap(), Position(doodPosX, doodPosY, doodPosZ, doodOrientation), QuaternionData::fromEulerAnglesZYX(yaw+(player->GetOrientation() - refOrientation), pitch, roll), 255, GO_STATE_READY);

                // sans ça mon serveur copiait l'intégrité de la map 0 (royaume de l'est) :lmaofam: 
                PhasingHandler::InheritPhaseShift(object2, player);

                // on récup le scale 
                object2->SetObjectScale(doodSize);
                object2->Relocate(object2->GetPositionX(), object2->GetPositionY(), object2->GetPositionZ(), object2->GetOrientation());
                object2->SaveToDB(player->GetMap()->GetId(), { player->GetMap()->GetDifficultyID() });

                spawnId = object2->GetSpawnId();

                // delete the old object and do a clean load from DB with a fresh new GameObject instance. 
                // this is required to avoid weird behavior and memory leaks 
                delete object2;

                object2 = GameObject::CreateGameObjectFromDB(spawnId, player->GetMap());

                sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGOData(spawnId)));

                // Log
                WorldDatabasePreparedStatement* gobInfo = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_LOG_DUPPLICATION_GUID);
                gobInfo->setUInt64(0, spawnId);
                gobInfo->setUInt32(1, spawnerAccountId);
                gobInfo->setUInt64(2, spawnerGuid);
                gobInfo->setUInt32(3, dupplicationGuid+1);
                WorldDatabase.Execute(gobInfo);

            } while (duppDoodads->NextRow());
        }

        handler->PSendSysMessage(LANG_DUPPLICATION_ADD_SUCCESS, dupplicationGuid + 1, dupId, dupName.c_str());
        return true;
    }

    static bool HandleGameObjectDuplicationTargetCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_GUID);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt16(3, player->GetMapId());
        PreparedQueryResult duppTarget = WorldDatabase.Query(stmt);

        if (!duppTarget)
        {
            handler->SendSysMessage(LANG_DUPPLICATION_TARGET_ERROR);
            return true;
        }

        Field* dupfields = duppTarget->Fetch();
        
        uint32 duppGuid = dupfields[0].GetUInt32();
        uint32 duppEntry = dupfields[1].GetUInt32();
        uint64 refObjGuid = dupfields[2].GetUInt64();
        std::string dupName = dupfields[3].GetString();
        float duppX = dupfields[4].GetFloat();
        float duppY = dupfields[5].GetFloat();
        float duppZ = dupfields[6].GetFloat();

        // Supprimer au cas ou dupplication vide
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_LOG);
        stmt->setUInt32(0, duppGuid);
        PreparedQueryResult getGuid = WorldDatabase.Query(stmt);
        if (!getGuid)
        {
            // Delete dupplication guid
            WorldDatabasePreparedStatement* psDupGuid = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_GUID);
            psDupGuid->setUInt32(0, duppGuid);
            WorldDatabase.Execute(psDupGuid);

            handler->PSendSysMessage(LANG_DUPPLICATION_TARGET_EMPTY);

            return true;
        }

        // Distance
        
        double playerX = (trunc((handler->GetSession()->GetPlayer()->GetPositionX()) * 10 * 0.9144)) / 10;
        double playerY = (trunc((handler->GetSession()->GetPlayer()->GetPositionY()) * 10 * 0.9144)) / 10;
        double playerZ = (trunc((handler->GetSession()->GetPlayer()->GetPositionZ()) * 10 * 0.9144)) / 10;
        double targetX = (trunc((duppX) * 10 * 0.9144)) / 10;
        double targetY = (trunc((duppY) * 10 * 0.9144)) / 10;
        double targetZ = (trunc((duppZ) * 10 * 0.9144)) / 10;

        double distanceFull = sqrt((pow((playerX - targetX), 2)) + (pow((playerY - targetY), 2)) + (pow((playerZ - targetZ), 2)));
        double distance = (trunc(distanceFull * 10)) / 10;

        //Phase 4 : Envoi du message
        if (distance > 300)
            handler->SendSysMessage(LANG_DUPPLICATION_TARGET_FAR);
        else
        { 
            if (distance < 1)
                handler->PSendSysMessage(LANG_DUPPLICATION_TARGET_CLOSE, distance);
            else
                handler->PSendSysMessage(LANG_DUPPLICATION_TARGET_HERE, distance);

            // Infos
            handler->PSendSysMessage(LANG_DUPPLICATION_TARGET_SUCCESS, dupName.c_str(), duppGuid, duppEntry,duppX,duppY,duppZ, player->GetMapId());
        }
        return true;
    }

    static bool HandleGameObjectDuplicationDeleteCommand(ChatHandler* handler, char const* args)
    {
        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Player
        Player* player = handler->GetSession()->GetPlayer();
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        uint64 spawnerGuid = player->GetSession()->GetPlayer()->GetGUID().GetCounter();

        //first parameter : dupplication guid
        if (!*args)
            return false;

        char const* pId = strtok((char*)args, " ");
        uint32 dupGuid = atoi(pId);


        // Delete all objects
        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_LOG);
        stmt->setUInt32(0, dupGuid);
        PreparedQueryResult getGuid = WorldDatabase.Query(stmt);
        if (getGuid)
        {
            do {

                Field* field = getGuid->Fetch();
                uint64 guidLow = field[0].GetUInt64();

                if (guidLow == 0)
                    return false;

                GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                object->SetRespawnTime(0);                                 // not save respawn time
                object->Delete();
                object->DeleteFromDB();

                WorldDatabasePreparedStatement* del = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_LOG);
                del->setUInt64(0, guidLow);
                WorldDatabase.Execute(del);

                // Flood chat in case of too many gameobjects
                //handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, std::to_string(guidLow).c_str());

            } while (getGuid->NextRow());

            // Delete dupplication guid
            WorldDatabasePreparedStatement* psDupGuid = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_GUID);
            psDupGuid->setUInt32(0, dupGuid);
            WorldDatabase.Execute(psDupGuid);

            handler->PSendSysMessage(LANG_DUPPLICATION_DELETE_SUCCESS, dupGuid);

        }
        else
        {
            handler->PSendSysMessage(LANG_DUPPLICATION_DELETE_ERROR, dupGuid);
            return true;
        }

        // Clean DB
        WorldDatabasePreparedStatement* cleanDupGuid = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATIONS);
        WorldDatabase.Execute(cleanDupGuid);

        return true;
    }

    static bool HandleGameObjectDuplicationPrivateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* pId = strtok((char*)args, " ");
        uint32 dupEntry = atoi(pId);

        char const* boolArg = strtok(NULL, "");
        if (!boolArg)
            return false;

        
        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE_ACCOUNT);
        stmt->setUInt32(0, dupEntry);
        stmt->setUInt32(1, handler->GetSession()->GetAccountId());
        PreparedQueryResult duppAccount = WorldDatabase.Query(stmt);

        if (!duppAccount)
        {
            handler->PSendSysMessage(LANG_DUPPLICATION_ERROR_OR_PRIVATE, dupEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            uint8 boolean = 0;

            if (strncmp(boolArg, "on", 3) == 0)
            {
                handler->PSendSysMessage(LANG_DUPPLICATION_PRIVATE_ON, dupEntry);
                boolean = 1;
            }
            else if (strncmp(boolArg, "off", 4) == 0)
                handler->PSendSysMessage(LANG_DUPPLICATION_PRIVATE_OFF, dupEntry);
            else
            {
                handler->SendSysMessage(LANG_USE_BOL);
                handler->SetSentErrorMessage(true);
                return false;
            }

            WorldDatabasePreparedStatement* updSkybox = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_TEMPLATE);
            updSkybox->setUInt32(0, boolean);
            updSkybox->setUInt64(1, dupEntry);
            WorldDatabase.Execute(updSkybox);

            return true;
        }

    }

    static bool HandleGameObjectDuplicationRemoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* pId = strtok((char*)args, " ");
        uint32 dupEntry = atoi(pId);

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE_ACCOUNT);
        stmt->setUInt32(0, dupEntry);
        stmt->setUInt32(1, handler->GetSession()->GetAccountId());
        PreparedQueryResult duppAccount = WorldDatabase.Query(stmt);

        if (!duppAccount)
        {
            handler->PSendSysMessage(LANG_DUPPLICATION_ERROR_OR_PRIVATE,dupEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            // dupplication_template
            WorldDatabasePreparedStatement* psDupTemplate = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATION_TEMPLATE);
            psDupTemplate->setUInt32(0, dupEntry);
            WorldDatabase.Execute(psDupTemplate);

            // dupplication_doodads
            WorldDatabasePreparedStatement* psDupDoodads = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATION_DOODADS);
            psDupDoodads->setUInt32(0, dupEntry);
            WorldDatabase.Execute(psDupDoodads);

            // dupplication_guid
            WorldDatabasePreparedStatement* psDupGuid = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_GUID_ENTRY);
            psDupGuid->setUInt32(0, dupEntry);
            WorldDatabase.Execute(psDupGuid);

            handler->PSendSysMessage(LANG_DUPPLICATION_REMOVE_SUCCESS, dupEntry);
        }

        return true;
    }
  
};

void AddSC_gobject_commandscript()
{
    new gobject_commandscript();
}
