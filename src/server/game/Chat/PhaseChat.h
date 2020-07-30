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

#ifndef TRINITYCORE_PHASECHAT_H
#define TRINITYCORE_PHASECHAT_H

#include <vector>
#include <bitset>
#include <utility>

namespace PhaseChat {

    constexpr auto PhaseMaxPermissions = 16;
    constexpr auto SQLStorageSize = sizeof(uint8_t) * 8;

    static_assert(PhaseMaxPermissions% SQLStorageSize == 0, "Max number of permissions should be valid modulo SQLStorageSize");

    enum class Permissions : uint8_t {
        // Gameobjects
        Gameobjects_Update = 0,
        Gameobjects_Create = 1,
        Gameobjects_Delete = 2,

        // NPC
        Creature_Update = 3,
        Creature_Create = 4,
        Creature_Delete = 5,

        // Phase
        Phase_Edit = 6,
        Phase_Moderation = 7,
        Phase_Administration = 8,
        Phase_Invite = 9,
    };

    template <typename T, typename std::enable_if<
        std::is_integral<T>::value>::type* = nullptr>
        inline constexpr T GetPermissionsAs(Permissions permission) { return static_cast<T>(permission); }

    template <typename T, typename std::enable_if<
        std::is_integral<T>::value>::type* = nullptr>
        inline bool IsPermissionValid(T permission) { return permission >= GetPermissionsAs<T>(Permissions::Gameobjects_Update) && permission <= GetPermissionsAs<T>(Permissions::Phase_Invite); }

    constexpr static uint8_t AllPermissions[]
    {
        GetPermissionsAs<uint8_t>(Permissions::Gameobjects_Update),
        GetPermissionsAs<uint8_t>(Permissions::Gameobjects_Create),
        GetPermissionsAs<uint8_t>(Permissions::Gameobjects_Delete),

        GetPermissionsAs<uint8_t>(Permissions::Creature_Update),
        GetPermissionsAs<uint8_t>(Permissions::Creature_Create),
        GetPermissionsAs<uint8_t>(Permissions::Creature_Delete),

        GetPermissionsAs<uint8_t>(Permissions::Phase_Edit),
        GetPermissionsAs<uint8_t>(Permissions::Phase_Moderation),
        GetPermissionsAs<uint8_t>(Permissions::Phase_Administration),
        GetPermissionsAs<uint8_t>(Permissions::Phase_Invite),
    };

    template <typename T>
    inline void SetGenericPermission(T& data, Permissions perm) { data ^= 1 << GetPermissionsAs<size_t>(perm); }

    template <typename T>
    inline void RemoveGenericPermission(T& data, Permissions perm) { data &= ~(1 << GetPermissionsAs<size_t>(perm)); }

    template <typename T>
    inline bool CheckGenericPermissionRef(T& data, Permissions perm) { return (data >> GetPermissionsAs<size_t>(perm)) & 1; }

    inline std::vector<uint8_t> GetPermissionsToVector(std::bitset<PhaseMaxPermissions>& permissions) {
        std::vector<uint8_t> permissionsVector(PhaseMaxPermissions / SQLStorageSize);
        for (auto i = 0; i < PhaseMaxPermissions; ++i)
            permissions[i] ? SetGenericPermission<uint8_t>(permissionsVector[i / SQLStorageSize], Permissions(i % SQLStorageSize)) :
            RemoveGenericPermission<uint8_t>(permissionsVector[i / SQLStorageSize], Permissions(i % SQLStorageSize));
        return permissionsVector;
    }


    inline std::bitset<PhaseMaxPermissions> GetPermissionsFromArray(uint8_t* permissions) {
        std::bitset<PhaseMaxPermissions> bitPermissions;

        for (auto i = 0; i < PhaseMaxPermissions; ++i)
            bitPermissions[i] = CheckGenericPermissionRef<uint8_t>(permissions[i / SQLStorageSize], Permissions(i % SQLStorageSize));

        return bitPermissions;
    }

    inline std::bitset<PhaseMaxPermissions> GetPermissionsFromVector(std::vector<uint8_t>& permissions) { return GetPermissionsFromArray(&permissions[0]); }

    inline std::bitset<PhaseMaxPermissions> GetPermissionsFromParameters(std::initializer_list<Permissions> permissions) {
        std::bitset<PhaseMaxPermissions> bitPermissions;

        for (auto permission : permissions)
            bitPermissions[GetPermissionsAs<uint8_t>(permission)] = 1;

        return bitPermissions;
    }

    inline void SetFlags(std::bitset<PhaseMaxPermissions>& permissions, size_t size, uint8_t* flags) { for (auto i = 0; i < size; i++) permissions.set(flags[i]); }

    inline void SetAllFlags(std::bitset<PhaseMaxPermissions>& permissions) { SetFlags(permissions, sizeof(AllPermissions), (uint8_t*)(AllPermissions)); }

    struct PermissionsMap : public std::map<Permissions, std::string>
    {
        PermissionsMap()
        {
            this->operator[](Permissions::Gameobjects_Update) = "GOB_UPDATE";
            this->operator[](Permissions::Gameobjects_Create) = "GOB_CREATE";
            this->operator[](Permissions::Gameobjects_Delete) = "GOB_DELETE";

            this->operator[](Permissions::Creature_Update) = "NPC_UPDATE";
            this->operator[](Permissions::Creature_Create) = "NPC_CREATE";
            this->operator[](Permissions::Creature_Delete) = "NPC_DELETE";

            this->operator[](Permissions::Phase_Edit) = "PHASE_EDIT";
            this->operator[](Permissions::Phase_Moderation) = "PHASE_MODERATION";
            this->operator[](Permissions::Phase_Administration) = "PHASE_ADMINISTRATION";
            this->operator[](Permissions::Phase_Invite) = "PHASE_INVITE";
        };
        ~PermissionsMap() {}
    };


    struct PermissionsDescription : public std::map<Permissions, std::string>
    {
        PermissionsDescription()
        {
            this->operator[](Permissions::Gameobjects_Update) = "Allow to modify properties of objects (rotate, scale, ...)";
            this->operator[](Permissions::Gameobjects_Create) = "Allow to spawn new objects";
            this->operator[](Permissions::Gameobjects_Delete) = "Allow to delete existing objects";

            this->operator[](Permissions::Creature_Update) = "Allow you to modify properties of creature (scale, set anim ...)";
            this->operator[](Permissions::Creature_Create) = "Allow to spawn new creatures";
            this->operator[](Permissions::Creature_Delete) = "Allo to delete existing creatures";

            this->operator[](Permissions::Phase_Edit) = "Allow to change the apparanceo of the phase (terrain, sound, ...)";
            this->operator[](Permissions::Phase_Moderation) = "Allow you to kick from the phase";
            this->operator[](Permissions::Phase_Administration) = "Allow you to manage the phase (ranks, permissions) ";
            this->operator[](Permissions::Phase_Invite) = "Allow to invite other players in a phase.";
        };
        ~PermissionsDescription() {}
    };

}

#endif
