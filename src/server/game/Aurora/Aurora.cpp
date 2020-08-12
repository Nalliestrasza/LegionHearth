#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "GameTime.h"
#include "World.h"
#include "Util.h"
#include "Aurora.h"
#include "AccountMgr.h"
#include "AuroraPackets.h"

#include <openssl/sha.h>

Aurora::Aurora() : _session(NULL), _checkTimer(10000/*10 sec*/), _clientResponseTimer(0),
_dataSent(false), _previousTimestamp(0), _initialized(false)
{

}

Aurora::~Aurora()
{
    _initialized = false;
}

void Aurora::Update()
{
    if (_initialized)
    {
        uint32 currentTimestamp = GameTime::GetGameTimeMS();
        uint32 diff = currentTimestamp - _previousTimestamp;
        _previousTimestamp = currentTimestamp;

        if (_dataSent)
        {
            uint32 maxClientResponseDelay = 5;

            if (maxClientResponseDelay > 0)
            {
                // Kick player if client response delays more than set in config
                if (_clientResponseTimer > maxClientResponseDelay * IN_MILLISECONDS)
                {
                    TC_LOG_ERROR("misc", "%s (latency: %u, IP: %s) exceeded Aurora module response delay for more than %s - disconnecting client",
                        _session->GetPlayerInfo().c_str(), _session->GetLatency(), _session->GetRemoteAddress().c_str(), secsToTimeString(maxClientResponseDelay, true).c_str());
                    _session->KickPlayer();
                }
                else {
                    _clientResponseTimer += diff;
                }
            }
        }
        else
        {
            if (diff >= _checkTimer)
            {
                RequestData();
            }
            else {
                _checkTimer -= diff;
            }
        }
    }
}

void Aurora::RequestData()
{
    WorldPacket pkt;
    pkt.Initialize(SMSG_AURORA_TRACKER, 1);
    pkt << 666; // implement a key system to avoid spoofing ?

    _session->SendPacket(&pkt);

    _dataSent = true;
}

void WorldSession::HandleAuroraData(WorldPackets::Aurora::AuroraHWID& packet)
{
    if (_aurora) 
        _aurora->HandleData(packet);
}

void WorldSession::SetHWID(uint32 hardDrive, uint32 processor, uint32 partition, bool isVM)
{
    _physicalDriveID = hardDrive;
    _cpuID = processor;
    _volumeInformation = partition;
    _isVirtualMachine = isVM;
}

uint32 WorldSession::GetHardDriveSerial()
{
    return _physicalDriveID;
}

uint32 WorldSession::GetProcessorID()
{
    return _cpuID;
}

uint32 WorldSession::GetPartitionID()
{
    return _volumeInformation;
}

bool WorldSession::IsVirtualMachine()
{
    return _isVirtualMachine;
}
