#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceSaveMgr.h"
#include "IpAddress.h"
#include "IPLocation.h"
#include "Item.h"
#include "Language.h"
#include "LFG.h"
#include "Log.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MMapFactory.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Pet.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "Realm.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "TargetedMovementGenerator.h"
#include "Transport.h"
#include "Weather.h"
#include "WeatherMgr.h"
#include "World.h"
#include "WorldSession.h"
#include <boost/asio/ip/address_v4.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <G3D/Quat.h>
#include "ItemTemplate.h"
#include "HotfixPackets.h"
#include "Position.h"
#include "Object.h"
#include "Bag.h"
//
#include "GossipDef.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "Log.h"
#include "NPCPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "QuestPackets.h"
#include "World.h"
#include "WorldSession.h"
#include "GameEventMgr.h"
#include <regex>
#include "AuroraPackets.h"
#include "ScriptedGossip.h";

 // temporary hack until database includes are sorted out (don't want to pull in Windows.h everywhere from mysql.h)
#ifdef GetClassName
#undef GetClassName
#endif

class custom_commandscript : public CommandScript
{
public:
    custom_commandscript() : CommandScript("custom_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> customCommandTable =
        {
            { "sendfly",		rbac::RBAC_PERM_COMMAND_AURA,       false, &HandleSendFlyCommand,       "" },
        };

        return customCommandTable;
    }


    static bool HandleSendFlyCommand(ChatHandler* handler, char const* args)
    {
 
        Player* plr = handler->getSelectedPlayerOrSelf();

        if (!plr)
            return false;

        std::string trinityArgs(args);
        Tokenizer dataArgs(trinityArgs, ' ', 0, false);

        if (dataArgs.size() < 2)
        {
            handler->PSendSysMessage("Tu dois spécifier un pathId ET le displayId du griffon !");
            return false;
        }


        uint32_t taxiId;
        uint32_t displayId;

        std::string taxiIdStr = dataArgs[0];
        std::string displayIdStr = dataArgs[1];

        if (std::all_of(taxiIdStr.begin(), taxiIdStr.end(), ::isdigit) && std::all_of(displayIdStr.begin(), displayIdStr.end(), ::isdigit))
        {
            taxiId = uint32(std::stoul(taxiIdStr));
            displayId = uint32(std::stoul(displayIdStr));

            plr->GryphonCommand(taxiId, displayId);
            return true;
      

        }
        else
            return false;
    }



};
void AddSC_KCommands()
{
    new custom_commandscript();
}


