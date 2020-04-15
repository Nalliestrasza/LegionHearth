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
             { "owner",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseSetOwnerCommand,              "" },
             { "public",    rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseSetPublicCommand,             "" },
             { "private",   rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseSetPrivateCommand,            "" },
        };

        static std::vector<ChatCommand> phaseRemoveCommandTable =
        {
            { "terrain",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRemoveTerrainCommand,             "" },
            { "invite",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRemoveInviteCommand,              "" },
            { "owner",      rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseRemoveOwnerCommand,                "" },
        };

        static std::vector<ChatCommand> phaseMainCommandTable =
        {
            { "create",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseCreateCommand,              "" },
            { "initialize", rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseInitializeCommand,          "" },
            { "terrain",	rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseTerrainCommand,             "" },
            { "invite",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandlePhaseInviteCommand,              "" },
            { "skybox",		rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhaseSkyboxCommand,				 "" },
            { "message",	rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhaseMessageCommand,			 "" },
            { "remove",     rbac::RBAC_PERM_COMMAND_AURA,	  false, nullptr, "", phaseRemoveCommandTable },
            { "playsound",  rbac::RBAC_PERM_COMMAND_AURA,	  false, &HandlePhasePlaySoundCommand,	         "" },
            { "set",        rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", phaseSetCommandTable    },
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
        if (pMap >= 5000)
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
            if (tId < 5000 || tId > 65535) // Si plus petit 5000 & plus grand 65535 > message.
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

            handler->PSendSysMessage(LANG_PHASE_CREATED_SUCCESS);
            handler->PSendSysMessage(LANG_PHASE_CREATED_FINAL, tele.name);

        }

        return true;

    }

    static bool HandlePhaseInviteCommand(ChatHandler* handler, char const* args)
    {

        // Player & Target variables
        Player* _target;
        ObjectGuid _targetGuid;

        // User input
        char const* targetName = strtok((char*)args, " ");
        char const* phId = strtok(NULL, " ");

        if (targetName == NULL)
        {
            printf("if targetname");
            return false;
        }
        std::string strName = targetName;
        uint32 iPhaseId = uint32(atoi(phId));

        if (!PhaseExist(iPhaseId))
        {
            printf("if phase exist");
                printf("%" PRIu32 "%" "\n", ((uint32)phId));
                return false;
        }

        // Check Part

        if (!handler->extractPlayerTarget((char*)args, &_target, &_targetGuid, &strName))
            return false;


        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner(iPhaseId))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

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
                sMapStore.LoadStringsFromDB(2); // locale frFR 
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
                sMapStore.LoadStringsFromDB(2); // locale frFR 
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


        uint32 replaceID = uint32(atoi(pId)); // new skybox ID 

        // Can't use in phase, if not owner.
        if (!handler->GetSession()->GetPlayer()->IsPhaseOwner())
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetMapId() >= 5000)
        {
            QueryResult mapresult = HotfixDatabase.PQuery("SELECT ParentMapID FROM map WHERE id = %u", map);
            Field* mapfields = mapresult->Fetch();
            map = mapfields[0].GetUInt16();
        }

        QueryResult results = WorldDatabase.PQuery("Select m_ID from light where mapid = %u", map);
        if (!results)
        {
            handler->PSendSysMessage(LANG_PHASE_SKYBOX_ERROR);
            return false;
        }

        Field* fields = results->Fetch();
        uint32 lightId = fields[0].GetUInt32();

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
        if (player->GetMapId() >= 5000)
            sWorld->SendMapSkybox(player->GetMapId(), WorldPackets::Misc::OverrideLight(int32(lightId), int32(200), int32(replaceID)).Write());
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

        // Refresh Hotfixe
        sDB2Manager.LoadHotfixData();
        sMapStore.LoadFromDB();
        sMapStore.LoadStringsFromDB(2); // locale frFR 

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
        uint32 terrainMap = uint32(atoi(tId));

        // Check parts
        QueryResult checkWdt = WorldDatabase.PQuery("SELECT MapID from phase_mapcheck WHERE Compatible = 1 and MapID = %u", terrainMap);
        if (!checkWdt)
            return false;

        if (!tId)
            return false;

        if (mapId < 5000)
            return false;

        MapEntry const* mapEntry = sMapStore.LookupEntry(terrainMap);
        if (!mapEntry)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, terrainMap);
            handler->SetSentErrorMessage(true);
            return false;
        }

        QueryResult checkAlready = WorldDatabase.PQuery("SELECT MapId, TerrainSwapMap FROM terrain_swap_defaults WHERE MapId = %u AND TerrainSwapMap = %u", mapId, terrainMap);
        if (checkAlready)
            return false;


        QueryResult checkSql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner where accountOwner = %u and phaseId = %u", handler->GetSession()->GetAccountId(), mapId);

        if (!checkSql)
        {
            handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
            return false;
        }

        Field* field = checkSql->Fetch();
        uint32 accId = field[0].GetUInt32();


        if (accId == handler->GetSession()->GetAccountId())
        {

            WorldDatabasePreparedStatement* swap = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_TERRAIN);
            swap->setUInt32(0, mapId);
            swap->setUInt32(1, terrainMap);
            WorldDatabase.Execute(swap);

            sObjectMgr->LoadPhases();

            // Actualize 
            if (!tp->GetPhaseShift().HasVisibleMapId(terrainMap))
                PhasingHandler::AddVisibleMapId(tp, terrainMap);

            PhasingHandler::SendToPlayer(tp);

        }
        else
        {
            handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
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
        uint32 terrainMap = uint32(atoi(tId));

        MapEntry const* mapEntry = sMapStore.LookupEntry(terrainMap);
        if (!mapEntry)
        {
            handler->PSendSysMessage(LANG_PHASE_CREATED_PARENTMAP_INVALID, terrainMap);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (mapId < 5000)
            return false;

        QueryResult checkSql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner where accountOwner = %u and phaseId = %u", handler->GetSession()->GetAccountId(), mapId);
        Field* field = checkSql->Fetch();
        uint32 accId = field[0].GetUInt32();

        if (accId == handler->GetSession()->GetAccountId())
        {

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

        }

        else

        {
            handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
        }

        return true;
    }


    static bool HandlePhaseMessageCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Player Variable
        Player* player = handler->GetSession()->GetPlayer();
        uint32 mapId = player->GetMapId();

        if (mapId > 5000)
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

        if (mapid > 5000) {

            QueryResult query = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner where accountOwner = %u and phaseId = %u", handler->GetSession()->GetAccountId(), mapid);

            if (query) {

                Field* result = query->Fetch();
                uint32 accId = result[0].GetUInt32();

                if (accId == handler->GetSession()->GetAccountId()) {

                    handler->PSendSysMessage(LANG_PHASE_PLAY_SOUND, soundId);
                    sWorld->SendMapSound(mapid, WorldPackets::Misc::PlaySound(handler->GetSession()->GetPlayer()->GetGUID(), soundId).Write());
                    return true;

                }

            }

            else {

                handler->PSendSysMessage(LANG_PHASE_PLAY_SOUND_NOT_OWNER);
                handler->SetSentErrorMessage(true);
                return false;

            }

        }
        else {

            handler->PSendSysMessage(LANG_PHASE_PLAY_SOUND_NO_AUTHORIZE);
            handler->SetSentErrorMessage(true);
            return false;

        }

        return true;

    }

    static bool HandlePhaseSetOwnerCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
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
        uint32 phaseId = uint32(atoi(phId));

        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &pName))
            return false;

        targetGuid = sCharacterCache->GetCharacterGuidByName(pName.c_str());
        target = ObjectAccessor::FindConnectedPlayer(targetGuid);

        std::string nameLink = handler->playerLink(pName);
        std::string ownerLink = handler->playerLink(handler->GetSession()->GetPlayerName());

        if (phaseId < 1)
            return false;

        if (phaseId < 5000)
            return false;

        // Check if map exist
        QueryResult cExist = HotfixDatabase.PQuery("SELECT ID from Map WHERE ID = %u", phaseId);
        if (!cExist)
            return false;

        if (target)
        {
            //sql

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_OWNER);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));

                QueryResult alreadyExist = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                if (alreadyExist)
                    return false;

                WorldDatabase.Execute(invit);

                handler->PSendSysMessage(LANG_PHASE_INVITE_OWNER, nameLink);

                if (target->GetSession() == NULL) {
                    handler->PSendSysMessage(LANG_ERROR);
                }
                else {

                    if (handler->needReportToTarget(target))
                        ChatHandler(target->GetSession()).PSendSysMessage(LANG_PHASE_INVITE_OWNER_TARGET, phaseId, ownerLink);

                    return true;
                }

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }

        }
        else {

            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_INS_PHASE_OWNER);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                WorldDatabase.Execute(invit);

                handler->PSendSysMessage(LANG_PHASE_INVITE_OWNER, nameLink);
                return true;

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }


        }

        return true;

    }

    static bool HandlePhaseRemoveOwnerCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
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
        uint32 phaseId = uint32(atoi(phId));

        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &pName))
            return false;

        targetGuid = sCharacterCache->GetCharacterGuidByName(pName.c_str());
        target = ObjectAccessor::FindConnectedPlayer(targetGuid);

        std::string nameLink = handler->playerLink(pName);
        std::string ownerLink = handler->playerLink(handler->GetSession()->GetPlayerName());

        if (phaseId < 1)
            return false;

        if (phaseId < 5000)
            return false;

        // Check if map exist
        QueryResult cExist = HotfixDatabase.PQuery("SELECT ID from Map WHERE ID = %u", phaseId);
        if (!cExist)
            return false;

        if (target)
        {
            //sql

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_SET_OWNER);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));

                QueryResult alreadyExist = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                if (!alreadyExist)
                    return false;

                WorldDatabase.Execute(invit);

                handler->PSendSysMessage(LANG_PHASE_DEL_OWNER, nameLink);

                if (target->GetSession() == NULL) {
                    handler->PSendSysMessage(LANG_ERROR);
                }
                else {

                    if (handler->needReportToTarget(target))
                        ChatHandler(target->GetSession()).PSendSysMessage(LANG_PHASE_DEL_OWNER_TARGET, phaseId, ownerLink);

                    return true;
                }

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }

        }
        else {

            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_SET_OWNER);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                WorldDatabase.Execute(invit);

                handler->PSendSysMessage(LANG_PHASE_DEL_OWNER, nameLink);
                return true;

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }


        }

        return true;
    }
    static bool HandlePhaseSetPublicCommand(ChatHandler* handler, char const* args)
    {
        QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", handler->GetSession()->GetPlayer()->GetMapId(), handler->GetSession()->GetAccountId());

        if (!checksql)
        {
            handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            Field* field = checksql->Fetch();
            uint32 accId = field[0].GetUInt32();

            // Fix for mangolian
            if (handler->GetSession()->GetPlayer()->GetMapId() < 5000)
                return false;

            if (accId == handler->GetSession()->GetAccountId())
            {
                WorldDatabasePreparedStatement* set = WorldDatabase.GetPreparedStatement(WORLD_UPD_PHASE_SET_TYPE);
                set->setUInt16(0, 1);
                set->setUInt32(1, handler->GetSession()->GetPlayer()->GetMapId());
                WorldDatabase.Execute(set);

                handler->PSendSysMessage(LANG_PHASE_IS_NOW_PUBLIC);
            }
            else
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }
        }

        return true;

    }

    static bool HandlePhaseSetPrivateCommand(ChatHandler* handler, char const* args)
    {
        QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", handler->GetSession()->GetPlayer()->GetMapId(), handler->GetSession()->GetAccountId());

        if (!checksql)
        {
            handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            Field* field = checksql->Fetch();
            uint32 accId = field[0].GetUInt32();

            // Fix for mangolian
            if (handler->GetSession()->GetPlayer()->GetMapId() < 5000)
                return false;

            if (accId == handler->GetSession()->GetAccountId())
            {
                WorldDatabasePreparedStatement* set = WorldDatabase.GetPreparedStatement(WORLD_UPD_PHASE_SET_TYPE);
                set->setUInt16(0, 0);
                set->setUInt32(1, handler->GetSession()->GetPlayer()->GetMapId());
                WorldDatabase.Execute(set);

                handler->PSendSysMessage(LANG_PHASE_IS_NOW_PRIVATE);
            }
            else
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }
        }

        return true;
    }

    static bool HandlePhaseRemoveInviteCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
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
        uint32 phaseId = uint32(atoi(phId));

        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &pName))
            return false;

        targetGuid = sCharacterCache->GetCharacterGuidByName(pName.c_str());
        target = ObjectAccessor::FindConnectedPlayer(targetGuid);

        std::string nameLink = handler->playerLink(pName);
        std::string ownerLink = handler->playerLink(handler->GetSession()->GetPlayerName());

        if (phaseId < 1)
            return false;

        if (phaseId < 5000)
            return false;

        // Check if map exist
        QueryResult cExist = HotfixDatabase.PQuery("SELECT ID from Map WHERE ID = %u", phaseId);
        if (!cExist)
            return false;

        if (target)
        {

            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            //sql

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_INVITE);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));

                QueryResult alreadyDel = WorldDatabase.PQuery("SELECT playerId FROM phase_allow WHERE phaseId = %u AND playerId = %u", phaseId, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                if (!alreadyDel)
                    return false;


                WorldDatabase.Execute(invit);

                handler->PSendSysMessage(LANG_PHASE_INVITE_DEL_OWNER, nameLink);

                if (target->GetSession() == NULL) {
                    handler->PSendSysMessage(LANG_ERROR);
                }
                else {

                    if (handler->needReportToTarget(target))
                        ChatHandler(target->GetSession()).PSendSysMessage(LANG_PHASE_INVITE_DEL_TARGET, phaseId, ownerLink);

                    if (handler->needReportToTarget(target))
                    {
                        // Sanctuaire Coords
                        uint32 mapId = 1374;
                        float x = 2018.339355f;
                        float y = 2953.082031f;
                        float z = 25.213768f;
                        float o = 6.224421f;

                        // Send Packet to target player
                        sDB2Manager.LoadHotfixData();
                        sMapStore.LoadFromDB();
                        sMapStore.LoadStringsFromDB(2); // locale frFR 
                        target->GetSession()->SendPacket(WorldPackets::Hotfix::AvailableHotfixes(int32(sWorld->getIntConfig(CONFIG_HOTFIX_CACHE_VERSION)), sDB2Manager.GetHotfixCount(), sDB2Manager.GetHotfixData()).Write());
                        target->TeleportTo(mapId, x, y, z, o);

                    }

                    return true;
                }

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }

        }
        else {

            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            QueryResult checksql = WorldDatabase.PQuery("SELECT accountOwner FROM phase_owner WHERE phaseId = %u AND accountOwner = %u", phaseId, handler->GetSession()->GetAccountId());
            if (!checksql)
            {
                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field* field1 = checksql->Fetch();
            uint32 OwnerId = field1[0].GetUInt32();

            if (OwnerId == handler->GetSession()->GetAccountId())
            {
                // ajouter
                WorldDatabasePreparedStatement* invit = WorldDatabase.GetPreparedStatement(WORLD_DEL_PHASE_INVITE);
                invit->setUInt32(0, phaseId);
                invit->setUInt32(1, sCharacterCache->GetCharacterAccountIdByName(target->GetSession()->GetPlayerName().c_str()));
                WorldDatabase.Execute(invit);

                //handler->PSendSysMessage(LANG_PHASE_INVITE_DEL_TARGET, phaseId, ownerLink);
                return true;

            }
            else {

                handler->PSendSysMessage(LANG_PHASE_INVITE_ERROR);
                return false;
            }


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

    static bool PhaseExist(uint32 phase)
    {
        QueryResult checkPhase = HotfixDatabase.PQuery("SELECT ID FROM map WHERE ID = %u", phase);

        if (checkPhase)
            return true;
        else
            return false;
    }
};



void AddSC_phase()
{
    new phase_commandscript();
}
