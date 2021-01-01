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

#include "WorldSession.h"
#include "DB2Stores.h"
#include "GameTables.h"
#include "Player.h"
#include "GameObject.h"
#include "Log.h"
#include "AuroraPackets.h"
#include "Map.h"
#include "PhasingHandler.h"
#include "ObjectMgr.h"
#include "Chat.h"
/* CMSG */

void WorldSession::HandleAuroraCreateGameObject(WorldPackets::Aurora::AuroraCreateGameObject& gameobjectData)
{
    WorldSession* session = _player->GetSession();
    if (!session->HasPhasePermission(_player->GetMapId(), PhaseChat::Permissions::Gameobjects_Create)) {
        return;
    }

    for (auto& gameObject : gameobjectData.GameObjects) {

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(gameObject.Entry);
        if (!objectInfo)
            return;

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            return;

        Map* map = _player->GetMap();

        GameObject* object = GameObject::CreateGameObject(objectInfo->entry, map, Position(gameObject.Position.x, gameObject.Position.y, gameObject.Position.z, 0), QuaternionData(gameObject.Rotation.x, gameObject.Rotation.y, gameObject.Rotation.z, gameObject.Rotation.w), 255, GO_STATE_READY);

        if (!object)
            return;

        PhasingHandler::InheritPhaseShift(object, _player);

        // fill the gameobject data and save to the db
        // on récup le scale
        object->SetObjectScale(gameObject.Scale);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), 0);
        object->SaveToDB(_player->GetMap()->GetId(), { _player->GetMap()->GetDifficultyID() });

        ObjectGuid::LowType spawnId = object->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        // this will generate a new guid if the object is in an instance
        object = GameObject::CreateGameObjectFromDB(spawnId, map);
        if (!object)
            return;

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGameObjectData(spawnId)));
    }
}

void WorldSession::HandleAuroraMoveGameObject(WorldPackets::Aurora::AuroraMoveGameObject& gameobjectData)
{
    WorldSession* session = _player->GetSession();
    if (!session->HasPhasePermission(_player->GetMapId(), PhaseChat::Permissions::Gameobjects_Update)) {
        return;
    }

    Map* map = _player->GetMap();
    for (auto& gameObject : gameobjectData.GameObjects) {

        if (GameObject* object = map->GetGameObject(gameObject.ObjectManagerGuid)) {
            ObjectGuid::LowType guidLow = object->GetSpawnId();
            const_cast<GameObjectData*>(object->GetGameObjectData())->size = gameObject.Scale;
            object->SetObjectScale(gameObject.Scale);
            object->SetWorldRotation(gameObject.Rotation.x, gameObject.Rotation.y, gameObject.Rotation.z, gameObject.Rotation.w);
            object->EnableCollision(true);
            object->Relocate(gameObject.Position.x, gameObject.Position.y, gameObject.Position.z, _player->GetOrientation());
            object->SaveToDB();
            object->Delete();
            object->UpdateObjectVisibility(true);
            object = GameObject::CreateGameObjectFromDB(guidLow, map);
            object->UpdateObjectVisibility(true);
        }
    }
}


void WorldSession::HandleAuroraDeleteGameObject(WorldPackets::Aurora::AuroraDeleteGameObject& gameobjectData)
{
    WorldSession* session = _player->GetSession();
    if (!session->HasPhasePermission(_player->GetMapId(), PhaseChat::Permissions::Gameobjects_Delete)) {
        return;
    }

    Map* map = _player->GetMap();

    for (auto& guid : gameobjectData.GameObjectsGuid) {
        if (GameObject* object = map->GetGameObject(guid)) {
            object->Delete();
            object->UpdateObjectVisibility(true);
            object->DeleteFromDB();
        }
    }

}

void WorldSession::HandleAuroraEnableCommentator(WorldPackets::Aurora::AuroraEnableFreelook& clientEnableFreelook) {

    if (clientEnableFreelook.Enable)
    {
        _player->AddPlayerFlag(PLAYER_FLAGS_UBER);
        _player->AddPlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
    }
    else
    {
        _player->RemovePlayerFlag(PLAYER_FLAGS_UBER);
        _player->RemovePlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
    }
}

/* SMSG */

void WorldSession::SendAuroraZoneCustom(WorldPackets::Aurora::AuroraZoneCustom& zoneCustom)
{
    SendPacket(zoneCustom.Write());
}

void WorldSession::SendAuroraTracker(WorldPackets::Aurora::AuroraTracker& tracker)
{
    SendPacket(tracker.Write());
}

void WorldSession::SendAuroraCustomWorldModelObject(WorldPackets::Aurora::AuroraCustomWorldModelObject& worldModelObject)
{
    SendPacket(worldModelObject.Write());
}
