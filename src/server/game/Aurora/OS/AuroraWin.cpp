#include "Cryptography/HmacHash.h"
#include "Cryptography/SessionKeyGeneration.h"
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "Database/DatabaseEnv.h"
#include "GameTime.h"
#include "World.h"
#include "Player.h"
#include "Util.h"
#include "AuroraWin.h"
#include "SHA1.h"
#include "Random.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <openssl/md5.h>
#include "AuroraPackets.h"

AuroraWin::AuroraWin() : Aurora(), _serverTicks(0) {}

AuroraWin::~AuroraWin() { }

void AuroraWin::Init(WorldSession* session)
{
    _session = session;
    _initialized = true;
    TC_LOG_DEBUG("misc", "Server side HWID Checker for client %llu initializing ...", session->GetAccountId());
}

void AuroraWin::RequestData()
{
    TC_LOG_DEBUG("misc", "Serverside side HWID Checker for client %llu sending a request ...", _session->GetAccountId());

    WorldPacket pkt;
    pkt.Initialize(SMSG_AURORA_TRACKER, 1);
    pkt << 666;

    _session->SendPacket(&pkt);
    _dataSent = true;
}

void AuroraWin::HandleData(WorldPackets::Aurora::AuroraHWID& packet)
{
    TC_LOG_DEBUG("misc", "Serverside side HWID Checker for client %llu processing data ...", _session->GetAccountId());

    _dataSent = false;
    _clientResponseTimer = 0;

    // get all accounts matchting the hwid ...
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_HWID_BAN);
    stmt->setUInt32(0, packet.PhysicalDriveId);
    stmt->setUInt32(1, packet.CPUId);
    stmt->setUInt32(2, packet.VolumeInformation);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    bool hasBannedAccoutHWID = false;
    bool recordAlreadyExists = false;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32_t accountID = fields[0].GetUInt32();
            bool isBanned = fields[1].GetBool();

            // check if at least one account is banned
            if (isBanned)
                hasBannedAccoutHWID = true;

            // don't want multiple rows with the same accountID
            if (accountID == _session->GetAccountId())
                recordAlreadyExists = true;

            // if hwid banned on any accounts prevent the player from connecting
            if (isBanned)
                _session->KickPlayer();

        } while (result->NextRow());


        // duplicate existings bans for the same hwid
        if (!recordAlreadyExists)
        {
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_HWID_BAN);
            stmt->setUInt32(0, _session->GetAccountId());
            stmt->setUInt32(1, packet.PhysicalDriveId);
            stmt->setUInt32(2, packet.CPUId);
            stmt->setUInt32(3, packet.VolumeInformation);
            stmt->setString(4, _session->GetRemoteAddress());
            stmt->setBool(5, hasBannedAccoutHWID);
            stmt->setBool(6, packet.IsVirtualMachine);
            LoginDatabase.Execute(stmt);
        }
        else {
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_HWID_INFO_IP);
            stmt->setString(0, _session->GetRemoteAddress());
            stmt->setBool(1, packet.IsVirtualMachine);
            stmt->setUInt32(2, _session->GetAccountId());
            LoginDatabase.Execute(stmt);
        }

    }
    else {
        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_HWID_BAN);
        stmt->setUInt32(0, _session->GetAccountId());
        stmt->setUInt32(1, packet.PhysicalDriveId);
        stmt->setUInt32(2, packet.CPUId);
        stmt->setUInt32(3, packet.VolumeInformation);
        stmt->setString(4, _session->GetRemoteAddress());
        stmt->setBool(5, false);
        stmt->setBool(6, packet.IsVirtualMachine);
        LoginDatabase.Execute(stmt);
    }

    _session->SetHWID(packet.PhysicalDriveId, packet.CPUId, packet.VolumeInformation, packet.IsVirtualMachine);

    // Set hold off timer, minimum timer should at least be 1 second
    uint32 holdOff = 3600;
    _checkTimer = (holdOff < 1 ? 1 : holdOff) * IN_MILLISECONDS;
}
