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

#ifndef AuroraPackets_h__
#define AuroraPackets_h__

#include "Packet.h"
#include "Object/ObjectGuid.h"
#include "G3D/Vector3.h"
#include "G3D/Quat.h"
#include "Log.h"

namespace WorldPackets
{
    namespace Aurora
    {
        /* CMSG */
        using AuroraKey = std::array<uint8, 16>;

        class AuroraHWID final : public ClientPacket
        {
        public:
            AuroraHWID(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_HWID, std::move(packet)) { }

            void Read() override;

            uint32 Seed = 0;
            uint32 PhysicalDriveId = 0;
            uint32 CPUId = 0;
            uint32 VolumeInformation = 0;
            bool IsVirtualMachine = false;
        };

        class AuroraMoveGameObject final : public ClientPacket
        {
        public:
            AuroraMoveGameObject(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_MOVE_GAMEOBJECT, std::move(packet)) { }

            void Read() override;

            struct GameObjectData {
                ObjectGuid ObjectManagerGuid;
                G3D::Vector3 Position = { 0, 0, 0 };
                G3D::Quat Rotation = { 0, 0, 0, 0 };
                float Scale = 1.0f;
            };

            std::vector<GameObjectData> GameObjects;
            uint32 GameObjectsCount;
        };

        class AuroraCreateGameObject final : public ClientPacket
        {
        public:
            AuroraCreateGameObject(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_SPAWN_GAMEOBJECT, std::move(packet)) { }

            void Read() override;

            struct GameObjectData {
                uint32_t Entry = 0;
                G3D::Vector3 Position = { 0, 0, 0 };
                G3D::Quat Rotation = { 0, 0, 0, 0 };
                float Scale = 1.0f;
            };

            std::vector<GameObjectData> GameObjects;
            uint32 GameObjectsCount;
        };

        class AuroraDeleteGameObject final : public ClientPacket
        {
        public:
            AuroraDeleteGameObject(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_DELETE_GAMEOBJECT, std::move(packet)) { }

            void Read() override;

            std::vector <ObjectGuid> GameObjectsGuid;
            uint32 GameObjectsCount;
        };

        class AuroraEnableFreelook final : public ClientPacket
        {
        public:
            AuroraEnableFreelook(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_ENABLE_FREELOOK, std::move(packet)) { }

            void Read() override;

            bool Enable = true;
        };

        /* SMSG */

        class AuroraZoneCustom final : public ServerPacket
        {
        public:
            AuroraZoneCustom() : ServerPacket(SMSG_AURORA_ZONE_CUSTOM, 2) { }

            WorldPacket const* Write() override;

            uint32 AreaID;
            uint32 MapID;
            uint32 ZoneID;
            std::string ZoneName;
            std::string SubZoneName;
            uint32 Delete;
        };

        class AuroraTracker final : public ServerPacket
        {
        public:
            AuroraTracker(uint8 type, uint32 seed, Optional<AuroraKey> auroraKey = boost::none) : ServerPacket(SMSG_AURORA_TRACKER, 1 + sizeof(AuroraKey) + 4),
                Type(type), Seed(seed), AuroraKey(auroraKey) { }

            WorldPacket const* Write() override;

            uint8_t Type = 0;
            Optional<AuroraKey> AuroraKey = boost::none;
            uint32_t Seed = 0;
        };

        class AuroraCustomWorldModelObject final : public ServerPacket
        {
        public:
		
            AuroraCustomWorldModelObject() : ServerPacket(SMSG_AURORA_UPDATE_WMO, (8 * 2) + (4 * 4) + 4) { }

            WorldPacket const* Write() override;

            uint64 GuidLow = 0;
            uint64 GuidHigh = 0;
            float Yaw = 0.0;
            float Pitch = 0.0;
            float Roll = 0.0;
            float Scale = 1.0;
            uint32 HasDoodads = 0;
        };


    }
}

#endif // AuroraPackets_h__
