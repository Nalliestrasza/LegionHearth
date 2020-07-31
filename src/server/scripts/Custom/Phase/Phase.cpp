#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceSaveMgr.h"
#include "IpAddress.h"
#include "IPLocation.h"
#include "Item.h"
#include "Language.h"
#include "LFG.h"
#include "Log.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MMapFactory.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Pet.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "Realm.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "TargetedMovementGenerator.h"
#include "Transport.h"
#include "Weather.h"
#include "WeatherMgr.h"
#include "World.h"
#include "WorldSession.h"
#include <boost/asio/ip/address_v4.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <G3D/Quat.h>
#include "ItemTemplate.h"
#include "HotfixPackets.h"
#include "Position.h"
#include "Object.h"
#include "Bag.h"
//
#include "GossipDef.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "Log.h"
#include "NPCPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "QuestPackets.h"
#include "World.h"
#include "WorldSession.h"
#include "GameEventMgr.h"
#include <regex>
#include "AuroraPackets.h"

 // temporary hack until database includes are sorted out (don't want to pull in Windows.h everywhere from mysql.h)
#ifdef GetClassName
#undef GetClassName
#endif

class phase_commandscript : public CommandScript
{
public:
    phase_commandscript() : CommandScript("phase_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> phaseSetCommandTable =
        {
             { "public",    rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseSetPublicCommand,             "" , std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Administration}},
             { "private",   rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseSetPrivateCommand,            "" , std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Administration}},
        };

        static std::vector<ChatCommand> phaseRemoveCommandTable =
        {
            { "terrain",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRemoveTerrainCommand,             "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit}},
            { "invite",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRemoveInviteCommand,              "", std::vector<ChatCommand>() },
        };

        static std::vector<ChatCommand> phasePermissionCommandTable =
        {
            { "add",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhasePermissionAddCommand,            "", std::vector<ChatCommand>() },
            { "get",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhasePermissionGetCommand,            "", std::vector<ChatCommand>() },
            { "list",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhasePermissionListCommand,            "", std::vector<ChatCommand>() },
            { "remove",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhasePermissionRemoveCommand,            "", std::vector<ChatCommand>() },
        };

        static std::vector<ChatCommand> phaseRankCommandTable =
        {
            { "create",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankCreateCommand,            "", std::vector<ChatCommand>() },
            { "remove",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankDeleteCommand,            "", std::vector<ChatCommand>() },
            { "list",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankListCommand,            "", std::vector<ChatCommand>() },
            { "get",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankGetCommand,            "", std::vector<ChatCommand>() },
            { "add",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankAddPermCommand,            "", std::vector<ChatCommand>() },
            { "del",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankDelPermCommand,            "", std::vector<ChatCommand>() },
            { "set",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRankSetPermCommand,            "", std::vector<ChatCommand>() },
        };

        static std::vector<ChatCommand> phaseMainCommandTable =
        {
            { "create",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseCreateCommand,              "" },
            { "initialize",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseInitializeCommand,          "" },
            { "terrain",	    rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseTerrainCommand,             "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit}   },
            { "invite",         rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseInviteCommand,              "" },
            { "skybox",		    rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhaseSkyboxCommand,				 "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit}   },
            { "message",	    rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhaseMessageCommand,			 "" },
            { "remove",         rbac::RBAC_PERM_COMMAND_AURA,	  false, nullptr, "", phaseRemoveCommandTable },
            { "playsound",      rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhasePlaySoundCommand,	         "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit}  },
            { "set",            rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", phaseSetCommandTable    },
            { "setareaname",    rbac::RBAC_PERM_COMMAND_AURA,     false, HandlePhaseSetAreaNameCommand,             "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit} },
            { "delareaname",    rbac::RBAC_PERM_COMMAND_AURA,     false, HandlePhaseDelAreaNameCommand,             "", std::vector<ChatCommand>(), {PhaseChat::Permissions::Phase_Edit} },
            { "permission",     rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", phasePermissionCommandTable      },
            { "rank",     rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", phaseRankCommandTable      },

        };

        static std::vector<ChatCommand> phaseCommandTable =
        {
            { "phase",		rbac::RBAC_PERM_COMMAND_AURA,	 false, nullptr, "", phaseMainCommandTable     },
        };

        return phaseCommandTable;
    }

    static bool HandlePhaseCreateCommand(ChatHandler* handler, char const* args)
    {

        // PLAYER
        Player* player = handler->GetSession()->GetPlayer();
        uint32 pMap = handler->GetSession()->GetPlayer()->GetMapId();


        // auto-increment
        QueryResult lastId = HotfixDatabase.PQuery("SELECT MAX(ID) from map");
        Field* field = lastId->Fetch();
        uint32 tId = field[0].GetUInt32();
        ++tId;

        // Check if parentmap values is aphase map
        if (pMap >= MAP_CUSTOM_PHASE)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, pMap);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check if parentmap values is an existing map	
        MapEntry const* mapEntry = sMapStore.LookupEntry(pMap);
        if (!mapEntry)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, pMap);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            HotfixDatabasePreparedStatement* map = HotfixDatabase.GetPreparedStatement(HOTFIX_INS_CREATE_PHASE);

            map->setUInt32(0, tId);
            if (tId < MAP_CUSTOM_PHASE || tId > 65535) // Si plus petit MAP_CUSTOM_PHASE & plus grand 65535 > message.
            {

                handler->PSendSysMessage(LANG_PHASE_CREATED_BADID);
                handler->SetSentErrorMessage(true);
                return false;
            }

            map->setUInt32(1, pMap);
            map->setUInt32(2, pMap);
            if (mapEntry->WdtFile() == 0)
                handler->PSendSysMessage("Impossible de copier cette map, Von Nigger");
            else
                map->setInt32(3, mapEntry->WdtFile());


            HotfixDatabase.Execute(map);

            // hotfix_data
            HotfixDatabasePreparedStatement* data = HotfixDatabase.GetPreparedStatement(HOTFIX_INS_CREATE_PHASE_DATA);
            data->setUInt32(0, tId);
            data->setUInt32(1, tId);
            HotfixDatabase.Execute(data);


            // phase owner - world database
            ObjectGuid::LowType pGuid = UI64LIT(0);
            pGuid = handler->GetSession()->GetAccountId();

            WorldDatabasePreparedStatement* owner = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_OWNER);
            owner->setUInt32(0, tId);
            owner->setUInt64(1, pGuid);
            WorldDatabase.Execute(owner);

            //phase allow
            WorldDatabasePreparedStatement* allow = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_ALLOW);
            allow->setUInt32(0, tId);
            allow->setUInt64(1, pGuid);
            WorldDatabase.Execute(allow);


            // game_tele
            std::string pName = player->GetName();
            pName.erase(std::remove_if(pName.begin(), pName.end(), ::isspace), pName.end());
            std::transform(pName.begin(), pName.end(), pName.begin(), ::tolower);

            GameTele tele;
            tele.position_x = player->GetPositionX();
            tele.position_y = player->GetPositionY();
            tele.position_z = player->GetPositionZ();
            tele.orientation = player->GetOrientation();
            tele.mapId = tId;
            tele.name = pName + std::to_string(tId);
            ;

            if (sObjectMgr->AddGameTele(tele))
            {
                handler->SendSysMessage(LANG_COMMAND_TP_ADDED);
            }
            else
            {
                handler->SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
                handler->SetSentErrorMessage(true);
                return false;
            }

            //phaseown_map add for lookup phase own/aut
            WorldDatabasePreparedStatement* insertphaseown = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASEOWN_MAP);
            insertphaseown->setFloat(0, player->GetPositionX());
            insertphaseown->setFloat(1, player->GetPositionY());
            insertphaseown->setFloat(2, player->GetPositionZ());
            insertphaseown->setFloat(3, player->GetOrientation());
            insertphaseown->setUInt16(4, tId);
            insertphaseown->setString(5, pName + std::to_string(tId));
            WorldDatabase.Execute(insertphaseown);

            handler->GetSession()->AddPhasePermissions(tId, (PhaseChat::Permissions*)PhaseChat::AllPermissions, sizeof(PhaseChat::AllPermissions));

            handler->PSendSysMessage(LANG_PHASE_CREATED_SUCCESS);
            handler->PSendSysMessage(LANG_PHASE_CREATED_FINAL, tele.name);

        }

        return true;

    }

    static bool HandlePhaseInviteCommand(ChatHandler* handler, char const* args)
    {

        WorldSession* session = handler->GetSession();
        // Player & Target variables
        Player* _target;
        ObjectGuid _targetGuid;

        // User input
        char const* targetName = strtok((char*)args, " ");
        char const* phId = strtok(NULL, " ");

        if (targetName == NULL)
            return false;

        std::string phName = phId;
        std::string strName = targetName;
        uint32 iPhaseId;

        if (std::all_of(phName.begin(), phName.end(), ::isdigit))
            iPhaseId = uint32(atoi(phId));
        else
            return false;

        if (!PhaseExists(iPhaseId))
            return false;

        if (!session->HasPhasePermission(iPhaseId, PhaseChat::Permissions::Phase_Invite)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        // Check Part

        if (!handler->extractPlayerTarget((char*)args, &_target, &_targetGuid, &strName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        std::string strOwnerLink = handler->playerLink(_player->GetName().c_str());
        if (_target == _player || _targetGuid == _player->GetGUID())
            return false;

        if (_target)
        {
            std::string strNameLink = handler->playerLink(targetName);
            AddPlayerToPhase(iPhaseId, _target);

            handler->PSendSysMessage(LANG_PHASE_INVITE_SUCCESS, strNameLink.c_str());

            if (handler->needReportToTarget(_target))
            {
                ChatHandler(_target->GetSession()).PSendSysMessage(LANG_PHASE_PHASE_INVITE_INI, iPhaseId, strOwnerLink.c_str());
                // Send Packet to target player
                sDB2Manager.LoadHotfixData();
                sMapStore.LoadFromDB();
                sMapStore.LoadStringsFromDB(LocaleConstant::LOCALE_frFR); // locale frFR 
                _target->GetSession()->SendPacket(WorldPackets::Hotfix::AvailableHotfixes(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)), sDB2Manager.GetHotfixCount(), sDB2Manager.GetHotfixData()).Write());
            }

        }
        else
        {
            std::string strNameLink = handler->playerLink(targetName);
            AddPlayerToPhase(iPhaseId, _target);

            handler->PSendSysMessage(LANG_PHASE_INVITE_SUCCESS, strNameLink.c_str());

            if (handler->needReportToTarget(_target))
            {
                ChatHandler(_target->GetSession()).PSendSysMessage(LANG_PHASE_PHASE_INVITE_INI, iPhaseId, strOwnerLink.c_str());
                // Send Packet to target player
                sDB2Manager.LoadHotfixData();
                sMapStore.LoadFromDB();
                sMapStore.LoadStringsFromDB(LocaleConstant::LOCALE_frFR); // locale frFR 
                _target->GetSession()->SendPacket(WorldPackets::Hotfix::AvailableHotfixes(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)), sDB2Manager.GetHotfixCount(), sDB2Manager.GetHotfixData()).Write());
            }
        }

        return true;
    }

    static bool HandlePhaseSkyboxCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* pId = strtok((char*)args, " ");
        Player* player = handler->GetSession()->GetPlayer();
        uint32 map = player->GetMapId();

        std::string replaceName = pId;
        uint32 replaceID;

        if (std::all_of(replaceName.begin(), replaceName.end(), ::isdigit))
            replaceID = uint32(atoi(pId));
        else
            return false;

        if (player->GetMapId() >= MAP_CUSTOM_PHASE)
        {
            if (MapEntry const* entry = sMapStore.AssertEntry(map))
                map = entry->ParentMapID;
        }

        uint32 lightId = DB2Manager::GetMapLightId(map);
        if (lightId == 0)
        {
            handler->PSendSysMessage(LANG_PHASE_SKYBOX_ERROR);
            return false;
        }

        // On contrôle si le joueur possède une entrée dans la base de donnée qui contient les settings perma (morph, etc..)
        // si oui, on update, sinon, insert.
        if (handler->GetSession()->GetPlayer()->isSaved())
        {
            // UPDATE
            WorldDatabasePreparedStatement* updSkybox = WorldDatabase.GetPreparedStatement(WORLD_UPD_PERMASKYBOX);
            updSkybox->setUInt32(0, replaceID);
            updSkybox->setUInt64(1, handler->GetSession()->GetPlayer()->GetGUID().GetCounter());
            WorldDatabase.Execute(updSkybox);
        }
        else
        {
            // INSERT
            WorldDatabasePreparedStatement* getSkybox = WorldDatabase.GetPreparedStatement(WORLD_INS_PERMASKYBOX);
            getSkybox->setUInt64(0, handler->GetSession()->GetPlayer()->GetGUID().GetCounter());
            getSkybox->setUInt32(1, replaceID);
            WorldDatabase.Execute(getSkybox);
        }

        // Si le joueur est dans une phase, on envoit le packet à tous les joueurs présent dans la phase, sinon qu'à lui même
        if (player->GetMapId() >= MAP_CUSTOM_PHASE)
            sWorld->SendMapMessage(player->GetMapId(), WorldPackets::Misc::OverrideLight(int32(lightId), int32(200), int32(replaceID)).Write());
        else
        {
            WorldPacket data(SMSG_OVERRIDE_LIGHT, 12);
            data << lightId;
            data << replaceID;
            data << 200;
            handler->GetSession()->SendPacket(&data, true);
        }

        return true;
    }



    static bool HandlePhaseInitializeCommand(ChatHandler* handler, char const* args)
    {

        // Refresh Hotfixs
        sDB2Manager.LoadHotfixData();
        sMapStore.LoadFromDB();
        sMapStore.LoadStringsFromDB(LocaleConstant::LOCALE_frFR); // locale frFR 

        // Send Packet to the Player
        handler->GetSession()->SendPacket(WorldPackets::Hotfix::AvailableHotfixes(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)), sDB2Manager.GetHotfixCount(), sDB2Manager.GetHotfixData()).Write());
        handler->PSendSysMessage(LANG_PHASE_INI);

        // Send Terrain Swap to the player
        //handler->GetSession()->GetPlayer()->SendUpdatePhasing();
        PhasingHandler::SendToPlayer(handler->GetSession()->GetPlayer());

        return true;

    }


    static bool HandlePhaseTerrainCommand(ChatHandler* handler, char const* args)
    {
        Player* tp = handler->GetSession()->GetPlayer();
        uint32 mapId = tp->GetMapId();

        if (!*args)
            return false;

        char const* tId = strtok((char*)args, " ");

        std::string replaceName = tId;
        uint32 terrainMap;

        if (std::all_of(replaceName.begin(), replaceName.end(), ::isdigit))
            terrainMap = uint32(atoi(tId));
        else
            return false;

        MapEntry const* mapEntry = sMapStore.LookupEntry(terrainMap);
        if (!mapEntry)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, terrainMap);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Check parts
        if (mapEntry->WdtFile() == 0)
            return false;

        if (!tId)
            return false;

        if (mapId < MAP_CUSTOM_PHASE)
            return false;

        QueryResult checkAlready = WorldDatabase.PQuery("SELECT MapId, TerrainSwapMap FROM terrain_swap_defaults WHERE MapId = %u AND TerrainSwapMap = %u", mapId, terrainMap);
        if (checkAlready)
            return false;

        WorldDatabasePreparedStatement* swap = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_TERRAIN);
        swap->setUInt32(0, mapId);
        swap->setUInt32(1, terrainMap);
        WorldDatabase.Execute(swap);

        sObjectMgr->LoadPhases();

        // Actualize 
        if (!tp->GetPhaseShift().HasVisibleMapId(terrainMap))
            PhasingHandler::AddVisibleMapId(tp, terrainMap);

        PhasingHandler::SendToPlayer(tp);

        return true;
    }

    static bool HandlePhaseSetAreaNameCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint32 mapId = player->GetMapId();
        uint32 areaId = player->GetAreaId();

        std::string trinityArgs(args);

        if (trinityArgs.find(";") == std::string::npos || trinityArgs.empty())
            return false;

        Tokenizer dataArgs(trinityArgs, ';', 0, false);

        if (dataArgs.size() < 2)
            return false;

        if (mapId < MAP_CUSTOM_PHASE)
            return false;

        std::string zoneName = dataArgs[0];
        std::string subZoneName = dataArgs[1];

        // max buffer size by blizzard
        const auto maxBufferSize = 1024;
        if (zoneName.length() > maxBufferSize || subZoneName.length() > maxBufferSize)
            return false;

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_PHASE_AREA_NAME);
        stmt->setInt32(0, mapId);
        stmt->setInt32(1, areaId);
        PreparedQueryResult customArea = WorldDatabase.Query(stmt);

        if (customArea)
        {
            Field* field = customArea->Fetch();

            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_PHASE_AREA_NAME);
            stmt->setString(0, zoneName);
            stmt->setString(1, subZoneName);
            stmt->setInt32(2, mapId);
            stmt->setInt32(3, areaId);
            WorldDatabase.Execute(stmt);
        }
        else {
            WorldDatabasePreparedStatement* name = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_AREA_NAME);
            name->setUInt32(0, mapId);
            name->setUInt32(1, areaId);
            name->setString(2, zoneName);
            name->setString(3, subZoneName);

            WorldDatabase.Execute(name);
        }


        WorldPackets::Aurora::AuroraZoneCustom zoneCustom;
        zoneCustom.AreaID = areaId;
        zoneCustom.MapID = mapId;
        zoneCustom.ZoneID = player->GetZoneId();
        zoneCustom.ZoneName = zoneName;
        zoneCustom.SubZoneName = subZoneName;
        zoneCustom.Delete = 0;

        sWorld->SendAreaIDMessage(areaId, zoneCustom.Write());

        return true;
    }

    static bool HandlePhaseDelAreaNameCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint32 mapId = player->GetMapId();
        uint32 areaId = player->GetAreaId();

        if (mapId < MAP_CUSTOM_PHASE)
            return false;

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_PHASE_AREA_NAME);
        stmt->setInt32(0, mapId);
        stmt->setInt32(1, areaId);
        PreparedQueryResult customArea = WorldDatabase.Query(stmt);

        if (customArea)
        {
            Field* field = customArea->Fetch();
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_AREA_NAME);
            stmt->setInt32(0, mapId);
            stmt->setInt32(1, areaId);
            WorldDatabase.Execute(stmt);

            WorldPackets::Aurora::AuroraZoneCustom zoneCustom;
            zoneCustom.AreaID = areaId;
            zoneCustom.MapID = mapId;
            zoneCustom.ZoneID = player->GetZoneId();
            zoneCustom.ZoneName = " ";
            zoneCustom.SubZoneName = " ";
            zoneCustom.Delete = 1;

            sWorld->SendMapMessage(mapId, zoneCustom.Write());
        }
        else
        {
            return false;
        }

        return true;
    }

    static bool HandlePhaseRemoveTerrainCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* tp = handler->GetSession()->GetPlayer();
        uint32 mapId = tp->GetMapId();

        char const* tId = strtok((char*)args, " ");
        std::string replaceName = tId;
        uint32 terrainMap;

        if (std::all_of(replaceName.begin(), replaceName.end(), ::isdigit))
            terrainMap = uint32(atoi(tId));
        else
            return false;

        MapEntry const* mapEntry = sMapStore.LookupEntry(terrainMap);
        if (!mapEntry)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, terrainMap);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (mapId < MAP_CUSTOM_PHASE)
            return false;

        WorldDatabasePreparedStatement* remove = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_TERRAIN);
        remove->setUInt32(0, mapId);
        remove->setUInt32(1, terrainMap);
        WorldDatabase.Execute(remove);

        sObjectMgr->LoadPhases();

        // Actualize 
        //tp->SendUpdatePhasing();
        if (tp->GetPhaseShift().HasVisibleMapId(terrainMap))
            PhasingHandler::RemoveVisibleMapId(tp, terrainMap);

        PhasingHandler::SendToPlayer(tp);

        return true;
    }

    static bool HandlePhaseMessageCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Player Variable
        Player* player = handler->GetSession()->GetPlayer();
        uint32 mapId = player->GetMapId();

        if (mapId > MAP_CUSTOM_PHASE)
        {
            sWorld->SendMapText(mapId, LANG_PHASE_EVENT_MESSAGE, args);
        }

        return true;
    }

    static bool HandlePhasePlaySoundCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 soundId = atoul(args);
        uint32 mapid = handler->GetSession()->GetPlayer()->GetMapId();

        if (!sSoundKitStore.LookupEntry(soundId))
        {
            handler->PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (mapid > MAP_CUSTOM_PHASE) {
            handler->PSendSysMessage(LANG_PHASE_PLAY_SOUND, soundId);
            sWorld->SendMapMessage(mapid, WorldPackets::Misc::PlaySound(handler->GetSession()->GetPlayer()->GetGUID(), soundId).Write());
        }
        else {

            handler->PSendSysMessage(LANG_PHASE_PLAY_SOUND_NO_AUTHORIZE);
            handler->SetSentErrorMessage(true);
            return false;

        }

        return true;

    }



    static bool HandlePhaseSetPublicCommand(ChatHandler* handler, char const* args)
    {
        // Fix for mangolian
        if (handler->GetSession()->GetPlayer()->GetMapId() < MAP_CUSTOM_PHASE)
            return false;

        WorldDatabasePreparedStatement* set = WorldDatabase.GetPreparedStatement(WORLD_UPD_PHASE_SET_TYPE);
        set->setUInt16(0, 1);
        set->setUInt32(1, handler->GetSession()->GetPlayer()->GetMapId());
        WorldDatabase.Execute(set);

        handler->PSendSysMessage(LANG_PHASE_IS_NOW_PUBLIC);

        return true;
    }

    static bool HandlePhaseSetPrivateCommand(ChatHandler* handler, char const* args)
    {
        // Fix for mangolian
        if (handler->GetSession()->GetPlayer()->GetMapId() < MAP_CUSTOM_PHASE)
            return false;

        WorldDatabasePreparedStatement* set = WorldDatabase.GetPreparedStatement(WORLD_UPD_PHASE_SET_TYPE);
        set->setUInt16(0, 0);
        set->setUInt32(1, handler->GetSession()->GetPlayer()->GetMapId());
        WorldDatabase.Execute(set);

        handler->PSendSysMessage(LANG_PHASE_IS_NOW_PRIVATE);

        return true;
    }

    static bool HandlePhaseRemoveInviteCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
        WorldSession* session = handler->GetSession();
        Player* target;
        ObjectGuid targetGuid;

        char const* targetName = strtok((char*)args, " ");
        char const* phId = strtok(NULL, " ");

        if (!targetName || targetName == NULL)
            return false;
        if (!phId || phId == NULL)
            return false;
        if (!targetName && !phId || targetName == NULL && phId == NULL)
            return false;

        std::string pName = targetName;
        std::string phName = phId;
        uint32 phaseId;

        if (std::all_of(phName.begin(), phName.end(), ::isdigit))
            phaseId = uint32(atoi(phId));
        else
            return false;

        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &pName))
            return false;

        targetGuid = sCharacterCache->GetCharacterGuidByName(pName.c_str());
        target = ObjectAccessor::FindConnectedPlayer(targetGuid);


        std::string nameLink = handler->playerLink(pName);
        std::string ownerLink = handler->playerLink(handler->GetSession()->GetPlayerName());

        if (phaseId < 1)
            return false;

        if (phaseId < MAP_CUSTOM_PHASE)
            return false;

        // Check if map exist
        QueryResult cExist = HotfixDatabase.PQuery("SELECT ID from Map WHERE ID = %u", phaseId);
        if (!cExist)
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Moderation)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        if (target)
        {

            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            if (target->IsPhaseOwner(phaseId))
                return false;

            // ajouter
            WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_INVITE);
            invit->setUInt32(0, phaseId);
            invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));

            QueryResult alreadyDel = WorldDatabase.PQuery("SELECT playerId FROM phase_allow WHERE phaseId = %u AND playerId = %u", phaseId, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
            if (!alreadyDel)
                return false;

            WorldDatabase.Execute(invit);

            handler->PSendSysMessage(LANG_PHASE_INVITE_DEL_OWNER, nameLink);

            if (target->GetSession() == nullptr) {
                handler->PSendSysMessage(LANG_ERROR);
            }
            else {

                if (handler->needReportToTarget(target))
                    ChatHandler(target->GetSession()).PSendSysMessage(LANG_PHASE_INVITE_DEL_TARGET, phaseId, ownerLink);

                if (handler->needReportToTarget(target))
                {
                    // Sanctuaire Coords
                    const uint32 mapId = 1374;
                    const float x = 2018.339355f;
                    const float y = 2953.082031f;
                    const float z = 25.213768f;
                    const float o = 6.224421f;

                    // Send Packet to target player
                    sDB2Manager.LoadHotfixData();
                    sMapStore.LoadFromDB();
                    sMapStore.LoadStringsFromDB(LocaleConstant::LOCALE_frFR); // locale frFR 
                    target->GetSession()->SendPacket(WorldPackets::Hotfix::AvailableHotfixes(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)), sDB2Manager.GetHotfixCount(), sDB2Manager.GetHotfixData()).Write());
                    target->TeleportTo(mapId, x, y, z, o);
                }

            }

        }
        else {

            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            // ajouter
            WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_INVITE);
            invit->setUInt32(0, phaseId);
            invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
            WorldDatabase.Execute(invit);

            //handler->PSendSysMessage(LANG_PHASE_INVITE_DEL_TARGET, phaseId, ownerLink);

        }

        return true;

    }

    static bool HandlePhasePermissionAddCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);

        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 3) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string playerName = dataArgs[1];
        uint32_t permissionId;

        std::string phaseIdStr = dataArgs[0];
        std::string permissionIdStr = dataArgs[2];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (std::all_of(permissionIdStr.begin(), permissionIdStr.end(), ::isdigit))
            permissionId = uint32(std::stoul(permissionIdStr));
        else
            return false;

        auto target = GetPlayerFromName(handler, playerName);
        if (target == nullptr)
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        if (PhaseChat::IsPermissionValid<uint32_t>(permissionId)) {
            auto matchMap = PhaseChat::PermissionsMap();
            handler->PSendSysMessage("Added permission %s for player %s in phase %d", matchMap[PhaseChat::Permissions(permissionId)], handler->playerLink(playerName), phaseId);

            if (!target->IsPhaseOwner(phaseId))
                target->GetSession()->AddPhasePermission(phaseId, PhaseChat::Permissions(permissionId));
        }
 
        return true;
    }


    static bool HandlePhasePermissionGetCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);

        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 2) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string phaseIdStr = dataArgs[0];
        std::string playerName = dataArgs[1];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        auto target = GetPlayerFromName(handler, playerName);
        if (target == nullptr)
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> targetPermissions(0);
        if (target->GetSession()->GetPhasePermissions(phaseId, targetPermissions))
        {
            handler->PSendSysMessage("Permissions for player %s on phase %d :", handler->playerLink(playerName), phaseId);
            auto matchMap = PhaseChat::PermissionsMap();

            for (uint32_t i = 0; i <= PhaseChat::GetPermissionsAs<uint32_t>(PhaseChat::Permissions(PhaseChat::AllPermissions[sizeof(PhaseChat::AllPermissions) - 1])); i++) {
                std::string hasPerm = targetPermissions[i] ? "true" : "false";
                handler->PSendSysMessage("| - |cffffffff[%s]|r : %s ", matchMap[PhaseChat::Permissions(i)], hasPerm);
            }

        }
        else {
            handler->PSendSysMessage("No permissions found for player on map ( fix this )");
        }

        return true;
    }

     static bool HandlePhasePermissionListCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();

        auto matchName = PhaseChat::PermissionsMap();
        auto matchDescription = PhaseChat::PermissionsDescription();

        for (uint32 i = 0; i < sizeof(PhaseChat::AllPermissions); i++) {
            handler->PSendSysMessage("| - Permission : |cffffffff[%s]|r (%d)", matchName[PhaseChat::Permissions(i)], i);
            handler->PSendSysMessage("|     |cffffffff[%s]|r", matchDescription[PhaseChat::Permissions(i)]);
        }
       
        return true;
    }

    static bool HandlePhasePermissionRemoveCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 3) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32 phaseId;
        std::string playerName = dataArgs[1];
        uint32 permissionId;

        std::string phaseIdStr = dataArgs[0];
        std::string permissionIdStr = dataArgs[2];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (std::all_of(permissionIdStr.begin(), permissionIdStr.end(), ::isdigit))
            permissionId = uint32(std::stoul(permissionIdStr));
        else
            return false;

        auto target = GetPlayerFromName(handler, playerName);
        if (target == nullptr)
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        if (PhaseChat::IsPermissionValid<uint32_t>(permissionId)) {
            auto matchMap = PhaseChat::PermissionsMap();
            handler->PSendSysMessage("Removed permission %s for player %s in phase %d", matchMap[PhaseChat::Permissions(permissionId)], handler->playerLink(playerName), phaseId);

            if (!target->IsPhaseOwner(phaseId))
                target->GetSession()->RemovePhasePermission(phaseId, PhaseChat::Permissions(permissionId));
        }

        return true;
    }

    static bool HandlePhaseRankCreateCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 2) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32 phaseId;
        std::string rankName = dataArgs[1];

        std::string phaseIdStr = dataArgs[0];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {
            handler->PSendSysMessage("An account with the same name already exists in this phase");
            return false;
        }
        else {
            std::bitset<PhaseChat::PhaseMaxPermissions> rankPermissionsData(0);
            if (dataArgs.size() >= 3) {
                std::string rankPermissions = dataArgs[2];
                Tokenizer permArgs(rankPermissions, ',', 0, false);

                for (uint32 i = 0; i < permArgs.size(); i++) {
                    uint32 permissionId;
                    std::string permissionStr = permArgs[i];
                    
                    if (std::all_of(permissionStr.begin(), permissionStr.end(), ::isdigit))
                        permissionId = uint32(std::stoul(permissionStr));
                    else
                        continue;

                    if (PhaseChat::IsPermissionValid<uint32_t>(permissionId))
                        rankPermissionsData.set(permissionId);
                }

            }
            else {

            }

            handler->PSendSysMessage("Created rank |cffffffff%s|r in phase %d", rankName, phaseId);
            CreateRank(phaseId, rankName, rankPermissionsData);
        }
     
        return true;
    }

    static bool HandlePhaseRankGetCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 2) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32 phaseId;
        std::string phaseIdStr = dataArgs[0];
        std::string rankName = dataArgs[1];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {

            handler->PSendSysMessage("Permissions for rank |cffffffff%s|r on phase %d", rankName, phaseId);
            auto matchMap = PhaseChat::PermissionsMap();

            for (uint32 i = 0; i <= PhaseChat::GetPermissionsAs<uint32>(PhaseChat::Permissions(PhaseChat::AllPermissions[sizeof(PhaseChat::AllPermissions) - 1])); i++) {
                std::string hasPerm = permissions[i] ? "true" : "false";
                handler->PSendSysMessage("| - |cffffffff%s|r : %s ", matchMap[PhaseChat::Permissions(i)], hasPerm);
            }
        }
        else {
            handler->PSendSysMessage("Rank does not exists.");
            return false;
        }

        return true;
    }


    static bool HandlePhaseRankListCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 1) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32 phaseId;

        std::string phaseIdStr = dataArgs[0];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        auto ranks = GetAllRanks(phaseId);

        handler->PSendSysMessage("Alls ranks on phase %d : ", phaseId);

        for (const auto& rank : ranks) {
            handler->PSendSysMessage("| - |cffffffff%s|r ", rank);
        }

        if(ranks.size() == 0)
            handler->PSendSysMessage("| - No rank(s) found");

        return true;
    }

    static bool HandlePhaseRankDeleteCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 2) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string rankName = dataArgs[1];
        std::string phaseIdStr = dataArgs[0];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {
            DeleteRank(phaseId, rankName);
            handler->PSendSysMessage("Deleted rank |cffffffff%s|r on phase %d.", rankName, phaseId);
        }
        else {
            handler->PSendSysMessage("Rank does not exists.");
            return false;
        }


        return true;
    }

    static bool HandlePhaseRankAddPermCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 3) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string rankName = dataArgs[1];
        uint32_t permissionId;

        std::string phaseIdStr = dataArgs[0];
        std::string permissionIdStr = dataArgs[2];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (std::all_of(permissionIdStr.begin(), permissionIdStr.end(), ::isdigit))
            permissionId = uint32(std::stoul(permissionIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {
            if (PhaseChat::IsPermissionValid<uint32_t>(permissionId))
                permissions.set(permissionId);

            UpdateRank(permissions, phaseId, rankName);

            auto matchName = PhaseChat::PermissionsMap();

            handler->PSendSysMessage("Added perm %s for rank |cffffffff%s|r on phase %d.", matchName[PhaseChat::Permissions(permissionId)], rankName, phaseId);
        }
        else {
            handler->PSendSysMessage("Rank does not exists.");
            return false;
        }

        return true;
    }

    static bool HandlePhaseRankDelPermCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 3) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string rankName = dataArgs[1];
        uint32_t permissionId;

        std::string phaseIdStr = dataArgs[0];
        std::string permissionIdStr = dataArgs[2];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        if (std::all_of(permissionIdStr.begin(), permissionIdStr.end(), ::isdigit))
            permissionId = uint32(std::stoul(permissionIdStr));
        else
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {
            if (PhaseChat::IsPermissionValid<uint32_t>(permissionId))
                permissions.reset(permissionId);

            UpdateRank(permissions, phaseId, rankName);

            auto matchName = PhaseChat::PermissionsMap();

            handler->PSendSysMessage("Removed perm %s for rank name |cffffffff%s|r on phase %d.", matchName[PhaseChat::Permissions(permissionId)], rankName, phaseId);
        }
        else {
            handler->PSendSysMessage("Rank does not exists.");
            return false;
        }

        return true;
    }

    static bool HandlePhaseRankSetPermCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 3) {
            handler->PSendSysMessage("Not enough arguments");
            return false;
        }

        uint32_t phaseId;
        std::string playerName = dataArgs[1];
        std::string rankName = dataArgs[2];

        std::string phaseIdStr = dataArgs[0];

        if (std::all_of(phaseIdStr.begin(), phaseIdStr.end(), ::isdigit))
            phaseId = uint32(std::stoul(phaseIdStr));
        else
            return false;

        auto target = GetPlayerFromName(handler, playerName);
        if (target == nullptr)
            return false;

        if (!IsValidPhase(phaseId))
            return false;

        if (!session->HasPhasePermission(phaseId, PhaseChat::Permissions::Phase_Administration)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        std::bitset<PhaseChat::PhaseMaxPermissions> permissions;
        if (GetPermissionsFromRank(phaseId, rankName, permissions)) {
            handler->PSendSysMessage("Promoted %s to |cffffffff%s|r.", handler->playerLink(playerName), rankName);

            if(!target->IsPhaseOwner(phaseId))
                target->GetSession()->AddPhasePermissions(phaseId, permissions);
        }
        else {
            handler->PSendSysMessage("Rank does not exists.");
            return false;
        }

        return true;
    }

    static void AddPlayerToPhase(uint32 phase, Player* target)
    {
        // In first, we need to known if the player is already allowed to join the phase.
        QueryResult alreadyInvit = WorldDatabase.PQuery("SELECT playerId FROM phase_allow WHERE phaseId = %u AND playerId = %u", phase, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
        if (!alreadyInvit)
        {
            WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_INVITE);
            invit->setUInt32(0, phase);
            invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
            WorldDatabase.Execute(invit);

        }
    }

    static bool PhaseExists(uint32 phase)
    {
        QueryResult checkPhase = HotfixDatabase.PQuery("SELECT ID FROM map WHERE ID = %u", phase);

        if (checkPhase)
            return true;
        else
            return false;
    }

    static bool IsValidPhase(uint32 phaseId) {
        return phaseId >= MAP_CUSTOM_PHASE && PhaseExists(phaseId);
    }

    static Player* GetPlayerFromName(ChatHandler* handler, std::string name) {
        if(!normalizePlayerName(name))
            return nullptr;

        return ObjectAccessor::FindPlayerByName(name);
    }

    /* Rank */

    static bool GetPermissionsFromRank(uint32 phaseId, std::string rankName, std::bitset<PhaseChat::PhaseMaxPermissions>& permissions) {
        LoginDatabasePreparedStatement* rankStmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PHASE_RANK);
        rankStmt->setUInt32(0, phaseId);
        rankStmt->setString(1, rankName);

        PreparedQueryResult rankData = LoginDatabase.Query(rankStmt);

        if (rankData) {
            Field* field = rankData->Fetch();
            permissions = PhaseChat::GetPermissionsFromVector(field[0].GetBinary());
        }
        else {
            return false;
        }

        return true;
    }

    static std::vector<std::string> GetAllRanks(uint32 phaseId) {
        std::vector<std::string> ranks;

        LoginDatabasePreparedStatement* rankStmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PHASE_ALL_RANKS);
        rankStmt->setUInt32(0, phaseId);

        PreparedQueryResult rankData = LoginDatabase.Query(rankStmt);

        if (rankData) {
            auto matchMap = PhaseChat::PermissionsMap();
            do
            {
                Field* fields = rankData->Fetch();
                std::string rankName = fields[0].GetString();
                std::bitset<PhaseChat::PhaseMaxPermissions> permissions = PhaseChat::GetPermissionsFromVector(fields[1].GetBinary());
                std::string permissionsString;

                for (uint32 i = 0; i < permissions.size(); i++) {
                    if(permissions[i])
                        permissionsString += Trinity::StringFormat("%s,", matchMap[PhaseChat::Permissions(i)]);
                }

                ranks.emplace_back(Trinity::StringFormat("%s : %s", rankName, permissionsString));

            } while (rankData->NextRow());
        }

        return ranks;
    }

    static void CreateRank(uint32 phaseId, std::string rankName, std::bitset<PhaseChat::PhaseMaxPermissions>& permissions) {
        LoginDatabasePreparedStatement* rankStmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_PHASE_RANK);
        rankStmt->setUInt32(0, phaseId);
        rankStmt->setString(1, rankName);
        rankStmt->setBinary(2, PhaseChat::GetPermissionsToVector(permissions));


        LoginDatabase.Execute(rankStmt);
    }

    static void UpdateRank(std::bitset<PhaseChat::PhaseMaxPermissions>& permissions, uint32 phaseId, std::string rankName) {
        LoginDatabasePreparedStatement*  rankStmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_PHASE_RANK);
        rankStmt->setBinary(0, PhaseChat::GetPermissionsToVector(permissions));
        rankStmt->setUInt32(1, phaseId);
        rankStmt->setString(2, rankName);

        LoginDatabase.Execute(rankStmt);
    }

    static void DeleteRank(uint32 phaseId, std::string rankName) {
        LoginDatabasePreparedStatement*  rankStmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_PHASE_RANK);
        rankStmt->setUInt32(0, phaseId);
        rankStmt->setString(1, rankName);

        LoginDatabase.Execute(rankStmt);
    }

};



void AddSC_phase()
{
    new phase_commandscript();
}
