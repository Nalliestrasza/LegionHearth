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

#include "WorldDatabase.h"
#include "MySQLPreparedStatement.h"

void WorldDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_WORLDDATABASE_STATEMENTS);

    PrepareStatement(WORLD_SEL_QUEST_POOLS, "SELECT entry, pool_entry FROM pool_quest", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_CRELINKED_RESPAWN, "DELETE FROM linked_respawn WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_REP_CREATURE_LINKED_RESPAWN, "REPLACE INTO linked_respawn (guid, linkedGuid) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_CREATURE_TEXT, "SELECT CreatureID, GroupID, ID, Text, Type, Language, Probability, Emote, Duration, Sound, BroadcastTextId, TextRange FROM creature_text", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_SMART_SCRIPTS, "SELECT entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, event_param_string, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o FROM smart_scripts ORDER BY entryorguid, source_type, id, link", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_SMARTAI_WP, "SELECT entry, pointid, position_x, position_y, position_z FROM waypoints ORDER BY entry, pointid", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_GAMEOBJECT, "DELETE FROM gameobject WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_EVENT_GAMEOBJECT, "DELETE FROM game_event_gameobject WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GRAVEYARD_ZONE, "INSERT INTO graveyard_zone (ID, GhostZone, Faction) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GRAVEYARD_ZONE, "DELETE FROM graveyard_zone WHERE ID = ? AND GhostZone = ? AND Faction = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GAME_TELE, "INSERT INTO game_tele (id, position_x, position_y, position_z, orientation, map, name) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_TELE, "DELETE FROM game_tele WHERE name = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_NPC_VENDOR, "INSERT INTO npc_vendor (entry, item, maxcount, incrtime, extendedcost, type) VALUES(?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_NPC_VENDOR, "DELETE FROM npc_vendor WHERE entry = ? AND item = ? AND type = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_NPC_VENDOR_REF, "SELECT item, maxcount, incrtime, ExtendedCost, type, BonusListIDs, PlayerConditionID, IgnoreFiltering FROM npc_vendor WHERE entry = ? ORDER BY slot ASC", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE, "UPDATE creature SET MovementType = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_FACTION, "UPDATE creature_template SET faction = ? WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_NPCFLAG, "UPDATE creature_template SET npcflag = ? WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_POSITION, "UPDATE creature SET position_x = ?, position_y = ?, position_z = ?, orientation = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_SPAWN_DISTANCE, "UPDATE creature SET spawndist = ?, MovementType = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS, "UPDATE creature SET spawntimesecs = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_CREATURE_FORMATION, "INSERT INTO creature_formations (leaderGUID, memberGUID, dist, angle, groupAI) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_WAYPOINT_DATA, "INSERT INTO waypoint_data (id, point, position_x, position_y, position_z) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_WAYPOINT_DATA, "DELETE FROM waypoint_data WHERE id = ? AND point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_POINT, "UPDATE waypoint_data SET point = point - 1 WHERE id = ? AND point > ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_POSITION, "UPDATE waypoint_data SET position_x = ?, position_y = ?, position_z = ? where id = ? AND point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_WPGUID, "UPDATE waypoint_data SET wpguid = ? WHERE id = ? and point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_ID, "SELECT MAX(id) FROM waypoint_data", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_POINT, "SELECT MAX(point) FROM waypoint_data WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID, "SELECT point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data WHERE id = ? ORDER BY point", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_BY_ID, "SELECT point, position_x, position_y, position_z FROM waypoint_data WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_FIRST_BY_ID, "SELECT position_x, position_y, position_z FROM waypoint_data WHERE point = 1 AND id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_LAST_BY_ID, "SELECT position_x, position_y, position_z, orientation FROM waypoint_data WHERE id = ? ORDER BY point DESC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_WPGUID, "SELECT id, point FROM waypoint_data WHERE wpguid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_ALL_BY_WPGUID, "SELECT id, point, delay, move_type, action, action_chance FROM waypoint_data WHERE wpguid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_ALL_WPGUID, "UPDATE waypoint_data SET wpguid = 0", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_POS, "SELECT id, point FROM waypoint_data WHERE (abs(position_x - ?) <= ?) and (abs(position_y - ?) <= ?) and (abs(position_z - ?) <= ?)", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_WPGUID_BY_ID, "SELECT wpguid FROM waypoint_data WHERE id = ? and wpguid <> 0", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_ACTION, "SELECT DISTINCT action FROM waypoint_data", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPTS_MAX_ID, "SELECT MAX(guid) FROM waypoint_scripts", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_CREATURE_ADDON, "INSERT INTO creature_addon(guid, path_id) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_ADDON_PATH, "UPDATE creature_addon SET path_id = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_CREATURE_ADDON, "DELETE FROM creature_addon WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_CREATURE_ADDON_BY_GUID, "SELECT guid FROM creature_addon WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_WAYPOINT_SCRIPT, "INSERT INTO waypoint_scripts (guid) VALUES (?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_WAYPOINT_SCRIPT, "DELETE FROM waypoint_scripts WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_ID, "UPDATE waypoint_scripts SET id = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_X, "UPDATE waypoint_scripts SET x = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Y, "UPDATE waypoint_scripts SET y = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Z, "UPDATE waypoint_scripts SET z = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_O, "UPDATE waypoint_scripts SET o = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_ID_BY_GUID, "SELECT id FROM waypoint_scripts WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_CREATURE, "DELETE FROM creature WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_COMMANDS, "SELECT name, permission, help FROM command", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_TEMPLATE, "SELECT entry, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2, name, femaleName, subname, TitleAlt, IconName, gossip_menu_id, minlevel, maxlevel, HealthScalingExpansion, RequiredExpansion, VignetteID, faction, npcflag, speed_walk, speed_run, scale, `rank`, dmgschool, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags, unit_flags2, unit_flags3, dynamicflags, family, trainer_class, type, type_flags, type_flags2, lootid, pickpocketloot, skinloot, resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, spell1, spell2, spell3, spell4, spell5, spell6, spell7, spell8, VehicleId, mingold, maxgold, AIName, MovementType, InhabitType, HoverHeight, HealthModifier, HealthModifierExtra, ManaModifier, ManaModifierExtra, ArmorModifier, DamageModifier, ExperienceModifier, RacialLeader, movementId, WidgetSetID, WidgetSetUnitConditionID, RegenHealth, mechanic_immune_mask, flags_extra, ScriptName FROM creature_template WHERE entry = ? OR 1 = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_BY_ID, "SELECT guid, delay, command, datalong, datalong2, dataint, x, y, z, o FROM waypoint_scripts WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_BY_ID, "SELECT guid FROM creature WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_NEAREST, "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM gameobject WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_NEAREST_ADVANCED, "SELECT guid, id, position_x, position_y, position_z, map, orientation, size, rotation0 , rotation1 , rotation2 , rotation3, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM gameobject WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_NEAREST, "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM creature WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_CREATURE, "INSERT INTO creature (guid, id , map, spawnDifficulties, PhaseId, PhaseGroup, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, npcflag, unit_flags, unit_flags2, unit_flags3, dynamicflags, size) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_EVENT_CREATURE, "DELETE FROM game_event_creature WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_EVENT_MODEL_EQUIP, "DELETE FROM game_event_model_equip WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GAMEOBJECT, "INSERT INTO gameobject (guid, id, map, spawnDifficulties, PhaseId, PhaseGroup, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, size, hasDoodads, visibility) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_DISABLES, "INSERT INTO disables (entry, sourceType, flags, comment) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_DISABLES, "SELECT entry FROM disables WHERE entry = ? AND sourceType = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_DISABLES, "DELETE FROM disables WHERE entry = ? AND sourceType = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_ZONE_AREA_DATA, "UPDATE creature SET zoneId = ?, areaId = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_GAMEOBJECT_ZONE_AREA_DATA, "UPDATE gameobject SET zoneId = ?, areaId = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_SPAWNGROUP_MEMBER, "DELETE FROM spawn_group WHERE spawnType = ? AND spawnId = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GUILD_REWARDS_REQ_ACHIEVEMENTS, "SELECT AchievementRequired FROM guild_rewards_req_achievements WHERE ItemID = ?", CONNECTION_SYNCH);
	PrepareStatement(WORLD_INS_WP_MOVETYPE, "INSERT INTO waypoint_data (id, point, move_type) VALUES (?, ?, ?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_WP_MOVETYPE, "UPDATE waypoint_data SET move_type = ? WHERE id = ? AND point = ?", CONNECTION_ASYNC);
	PrepareStatement(WORLD_SEL_WAYPOINT_LOOKUP, "SELECT * FROM waypoint_data WHERE id = ?", CONNECTION_SYNCH);
	PrepareStatement(WORLD_INS_WP_DELAY, "INSERT INTO waypoint_data (id, point, delay) VALUES (?, ?, ?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_WP_DELAY, "UPDATE waypoint_data SET delay = ? WHERE id = ? AND point = ?", CONNECTION_ASYNC);

	//CUSTOM
	PrepareStatement(WORLD_INS_SET_ANIM, "INSERT INTO creature_addon(guid, emote) VALUES (?, ?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_SET_ANIM, "UPDATE creature_addon SET emote = ? WHERE guid = ?", CONNECTION_ASYNC);
	PrepareStatement(WORLD_INS_SET_AURA, "INSERT INTO creature_addon(guid, auras) VALUES (?, ?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_SET_AURA, "UPDATE creature_addon SET auras = ? WHERE guid = ?", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_SET_MOUNT, "UPDATE creature_addon SET mount = ? WHERE guid = ?", CONNECTION_ASYNC);
	PrepareStatement(WORLD_INS_SET_MOUNT, "INSERT INTO creature_addon(guid, mount) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_SET_ANIMKIT, "UPDATE creature_addon SET aiAnimKit = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_SET_ANIMKIT, "INSERT INTO creature_addon(guid, aiAnimKit) VALUES (?, ?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_SEL_GAMEOBJECT_TELE, "SELECT pos_x,pos_y,pos_z,mapid,orientation FROM gameobject_tele WHERE entry = ?", CONNECTION_SYNCH);
	PrepareStatement(WORLD_SEL_GAMEOBJECT_DOOR, "SELECT id_item FROM gameobject_door WHERE entry = ?", CONNECTION_SYNCH);

    // PHASE SYSTEMS

    //OWNER
    PrepareStatement(WORLD_INS_PHASE_OWNER, "INSERT INTO phase_owner (phaseId, accountOwner) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_SET_OWNER, "DELETE FROM phase_owner WHERE phaseId = ? AND accountOwner = ?", CONNECTION_ASYNC);

    //INVITE
    PrepareStatement(WORLD_INS_PHASE_INVITE, "INSERT INTO phase_allow (phaseId, playerId) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_PHASE_INVITE, "DELETE FROM phase_allow WHERE phaseId = ? AND playerId = ?", CONNECTION_ASYNC);

    //TERRAIN
    PrepareStatement(WORLD_INS_PHASE_TERRAIN, "INSERT INTO terrain_swap_defaults (MapId, TerrainSwapMap) VALUES (?,?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_PHASE_TERRAIN, "DELETE FROM terrain_swap_defaults WHERE MapId = ? AND TerrainSwapMap = ?", CONNECTION_ASYNC);

    // OWNE


    // TP CHECK
    PrepareStatement(WORLD_SEL_PHASETP, "SELECT playerId from phase_allow WHERE phaseId = ?", CONNECTION_SYNCH);

    // ALLOW
    PrepareStatement(WORLD_INS_PHASE_ALLOW, "INSERT INTO phase_allow (phaseId, playerId) VALUES (?,?)", CONNECTION_ASYNC);

    // TICKET System
    PrepareStatement(WORLD_INS_NEW_TICKET, "INSERT INTO ticket (ticketId, ticketContents, ticketOwner, ticketOwnerAccId, ticketOwnerGuid) VALUES (?,?,?,?,?)", CONNECTION_ASYNC);

    // ticket update, cancel by players
    PrepareStatement(WORLD_UPD_CANCEL_TICKET, "UPDATE ticket SET ticketStatus = 1 WHERE ticketId = ?", CONNECTION_ASYNC);

    // ticket update, close by gamemasters
    PrepareStatement(WORLD_UPD_CLOSE_TICKET, "UPDATE ticket SET ticketStatus = 1 WHERE ticketId = ?", CONNECTION_ASYNC);

    // gameobject log
    PrepareStatement(WORLD_INS_GAMEOBJECT_LOG, "INSERT INTO gameobject_log (guid, spawnerAccountId, spawnerPlayerId) VALUES (?,?,?)", CONNECTION_ASYNC);
    
    // gameobject raz
    PrepareStatement(WORLD_DEL_GAMEOBJECT_LOG, "DELETE FROM gameobject_log WHERE guid = ?", CONNECTION_ASYNC);

    // creature log
    PrepareStatement(WORLD_INS_CREATURE_LOG, "INSERT INTO creature_log (guid, spawnerAccountId, spawnerPlayerId, saved) VALUES (?,?,?,?)", CONNECTION_ASYNC);

    // creature raz
    PrepareStatement(WORLD_DEL_CREATURE_LOG, "DELETE FROM creature_log WHERE guid = ? and saved = 0", CONNECTION_ASYNC);

    // phaseown_map
    PrepareStatement(WORLD_INS_PHASEOWN_MAP, "INSERT INTO phaseown_map (position_x, position_y, position_z, orientation, map, name) VALUES (?,?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_PHASEOWN_MAP, "DELETE FROM phaseown_map WHERE map = ?", CONNECTION_ASYNC);

	//perma
	PrepareStatement(WORLD_INS_PERMAMORPH, "INSERT INTO player_custom (guid, displayId) VALUES (?,?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_PERMAMORPH, "UPDATE player_custom SET displayId = ? WHERE guid = ?", CONNECTION_ASYNC);

	PrepareStatement(WORLD_INS_PERMASCALE, "INSERT INTO player_custom (guid, scale) VALUES (?,?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_PERMASCALE, "UPDATE player_custom SET scale = ? WHERE guid = ?", CONNECTION_ASYNC);

	PrepareStatement(WORLD_INS_PERMASKYBOX, "INSERT INTO player_custom (guid, skybox) VALUES (?,?)", CONNECTION_ASYNC);
	PrepareStatement(WORLD_UPD_PERMASKYBOX, "UPDATE player_custom SET skybox = ? WHERE guid = ?", CONNECTION_ASYNC);

    // public & private
    PrepareStatement(WORLD_UPD_PHASE_SET_TYPE, "UPDATE phase_allow SET type = ? WHERE phaseId = ?", CONNECTION_ASYNC);

    // area name 
    PrepareStatement(WORLD_INS_PHASE_AREA_NAME, "INSERT INTO phase_custom_areaid(MapID, AreaID, ZoneID, MapName, AreaName) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_PHASE_AREA_NAME, "SELECT * FROM phase_custom_areaid WHERE MapID = ? AND AreaID = ? AND ZoneID = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_PHASE_AREA_NAME, "UPDATE phase_custom_areaid SET MapName = ?, AreaName = ? WHERE MapID = ? AND AreaID = ? AND ZoneID = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_PHASE_AREA_NAME, "DELETE FROM phase_custom_areaid WHERE MapID = ? AND AreaID = ? AND ZoneID = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_PHASE_MAP_AREAS_NAMES, "SELECT * FROM phase_custom_areaid WHERE MapID = ?", CONNECTION_SYNCH);


    // DUPPLICATIONS

    // dupplication_template
    PrepareStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_TEMPLATE, "INSERT INTO gameobject_dupplication_template (entry,name,account,referenceEntry,referencePosZ,referenceOrientation,referenceSize,author) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE, "SELECT name,referenceEntry,referencePosZ,referenceOrientation,referenceSize,isPrivate,account FROM gameobject_dupplication_template WHERE entry = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_TEMPLATE, "UPDATE gameobject_dupplication_template SET isPrivate = ? WHERE entry = ? ", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE_ACCOUNT, "SELECT entry FROM gameobject_dupplication_template WHERE entry = ? AND account = ? ", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATION_TEMPLATE, "DELETE FROM gameobject_dupplication_template WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_TEMPLATE_MAX_ID, "SELECT MAX(entry) from gameobject_dupplication_template", CONNECTION_SYNCH);

    // dupplication_doodads
    PrepareStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_DOODADS, "INSERT INTO gameobject_dupplication_doodads(entry, objectID, diffX, diffY, diffZ, diffO, size, rotationX, rotationY, rotationZ, rotationW, distance, angle, hasDoodads, visibility) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_DOODADS, "SELECT objectID, diffX, diffY, diffZ, diffO, size, rotationX, rotationY, rotationZ, rotationW, distance, angle, hasDoodads, visibility FROM gameobject_dupplication_doodads WHERE entry = ? ORDER BY guid ASC", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATION_DOODADS, "DELETE FROM gameobject_dupplication_doodads WHERE entry = ?", CONNECTION_ASYNC);

    // dupplication_guid
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_GUID, "SELECT guid,entry,refObjGuid,nom,posX,posY,posZ, (POW(posX - ?, 2) + POW(posY - ?, 2) + POW(posZ - ?, 2)) AS order_ FROM gameobject_dupplication_guid WHERE deleted = 0 AND mapID = ? ORDER BY order_ ASC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_GAMEOBJECT_DUPPLICATION_GUID, "INSERT INTO `gameobject_dupplication_guid` (`guid`, `entry`, `refObjGuid`, `nom`, `spawner`, `posX`, `posY`, `posZ`, `mapID`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_DUPPLICATION_GUID_MAX_ID, "SELECT MAX(guid) FROM gameobject_dupplication_guid", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_GUID, "UPDATE gameobject_dupplication_guid SET deleted = 1 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_GAMEOBJECT_DUPPLICATION_GUID_ENTRY, "UPDATE gameobject_dupplication_guid SET deleted = 1 WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAMEOBJECT_DUPPLICATIONS, "DELETE FROM gameobject_dupplication_guid WHERE deleted = 1 AND guid NOT IN(SELECT guid FROM(SELECT guid FROM gameobject_dupplication_guid ORDER BY guid DESC LIMIT 1) del);", CONNECTION_ASYNC);
    

    // dupplication | gameobject_logs
    PrepareStatement(WORLD_SEL_GAMEOBJECT_LOG, "SELECT guid from gameobject_log WHERE dupplicationGuid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_GAMEOBJECT_LOG_DUPPLICATION_GUID, "INSERT INTO gameobject_log (guid, spawnerAccountId, spawnerPlayerId, dupplicationGuid) VALUES (?,?,?,?)", CONNECTION_ASYNC);

    // outfit customization | creature_template_outfit_customization
    PrepareStatement(WORLD_SEL_OUTFIT_CUSTOMIZATIONS, "SELECT chrCustomizationOptionID,chrCustomizationChoiceID from creature_template_outfits_customizations WHERE outfitID = ?", CONNECTION_SYNCH);
	
}

WorldDatabaseConnection::WorldDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

WorldDatabaseConnection::WorldDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

WorldDatabaseConnection::~WorldDatabaseConnection()
{
}
