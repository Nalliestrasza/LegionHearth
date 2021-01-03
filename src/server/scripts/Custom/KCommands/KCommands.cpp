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
        static std::vector<ChatCommand> castCommandTable =
        {
            { "target",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleSetCastTargetCommand,            "" },
        };

        static std::vector<ChatCommand> uncastCommandTable =
        {
            { "target",     rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleUnsetCastTargetCommand,          "" },
        };

        static std::vector<ChatCommand> setCommandTable =
        {
            { "cast",       rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", castCommandTable },
       //   { "name",       rbac::RBAC_PERM_COMMAND_AURA,     false, &HandleSetNameCommand,     "" },
        };

        static std::vector<ChatCommand> unsetCommandTable =
        {
            { "cast",       rbac::RBAC_PERM_COMMAND_AURA,     false, nullptr, "", uncastCommandTable },
        };

        static std::vector<ChatCommand> ticketCommandTable =
        {
            {"create",  rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleCreateTicketCommand,                   "" },
            {"cancel",  rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleCancelTicketCommand,                   "" },
        //  {"close",   rbac::RBAC_PERM_COMMAND_KICK,          false, &HandleCloseTicketCommand,                    "" },
            {"list",    rbac::RBAC_PERM_COMMAND_AURA,          false, &HandleListTicketCommand,                     "" },
        };

        static std::vector<ChatCommand> customCommandTable =
        {

            { "sendfly",		rbac::RBAC_PERM_COMMAND_AURA,       false, &HandleSendFlyCommand,                   "" },
            // CUSTOM
            //{ "database",         rbac::RBAC_PERM_COMMAND_KICK,              true, &HandleDebugDatabase,           "" },
            { "dupecore",         rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleModelCommand,            "" },
            { "coredebug",        rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleArrayCommand,            "" },
            { "custom",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleCustomCommand,           "" },
            { "mount",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMountCommand,            "" },
            { "distance",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleDistanceCommand,         "" },
            { "move",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMoveCommand,             "" },
            { "rand",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRandomCommand,           "" },
            { "randmp",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRandomMPCommand,         "" },
            { "pandaren",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandlePandarenCommand,         "" },
            { "death",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleMortCommand,             "" },
            { "combat",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLutteCommand,            "" },
            { "camouflage",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTraquerCommand,          "" },
            { "lire",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLireCommand,             "" },
            { "ligoter",          rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAttacherCommand,         "" },
            { "ivre",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleExcesCommand,            "" },
            { "livre",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleLivreCommand,            "" },
            { "ombrelle",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleOmbrelleCommand,         "" },
            { "carquois",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleCarquoisCommand,         "" },
            { "sac",              rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSacCommand,              "" },
            { "bondir",           rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleBondirCommand,           "" },
            { "vomir",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleVomirCommand,            "" },
            { "invisible",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleInvisibleCommand,        "" },
            { "sang",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSangCommand,             "" },
            { "nuit",             rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleNuitCommand,             "" },
            { "forgeinfo",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleForgeInfoCommand,        "" },
            { "spellviskit",      rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSpellViskitCommand,      "" },
            { "unspellviskit",    rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnSpellViskitCommand,    "" },
            { "animkit",          rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAnimKitCommand,          "" },
            { "debugsync",        rbac::RBAC_PERM_COMMAND_KICK,             false, &HandleDebugSyncCommand,        "" },
            { "health",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleHealthCommand,           "" },
            { "denied",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleDeniedCommand,           "" },
            { "ticket",           rbac::RBAC_PERM_COMMAND_DAMAGE,           false, nullptr, "", ticketCommandTable },
            { "ticketlist",       rbac::RBAC_PERM_COMMAND_DAMAGE,           false, &HandleTicketListCommand,       "" },
            { "spellvis",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSpellVisCommand,         "" },
            { "unspellvis",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnSpellVisCommand,       "" },
            { "set",              rbac::RBAC_PERM_COMMAND_AURA,             false, nullptr, "", setCommandTable       },
            { "unset",            rbac::RBAC_PERM_COMMAND_AURA,             false, nullptr, "", unsetCommandTable },
            { "invisible",        rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleInvisibleCommand,        "" },
            { "power",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSetPowerCommand,         "" },
            { "hp",               rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleSetHealthCommand,        "" },
            { "regen",            rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleRegenCommand,            "" },
            { "selfunaura",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleUnAuraSelfCommand,       "" }, // For Brikabrok addon
            { "selfaura",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAuraSelfCommand,         "" }, // For Brikabrok addon
            { "addonhelper",      rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleAddonHelper,             "" }, // For Brikabrok and the other
            { "testpacket",       rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTestPacket,              "" },
            { "aurorapacket",     rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleTestPacketAurora,        "" },
            { "freelook",         rbac::RBAC_PERM_COMMAND_AURA,             false, &HandleFreeLook,                "" },
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


    /*
    static bool HandleDebugDatabase(ChatHandler* handler, char const* args) // party
    {
        if (!*args)
            return false;

        char* temp = (char*)args;
        char* min = strtok(temp, "-");
        char* max = strtok(NULL, ";");
        char* rollBaby = strtok(NULL, "\0");
        if (!min || !max || !rollBaby)
            return false;


        int intminimum = atoi(min);
        int intmaximum = atoi(max);
        int introll = atoi(rollBaby);
        uint32 minimum = (uint32)intminimum;
        uint32 maximum = (uint32)intmaximum;
        uint32 roll = (uint32)introll;

        Player* p = handler->GetSession()->GetPlayer();

        if (minimum > maximum || maximum > 10000)
            return false;

        if (roll > maximum || roll < minimum)
            return false;

        WorldPackets::Misc::RandomRoll randomRoll;
        randomRoll.Min = minimum;
        randomRoll.Max = maximum;
        randomRoll.Result = roll;
        randomRoll.Roller = p->GetGUID();
        randomRoll.RollerWowAccount = handler->GetSession()->GetAccountGUID();
        if (p->GetGroup())
            p->GetGroup()->BroadcastPacket(randomRoll.Write(), false);
        else
            handler->GetSession()->SendPacket(randomRoll.Write());

        return true;

    }
    */

    static bool HandleModelCommand(ChatHandler* handler, char const* args)
    {

        Unit* p = handler->GetSession()->GetPlayer();
        Unit* target = handler->getSelectedUnit();

        uint32 spellId = 102284;
        uint32 spellId2 = 111232;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, p->GetMapId(), spellId, p->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, p, target, target->GetMap()->GetDifficultyID());
        }

        if (SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(spellId2, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, p->GetMapId(), spellId2, p->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo2, castId, MAX_EFFECT_MASK, p, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    static bool HandleArrayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Unit* target = handler->getSelectedUnit();

        if (!target)
            return false;

        // creature->TextEmote(args);
        target->Talk(args, CHAT_MSG_SAY, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), nullptr);

        return true;
    }

    static bool HandleCustomCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .custom", handler->GetSession()->GetPlayer()->GetName().c_str());
        uint32 objectId = 999999;
        Player* player = handler->GetSession()->GetPlayer();

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        float x = float(player->GetPositionX());
        float y = float(player->GetPositionY());
        float z = float(player->GetPositionZ());
        float ang = player->GetOrientation();
        //float rot2 = std::sin(ang / 2);
        //float rot3 = std::cos(ang / 2);
        Map* map = player->GetMap();

        uint32 spawntm = 400;

        // Maintenant la commande a besoin d'une variable quaternion au lieu de 4 valeurs.
        G3D::Quat rotation = G3D::Matrix3::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);

        GameObject* object = player->SummonGameObject(objectId, x, y, z, ang, QuaternionData(rotation.x, rotation.y, rotation.z, rotation.w), spawntm);

        player->SummonGameObject(objectId, x, y, z, ang, QuaternionData(rotation.x, rotation.y, rotation.z, rotation.w), spawntm);

        object->DestroyForNearbyPlayers();
        object->UpdateObjectVisibility();

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(110851, handler->GetSession()->GetPlayer()->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, player->GetMapId(), 110851, player->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, player, player, handler->GetSession()->GetPlayer()->GetMap()->GetDifficultyID());
        }

        object->Use(handler->GetSession()->GetPlayer());

        GameObject* go = player->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_BARBER_CHAIR, 0.01f);

        player->SetStandState(UnitStandStateType(UNIT_STAND_STATE_SIT_LOW_CHAIR + go->GetGOInfo()->barberChair.chairheight));

        return true;
    }

    static bool HandleMountCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        Creature* creature = handler->getSelectedCreature();
        Player* player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        if (!target)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .mount", handler->GetSession()->GetPlayer()->GetName().c_str());

        WorldPacket data;
        if (strncmp(args, "self", 3) == 0)
        {

            if (!target || target != creature)
                return false;

            uint32 baseEntry;
            baseEntry = creature->GetDisplayId();
            player->Mount(baseEntry);
            player->SetSpeed(MOVE_WALK, 2);
            player->SetSpeed(MOVE_RUN, 2);
            player->SetSpeed(MOVE_SWIM, 2);
            player->SetSpeed(MOVE_FLIGHT, 2);
            return true;
        }

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        // CreatureModelInfo const* modelInfo = sObjectMgr->GetCreatureModelInfo(newEntry);

        target->Mount(newEntry);

        target->SetSpeed(MOVE_WALK, 2);
        target->SetSpeed(MOVE_RUN, 2);
        target->SetSpeed(MOVE_SWIM, 2);
        //target->SetSpeed(MOVE_TURN,    ASpeed, true);
        target->SetSpeed(MOVE_FLIGHT, 2);

        return true;
    }

    static bool HandleSpellViskitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendPlaySpellVisualKit(newEntry, 2, 0);
        }
        else {
            handler->GetSession()->GetPlayer()->SendPlaySpellVisualKit(newEntry, 2, 0);
        }


        return true;
    }

    static bool HandleUnSpellViskitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        // Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .unspellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {

            target->SendCancelSpellVisualKit(newEntry);
        }
        else {

            handler->GetSession()->GetPlayer()->SendCancelSpellVisualKit(newEntry);
        }

        return true;
    }

    static bool HandleAnimKitCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .animkit", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {

            target->SetAIAnimKitId(newEntry);

        }
        else {

            handler->GetSession()->GetPlayer()->SetAIAnimKitId(newEntry);

        }

        return true;
    }

    static bool HandleDistanceCommand(ChatHandler* handler, char const* args)
    {
        //Phase 1 : recherche du type de la cible : gob, npc ou player ?
        WorldObject* target;
        std::string targetName;
        if (!*args)
        {
            //Partie ciblage classique, si il y a pas d'args
            target = handler->getSelectedObject();
            if (!target)
            {
                handler->SendSysMessage("Vous n avez pas de cible, ni de guid, ni de nom de joueur");
                return false;
            }
            targetName = target->GetName();
        }
        else
        {
            //Partie "joueur"
            std::string nameTargetPlayer = (char*)args;
            target = ObjectAccessor::FindPlayerByName(nameTargetPlayer);
            if (!target)
            {
                //Partie "gob"
                char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
                if (!id)
                    return false;

                ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
                if (!guidLow)
                    return false;

                GameObject* object = NULL;
                if (GameObjectData const* gameObjectData = sObjectMgr->GetGameObjectData(guidLow))
                    object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);

                if (!object)
                {
                    handler->PSendSysMessage("Le guid du gob ou le nom du joueur est invalide");
                    return false;
                }
                targetName = object->GetGOInfo()->name.c_str();
                target = object;
            }
            else
                targetName = target->GetName();
        }
        //Phase 3 : Calcul des positions et distance en m�tres (� deux decimal pres, arrondi � l'inf�rieur )
        double playerX = (trunc((handler->GetSession()->GetPlayer()->GetPositionX()) * 10 * 0.9144)) / 10;
        double playerY = (trunc((handler->GetSession()->GetPlayer()->GetPositionY()) * 10 * 0.9144)) / 10;
        double playerZ = (trunc((handler->GetSession()->GetPlayer()->GetPositionZ()) * 10 * 0.9144)) / 10;
        double targetX = (trunc((target->GetPositionX()) * 10 * 0.9144)) / 10;
        double targetY = (trunc((target->GetPositionY()) * 10 * 0.9144)) / 10;
        double targetZ = (trunc((target->GetPositionZ()) * 10 * 0.9144)) / 10;

        double distanceFull = sqrt((pow((playerX - targetX), 2)) + (pow((playerY - targetY), 2)) + (pow((playerZ - targetZ), 2)));
        double distance = (trunc(distanceFull * 10)) / 10;

        //Phase 4 : Envoi du message
        if (distance > 300)
            handler->SendSysMessage("Vous etes trop loin de votre cible, les mechants devs veulent pas vous dire la distance");
        else if (distance < 1)
            handler->PSendSysMessage("%s est a %2.0f cm de vous", targetName, distance);
        else
            handler->PSendSysMessage("%s est a %3.2f m de vous", targetName, distance);
        return true;
    }

    static bool HandleMoveCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        char* temp = (char*)args;
        char* str1 = strtok(temp, " ");
        char* str2 = strtok(NULL, "\0");
        Player* player = handler->GetSession()->GetPlayer();
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float rot = player->GetOrientation();
        player->SetCanFly(true);
        if (strcmp(str1, "x") == 0)
        {
            if (str2 != NULL)
            {
                x = player->GetPositionX() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else if (strcmp(str1, "y") == 0)
        {
            if (str2 != NULL)
            {
                y = player->GetPositionY() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else if (strcmp(str1, "z") == 0)
        {
            if (str2 != NULL)
            {
                z = player->GetPositionZ() + atof(str2);
            }
            else
            {
                handler->SendSysMessage("Entrees non valides");
                return false;
            }
        }
        else
        {
            float nb = atof(str1);
            x = player->GetPositionX() + (nb * cos((double)rot));
            y = player->GetPositionY() + (nb * sin((double)rot));
        }
        //player->GetMotionMaster()->MovePoint(0, x, y, z);
        //Position pos = Position(x, y, z, rot);
        //player->MovePosition(pos, 50, 0);
        WorldLocation position = WorldLocation(player->GetMapId(), x, y, z, rot);
        player->TeleportTo(position);
        handler->PSendSysMessage("Position : x = %5.3f ; y = %5.3f ; z = %5.3f", x, y, z);
        return true;
    }


    static bool HandleRandomSayCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s a fait un jet de %u (%u-%u) [Rand en /dire]", playerName.c_str(), roll, min, max);
        player->Talk(msg, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
        return true;
    }

    static bool HandleRandomMPCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s a fait un jet de %u (%u-%u) [Rand en priv�", playerName.c_str(), roll, min, max);
        Unit* target = player->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage("Vous n'avez cible personne, ce rand ne sera visible que par vous");
        }
        else if (target->GetTypeId() == TYPEID_PLAYER)
        {
            Player* targetPlayer = player->GetSelectedPlayer();
            ChatHandler(targetPlayer->GetSession()).PSendSysMessage(msg);
        }
        else
        {
            handler->SendSysMessage("Vous n'avez cible personne, ce rand ne sera visible que par vous");
        }
        handler->PSendSysMessage(msg);
        return true;
    }

    static bool HandlePandarenCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!*args)
            return false;

        if (player->getRace() != RACE_PANDAREN_NEUTRAL)
            return true;

        std::string argstr = (char*)args;

        if (argstr == "alliance")
        {
            player->SetRace(RACE_PANDAREN_ALLIANCE);
            player->setFactionForRace(RACE_PANDAREN_ALLIANCE);
            player->SaveToDB();
            player->LearnSpell(108130, false); // Language Pandaren Alliance
            handler->PSendSysMessage("Vous �tes d�sormais un Pandaren de l'alliance !");
        }
        else if (argstr == "horde")
        {
            player->SetRace(RACE_PANDAREN_HORDE);
            player->setFactionForRace(RACE_PANDAREN_HORDE);
            player->SaveToDB();
            player->LearnSpell(108131, false); // Language Pandaren Horde
            handler->PSendSysMessage("Vous �tes d�sormais un Pandaren de la horde !");
        }
        else
        {
            handler->PSendSysMessage("Param�tre incorrect, veuillez entrez horde ou alliance");
        }

        return true;
    }
    static bool HandleMortCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 85267;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }


    // Le .lutte

    static bool HandleLutteCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 94610;
        uint32 spellId2 = 245843;
        uint32 spellId3 = 245853;
        uint32 spellId4 = 245849;
        uint32 spellId5 = 245848;

        switch (target->getClass()) {
        case 8:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId2, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId2, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 5:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId3, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId3, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 7:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId4, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId4, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        case 11:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId5, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId5, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        default:
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }

            return true;
            break;
        }

        return true;
    }

    //Le .traquer

    static bool HandleTraquerCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 80264;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .lire ( PS : H devrait essayer de retirer le timer sur le le spell )


    static bool HandleLireCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 147164;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }
    // Le .attacher

    static bool HandleAttacherCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 93090;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .exces

    static bool HandleExcesCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 80109;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .livre

    static bool HandleLivreCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 124064;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .ombrelle

    static bool HandleOmbrelleCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 131076;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }


    //Le .carquois

    static bool HandleCarquoisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        int choix = atoi(args);
        uint32 spellId;
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        if (!args)
            spellId = 80642;
        switch (choix)
        {
        case 1: spellId = 80642; break;
        case 2: spellId = 125165; break;
        default: handler->SetSentErrorMessage(true); return false;
        }

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    //Le .sac

    static bool HandleSacCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        int choix = atoi(args);
        uint32 spellId;
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        if (!args)
            spellId = 105008;

        switch (choix)
        {
        case 1: spellId = 105008; break;
        case 2: spellId = 104953; break;
        case 3: spellId = 85500; break;
        case 4: spellId = 89695; break;
        case 5: spellId = 88330; break;
        case 6: spellId = 78012; break;
        case 7: spellId = 90085; break;
        case 8: spellId = 84516; break;
        case 9: spellId = 122159; break;
        case 10: spellId = 131732; break;
        case 11: spellId = 79252; break;
        case 12: spellId = 106356; break;
        case 13: spellId = 168026; break;
        case 14: spellId = 163398; break;
        default: handler->SetSentErrorMessage(true); return false;
        }

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    //Le .bondir

    static bool HandleBondirCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;
        uint32 spellId = 55518;
        handler->GetSession()->GetPlayer()->CastSpell(unit, spellId, true);
        return true;
    }

    //Le .vomir

    static bool HandleVomirCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->getSelectedUnit();
        if (!unit)
            return false;
        uint32 spellId = 85234;
        handler->GetSession()->GetPlayer()->CastSpell(unit, spellId, true);
        return true;
    }

    // Le .sang
    static bool HandleSangCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = 169471;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    // Le .invisible
    static bool HandleInvisibleCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();
        else if (target->GetTypeId() == TYPEID_PLAYER && handler->HasLowerSecurity(target->ToPlayer(), ObjectGuid::Empty))
            return false;

        target->SetDisplayId(31515);

        return true;
    }

    static bool HandleNuitCommand(ChatHandler* handler, char const* args)
    {

        std::string argstr = (char*)args;
        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (argstr == "off")
        {
            target->RemoveAura(185394);
            handler->SendSysMessage("Nuit noire d�sactiv�e !");
            return true;
        }
        else if (argstr == "on")
        {
            uint32 spellId = 185394;
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
            {
                ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
                Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
            }
            handler->SendSysMessage("Nuit noire activ�e ! Tapez .nuit off pour la d�sactiv�e.");
            return true;
        }

        return true;
    }

    static bool HandleForgeInfoCommand(ChatHandler* handler, char const* args)
    {
        /*
        Unit* target = handler->getSelectedUnit();

        // Get Textures
        if (!handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_NO_PLAYERS_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 guid = 0;
        uint32 displayId = 0;
        uint32 entryExtra = 0;
        uint32 entryId = 0;
        uint8 skin = 0;
        uint8 face = 0;
        uint8 hair = 0;
        uint8 hcol = 0;
        uint8 pilo = 0;
        uint8 cust1 = 0;
        uint8 cust2 = 0;
        uint8 cust3 = 0;
        uint32 arme1 = 0;
        uint32 arme2 = 0;
        uint32 arme3 = 0;
        uint64 head = 0;
        uint64 shoulders = 0;
        uint64 body = 0;
        uint64 chest = 0;
        uint64 waist = 0;
        uint64 legs = 0;
        uint64 feet = 0;
        uint64 wrists = 0;
        uint64 hands = 0;
        uint64 back = 0;
        uint64 tabard = 0;

        if (target) {
            if (target->IsPlayer()) {
                guid = target->GetGUID().GetCounter();
            }
            else if (target->IsCreature()) {
                displayId = target->GetDisplayId();
                guid = target->GetGUID().GetCounter();
            }
        }
        else {
            guid = handler->GetSession()->GetPlayer()->GetGUID().GetCounter();
        }

        if (!target || target && target->IsPlayer()) {

            //Query
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARSINFO);
            stmt->setUInt64(0, guid);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
                return false;

            Field* fields = result->Fetch();

            skin = fields[0].GetUInt8();
            face = fields[1].GetUInt8();
            hair = fields[2].GetUInt8();
            hcol = fields[3].GetUInt8();
            pilo = fields[4].GetUInt8();
            cust1 = fields[5].GetUInt8();
            cust2 = fields[6].GetUInt8();
            cust3 = fields[7].GetUInt8();

            //Get Equipment
            Player* player = handler->getSelectedPlayerOrSelf();
            static EquipmentSlots const itemSlots[] =
            {
                EQUIPMENT_SLOT_HEAD,
                EQUIPMENT_SLOT_SHOULDERS,
                EQUIPMENT_SLOT_BODY,
                EQUIPMENT_SLOT_CHEST,
                EQUIPMENT_SLOT_WAIST,
                EQUIPMENT_SLOT_LEGS,
                EQUIPMENT_SLOT_FEET,
                EQUIPMENT_SLOT_WRISTS,
                EQUIPMENT_SLOT_HANDS,
                EQUIPMENT_SLOT_BACK,
                EQUIPMENT_SLOT_TABARD,
                EQUIPMENT_SLOT_MAINHAND,
                EQUIPMENT_SLOT_OFFHAND,
            };

            std::vector<uint32> eqqList = std::vector<uint32>();

            // Stolen code from SpellHandler
            for (EquipmentSlots slot : itemSlots)
            {
                uint32 itemDisplayId;
                if (Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    itemDisplayId = item->GetDisplayId(player);


                    if (slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND)
                        itemDisplayId = item->GetEntry();
                }
                else
                    itemDisplayId = 0;


                eqqList.push_back(itemDisplayId);
            }

            // TEXTURES
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_TEXT);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_SKIN, skin);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_FACE, face);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HAIR, hair);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HCOL, hcol);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_PILO, pilo);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH01, cust1);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH02, cust2);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH03, cust3);

            // EQUIPMENTS
            handler->PSendSysMessage(LANG_CUST_SEPARATOR);
            handler->PSendSysMessage(LANG_CUST_HEAD, eqqList[0]);
            handler->PSendSysMessage(LANG_CUST_SHOULDERS, eqqList[1]);
            handler->PSendSysMessage(LANG_CUST_BODY, eqqList[2]);
            handler->PSendSysMessage(LANG_CUST_CHEST, eqqList[3]);
            handler->PSendSysMessage(LANG_CUST_WAIST, eqqList[4]);
            handler->PSendSysMessage(LANG_CUST_LEGS, eqqList[5]);
            handler->PSendSysMessage(LANG_CUST_FEET, eqqList[6]);
            handler->PSendSysMessage(LANG_CUST_WRISTS, eqqList[7]);
            handler->PSendSysMessage(LANG_CUST_HANDS, eqqList[8]);
            handler->PSendSysMessage(LANG_CUST_BACK, eqqList[9]);
            handler->PSendSysMessage(LANG_CUST_TABARD, eqqList[10]);
            handler->PSendSysMessage(LANG_CUST_MAINHAND, eqqList[11]);
            handler->PSendSysMessage(LANG_CUST_OFFHAND, eqqList[12]);

            // WEAPONS
        }
        else if (target->IsCreature()) {

            uint32 eqqList[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            CreatureDisplayInfoEntry const* entry = sCreatureDisplayInfoStore.LookupEntry(displayId);
            if (entry) {
                entryExtra = entry->ExtendedDisplayInfoID;
            }

            if (entryExtra != 0) {

                CreatureDisplayInfoExtraEntry const* extraEntry = sCreatureDisplayInfoExtraStore.LookupEntry(entryExtra);
                if (extraEntry) {
                    skin = extraEntry->SkinID;
                    face = extraEntry->FaceID;
                    hair = extraEntry->HairStyleID;
                    hcol = extraEntry->HairColorID;
                    pilo = extraEntry->FacialHairID;
                    cust1 = extraEntry->CustomDisplayOption[0];
                    cust2 = extraEntry->CustomDisplayOption[1];
                    cust3 = extraEntry->CustomDisplayOption[2];
                }

                for (uint32 id = 204560; id < sNPCModelItemSlotDisplayInfoStore.GetNumRows(); ++id)
                {

                    NPCModelItemSlotDisplayInfoEntry const* armorEntry = sNPCModelItemSlotDisplayInfoStore.LookupEntry(id);
                    if (armorEntry)
                    {
                        if (armorEntry->ExtendedDisplayID == entryExtra)
                        {
                            if (armorEntry->Slot == 0)
                                eqqList[0] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 1)
                                eqqList[1] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 2)
                                eqqList[2] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 3)
                                eqqList[3] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 4)
                                eqqList[4] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 5)
                                eqqList[5] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 6)
                                eqqList[6] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 7)
                                eqqList[7] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 8)
                                eqqList[8] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 9)
                                eqqList[9] = armorEntry->DisplayID;
                            if (armorEntry->Slot == 10)
                                eqqList[10] = armorEntry->DisplayID;
                        }
                    }
                }
            }

            entryId = target->GetEntry();

            arme1 = target->GetVirtualItemId(0);
            arme2 = target->GetVirtualItemId(1);
            arme3 = target->GetVirtualItemId(2);

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme1 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme1) {
                    if (arme1 == appArme1->ItemID) {
                        arme1 = appArme1->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme1 = sItemAppearanceStore.LookupEntry(arme1);
            if (displayArme1) {
                arme1 = displayArme1->ItemDisplayInfoID;
            }

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme2 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme2) {
                    if (arme2 == appArme2->ItemID) {
                        arme2 = appArme2->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme2 = sItemAppearanceStore.LookupEntry(arme2);
            if (displayArme2) {
                arme2 = displayArme2->ItemDisplayInfoID;
            }

            for (uint32 id = 0; id < sItemModifiedAppearanceStore.GetNumRows(); ++id) {
                ItemModifiedAppearanceEntry const* appArme3 = sItemModifiedAppearanceStore.LookupEntry(id);
                if (appArme3) {
                    if (arme3 == appArme3->ItemID) {
                        arme3 = appArme3->ItemAppearanceID;
                    }
                }
            }

            ItemAppearanceEntry const* displayArme3 = sItemAppearanceStore.LookupEntry(arme3);
            if (displayArme3) {
                arme3 = displayArme3->ItemDisplayInfoID;
            }

            // TEXTURES
            handler->PSendSysMessage(LANG_CUST_NPCINFOS_TEXT);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_SKIN, skin);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_FACE, face);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HAIR, hair);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_HCOL, hcol);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_PILO, pilo);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH01, cust1);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH02, cust2);
            handler->PSendSysMessage(LANG_CUST_CHARINFOS_DH03, cust3);

            // EQUIPMENTS
            handler->PSendSysMessage(LANG_CUST_SEPARATOR);
            handler->PSendSysMessage(LANG_CUST_HEAD, eqqList[0]);
            handler->PSendSysMessage(LANG_CUST_SHOULDERS, eqqList[1]);
            handler->PSendSysMessage(LANG_CUST_BODY, eqqList[2]);
            handler->PSendSysMessage(LANG_CUST_CHEST, eqqList[3]);
            handler->PSendSysMessage(LANG_CUST_WAIST, eqqList[4]);
            handler->PSendSysMessage(LANG_CUST_LEGS, eqqList[5]);
            handler->PSendSysMessage(LANG_CUST_FEET, eqqList[6]);
            handler->PSendSysMessage(LANG_CUST_WRISTS, eqqList[7]);
            handler->PSendSysMessage(LANG_CUST_HANDS, eqqList[8]);
            handler->PSendSysMessage(LANG_CUST_BACK, eqqList[10]);
            handler->PSendSysMessage(LANG_CUST_TABARD, eqqList[9]);
            handler->PSendSysMessage(LANG_CUST_PRIMARYHAND, arme1);
            handler->PSendSysMessage(LANG_CUST_SECONDARYHAND, arme2);
            handler->PSendSysMessage(LANG_CUST_TERTIARYHAND, arme3);

        }
        */

        return true;
    }

    static bool HandleDebugSyncCommand(ChatHandler* handler, const char* args) //Cmd � retest
    {
        char* temp = (char*)args;
        char* str1 = strtok(temp, "-");
        char* str2 = strtok(NULL, ";");
        char* str3 = strtok(NULL, "\0");
        int minStr = 1;
        int maxStr = 100;
        int master = 0;
        if (str1 != NULL)
        {
            minStr = atoi(str1);
        }
        if (str2 != NULL)
        {
            maxStr = atoi(str2);
        }
        else
        {
            maxStr = minStr;
            minStr = 1;
        }
        if (str3 != NULL && handler->GetSession()->GetSecurity() >= 3)
        {
            master = atoi(str3);
        }

        if (minStr <= 0 || minStr > maxStr || maxStr > 9999 || master > maxStr)
        {
            handler->SendSysMessage("Entrees non valides");
            return false;
        }
        uint32 min = (uint32)minStr;
        uint32 max = (uint32)maxStr;
        uint32 roll;
        if (master == 0)
        {
            roll = urand(min, max);
        }
        else
        {
            roll = (uint32)master;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string playerName = player->GetName();
        char msg[255];
        sprintf(msg, "%s obtient un %u (%u-%u)", playerName.c_str(), roll, min, max);
        (msg, CHAT_MSG_PARTY, CHAT_MSG_RAID, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
        return true;
    }

    static bool HandleDebugDatabase(ChatHandler* handler, char const* args) // party
    {
        if (!*args)
            return false;

        char* temp = (char*)args;
        char* min = strtok(temp, "-");
        char* max = strtok(NULL, ";");
        char* rollBaby = strtok(NULL, "\0");
        if (!min || !max || !rollBaby)
            return false;


        int intminimum = atoi(min);
        int intmaximum = atoi(max);
        int introll = atoi(rollBaby);
        uint32 minimum = (uint32)intminimum;
        uint32 maximum = (uint32)intmaximum;
        uint32 roll = (uint32)introll;

        Player* p = handler->GetSession()->GetPlayer();

        if (minimum > maximum || maximum > 10000)
            return false;

        if (roll > maximum || roll < minimum)
            return false;

        /*
        char* rollChar;
        sprintf(rollChar, "%s obtient un %u (%u-%u).", p->GetName().c_str(), roll, minimum, maximum);
        std::string rollStr(rollChar);
        if (p->GetGroup())
        {
            WorldPackets::Chat::Chat packet;
            packet.Initialize(ChatMsg(CHAT_MSG_SYSTEM), Language(LANG_UNIVERSAL), p, nullptr, rollStr);
            p->GetGroup()->BroadcastPacket(packet.Write(), false);
        }
        else
        {
            handler->SendSysMessage("%s", rollChar);
        }
        */

        /*WorldPackets::Misc::RandomRoll const randomRoll;
        randomRoll.Min = minimum;
        randomRoll.Max = maximum;
        randomRoll.Result = roll;
        randomRoll.Roller = p->GetGUID();
        randomRoll.RollerWowAccount = handler->GetSession()->GetAccountGUID();*/

        return true;

    }



    static bool CheckModifyResources(ChatHandler* handler, const char* args, Player* target, int32& res, int8 const multiplier = 1)
    {
        if (!*args)
            return false;

        res = atoi((char*)args) * multiplier;

        if (res < 1)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        return true;
    }

    static bool HandleHealthCommand(ChatHandler* handler, const char* args)
    {

        if (!*args)
            return false;

        char const* health = strtok((char*)args, " ");

        int32 hp = 0;
        int32 lp = 0;

        Player* target = handler->getSelectedPlayerOrSelf();

        hp = atoi(health);
        lp = target->GetHealth() + hp;
        if (lp >= target->GetMaxHealth()) {
            target->SetHealth(target->GetMaxHealth());
            return true;
        }
        else if (lp <= target->GetMaxHealth()) {
            target->SetHealth(lp);
            return true;
        }

        return true;

    }


    static bool HandleDeniedCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;


        char const* aId = strtok((char*)args, " ");
        char const* pId = strtok(NULL, " ");


        if (!aId || !pId)
            return false;


        uint32 accountId = uint32(atoi(aId));
        uint32 permission = uint32(atoi(pId));

        QueryResult permCheck = LoginDatabase.PQuery("SELECT permissionId FROM rbac_account_permissions WHERE permissionId = %u AND accountId = %u ", permission, accountId);
        if (permCheck)
        {
            handler->PSendSysMessage(LANG_DENIED_ERROR, accountId, permission);
        }

        else
        {
            LoginDatabasePreparedStatement* addPerm = LoginDatabase.GetPreparedStatement(LOGIN_INS_DENIED_PERMISSION);
            addPerm->setUInt32(0, accountId);
            addPerm->setUInt32(1, permission);
            LoginDatabase.Execute(addPerm);

            sAccountMgr->LoadRBAC();
            sWorld->ReloadRBAC();
            handler->SendGlobalGMSysMessage("RBAC data reloaded.");

            handler->PSendSysMessage(LANG_DENIED_SUCCESSFULL, accountId, permission);
        }


        return true;

    }

    static bool HandleCreateTicketCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* msg = (char*)args;
        std::string playerName = handler->GetSession()->GetPlayer()->GetName();



        // auto increment test 
        QueryResult lastId = WorldDatabase.PQuery("SELECT MAX(ticketId) from ticket");
        Field* field = lastId->Fetch();
        uint32 tId = field[0].GetUInt32();
        ++tId;


        WorldDatabasePreparedStatement* sendTicket = WorldDatabase.GetPreparedStatement(WORLD_INS_NEW_TICKET);
        sendTicket->setUInt32(0, tId);
        sendTicket->setString(1, msg);
        sendTicket->setString(2, playerName.c_str());
        sendTicket->setInt64(3, handler->GetSession()->GetAccountId());
        sendTicket->setInt64(4, handler->GetSession()->GetPlayer()->GetGUID().GetCounter());
        WorldDatabase.Execute(sendTicket);


        sWorld->SendGMText(LANG_TICKET_SEND_GM, tId, handler->GetSession()->GetAccountId(), handler->GetSession()->GetPlayer()->GetGUID().GetCounter(), playerName.c_str());
        sWorld->SendGMText(LANG_TICKET_SEND_GM_CONTENT, tId, msg);

        return true;

    }

    static bool HandleTicketListCommand(ChatHandler* handler, const char* args)
    {
        QueryResult listTicket = WorldDatabase.PQuery("SELECT ticketId, ticketContents, ticketOwner from ticket WHERE ticketStatus = 0");
        Field* field = listTicket->Fetch();
        uint32 ticketId = field[0].GetUInt32();
        std::string ticketMsg = field[1].GetString();
        std::string ticketOwner = field[2].GetString();

        sWorld->SendGMText(LANG_TICKET_LIST, ticketId, ticketMsg, ticketOwner);

        return true;
    }


    static bool HandleSpellVisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendPlaySpellVisual(target->GetPosition(), target->GetOrientation(), newEntry, 0, 0, 5000, false);
        }
        else {
            handler->GetSession()->GetPlayer()->SendPlaySpellVisual(target->GetPosition(), target->GetOrientation(), newEntry, 0, 0, 5000, false);
        }


        return true;
    }

    static bool HandleUnSpellVisCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->getSelectedUnit();
        //Player* target = handler->GetSession()->GetPlayer(); // Only self

        if (!*args)
            return false;

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .spellvis", handler->GetSession()->GetPlayer()->GetName().c_str());

        const char* entry;
        entry = strtok((char*)args, " ");

        const uint32 newEntry = uint32(atoi(entry));

        if (target) {
            target->SendCancelSpellVisual(newEntry);
        }
        else {
            handler->GetSession()->GetPlayer()->SendCancelSpellVisual(newEntry);
        }


        return true;
    }

    static bool HandleSetCastTargetCommand(ChatHandler* handler, char const* args) {

        if (!args)
            return false;

        Unit* target = handler->getSelectedUnit();

        char const* cible = strtok((char*)args, " ");
        char const* spell = strtok(NULL, " ");

        if (!cible)
            return false;
        if (!spell)
            return false;

        WorldObject* object = nullptr;
        ObjectGuid::LowType guid = UI64LIT(0);
        uint32 spellId = 0;
        std::string checkIsValid = cible;
        std::string checkSpell = spell;

        if (std::all_of(checkIsValid.begin(), checkIsValid.end(), ::isdigit) && std::all_of(checkSpell.begin(), checkSpell.end(), ::isdigit)) {

            guid = atoull(cible);
            spellId = atoi(spell);

        }
        else {

            return false;

        }

        object = handler->GetCreatureFromPlayerMapByDbGuid(guid);
        if (!object) {
            handler->PSendSysMessage(LANG_CAST_TARGET_NO_CREATURE_ON_MAP, guid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target && target->IsCreature()) {

            target->ToCreature()->SetTarget(object->GetGUID());
            target->ToCreature()->CastSpell(object->ToUnit(), spellId);
        }
        else if (target && !target->IsCreature()) {
            handler->PSendSysMessage(LANG_CAST_TARGET_NOT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .set cast target", handler->GetSession()->GetPlayer()->GetName().c_str());


        return true;

    }

    static bool HandleUnsetCastTargetCommand(ChatHandler* handler, char const* args) {

        Unit* target = handler->getSelectedUnit();

        TC_LOG_DEBUG("chat.log.whisper", "Negre de %s fait un .unset cast target", handler->GetSession()->GetPlayer()->GetName().c_str());

        if (target && target->IsCreature()) {

            target->ToCreature()->SendClearTarget();
            target->ToCreature()->CastStop();

        }
        else if (target && !target->IsCreature()) {
            handler->PSendSysMessage(LANG_CAST_TARGET_NOT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    /*static bool HandleSetNameCommand(ChatHandler* handler, char const* args)
    {

        if (!args)
            return false;

        uint8 compteur = 0;
        uint8 iter = 0;
        std::string newName;
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        int32 alphabetASCII[103] = { 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100,
            101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56,
            57, 233, 232, 45, 231, 224, 249, 181, 44, 226, 234, 251, 238, 244, 228, 235, 252, 239, 246, 230, 339, 46, 192, 194, 196, 198, 199, 200, 201, 202,
            203, 206, 207, 212, 214, 219, 220, 338, 34, 145, 146, 39 };

        char const* newNameStr = strtok((char*)args, " ");
        char const* newNameStr2 = strtok(NULL, " ");
        char const* newNameStr3 = strtok(NULL, " ");
        char const* newNameStr4 = strtok(NULL, " ");

        if (!newNameStr)
            return false;

        std::string name;
        std::string name2;
        std::string name3;
        std::string name4;
        std::stringstream ss;

        if (newNameStr) {
            name = newNameStr;
            std::wstring wname;
            Utf8toWStr(name, wname);
            for (int32 i = 0; i < wname.size(); ++i) {
                wchar_t a = wname[i];
                int32 b = int32(a);
                ++iter;
                for (int32 j = 0; j < 103; ++j) {
                    int32 d = alphabetASCII[j];
                    if (b == d) {
                        ++compteur;
                    }
                }
            }
            if (compteur == iter) {
                compteur = 0;
                iter = 0;
                ss << newNameStr;
                newName = ss.str().c_str();
                if (newNameStr2) {
                    name2 = newNameStr2;
                    std::wstring wname2;
                    Utf8toWStr(name2, wname2);
                    for (int32 i = 0; i < wname2.size(); ++i) {
                        wchar_t a = wname2[i];
                        int32 b = int32(a);
                        ++iter;
                        for (int32 j = 0; j < 103; ++j) {
                            int32 d = alphabetASCII[j];
                            if (b == d) {
                                ++compteur;
                            }
                        }
                    }
                    if (compteur == iter) {
                        compteur = 0;
                        iter = 0;
                        ss << "_" << newNameStr2;
                        newName = ss.str().c_str();
                        if (newNameStr3) {
                            name3 = newNameStr3;
                            std::wstring wname3;
                            Utf8toWStr(name3, wname3);
                            for (int32 i = 0; i < wname3.size(); ++i) {
                                wchar_t a = wname3[i];
                                int32 b = int32(a);
                                ++iter;
                                for (int32 j = 0; j < 103; ++j) {
                                    int32 d = alphabetASCII[j];
                                    if (b == d) {
                                        ++compteur;
                                    }
                                }
                            }
                            if (compteur == iter) {
                                compteur = 0;
                                iter = 0;
                                ss << "_" << newNameStr3;
                                newName = ss.str().c_str();
                                if (newNameStr4) {
                                    name4 = newNameStr4;
                                    std::wstring wname4;
                                    Utf8toWStr(name4, wname4);
                                    for (int32 i = 0; i < wname4.size(); ++i) {
                                        wchar_t a = wname4[i];
                                        int32 b = int32(a);
                                        ++iter;
                                        for (int32 j = 0; j < 103; ++j) {
                                            int32 d = alphabetASCII[j];
                                            if (b == d) {
                                                ++compteur;
                                            }
                                        }
                                    }
                                    if (compteur == iter) {
                                        compteur = 0;
                                        iter = 0;
                                        ss << "_" << newNameStr4;
                                        newName = ss.str().c_str();
                                    }
                                    else {
                                        compteur = 0;
                                        iter = 0;
                                        handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                                        handler->SetSentErrorMessage(true);
                                        return false;
                                    }
                                }
                                else {
                                    newName == ss.str().c_str();
                                }
                            }
                            else {
                                compteur = 0;
                                iter = 0;
                                handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                                handler->SetSentErrorMessage(true);
                                return false;
                            }
                        }
                        else {
                            newName = ss.str().c_str();
                        }
                    }
                    else {
                        compteur = 0;
                        iter = 0;
                        handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }
                else {
                    newName == ss.str().c_str();
                }
            }
            else {
                compteur = 0;
                iter = 0;
                handler->PSendSysMessage(LANG_NAME_NOT_GOOD);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!normalizePlayerName(newName))
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (ObjectMgr::CheckPlayerName(newName, handler->GetSession()->GetPlayer() ? handler->GetSession()->GetSessionDbcLocale() : sWorld->GetDefaultDbcLocale(), true) != CHAR_NAME_SUCCESS)
            {
                handler->PSendSysMessage(LANG_NAME_TOO_LONG);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (WorldSession* session = handler->GetSession())
            {
                if (!session->HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RESERVEDNAME) && sObjectMgr->IsReservedName(newName))
                {
                    handler->SendSysMessage(LANG_RESERVED_NAME);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            stmt->setString(0, newName);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                handler->PSendSysMessage(LANG_RENAME_PLAYER_ALREADY_EXISTS, newName.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            // Remove declined name from db
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
            stmt->setUInt64(0, playerGuid.GetCounter());
            CharacterDatabase.Execute(stmt);


            handler->GetSession()->GetPlayer()->SetName(newName);

            handler->PSendSysMessage(LANG_NAME_GOOD, newName.c_str());

            if (WorldSession* session = handler->GetSession())
                session->KickPlayer();


            sWorld->UpdateCharacterInfo(playerGuid, newName);

            handler->PSendSysMessage(LANG_RENAME_PLAYER_WITH_NEW_NAME, handler->GetSession()->GetPlayerName().c_str(), newName.c_str());

            if (WorldSession* session = handler->GetSession())
            {
                if (Player* player = session->GetPlayer())
                    sLog->outCommand(session->GetAccountId(), "GM %s (Account: %u) forced rename %s to player %s (Account: %u)", player->GetName().c_str(), session->GetAccountId(), newName.c_str(), handler->GetSession()->GetPlayerName().c_str(), ObjectMgr::GetPlayerAccountIdByGUID(playerGuid));
            }
            else
                sLog->outCommand(0, "CONSOLE forced rename '%s' to '%s' (%s)", handler->GetSession()->GetPlayerName().c_str(), newName.c_str(), playerGuid.ToString().c_str());

        }

        return true;
    }*/

    static bool HandleCancelTicketCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char const* tId = strtok((char*)args, " ");
        uint32 ticketId = uint32(atoi(tId));
        std::string playerName = handler->GetSession()->GetPlayer()->GetName();


        // check if the ticket exist in the database
        QueryResult checkExist = WorldDatabase.PQuery("SELECT ticketId, ticketStatus FROM ticket WHERE ticketId = %u", ticketId);
        Field* checkStatus = checkExist->Fetch();
        uint8 status = checkStatus[1].GetUInt8();

        if (status == 1)
        {
            handler->SendSysMessage(LANG_TICKET_ALREADY_CANCEL);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!checkExist)
        {
            handler->SendSysMessage(LANG_TICKET_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;

        }
        else
        {
            QueryResult checkOwn = WorldDatabase.PQuery("SELECT ticketOwnerAccId FROM ticket WHERE ticketId = %u", ticketId);
            Field* field = checkOwn->Fetch();
            uint32 accId = field[0].GetUInt32();

            if (accId == handler->GetSession()->GetAccountId())
            {
                WorldDatabasePreparedStatement* updateTicket = WorldDatabase.GetPreparedStatement(WORLD_UPD_CANCEL_TICKET);
                updateTicket->setUInt32(0, ticketId);
                WorldDatabase.Execute(updateTicket);

                handler->PSendSysMessage(LANG_TICKET_SEND_NOTFI_CANCEL);
                sWorld->SendGMText(LANG_TICKET_CANCEL_BY_PLAYERS, playerName.c_str(), handler->GetSession()->GetAccountId());

            }
            else
            {
                handler->SendSysMessage(LANG_TICKET_NOTFOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

        }

        return true;

    }


    static bool HandleListTicketCommand(ChatHandler* handler, const char* args)
    {

        if (*args)
            return false;

        if (handler->GetSession()->GetSecurity() >= 2)
        {
            QueryResult query = WorldDatabase.PQuery("SELECT ticketId, ticketContents, ticketOwner, ticketOwnerAccId, ticketOwnerGuid FROM ticket WHERE ticketStatus = 0");

            if (query) {

                do {

                    Field* result = query->Fetch();

                    uint32 id = result[0].GetUInt32();
                    std::string ticketContents = result[1].GetString();
                    std::string ticketOwner = result[2].GetString();
                    uint32 ticketAccId = result[3].GetUInt32();
                    uint32 ticketOwnerGuid = result[4].GetUInt32();


                    handler->PSendSysMessage(LANG_TICKET_RESULT_GM_QUERY, id, ticketContents.c_str(), ticketOwner.c_str(), ticketAccId, ticketOwnerGuid);

                } while (query->NextRow());
            }
            else {
                handler->PSendSysMessage(LANG_TICKET_RESULT_NONE_GM);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            QueryResult queryP = WorldDatabase.PQuery("SELECT ticketId, ticketContents FROM ticket WHERE ticketOwnerAccId = %u AND ticketStatus = 0", handler->GetSession()->GetAccountId());


            if (queryP) {

                do {

                    Field* pResult = queryP->Fetch();

                    uint32 id = pResult[0].GetUInt32();
                    std::string ticketContents = pResult[1].GetString();

                    handler->PSendSysMessage(LANG_TICKET_RESULT_PLAYER_QUERY, id, ticketContents.c_str());

                } while (queryP->NextRow());
            }
            else {
                handler->PSendSysMessage(LANG_TICKET_RESULT_NONE_PLAYER);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }


        return true;

    }

    /*
    *  Check if the argument is correct for a rand
    *
    *  @pre: str, then argument for rand
    *        validArg other character than digit accepted
    *		 bool digitExpect if it expect digit for the first value in str (defaut false)
    *        index, the length of argument already checked (default 0)
    *
    *  @post: true, str must null or be digit(s) or
    * its characters (if it have) != digits is in validArg and have minimum a digit between them
    * and its characters respect partially the order of validArg
    *         false, else
    */
    static bool checkRandParsing(std::string str, std::string validArg, bool digitExpected = false, int index = 0)
    {
        if (str.size() == index)
        {
            if (!digitExpected)
                return true;
            else
                return false;
        }
        else
        {
            char arg = str.at(index);
            size_t pos = validArg.find(arg);

            if (isdigit(arg))
                return checkRandParsing(str, validArg, false, index + 1);
            else if (pos != std::string::npos && !digitExpected)
                return checkRandParsing(str, validArg.substr(pos + 1, std::string::npos), true, index + 1);
            else
                return false;
        }
    }


    /*
    *   Get position of the first character (!= digit) from a string (str)
    *   who is made up by digits and, possibly, characters
    *   @pre : str, string to check
    *          pos, length of str already checked (default 0)
    *
    *   @post: pos, the position of the first character that is not a digit
    *          std::string::npos if there is no character != digit
    */
    static size_t getPositionFirstChar(std::string str, size_t pos = 0)
    {
        if (pos == str.size())
            return std::string::npos;
        else if (isdigit(str.at(pos)))
            return getPositionFirstChar(str, pos + 1);
        else
            return pos;
    }


    /*
    *   Get position of the first digit from a string (str)
    *   who is made up by digits and, possibly, characters
    *   @pre : str, string to check
    *          pos, length of str already checked (default 0)
    *
    *   @post: pos, the position of the first digit
    *          std::string::npos if no digit
    */
    static size_t getPositionFirstDigit(std::string str, size_t pos = 0)
    {
        if (pos == str.size())
            return std::string::npos;
        else if (!isdigit(str.at(pos)))
            return getPositionFirstDigit(str, pos + 1);
        else
            return pos;
    }

    /*
    *   Parse a roll argument from a random command to get all values needed
    *   @pre :   arg, argument from a rand command who's respect checkParseRand conditions
    *            min, minimum roll
    *            max, maximum roll
    *            dice, number of rolls
    *            bonus, bonus after roll
    *
    *   @post :  min, max, dice, bonus changed according to args
    *            true if numbers used respected basics conditions of a roll
    *            false if min > max && dice > 30 && max >= 10000 && bonus >= 1000
    */
    static bool parseRand(std::string str, int& min, int& max, int& dice, int& bonus, int& expected)
    {
        size_t pos;
        pos = str.find("d");
        if (pos != std::string::npos)
        {
            dice = atoi(str.substr(0, pos).c_str());
            str.erase(0, pos + 1);
        }

        pos = str.find("-");

        if (pos != std::string::npos)
        {
            min = atoi(str.substr(0, pos).c_str());
            str.erase(0, pos + 1);
        }

        pos = getPositionFirstChar(str); // Get max from arg if it exists

        if (str.size() != 0 && pos != 0)
            max = atoi(str.substr(0, pos).c_str());

        pos = str.find("+");

        if (pos != std::string::npos)
        {
            str.erase(0, pos);
            pos = str.find(";");

            if (pos != std::string::npos)
            {
                bonus = atoi(str.substr(0, pos).c_str());
                str.erase(0, pos + 1);
                expected = atoi(str.c_str());
            }
            else
            {
                expected = min;
                bonus = atoi(str.c_str());
            }
        }

        if (min > max || dice >= 30 || max >= 10000 || bonus >= 1000 || expected > max)
            return false;
        else
            return true;
    }

    /*
    *   Create sentence according to parameters for a roll command argument
    *   @pre : playerName, the name of player
    *          args, argument from a roll command
    *
    *   @post : "Entr�es invalides" if args does not respect checkRandParsing conditions
    *and min > max && dice > 30 && max >= 10000 && bonus >= 1000
    *           else, the sentence to send for a roll command.
    */
    static std::string interpretRand(ChatHandler* handler, std::string str, std::string nameRoll = std::string(""))
    {
        std::string playerName = handler->GetSession()->GetPlayer()->GetName();
        std::string result = std::string("[CHECK_ROLL_ERROR] Entrees invalides");
        int min = 1, max = 100, dice = 0, bonus = 0, randResult = 0, expected = min;

        if (!checkRandParsing(str, std::string("d-+;")))
            return result;

        if (!parseRand(str, min, max, dice, bonus, expected))
            return result;

        if (handler->GetSession()->GetSecurity() < 3 && expected > min)
            return result;

        result = playerName;
        result.append(" a fait ");

        if (dice < 2)
        {
            if (nameRoll.size() > 0)
            {
                result.append("un jet de |cffffffff< ");
                result.append(nameRoll);
                result.append(" >|r ");
            }
            else
            {
                result.append("un jet de ");
            }

            if (expected <= min)
                randResult = urand((uint32)min, (uint32)max);
            else
                randResult = urand((uint32)expected, (uint32)max);

            result.append(std::to_string(randResult));
        }
        else
        {
            int tmp;
            result.append(std::to_string(dice));

            if (nameRoll.size() > 0)
            {
                result.append(" jets de |cffffffff< ");
                result.append(nameRoll);
                result.append(" >|r: ");
            }
            else
            {
                result.append(" jets : ");
            }

            do
            {
                if (expected <= min)
                    tmp = urand((uint32)min, (uint32)max);
                else
                    tmp = urand((uint32)expected, (uint32)max);

                randResult += tmp;
                result.append("[");
                result.append(std::to_string(tmp));
                result.append("]");
                dice--;

                if (dice != 0)
                    result.append(" + ");
                else if (bonus == 0)
                    result.append(" = ");
                else
                    result.append("  ");

            } while (dice > 0);

            result.append(std::to_string(randResult));
        }

        if (bonus > 0)
        {
            result.append("+");
            result.append(std::to_string(bonus));
            result.append(" = ");
            result.append(std::to_string(randResult + bonus));
            result.append(" !");
        }
        result.append(" (");
        result.append(std::to_string(min));
        result.append("-");
        result.append(std::to_string(max));
        result.append(")");
        return result;
    }

    /*
    *   Format and spread a message from player to a range depends on the type chosen.
    *
    *   @pre : player, Player who's spread the message
    *          str, the message
    *          type, type of spread (i.e : party, say, etc.)
    *          limit, number of 255 characters is used to limit spam
    *          maxLimit, the number maximum of messages (1 msg = 255 characters).
    *
    */
    static void formatAndSpread(Player* player, std::string str, int type, int maxSpread = 1, int limit = 0)
    {

        if (str.size() > maxSpread * 255 || limit >= maxSpread)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("[SPAM_DETECTED] Arret de la commande");
            return;
        }

        if (str.size() > 255)
        {
            int limitNeeded = spread(player, str.substr(0, 254), type, limit);
            formatAndSpread(player, str.substr(255, std::string::npos), type, maxSpread, limitNeeded);
        }
        else
        {
            spread(player, str.substr(0, 254), type, limit);
        }

    }

    /*
    *   Spread a message from player to a range depends on the type chosen.
    *
    *   @pre : player, Player who's spread the message
    *          str, the message ( < 255 characters)
    *          type, type of spread (i.e : party, say, etc.)
    *          limit, number of 255 characters is used to limit spam
    *
    *   @post: the limit used after spread the message.
    */
    static int spread(Player* player, std::string str, int type, int limit = 0)
    {

        switch (type)
        {
        case CHAT_MSG_SAY:
        {
            player->Talk(str.c_str(), CHAT_MSG_SYSTEM, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
            break;
        }
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        {
            Group* group = player->GetGroup();

            if (!group)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("%s", str.c_str());
                break;
            }

            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* receiver = itr->GetSource();
                if (!receiver || receiver->GetGroup() != group)
                    continue;

                if (receiver->GetSession())
                    ChatHandler(receiver->GetSession()).PSendSysMessage("%s", str.c_str());

            }

            break;

        }
        case CHAT_MSG_GUILD: // Idea : by GuildID => checkConnectedPlayer
        default:
        {
            ChatHandler(player->GetSession()).PSendSysMessage("%s", str.c_str());
            break;
        }
        }

        return limit + 1;
    }

    /*
    * Extract all information from a roll command
    * @pre : str the argument from a roll command
    *        typeSpread, nameRoll, rollToParse : informations to extract
    *
    * @post : true always for the moment (in the future it will be useful)
    *         + informations extracted if there exist in str
    */
    static bool parseRandCommandInfo(std::string str, int& typeSpread, std::string& nameRoll, std::string& rollToParse)
    {
        if (str.size() == 0)
            return true;

        size_t pos = getPositionFirstDigit(str); // extract rollToParse

        if (pos == 0)
        {
            rollToParse = str;
            return true;
        }
        else if (pos != std::string::npos)
        {
            rollToParse = str.substr(pos, std::string::npos);
            str.erase(pos, std::string::npos);
        }

        pos = str.find(" ");

        if (pos != std::string::npos) // Can be spread and nameRoll, at this point we are not sure
        {
            std::string spreadStr = str.substr(0, pos);

            if (spreadStr.find("say") == 0 || spreadStr.find("dire") == 0)
            {
                typeSpread = CHAT_MSG_SAY;
                str.erase(0, pos + 1);
                nameRoll = str;
                str.erase(0, std::string::npos);
            }
            else
            {
                typeSpread = CHAT_MSG_PARTY;
                nameRoll = str;
                str.erase(0, std::string::npos);
            }
        }
        else if (str.find("say") == 0 || str.find("dire") == 0) // Only spread none nameRoll
        {
            typeSpread = CHAT_MSG_SAY;
            str.erase(0, std::string::npos);
        }

        if (str.size() > 0) // Only nameRoll
            nameRoll = str;

        if (nameRoll.size() > 0)
        {
            if (nameRoll.find(" ") == 0)  // Suppress useless first char if it's space
                nameRoll.erase(0, 1);

            if (nameRoll.back() == ' ') // Suppress useless last char if it's space
                nameRoll = nameRoll.substr(0, nameRoll.size() - 1);
        }

        return true;
    }

    /*
    *   Main command for a rand
    *   @pre : handler, args..I don't really need explain theses
    *
    *   @post : the answer of "Do you think Helnesis is planning to invade Poland ?".
    */
    static bool HandleRandomCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        std::string str = std::string(args);
        std::string nameRoll, rollToParse;
        int type = CHAT_MSG_PARTY;

        if (!parseRandCommandInfo(str, type, nameRoll, rollToParse))
        {
            handler->SendSysMessage("[RAND_PARSEINFO_ERROR] Entrees invalides");
            return true;
        }

        str = interpretRand(handler, rollToParse, nameRoll);

        if (str.compare("Entrees invalides") == 0)
        {
            handler->PSendSysMessage("%s", str.c_str());
            return true;
        }

        formatAndSpread(player, str, type);
        return true;
    }
    /*
    *   Base on type of power, get its name
    *
    *   @pre : type, the type of power
    *
    *   @post : the name of the power, if not found : power
    */
    static std::string getPowerName(int type)
    {
        std::string typeName;
        switch (type)
        {
        case POWER_MANA: typeName.append("mana"); break;
        case POWER_ENERGY: typeName.append("energie"); break;
        case POWER_RAGE: typeName.append("rage"); break;
        case POWER_FURY: typeName.append("furie"); break;
        case POWER_RUNIC_POWER: typeName.append("pouvoir runique"); break;
        case POWER_FOCUS: typeName.append("focus"); break;
        case POWER_CHI: typeName.append("chi"); break;
        default: typeName.append("power"); break;
        };
        return typeName;
    }
    /*
    *   Parse value to set, the value can be a fix value or add/minus to current
    *   @pre : str, string to parse
    *          current, current value
    *          max, max value to reach
    *
    *   @post : value = max if value > max
    *           value = 0 if value < 0
    */
    static int parseValueToSet(std::string str, int current, int max)
    {
        bool add = false, minus = false;

        if (str.size() > 1 && str.find("-") != std::string::npos)
        {
            minus = true;
            str.erase(0, 1);
        }
        else if (str.size() > 1 && str.find("+") != std::string::npos)
        {
            add = true;
            str.erase(0, 1);
        }

        size_t pos = getPositionFirstChar(str);

        if (pos != std::string::npos)
            return current;

        int32 value;

        if (str.size() > 6)
            value = max;
        else
            value = stoi(str);

        if ((minus && current - value <= 0) || value < 0)
            value = 0;
        else if (minus)
            value = current - value;
        else if (add && current + value >= max || value >= max)
            value = max;
        else if (add)
            value = current + value;
        return value;
    }

    static bool HandleSetPowerCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        std::string str = std::string(args);
        Player* player = handler->GetSession()->GetPlayer();
        Player* target = handler->getSelectedPlayerOrSelf();
        Powers type = target->GetPowerType();
        int value = parseValueToSet(str, target->GetPower(type), target->GetMaxPower(type));

        if (value == target->GetPower(type))
            return false;

        target->SetPower(type, value);
        target->CustomSetRegen(false); // Stop all regen, even HP

        if (type == POWER_MANA)
        {
            target->UpdateManaRegen();
        }

        handler->PSendSysMessage("Points de mana de %s : %d / %u", target->GetName().c_str(), value, target->GetMaxPower(type));
        ChatHandler(target->GetSession()).PSendSysMessage("%s change vos points de mana : %d / %u", player->GetName().c_str(), value, target->GetMaxPower(type));
        return true;
    }

    static bool HandleSetHealthCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        Unit* target = player->GetSelectedUnit();
        if (!target)
            target = player;

        std::string str = std::string(args);
        int value = parseValueToSet(str, target->GetHealth(), target->GetMaxHealth());

        if (value == target->GetHealth())
        {
            handler->SendSysMessage("Cela ne change rien");
            return true;
        }
        else if (value <= 0)
        {
            handler->SendSysMessage("Vous ne pouvez pas tuer avec cette commande");
            return true;
        }

        target->SetHealth(value);

        if (target->IsCreature())
        {
            //target->ToCreature()->setRegeneratingHealth(false);
            handler->PSendSysMessage("Points de vie de %s : %d / %u", target->GetName().c_str(), value, target->GetMaxHealth());
            return true;
        }

        Player* receiver = target->ToPlayer();
        receiver->CustomSetRegen(false); // Stop all regen, even power

        if (receiver->GetPowerType() == POWER_MANA)
        {
            receiver->UpdateManaRegen();
        }

        handler->PSendSysMessage("Points de vie de %s : %d / %u", target->GetName().c_str(), value, receiver->GetMaxHealth());
        ChatHandler(receiver->GetSession()).PSendSysMessage("%s change vos points de vie : %d / %u", player->GetName().c_str(), value, receiver->GetMaxHealth());
        return true;
    }

    static bool HandleRegenCommand(ChatHandler* handler, const char* args)
    {
        bool isRegen;
        Player* player = handler->GetSession()->GetPlayer();
        bool hasMana = player->GetPowerType() == POWER_MANA;

        if (!*args)
            return false;
        else
        {
            std::string str = std::string(args);

            if (str.find("on") != std::string::npos)
                isRegen = true;
            else if (str.find("off") != std::string::npos)
                isRegen = false;
            else
                return false;
        }

        if (isRegen)
        {
            player->CustomSetRegen(true);

            if (hasMana)
                player->UpdateManaRegen();

            handler->SendSysMessage("Regeneration ON");
        }
        else
        {
            player->CustomSetRegen(false);

            if (hasMana)
            {
                player->UpdateManaRegen();
            }

            handler->SendSysMessage("Regeneration OFF");
        }

        return true;
    }


    static bool HandleUnAuraSelfCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->GetSession()->GetPlayer();

        std::string argstr = args;
        if (argstr == "all")
        {
            target->RemoveAllAuras();
            return true;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        target->RemoveAurasDueToSpell(spellId);

        return true;
    }

    static bool HandleAuraSelfCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSession()->GetPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId, target->GetMap()->GetDifficultyID()))
        {
            ObjectGuid castId = ObjectGuid::Create<HighGuid::Cast>(SPELL_CAST_SOURCE_NORMAL, target->GetMapId(), spellId, target->GetMap()->GenerateLowGuid<HighGuid::Cast>());
            Aura::TryRefreshStackOrCreate(spellInfo, castId, MAX_EFFECT_MASK, target, target, target->GetMap()->GetDifficultyID());
        }

        return true;
    }

    /*
    * ADDON HELPER
    */
    static std::queue<std::string> parseParameters(const char* args) {
        std::string str = std::string(args);
        std::queue<std::string> q;
        std::regex reg("([^ ]+)");
        std::sregex_iterator currentMatch(str.begin(), str.end(), reg);
        std::smatch match = *currentMatch;
        std::sregex_iterator lastMatch;


        while (currentMatch != lastMatch) {
            match = *currentMatch;
            q.push(match.str());
            currentMatch++;
        }

        return q;
    }

    static bool brikabrokGobPosInfo(ChatHandler* handler, std::queue<std::string> q) {
        if (q.empty())
            return false;

        std::string guidStr = q.front();
        q.pop();

        if (getPositionFirstDigit(guidStr) == std::string::npos)
            return false;

        ObjectGuid::LowType guidLow = std::stoi(guidStr);
        if (!guidLow)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);

        if (!object) {
            handler->SendSysMessage("Guid invalid");
            return false;
        }

        const GameObjectData* data = sObjectMgr->GetGameObjectData(guidLow);
        if (!data)
            return false;

        float yaw, pitch, roll;
        data->rotation.toEulerAnglesZYX(yaw, pitch, roll);
        handler->PSendSysMessage("%s %f %f %f %f %f %f %f %f", object->GetName().c_str(), object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation(), yaw * (180 / M_PI), pitch * (180 / M_PI), roll * (180 / M_PI), data->size);
        return true;
    }

    static bool brikabrokGobPosAdd(ChatHandler* handler, std::queue<std::string> q)
    {
        if (q.empty())
            return false;

        if (!handler->GetSession()->HasPhasePermission(handler->GetSession()->GetPlayer()->GetMapId(), PhaseChat::Permissions::Gameobjects_Create)) {
            handler->PSendSysMessage("You don't have the permission to do that");
            return false;
        }

        uint32 gobEntry;
        float xF;
        float yF;
        float zF;
        float oF;
        float yawF;
        float pitchF;
        float rollF;
        float sizeF;

        gobEntry = atoul(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        xF = (float)atof(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        yF = (float)atof(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        zF = (float)atof(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        oF = (float)atof(q.front().c_str());
        q.pop();

        if (q.empty())
            return false;

        yawF = ((float)atof(q.front().c_str())) * (M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        pitchF = ((float)atof(q.front().c_str())) * (M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        rollF = ((float)atof(q.front().c_str())) * (M_PI / 180);
        q.pop();

        if (q.empty())
            return false;

        sizeF = (float)atof(q.front().c_str());
        q.pop();

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(gobEntry);
        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, gobEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            // report to DB errors log as in loading case
            TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", gobEntry, objectInfo->type, objectInfo->displayId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_HAVE_INVALID_DATA, gobEntry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();

        GameObject* object = GameObject::CreateGameObject(objectInfo->entry, map, Position(xF, yF, zF, oF), QuaternionData::fromEulerAnglesZYX(yawF, pitchF, rollF), 255, GO_STATE_READY);

        if (!object)
            return false;

        PhasingHandler::InheritPhaseShift(object, player);

        // fill the gameobject data and save to the db
        // on r�cup le scale 
        object->SetObjectScale(sizeF);
        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), object->GetOrientation());
        object->SaveToDB(player->GetMap()->GetId(), { player->GetMap()->GetDifficultyID() });

        ObjectGuid::LowType spawnId = object->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        // this will generate a new guid if the object is in an instance
        object = GameObject::CreateGameObjectFromDB(spawnId, map);
        if (!object)
            return false;

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGameObjectData(spawnId)));

        // Log
        uint32 spawnerAccountId = player->GetSession()->GetAccountId();
        uint64 spawnerGuid = player->GetSession()->GetPlayer()->GetGUID().GetCounter();

        WorldDatabasePreparedStatement* gobInfo = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT_LOG);
        gobInfo->setUInt64(0, spawnId);
        gobInfo->setUInt32(1, spawnerAccountId);
        gobInfo->setUInt64(2, spawnerGuid);
        WorldDatabase.Execute(gobInfo);

        return true;
    }

    static bool brikabrok(ChatHandler* handler, std::queue<std::string> q) {
        if (q.empty()) {
            handler->SendSysMessage("No method called");
            return false;
        }

        std::string methodCalled = q.front();
        q.pop();

        if (methodCalled.compare("gobpos") == 0)
            return brikabrokGobPosInfo(handler, q);
        else if (methodCalled.compare("gobaddxyz") == 0)
            return brikabrokGobPosAdd(handler, q);
        else
            return false;
    }

    static bool HandleAddonHelper(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        std::string params = std::string(args);
        std::queue<std::string> q = parseParameters(args);

        if (q.empty())
            return false;

        std::string nameAddon = q.front();
        q.pop();

        if (nameAddon.compare("brikabrok") == 0)
            return brikabrok(handler, q);
        else
            return false;
    }

    static bool HandleTestPacket(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        std::string _args = args;

        WorldPacket data(SMSG_DISPLAY_GAME_ERROR, 9);
        data << 0;
        data << _args;
        data << 200;
        handler->GetSession()->SendPacket(&data, true);

    }

    static bool HandleTestPacketAurora(ChatHandler* handler, const char* args)
    {
        WorldPacket data(SMSG_AURORA_TEST, 1);
        handler->GetSession()->SendPacket(&data, true);
    }

    // will use that for the gob addon since it allows to move freely without collisions ...
    static bool HandleFreeLook(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            if (handler->GetSession()->GetPlayer()->HasPlayerFlag(PLAYER_FLAGS_UBER) && handler->GetSession()->GetPlayer()->HasPlayerFlag(PLAYER_FLAGS_COMMENTATOR2))
                handler->GetSession()->SendNotification("Mode libre (ON)");
            else
                handler->GetSession()->SendNotification("Mode libre (OFF)");
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->AddPlayerFlag(PLAYER_FLAGS_UBER);
            handler->GetSession()->GetPlayer()->AddPlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
            handler->GetSession()->SendNotification("Mode libre (ON)");
            return true;
        }
        else if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->RemovePlayerFlag(PLAYER_FLAGS_UBER);
            handler->GetSession()->GetPlayer()->RemovePlayerFlag(PLAYER_FLAGS_COMMENTATOR2);
            handler->GetSession()->SendNotification("Mode libre (OFF)");
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);

    }




    /*
    * END OF ADDON HELPER
    */

    /*
    static bool HandleSendTaxiCommand(ChatHandler* handler, const char* args)
    {
        //if (!*args)
          //  return false;

      //  uint32 uMountDisplay = atoul(args);
        Player* target = handler->getSelectedPlayer();

        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }



        target->GetSession()->SendActivateTaxiReply(ERR_TAXIOK);
        target->GetSession()->SendDoFlight(22234, 113);

    }
    */

};
void AddSC_KCommands()
{
    new custom_commandscript();
}


