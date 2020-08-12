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

#ifndef _AURORA_BASE_H
#define _AURORA_BASE_H

#include <map>
#include "ARC4.h"
#include "ByteBuffer.h"
#include <array>

class WorldSession;

class TC_GAME_API Aurora
{
    friend class AuroraWin;

public:
    Aurora();
    virtual ~Aurora();

    virtual void Init(WorldSession* session, SessionKey const& K) = 0;
    virtual void RequestData() = 0;
    virtual void HandleData(WorldPackets::Aurora::AuroraHWID& packet) = 0;

    void DecryptData(uint8* buffer, uint32 length);
    void EncryptData(uint8* buffer, uint32 length);

    void Update();
private:
    WorldSession* _session;
    uint32 _checkTimer;                          // Timer for sending check requests
    uint32 _clientResponseTimer;                 // Timer for client response delay
    bool _dataSent;

    uint32 _previousTimestamp;
    bool _initialized;
    uint8 _inputKey[16];
    uint32 _baseSeed;
    bool _initialPacket;
    Trinity::Crypto::ARC4 _keyCrypto;
};

#endif
