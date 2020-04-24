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

#ifndef _AURORA_WIN_H
#define _AURORA_WIN_H

#include <map>
#include "Cryptography/ARC4.h"
#include "Cryptography/BigNumber.h"
#include "ByteBuffer.h"
#include "Aurora.h"

#pragma pack(push, 1)

#pragma pack(pop)

class WorldSession;
class Aurora;

class TC_GAME_API AuroraWin : public Aurora
{
public:
    AuroraWin();
    ~AuroraWin();

    void Init(WorldSession* session) override;
    void RequestData() override;
    void HandleData(WorldPackets::Aurora::AuroraHWID& packet) override;

private:
    uint32 _serverTicks;
};

#endif