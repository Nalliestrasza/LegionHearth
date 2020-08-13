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

#include "AuroraPackets.h"

void WorldPackets::Aurora::AuroraHWID::Read()
{
    uint32 bufferFixIsVM;

    _worldPacket >> Version;
    _worldPacket >> PhysicalDriveId;
    _worldPacket >> VolumeInformation;
    _worldPacket >> CPUId;
    _worldPacket >> bufferFixIsVM;
    IsVirtualMachine = static_cast<bool>(bufferFixIsVM);
}

void WorldPackets::Aurora::AuroraMoveGameObject::Read()
{
    _worldPacket >> GameObjectsCount;

    GameObjects.resize(GameObjectsCount);

    for (uint32 i = 0; i < GameObjectsCount; ++i) {
        uint64_t GuidLow, GuidHigh;
        _worldPacket >> GuidLow;
        _worldPacket >> GuidHigh;

        GameObjects[i].ObjectManagerGuid.SetRawValue(GuidHigh, GuidLow);

        _worldPacket >> GameObjects[i].Position.x;
        _worldPacket >> GameObjects[i].Position.y;
        _worldPacket >> GameObjects[i].Position.z;

        _worldPacket >> GameObjects[i].Rotation.x;
        _worldPacket >> GameObjects[i].Rotation.y;
        _worldPacket >> GameObjects[i].Rotation.z;
        _worldPacket >> GameObjects[i].Rotation.w;

        _worldPacket >> GameObjects[i].Scale;
    }

}

void WorldPackets::Aurora::AuroraCreateGameObject::Read()
{
    _worldPacket >> GameObjectsCount;

    GameObjects.resize(GameObjectsCount);

    for (uint32 i = 0; i < GameObjectsCount; ++i) {

        _worldPacket >> GameObjects[i].Entry;

        _worldPacket >> GameObjects[i].Position.x;
        _worldPacket >> GameObjects[i].Position.y;
        _worldPacket >> GameObjects[i].Position.z;

        _worldPacket >> GameObjects[i].Rotation.x;
        _worldPacket >> GameObjects[i].Rotation.y;
        _worldPacket >> GameObjects[i].Rotation.z;
        _worldPacket >> GameObjects[i].Rotation.w;

        _worldPacket >> GameObjects[i].Scale;
    }
}

void WorldPackets::Aurora::AuroraDeleteGameObject::Read()
{

    _worldPacket >> GameObjectsCount;

    GameObjectsGuid.resize(GameObjectsCount);

    for (uint32 i = 0; i < GameObjectsCount; ++i) {

        uint64_t GuidLow, GuidHigh;
        _worldPacket >> GuidLow;
        _worldPacket >> GuidHigh;

        GameObjectsGuid[i].SetRawValue(GuidHigh, GuidLow);
    }
}

void WorldPackets::Aurora::AuroraEnableFreelook::Read()
{
    _worldPacket >> Enable;
}

WorldPacket const* WorldPackets::Aurora::AuroraZoneCustom::Write()
{
    _worldPacket << uint32(AreaID);
    _worldPacket << uint32(MapID);
    _worldPacket << uint32_t(ZoneID);

    _worldPacket << std::string(ZoneName);
    _worldPacket << std::string(SubZoneName);

    _worldPacket << uint32_t(Delete);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Aurora::AuroraTracker::Write()
{
    _worldPacket << Seed;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Aurora::AuroraCustomWorldModelObject::Write()
{
    _worldPacket << GuidLow;
    _worldPacket << GuidHigh;
    _worldPacket << Yaw;
    _worldPacket << Pitch;
    _worldPacket << Roll;
    _worldPacket << Scale;
    _worldPacket << HasDoodads;

    return &_worldPacket;
}
