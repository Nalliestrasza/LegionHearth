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

 /* ScriptData
 Name: character_commandscript
 %Complete: 100
 Comment: All character related commands
 Category: commandscripts
 EndScriptData */

#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerDump.h"
#include "ReputationMgr.h"
#include "World.h"
#include "WorldSession.h"
#include <sstream>

class character_commandscript : public CommandScript
{
public:
    character_commandscript() : CommandScript("character_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> pdumpCommandTable =
        {
            { "load",          rbac::RBAC_PERM_COMMAND_PDUMP_LOAD,                true,  &HandlePDumpLoadCommand,               "" },
            { "write",         rbac::RBAC_PERM_COMMAND_PDUMP_WRITE,               true,  &HandlePDumpWriteCommand,              "" },
        };
        static std::vector<ChatCommand> characterDeletedCommandTable =
        {
            { "delete",        rbac::RBAC_PERM_COMMAND_CHARACTER_DELETED_DELETE,  true,  &HandleCharacterDeletedDeleteCommand,  "" },
            { "list",          rbac::RBAC_PERM_COMMAND_CHARACTER_DELETED_LIST,    true,  &HandleCharacterDeletedListCommand,    "" },
            { "restore",       rbac::RBAC_PERM_COMMAND_CHARACTER_DELETED_RESTORE, true,  &HandleCharacterDeletedRestoreCommand, "" },
            { "old",           rbac::RBAC_PERM_COMMAND_CHARACTER_DELETED_OLD,     true,  &HandleCharacterDeletedOldCommand,     "" },
            { "race",          rbac::RBAC_PERM_COMMAND_CHARACTER_CUSTOMIZE,       true,  &HandleCharacterSetRaceCommand,        "" },
            { "classe",        rbac::RBAC_PERM_COMMAND_CHARACTER_CUSTOMIZE,       true,  &HandleCharacterSetClassCommand,       "" },
        };

        static std::vector<ChatCommand> characterCommandTable =
        {
            { "customize",     rbac::RBAC_PERM_COMMAND_CHARACTER_CUSTOMIZE,       true,  &HandleCharacterCustomizeCommand,      "", },
            { "set",           rbac::RBAC_PERM_COMMAND_CHARACTER_CUSTOMIZE,       true,  NULL,                                  "", characterDeletedCommandTable },
            { "changefaction", rbac::RBAC_PERM_COMMAND_CHARACTER_CHANGEFACTION,   true,  &HandleCharacterChangeFactionCommand,  "", },
            { "changerace",    rbac::RBAC_PERM_COMMAND_CHARACTER_CHANGERACE,      true,  &HandleCharacterChangeRaceCommand,     "", },
            { "changeaccount", rbac::RBAC_PERM_COMMAND_CHARACTER_CHANGEACCOUNT,   true,  &HandleCharacterChangeAccountCommand,  "", },
            { "deleted",       rbac::RBAC_PERM_COMMAND_CHARACTER_DELETED,         true,  nullptr,                               "", characterDeletedCommandTable },
            { "erase",         rbac::RBAC_PERM_COMMAND_CHARACTER_ERASE,           true,  &HandleCharacterEraseCommand,          "", },
            { "level",         rbac::RBAC_PERM_COMMAND_CHARACTER_LEVEL,           true,  &HandleCharacterLevelCommand,          "", },
            { "rename",        rbac::RBAC_PERM_COMMAND_CHARACTER_RENAME,          true,  &HandleCharacterRenameCommand,         "", },
            { "reputation",    rbac::RBAC_PERM_COMMAND_CHARACTER_REPUTATION,      true,  &HandleCharacterReputationCommand,     "", },
            { "titles",        rbac::RBAC_PERM_COMMAND_CHARACTER_TITLES,          true,  &HandleCharacterTitlesCommand,         "", },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "character",     rbac::RBAC_PERM_COMMAND_CHARACTER,                 true,  nullptr,                               "", characterCommandTable },
            { "levelup",       rbac::RBAC_PERM_COMMAND_LEVELUP,                   false, &HandleLevelUpCommand,                 "" },
            { "pdump",         rbac::RBAC_PERM_COMMAND_PDUMP,                     true,  nullptr,                               "", pdumpCommandTable },
        };
        return commandTable;
    }

    // Stores informations about a deleted character
    struct DeletedInfo
    {
        ObjectGuid  guid;                               ///< the GUID from the character
        std::string name;                               ///< the character name
        uint32      accountId;                          ///< the account id
        std::string accountName;                        ///< the account name
        time_t      deleteDate;                         ///< the date at which the character has been deleted
    };

    typedef std::list<DeletedInfo> DeletedInfoList;

    /**
    * Collects all GUIDs (and related info) from deleted characters which are still in the database.
    *
    * @param foundList    a reference to an std::list which will be filled with info data
    * @param searchString the search string which either contains a player GUID or a part fo the character-name
    * @return             returns false if there was a problem while selecting the characters (e.g. player name not normalizeable)
    */
    static bool GetDeletedCharacterInfoList(DeletedInfoList& foundList, std::string searchString)
    {
        PreparedQueryResult result;
        CharacterDatabasePreparedStatement* stmt;
        if (!searchString.empty())
        {
            // search by GUID
            if (isNumeric(searchString.c_str()))
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_DEL_INFO_BY_GUID);
                stmt->setUInt64(0, strtoull(searchString.c_str(), nullptr, 10));
                result = CharacterDatabase.Query(stmt);
            }
            // search by name
            else
            {
                if (!normalizePlayerName(searchString))
                    return false;

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_DEL_INFO_BY_NAME);
                stmt->setString(0, searchString);
                result = CharacterDatabase.Query(stmt);
            }
        }
        else
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_DEL_INFO);
            result = CharacterDatabase.Query(stmt);
        }

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();

                DeletedInfo info;

                info.guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
                info.name = fields[1].GetString();
                info.accountId = fields[2].GetUInt32();

                // account name will be empty for nonexisting account
                AccountMgr::GetName(info.accountId, info.accountName);
                info.deleteDate = time_t(fields[3].GetUInt32());
                foundList.push_back(info);
            } while (result->NextRow());
        }

        return true;
    }

    /**
    * Shows all deleted characters which matches the given search string, expected non empty list
    *
    * @see HandleCharacterDeletedListCommand
    * @see HandleCharacterDeletedRestoreCommand
    * @see HandleCharacterDeletedDeleteCommand
    * @see DeletedInfoList
    *
    * @param foundList contains a list with all found deleted characters
    */
    static void HandleCharacterDeletedListHelper(DeletedInfoList const& foundList, ChatHandler* handler)
    {
        if (!handler->GetSession())
        {
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_BAR);
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_HEADER);
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_BAR);
        }

        for (DeletedInfoList::const_iterator itr = foundList.begin(); itr != foundList.end(); ++itr)
        {
            std::string dateStr = TimeToTimestampStr(itr->deleteDate);

            if (!handler->GetSession())
                handler->PSendSysMessage(LANG_CHARACTER_DELETED_LIST_LINE_CONSOLE,
                    itr->guid.ToString().c_str(), itr->name.c_str(), itr->accountName.empty() ? "<Not existing>" : itr->accountName.c_str(),
                    itr->accountId, dateStr.c_str());
            else
                handler->PSendSysMessage(LANG_CHARACTER_DELETED_LIST_LINE_CHAT,
                    itr->guid.ToString().c_str(), itr->name.c_str(), itr->accountName.empty() ? "<Not existing>" : itr->accountName.c_str(),
                    itr->accountId, dateStr.c_str());
        }

        if (!handler->GetSession())
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_BAR);
    }

    /**
    * Restore a previously deleted character
    *
    * @see HandleCharacterDeletedListHelper
    * @see HandleCharacterDeletedRestoreCommand
    * @see HandleCharacterDeletedDeleteCommand
    * @see DeletedInfoList
    *
    * @param delInfo the informations about the character which will be restored
    */
    static void HandleCharacterDeletedRestoreHelper(DeletedInfo const& delInfo, ChatHandler* handler)
    {
        if (delInfo.accountName.empty())                    // account does not exist
        {
            handler->PSendSysMessage(LANG_CHARACTER_DELETED_SKIP_ACCOUNT, delInfo.name.c_str(), delInfo.guid.ToString().c_str(), delInfo.accountId);
            return;
        }

        // check character count
        uint32 charcount = AccountMgr::GetCharactersCount(delInfo.accountId);
        if (charcount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        {
            handler->PSendSysMessage(LANG_CHARACTER_DELETED_SKIP_FULL, delInfo.name.c_str(), delInfo.guid.ToString().c_str(), delInfo.accountId);
            return;
        }

        if (!sCharacterCache->GetCharacterGuidByName(delInfo.name).IsEmpty())
        {
            handler->PSendSysMessage(LANG_CHARACTER_DELETED_SKIP_NAME, delInfo.name.c_str(), delInfo.guid.ToString().c_str(), delInfo.accountId);
            return;
        }

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_RESTORE_DELETE_INFO);
        stmt->setString(0, delInfo.name);
        stmt->setUInt32(1, delInfo.accountId);
        stmt->setUInt64(2, delInfo.guid.GetCounter());
        CharacterDatabase.Execute(stmt);

        sCharacterCache->UpdateCharacterInfoDeleted(delInfo.guid, false, &delInfo.name);
    }

    static void HandleCharacterLevel(Player* player, ObjectGuid playerGuid, uint32 oldLevel, uint32 newLevel, ChatHandler* handler)
    {
        if (player)
        {
            player->GiveLevel(newLevel);
            player->InitTalentForLevel();
            player->SetXP(0);

            if (handler->needReportToTarget(player))
            {
                if (oldLevel == newLevel)
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_YOURS_LEVEL_PROGRESS_RESET, handler->GetNameLink().c_str());
                else if (oldLevel < newLevel)
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_YOURS_LEVEL_UP, handler->GetNameLink().c_str(), newLevel);
                else                                                // if (oldlevel > newlevel)
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_YOURS_LEVEL_DOWN, handler->GetNameLink().c_str(), newLevel);
            }
        }
        else
        {
            // Update level and reset XP, everything else will be updated at login
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_LEVEL);
            stmt->setUInt8(0, uint8(newLevel));
            stmt->setUInt64(1, playerGuid.GetCounter());
            CharacterDatabase.Execute(stmt);
        }
    }

    static bool HandleCharacterTitlesCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        LocaleConstant loc = handler->GetSessionDbcLocale();
        char const* targetName = target->GetName().c_str();
        char const* knownStr = handler->GetTrinityString(LANG_KNOWN);

        // Search in CharTitles.dbc
        for (uint32 id = 0; id < sCharTitlesStore.GetNumRows(); id++)
        {
            CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id);

            if (titleInfo && target->HasTitle(titleInfo))
            {
                std::string name = (target->getGender() == GENDER_MALE ? titleInfo->Name : titleInfo->Name1)[handler->GetSessionDbcLocale()];
                if (name.empty())
                    continue;

                char const* activeStr = *target->m_playerData->PlayerTitle == titleInfo->MaskID
                ? handler->GetTrinityString(LANG_ACTIVE)
                : "";

                std::string titleNameStr = Trinity::StringFormat(name.c_str(), targetName);

                // send title in "id (idx:idx) - [namedlink locale]" format
                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_TITLE_LIST_CHAT, id, titleInfo->MaskID, id, titleNameStr.c_str(), localeNames[loc], knownStr, activeStr);
                else
                    handler->PSendSysMessage(LANG_TITLE_LIST_CONSOLE, id, titleInfo->MaskID, name.c_str(), localeNames[loc], knownStr, activeStr);
            }
        }

        return true;
    }

    //rename characters
    static bool HandleCharacterRenameCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        char const* newNameStr = strtok(nullptr, " ");

        if (newNameStr)
        {
            std::string playerOldName;
            std::string newName = newNameStr;

            if (target)
            {
                // check online security
                if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                    return false;

                playerOldName = target->GetName();
            }
            else
            {
                // check offline security
                if (handler->HasLowerSecurity(nullptr, targetGuid))
                    return false;

                sCharacterCache->GetCharacterNameByGuid(targetGuid, playerOldName);
            }

            if (!normalizePlayerName(newName))
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (ObjectMgr::CheckPlayerName(newName, target ? target->GetSession()->GetSessionDbcLocale() : sWorld->GetDefaultDbcLocale(), true) != CHAR_NAME_SUCCESS)
            {
                handler->SendSysMessage(LANG_BAD_VALUE);
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

            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
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
            stmt->setUInt64(0, targetGuid.GetCounter());
            CharacterDatabase.Execute(stmt);

            if (target)
            {
                target->SetName(newName);

                if (WorldSession* session = target->GetSession())
                    session->KickPlayer();
            }
            else
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_NAME_BY_GUID);
                stmt->setString(0, newName);
                stmt->setUInt64(1, targetGuid.GetCounter());
                CharacterDatabase.Execute(stmt);
            }

            sCharacterCache->UpdateCharacterData(targetGuid, newName);

            handler->PSendSysMessage(LANG_RENAME_PLAYER_WITH_NEW_NAME, playerOldName.c_str(), newName.c_str());

            if (WorldSession* session = handler->GetSession())
            {
                if (Player* player = session->GetPlayer())
                    sLog->outCommand(session->GetAccountId(), "GM %s (Account: %u) forced rename %s to player %s (Account: %u)", player->GetName().c_str(), session->GetAccountId(), newName.c_str(), playerOldName.c_str(), sCharacterCache->GetCharacterAccountIdByGuid(targetGuid));
            }
            else
                sLog->outCommand(0, "CONSOLE forced rename '%s' to '%s' (%s)", playerOldName.c_str(), newName.c_str(), targetGuid.ToString().c_str());
        }
        else
        {
            if (target)
            {
                // check online security
                if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                    return false;

                handler->PSendSysMessage(LANG_RENAME_PLAYER, handler->GetNameLink(target).c_str());
                target->SetAtLoginFlag(AT_LOGIN_RENAME);
            }
            else
            {
                // check offline security
                if (handler->HasLowerSecurity(nullptr, targetGuid))
                    return false;

                std::string oldNameLink = handler->playerLink(targetName);
                handler->PSendSysMessage(LANG_RENAME_PLAYER_GUID, oldNameLink.c_str(), targetGuid.ToString().c_str());

                CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
                stmt->setUInt16(0, uint16(AT_LOGIN_RENAME));
                stmt->setUInt64(1, targetGuid.GetCounter());
                CharacterDatabase.Execute(stmt);
            }
        }

        return true;
    }

    static bool HandleCharacterLevelCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* levelStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &levelStr);
        if (!levelStr)
            return false;

        // exception opt second arg: .character level $name
        if (isalpha(levelStr[0]))
        {
            nameStr = levelStr;
            levelStr = nullptr;                                    // current level will used
        }

        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &targetName))
            return false;

        int32 oldlevel = target ? target->getLevel() : sCharacterCache->GetCharacterLevelByGuid(targetGuid);
        int32 newlevel = levelStr ? atoi(levelStr) : oldlevel;

        if (newlevel < 1)
            return false;                                       // invalid level

        if (newlevel > STRONG_MAX_LEVEL)                         // hardcoded maximum level
            newlevel = STRONG_MAX_LEVEL;

        HandleCharacterLevel(target, targetGuid, oldlevel, newlevel, handler);
        if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)      // including player == NULL
        {
            std::string nameLink = handler->playerLink(targetName);
            handler->PSendSysMessage(LANG_YOU_CHANGE_LVL, nameLink.c_str(), newlevel);
        }

        return true;
    }

    // customize characters
    static bool HandleCharacterCustomizeCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CUSTOMIZE));
		
        if (target)
        {
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER, handler->GetNameLink(target).c_str());
            target->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
            stmt->setUInt64(1, target->GetGUID().GetCounter());
        }
        else
        {
            std::string oldNameLink = handler->playerLink(targetName);
            stmt->setUInt64(1, targetGuid.GetCounter());
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER_GUID, oldNameLink.c_str(), targetGuid.ToString().c_str());
        }
        CharacterDatabase.Execute(stmt);

        return true;
    }

    static bool HandleCharacterChangeFactionCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;

        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        if (target->getRace() == RACE_PANDAREN_NEUTRAL || target->getRace() == RACE_PANDAREN_ALLIANCE || target->getRace() == RACE_PANDAREN_HORDE)
            return false;

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_FACTION));
        if (target)
        {
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER, handler->GetNameLink(target).c_str());
            target->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
            stmt->setUInt64(1, target->GetGUID().GetCounter());
        }
        else
        {
            std::string oldNameLink = handler->playerLink(targetName);
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER_GUID, oldNameLink.c_str(), targetGuid.ToString().c_str());
            stmt->setUInt64(1, targetGuid.GetCounter());
        }
        CharacterDatabase.Execute(stmt);

        return true;
    }

    static bool HandleCharacterChangeRaceCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_RACE));
		
        if (target)
        {
            /// @todo add text into database
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER, handler->GetNameLink(target).c_str());
            target->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
            stmt->setUInt64(1, target->GetGUID().GetCounter());
        }
        else
        {
            std::string oldNameLink = handler->playerLink(targetName);
            /// @todo add text into database
            handler->PSendSysMessage(LANG_CUSTOMIZE_PLAYER_GUID, oldNameLink.c_str(), targetGuid.ToString().c_str());
            stmt->setUInt64(1, targetGuid.GetCounter());
        }
        CharacterDatabase.Execute(stmt);

        return true;
    }

    static bool HandleCharacterChangeAccountCommand(ChatHandler* handler, char const* args)
    {
        char* playerNameStr;
        char* accountNameStr;
        handler->extractOptFirstArg(const_cast<char*>(args), &playerNameStr, &accountNameStr);
        if (!accountNameStr)
            return false;

        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget(playerNameStr, nullptr, &targetGuid, &targetName))
            return false;

        CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(targetGuid);
        if (!characterInfo)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 oldAccountId = characterInfo->AccountId;
        uint32 newAccountId = oldAccountId;

        std::string accountName(accountNameStr);
        if (!Utf8ToUpperOnlyLatin(accountName))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME);
        stmt->setString(0, accountName);
        if (PreparedQueryResult result = LoginDatabase.Query(stmt))
            newAccountId = (*result)[0].GetUInt32();
        else
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // nothing to do :)
        if (newAccountId == oldAccountId)
            return true;

        if (uint32 charCount = AccountMgr::GetCharactersCount(newAccountId))
        {
            if (charCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
            {
                handler->PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL, accountName.c_str(), newAccountId);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        CharacterDatabasePreparedStatement* charStmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ACCOUNT_BY_GUID);
        charStmt->setUInt32(0, newAccountId);
        charStmt->setUInt32(1, targetGuid.GetCounter());
        CharacterDatabase.DirectExecute(charStmt);

        sWorld->UpdateRealmCharCount(oldAccountId);
        sWorld->UpdateRealmCharCount(newAccountId);

        sCharacterCache->UpdateCharacterAccountId(targetGuid, newAccountId);

        handler->PSendSysMessage(LANG_CHANGEACCOUNT_SUCCESS, targetName.c_str(), accountName.c_str());

        std::string logString = Trinity::StringFormat("changed ownership of player %s (%s) from account %u to account %u", targetName.c_str(), targetGuid.ToString().c_str(), oldAccountId, newAccountId);
        if (WorldSession* session = handler->GetSession())
        {
            if (Player* player = session->GetPlayer())
                sLog->outCommand(session->GetAccountId(), "GM %s (Account: %u) %s", player->GetName().c_str(), session->GetAccountId(), logString.c_str());
        }
        else
            sLog->outCommand(0, "%s %s", handler->GetTrinityString(LANG_CONSOLE), logString.c_str());
        return true;
    }

    static bool HandleCharacterReputationCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        LocaleConstant loc = handler->GetSessionDbcLocale();

        FactionStateList const& targetFSL = target->GetReputationMgr().GetStateList();
        for (FactionStateList::const_iterator itr = targetFSL.begin(); itr != targetFSL.end(); ++itr)
        {
            FactionState const& faction = itr->second;
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction.ID);
            char const* factionName = factionEntry ? factionEntry->Name[loc] : "#Not found#";
            ReputationRank rank = target->GetReputationMgr().GetRank(factionEntry);
            std::string rankName = handler->GetTrinityString(ReputationRankStrIndex[rank]);
            std::ostringstream ss;
            if (handler->GetSession())
                ss << faction.ID << " - |cffffffff|Hfaction:" << faction.ID << "|h[" << factionName << ' ' << localeNames[loc] << "]|h|r";
            else
                ss << faction.ID << " - " << factionName << ' ' << localeNames[loc];

            ss << ' ' << rankName << " (" << target->GetReputationMgr().GetReputation(factionEntry) << ')';

            if (faction.Flags & FACTION_FLAG_VISIBLE)
                ss << handler->GetTrinityString(LANG_FACTION_VISIBLE);
            if (faction.Flags & FACTION_FLAG_AT_WAR)
                ss << handler->GetTrinityString(LANG_FACTION_ATWAR);
            if (faction.Flags & FACTION_FLAG_PEACE_FORCED)
                ss << handler->GetTrinityString(LANG_FACTION_PEACE_FORCED);
            if (faction.Flags & FACTION_FLAG_HIDDEN)
                ss << handler->GetTrinityString(LANG_FACTION_HIDDEN);
            if (faction.Flags & FACTION_FLAG_INVISIBLE_FORCED)
                ss << handler->GetTrinityString(LANG_FACTION_INVISIBLE_FORCED);
            if (faction.Flags & FACTION_FLAG_INACTIVE)
                ss << handler->GetTrinityString(LANG_FACTION_INACTIVE);

            handler->SendSysMessage(ss.str().c_str());
        }

        return true;
    }

    /**
     * Handles the '.character deleted list' command, which shows all deleted characters which matches the given search string
     *
     * @see HandleCharacterDeletedListHelper
     * @see HandleCharacterDeletedRestoreCommand
     * @see HandleCharacterDeletedDeleteCommand
     * @see DeletedInfoList
     *
     * @param args the search string which either contains a player GUID or a part fo the character-name
     */
    static bool HandleCharacterDeletedListCommand(ChatHandler* handler, char const* args)
    {
        DeletedInfoList foundList;
        if (!GetDeletedCharacterInfoList(foundList, args))
            return false;

        // if no characters have been found, output a warning
        if (foundList.empty())
        {
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_EMPTY);
            return false;
        }

        HandleCharacterDeletedListHelper(foundList, handler);

        return true;
    }

    /**
     * Handles the '.character deleted restore' command, which restores all deleted characters which matches the given search string
     *
     * The command automatically calls '.character deleted list' command with the search string to show all restored characters.
     *
     * @see HandleCharacterDeletedRestoreHelper
     * @see HandleCharacterDeletedListCommand
     * @see HandleCharacterDeletedDeleteCommand
     *
     * @param args the search string which either contains a player GUID or a part of the character-name
     */
    static bool HandleCharacterDeletedRestoreCommand(ChatHandler* handler, char const* args)
    {
        // It is required to submit at least one argument
        if (!*args)
            return false;

        std::string searchString;
        std::string newCharName;
        uint32 newAccount = 0;

        // GCC by some strange reason fail build code without temporary variable
        std::istringstream params(args);
        params >> searchString >> newCharName >> newAccount;

        DeletedInfoList foundList;
        if (!GetDeletedCharacterInfoList(foundList, searchString))
            return false;

        if (foundList.empty())
        {
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_EMPTY);
            return false;
        }

        handler->SendSysMessage(LANG_CHARACTER_DELETED_RESTORE);
        HandleCharacterDeletedListHelper(foundList, handler);

        if (newCharName.empty())
        {
            // Drop nonexisting account cases
            for (DeletedInfoList::iterator itr = foundList.begin(); itr != foundList.end(); ++itr)
                HandleCharacterDeletedRestoreHelper(*itr, handler);
        }
        else if (foundList.size() == 1 && normalizePlayerName(newCharName))
        {
            DeletedInfo delInfo = foundList.front();

            // update name
            delInfo.name = newCharName;

            // if new account provided update deleted info
            if (newAccount && newAccount != delInfo.accountId)
            {
                delInfo.accountId = newAccount;
                AccountMgr::GetName(newAccount, delInfo.accountName);
            }

            HandleCharacterDeletedRestoreHelper(delInfo, handler);
        }
        else
            handler->SendSysMessage(LANG_CHARACTER_DELETED_ERR_RENAME);

        return true;
    }

    /**
     * Handles the '.character deleted delete' command, which completely deletes all deleted characters which matches the given search string
     *
     * @see Player::GetDeletedCharacterGUIDs
     * @see Player::DeleteFromDB
     * @see HandleCharacterDeletedListCommand
     * @see HandleCharacterDeletedRestoreCommand
     *
     * @param args the search string which either contains a player GUID or a part fo the character-name
     */
    static bool HandleCharacterDeletedDeleteCommand(ChatHandler* handler, char const* args)
    {
        // It is required to submit at least one argument
        if (!*args)
            return false;

        DeletedInfoList foundList;
        if (!GetDeletedCharacterInfoList(foundList, args))
            return false;

        if (foundList.empty())
        {
            handler->SendSysMessage(LANG_CHARACTER_DELETED_LIST_EMPTY);
            return false;
        }

        handler->SendSysMessage(LANG_CHARACTER_DELETED_DELETE);
        HandleCharacterDeletedListHelper(foundList, handler);

        // Call the appropriate function to delete them (current account for deleted characters is 0)
        for (DeletedInfoList::const_iterator itr = foundList.begin(); itr != foundList.end(); ++itr)
            Player::DeleteFromDB(itr->guid, 0, false, true);

        return true;
    }

    /**
     * Handles the '.character deleted old' command, which completely deletes all deleted characters deleted with some days ago
     *
     * @see Player::DeleteOldCharacters
     * @see Player::DeleteFromDB
     * @see HandleCharacterDeletedDeleteCommand
     * @see HandleCharacterDeletedListCommand
     * @see HandleCharacterDeletedRestoreCommand
     *
     * @param args the search string which either contains a player GUID or a part fo the character-name
     */
    static bool HandleCharacterDeletedOldCommand(ChatHandler* /*handler*/, char const* args)
    {
        int32 keepDays = sWorld->getIntConfig(CONFIG_CHARDELETE_KEEP_DAYS);

        char* daysStr = strtok((char*)args, " ");
        if (daysStr)
        {
            if (!isNumeric(daysStr))
                return false;

            keepDays = atoi(daysStr);
            if (keepDays < 0)
                return false;
        }
        // config option value 0 -> disabled and can't be used
        else if (keepDays <= 0)
            return false;

        Player::DeleteOldCharacters(uint32(keepDays));

        return true;
    }

    static bool HandleCharacterEraseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* characterName_str = strtok((char*)args, " ");
        if (!characterName_str)
            return false;

        std::string characterName = characterName_str;
        if (!normalizePlayerName(characterName))
            return false;

        ObjectGuid characterGuid;
        uint32 accountId;

        Player* player = ObjectAccessor::FindPlayerByName(characterName);
        if (player)
        {
            characterGuid = player->GetGUID();
            accountId = player->GetSession()->GetAccountId();
            player->GetSession()->KickPlayer();
        }
        else
        {
            characterGuid = sCharacterCache->GetCharacterGuidByName(characterName);
            if (!characterGuid)
            {
                handler->PSendSysMessage(LANG_NO_PLAYER, characterName.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
            accountId = sCharacterCache->GetCharacterAccountIdByGuid(characterGuid);
        }

        std::string accountName;
        AccountMgr::GetName(accountId, accountName);

        Player::DeleteFromDB(characterGuid, accountId, true, true);
        handler->PSendSysMessage(LANG_CHARACTER_DELETED, characterName.c_str(), characterGuid.ToString().c_str(), accountName.c_str(), accountId);

        return true;
    }

    static bool HandleLevelUpCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* levelStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &levelStr);

        // exception opt second arg: .character level $name
        if (levelStr && isalpha(levelStr[0]))
        {
            nameStr = levelStr;
            levelStr = nullptr;                                    // current level will be used
        }

        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &targetName))
            return false;

        int32 oldlevel = target ? target->getLevel() : sCharacterCache->GetCharacterLevelByGuid(targetGuid);
        int32 addlevel = levelStr ? atoi(levelStr) : 1;
        int32 newlevel = oldlevel + addlevel;

        if (newlevel < 1)
            newlevel = 1;

        /*
        if (newlevel > STRONG_MAX_LEVEL)                         // hardcoded maximum level
            newlevel = STRONG_MAX_LEVEL;
        */

        if (newlevel > 120)                         // bye 255 faggots
            newlevel = 120;

        HandleCharacterLevel(target, targetGuid, oldlevel, newlevel, handler);

        if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)      // including chr == NULL
        {
            std::string nameLink = handler->playerLink(targetName);
            handler->PSendSysMessage(LANG_YOU_CHANGE_LVL, nameLink.c_str(), newlevel);
        }

        return true;
    }

    static bool HandlePDumpLoadCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* fileStr = strtok((char*)args, " ");
        if (!fileStr)
            return false;

        char* accountStr = strtok(nullptr, " ");
        if (!accountStr)
            return false;

        std::string accountName = accountStr;
        if (!Utf8ToUpperOnlyLatin(accountName))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountId = AccountMgr::GetId(accountName);
        if (!accountId)
        {
            accountId = atoi(accountStr);                             // use original string
            if (!accountId)
            {
                handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!AccountMgr::GetName(accountId, accountName))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, accountName.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* guidStr = nullptr;
        char* nameStr = strtok(nullptr, " ");

        std::string name;
        if (nameStr)
        {
            name = nameStr;
            // normalize the name if specified and check if it exists
            if (!normalizePlayerName(name))
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (ObjectMgr::CheckPlayerName(name, sWorld->GetDefaultDbcLocale(), true) != CHAR_NAME_SUCCESS)
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
                handler->SetSentErrorMessage(true);
                return false;
            }

            guidStr = strtok(nullptr, " ");
        }

        ObjectGuid::LowType guid = UI64LIT(0);

        if (guidStr)
        {
            guid = strtoull(guidStr, nullptr, 10);
            if (!guid)
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_GUID);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (sCharacterCache->GetCharacterAccountIdByGuid(ObjectGuid::Create<HighGuid::Player>(guid)))
            {
                handler->PSendSysMessage(LANG_CHARACTER_GUID_IN_USE, std::to_string(guid).c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        switch (PlayerDumpReader().LoadDump(fileStr, accountId, name, guid))
        {
        case DUMP_SUCCESS:
            handler->PSendSysMessage(LANG_COMMAND_IMPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            handler->PSendSysMessage(LANG_FILE_OPEN_FAIL, fileStr);
            handler->SetSentErrorMessage(true);
            return false;
        case DUMP_FILE_BROKEN:
            handler->PSendSysMessage(LANG_DUMP_BROKEN, fileStr);
            handler->SetSentErrorMessage(true);
            return false;
        case DUMP_TOO_MANY_CHARS:
            handler->PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL, accountName.c_str(), accountId);
            handler->SetSentErrorMessage(true);
            return false;
        default:
            handler->PSendSysMessage(LANG_COMMAND_IMPORT_FAILED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandlePDumpWriteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* fileStr = strtok((char*)args, " ");
        char* playerStr = strtok(nullptr, " ");

        if (!fileStr || !playerStr)
            return false;

        ObjectGuid guid;
        // character name can't start from number
        if (isNumeric(playerStr))
            guid = ObjectGuid::Create<HighGuid::Player>(strtoull(playerStr, nullptr, 10));
        else
        {
            std::string name = handler->extractPlayerNameFromLink(playerStr);
            if (name.empty())
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            guid = sCharacterCache->GetCharacterGuidByName(name);
        }

        if (!sCharacterCache->GetCharacterAccountIdByGuid(guid))
        {
            handler->PSendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        switch (PlayerDumpWriter().WriteDump(fileStr, guid.GetCounter()))
        {
        case DUMP_SUCCESS:
            handler->PSendSysMessage(LANG_COMMAND_EXPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            handler->PSendSysMessage(LANG_FILE_OPEN_FAIL, fileStr);
            handler->SetSentErrorMessage(true);
            return false;
        case DUMP_CHARACTER_DELETED:
            handler->PSendSysMessage(LANG_COMMAND_EXPORT_DELETED_CHAR);
            handler->SetSentErrorMessage(true);
            return false;
        default:
            handler->PSendSysMessage(LANG_COMMAND_EXPORT_FAILED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleCharacterSetRaceCommand(ChatHandler* handler, char const* args)
    {
        /*
        if (!*args)
        {
            handler->PSendSysMessage(LANG_BAD_SET_RACE_COMMAND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        const char* race = strtok((char*)args, " ");
        const char* sexe = strtok(NULL, " ");
        uint8 IdRace = 0;
        uint8 len = 0;
        std::string raceStr = "";
        std::string sexeStr = "";
        Gender gender;

        if (!race || !sexe)
        {
            handler->PSendSysMessage(LANG_BAD_SET_RACE_COMMAND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        raceStr = race;
        sexeStr = sexe;

        len = strlen(raceStr.c_str());

        for (uint8 i = 0; i < len; ++i)
        {
            raceStr[i] = tolower(raceStr[i]);
        }

        len = strlen(sexeStr.c_str());

        for (uint8 i = 0; i < len; ++i)
        {
            sexeStr[i] = tolower(sexeStr[i]);
        }

#pragma region RaceID
        if (raceStr.compare("humain") == 0)
            IdRace = RACE_HUMAN;
        else if (raceStr.compare("orc") == 0)
            IdRace = RACE_ORC;
        else if (raceStr.compare("nain") == 0)
            IdRace = RACE_DWARF;
        else if (raceStr.compare("edn") == 0)
            IdRace = RACE_NIGHTELF;
        else if (raceStr.compare("undead") == 0)
            IdRace = RACE_UNDEAD_PLAYER;
        else if (raceStr.compare("tauren") == 0)
            IdRace = RACE_TAUREN;
        else if (raceStr.compare("gnome") == 0)
            IdRace = RACE_GNOME;
        else if (raceStr.compare("troll") == 0)
            IdRace = RACE_TROLL;
        else if (raceStr.compare("goblin") == 0)
            IdRace = RACE_GOBLIN;
        else if (raceStr.compare("eds") == 0)
            IdRace = RACE_BLOODELF;
        else if (raceStr.compare("draenei") == 0)
            IdRace = RACE_DRAENEI;
        else if (raceStr.compare("naga") == 0)
            IdRace = RACE_NAGA;
        else if (raceStr.compare("broken") == 0)
            IdRace = RACE_BROKEN;
        else if (raceStr.compare("squelette") == 0)
            IdRace = RACE_SKELETON;
        else if (raceStr.compare("vrykul") == 0)
            IdRace = RACE_VRYKUL;
        else if (raceStr.compare("rohart") == 0)
            IdRace = RACE_TUSKARR;
        else if (raceStr.compare("foresttroll") == 0)
            IdRace = RACE_FOREST_TROLL;
        else if (raceStr.compare("taunka") == 0)
            IdRace = RACE_TAUNKA;
        else if (raceStr.compare("northskeleton") == 0)
            IdRace = RACE_NORTHREND_SKELETON;
        else if (raceStr.compare("icetroll") == 0)
            IdRace = RACE_ICE_TROLL;
        else if (raceStr.compare("worgen") == 0)
            IdRace = RACE_WORGEN;
        else if (raceStr.compare("pandaren") == 0)
            if (handler->GetSession()->GetPlayer()->getFaction() == 1)
                IdRace = RACE_PANDAREN_ALLIANCE;
            else if (handler->GetSession()->GetPlayer()->getFaction() == 2)
                IdRace = RACE_PANDAREN_HORDE;
            else
                IdRace = RACE_PANDAREN_ALLIANCE;
        else if (raceStr.compare("sacrenuit") == 0)
            IdRace = RACE_NIGHTBORNE;
        else if (raceStr.compare("hmtauren") == 0)
            IdRace = RACE_HIGHMOUNTAIN_TAUREN;
        else if (raceStr.compare("voidelf") == 0)
            IdRace = RACE_VOID_ELF;
        else if (raceStr.compare("lfdraenei") == 0)
            IdRace = RACE_LIGHTFORGED_DRAENEI;
        else if (raceStr.compare("zandalari") == 0)
            IdRace = RACE_ZANDALARI_TROLL;
        else if (raceStr.compare("kultiran") == 0)
            IdRace = RACE_KUL_TIRAN;
        else if (raceStr.compare("maigre") == 0)
            IdRace = RACE_THIN_HUMAN;
        else if (raceStr.compare("sombrefer") == 0)
            IdRace = RACE_DARK_IRON_DWARF;
        else if (raceStr.compare("vulpera") == 0)
            IdRace = RACE_VULPERA;
        else if (raceStr.compare("maghar") == 0)
            IdRace = RACE_MAGHAR_ORC;
        else if (raceStr.compare("ogre") == 0)
            IdRace = RACE_OGRE;
        else if (raceStr.compare("rouee") == 0)
            IdRace = RACE_ROUEE;
        else
        {
            handler->PSendSysMessage(LANG_BAD_RACE);
            handler->SetSentErrorMessage(true);
            return false;
        }
#pragma endregion

        if (sexeStr.compare("male") == 0)
            gender = GENDER_MALE;
        else if (sexeStr.compare("femelle") == 0)
            gender = GENDER_FEMALE;
        else if (sexeStr.compare("male") != 0 && sexeStr.compare("femelle") != 0)
        {
            handler->PSendSysMessage(LANG_BAD_RACE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (IdRace == RACE_TAUNKA || IdRace == RACE_FEL_ORC ||
            IdRace == RACE_ICE_TROLL || IdRace == RACE_TUSKARR ||
            IdRace == RACE_VRYKUL || IdRace == RACE_FOREST_TROLL ||
            IdRace == RACE_SKELETON || IdRace == RACE_NORTHREND_SKELETON ||
            IdRace == RACE_BROKEN || IdRace == RACE_THIN_HUMAN ||
            IdRace == RACE_OGRE || IdRace == RACE_ROUEE)
        {
            gender = GENDER_MALE;
        }

        Player* player = handler->GetSession()->GetPlayer();
        player->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, IdRace);
        player->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, gender);
        player->SetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER, gender);

        switch (IdRace)
        {
            case RACE_ORC:
                player->setFaction(2);
            case RACE_UNDEAD_PLAYER:
                player->setFaction(2);
            case RACE_TAUREN:
                player->setFaction(2);
            case RACE_TROLL:
                player->setFaction(2);
            case RACE_GOBLIN:
                player->setFaction(2);
            case RACE_BLOODELF:
                player->setFaction(2);
            case RACE_NAGA:
                player->setFaction(2);
            case RACE_SKELETON:
                player->setFaction(2);
            case RACE_VRYKUL:
                player->setFaction(2);
            case RACE_FOREST_TROLL:
                player->setFaction(2);
            case RACE_TAUNKA:
                player->setFaction(2);
            case RACE_NORTHREND_SKELETON:
                player->setFaction(2);
            case RACE_ICE_TROLL:
                player->setFaction(2);
            case RACE_PANDAREN_HORDE:
                player->setFaction(2);
            case RACE_HIGHMOUNTAIN_TAUREN:
                player->setFaction(2);
            case RACE_NIGHTBORNE:
                player->setFaction(2);
            case RACE_ZANDALARI_TROLL:
                player->setFaction(2);
            case RACE_VULPERA:
                player->setFaction(2);
            case RACE_MAGHAR_ORC:
                player->setFaction(2);
            default:
                player->setFaction(1);
        }

        player->SaveToDB();

        if (WorldSession* session = player->GetSession())
            session->KickPlayer();

        sWorld->UpdateCharacterInfo(player->GetGUID(), player->GetName().c_str(), gender, IdRace);

        */
        return true;

    }

    static bool HandleCharacterSetClassCommand(ChatHandler* handler, char const* args)
    {
        /*
        if (!*args)
        {
            handler->PSendSysMessage(LANG_BAD_SET_CLASS_COMMAND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        const char* classid = strtok((char*)args, " ");
        uint8 IdClass = 0;
        uint8 len = 0;
        std::string classStr = "";

        if (!classid)
        {
            handler->PSendSysMessage(LANG_BAD_SET_CLASS_COMMAND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        classStr = classid;

        len = strlen(classStr.c_str());

        for (uint8 i = 0; i < len; ++i)
        {
            classStr[i] = tolower(classStr[i]);
        }

#pragma region ClassID
        if (classStr.compare("guerrier") == 0)
            IdClass = CLASS_WARRIOR;
        else if (classStr.compare("paladin") == 0)
            IdClass = CLASS_PALADIN;
        else if (classStr.compare("chasseur") == 0)
            IdClass = CLASS_HUNTER;
        else if (classStr.compare("voleur") == 0)
            IdClass = CLASS_ROGUE;
        else if (classStr.compare("pretre") == 0)
            IdClass = CLASS_PRIEST;
        else if (classStr.compare("dk") == 0)
            IdClass = CLASS_DEATH_KNIGHT;
        else if (classStr.compare("chaman") == 0)
            IdClass = CLASS_SHAMAN;
        else if (classStr.compare("mage") == 0)
            IdClass = CLASS_MAGE;
        else if (classStr.compare("demoniste") == 0)
            IdClass = CLASS_WARLOCK;
        else if (classStr.compare("moine") == 0)
            IdClass = CLASS_MONK;
        else if (classStr.compare("druide") == 0)
            IdClass = CLASS_DRUID;
        else if (classStr.compare("dh") == 0)
            IdClass = CLASS_DEMON_HUNTER;
        else
        {
            handler->PSendSysMessage(LANG_BAD_CLASS);
            handler->SetSentErrorMessage(true);
            return false;
        }
#pragma endregion

        Player* player = handler->GetSession()->GetPlayer();

        // Reset passive class spell for avoiding crash

        // All class Spec except DH
        uint32 passiveSpell = 137006;
        for (uint32 i = 0; i < 45; ++i) // first classe/spec spellid = 137006, last is 137050 
        {
            if (player->HasSpell(passiveSpell))
                player->RemoveSpell(passiveSpell, false, false);
            passiveSpell++;
        }
        // DH Spec
        passiveSpell = 212612;
        for (uint32 i = 0; i < 2; ++i) // 212612 dps / 212613 tank
        {
            if (player->HasSpell(passiveSpell))
                player->RemoveSpell(passiveSpell, false, false);
            passiveSpell++;
        }

        // Change class
        player->SetClass(IdClass);

        // Save
        player->SaveToDB();

        // Kick player
        if (WorldSession* session = player->GetSession())
            session->KickPlayer();

        // Infos
        sCharacterCache->UpdateCharacterData(player->GetGUID(), player->GetName().c_str(), &player->getGender, &player->getRace);
        */
        return true;

    }
};

void AddSC_character_commandscript()
{
    new character_commandscript();
}
