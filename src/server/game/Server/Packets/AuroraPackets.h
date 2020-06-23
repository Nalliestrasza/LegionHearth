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
        class AuroraHWID final : public ClientPacket
        {
        public:
            AuroraHWID(WorldPacket&& packet) : ClientPacket(CMSG_AURORA_HWID, std::move(packet)) { }

            void Read() override;

            std::uint32_t PhysicalDriveId = 0;
            std::uint32_t CPUId = 0;
            std::uint32_t VolumeInformation = 0;
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
    }
}

#endif // AuroraPackets_h__
