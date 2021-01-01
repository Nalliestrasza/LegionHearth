﻿/*
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

#include "ScriptMgr.h"
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

class misc_commandscript : public CommandScript
{
public:
    misc_commandscript() : CommandScript("misc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {

        static std::vector<ChatCommand> ticketCommandTable =
        {
            {"create",  rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleCreateTicketCommand,                   "" },
            {"cancel",  rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleCancelTicketCommand,                   "" },
         //   {"close",   rbac::RBAC_PERM_COMMAND_KICK,          false, &HandleCloseTicketCommand,                    "" },
            {"list",    rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleListTicketCommand,                     "" },
        };

        static std::vector<ChatCommand> castCommandTable =
        {
            { "target",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleSetCastTargetCommand,            "" },
        };

        static std::vector<ChatCommand> uncastCommandTable =
        {
            { "target",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleUnsetCastTargetCommand,          "" },
        };

        static std::vector<ChatCommand> setCommandTable =
        {
            { "cast",       rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", castCommandTable },
         // { "name",       rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleSetNameCommand,     "" },
        };

        static std::vector<ChatCommand> unsetCommandTable =
        {
            { "cast",       rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", uncastCommandTable },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "additem",          rbac::RBAC_PERM_COMMAND_ADDITEM,          false, &HandleAddItemCommand,          "" },
            { "additemset",       rbac::RBAC_PERM_COMMAND_ADDITEMSET,       false, &HandleAddItemSetCommand,       "" },
            { "appear",           rbac::RBAC_PERM_COMMAND_APPEAR,           false, &HandleAppearCommand,           "" },
            { "aura",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAuraCommand,             "" },
            { "bank",             rbac::RBAC_PERM_COMMAND_BANK,             false, &HandleBankCommand,             "" },
            { "bindsight",        rbac::RBAC_PERM_COMMAND_BINDSIGHT,        false, &HandleBindSightCommand,        "" },
            { "combatstop",       rbac::RBAC_PERM_COMMAND_COMBATSTOP,        true, &HandleCombatStopCommand,       "" },
            { "cometome",         rbac::RBAC_PERM_COMMAND_COMETOME,         false, &HandleComeToMeCommand,         "" },
            { "commands",         rbac::RBAC_PERM_COMMAND_COMMANDS,          true, &HandleCommandsCommand,         "" },
            { "cooldown",         rbac::RBAC_PERM_COMMAND_COOLDOWN,         false, &HandleCooldownCommand,         "" },
            { "damage",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleDamageCommand,           "" },
            { "dev",              rbac::RBAC_PERM_COMMAND_DEV,              false, &HandleDevCommand,              "" },
            { "die",              rbac::RBAC_PERM_COMMAND_DIE,              false, &HandleDieCommand,              "" },
            { "dismount",         rbac::RBAC_PERM_COMMAND_DISMOUNT,         false, &HandleDismountCommand,         "" },
            { "distance",         rbac::RBAC_PERM_COMMAND_DISTANCE,         false, &HandleGetDistanceCommand,      "" },
            { "freeze",           rbac::RBAC_PERM_COMMAND_FREEZE,           false, &HandleFreezeCommand,           "" },
            { "gps",              rbac::RBAC_PERM_COMMAND_GPS,              false, &HandleGPSCommand,              "" },
            { "guid",             rbac::RBAC_PERM_COMMAND_GUID,             false, &HandleGUIDCommand,             "" },
            { "help",             rbac::RBAC_PERM_COMMAND_HELP,              true, &HandleHelpCommand,             "" },
            { "hidearea",         rbac::RBAC_PERM_COMMAND_HIDEAREA,         false, &HandleHideAreaCommand,         "" },
            { "itemmove",         rbac::RBAC_PERM_COMMAND_ITEMMOVE,         false, &HandleItemMoveCommand,         "" },
            { "kick",             rbac::RBAC_PERM_COMMAND_KICK,              true, &HandleKickPlayerCommand,       "" },
            { "linkgrave",        rbac::RBAC_PERM_COMMAND_LINKGRAVE,        false, &HandleLinkGraveCommand,        "" },
            { "listfreeze",       rbac::RBAC_PERM_COMMAND_LISTFREEZE,       false, &HandleListFreezeCommand,       "" },
            { "movegens",         rbac::RBAC_PERM_COMMAND_MOVEGENS,         false, &HandleMovegensCommand,         "" },
            { "mute",             rbac::RBAC_PERM_COMMAND_MUTE,              true, &HandleMuteCommand,             "" },
            { "mutehistory",      rbac::RBAC_PERM_COMMAND_MUTEHISTORY,       true, &HandleMuteInfoCommand,         "" },
            { "neargrave",        rbac::RBAC_PERM_COMMAND_NEARGRAVE,        false, &HandleNearGraveCommand,        "" },
            { "pinfo",            rbac::RBAC_PERM_COMMAND_PINFO,             true, &HandlePInfoCommand,            "" },
            { "playall",          rbac::RBAC_PERM_COMMAND_PLAYALL,          false, &HandlePlayAllCommand,          "" },
            { "possess",          rbac::RBAC_PERM_COMMAND_POSSESS,          false, &HandlePossessCommand,          "" },
            { "pvpstats",         rbac::RBAC_PERM_COMMAND_PVPSTATS,          true, &HandlePvPstatsCommand,         "" },
            { "recall",           rbac::RBAC_PERM_COMMAND_RECALL,           false, &HandleRecallCommand,           "" },
            { "repairitems",      rbac::RBAC_PERM_COMMAND_REPAIRITEMS,       true, &HandleRepairitemsCommand,      "" },
            { "respawn",          rbac::RBAC_PERM_COMMAND_RESPAWN,          false, &HandleRespawnCommand,          "" },
            { "revive",           rbac::RBAC_PERM_COMMAND_REVIVE,            true, &HandleReviveCommand,           "" },
            { "saveall",          rbac::RBAC_PERM_COMMAND_SAVEALL,           true, &HandleSaveAllCommand,          "" },
            { "save",             rbac::RBAC_PERM_COMMAND_SAVE,             false, &HandleSaveCommand,             "" },
            { "setskill",         rbac::RBAC_PERM_COMMAND_SETSKILL,         false, &HandleSetSkillCommand,         "" },
            { "showarea",         rbac::RBAC_PERM_COMMAND_SHOWAREA,         false, &HandleShowAreaCommand,         "" },
            { "summon",           rbac::RBAC_PERM_COMMAND_SUMMON,           false, &HandleSummonCommand,           "" },
            { "unaura",           rbac::RBAC_PERM_COMMAND_UNAURA,           false, &HandleUnAuraCommand,           "" },
            { "unbindsight",      rbac::RBAC_PERM_COMMAND_UNBINDSIGHT,      false, HandleUnbindSightCommand,       "" },
            { "unfreeze",         rbac::RBAC_PERM_COMMAND_UNFREEZE,         false, &HandleUnFreezeCommand,         "" },
            { "unmute",           rbac::RBAC_PERM_COMMAND_UNMUTE,            true, &HandleUnmuteCommand,           "" },
            { "unpossess",        rbac::RBAC_PERM_COMMAND_UNPOSSESS,        false, &HandleUnPossessCommand,        "" },
            { "unstuck",          rbac::RBAC_PERM_COMMAND_UNSTUCK,           true, &HandleUnstuckCommand,          "" },
            { "wchange",          rbac::RBAC_PERM_COMMAND_WCHANGE,          false, &HandleChangeWeather,           "" },
            { "mailbox",          rbac::RBAC_PERM_COMMAND_MAILBOX,          false, &HandleMailBoxCommand,          "" },
            { "auras  ",          rbac::RBAC_PERM_COMMAND_LIST_AURAS,       false, &HandleAurasCommand,            "" },
            // CUSTOM
            //{ "database",         rbac::RBAC_PERM_COMMAND_KICK,              true, &HandleDebugDatabase,           "" },
            { "dupecore",         rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleModelCommand,            "" },
            { "coredebug",        rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleArrayCommand,            "" },
            { "custom",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleCustomCommand,           "" },
            { "mount",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMountCommand,            "" },
            { "distance",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleDistanceCommand,         "" },
            { "move",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMoveCommand,             "" },
            { "rand",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRandomCommand,           "" },
            { "randmp",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRandomMPCommand,         "" },
            { "pandaren",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandlePandarenCommand,         "" },
            { "death",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMortCommand,             "" },
            { "combat",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLutteCommand,            "" },
            { "camouflage",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTraquerCommand,          "" },
            { "lire",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLireCommand,             "" },
            { "ligoter",          rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAttacherCommand,         "" },
            { "ivre",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleExcesCommand,            "" },
            { "livre",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLivreCommand,            "" },
            { "ombrelle",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleOmbrelleCommand,         "" },
            { "carquois",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleCarquoisCommand,         "" },
            { "sac",              rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSacCommand,              "" },
            { "bondir",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleBondirCommand,           "" },
            { "vomir",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleVomirCommand,            "" },
            { "invisible",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleInvisibleCommand,        "" },
            { "sang",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSangCommand,             "" },
            { "nuit",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleNuitCommand,             "" },
            { "forgeinfo",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleForgeInfoCommand,        "" },
            { "spellviskit",      rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSpellViskitCommand,      "" },
            { "unspellviskit",    rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnSpellViskitCommand,    "" },
            { "animkit",          rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAnimKitCommand,          "" },
            { "debugsync",        rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleDebugSyncCommand,        "" },
            { "health",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleHealthCommand,           "" },
            { "denied",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleDeniedCommand,           "" },
            { "ticket",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, nullptr, "", ticketCommandTable },
            { "ticketlist",       rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleTicketListCommand,       "" },
            { "spellvis",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSpellVisCommand,         "" },
            { "unspellvis",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnSpellVisCommand,       "" },
            { "set",              rbac::RBAC_PERM_COMMAND_AURA,             false, nullptr, "", setCommandTable       },
            { "unset",            rbac::RBAC_PERM_COMMAND_AURA,             false, nullptr, "", unsetCommandTable },
            { "invisible",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleInvisibleCommand,        "" },
            { "power",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSetPowerCommand,         "" },
            { "hp",               rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSetHealthCommand,        "" },
            { "regen",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRegenCommand,            "" },
            { "selfunaura",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnAuraSelfCommand,       "" }, // For Brikabrok addon
            { "selfaura",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAuraSelfCommand,         "" }, // For Brikabrok addon
            { "addonhelper",      rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAddonHelper,             "" }, // For Brikabrok and the other
            { "testpacket",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTestPacket,              "" },
            { "aurorapacket",     rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTestPacketAurora,        "" },
            { "freelook",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleFreeLook,                "" },
            //{ "sendtaxi",        rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleSendTaxiCommand,          "" }, // send taxi
        };
        return commandTable;
    }

    static bool HandlePvPstatsCommand(ChatHandler * handler, char const* /*args*/)
    {
        if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_STORE_STATISTICS_ENABLE))
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PVPSTATS_FACTIONS_OVERALL);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (result)
            {
                Field* fields = result->Fetch();
                uint32 horde_victories = fields[1].GetUInt32();

                if (!(result->NextRow()))
                    return false;

                fields = result->Fetch();
                uint32 alliance_victories = fields[1].GetUInt32();

                handler->PSendSysMessage(LANG_PVPSTATS, alliance_victories, horde_victories);
            }
            else
                return false;
        }
        else
            handler->PSendSysMessage(LANG_PVPSTATS_DISABLED);

        return true;
    }

    static bool HandleDevCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            if (handler->GetSession()->GetPlayer()->HasPlayerFlag(PLAYER_FLAGS_DEVELOPER))
                handler->GetSession()->SendNotification(LANG_DEV_ON);
            else
                handler->GetSession()->SendNotification(LANG_DEV_OFF);
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->AddPlayerFlag(PLAYER_FLAGS_DEVELOPER);
            handler->GetSession()->SendNotification(LANG_DEV_ON);
            return true;
        }

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->RemovePlayerFlag(PLAYER_FLAGS_DEVELOPER);
            handler->GetSession()->SendNotification(LANG_DEV_OFF);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleGPSCommand(ChatHandler* handler, char const* args)
    {
        WorldObject* object = nullptr;
        if (*args)
        {
            HighGuid guidHigh;
            ObjectGuid::LowType guidLow = handler->extractLowGuidFromLink((char*)args, guidHigh);
            if (!guidLow)
                return false;
            switch (guidHigh)
            {
            case HighGuid::Player:
            {
                object = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(guidLow));
                if (!object)
                {
                    handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            case HighGuid::Creature:
            {
                object = handler->GetCreatureFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            case HighGuid::GameObject:
            {
                object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            default:
                return false;
            }
            if (!object)
                return false;
        }
        else
        {
            object = handler->getSelectedUnit();

            if (!object)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        CellCoord cellCoord = Trinity::ComputeCellCoord(object->GetPositionX(), object->GetPositionY());
        Cell cell(cellCoord);

        uint32 zoneId, areaId;
        object->GetZoneAndAreaId(zoneId, areaId);
        uint32 mapId = object->GetMapId();

        MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
        AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(zoneId);
        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaId);

        float zoneX = object->GetPositionX();
        float zoneY = object->GetPositionY();

        sDB2Manager.Map2ZoneCoordinates(zoneId, zoneX, zoneY);

        Map* map = object->GetMap();
        float groundZ = map->GetHeight(object->GetPhaseShift(), object->GetPositionX(), object->GetPositionY(), MAX_HEIGHT);
        float floorZ = map->GetHeight(object->GetPhaseShift(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ());

        GridCoord gridCoord = Trinity::ComputeGridCoord(object->GetPositionX(), object->GetPositionY());

        int gridX = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.x_coord;
        int gridY = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.y_coord;

        uint32 haveMap = Map::ExistMap(mapId, gridX, gridY) ? 1 : 0;
        uint32 haveVMap = Map::ExistVMap(mapId, gridX, gridY) ? 1 : 0;
        uint32 haveMMap = (DisableMgr::IsPathfindingEnabled(mapId) && MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(handler->GetSession()->GetPlayer()->GetMapId())) ? 1 : 0;

        if (haveVMap)
        {
            if (map->IsOutdoors(object->GetPhaseShift(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ()))
                handler->PSendSysMessage(LANG_GPS_POSITION_OUTDOORS);
            else
                handler->PSendSysMessage(LANG_GPS_POSITION_INDOORS);
        }
        else
            handler->PSendSysMessage(LANG_GPS_NO_VMAP);

        char const* unknown = handler->GetTrinityString(LANG_UNKNOWN);

        handler->PSendSysMessage(LANG_MAP_POSITION,
            mapId, (mapEntry ? mapEntry->MapName[handler->GetSessionDbcLocale()] : unknown),
            zoneId, (zoneEntry ? zoneEntry->AreaName[handler->GetSessionDbcLocale()] : unknown),
            areaId, (areaEntry ? areaEntry->AreaName[handler->GetSessionDbcLocale()] : unknown),
            object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        if (Transport* transport = object->GetTransport())
            handler->PSendSysMessage(LANG_TRANSPORT_POSITION,
                transport->GetGOInfo()->moTransport.SpawnMap, object->GetTransOffsetX(), object->GetTransOffsetY(), object->GetTransOffsetZ(), object->GetTransOffsetO(),
                transport->GetEntry(), transport->GetName().c_str());
        handler->PSendSysMessage(LANG_GRID_POSITION,
            cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), object->GetInstanceId(),
            zoneX, zoneY, groundZ, floorZ, haveMap, haveVMap, haveMMap);

        LiquidData liquidStatus;
        ZLiquidStatus status = map->GetLiquidStatus(object->GetPhaseShift(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), MAP_ALL_LIQUIDS, &liquidStatus);

        if (status)
            handler->PSendSysMessage(LANG_LIQUID_STATUS, liquidStatus.level, liquidStatus.depth_level, liquidStatus.entry, liquidStatus.type_flags, status);

        PhasingHandler::PrintToChat(handler, object->GetPhaseShift());

        return true;
    }

    static bool HandleAuraCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    static bool HandleUnAuraCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string argstr = args;
        if (argstr == "all")
        {
            target->RemoveAllAuras();
            return true;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        target->RemoveAurasDueToSpell(spellId);

        return true;
    }
    // Teleport to Player
    static bool HandleAppearCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            handler->SendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            std::string chrNameLink = handler->playerLink(targetName);

            Map* map = target->GetMap();
            if (map->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!_player->IsGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, chrNameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                else if (_player->GetBattlegroundId() && _player->GetBattlegroundId() != target->GetBattlegroundId())
                    _player->LeaveBattleground(false); // Note: should be changed so _player gets no Deserter debuff

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                _player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!_player->GetMap()->IsBattlegroundOrArena())
                    _player->SetBattlegroundEntryPoint();
            }
            else if (map->IsDungeon())
            {
                // we have to go to instance, and can go to player only if:
                //   1) we are in his group (either as leader or as member)
                //   2) we are not bound to any group and have GM mode on
                if (_player->GetGroup())
                {
                    // we are in group, we can go only if we are in the player group
                    if (_player->GetGroup() != target->GetGroup())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY, chrNameLink.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }
                else
                {
                    // we are not in group, let's verify our GM mode
                    if (!_player->IsGameMaster())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM, chrNameLink.c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }

                // if the player or the player's group is bound to another instance
                // the player will not be bound to another one
                InstancePlayerBind* bind = _player->GetBoundInstance(target->GetMapId(), target->GetDifficultyID(map->GetEntry()));
                if (!bind)
                {
                    Group* group = _player->GetGroup();
                    // if no bind exists, create a solo bind
                    InstanceGroupBind* gBind = group ? group->GetBoundInstance(target) : nullptr;                // if no bind exists, create a solo bind
                    if (!gBind)
                        if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(target->GetInstanceId()))
                            _player->BindToInstance(save, !save->CanReset());
                }

                if (map->IsRaid())
                {
                    _player->SetRaidDifficultyID(target->GetRaidDifficultyID());
                    _player->SetLegacyRaidDifficultyID(target->GetLegacyRaidDifficultyID());
                }
                else
                    _player->SetDungeonDifficultyID(target->GetDungeonDifficultyID());
            }

            handler->PSendSysMessage(LANG_APPEARING_AT, chrNameLink.c_str());

            // stop flight if need
            if (_player->IsInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            // to point to see at target with same orientation
            float x, y, z;
            target->GetContactPoint(_player, x, y, z);

            _player->TeleportTo(target->GetMapId(), x, y, z, _player->GetAngle(target), TELE_TO_GM_MODE);
            PhasingHandler::InheritPhaseShift(_player, target);
            _player->UpdateObjectVisibility();
        }
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(nullptr, targetGuid))
                return false;

            std::string nameLink = handler->playerLink(targetName);

            handler->PSendSysMessage(LANG_APPEARING_AT, nameLink.c_str());

            // to point where player stay (if loaded)
            float x, y, z, o;
            uint32 map;
            bool in_flight;
            if (!Player::LoadPositionFromDB(map, x, y, z, o, in_flight, targetGuid))
                return false;

            // stop flight if need
            if (_player->IsInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            _player->TeleportTo(map, x, y, z, _player->GetOrientation());
        }

        return true;
    }
    // Summon Player
    static bool HandleSummonCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            handler->PSendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            std::string nameLink = handler->playerLink(targetName);
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            if (target->IsBeingTeleported())
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, nameLink.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            Map* map = _player->GetMap();

            if (map->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!_player->IsGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, nameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                else if (target->GetBattlegroundId() && _player->GetBattlegroundId() != target->GetBattlegroundId())
                    target->LeaveBattleground(false); // Note: should be changed so target gets no Deserter debuff

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                target->SetBattlegroundId(_player->GetBattlegroundId(), _player->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!target->GetMap()->IsBattlegroundOrArena())
                    target->SetBattlegroundEntryPoint();
            }
            else if (map->Instanceable())
            {
                Map* targetMap = target->GetMap();
                Player* targetGroupLeader = nullptr;
                if (Group* targetGroup = target->GetGroup())
                    targetGroupLeader = ObjectAccessor::GetPlayer(map, targetGroup->GetLeaderGUID());

                // check if far teleport is allowed
                if (!targetGroupLeader || (targetGroupLeader->GetMapId() != map->GetId()) || (targetGroupLeader->GetInstanceId() != map->GetInstanceId()))
                    if ((targetMap->GetId() != map->GetId()) || (targetMap->GetInstanceId() != map->GetInstanceId()))
                    {
                        handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
                        handler->SetSentErrorMessage(true);
                        return false;
                    }

                // check if we're already in a different instance of the same map
                if ((targetMap->GetId() == map->GetId()) && (targetMap->GetInstanceId() != map->GetInstanceId()))
                {
                    handler->PSendSysMessage(LANG_CANNOT_SUMMON_INST_INST, nameLink.c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            handler->PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), "");
            if (handler->needReportToTarget(target))
                ChatHandler(target->GetSession()).PSendSysMessage(LANG_SUMMONED_BY, handler->playerLink(_player->GetName()).c_str());

            // stop flight if need
            if (target->IsInFlight())
            {
                target->GetMotionMaster()->MovementExpired();
                target->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                target->SaveRecallPosition();

            // before GM
            float x, y, z;
            _player->GetClosePoint(x, y, z, target->GetCombatReach());
            target->TeleportTo(_player->GetMapId(), x, y, z, target->GetOrientation());
            PhasingHandler::InheritPhaseShift(target, _player);
            target->UpdateObjectVisibility();
        }
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(nullptr, targetGuid))
                return false;

            std::string nameLink = handler->playerLink(targetName);

            handler->PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), handler->GetTrinityString(LANG_OFFLINE));

            // in point where GM stay
            CharacterDatabaseTransaction dummy;
            Player::SavePositionInDB(WorldLocation(_player->GetMapId(),
                _player->GetPositionX(),
                _player->GetPositionY(),
                _player->GetPositionZ(),
                _player->GetOrientation()),
                _player->GetZoneId(),
                targetGuid, dummy);
        }

        return true;
    }

    static bool HandleCommandsCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->ShowHelpForCommand(handler->getCommandTable(), "");
        return true;
    }

    static bool HandleDieCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->getSelectedUnit();

        if (!target || !handler->GetSession()->GetPlayer()->GetTarget())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Player* player = target->ToPlayer())
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty, false))
                return false;

        if (target->IsAlive())
        {
            if (sWorld->getBoolConfig(CONFIG_DIE_COMMAND_MODE))
                handler->GetSession()->GetPlayer()->Kill(target);
            else
                handler->GetSession()->GetPlayer()->DealDamage(target, target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
        }

        return true;
    }

    static bool HandleReviveCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid))
            return false;

        if (target)
        {
            target->ResurrectPlayer(target->GetSession()->HasPermission(rbac::RBAC_PERM_RESURRECT_WITH_FULL_HPS) ? 1.0f : 0.5f);
            target->SpawnCorpseBones();
            target->SaveToDB();
        }
        else
        {
            CharacterDatabaseTransaction trans(nullptr);
            Player::OfflineResurrect(targetGuid, trans);
        }

        return true;
    }

    static bool HandleDismountCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // If player is not mounted, so go out :)
        if (!player->IsMounted())
        {
            handler->SendSysMessage(LANG_CHAR_NON_MOUNTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->Dismount();
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
        return true;
    }

    static bool HandleGUIDCommand(ChatHandler* handler, char const* /*args*/)
    {
        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetTarget();

        if (guid.IsEmpty())
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_OBJECT_GUID, guid.ToString().c_str());
        return true;
    }

    static bool HandleHelpCommand(ChatHandler* handler, char const* args)
    {
        char const* cmd = strtok((char*)args, " ");
        if (!cmd)
        {
            handler->ShowHelpForCommand(handler->getCommandTable(), "help");
            handler->ShowHelpForCommand(handler->getCommandTable(), "");
        }
        else
        {
            if (!handler->ShowHelpForCommand(handler->getCommandTable(), cmd))
                handler->SendSysMessage(LANG_NO_HELP_CMD);
        }

        return true;
    }
    // move item to other slot
    static bool HandleItemMoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* param1 = strtok((char*)args, " ");
        if (!param1)
            return false;

        char const* param2 = strtok(nullptr, " ");
        if (!param2)
            return false;

        uint8 srcSlot = uint8(atoi(param1));
        uint8 dstSlot = uint8(atoi(param2));

        if (srcSlot == dstSlot)
            return true;

        if (!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0, srcSlot, true))
            return false;

        if (!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0, dstSlot, false))
            return false;

        uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | srcSlot);
        uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | dstSlot);

        handler->GetSession()->GetPlayer()->SwapItem(src, dst);

        return true;
    }

    static bool HandleCooldownCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* owner = target->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner)
        {
            owner = handler->GetSession()->GetPlayer();
            target = owner;
        }

        std::string nameLink = handler->GetNameLink(owner);

        if (!*args)
        {
            target->GetSpellHistory()->ResetAllCooldowns();
            target->GetSpellHistory()->ResetAllCharges();
            handler->PSendSysMessage(LANG_REMOVEALL_COOLDOWN, nameLink.c_str());
        }
        else
        {
            // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
            uint32 spellIid = handler->extractSpellIdFromLink((char*)args);
            if (!spellIid)
                return false;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellIid, target->GetMap()->GetDifficultyID());
            if (!spellInfo)
            {
                handler->PSendSysMessage(LANG_UNKNOWN_SPELL, owner == handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : nameLink.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            target->GetSpellHistory()->ResetCooldown(spellIid, true);
            target->GetSpellHistory()->ResetCharges(spellInfo->ChargeCategoryId);
            handler->PSendSysMessage(LANG_REMOVE_COOLDOWN, spellIid, owner == handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : nameLink.c_str());
        }
        return true;
    }

    static bool HandleGetDistanceCommand(ChatHandler* handler, char const* args)
    {
        WorldObject* object = nullptr;
        if (*args)
        {
            HighGuid guidHigh;
            ObjectGuid::LowType guidLow = handler->extractLowGuidFromLink((char*)args, guidHigh);
            if (!guidLow)
                return false;
            switch (guidHigh)
            {
            case HighGuid::Player:
            {
                object = ObjectAccessor::GetPlayer(*handler->GetSession()->GetPlayer(), ObjectGuid::Create<HighGuid::Player>(guidLow));
                if (!object)
                {
                    handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            case HighGuid::Creature:
            {
                object = handler->GetCreatureFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            case HighGuid::GameObject:
            {
                object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
                if (!object)
                {
                    handler->SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);
                    handler->SetSentErrorMessage(true);
                }
                break;
            }
            default:
                return false;
            }
            if (!object)
                return false;
        }
        else
        {
            object = handler->getSelectedUnit();
            if (!object)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        handler->PSendSysMessage(LANG_DISTANCE, handler->GetSession()->GetPlayer()->GetDistance(object), handler->GetSession()->GetPlayer()->GetDistance2d(object), handler->GetSession()->GetPlayer()->GetExactDist(object), handler->GetSession()->GetPlayer()->GetExactDist2d(object));
        return true;
    }
    // Teleport player to last position
    static bool HandleRecallCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        if (target->IsBeingTeleported())
        {
            handler->PSendSysMessage(LANG_IS_TELEPORTED, handler->GetNameLink(target).c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (target->IsInFlight())
        {
            target->GetMotionMaster()->MovementExpired();
            target->CleanupAfterTaxiFlight();
        }

        target->Recall();
        return true;
    }

    static bool HandleSaveCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // save GM account without delay and output message
        if (handler->GetSession()->HasPermission(rbac::RBAC_PERM_COMMANDS_SAVE_WITHOUT_DELAY))
        {
            if (Player* target = handler->getSelectedPlayer())
                target->SaveToDB();
            else
                player->SaveToDB();
            handler->SendSysMessage(LANG_PLAYER_SAVED);
            return true;
        }

        // save if the player has last been saved over 20 seconds ago
        uint32 saveInterval = sWorld->getIntConfig(CONFIG_INTERVAL_SAVE);
        if (saveInterval == 0 || (saveInterval > 20 * IN_MILLISECONDS && player->GetSaveTimer() <= saveInterval - 20 * IN_MILLISECONDS))
            player->SaveToDB();

        return true;
    }

    // Save all players in the world
    static bool HandleSaveAllCommand(ChatHandler* handler, char const* /*args*/)
    {
        ObjectAccessor::SaveAllPlayers();
        handler->SendSysMessage(LANG_PLAYERS_SAVED);
        return true;
    }

    // kick player
    static bool HandleKickPlayerCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        std::string playerName;
        if (!handler->extractPlayerTarget((char*)args, &target, nullptr, &playerName))
            return false;

        if (handler->GetSession() && target == handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_COMMAND_KICKSELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        std::string kickReasonStr = handler->GetTrinityString(LANG_NO_REASON);
        if (*args != '\0')
        {
            char const* kickReason = strtok(nullptr, "\r");
            if (kickReason != nullptr)
                kickReasonStr = kickReason;
        }

        if (sWorld->getBoolConfig(CONFIG_SHOW_KICK_IN_WORLD))
            sWorld->SendWorldText(LANG_COMMAND_KICKMESSAGE_WORLD, (handler->GetSession() ? handler->GetSession()->GetPlayerName().c_str() : "Server"), playerName.c_str(), kickReasonStr.c_str());
        else
            handler->PSendSysMessage(LANG_COMMAND_KICKMESSAGE, playerName.c_str());

        target->GetSession()->KickPlayer();

        return true;
    }

    static bool HandleUnstuckCommand(ChatHandler* handler, char const* args)
    {
#define SPELL_UNSTUCK_ID 7355
#define SPELL_UNSTUCK_VISUAL 2683

        // No args required for players
        if (handler->GetSession() && !handler->GetSession()->HasPermission(rbac::RBAC_PERM_COMMANDS_USE_UNSTUCK_WITH_ARGS))
        {
            // 7355: "Stuck"
            if (Player* player = handler->GetSession()->GetPlayer())
                player->CastSpell(player, SPELL_UNSTUCK_ID, false);
            return true;
        }

        if (!*args)
            return false;

        char* player_str = strtok((char*)args, " ");
        if (!player_str)
            return false;

        std::string location_str = "inn";
        if (char const* loc = strtok(nullptr, " "))
            location_str = loc;

        Player* player = nullptr;
        ObjectGuid targetGUID;
        if (!handler->extractPlayerTarget(player_str, &player, &targetGUID))
            return false;

        if (!player)
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_HOMEBIND);
            stmt->setUInt64(0, targetGUID.GetCounter());
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                Field* fields = result->Fetch();

                CharacterDatabaseTransaction dummy;
                Player::SavePositionInDB(WorldLocation(fields[0].GetUInt16(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), 0.0f), fields[1].GetUInt16(), targetGUID, dummy);
                return true;
            }

            return false;
        }

        if (player->IsInFlight() || player->IsInCombat())
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_UNSTUCK_ID, DIFFICULTY_NONE);
            if (!spellInfo)
                return false;

            if (Player* caster = handler->GetSession()->GetPlayer())
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, player->GetMapId(), SPELL_UNSTUCK_ID, player->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Spell::SendCastResult(caster, spellInfo, { SPELL_UNSTUCK_VISUAL, 0 }, castId, SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW);
            }

            return false;
        }

        if (location_str == "inn")
        {
            player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
            return true;
        }

        if (location_str == "graveyard")
        {
            player->RepopAtGraveyard();
            return true;
        }

        if (location_str == "startzone")
        {
            player->TeleportTo(player->GetStartPosition());
            return true;
        }

        //Not a supported argument
        return false;

    }

    static bool HandleLinkGraveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* px = strtok((char*)args, " ");
        if (!px)
            return false;

        uint32 graveyardId = atoul(px);

        uint32 team;

        char* px2 = strtok(nullptr, " ");

        if (!px2)
            team = 0;
        else if (strncmp(px2, "horde", 6) == 0)
            team = HORDE;
        else if (strncmp(px2, "alliance", 9) == 0)
            team = ALLIANCE;
        else
            return false;

        WorldSafeLocsEntry const* graveyard = sObjectMgr->GetWorldSafeLoc(graveyardId);

        if (!graveyard)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, graveyardId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        uint32 zoneId = player->GetZoneId();

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(zoneId);
        if (!areaEntry || areaEntry->ParentAreaID != 0)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, graveyardId, zoneId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (sObjectMgr->AddGraveyardLink(graveyardId, zoneId, team))
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, graveyardId, zoneId);
        else
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, graveyardId, zoneId);

        return true;
    }

    static bool HandleNearGraveCommand(ChatHandler* handler, char const* args)
    {
        uint32 team;

        size_t argStr = strlen(args);

        if (!*args)
            team = 0;
        else if (strncmp((char*)args, "horde", argStr) == 0)
            team = HORDE;
        else if (strncmp((char*)args, "alliance", argStr) == 0)
            team = ALLIANCE;
        else
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 zone_id = player->GetZoneId();

        WorldSafeLocsEntry const* graveyard = sObjectMgr->GetClosestGraveyard(*player, team, nullptr);
        if (graveyard)
        {
            uint32 graveyardId = graveyard->ID;

            GraveyardData const* data = sObjectMgr->FindGraveyardData(graveyardId, zone_id);
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR, graveyardId);
                handler->SetSentErrorMessage(true);
                return false;
            }

            team = data->team;

            std::string team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_NOTEAM);

            if (team == 0)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
            else if (team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if (team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, graveyardId, team_name.c_str(), zone_id);
        }
        else
        {
            std::string team_name;

            if (team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if (team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            if (!team)
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, zone_id);
            else
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, zone_id, team_name.c_str());
        }

        return true;
    }

    static bool HandleShowAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(atoi(args));
        if (!area)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (area->AreaBit < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 offset = area->AreaBit / 64;
        if (offset >= PLAYER_EXPLORED_ZONES_SIZE)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint64 val = UI64LIT(1) << (area->AreaBit % 64);
        playerTarget->AddExploredZones(offset, val);

        handler->SendSysMessage(LANG_EXPLORE_AREA);
        return true;
    }

    static bool HandleHideAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(atoi(args));
        if (!area)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (area->AreaBit < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 offset = area->AreaBit / 64;
        if (offset >= PLAYER_EXPLORED_ZONES_SIZE)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint64 val = UI64LIT(1) << (area->AreaBit % 64);
        playerTarget->RemoveExploredZones(offset, val);

        handler->SendSysMessage(LANG_UNEXPLORE_AREA);
        return true;
    }

    static bool HandleAddItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 itemId = 0;

        if (args[0] == '[')                                        // [name] manual form
        {
            char const* itemNameStr = strtok((char*)args, "]");

            if (itemNameStr && itemNameStr[0])
            {
                std::string itemName = itemNameStr + 1;
                auto itr = std::find_if(sItemSparseStore.begin(), sItemSparseStore.end(), [&itemName](ItemSparseEntry const* sparse)
                {
                    for (LocaleConstant i = LOCALE_enUS; i < TOTAL_LOCALES; i = LocaleConstant(i + 1))
                        if (itemName == sparse->Display[i])
                            return true;
                    return false;
                });

                if (itr == sItemSparseStore.end())
                {
                    handler->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, itemNameStr + 1);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                itemId = itr->ID;
            }
            else
                return false;
        }
        else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
        {
            char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
            if (!id)
                return false;
            itemId = atoul(id);
        }

        char const* ccount = strtok(nullptr, " ");

        int32 count = 1;

        if (ccount)
            count = strtol(ccount, nullptr, 10);

        if (count == 0)
            count = 1;

        std::vector<int32> bonusListIDs;
        char const* bonuses = strtok(nullptr, " ");

        // semicolon separated bonuslist ids (parse them after all arguments are extracted by strtok!)
        if (bonuses)
        {
            Tokenizer tokens(bonuses, ';');
            for (char const* token : tokens)
                bonusListIDs.push_back(atoul(token));
        }


        // LegionHearth custom
        char const* transmog = strtok(NULL, " ");
        uint32 transmogId = 0;
        if (transmog)
        {
            transmogId = strtol(transmog, NULL, 10);

        }

        char const* enchant = strtok(NULL, " ");
        uint32 enchantId = 0;
        if (enchant)
        {
            enchantId = strtol(enchant, NULL, 10);
        }


        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        TC_LOG_DEBUG("misc", handler->GetTrinityString(LANG_ADDITEM), itemId, count);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // LegionHearth
        ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(itemId);
        uint8 artifact = itemProto->GetArtifactID();

        // Subtract (modif by LEGIONHEARTH team = ) )
        if (count < 0)
        {

            if (artifact < 1)
            {
                playerTarget->DestroyItemCount(itemId, -count, true, false);
                handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(playerTarget).c_str());

            }
            else
            {
                handler->PSendSysMessage(LANG_REMOVE_ARTIFACT);
            }
            return true;
        }

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            count -= noSpaceForCount;

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player != playerTarget && artifact > 0)
        {
            handler->PSendSysMessage(LANG_ADD_ARTIFACT_TO_PLAYER);
            return false;
        }

        Item* item = playerTarget->StoreNewItem(dest, itemId, true, GenerateItemRandomBonusListId(itemId), GuidSet(), ItemContext::NONE, bonusListIDs,transmogId, enchantId);
        
        // remove binding (let GM give it to another player later)
        if (player == playerTarget)
            for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if (Item* item1 = player->GetItemByPos(itr->pos))
                    item1->SetBinding(false);

        if (count > 0 && item)
        {
            player->SendNewItem(item, count, false, true);
            if (player != playerTarget)
                playerTarget->SendNewItem(item, count, true, false);
        }

        if (noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleAddItemSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* id = handler->extractKeyFromLink((char*)args, "Hitemset"); // number or [name] Shift-click form |color|Hitemset:itemset_id|h[name]|h|r
        if (!id)
            return false;

        uint32 itemSetId = atoul(id);

        // prevent generation all items with itemset field value '0'
        if (itemSetId == 0)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemSetId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::vector<int32> bonusListIDs;
        char const* bonuses = strtok(nullptr, " ");

        // semicolon separated bonuslist ids (parse them after all arguments are extracted by strtok!)
        if (bonuses)
        {
            Tokenizer tokens(bonuses, ';');
            for (char const* token : tokens)
                bonusListIDs.push_back(atoul(token));
        }

        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        TC_LOG_DEBUG("misc", handler->GetTrinityString(LANG_ADDITEMSET), itemSetId);

        bool found = false;
        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            if (itr->second.GetItemSet() == itemSetId)
            {
                found = true;
                ItemPosCountVec dest;
                InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itr->second.GetId(), 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = playerTarget->StoreNewItem(dest, itr->second.GetId(), true, {}, GuidSet(), ItemContext::NONE, bonusListIDs);

                    // remove binding (let GM give it to another player later)
                    if (player == playerTarget)
                        item->SetBinding(false);

                    player->SendNewItem(item, 1, false, true);
                    if (player != playerTarget)
                        playerTarget->SendNewItem(item, 1, true, false);
                }
                else
                {
                    player->SendEquipError(msg, nullptr, nullptr, itr->second.GetId());
                    handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itr->second.GetId(), 1);
                }
            }
        }

        if (!found)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemSetId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->SendShowBank(handler->GetSession()->GetPlayer()->GetGUID());
        return true;
    }

    static bool HandleChangeWeather(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Weather is OFF
        if (!sWorld->getBoolConfig(CONFIG_WEATHER))
        {
            handler->SendSysMessage(LANG_WEATHER_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // *Change the weather of a cell
        char const* px = strtok((char*)args, " ");
        char const* py = strtok(nullptr, " ");

        if (!px || !py)
            return false;

        uint32 type = uint32(atoi(px));                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
        float grade = float(atof(py));                          //0 to 1, sending -1 is instand good weather

        Player* player = handler->GetSession()->GetPlayer();
        uint32 zoneid = player->GetZoneId();

        Weather* weather = player->GetMap()->GetOrGenerateZoneDefaultWeather(zoneid);
        if (!weather)
        {
            handler->SendSysMessage(LANG_NO_WEATHER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        weather->SetWeather(WeatherType(type), grade);

        return true;
    }

    static bool HandleSetSkillCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
        char const* skillStr = handler->extractKeyFromLink((char*)args, "Hskill");
        if (!skillStr)
            return false;

        char const* levelStr = strtok(nullptr, " ");
        if (!levelStr)
            return false;

        char const* maxPureSkill = strtok(nullptr, " ");

        uint32 skill = atoul(skillStr);
        if (skill == 0)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 level = atoul(levelStr);

        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skill);
        if (!skillLine)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        bool targetHasSkill = target->GetSkillValue(skill) != 0;

        // If our target does not yet have the skill they are trying to add to them, the chosen level also becomes
        // the max level of the new profession.
        uint16 max = maxPureSkill ? atoul(maxPureSkill) : targetHasSkill ? target->GetPureMaxSkillValue(skill) : uint16(level);

        if (level == 0 || level > max || max <= 0)
            return false;

        // If the player has the skill, we get the current skill step. If they don't have the skill, we
        // add the skill to the player's book with step 1 (which is the first rank, in most cases something
        // like 'Apprentice <skill>'.
        target->SetSkill(skill, targetHasSkill ? target->GetSkillStep(skill) : 1, level, max);
        handler->PSendSysMessage(LANG_SET_SKILL, skill, skillLine->DisplayName[handler->GetSessionDbcLocale()], handler->GetNameLink(target).c_str(), level, max);
        return true;
    }

    /**
    * @name Player command: .pinfo
    * @date 05/19/2013
    *
    * @brief Prints information about a character and it's linked account to the commander
    *
    * Non-applying information, e.g. a character that is not in gm mode right now or
    * that is not banned/muted, is not printed
    *
    * This can be done either by giving a name or by targeting someone, else, it'll use the commander
    *
    * @param args name   Prints information according to the given name to the commander
    *             target Prints information on the target to the commander
    *             none   No given args results in printing information on the commander
    *
    * @return Several pieces of information about the character and the account
    **/

    static bool HandlePInfoCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        CharacterDatabasePreparedStatement* stmt = nullptr;

        // To make sure we get a target, we convert our guid to an omniversal...
        ObjectGuid parseGUID = ObjectGuid::Create<HighGuid::Player>(strtoull(args, nullptr, 10));

        // ... and make sure we get a target, somehow.
        if (sCharacterCache->GetCharacterNameByGuid(parseGUID, targetName))
        {
            target = ObjectAccessor::FindPlayer(parseGUID);
            targetGuid = parseGUID;
        }
        // if not, then return false. Which shouldn't happen, now should it ?
        else if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        /* The variables we extract for the command. They are
         * default as "does not exist" to prevent problems
         * The output is printed in the follow manner:
         *
         * Player %s %s (guid: %u)                   - I.    LANG_PINFO_PLAYER
         * ** GM Mode active, Phase: -1              - II.   LANG_PINFO_GM_ACTIVE (if GM)
         * ** Banned: (Type, Reason, Time, By)       - III.  LANG_PINFO_BANNED (if banned)
         * ** Muted: (Reason, Time, By)              - IV.   LANG_PINFO_MUTED (if muted)
         * * Account: %s (id: %u), GM Level: %u      - V.    LANG_PINFO_ACC_ACCOUNT
         * * Last Login: %u (Failed Logins: %u)      - VI.   LANG_PINFO_ACC_LASTLOGIN
         * * Uses OS: %s - Latency: %u ms            - VII.  LANG_PINFO_ACC_OS
         * * Registration Email: %s - Email: %s      - VIII. LANG_PINFO_ACC_REGMAILS
         * * Last IP: %u (Locked: %s)                - IX.   LANG_PINFO_ACC_IP
         * * Level: %u (%u/%u XP (%u XP left)        - X.    LANG_PINFO_CHR_LEVEL
         * * Race: %s %s, Class %s                   - XI.   LANG_PINFO_CHR_RACE
         * * Alive ?: %s                             - XII.  LANG_PINFO_CHR_ALIVE
         * * Phases: %s                              - XIII. LANG_PINFO_CHR_PHASE (if not GM)
         * * Money: %ug%us%uc                        - XIV.  LANG_PINFO_CHR_MONEY
         * * Map: %s, Area: %s                       - XV.   LANG_PINFO_CHR_MAP
         * * Guild: %s (Id: %s)                      - XVI.  LANG_PINFO_CHR_GUILD (if in guild)
         * ** Rank: %s, ID: %u                       - XVII. LANG_PINFO_CHR_GUILD_RANK (if in guild)
         * ** Note: %s                               - XVIII.LANG_PINFO_CHR_GUILD_NOTE (if in guild and has note)
         * ** O. Note: %s                            - XVIX. LANG_PINFO_CHR_GUILD_ONOTE (if in guild and has officer note)
         * * Played time: %s                         - XX.   LANG_PINFO_CHR_PLAYEDTIME
         * * Mails: %u Read/%u Total                 - XXI.  LANG_PINFO_CHR_MAILS (if has mails)
         *
         * Not all of them can be moved to the top. These should
         * place the most important ones to the head, though.
         *
         * For a cleaner overview, I segment each output in Roman numerals
         */

         // Account data print variables
        std::string userName = handler->GetTrinityString(LANG_ERROR);
        uint32 accId = 0;
        ObjectGuid::LowType lowguid = targetGuid.GetCounter();
        std::string eMail = handler->GetTrinityString(LANG_ERROR);
        std::string regMail = handler->GetTrinityString(LANG_ERROR);
        uint32 security = 0;
        std::string lastIp = handler->GetTrinityString(LANG_ERROR);
        uint8 locked = 0;
        std::string lastLogin = handler->GetTrinityString(LANG_ERROR);
        uint32 failedLogins = 0;
        uint32 latency = 0;
        std::string OS = handler->GetTrinityString(LANG_UNKNOWN);
        uint32 hddID = 0;
        uint32 cpuID = 0;
        uint32 volumeID = 0;
        bool isVM = false;

        // Mute data print variables
        int64 muteTime = -1;
        std::string muteReason = handler->GetTrinityString(LANG_NO_REASON);
        std::string muteBy = handler->GetTrinityString(LANG_UNKNOWN);

        // Ban data print variables
        int64 banTime = -1;
        std::string banType = handler->GetTrinityString(LANG_UNKNOWN);
        std::string banReason = handler->GetTrinityString(LANG_NO_REASON);
        std::string bannedBy = handler->GetTrinityString(LANG_UNKNOWN);

        // Character data print variables
        uint8 raceid, classid = 0; //RACE_NONE, CLASS_NONE
        std::string raceStr, classStr = handler->GetTrinityString(LANG_UNKNOWN);
        uint8 gender = 0;
        LocaleConstant locale = handler->GetSessionDbcLocale();
        uint32 totalPlayerTime = 0;
        uint8 level = 0;
        std::string alive = handler->GetTrinityString(LANG_ERROR);
        uint64 money = 0;
        uint32 xp = 0;
        uint32 xptotal = 0;

        // Position data print
        uint32 mapId;
        uint32 areaId;
        std::string areaName    = handler->GetTrinityString(LANG_UNKNOWN);
        std::string zoneName    = handler->GetTrinityString(LANG_UNKNOWN);

        // Guild data print variables defined so that they exist, but are not necessarily used
        ObjectGuid::LowType guildId = UI64LIT(0);
        uint8 guildRankId = 0;
        std::string guildName;
        std::string guildRank;
        std::string note;
        std::string officeNote;

        // Mail data print is only defined if you have a mail

        if (target)
        {
            // check online security
            if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                return false;

            accId             = target->GetSession()->GetAccountId();
            money             = target->GetMoney();
            totalPlayerTime   = target->GetTotalPlayedTime();
            level             = target->getLevel();
            latency           = target->GetSession()->GetLatency();
            raceid            = target->getRace();
            classid           = target->getClass();
            muteTime          = target->GetSession()->m_muteTime;
            mapId             = target->GetMapId();
            areaId            = target->GetAreaId();
            alive             = target->IsAlive() ? handler->GetTrinityString(LANG_YES) : handler->GetTrinityString(LANG_NO);
            gender            = target->GetNativeSex();
        }
        // get additional information from DB
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(nullptr, targetGuid))
                return false;

            // Query informations from the DB
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_PINFO);
            stmt->setUInt64(0, lowguid);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
                return false;

            Field* fields = result->Fetch();
            totalPlayerTime = fields[0].GetUInt32();
            level = fields[1].GetUInt8();
            money = fields[2].GetUInt64();
            accId = fields[3].GetUInt32();
            raceid = fields[4].GetUInt8();
            classid = fields[5].GetUInt8();
            mapId = fields[6].GetUInt16();
            areaId = fields[7].GetUInt16();
            gender = fields[8].GetUInt8();
            uint32 health = fields[9].GetUInt32();
            uint32 playerFlags = fields[10].GetUInt32();

            if (!health || playerFlags & PLAYER_FLAGS_GHOST)
                alive = handler->GetTrinityString(LANG_NO);
            else
                alive = handler->GetTrinityString(LANG_YES);
        }

        // Query the prepared statement for login data
        LoginDatabasePreparedStatement* stmt2 = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
        stmt2->setInt32(0, int32(realm.Id.Realm));
        stmt2->setUInt32(1, accId);
        PreparedQueryResult result = LoginDatabase.Query(stmt2);

        if (result)
        {
            Field* fields = result->Fetch();
            userName = fields[0].GetString();
            security = fields[1].GetUInt8();

            // Only fetch these fields if commander has sufficient rights)
            if (handler->HasPermission(rbac::RBAC_PERM_COMMANDS_PINFO_CHECK_PERSONAL_DATA) && // RBAC Perm. 48, Role 39
                (!handler->GetSession() || handler->GetSession()->GetSecurity() >= AccountTypes(security)))
            {
                eMail = fields[2].GetString();
                regMail = fields[3].GetString();
                lastIp = fields[4].GetString();
                lastLogin = fields[5].GetString();

                if (IpLocationRecord const* location = sIPLocation->GetLocationRecord(lastIp))
                {
                    lastIp.append(" (");
                    lastIp.append(location->CountryName);
                    lastIp.append(")");
                }
            }
            else
            {
                eMail = handler->GetTrinityString(LANG_UNAUTHORIZED);
                regMail = handler->GetTrinityString(LANG_UNAUTHORIZED);
                lastIp = handler->GetTrinityString(LANG_UNAUTHORIZED);
                lastLogin = handler->GetTrinityString(LANG_UNAUTHORIZED);
            }
            muteTime = fields[6].GetUInt64();
            muteReason = fields[7].GetString();
            muteBy = fields[8].GetString();
            failedLogins = fields[9].GetUInt32();
            locked = fields[10].GetUInt8();
            OS = fields[11].GetString();
        }

        LoginDatabasePreparedStatement* hwidBan = LoginDatabase.GetPreparedStatement(LOGIN_SEL_HWID_INFO_ACC);
        hwidBan->setUInt32(0, accId);
        PreparedQueryResult hwidDataRequest = LoginDatabase.Query(hwidBan);

        if (hwidDataRequest) {
            Field* fieldsHwid = hwidDataRequest->Fetch();
            hddID = fieldsHwid[0].GetUInt32();
            cpuID = fieldsHwid[1].GetUInt32();
            volumeID = fieldsHwid[2].GetUInt32();
            isVM = fieldsHwid[3].GetBool();
        }

        // Creates a chat link to the character. Returns nameLink
        std::string nameLink = handler->playerLink(targetName);

        // Returns banType, banTime, bannedBy, banreason
        stmt2 = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO_BANS);
        stmt2->setUInt32(0, accId);
        PreparedQueryResult result2 = LoginDatabase.Query(stmt2);
        if (!result2)
        {
            banType = handler->GetTrinityString(LANG_CHARACTER);
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_BANS);
            stmt->setUInt64(0, lowguid);
            result2 = CharacterDatabase.Query(stmt);
        }

        if (result2)
        {
            Field* fields = result2->Fetch();
            banTime = int64(fields[1].GetUInt64() ? 0 : fields[0].GetUInt32());
            bannedBy = fields[2].GetString();
            banReason = fields[3].GetString();
        }

        // Can be used to query data from Characters database
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_XP);
        stmt->setUInt64(0, lowguid);
        PreparedQueryResult result4 = CharacterDatabase.Query(stmt);

        if (result4)
        {
            Field* fields = result4->Fetch();
            xp = fields[0].GetUInt32(); // Used for "current xp" output and "%u XP Left" calculation
            ObjectGuid::LowType gguid = fields[1].GetUInt64(); // We check if have a guild for the person, so we might not require to query it at all
            xptotal = sObjectMgr->GetXPForLevel(level);

            if (gguid)
            {
                // Guild Data - an own query, because it may not happen.
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER_EXTENDED);
                stmt->setUInt64(0, lowguid);
                PreparedQueryResult result5 = CharacterDatabase.Query(stmt);
                if (result5)
                {
                    Field* fields5 = result5->Fetch();
                    guildId = fields5[0].GetUInt64();
                    guildName = fields5[1].GetString();
                    guildRank = fields5[2].GetString();
                    guildRankId = fields5[3].GetUInt8();
                    note = fields5[4].GetString();
                    officeNote = fields5[5].GetString();
                }
            }
        }

        // Initiate output
        // Output I. LANG_PINFO_PLAYER
        handler->PSendSysMessage(LANG_PINFO_PLAYER, target ? "" : handler->GetTrinityString(LANG_OFFLINE), nameLink.c_str(), targetGuid.ToString().c_str());

        // Output II. LANG_PINFO_GM_ACTIVE if character is gamemaster
        if (target && target->IsGameMaster())
            handler->PSendSysMessage(LANG_PINFO_GM_ACTIVE);

        // Output III. LANG_PINFO_BANNED if ban exists and is applied
        if (banTime >= 0)
            handler->PSendSysMessage(LANG_PINFO_BANNED, banType.c_str(), banReason.c_str(), banTime > 0 ? secsToTimeString(banTime - time(nullptr), true).c_str() : handler->GetTrinityString(LANG_PERMANENTLY), bannedBy.c_str());

        // Output IV. LANG_PINFO_MUTED if mute is applied
        if (muteTime > 0)
            handler->PSendSysMessage(LANG_PINFO_MUTED, muteReason.c_str(), secsToTimeString(muteTime - time(nullptr), true).c_str(), muteBy.c_str());

        // Output V. LANG_PINFO_ACC_ACCOUNT
        handler->PSendSysMessage(LANG_PINFO_ACC_ACCOUNT, userName.c_str(), accId, security);

        // Output VI. LANG_PINFO_ACC_LASTLOGIN
        handler->PSendSysMessage(LANG_PINFO_ACC_LASTLOGIN, lastLogin.c_str(), failedLogins);

        // Output VII. LANG_PINFO_ACC_OS
        handler->PSendSysMessage(LANG_PINFO_ACC_OS, OS.c_str(), latency);

        // Output VII. bis LANG_PINFO_ACC_HWID
        handler->PSendSysMessage("? VM: %s", isVM ? "true" : "false");

        // Output VIII. LANG_PINFO_ACC_REGMAILS
        handler->PSendSysMessage(LANG_PINFO_ACC_REGMAILS, regMail.c_str(), eMail.c_str());

        // Output IX. LANG_PINFO_ACC_IP
        handler->PSendSysMessage(LANG_PINFO_ACC_IP, lastIp.c_str(), locked ? handler->GetTrinityString(LANG_YES) : handler->GetTrinityString(LANG_NO));

        // Output X. LANG_PINFO_CHR_LEVEL
        if (level != sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            handler->PSendSysMessage(LANG_PINFO_CHR_LEVEL_LOW, level, xp, xptotal, (xptotal - xp));
        else
            handler->PSendSysMessage(LANG_PINFO_CHR_LEVEL_HIGH, level);

        // Output XI. LANG_PINFO_CHR_RACE
        raceStr = DB2Manager::GetChrRaceName(raceid, locale);
        classStr = DB2Manager::GetClassName(classid, locale);
        handler->PSendSysMessage(LANG_PINFO_CHR_RACE, (gender == 0 ? handler->GetTrinityString(LANG_CHARACTER_GENDER_MALE) : handler->GetTrinityString(LANG_CHARACTER_GENDER_FEMALE)), raceStr.c_str(), classStr.c_str());

        // Output XII. LANG_PINFO_CHR_ALIVE
        handler->PSendSysMessage(LANG_PINFO_CHR_ALIVE, alive.c_str());

        // Output XIII. phases
        if (target)
            PhasingHandler::PrintToChat(handler, target->GetPhaseShift());

        // Output XIV. LANG_PINFO_CHR_MONEY
        uint32 gold = money / GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;
        handler->PSendSysMessage(LANG_PINFO_CHR_MONEY, gold, silv, copp);

        // Position data
        MapEntry const* map = sMapStore.LookupEntry(mapId);
        AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);
        if (area)
        {
            areaName = area->AreaName[handler->GetSessionDbcLocale()];

            AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->ParentAreaID);
            if (zone)
                zoneName = zone->AreaName[handler->GetSessionDbcLocale()];
        }

        if (target)
            handler->PSendSysMessage(LANG_PINFO_CHR_MAP, map->MapName[handler->GetSessionDbcLocale()],
                (!zoneName.empty() ? zoneName.c_str() : handler->GetTrinityString(LANG_UNKNOWN)),
                (!areaName.empty() ? areaName.c_str() : handler->GetTrinityString(LANG_UNKNOWN)));

        // Output XVII. - XVIX. if they are not empty
        if (!guildName.empty())
        {
            handler->PSendSysMessage(LANG_PINFO_CHR_GUILD, guildName.c_str(), std::to_string(guildId).c_str());
            handler->PSendSysMessage(LANG_PINFO_CHR_GUILD_RANK, guildRank.c_str(), uint32(guildRankId));
            if (!note.empty())
                handler->PSendSysMessage(LANG_PINFO_CHR_GUILD_NOTE, note.c_str());
            if (!officeNote.empty())
                handler->PSendSysMessage(LANG_PINFO_CHR_GUILD_ONOTE, officeNote.c_str());
        }

        // Output XX. LANG_PINFO_CHR_PLAYEDTIME
        handler->PSendSysMessage(LANG_PINFO_CHR_PLAYEDTIME, (secsToTimeString(totalPlayerTime, true, true)).c_str());

        // Mail Data - an own query, because it may or may not be useful.
        // SQL: "SELECT SUM(CASE WHEN (checked & 1) THEN 1 ELSE 0 END) AS 'readmail', COUNT(*) AS 'totalmail' FROM mail WHERE `receiver` = ?"
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_MAILS);
        stmt->setUInt64(0, lowguid);
        PreparedQueryResult result6 = CharacterDatabase.Query(stmt);
        if (result6)
        {
            Field* fields = result6->Fetch();
            uint32 readmail = uint32(fields[0].GetDouble());
            uint32 totalmail = uint32(fields[1].GetUInt64());

            // Output XXI. LANG_INFO_CHR_MAILS if at least one mail is given
            if (totalmail >= 1)
                handler->PSendSysMessage(LANG_PINFO_CHR_MAILS, readmail, totalmail);
        }

        CharacterDatabasePreparedStatement* stmt5 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_MAILINFO_SEND);
        stmt5->setUInt64(0, lowguid);
        PreparedQueryResult result7 = CharacterDatabase.Query(stmt5);
        if (result7) {
            do {
                Field* fields = result7->Fetch();
                uint32 ID = fields[0].GetUInt32();
                std::string RECEIVER = fields[1].GetString();
                std::string SUBJECT = fields[2].GetString();

                handler->PSendSysMessage(LANG_PINFO_CHR_MAILINFO_SEND, ID, RECEIVER.c_str(), SUBJECT.c_str());

            } while (result7->NextRow());
        }

        CharacterDatabasePreparedStatement* stmt6 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_MAILINFO_RECEIVE);
        stmt6->setUInt64(0, lowguid);
        PreparedQueryResult result8 = CharacterDatabase.Query(stmt6);
        if (result8) {
            do {
                Field* fields = result8->Fetch();
                uint32 ID = fields[0].GetUInt32();
                std::string SENDER = fields[1].GetString();
                std::string SUBJECT = fields[2].GetString();

                handler->PSendSysMessage(LANG_PINFO_CHR_MAILINFO_RECEIVE, ID, SENDER.c_str(), SUBJECT.c_str());

            } while (result8->NextRow());
        }

        return true;
    }

    static bool HandleRespawnCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // accept only explicitly selected target (not implicitly self targeting case)
        Creature* target = !player->GetTarget().IsEmpty() ? handler->getSelectedCreature() : nullptr;
        if (target)
        {
            if (target->IsPet())
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (target->isDead())
                target->Respawn();
            return true;
        }

        // First handle any creatures that still have a corpse around
        Trinity::RespawnDo u_do;
        Trinity::WorldObjectWorker<Trinity::RespawnDo> worker(player, u_do);
        Cell::VisitGridObjects(player, worker, player->GetGridActivationRange());

        // Now handle any that had despawned, but had respawn time logged.
        RespawnVector data;
        player->GetMap()->GetRespawnInfo(data, SPAWN_TYPEMASK_ALL, 0);
        if (!data.empty())
        {
            uint32 const gridId = Trinity::ComputeGridCoord(player->GetPositionX(), player->GetPositionY()).GetId();
            for (RespawnInfo* info : data)
                if (info->gridId == gridId)
                    player->GetMap()->RemoveRespawnTime(info, true);
        }

        return true;
    }

    // mute player for some times
    static bool HandleMuteCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* delayStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &delayStr);
        if (!delayStr)
            return false;

        char const* muteReason = strtok(nullptr, "\r");
        std::string muteReasonStr = handler->GetTrinityString(LANG_NO_REASON);
        if (muteReason != nullptr)
            muteReasonStr = muteReason;

        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &targetName))
            return false;

        uint32 accountId = target ? target->GetSession()->GetAccountId() : sCharacterCache->GetCharacterAccountIdByGuid(targetGuid);

        // find only player from same account if any
        if (!target)
            if (WorldSession* session = sWorld->FindSession(accountId))
                target = session->GetPlayer();

        uint32 notSpeakTime = uint32(atoi(delayStr));

        // must have strong lesser security level
        if (handler->HasLowerSecurity(target, targetGuid, true))
            return false;

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
        std::string muteBy = "";
        if (handler->GetSession())
            muteBy = handler->GetSession()->GetPlayerName();
        else
            muteBy = handler->GetTrinityString(LANG_CONSOLE);

        if (target)
        {
            // Target is online, mute will be in effect right away.
            int64 muteTime = time(nullptr) + notSpeakTime * MINUTE;
            target->GetSession()->m_muteTime = muteTime;
            stmt->setInt64(0, muteTime);
            std::string nameLink = handler->playerLink(targetName);

            if (sWorld->getBoolConfig(CONFIG_SHOW_MUTE_IN_WORLD))
                sWorld->SendWorldText(LANG_COMMAND_MUTEMESSAGE_WORLD, muteBy.c_str(), nameLink.c_str(), notSpeakTime, muteReasonStr.c_str());

            ChatHandler(target->GetSession()).PSendSysMessage(LANG_YOUR_CHAT_DISABLED, notSpeakTime, muteBy.c_str(), muteReasonStr.c_str());
        }
        else
        {
            // Target is offline, mute will be in effect starting from the next login.
            int32 muteTime = -int32(notSpeakTime * MINUTE);
            stmt->setInt64(0, muteTime);
        }

        stmt->setString(1, muteReasonStr.c_str());
        stmt->setString(2, muteBy.c_str());
        stmt->setUInt32(3, accountId);
        LoginDatabase.Execute(stmt);
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_MUTE);
        stmt->setUInt32(0, accountId);
        stmt->setUInt32(1, notSpeakTime);
        stmt->setString(2, muteBy.c_str());
        stmt->setString(3, muteReasonStr.c_str());
        LoginDatabase.Execute(stmt);
        std::string nameLink = handler->playerLink(targetName);

        if (sWorld->getBoolConfig(CONFIG_SHOW_MUTE_IN_WORLD) && !target)
            sWorld->SendWorldText(LANG_COMMAND_MUTEMESSAGE_WORLD, muteBy.c_str(), nameLink.c_str(), notSpeakTime, muteReasonStr.c_str());
        else
            handler->PSendSysMessage(target ? LANG_YOU_DISABLE_CHAT : LANG_COMMAND_DISABLE_CHAT_DELAYED, nameLink.c_str(), notSpeakTime, muteReasonStr.c_str());

        return true;
    }

    // unmute player
    static bool HandleUnmuteCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        uint32 accountId = target ? target->GetSession()->GetAccountId() : sCharacterCache->GetCharacterAccountIdByGuid(targetGuid);

        // find only player from same account if any
        if (!target)
            if (WorldSession* session = sWorld->FindSession(accountId))
                target = session->GetPlayer();

        // must have strong lesser security level
        if (handler->HasLowerSecurity(target, targetGuid, true))
            return false;

        if (target)
        {
            if (target->GetSession()->CanSpeak())
            {
                handler->SendSysMessage(LANG_CHAT_ALREADY_ENABLED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            target->GetSession()->m_muteTime = 0;
        }

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
        stmt->setInt64(0, 0);
        stmt->setString(1, "");
        stmt->setString(2, "");
        stmt->setUInt32(3, accountId);
        LoginDatabase.Execute(stmt);

        if (target)
            ChatHandler(target->GetSession()).PSendSysMessage(LANG_YOUR_CHAT_ENABLED);

        std::string nameLink = handler->playerLink(targetName);

        handler->PSendSysMessage(LANG_YOU_ENABLE_CHAT, nameLink.c_str());

        return true;
    }

    // mutehistory command
    static bool HandleMuteInfoCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char *nameStr = strtok((char*)args, "");
        if (!nameStr)
            return false;

        std::string accountName = nameStr;
        if (!Utf8ToUpperOnlyLatin(accountName))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountId = AccountMgr::GetId(accountName);
        if (!accountId)
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            return false;
        }

        return HandleMuteInfoHelper(accountId, accountName.c_str(), handler);
    }

    // helper for mutehistory
    static bool HandleMuteInfoHelper(uint32 accountId, char const* accountName, ChatHandler *handler)
    {
        LoginDatabasePreparedStatement *stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_MUTE_INFO);
        stmt->setUInt32(0, accountId);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(LANG_COMMAND_MUTEHISTORY_EMPTY, accountName);
            return true;
        }

        handler->PSendSysMessage(LANG_COMMAND_MUTEHISTORY, accountName);
        do
        {
            Field* fields = result->Fetch();

            // we have to manually set the string for mutedate
            time_t sqlTime = fields[0].GetUInt32();
            tm timeinfo;
            char buffer[80];

            // set it to string
            localtime_r(&sqlTime, &timeinfo);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %I:%M%p", &timeinfo);

            handler->PSendSysMessage(LANG_COMMAND_MUTEHISTORY_OUTPUT, buffer, fields[1].GetUInt32(), fields[2].GetCString(), fields[3].GetCString());
        } while (result->NextRow());
        return true;
    }

    static bool HandleMovegensCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_MOVEGENS_LIST, (unit->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), unit->GetGUID().ToString().c_str());

        MotionMaster* motionMaster = unit->GetMotionMaster();
        float x, y, z;
        motionMaster->GetDestination(x, y, z);

        for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
        {
            MovementGenerator* movementGenerator = motionMaster->GetMotionSlot(i);
            if (!movementGenerator)
            {
                handler->SendSysMessage("Empty");
                continue;
            }

            switch (movementGenerator->GetMovementGeneratorType())
            {
                case IDLE_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_IDLE);
                    break;
                case RANDOM_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_RANDOM);
                    break;
                case WAYPOINT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_WAYPOINT);
                    break;
                case CONFUSED_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_CONFUSED);
                    break;
                case CHASE_MOTION_TYPE:
                {
                    Unit* target = nullptr;
                    if (unit->GetTypeId() == TYPEID_PLAYER)
                        target = static_cast<ChaseMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                    else
                        target = static_cast<ChaseMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                    if (!target)
                        handler->SendSysMessage(LANG_MOVEGENS_CHASE_NULL);
                    else if (target->GetTypeId() == TYPEID_PLAYER)
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, target->GetName().c_str(), target->GetGUID().ToString().c_str());
                    else
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, target->GetName().c_str(), target->GetGUID().ToString().c_str());
                    break;
                }
                case FOLLOW_MOTION_TYPE:
                {
                    Unit* target = nullptr;
                    if (unit->GetTypeId() == TYPEID_PLAYER)
                        target = static_cast<FollowMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                    else
                        target = static_cast<FollowMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                if (!target)
                    handler->SendSysMessage(LANG_MOVEGENS_FOLLOW_NULL);
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, target->GetName().c_str(), target->GetGUID().ToString().c_str());
                else
                    handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, target->GetName().c_str(), target->GetGUID().ToString().c_str());
                break;
            }
            case HOME_MOTION_TYPE:
            {
                if (unit->GetTypeId() == TYPEID_UNIT)
                    handler->PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE, x, y, z);
                else
                    handler->SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                break;
            }
            case FLIGHT_MOTION_TYPE:
                handler->SendSysMessage(LANG_MOVEGENS_FLIGHT);
                break;
            case POINT_MOTION_TYPE:
            {
                handler->PSendSysMessage(LANG_MOVEGENS_POINT, x, y, z);
                break;
            }
            case FLEEING_MOTION_TYPE:
                handler->SendSysMessage(LANG_MOVEGENS_FEAR);
                break;
            case DISTRACT_MOTION_TYPE:
                handler->SendSysMessage(LANG_MOVEGENS_DISTRACT);
                break;
            case EFFECT_MOTION_TYPE:
                handler->SendSysMessage(LANG_MOVEGENS_EFFECT);
                break;
            default:
                handler->PSendSysMessage(LANG_MOVEGENS_UNKNOWN, movementGenerator->GetMovementGeneratorType());
                break;
            }
        }
        return true;
    }

    static bool HandleComeToMeCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* caster = handler->getSelectedCreature();
        if (!caster)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        caster->GetMotionMaster()->MovePoint(0, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

        return true;
    }

    static bool HandleDamageCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* str = strtok((char*)args, " ");

        if (strcmp(str, "go") == 0)
        {
            char* guidStr = strtok(nullptr, " ");
            if (!guidStr)
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            ObjectGuid::LowType guidLow = atoull(guidStr);
            if (!guidLow)
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            char* damageStr = strtok(nullptr, " ");
            if (!damageStr)
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            int32 damage = atoi(damageStr);
            if (!damage)
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (Player* player = handler->GetSession()->GetPlayer())
            {
                GameObject* go = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
                if (!go)
                {
                    handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, std::to_string(guidLow).c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (!go->IsDestructibleBuilding())
                {
                    handler->SendSysMessage(LANG_INVALID_GAMEOBJECT_TYPE);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                go->ModifyHealth(-damage, player);
                handler->PSendSysMessage(LANG_GAMEOBJECT_DAMAGED, go->GetName().c_str(), std::to_string(guidLow).c_str(), -damage, go->GetGOValue()->Building.Health);
            }

            return true;
        }

        Unit* target = handler->getSelectedUnit();
        if (!target || !handler->GetSession()->GetPlayer()->GetTarget())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Player* player = target->ToPlayer())
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty, false))
                return false;

        if (!target->IsAlive())
            return true;

        char* damageStr = strtok((char*)args, " ");
        if (!damageStr)
            return false;

        int32 damage_int = atoi((char*)damageStr);
        if (damage_int <= 0)
            return true;

        uint32 damage = damage_int;

        char* schoolStr = strtok((char*)nullptr, " ");

        // flat melee damage without resistence/etc reduction
        if (!schoolStr)
        {
            handler->GetSession()->GetPlayer()->DealDamage(target, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            if (target != handler->GetSession()->GetPlayer())
                handler->GetSession()->GetPlayer()->SendAttackStateUpdate(HITINFO_AFFECTS_VICTIM, target, 1, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_HIT, 0);
            return true;
        }

        uint32 school = atoi((char*)schoolStr);
        if (school >= MAX_SPELL_SCHOOL)
            return false;

        SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

        if (handler->GetSession()->GetPlayer()->IsDamageReducedByArmor(schoolmask))
            damage = handler->GetSession()->GetPlayer()->CalcArmorReducedDamage(handler->GetSession()->GetPlayer(), target, damage, nullptr, BASE_ATTACK);

        char* spellStr = strtok((char*)nullptr, " ");

        Player* attacker = handler->GetSession()->GetPlayer();

        // melee damage by specific school
        if (!spellStr)
        {
            DamageInfo dmgInfo(attacker, target, damage, nullptr, schoolmask, SPELL_DIRECT_DAMAGE, BASE_ATTACK);
            attacker->CalcAbsorbResist(dmgInfo);

            if (!dmgInfo.GetDamage())
                return true;

            damage = dmgInfo.GetDamage();

            uint32 absorb = dmgInfo.GetAbsorb();
            uint32 resist = dmgInfo.GetResist();
            attacker->DealDamageMods(target, damage, &absorb);
            attacker->DealDamage(target, damage, nullptr, DIRECT_DAMAGE, schoolmask, nullptr, false);
            attacker->SendAttackStateUpdate(HITINFO_AFFECTS_VICTIM, target, 0, schoolmask, damage, absorb, resist, VICTIMSTATE_HIT, 0);
            return true;
        }

        // non-melee damage

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellid = handler->extractSpellIdFromLink((char*)args);
        if (!spellid)
            return false;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid, attacker->GetMap()->GetDifficultyID());
        if (!spellInfo)
            return false;

        SpellNonMeleeDamage damageInfo(attacker, target, spellInfo, { spellInfo->GetSpellXSpellVisualId(handler->GetSession()->GetPlayer()),0 }, spellInfo->SchoolMask);
        damageInfo.damage = damage;
        attacker->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);
        target->DealSpellDamage(&damageInfo, true);
        target->SendSpellNonMeleeDamageLog(&damageInfo);
        return true;
    }

    static bool HandleCombatStopCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;

        if (args && args[0] != '\0')
        {
            target = ObjectAccessor::FindPlayerByName(args);
            if (!target)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!target)
        {
            if (!handler->extractPlayerTarget((char*)args, &target))
                return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        target->CombatStop();
        target->getHostileRefManager().deleteReferences();
        return true;
    }

    static bool HandleRepairitemsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        // Repair items
        target->DurabilityRepairAll(false, 0, false);

        handler->PSendSysMessage(LANG_YOU_REPAIR_ITEMS, handler->GetNameLink(target).c_str());
        if (handler->needReportToTarget(target))
            ChatHandler(target->GetSession()).PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink().c_str());

        return true;
    }

    static bool HandleFreezeCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer(); // Selected player, if any. Might be null.
        uint32 freezeDuration = 0; // Freeze Duration (in seconds)
        bool canApplyFreeze = false; // Determines if every possible argument is set so Freeze can be applied
        bool getDurationFromConfig = false; // If there's no given duration, we'll retrieve the world cfg value later

        /*
            Possible Freeze Command Scenarios:
            case 1 - .freeze (without args and a selected player)
            case 2 - .freeze duration (with a selected player)
            case 3 - .freeze player duration
            case 4 - .freeze player (without specifying duration)
        */

        // case 1: .freeze
        if (!*args)
        {
            // Might have a selected player. We'll check it later
            // Get the duration from world cfg
            getDurationFromConfig = true;
        }
        else
        {
            // Get the args that we might have (up to 2)
            char const* arg1 = strtok((char*)args, " ");
            char const* arg2 = strtok(nullptr, " ");

            // Analyze them to see if we got either a playerName or duration or both
            if (arg1)
            {
                if (isNumeric(arg1))
                {
                    // case 2: .freeze duration
                    // We have a selected player. We'll check him later
                    freezeDuration = uint32(atoi(arg1));
                    canApplyFreeze = true;
                }
                else
                {
                    // case 3 or 4: .freeze player duration | .freeze player
                    // find the player
                    std::string name = arg1;
                    normalizePlayerName(name);
                    player = ObjectAccessor::FindPlayerByName(name);
                    // Check if we have duration set
                    if (arg2 && isNumeric(arg2))
                    {
                        freezeDuration = uint32(atoi(arg2));
                        canApplyFreeze = true;
                    }
                    else
                        getDurationFromConfig = true;
                }
            }
        }

        // Check if duration needs to be retrieved from config
        if (getDurationFromConfig)
        {
            freezeDuration = sWorld->getIntConfig(CONFIG_GM_FREEZE_DURATION);
            canApplyFreeze = true;
        }

        // Player and duration retrieval is over
        if (canApplyFreeze)
        {
            if (!player) // can be null if some previous selection failed
            {
                handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
            else if (player == handler->GetSession()->GetPlayer())
            {
                // Can't freeze himself
                handler->SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
                return true;
            }
            else // Apply the effect
            {
                // Add the freeze aura and set the proper duration
                // Player combat status and flags are now handled
                // in Freeze Spell AuraScript (OnApply)
                Aura* freeze = player->AddAura(9454, player);
                if (freeze)
                {
                    if (freezeDuration)
                        freeze->SetDuration(freezeDuration * IN_MILLISECONDS);
                    handler->PSendSysMessage(LANG_COMMAND_FREEZE, player->GetName().c_str());
                    // save player
                    player->SaveToDB();
                    return true;
                }
            }
        }
        return false;
    }

    static bool HandleUnFreezeCommand(ChatHandler* handler, char const*args)
    {
        std::string name;
        Player* player;
        char* targetName = strtok((char*)args, " "); // Get entered name

        if (targetName)
        {
            name = targetName;
            normalizePlayerName(name);
            player = ObjectAccessor::FindPlayerByName(name);
        }
        else // If no name was entered - use target
        {
            player = handler->getSelectedPlayer();
            if (player)
                name = player->GetName();
        }

        if (player)
        {
            handler->PSendSysMessage(LANG_COMMAND_UNFREEZE, name.c_str());

            // Remove Freeze spell (allowing movement and spells)
            // Player Flags + Neutral faction removal is now
            // handled on the Freeze Spell AuraScript (OnRemove)
            player->RemoveAurasDueToSpell(9454);
        }
        else
        {
            if (targetName)
            {
                // Check for offline players
                ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(name);
                if (guid.IsEmpty())
                {
                    handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                    return true;
                }

                // If player found: delete his freeze aura
                CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_AURA_FROZEN);
                stmt->setUInt64(0, guid.GetCounter());
                CharacterDatabase.Execute(stmt);

                handler->PSendSysMessage(LANG_COMMAND_UNFREEZE, name.c_str());
                return true;
            }
            else
            {
                handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
        }

        return true;
    }

    static bool HandleListFreezeCommand(ChatHandler* handler, char const* /*args*/)
    {
        // Get names from DB
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_AURA_FROZEN);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
            return true;
        }

        // Header of the names
        handler->PSendSysMessage(LANG_COMMAND_LIST_FREEZE);

        // Output of the results
        do
        {
            Field* fields = result->Fetch();
            std::string player = fields[0].GetString();
            int32 remaintime = fields[1].GetInt32();
            // Save the frozen player to update remaining time in case of future .listfreeze uses
            // before the frozen state expires
            if (Player* frozen = ObjectAccessor::FindPlayerByName(player))
                frozen->SaveToDB();
            // Notify the freeze duration
            if (remaintime == -1) // Permanent duration
                handler->PSendSysMessage(LANG_COMMAND_PERMA_FROZEN_PLAYER, player.c_str());
            else
                // show time left (seconds)
                handler->PSendSysMessage(LANG_COMMAND_TEMP_FROZEN_PLAYER, player.c_str(), remaintime / IN_MILLISECONDS);
        } while (result->NextRow());

        return true;
    }

    static bool HandlePlayAllCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* soundIdToken = strtok((char*)args, " ");

        uint32 soundId = atoul(soundIdToken);

        if (!sSoundKitStore.LookupEntry(soundId))
        {
            handler->PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 broadcastTextId = 0;
        char const* broadcastTextIdToken = strtok(nullptr, " ");
        if (broadcastTextIdToken)
            broadcastTextId = atoul(broadcastTextIdToken);

        sWorld->SendGlobalMessage(WorldPackets::Misc::PlaySound(handler->GetSession()->GetPlayer()->GetGUID(), soundId, broadcastTextId).Write());

        handler->PSendSysMessage(LANG_COMMAND_PLAYED_TO_ALL, soundId);
        return true;
    }

    static bool HandlePossessCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;

        //handler->GetSession()->GetPlayer()->CastSpell(unit, 530, true); // Charme (Possession) marche pas sur lvl 100+
        handler->GetSession()->GetPlayer()->CastSpell(unit, 206124, true); // Possession de Jeremy
        return true;
    }

    static bool HandleUnPossessCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            unit = handler->GetSession()->GetPlayer();

        unit->RemoveCharmAuras();

        return true;
    }

    static bool HandleBindSightCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;

        handler->GetSession()->GetPlayer()->CastSpell(unit, 6277, true);
        return true;
    }

    static bool HandleUnbindSightCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (player->isPossessing())
            return false;

        player->StopCastingBindSight();
        return true;
    }

    static bool HandleMailBoxCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        handler->GetSession()->SendShowMailBox(player->GetGUID());
        return true;
    }

    static bool HandleAurasCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->GetSession()->GetPlayer()->GetSelectedUnit();

        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Unit::AuraApplicationMap const& uAuras = target->GetAppliedAuras();
        handler->PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, std::to_string(uAuras.size()).c_str());
        for (Unit::AuraApplicationMap::const_iterator itr = uAuras.begin(); itr != uAuras.end(); ++itr)
        {
            AuraApplication const* aurApp = itr->second;
            Aura const* aura = aurApp->GetBase();
            char const* name = aura->GetSpellInfo()->SpellName->Str[handler->GetSessionDbcLocale()];

            bool self = target->GetGUID() == aura->GetCasterGUID();
            if (self)
                handler->PSendSysMessage("%u: %s (self)", aura->GetId(), name);
            else
            {
                if (Unit* u = aura->GetCaster())
                {
                    if (u->GetTypeId() == TYPEID_PLAYER)
                        handler->PSendSysMessage("%u: %s (player: %s)", aura->GetId(), name, u->GetName().c_str());
                    else if (u->GetTypeId() == TYPEID_UNIT)
                        handler->PSendSysMessage("%u: %s (creature: %s)", aura->GetId(), name, u->GetName().c_str());
                }
                else
                    handler->PSendSysMessage("%u: %s)", aura->GetId(), name);
            }
        }
        return true;
    }

    /*
    static bool HandleDebugDatabase(ChatHandler* handler, char const* args) // party
    {
        if (!*args)
            return false;

        char* temp = (char*)args;
        char* min = strtok(temp, "-");
        char* max = strtok(NULL, ";");
        char* rollBaby = strtok(NULL, "\0");
        if (!min || !max || !rollBaby)
            return false;


        int intminimum = atoi(min);
        int intmaximum = atoi(max);
        int introll = atoi(rollBaby);
        uint32 minimum = (uint32)intminimum;
        uint32 maximum = (uint32)intmaximum;
        uint32 roll = (uint32)introll;

        Player* p = handler->GetSession()->GetPlayer();

        if (minimum > maximum || maximum > 10000)
            return false;

        if (roll > maximum || roll < minimum)
            return false;

        WorldPackets::Misc::RandomRoll randomRoll;
        randomRoll.Min = minimum;
        randomRoll.Max = maximum;
        randomRoll.Result = roll;
        randomRoll.Roller = p->GetGUID();
        randomRoll.RollerWowAccount = handler->GetSession()->GetAccountGUID();
        if (p->GetGroup())
            p->GetGroup()->BroadcastPacket(randomRoll.Write(), false);
        else
            handler->GetSession()->SendPacket(randomRoll.Write());

        return true;

    }
    */

    static bool HandleModelCommand(ChatHandler* handler, char const* args)
    {

        Unit* p = handler->GetSession()->GetPlayer();
        Unit* target = handler->getSelectedUnit();

        uint32 spellId = 102284;
        uint32 spellId2 = 111232;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, p->GetMapId(), spellId, p->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, p, target, target->GetMap()->GetDifficultyID());
        }

        if (SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(spellId2, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, p->GetMapId(), spellId2, p->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo2, castId, MAX_EFFECT_MASK, p, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    static bool HandleArrayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Unit* target = handler->getSelectedUnit();

        if (!target)
            return false;

        // creature->TextEmote(args);
        target->Talk(args, CHAT_MSG_SAY, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), nullptr);

        return true;
    }

    static bool HandleCustomCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .custom", handler->GetSession()->GetPlayer()->GetName().c_str());
        uint32 objectId = 999999;
        Player* player = handler->GetSession()->GetPlayer();

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        float x = float(player->GetPositionX());
        float y = float(player->GetPositionY());
        float z = float(player->GetPositionZ());
        float ang = player->GetOrientation();
        //float rot2 = std::sin(ang / 2);
        //float rot3 = std::cos(ang / 2);
        Map* map = player->GetMap();

        uint32 spawntm = 400;

        // Maintenant la commande a besoin d'une variable quaternion au lieu de 4 valeurs.
        G3D::Quat rotation = G3D::Matrix3::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);

        GameObject* object = player->SummonGameObject(objectId, x, y, z, ang, QuaternionData(rotation.x, rotation.y, rotation.z, rotation.w), spawntm);

        player->SummonGameObject(objectId, x, y, z, ang, QuaternionData(rotation.x, rotation.y, rotation.z, rotation.w), spawntm);

        object->DestroyForNearbyPlayers();
        object->UpdateObjectVisibility();

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(110851, handler->GetSession()->GetPlayer()->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, player->GetMapId(), 110851, player->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, player, player, handler->GetSession()->GetPlayer()->GetMap()->GetDifficultyID());
        }

        object->Use(handler->GetSession()->GetPlayer());

        GameObject* go = player->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_BARBER_CHAIR, 0.01f);

        player->SetStandState(UnitStandStateType(UNIT_STAND_STATE_SIT_LOW_CHAIR + go->GetGOInfo()->barberChair.chairheight));

        return true;
    }

    static bool HandleMountCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        Creature* creature = handler->getSelectedCreature();
        Player* player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        if (!target)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .mount", handler->GetSession()->GetPlayer()->GetName().c_str());

        WorldPacket data;
        if (strncmp(args, "self", 3) == 0)
        {

            if (!target || target != creature)
                return false;

            uint32 baseEntry;
            baseEntry = creature->GetDisplayId();
            player->Mount(baseEntry);
            player->SetSpeed(MOVE_WALK, 2);
            player->SetSpeed(MOVE_RUN, 2);
            player->SetSpeed(MOVE_SWIM, 2);
            player->SetSpeed(MOVE_FLIGHT, 2);
            return true;
        }

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        // CreatureModelInfo const* modelInfo = sObjectMgr->GetCreatureModelInfo(newEntry);

        target->Mount(newEntry);

        target->SetSpeed(MOVE_WALK, 2);
        target->SetSpeed(MOVE_RUN, 2);
        target->SetSpeed(MOVE_SWIM, 2);
        //target->SetSpeed(MOVE_TURN,    ASpeed, true);
        target->SetSpeed(MOVE_FLIGHT, 2);

        return true;
    }

    static bool HandleSpellViskitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendPlaySpellVisualKit(newEntry, 2, 0);
        }
        else {
            handler->GetSession()->GetPlayer()->SendPlaySpellVisualKit(newEntry, 2, 0);
        }


        return true;
    }

    static bool HandleUnSpellViskitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        // Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .unspellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {

            target->SendCancelSpellVisualKit(newEntry);
        }
        else {

            handler->GetSession()->GetPlayer()->SendCancelSpellVisualKit(newEntry);
        }

        return true;
    }

    static bool HandleAnimKitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .animkit", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {

            target->SetAIAnimKitId(newEntry);

        }
        else {

            handler->GetSession()->GetPlayer()->SetAIAnimKitId(newEntry);

        }

        return true;
    }

    static bool HandleDistanceCommand(ChatHandler* handler, char const* args)
    {
        //Phase 1 : recherche du type de la cible : gob, npc ou player ?
        WorldObject* target;
        std::string targetName;
        if (!*args)
        {
            //Partie ciblage classique, si il y a pas d'args
            target = handler->getSelectedObject();
            if (!target)
            {
                handler->SendSysMessage("Vous n avez pas de cible, ni de guid, ni de nom de joueur");
                return false;
            }
            targetName = target->GetName();
        }
        else
        {
            //Partie "joueur"
            std::string nameTargetPlayer = (char*)args;
            target = ObjectAccessor::FindPlayerByName(nameTargetPlayer);
            if (!target)
            {
                //Partie "gob"
                char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
                if (!id)
                    return false;

                ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
                if (!guidLow)
                    return false;

                GameObject* object = NULL;
                if (GameObjectData const* gameObjectData = sObjectMgr->GetGameObjectData(guidLow))
                    object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);

                if (!object)
                {
                    handler->PSendSysMessage("Le guid du gob ou le nom du joueur est invalide");
                    return false;
                }
                targetName = object->GetGOInfo()->name.c_str();
                target = object;
            }
            else
                targetName = target->GetName();
        }
        //Phase 3 : Calcul des positions et distance en m�tres (� deux decimal pres, arrondi � l'inf�rieur )
        double playerX = (trunc((handler->GetSession()->GetPlayer()->GetPositionX()) * 10 * 0.9144)) / 10;
        double playerY = (trunc((handler->GetSession()->GetPlayer()->GetPositionY()) * 10 * 0.9144)) / 10;
        double playerZ = (trunc((handler->GetSession()->GetPlayer()->GetPositionZ()) * 10 * 0.9144)) / 10;
        double targetX = (trunc((target->GetPositionX()) * 10 * 0.9144)) / 10;
        double targetY = (trunc((target->GetPositionY()) * 10 * 0.9144)) / 10;
        double targetZ = (trunc((target->GetPositionZ()) * 10 * 0.9144)) / 10;

        double distanceFull = sqrt((pow((playerX - targetX), 2)) + (pow((playerY - targetY), 2)) + (pow((playerZ - targetZ), 2)));
        double distance = (trunc(distanceFull * 10)) / 10;

        //Phase 4 : Envoi du message
        if (distance > 300)
            handler->SendSysMessage("Vous etes trop loin de votre cible, les mechants devs veulent pas vous dire la distance");
        else if (distance < 1)
            handler->PSendSysMessage("%s est a %2.0f cm de vous", targetName, distance);
        else
            handler->PSendSysMessage("%s est a %3.2f m de vous", targetName, distance);
        return true;
    }

    static bool HandleMoveCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        char* temp = (char*)args;
        char* str1 = strtok(temp, " ");
        char* str2 = strtok(NULL, "\0");
        Player* player = handler->GetSession()->GetPlayer();
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float rot = player->GetOrientation();
        player->SetCanFly(true);
        if (strcmp(str1, "x") == 0)
        {
            if (str2 != NULL)
            {
                x = player->GetPositionX() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else if (strcmp(str1, "y") == 0)
        {
            if (str2 != NULL)
            {
                y = player->GetPositionY() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else if (strcmp(str1, "z") == 0)
        {
            if (str2 != NULL)
            {
                z = player->GetPositionZ() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else
        {
            float nb = atof(str1);
            x = player->GetPositionX() + (nb*cos((double)rot));
            y = player->GetPositionY() + (nb*sin((double)rot));
        }
        //player->GetMotionMaster()->MovePoint(0, x, y, z);
        //Position pos = Position(x, y, z, rot);
        //player->MovePosition(pos, 50, 0);
        WorldLocation position = WorldLocation(player->GetMapId(), x, y, z, rot);
        player->TeleportTo(position);
        handler->PSendSysMessage("Position : x = %5.3f ; y = %5.3f ; z = %5.3f", x, y, z);
        return true;
    }


    static bool HandleRandomSayCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s a fait un jet de %u (%u-%u) [Rand en /dire]", playerName.c_str(), roll, min, max);
        player->Talk(msg, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
        return true;
    }

    static bool HandleRandomMPCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s a fait un jet de %u (%u-%u) [Rand en priv�", playerName.c_str(), roll, min, max);
        Unit* target = player->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage("Vous n'avez cible personne, ce rand ne sera visible que par vous");
        }
        else if (target->GetTypeId() == TYPEID_PLAYER)
        {
            Player* targetPlayer = player->GetSelectedPlayer();
            ChatHandler(targetPlayer->GetSession()).PSendSysMessage(msg);
        }
        else
        {
            handler->SendSysMessage("Vous n'avez cible personne, ce rand ne sera visible que par vous");
        }
        handler->PSendSysMessage(msg);
        return true;
    }

    static bool HandlePandarenCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!*args)
            return false;

        if (player->getRace() != RACE_PANDAREN_NEUTRAL)
            return true;

        std::string argstr = (char*)args;

        if (argstr == "alliance")
        {
            player->SetRace(RACE_PANDAREN_ALLIANCE);
            player->setFactionForRace(RACE_PANDAREN_ALLIANCE);
            player->SaveToDB();
            player->LearnSpell(108130, false); // Language Pandaren Alliance
            handler->PSendSysMessage("Vous �tes d�sormais un Pandaren de l'alliance !");
        }
        else if (argstr == "horde")
        {
            player->SetRace(RACE_PANDAREN_HORDE);
            player->setFactionForRace(RACE_PANDAREN_HORDE);
            player->SaveToDB();
            player->LearnSpell(108131, false); // Language Pandaren Horde
            handler->PSendSysMessage("Vous �tes d�sormais un Pandaren de la horde !");
        }
        else
        {
            handler->PSendSysMessage("Param�tre incorrect, veuillez entrez horde ou alliance");
        }

        return true;
    }
    static bool HandleMortCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 85267;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }


    // Le .lutte

    static bool HandleLutteCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 94610;
        uint32 spellId2 = 245843;
        uint32 spellId3 = 245853;
        uint32 spellId4 = 245849;
        uint32 spellId5 = 245848;

        switch (target->getClass()) {
        case 8:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId2, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId2, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 5:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId3, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId3, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 7:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId4, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId4, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 11:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId5, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId5, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        default:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        }

        return true;
    }

    //Le .traquer

    static bool HandleTraquerCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 80264;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .lire ( PS : H devrait essayer de retirer le timer sur le le spell )


    static bool HandleLireCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 147164;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }
    // Le .attacher

    static bool HandleAttacherCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 93090;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .exces

    static bool HandleExcesCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 80109;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .livre

    static bool HandleLivreCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 124064;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .ombrelle

    static bool HandleOmbrelleCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 131076;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }


    //Le .carquois

    static bool HandleCarquoisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        int choix = atoi(args);
        uint32 spellId;
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        if (!args)
            spellId = 80642;
        switch (choix)
        {
        case 1: spellId = 80642; break;
        case 2: spellId = 125165; break;
        default: handler->SetSentErrorMessage(true); return false;
        }

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    //Le .sac

    static bool HandleSacCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        int choix = atoi(args);
        uint32 spellId;
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        if (!args)
            spellId = 105008;

        switch (choix)
        {
        case 1: spellId = 105008; break;
        case 2: spellId = 104953; break;
        case 3: spellId = 85500; break;
        case 4: spellId = 89695; break;
        case 5: spellId = 88330; break;
        case 6: spellId = 78012; break;
        case 7: spellId = 90085; break;
        case 8: spellId = 84516; break;
        case 9: spellId = 122159; break;
        case 10: spellId = 131732; break;
        case 11: spellId = 79252; break;
        case 12: spellId = 106356; break;
        case 13: spellId = 168026; break;
        case 14: spellId = 163398; break;
        default: handler->SetSentErrorMessage(true); return false;
        }

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    //Le .bondir

    static bool HandleBondirCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;
        uint32 spellId = 55518;
        handler->GetSession()->GetPlayer()->CastSpell(unit, spellId, true);
        return true;
    }

    //Le .vomir

    static bool HandleVomirCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;
        uint32 spellId = 85234;
        handler->GetSession()->GetPlayer()->CastSpell(unit, spellId, true);
        return true;
    }

    // Le .sang
    static bool HandleSangCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 169471;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .invisible
    static bool HandleInvisibleCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();
        else if (target->GetTypeId() == TYPEID_PLAYER && handler->HasLowerSecurity(target->ToPlayer(), ObjectGuid::Empty))
            return false;

        target->SetDisplayId(31515);

        return true;
    }

    static bool HandleNuitCommand(ChatHandler* handler, char const* args)
    {

        std::string argstr = (char*)args;
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (argstr == "off")
        {
            target->RemoveAura(185394);
            handler->SendSysMessage("Nuit noire d�sactiv�e !");
            return true;
        }
        else if (argstr == "on")
        {
            uint32 spellId = 185394;
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }
            handler->SendSysMessage("Nuit noire activ�e ! Tapez .nuit off pour la d�sactiv�e.");
            return true;
        }

        return true;
    }

    static bool HandleForgeInfoCommand(ChatHandler* handler, char const* args)
    {
        /*
        Unit* target = handler->getSelectedUnit();

        // Get Textures
        if (!handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_NO_PLAYERS_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 guid = 0;
        uint32 displayId = 0;
        uint32 entryExtra = 0;
        uint32 entryId = 0;
        uint8 skin = 0;
        uint8 face = 0;
        uint8 hair = 0;
        uint8 hcol = 0;
        uint8 pilo = 0;
        uint8 cust1 = 0;
        uint8 cust2 = 0;
        uint8 cust3 = 0;
        uint32 arme1 = 0;
        uint32 arme2 = 0;
        uint32 arme3 = 0;
        uint64 head = 0;
        uint64 shoulders = 0;
        uint64 body = 0;
        uint64 chest = 0;
        uint64 waist = 0;
        uint64 legs = 0;
        uint64 feet = 0;
        uint64 wrists = 0;
        uint64 hands = 0;
        uint64 back = 0;
        uint64 tabard = 0;

        if (target) {
            if (target->IsPlayer()) {
                guid = target->GetGUID().GetCounter();
            }
            else if (target->IsCreature()) {
                displayId = target->GetDisplayId();
                guid = target->GetGUID().GetCounter();
            }
        }
        else {
            guid = handler->GetSession()->GetPlayer()->GetGUID().GetCounter();
        }

        if (!target || target && target->IsPlayer()) {

            //Query
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARSINFO);
            stmt->setUInt64(0, guid);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
                return false;

            Field* fields = result->Fetch();

            skin = fields[0].GetUInt8();
            face = fields[1].GetUInt8();
            hair = fields[2].GetUInt8();
            hcol = fields[3].GetUInt8();
            pilo = fields[4].GetUInt8();
            cust1 = fields[5].GetUInt8();
            cust2 = fields[6].GetUInt8();
            cust3 = fields[7].GetUInt8();

            //Get Equipment		
            Player* player = handler->getSelectedPlayerOrSelf();
            static EquipmentSlots const itemSlots[] =
            {
                EQUIPMENT_SLOT_HEAD,
                EQUIPMENT_SLOT_SHOULDERS,
                EQUIPMENT_SLOT_BODY,
                EQUIPMENT_SLOT_CHEST,
                EQUIPMENT_SLOT_WAIST,
                EQUIPMENT_SLOT_LEGS,
                EQUIPMENT_SLOT_FEET,
                EQUIPMENT_SLOT_WRISTS,
                EQUIPMENT_SLOT_HANDS,
                EQUIPMENT_SLOT_BACK,
                EQUIPMENT_SLOT_TABARD,
                EQUIPMENT_SLOT_MAINHAND,
                EQUIPMENT_SLOT_OFFHAND,
            };

            std::vector<uint32> eqqList = std::vector<uint32>();

            // Stolen code from SpellHandler
            for (EquipmentSlots slot : itemSlots)
            {
                uint32 itemDisplayId; 
                if (Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    itemDisplayId = item->GetDisplayId(player);


                    if (slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND)
                        itemDisplayId = item->GetEntry();
                }
                else
                    itemDisplayId = 0;


                eqqList.push_back(itemDisplayId);
            }

            // TEXTURES
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_TEXT);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_SKIN, skin);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_FACE, face);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HAIR, hair);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HCOL, hcol);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_PILO, pilo);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH01, cust1);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH02, cust2);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH03, cust3);

            // EQUIPMENTS
            handler->PSendSysMessage(LANG_CUST_SEPARATOR);
            handler->PSendSysMessage(LANG_CUST_HEAD, eqqList[0]);
            handler->PSendSysMessage(LANG_CUST_SHOULDERS, eqqList[1]);
            handler->PSendSysMessage(LANG_CUST_BODY, eqqList[2]);
            handler->PSendSysMessage(LANG_CUST_CHEST, eqqList[3]);
            handler->PSendSysMessage(LANG_CUST_WAIST, eqqList[4]);
            handler->PSendSysMessage(LANG_CUST_LEGS, eqqList[5]);
            handler->PSendSysMessage(LANG_CUST_FEET, eqqList[6]);
            handler->PSendSysMessage(LANG_CUST_WRISTS, eqqList[7]);
            handler->PSendSysMessage(LANG_CUST_HANDS, eqqList[8]);
            handler->PSendSysMessage(LANG_CUST_BACK, eqqList[9]);
            handler->PSendSysMessage(LANG_CUST_TABARD, eqqList[10]);
            handler->PSendSysMessage(LANG_CUST_MAINHAND, eqqList[11]);
            handler->PSendSysMessage(LANG_CUST_OFFHAND, eqqList[12]);

            // WEAPONS
        }
        else if (target->IsCreature()) {

            uint32 eqqList[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            CreatureDisplayInfoEntry const* entry = sCreatureDisplayInfoStore.LookupEntry(displayId);
            if (entry) {
                entryExtra = entry->ExtendedDisplayInfoID;
            }

            if (entryExtra != 0) {

                CreatureDisplayInfoExtraEntry const* extraEntry = sCreatureDisplayInfoExtraStore.LookupEntry(entryExtra);
                if (extraEntry) {
                    skin = extraEntry->SkinID;
                    face = extraEntry->FaceID;
                    hair = extraEntry->HairStyleID;
                    hcol = extraEntry->HairColorID;
                    pilo = extraEntry->FacialHairID;
                    cust1 = extraEntry->CustomDisplayOption[0];
                    cust2 = extraEntry->CustomDisplayOption[1];
                    cust3 = extraEntry->CustomDisplayOption[2];
                }

                for (uint32 id = 204560; id < sNPCModelItemSlotDisplayInfoStore.GetNumRows(); ++id)
                {

                    NPCModelItemSlotDisplayInfoEntry const* armorEntry = sNPCModelItemSlotDisplayInfoStore.LookupEntry(id);
                    if (armorEntry)
                    {
                        if (armorEntry->ExtendedDisplayID == entryExtra)
                        {
                            if (armorEntry->Slot == 0)
                                eqqList[0] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 1)
                                eqqList[1] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 2)
                                eqqList[2] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 3)
                                eqqList[3] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 4)
                                eqqList[4] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 5)
                                eqqList[5] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 6)
                                eqqList[6] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 7)
                                eqqList[7] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 8)
                                eqqList[8] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 9)
                                eqqList[9] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 10)
                                eqqList[10] = armorEntry->DisplayID;
                        }
                    }
                }
            }

            entryId = target->GetEntry();

            arme1 = target->GetVirtualItemId(0);
            arme2 = target->GetVirtualItemId(1);
            arme3 = target->GetVirtualItemId(2);

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme1 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme1) {
                    if (arme1 == appArme1->ItemID) {
                        arme1 = appArme1->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme1 = sItemAppearanceStore.LookupEntry(arme1);
            if (displayArme1) {
                arme1 = displayArme1->ItemDisplayInfoID;
            }

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme2 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme2) {
                    if (arme2 == appArme2->ItemID) {
                        arme2 = appArme2->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme2 = sItemAppearanceStore.LookupEntry(arme2);
            if (displayArme2) {
                arme2 = displayArme2->ItemDisplayInfoID;
            }

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme3 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme3) {
                    if (arme3 == appArme3->ItemID) {
                        arme3 = appArme3->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme3 = sItemAppearanceStore.LookupEntry(arme3);
            if (displayArme3) {
                arme3 = displayArme3->ItemDisplayInfoID;
            }

            // TEXTURES
            handler->PSendSysMessage(LANG_CUST_NPCINFOS_TEXT);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_SKIN, skin);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_FACE, face);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HAIR, hair);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HCOL, hcol);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_PILO, pilo);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH01, cust1);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH02, cust2);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH03, cust3);

            // EQUIPMENTS
            handler->PSendSysMessage(LANG_CUST_SEPARATOR);
            handler->PSendSysMessage(LANG_CUST_HEAD, eqqList[0]);
            handler->PSendSysMessage(LANG_CUST_SHOULDERS, eqqList[1]);
            handler->PSendSysMessage(LANG_CUST_BODY, eqqList[2]);
            handler->PSendSysMessage(LANG_CUST_CHEST, eqqList[3]);
            handler->PSendSysMessage(LANG_CUST_WAIST, eqqList[4]);
            handler->PSendSysMessage(LANG_CUST_LEGS, eqqList[5]);
            handler->PSendSysMessage(LANG_CUST_FEET, eqqList[6]);
            handler->PSendSysMessage(LANG_CUST_WRISTS, eqqList[7]);
            handler->PSendSysMessage(LANG_CUST_HANDS, eqqList[8]);
            handler->PSendSysMessage(LANG_CUST_BACK, eqqList[10]);
            handler->PSendSysMessage(LANG_CUST_TABARD, eqqList[9]);
            handler->PSendSysMessage(LANG_CUST_PRIMARYHAND, arme1);
            handler->PSendSysMessage(LANG_CUST_SECONDARYHAND, arme2);
            handler->PSendSysMessage(LANG_CUST_TERTIARYHAND, arme3);

        }
        */

        return true;
    }

    static bool HandleDebugSyncCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s obtient un %u (%u-%u)", playerName.c_str(), roll, min, max);
        (msg, CHAT_MSG_PARTY, CHAT_MSG_RAID, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
        return true;
    }

    static bool HandleDebugDatabase(ChatHandler* handler, char const* args) // party
    {
        if (!*args)
            return false;

        char* temp = (char*)args;
        char* min = strtok(temp, "-");
        char* max = strtok(NULL, ";");
        char* rollBaby = strtok(NULL, "\0");
        if (!min || !max || !rollBaby)
            return false;


        int intminimum = atoi(min);
        int intmaximum = atoi(max);
        int introll = atoi(rollBaby);
        uint32 minimum = (uint32)intminimum;
        uint32 maximum = (uint32)intmaximum;
        uint32 roll = (uint32)introll;

        Player* p = handler->GetSession()->GetPlayer();

        if (minimum > maximum || maximum > 10000)
            return false;

        if (roll > maximum || roll < minimum)
            return false;

        /*
        char* rollChar;
        sprintf(rollChar, "%s obtient un %u (%u-%u).", p->GetName().c_str(), roll, minimum, maximum);
        std::string rollStr(rollChar);
        if (p->GetGroup())
        {
            WorldPackets::Chat::Chat packet;
            packet.Initialize(ChatMsg(CHAT_MSG_SYSTEM), Language(LANG_UNIVERSAL), p, nullptr, rollStr);
            p->GetGroup()->BroadcastPacket(packet.Write(), false);
        }
        else
        {
            handler->SendSysMessage("%s", rollChar);
        }
        */

        /*WorldPackets::Misc::RandomRoll const randomRoll;
        randomRoll.Min = minimum;
        randomRoll.Max = maximum;
        randomRoll.Result = roll;
        randomRoll.Roller = p->GetGUID();
        randomRoll.RollerWowAccount = handler->GetSession()->GetAccountGUID();*/

        return true;

    }

    

    static bool CheckModifyResources(ChatHandler* handler, const char* args, Player* target, int32& res, int8 const multiplier = 1)
    {
        if (!*args)
            return false;

        res = atoi((char*)args) * multiplier;

        if (res < 1)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        return true;
    }

    static bool HandleHealthCommand(ChatHandler* handler, const char* args)
    {

        if (!*args)
            return false;

        char const* health = strtok((char*)args, " ");

        int32 hp = 0;
        int32 lp = 0;

        Player* target = handler->getSelectedPlayerOrSelf();

        hp = atoi(health);
        lp = target->GetHealth() + hp;
        if (lp >= target->GetMaxHealth()) {
            target->SetHealth(target->GetMaxHealth());
            return true;
        }
        else if (lp <= target->GetMaxHealth()) {
            target->SetHealth(lp);
            return true;
        }

        return true;

    }


static bool HandleDeniedCommand(ChatHandler* handler, const char* args)
{
    if (!*args)
        return false;
    

    char const* aId = strtok((char*)args, " ");
    char const* pId = strtok(NULL, " ");


    if (!aId || !pId)
        return false;


    uint32 accountId = uint32(atoi(aId));
    uint32 permission = uint32(atoi(pId));

    QueryResult permCheck = LoginDatabase.PQuery("SELECT permissionId FROM rbac_account_permissions WHERE permissionId = %u AND accountId = %u ", permission, accountId);
    if (permCheck)
    {
        handler->PSendSysMessage(LANG_DENIED_ERROR, accountId, permission);
    }

    else
    {
        LoginDatabasePreparedStatement* addPerm = LoginDatabase.GetPreparedStatement(LOGIN_INS_DENIED_PERMISSION);
        addPerm->setUInt32(0, accountId);
        addPerm->setUInt32(1, permission);
        LoginDatabase.Execute(addPerm);

        sAccountMgr->LoadRBAC();
        sWorld->ReloadRBAC();
        handler->SendGlobalGMSysMessage("RBAC data reloaded.");

        handler->PSendSysMessage(LANG_DENIED_SUCCESSFULL, accountId, permission);
    }

   
    return true;

}

static bool HandleCreateTicketCommand(ChatHandler* handler, const char* args)
{
    if (!*args)
        return false;

    char* msg = (char*)args;
    std::string playerName = handler->GetSession()->GetPlayer()->GetName();
  


    // auto increment test 
    QueryResult lastId = WorldDatabase.PQuery("SELECT MAX(ticketId) from ticket");
    Field* field = lastId->Fetch();
    uint32 tId = field[0].GetUInt32();
        ++tId;
  

    WorldDatabasePreparedStatement* sendTicket = WorldDatabase.GetPreparedStatement(WORLD_INS_NEW_TICKET);
    sendTicket->setUInt32(0, tId);
    sendTicket->setString(1, msg);
    sendTicket->setString(2, playerName.c_str());
    sendTicket->setInt64(3, handler->GetSession()->GetAccountId());
    sendTicket->setInt64(4, handler->GetSession()->GetPlayer()->GetGUID().GetCounter());
    WorldDatabase.Execute(sendTicket);

    
    sWorld->SendGMText(LANG_TICKET_SEND_GM, tId, handler->GetSession()->GetAccountId(), handler->GetSession()->GetPlayer()->GetGUID().GetCounter(), playerName.c_str());
    sWorld->SendGMText(LANG_TICKET_SEND_GM_CONTENT, tId, msg);

    return true;

}

static bool HandleTicketListCommand(ChatHandler* handler, const char* args)
{
    QueryResult listTicket = WorldDatabase.PQuery("SELECT ticketId, ticketContents, ticketOwner from ticket WHERE ticketStatus = 0");
    Field* field = listTicket->Fetch();
    uint32 ticketId = field[0].GetUInt32();
    std::string ticketMsg = field[1].GetString();
    std::string ticketOwner = field[2].GetString();
    
    sWorld->SendGMText(LANG_TICKET_LIST, ticketId, ticketMsg, ticketOwner);

    return true;
}


    static bool HandleSpellVisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendPlaySpellVisual(target->GetPosition(), target->GetOrientation(), newEntry, 0, 0, 5000, false);
        }
        else {
            handler->GetSession()->GetPlayer()->SendPlaySpellVisual(target->GetPosition(), target->GetOrientation(), newEntry, 0, 0, 5000, false);
        }


        return true;
    }

    static bool HandleUnSpellVisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendCancelSpellVisual(newEntry);
        }
        else {
            handler->GetSession()->GetPlayer()->SendCancelSpellVisual(newEntry);
        }


        return true;
    }

    static bool HandleSetCastTargetCommand(ChatHandler* handler, char const* args) {

        if (!args)
            return false;

        Unit* target = handler->getSelectedUnit();

        char const* cible = strtok((char*)args, " ");
        char const* spell = strtok(NULL, " ");

        if (!cible)
            return false;
        if (!spell)
            return false;

        WorldObject* object = nullptr;
        ObjectGuid::LowType guid = UI64LIT(0);
        uint32 spellId = 0;
        std::string checkIsValid = cible;
        std::string checkSpell = spell;

        if (std::all_of(checkIsValid.begin(), checkIsValid.end(), ::isdigit) && std::all_of(checkSpell.begin(), checkSpell.end(), ::isdigit)) {

            guid = atoull(cible);
            spellId = atoi(spell);

        }
        else {

            return false;

        }

        object = handler->GetCreatureFromPlayerMapByDbGuid(guid);
        if (!object) {
            handler->PSendSysMessage(LANG_CAST_TARGET_NO_CREATURE_ON_MAP, guid);
            handler->SetSentErrorMessage(true);
            return false;
        }

            if (target && target->IsCreature()) {

                target->ToCreature()->SetTarget(object->GetGUID());
                target->ToCreature()->CastSpell(object->ToUnit(), spellId);
            }
            else if (target && !target->IsCreature()) {
                handler->PSendSysMessage(LANG_CAST_TARGET_NOT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .set cast target", handler->GetSession()->GetPlayer()->GetName().c_str());


        return true;

    }

    static bool HandleUnsetCastTargetCommand(ChatHandler* handler, char const* args) {

        Unit* target = handler->getSelectedUnit();

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .unset cast target", handler->GetSession()->GetPlayer()->GetName().c_str());

        if (target && target->IsCreature()) {

            target->ToCreature()->SendClearTarget();
            target->ToCreature()->CastStop();

        }
        else if (target && !target->IsCreature()) {
            handler->PSendSysMessage(LANG_CAST_TARGET_NOT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    /*static bool HandleSetNameCommand(ChatHandler* handler, char const* args)
    {

        if (!args)
            return false;

        uint8 compteur = 0;
        uint8 iter = 0;
        std::string newName;
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        int32 alphabetASCII[103] = { 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100,
            101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56,
            57, 233, 232, 45, 231, 224, 249, 181, 44, 226, 234, 251, 238, 244, 228, 235, 252, 239, 246, 230, 339, 46, 192, 194, 196, 198, 199, 200, 201, 202,
            203, 206, 207, 212, 214, 219, 220, 338, 34, 145, 146, 39 };

        char const* newNameStr = strtok((char*)args, " ");
        char const* newNameStr2 = strtok(NULL, " ");
        char const* newNameStr3 = strtok(NULL, " ");
        char const* newNameStr4 = strtok(NULL, " ");

        if (!newNameStr)
            return false;

        std::string name;
        std::string name2;
        std::string name3;
        std::string name4;
        std::stringstream ss;

        if (newNameStr) {
            name = newNameStr;
            std::wstring wname;
            Utf8toWStr(name, wname);
            for (int32 i = 0; i < wname.size(); ++i) {
                wchar_t a = wname[i];
                int32 b = int32(a);
                ++iter;
                for (int32 j = 0; j < 103; ++j) {
                    int32 d = alphabetASCII[j];
                    if (b == d) {
                        ++compteur;
                    }
                }
            }
            if (compteur == iter) {
                compteur = 0;
                iter = 0;
                ss << newNameStr;
                newName = ss.str().c_str();
                if (newNameStr2) {
                    name2 = newNameStr2;
                    std::wstring wname2;
                    Utf8toWStr(name2, wname2);
                    for (int32 i = 0; i < wname2.size(); ++i) {
                        wchar_t a = wname2[i];
                        int32 b = int32(a);
                        ++iter;
                        for (int32 j = 0; j < 103; ++j) {
                            int32 d = alphabetASCII[j];
                            if (b == d) {
                                ++compteur;
                            }
                        }
                    }
                    if (compteur == iter) {
                        compteur = 0;
                        iter = 0;
                        ss << "_" << newNameStr2;
                        newName = ss.str().c_str();
                        if (newNameStr3) {
                            name3 = newNameStr3;
                            std::wstring wname3;
                            Utf8toWStr(name3, wname3);
                            for (int32 i = 0; i < wname3.size(); ++i) {
                                wchar_t a = wname3[i];
                                int32 b = int32(a);
                                ++iter;
                                for (int32 j = 0; j < 103; ++j) {
                                    int32 d = alphabetASCII[j];
                                    if (b == d) {
                                        ++compteur;
                                    }
                                }
                            }
                            if (compteur == iter) {
                                compteur = 0;
                                iter = 0;
                                ss << "_" << newNameStr3;
                                newName = ss.str().c_str();
                                if (newNameStr4) {
                                    name4 = newNameStr4;
                                    std::wstring wname4;
                                    Utf8toWStr(name4, wname4);
                                    for (int32 i = 0; i < wname4.size(); ++i) {
                                        wchar_t a = wname4[i];
                                        int32 b = int32(a);
                                        ++iter;
                                        for (int32 j = 0; j < 103; ++j) {
                                            int32 d = alphabetASCII[j];
                                            if (b == d) {
                                                ++compteur;
                                            }
                                        }
                                    }
                                    if (compteur == iter) {
                                        compteur = 0;
                                        iter = 0;
                                        ss << "_" << newNameStr4;
                                        newName = ss.str().c_str();
                                    }
                                    else {
                                        compteur = 0;
                                        iter = 0;
                                        handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                                        handler->SetSentErrorMessage(true);
                                        return false;
                                    }
                                }
                                else {
                                    newName == ss.str().c_str();
                                }
                            }
                            else {
                                compteur = 0;
                                iter = 0;
                                handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                                handler->SetSentErrorMessage(true);
                                return false;
                            }
                        }
                        else {
                            newName = ss.str().c_str();
                        }
                    }
                    else {
                        compteur = 0;
                        iter = 0;
                        handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }
                else {
                    newName == ss.str().c_str();
                }
            }
            else {
                compteur = 0;
                iter = 0;
                handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!normalizePlayerName(newName))
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (ObjectMgr::CheckPlayerName(newName, handler->GetSession()->GetPlayer() ? handler->GetSession()->GetSessionDbcLocale() : sWorld->GetDefaultDbcLocale(), true) != CHAR_NAME_SUCCESS)
            {
                handler->PSendSysMessage(LANG_NAME_TOO_LONG);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (WorldSession* session = handler->GetSession())
            {
                if (!session->HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RESERVEDNAME) && sObjectMgr->IsReservedName(newName))
                {
                    handler->SendSysMessage(LANG_RESERVED_NAME);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            stmt->setString(0, newName);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                handler->PSendSysMessage(LANG_RENAME_PLAYER_ALREADY_EXISTS, newName.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            // Remove declined name from db
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
            stmt->setUInt64(0, playerGuid.GetCounter());
            CharacterDatabase.Execute(stmt);


            handler->GetSession()->GetPlayer()->SetName(newName);

            handler->PSendSysMessage(LANG_NAME_GOOD, newName.c_str());

            if (WorldSession* session = handler->GetSession())
                session->KickPlayer();


            sWorld->UpdateCharacterInfo(playerGuid, newName);

            handler->PSendSysMessage(LANG_RENAME_PLAYER_WITH_NEW_NAME, handler->GetSession()->GetPlayerName().c_str(), newName.c_str());

            if (WorldSession* session = handler->GetSession())
            {
                if (Player* player = session->GetPlayer())
                    sLog->outCommand(session->GetAccountId(), "GM %s (Account: %u) forced rename %s to player %s (Account: %u)", player->GetName().c_str(), session->GetAccountId(), newName.c_str(), handler->GetSession()->GetPlayerName().c_str(), ObjectMgr::GetPlayerAccountIdByGUID(playerGuid));
            }
            else
                sLog->outCommand(0, "CONSOLE forced rename '%s' to '%s' (%s)", handler->GetSession()->GetPlayerName().c_str(), newName.c_str(), playerGuid.ToString().c_str());

        }

        return true;
    }*/

    static bool HandleCancelTicketCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char const* tId = strtok((char*)args, " ");
        uint32 ticketId = uint32(atoi(tId));
        std::string playerName = handler->GetSession()->GetPlayer()->GetName();


        // check if the ticket exist in the database
        QueryResult checkExist = WorldDatabase.PQuery("SELECT ticketId, ticketStatus FROM ticket WHERE ticketId = %u", ticketId);
		Field* checkStatus = checkExist->Fetch();
        uint8 status = checkStatus[1].GetUInt8();

        if (status == 1)
        {
            handler->SendSysMessage(LANG_TICKET_ALREADY_CANCEL);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!checkExist)
        {
            handler->SendSysMessage(LANG_TICKET_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
            
        }
        else
        {
            QueryResult checkOwn = WorldDatabase.PQuery("SELECT ticketOwnerAccId FROM ticket WHERE ticketId = %u", ticketId);
            Field* field = checkOwn->Fetch();
            uint32 accId = field[0].GetUInt32();

            if (accId == handler->GetSession()->GetAccountId())
            {
                WorldDatabasePreparedStatement* updateTicket = WorldDatabase.GetPreparedStatement(WORLD_UPD_CANCEL_TICKET);
                updateTicket->setUInt32(0, ticketId);
                WorldDatabase.Execute(updateTicket);

                handler->PSendSysMessage(LANG_TICKET_SEND_NOTFI_CANCEL);
                sWorld->SendGMText(LANG_TICKET_CANCEL_BY_PLAYERS, playerName.c_str(), handler->GetSession()->GetAccountId());

            }
            else
            {
                handler->SendSysMessage(LANG_TICKET_NOTFOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
       
        }
           
        return true;

    }
    

    static bool HandleListTicketCommand(ChatHandler* handler, const char* args)
    {

        if (*args)
            return false;

        if (handler->GetSession()->GetSecurity() >= 2)
        {
            QueryResult query = WorldDatabase.PQuery("SELECT ticketId, ticketContents, ticketOwner, ticketOwnerAccId, ticketOwnerGuid FROM ticket WHERE ticketStatus = 0");

            if (query) {

                do {

                    Field* result = query->Fetch();

                    uint32 id = result[0].GetUInt32();
                    std::string ticketContents = result[1].GetString();
                    std::string ticketOwner = result[2].GetString();
                    uint32 ticketAccId = result[3].GetUInt32();
                    uint32 ticketOwnerGuid = result[4].GetUInt32();

               
                    handler->PSendSysMessage(LANG_TICKET_RESULT_GM_QUERY, id, ticketContents.c_str(), ticketOwner.c_str(), ticketAccId, ticketOwnerGuid);

                } while (query->NextRow());
            }
            else {
                handler->PSendSysMessage(LANG_TICKET_RESULT_NONE_GM);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            QueryResult queryP = WorldDatabase.PQuery("SELECT ticketId, ticketContents FROM ticket WHERE ticketOwnerAccId = %u AND ticketStatus = 0", handler->GetSession()->GetAccountId());
           

            if (queryP) {

                do {

                    Field* pResult = queryP->Fetch();

                    uint32 id = pResult[0].GetUInt32();
                    std::string ticketContents = pResult[1].GetString();
  
                    handler->PSendSysMessage(LANG_TICKET_RESULT_PLAYER_QUERY, id, ticketContents.c_str());

                } while (queryP->NextRow());
            }
            else {
                handler->PSendSysMessage(LANG_TICKET_RESULT_NONE_PLAYER);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }


        return true;

    }

    /*
    *  Check if the argument is correct for a rand
    *
    *  @pre: str, then argument for rand
    *        validArg other character than digit accepted
    *		 bool digitExpect if it expect digit for the first value in str (defaut false)
    *        index, the length of argument already checked (default 0)
    *
    *  @post: true, str must null or be digit(s) or
    * its characters (if it have) != digits is in validArg and have minimum a digit between them
    * and its characters respect partially the order of validArg
    *         false, else
    */
    static bool checkRandParsing(std::string str, std::string validArg, bool digitExpected = false, int index = 0)
    {
        if (str.size() == index)
        {
            if (!digitExpected)
                return true;
            else
                return false;
        }
        else
        {
            char arg = str.at(index);
            size_t pos = validArg.find(arg);

            if (isdigit(arg))
                return checkRandParsing(str, validArg, false, index + 1);
            else if (pos != std::string::npos && !digitExpected)
                return checkRandParsing(str, validArg.substr(pos + 1, std::string::npos), true, index + 1);
            else
                return false;
        }
    }


    /*
    *   Get position of the first character (!= digit) from a string (str)
    *   who is made up by digits and, possibly, characters
    *   @pre : str, string to check
    *          pos, length of str already checked (default 0)
    *
    *   @post: pos, the position of the first character that is not a digit
    *          std::string::npos if there is no character != digit
    */
    static size_t getPositionFirstChar(std::string str, size_t pos = 0)
    {
        if (pos == str.size())
            return std::string::npos;
        else if (isdigit(str.at(pos)))
            return getPositionFirstChar(str, pos + 1);
        else
            return pos;
    }


    /*
    *   Get position of the first digit from a string (str)
    *   who is made up by digits and, possibly, characters
    *   @pre : str, string to check
    *          pos, length of str already checked (default 0)
    *
    *   @post: pos, the position of the first digit
    *          std::string::npos if no digit
    */
    static size_t getPositionFirstDigit(std::string str, size_t pos = 0)
    {
        if (pos == str.size())
            return std::string::npos;
        else if (!isdigit(str.at(pos)))
            return getPositionFirstDigit(str, pos + 1);
        else
            return pos;
    }

    /*
    *   Parse a roll argument from a random command to get all values needed
    *   @pre :   arg, argument from a rand command who's respect checkParseRand conditions
    *            min, minimum roll
    *            max, maximum roll
    *            dice, number of rolls
    *            bonus, bonus after roll
    *
    *   @post :  min, max, dice, bonus changed according to args
    *            true if numbers used respected basics conditions of a roll
    *            false if min > max && dice > 30 && max >= 10000 && bonus >= 1000
    */
    static bool parseRand(std::string str, int& min, int& max, int& dice, int& bonus, int& expected)
    {
        size_t pos;
        pos = str.find("d");
        if (pos != std::string::npos)
        {
            dice = atoi(str.substr(0, pos).c_str());
            str.erase(0, pos + 1);
        }

        pos = str.find("-");

        if (pos != std::string::npos)
        {
            min = atoi(str.substr(0, pos).c_str());
            str.erase(0, pos + 1);
        }

        pos = getPositionFirstChar(str); // Get max from arg if it exists
        
        if (str.size() != 0 && pos != 0)
            max = atoi(str.substr(0, pos).c_str());

        pos = str.find("+");

        if (pos != std::string::npos)
        {
            str.erase(0, pos);
            pos = str.find(";");

            if (pos != std::string::npos)
            {
                bonus = atoi(str.substr(0, pos).c_str());
                str.erase(0, pos + 1);
                expected = atoi(str.c_str());
            }
            else
            {
                expected = min;
                bonus = atoi(str.c_str());
            }
        }

        if (min > max || dice >= 30 || max >= 10000 || bonus >= 1000 || expected > max)
            return false;
        else
            return true;
    }

    /*
    *   Create sentence according to parameters for a roll command argument
    *   @pre : playerName, the name of player
    *          args, argument from a roll command
    *
    *   @post : "Entr�es invalides" if args does not respect checkRandParsing conditions
    *and min > max && dice > 30 && max >= 10000 && bonus >= 1000
    *           else, the sentence to send for a roll command.
    */
    static std::string interpretRand(ChatHandler* handler, std::string str, std::string nameRoll = std::string(""))
    {
        std::string playerName = handler->GetSession()->GetPlayer()->GetName();
        std::string result = std::string("[CHECK_ROLL_ERROR] Entrees invalides");
        int min = 1, max = 100, dice = 0, bonus = 0, randResult = 0, expected = min;

        if (!checkRandParsing(str, std::string("d-+;")))
            return result;

        if (!parseRand(str, min, max, dice, bonus, expected))
            return result;

        if (handler->GetSession()->GetSecurity() < 3 && expected > min)
            return result;

        result = playerName;
        result.append(" a fait ");
   
        if (dice < 2)
        {
            if (nameRoll.size() > 0)
            {
                result.append("un jet de |cffffffff< ");
                result.append(nameRoll);
                result.append(" >|r ");
            }
            else
            {
                result.append("un jet de ");
            }

            if (expected <= min)
                randResult = urand((uint32)min, (uint32)max);
            else
                randResult = urand((uint32)expected, (uint32)max);

            result.append(std::to_string(randResult));
        }
        else
        {
            int tmp;
            result.append(std::to_string(dice));

            if (nameRoll.size() > 0)
            {
                result.append(" jets de |cffffffff< ");
                result.append(nameRoll);
                result.append(" >|r: ");
            }
            else
            {
                result.append(" jets : ");
            }

            do
            {
                if (expected <= min)
                    tmp = urand((uint32)min, (uint32) max);
                else
                    tmp = urand((uint32)expected, (uint32)max);

                randResult += tmp;
                result.append("[");
                result.append(std::to_string(tmp));
                result.append("]");
                dice--;

                if (dice != 0)
                    result.append(" + ");
                else if (bonus == 0)
                    result.append(" = ");
                else
                    result.append("  ");

            } while (dice > 0);
            
            result.append(std::to_string(randResult));
        }

        if (bonus > 0)
        {
            result.append("+");
            result.append(std::to_string(bonus));
            result.append(" = ");
            result.append(std::to_string(randResult + bonus));
            result.append(" !");
        }
        result.append(" (");
        result.append(std::to_string(min));
        result.append("-");
        result.append(std::to_string(max));
        result.append(")");
        return result;
    }

    /*
    *   Format and spread a message from player to a range depends on the type chosen.
    *
    *   @pre : player, Player who's spread the message
    *          str, the message
    *          type, type of spread (i.e : party, say, etc.)
    *          limit, number of 255 characters is used to limit spam
    *          maxLimit, the number maximum of messages (1 msg = 255 characters).
    *
    */
    static void formatAndSpread(Player* player, std::string str, int type, int maxSpread = 1, int limit = 0)
    {

        if (str.size() > maxSpread * 255 || limit >= maxSpread)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("[SPAM_DETECTED] Arret de la commande");
            return;
        }

        if (str.size() > 255)
        {
            int limitNeeded = spread(player, str.substr(0, 254), type, limit);
            formatAndSpread(player, str.substr(255, std::string::npos), type, maxSpread, limitNeeded);
        }
        else
        {
            spread(player, str.substr(0, 254), type, limit);
        }

    }

    /*
    *   Spread a message from player to a range depends on the type chosen.
    *
    *   @pre : player, Player who's spread the message
    *          str, the message ( < 255 characters)
    *          type, type of spread (i.e : party, say, etc.)
    *          limit, number of 255 characters is used to limit spam
    *
    *   @post: the limit used after spread the message.
    */
    static int spread(Player* player, std::string str, int type, int limit = 0)
    {

        switch (type)
        {
            case CHAT_MSG_SAY:
            {
                player->Talk(str.c_str(), CHAT_MSG_SYSTEM, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
                break;
            }
            case CHAT_MSG_PARTY:
            case CHAT_MSG_RAID:
            {
                Group* group = player->GetGroup();

                if (!group)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("%s", str.c_str());
                    break;
                }

                for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* receiver = itr->GetSource();
                    if (!receiver || receiver->GetGroup() != group)
                        continue;

                    if (receiver->GetSession())
                        ChatHandler(receiver->GetSession()).PSendSysMessage("%s", str.c_str());
                  
                }

            break;
            
        }
        case CHAT_MSG_GUILD: // Idea : by GuildID => checkConnectedPlayer
        default:
        {
            ChatHandler(player->GetSession()).PSendSysMessage("%s", str.c_str());
            break;
        }
    }

        return limit + 1;
    }

    /*
    * Extract all information from a roll command
    * @pre : str the argument from a roll command
    *        typeSpread, nameRoll, rollToParse : informations to extract
    *
    * @post : true always for the moment (in the future it will be useful)
    *         + informations extracted if there exist in str
    */
    static bool parseRandCommandInfo(std::string str, int& typeSpread, std::string& nameRoll, std::string& rollToParse)
    {
        if (str.size() == 0)
            return true;

        size_t pos = getPositionFirstDigit(str); // extract rollToParse

        if (pos == 0)
        {
            rollToParse = str;
            return true;
        }
        else if (pos != std::string::npos)
        {
            rollToParse = str.substr(pos, std::string::npos);
            str.erase(pos, std::string::npos);
        }

        pos = str.find(" ");

        if (pos != std::string::npos) // Can be spread and nameRoll, at this point we are not sure
        {
            std::string spreadStr = str.substr(0, pos);

            if (spreadStr.find("say") == 0 || spreadStr.find("dire") == 0)
            {
                typeSpread = CHAT_MSG_SAY;
                str.erase(0, pos + 1);
                nameRoll = str;
                str.erase(0, std::string::npos);
            }
            else
            {
                typeSpread = CHAT_MSG_PARTY;
                nameRoll = str;
                str.erase(0, std::string::npos);
            }
        }
        else if (str.find("say") == 0 || str.find("dire") == 0) // Only spread none nameRoll
        {
            typeSpread = CHAT_MSG_SAY;
            str.erase(0, std::string::npos);
        }

        if (str.size() > 0) // Only nameRoll
            nameRoll = str;

        if (nameRoll.size() > 0)
        {
            if (nameRoll.find(" ") == 0)  // Suppress useless first char if it's space
                nameRoll.erase(0, 1);

            if (nameRoll.back() == ' ') // Suppress useless last char if it's space
                nameRoll = nameRoll.substr(0, nameRoll.size() - 1);
        }

        return true;
    }

    /*
    *   Main command for a rand
    *   @pre : handler, args..I don't really need explain theses
    *
    *   @post : the answer of "Do you think Helnesis is planning to invade Poland ?".
    */
    static bool HandleRandomCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        std::string str = std::string(args);
        std::string nameRoll, rollToParse;
        int type = CHAT_MSG_PARTY;

        if (!parseRandCommandInfo(str, type, nameRoll, rollToParse))
        {
            handler->SendSysMessage("[RAND_PARSEINFO_ERROR] Entrees invalides");
            return true;
        }

        str = interpretRand(handler, rollToParse, nameRoll);

        if (str.compare("Entrees invalides") == 0)
        {
            handler->PSendSysMessage("%s", str.c_str());
            return true;
        }

        formatAndSpread(player, str, type);
        return true;
    }
    /*
    *   Base on type of power, get its name
    *
    *   @pre : type, the type of power
    *
    *   @post : the name of the power, if not found : power
    */
    static std::string getPowerName(int type)
    {
        std::string typeName;
        switch (type)
        {
        case POWER_MANA: typeName.append("mana"); break;
        case POWER_ENERGY: typeName.append("energie"); break;
        case POWER_RAGE: typeName.append("rage"); break;
        case POWER_FURY: typeName.append("furie"); break;
        case POWER_RUNIC_POWER: typeName.append("pouvoir runique"); break;
        case POWER_FOCUS: typeName.append("focus"); break;
        case POWER_CHI: typeName.append("chi"); break;
        default: typeName.append("power"); break;
        };
        return typeName;
    }
    /*
    *   Parse value to set, the value can be a fix value or add/minus to current
    *   @pre : str, string to parse
    *          current, current value
    *          max, max value to reach
    *
    *   @post : value = max if value > max
    *           value = 0 if value < 0
    */
    static int parseValueToSet(std::string str, int current, int max)
    {
        bool add = false, minus = false;

        if (str.size() > 1 && str.find("-") != std::string::npos)
        {
            minus = true;
            str.erase(0, 1);
        }
        else if (str.size() > 1 && str.find("+") != std::string::npos)
        {
            add = true;
            str.erase(0, 1);
        }

        size_t pos = getPositionFirstChar(str);

        if (pos != std::string::npos)
            return current;

        int32 value;

        if (str.size() > 6)
            value = max;
        else
            value = stoi(str);

        if ((minus && current - value <= 0) || value < 0)
            value = 0;
        else if (minus)
            value = current - value;
        else if (add && current + value >= max || value >= max)
            value = max;
        else if (add)
            value = current + value;
        return value;
    }

    static bool HandleSetPowerCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        std::string str = std::string(args);
        Player* player = handler->GetSession()->GetPlayer();
        Player* target = handler->getSelectedPlayerOrSelf();
        Powers type = target->GetPowerType();
        int value = parseValueToSet(str, target->GetPower(type), target->GetMaxPower(type));

        if (value == target->GetPower(type))
            return false;

        target->SetPower(type, value);
        target->CustomSetRegen(false); // Stop all regen, even HP

        if (type == POWER_MANA)
        {
            target->UpdateManaRegen();
        }

        handler->PSendSysMessage("Points de mana de %s : %d / %u", target->GetName().c_str(), value, target->GetMaxPower(type));
        ChatHandler(target->GetSession()).PSendSysMessage("%s change vos points de mana : %d / %u", player->GetName().c_str(), value, target->GetMaxPower(type));
        return true;
    }

    static bool HandleSetHealthCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        Unit* target = player->GetSelectedUnit();
        if (!target)
            target = player;

        std::string str = std::string(args);
        int value = parseValueToSet(str, target->GetHealth(), target->GetMaxHealth());

        if (value == target->GetHealth())
        {
            handler->SendSysMessage("Cela ne change rien");
            return true;
        }
        else if (value <= 0)
        {
            handler->SendSysMessage("Vous ne pouvez pas tuer avec cette commande");
            return true;
        }

        target->SetHealth(value);

        if (target->IsCreature())
        {
            //target->ToCreature()->setRegeneratingHealth(false);
            handler->PSendSysMessage("Points de vie de %s : %d / %u", target->GetName().c_str(), value, target->GetMaxHealth());
            return true;
        }

        Player* receiver = target->ToPlayer();
        receiver->CustomSetRegen(false); // Stop all regen, even power

        if (receiver->GetPowerType() == POWER_MANA)
        {
            receiver->UpdateManaRegen();
        }

        handler->PSendSysMessage("Points de vie de %s : %d / %u", target->GetName().c_str(), value, receiver->GetMaxHealth());
        ChatHandler(receiver->GetSession()).PSendSysMessage("%s change vos points de vie : %d / %u", player->GetName().c_str(), value, receiver->GetMaxHealth());
        return true;
    }
    
    static bool HandleRegenCommand(ChatHandler* handler, const char* args)
    {
        bool isRegen;
        Player* player = handler->GetSession()->GetPlayer();
        bool hasMana = player->GetPowerType() == POWER_MANA;

        if (!*args)
            return false;
        else
        {
            std::string str = std::string(args);

            if (str.find("on") != std::string::npos)
                isRegen = true;
            else if (str.find("off") != std::string::npos)
                isRegen = false;
            else
                return false;
        }

        if (isRegen)
        {
            player->CustomSetRegen(true);

            if (hasMana)
                player->UpdateManaRegen();

            handler->SendSysMessage("Regeneration ON");
        }
        else
        {
            player->CustomSetRegen(false);

            if (hasMana)
            {
                player->UpdateManaRegen();
            }

            handler->SendSysMessage("Regeneration OFF");
        }

        return true; 
    }


    static bool HandleUnAuraSelfCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->GetSession()->GetPlayer();

        std::string argstr = args;
        if (argstr == "all")
        {
            target->RemoveAllAuras();
            return true;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        target->RemoveAurasDueToSpell(spellId);

        return true;
    }

    static bool HandleAuraSelfCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSession()->GetPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    /*
    * ADDON HELPER 
    */
    static std::queue<std::string> parseParameters(const char* args) {
        std::string str = std::string(args);
        std::queue<std::string> q;
        std::regex reg("([^ ]+)");
        std::sregex_iterator currentMatch(str.begin(), str.end(), reg);
        std::smatch match = *currentMatch;
        std::sregex_iterator lastMatch;


        while (currentMatch != lastMatch) {
            match = *currentMatch;
            q.push(match.str());
            currentMatch++;
        }
            
        return q;
    }

    static bool brikabrokGobPosInfo(ChatHandler* handler, std::queue<std::string> q) {
        if (q.empty())
            return false;

        std::string guidStr = q.front();
        q.pop();

        if (getPositionFirstDigit(guidStr) == std::string::npos)
            return false;

        ObjectGuid::LowType guidLow = std::stoi(guidStr);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);

        if (!object) {
            handler->SendSysMessage("Guid invalid");
            return false;
        }

        const GameObjectData* data = sObjectMgr->GetGameObjectData(guidLow);
        if (!data)
            return false;

        float yaw, pitch, roll;
        data->rotation.toEulerAnglesZYX(yaw, pitch, roll);
        handler->PSendSysMessage("%s %f %f %f %f %f %f %f %f", object->GetName().c_str(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation(),yaw*(180 / M_PI),pitch*(180 / M_PI),roll*(180 / M_PI),data->size);
        return true;
    }

    static bool brikabrokGobPosAdd(ChatHandler* handler, std::queue<std::string> q)
    {
        if (q.empty())
            return false;

        if (!handler->GetSession()->HasPhasePermission(handler->GetSession()->GetPlayer()->GetMapId(), PhaseChat::Permissions::Gameobjects_Create)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        uint32 gobEntry;
        float xF;
        float yF;
        float zF;
        float oF;
        float yawF;
        float pitchF;
        float rollF;
        float sizeF;

        gobEntry = atoul(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;
        
        xF = (float)atof(q.front().c_str());
        q.pop();
        
        if (q.empty())
            return false;
        
        yF = (float) atof(q.front().c_str());
        q.pop();
        
        if (q.empty())
            return false;
        
        zF = (float)atof(q.front().c_str());
        q.pop();
        
        if (q.empty())
            return false;
        
        oF = (float)atof(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        yawF = ((float)atof(q.front().c_str()))*(M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        pitchF = ((float)atof(q.front().c_str()))*(M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        rollF = ((float)atof(q.front().c_str()))*(M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        sizeF = (float)atof(q.front().c_str());
        q.pop();

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(gobEntry);
        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, gobEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            // report to DB errors log as in loading case
            TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", gobEntry, objectInfo->type, objectInfo->displayId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_HAVE_INVALID_DATA, gobEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();

        GameObject* object = GameObject::CreateGameObject(objectInfo->entry, map, Position(xF, yF, zF, oF), QuaternionData::fromEulerAnglesZYX(yawF, pitchF, rollF), 255, GO_STATE_READY);

        if (!object)
            return false;

        PhasingHandler::InheritPhaseShift(object, player);

        // fill the gameobject data and save to the db
        // on r�cup le scale 
        object->SetObjectScale(sizeF);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        object->SaveToDB(player->GetMap()->GetId(), { player->GetMap()->GetDifficultyID() });

        ObjectGuid::LowType spawnId = object->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        // this will generate a new guid if the object is in an instance
        object = GameObject::CreateGameObjectFromDB(spawnId, map);
        if (!object)
            return false;

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGameObjectData(spawnId)));

        // Log
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        uint64 spawnerGuid = player->GetSession()->GetPlayer()->GetGUID().GetCounter();

        WorldDatabasePreparedStatement* gobInfo = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_LOG);
        gobInfo->setUInt64(0, spawnId);
        gobInfo->setUInt32(1, spawnerAccountId);
        gobInfo->setUInt64(2, spawnerGuid);
        WorldDatabase.Execute(gobInfo);

        return true;
    }

    static bool brikabrok(ChatHandler* handler, std::queue<std::string> q) {
        if (q.empty()) {
            handler->SendSysMessage("No method called");
            return false;
        }

        std::string methodCalled = q.front();
        q.pop();

        if (methodCalled.compare("gobpos") == 0)
            return brikabrokGobPosInfo(handler, q);
        else if (methodCalled.compare("gobaddxyz") == 0)
            return brikabrokGobPosAdd(handler, q);
        else
            return false;
    }

    static bool HandleAddonHelper(ChatHandler* handler, const char* args)
    {
        if (!*args) 
            return false;

        std::string params = std::string(args);
        std::queue<std::string> q = parseParameters(args);
        
        if (q.empty())
            return false;

        std::string nameAddon = q.front();
        q.pop();

        if (nameAddon.compare("brikabrok") == 0)
            return brikabrok(handler, q);
        else
            return false;
    }

    static bool HandleTestPacket(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        std::string _args = args;

        WorldPacket data(SMSG_DISPLAY_GAME_ERROR, 9);
        data << 0;
        data << _args;
        data << 200;
        handler->GetSession()->SendPacket(&data, true);

    }

    static bool HandleTestPacketAurora(ChatHandler* handler, const char* args)
    {
        WorldPacket data(SMSG_AURORA_TEST, 1);
        handler->GetSession()->SendPacket(&data, true);
    }

    // will use that for the gob addon since it allows to move freely without collisions ...
    static bool HandleFreeLook(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            if (handler->GetSession()->GetPlayer()->HasPlayerFlag(PLAYER_FLAGS_UBER) && handler->GetSession()->GetPlayer()->HasPlayerFlag(PLAYER_FLAGS_COMMENTATOR2))
                handler->GetSession()->SendNotification("Mode libre (ON)");
            else
                handler->GetSession()->SendNotification("Mode libre (OFF)");
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->AddPlayerFlag(PLAYER_FLAGS_UBER);
            handler->GetSession()->GetPlayer()->AddPlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
            handler->GetSession()->SendNotification("Mode libre (ON)");
            return true;
        }
        else if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->RemovePlayerFlag(PLAYER_FLAGS_UBER);
            handler->GetSession()->GetPlayer()->RemovePlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
            handler->GetSession()->SendNotification("Mode libre (OFF)");
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);

    }




    /*
    * END OF ADDON HELPER
    */

    /*
    static bool HandleSendTaxiCommand(ChatHandler* handler, const char* args)
    {
        //if (!*args)
          //  return false;

      //  uint32 uMountDisplay = atoul(args);
        Player* target = handler->getSelectedPlayer();

        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        

        target->GetSession()->SendActivateTaxiReply(ERR_TAXIOK);
        target->GetSession()->SendDoFlight(22234, 113);

    }
    */
    
};

void AddSC_misc_commandscript()
{
    new misc_commandscript();
}
