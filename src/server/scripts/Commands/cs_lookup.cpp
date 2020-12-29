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
Name: lookup_commandscript
%Complete: 100
Comment: All lookup related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameEventMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldSession.h"
#include <sstream>

class lookup_commandscript : public CommandScript
{
public:
    lookup_commandscript() : CommandScript("lookup_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> lookupPlayerCommandTable =
        {
            { "ip",      rbac::RBAC_PERM_COMMAND_LOOKUP_PLAYER_IP,      true, &HandleLookupPlayerIpCommand,        "" },
            { "account", rbac::RBAC_PERM_COMMAND_LOOKUP_PLAYER_ACCOUNT, true, &HandleLookupPlayerAccountCommand,   "" },
            { "email",   rbac::RBAC_PERM_COMMAND_LOOKUP_PLAYER_EMAIL,   true, &HandleLookupPlayerEmailCommand,     "" },
            { "mail",    rbac::RBAC_PERM_COMMAND_LOOKUP_PLAYER_EMAIL,   true, &HandleLookupPlayerMailCommand,      "" },

        };

        static std::vector<ChatCommand> lookupSpellCommandTable =
        {
            { "id", rbac::RBAC_PERM_COMMAND_LOOKUP_SPELL_ID, true, &HandleLookupSpellIdCommand,         "" },
            { "",   rbac::RBAC_PERM_COMMAND_LOOKUP_SPELL,    true, &HandleLookupSpellCommand,           "" },
        };

        static std::vector<ChatCommand> lookupPhaseCommandTable =
        {
            { "own", rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,    true, &HandleLookupPhaseOwnCommand,        "" },
            { "aut", rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,    true, &HandleLookupPhaseAutCommand,        "" },
        };

        static std::vector<ChatCommand> lookupItemForgeCommandTable =
        {
            { "item", rbac::RBAC_PERM_COMMAND_LOOKUP_ITEM,   true, &HandleLookupItemForgeCommand,       "" },
        };

        static std::vector<ChatCommand> lookupCommandTable =
        {
            { "area",           rbac::RBAC_PERM_COMMAND_LOOKUP_AREA,     true, &HandleLookupAreaCommand,            "" },
            { "creature",       rbac::RBAC_PERM_COMMAND_LOOKUP_CREATURE, true, &HandleLookupCreatureCommand,        "" },
            { "event",          rbac::RBAC_PERM_COMMAND_LOOKUP_EVENT,    true, &HandleLookupEventCommand,           "" },
            { "faction",        rbac::RBAC_PERM_COMMAND_LOOKUP_FACTION,  true, &HandleLookupFactionCommand,         "" },
            { "item",           rbac::RBAC_PERM_COMMAND_LOOKUP_ITEM,     true, &HandleLookupItemCommand,            "" },
            { "itemset",        rbac::RBAC_PERM_COMMAND_LOOKUP_ITEMSET,  true, &HandleLookupItemSetCommand,         "" },
            { "object",         rbac::RBAC_PERM_COMMAND_LOOKUP_OBJECT,   true, &HandleLookupObjectCommand,          "" },
            { "quest",          rbac::RBAC_PERM_COMMAND_LOOKUP_QUEST,    true, &HandleLookupQuestCommand,           "" },
            { "player",         rbac::RBAC_PERM_COMMAND_LOOKUP_PLAYER,   true, NULL,                                "", lookupPlayerCommandTable },
            { "skill",          rbac::RBAC_PERM_COMMAND_LOOKUP_SKILL,    true, &HandleLookupSkillCommand,           "" },
            { "spell",          rbac::RBAC_PERM_COMMAND_LOOKUP_SPELL,    true, NULL,                                "", lookupSpellCommandTable },
            { "taxinode",       rbac::RBAC_PERM_COMMAND_LOOKUP_TAXINODE, true, &HandleLookupTaxiNodeCommand,        "" },
            { "tele",           rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, &HandleLookupTeleCommand,            "" },
            { "title",          rbac::RBAC_PERM_COMMAND_LOOKUP_TITLE,    true, &HandleLookupTitleCommand,           "" },
            { "map",            rbac::RBAC_PERM_COMMAND_LOOKUP_MAP,      true, &HandleLookupMapCommand,             "" },
            { "skybox",         rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, &HandleLookupSkyboxCommand,          "" },
            { "ambiance",       rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, &HandleLookupAmbianceCommand,        "" },
            { "sound",          rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, &HandleLookupSoundCommand,           "" },
            { "terrain",        rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, &HandleLookupTerrainCommand,         "" },
            { "phase",          rbac::RBAC_PERM_COMMAND_LOOKUP_TELE,     true, NULL,                                "", lookupPhaseCommandTable },
            { "forge",          rbac::RBAC_PERM_COMMAND_LOOKUP_ITEM,     true, NULL,                                "", lookupItemForgeCommandTable },
            { "dupplication",   rbac::RBAC_PERM_COMMAND_GOB_DUPPLICATION_READ ,     true, &HandleLookupDupplicationCommand,    ""},
            

        };

        static std::vector<ChatCommand> commandTable =
        {
            { "lookup", rbac::RBAC_PERM_COMMAND_LOOKUP,  true, nullptr, "", lookupCommandTable },
        };
        return commandTable;
    }

    static bool HandleLookupAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        // Search in AreaTable.dbc
        for (uint32 i = 0; i < sAreaTableStore.GetNumRows(); ++i)
        {
            AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(i);
            if (areaEntry)
            {
                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = areaEntry->AreaName[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = areaEntry->AreaName[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    // send area in "id - [name]" format
                    std::ostringstream ss;
                    if (handler->GetSession())
                        ss << i << " - |cffffffff|Harea:" << i << "|h[" << name << "]|h|r";
                    else
                        ss << i << " - " << name;

                    handler->SendSysMessage(ss.str().c_str());

                    if (!found)
                        found = true;
                }
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOAREAFOUND);

        return true;
    }

    static bool HandleLookupCreatureCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
        for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
        {
            uint32 id = itr->second.Entry;
            uint8 localeIndex = handler->GetSessionDbLocaleIndex();
            if (CreatureLocale const* creatureLocale = sObjectMgr->GetCreatureLocale(id))
            {
                if (creatureLocale->Name.size() > localeIndex && !creatureLocale->Name[localeIndex].empty())
                {
                    std::string name = creatureLocale->Name[localeIndex];

                    if (Utf8FitTo(name, wNamePart))
                    {
                        if (maxResults && count++ == maxResults)
                        {
                            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                            return true;
                        }

                        if (handler->GetSession())
                            handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str());
                        else
                            handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str());

                        if (!found)
                            found = true;

                        continue;
                    }
                }
            }

            std::string name = itr->second.Name;
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str());
                else
                    handler->PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);

        return true;
    }

    static bool HandleLookupEventCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
        GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

        for (uint32 id = 0; id < events.size(); ++id)
        {
            GameEventData const& eventData = events[id];

            std::string descr = eventData.description;
            if (descr.empty())
                continue;

            if (Utf8FitTo(descr, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                char const* active = activeEvents.find(id) != activeEvents.end() ? handler->GetTrinityString(LANG_ACTIVE) : "";

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT, id, id, eventData.description.c_str(), active);
                else
                    handler->PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE, id, eventData.description.c_str(), active);

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_NOEVENTFOUND);

        return true;
    }

    static bool HandleLookupFactionCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // Can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower (wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        for (uint32 id = 0; id < sFactionStore.GetNumRows(); ++id)
        {
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(id);
            if (factionEntry)
            {
                FactionState const* factionState = target ? target->GetReputationMgr().GetState(factionEntry) : nullptr;

                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = factionEntry->Name[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = factionEntry->Name[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    // send faction in "id - [faction] rank reputation [visible] [at war] [own team] [unknown] [invisible] [inactive]" format
                    // or              "id - [faction] [no reputation]" format
                    std::ostringstream ss;
                    if (handler->GetSession())
                        ss << id << " - |cffffffff|Hfaction:" << id << "|h[" << name << ' ' << localeNames[locale] << "]|h|r";
                    else
                        ss << id << " - " << name << ' ' << localeNames[locale];

                    if (factionState) // and then target != NULL also
                    {
                        uint32 index = target->GetReputationMgr().GetReputationRankStrIndex(factionEntry);
                        std::string rankName = handler->GetTrinityString(index);

                        ss << ' ' << rankName << "|h|r (" << target->GetReputationMgr().GetReputation(factionEntry) << ')';

                        if (factionState->Flags & FACTION_FLAG_VISIBLE)
                            ss << handler->GetTrinityString(LANG_FACTION_VISIBLE);
                        if (factionState->Flags & FACTION_FLAG_AT_WAR)
                            ss << handler->GetTrinityString(LANG_FACTION_ATWAR);
                        if (factionState->Flags & FACTION_FLAG_PEACE_FORCED)
                            ss << handler->GetTrinityString(LANG_FACTION_PEACE_FORCED);
                        if (factionState->Flags & FACTION_FLAG_HIDDEN)
                            ss << handler->GetTrinityString(LANG_FACTION_HIDDEN);
                        if (factionState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
                            ss << handler->GetTrinityString(LANG_FACTION_INVISIBLE_FORCED);
                        if (factionState->Flags & FACTION_FLAG_INACTIVE)
                            ss << handler->GetTrinityString(LANG_FACTION_INACTIVE);
                    }
                    else
                        ss << handler->GetTrinityString(LANG_FACTION_NOREPUTATION);

                    handler->SendSysMessage(ss.str().c_str());

                    if (!found)
                        found = true;
                }
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_FACTION_NOTFOUND);
        return true;
    }

    static bool HandleLookupItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in ItemSparse
        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            std::string name = itr->second.GetName(handler->GetSessionDbcLocale());
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_ITEM_LIST_CHAT, itr->second.GetId(), itr->second.GetId(), name.c_str());
                else
                    handler->PSendSysMessage(LANG_ITEM_LIST_CONSOLE, itr->second.GetId(), name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOITEMFOUND);

        return true;
    }

    static bool HandleLookupItemSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in ItemSet.dbc
        for (uint32 id = 0; id < sItemSetStore.GetNumRows(); id++)
        {
            ItemSetEntry const* set = sItemSetStore.LookupEntry(id);
            if (set)
            {
                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = set->Name[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = set->Name[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    // send item set in "id - [namedlink locale]" format
                    if (handler->GetSession())
                        handler->PSendSysMessage(LANG_ITEMSET_LIST_CHAT, id, id, name.c_str(), "");
                    else
                        handler->PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE, id, name.c_str(), "");

                    if (!found)
                        found = true;
                }
            }
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);

        return true;
    }

    static bool HandleLookupObjectCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplates();
        for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
        {
            uint8 localeIndex = handler->GetSessionDbLocaleIndex();
            if (GameObjectLocale const* objectLocalte = sObjectMgr->GetGameObjectLocale(itr->second.entry))
            {
                if (objectLocalte->Name.size() > localeIndex && !objectLocalte->Name[localeIndex].empty())
                {
                    std::string name = objectLocalte->Name[localeIndex];

                    if (Utf8FitTo(name, wNamePart))
                    {
                        if (maxResults && count++ == maxResults)
                        {
                            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                            return true;
                        }

                        if (handler->GetSession())
                            handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->second.entry, itr->second.entry, name.c_str());
                        else
                            handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->second.entry, name.c_str());

                        if (!found)
                            found = true;

                        continue;
                    }
                }
            }

            std::string name = itr->second.name;
            if (name.empty())
                continue;

            if (Utf8FitTo(name, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->second.entry, itr->second.entry, name.c_str());
                else
                    handler->PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->second.entry, name.c_str());

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

        return true;
    }

    static bool HandleLookupQuestCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL at console call
        Player* target = handler->getSelectedPlayerOrSelf();

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        ObjectMgr::QuestMap const& qTemplates = sObjectMgr->GetQuestTemplates();
        for (ObjectMgr::QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
        {
            Quest* qInfo = iter->second;

            int localeIndex = handler->GetSessionDbLocaleIndex();
            if (localeIndex >= 0)
            {
                uint8 ulocaleIndex = uint8(localeIndex);
                if (QuestTemplateLocale const* questLocale = sObjectMgr->GetQuestLocale(qInfo->GetQuestId()))
                {
                    if (questLocale->LogTitle.size() > ulocaleIndex && !questLocale->LogTitle[ulocaleIndex].empty())
                    {
                        std::string title = questLocale->LogTitle[ulocaleIndex];

                        if (Utf8FitTo(title, wNamePart))
                        {
                            if (maxResults && count++ == maxResults)
                            {
                                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                                return true;
                            }

                            char const* statusStr = "";

                            if (target)
                            {
                                QuestStatus status = target->GetQuestStatus(qInfo->GetQuestId());

                                switch (status)
                                {
                                    case QUEST_STATUS_COMPLETE:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                                        break;
                                    case QUEST_STATUS_INCOMPLETE:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                                        break;
                                    case QUEST_STATUS_REWARDED:
                                        statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                                        break;
                                    default:
                                        break;
                                }
                            }

                            if (handler->GetSession())
                            {
                                int32 maxLevel = 0;
                                if (Optional<ContentTuningLevels> questLevels = sDB2Manager.GetContentTuningData(qInfo->GetContentTuningId(),
                                    handler->GetSession()->GetPlayer()->m_playerData->CtrOptions->ContentTuningConditionMask))
                                    maxLevel = questLevels->MaxLevel;

                                int32 scalingFactionGroup = 0;
                                if (ContentTuningEntry const* contentTuning = sContentTuningStore.LookupEntry(qInfo->GetContentTuningId()))
                                    scalingFactionGroup = contentTuning->GetScalingFactionGroup();

                                handler->PSendSysMessage(LANG_QUEST_LIST_CHAT, qInfo->GetQuestId(), qInfo->GetQuestId(),
                                    handler->GetSession()->GetPlayer()->GetQuestLevel(qInfo),
                                    handler->GetSession()->GetPlayer()->GetQuestMinLevel(qInfo),
                                    maxLevel, scalingFactionGroup,
                                    title.c_str(), statusStr);
                            }
                            else
                                handler->PSendSysMessage(LANG_QUEST_LIST_CONSOLE, qInfo->GetQuestId(), title.c_str(), statusStr);

                            if (!found)
                                found = true;

                            continue;
                        }
                    }
                }
            }

            std::string title = qInfo->GetLogTitle();
            if (title.empty())
                continue;

            if (Utf8FitTo(title, wNamePart))
            {
                if (maxResults && count++ == maxResults)
                {
                    handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                    return true;
                }

                char const* statusStr = "";

                if (target)
                {
                    QuestStatus status = target->GetQuestStatus(qInfo->GetQuestId());

                    switch (status)
                    {
                        case QUEST_STATUS_COMPLETE:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                            break;
                        case QUEST_STATUS_INCOMPLETE:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                            break;
                        case QUEST_STATUS_REWARDED:
                            statusStr = handler->GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                            break;
                        default:
                            break;
                    }
                }

                if (handler->GetSession())
                {
                    int32 maxLevel = 0;
                    if (Optional<ContentTuningLevels> questLevels = sDB2Manager.GetContentTuningData(qInfo->GetContentTuningId(),
                        handler->GetSession()->GetPlayer()->m_playerData->CtrOptions->ContentTuningConditionMask))
                        maxLevel = questLevels->MaxLevel;

                    int32 scalingFactionGroup = 0;
                    if (ContentTuningEntry const* contentTuning = sContentTuningStore.LookupEntry(qInfo->GetContentTuningId()))
                        scalingFactionGroup = contentTuning->GetScalingFactionGroup();

                    handler->PSendSysMessage(LANG_QUEST_LIST_CHAT, qInfo->GetQuestId(), qInfo->GetQuestId(),
                        handler->GetSession()->GetPlayer()->GetQuestLevel(qInfo),
                        handler->GetSession()->GetPlayer()->GetQuestMinLevel(qInfo),
                        maxLevel, scalingFactionGroup,
                        title.c_str(), statusStr);
                }
                else
                    handler->PSendSysMessage(LANG_QUEST_LIST_CONSOLE, qInfo->GetQuestId(), title.c_str(), statusStr);

                if (!found)
                    found = true;
            }
        }

        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

        return true;
    }

    static bool HandleLookupSkillCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL in console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in SkillLine.dbc
        for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); id++)
        {
            SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(id);
            if (skillInfo)
            {
                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = skillInfo->DisplayName[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = skillInfo->DisplayName[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    std::string valStr = "";
                    char const* knownStr = "";
                    if (target && target->HasSkill(id))
                    {
                        knownStr = handler->GetTrinityString(LANG_KNOWN);
                        uint32 curValue = target->GetPureSkillValue(id);
                        uint32 maxValue = target->GetPureMaxSkillValue(id);
                        uint32 permValue = target->GetSkillPermBonusValue(id);
                        uint32 tempValue = target->GetSkillTempBonusValue(id);

                        char const* valFormat = handler->GetTrinityString(LANG_SKILL_VALUES);
                        valStr = Trinity::StringFormat(valFormat, curValue, maxValue, permValue, tempValue);
                    }

                    // send skill in "id - [namedlink locale]" format
                    if (handler->GetSession())
                        handler->PSendSysMessage(LANG_SKILL_LIST_CHAT, id, id, name.c_str(), "", knownStr, valStr.c_str());
                    else
                        handler->PSendSysMessage(LANG_SKILL_LIST_CONSOLE, id, name.c_str(), "", knownStr, valStr.c_str());

                    if (!found)
                        found = true;
                }
            }
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOSKILLFOUND);

        return true;
    }

    static bool HandleLookupSpellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in SpellName.dbc
        for (SpellNameEntry const* spellName : sSpellNameStore)
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellName->ID, DIFFICULTY_NONE))
            {
                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = spellInfo->SpellName->Str[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart))
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = spellInfo->SpellName->Str[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && count++ == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    bool known = target && target->HasSpell(spellInfo->Id);

                    SpellEffectInfo const* effect = spellInfo->GetEffect(EFFECT_0);
                    bool learn = effect ? (effect->Effect == SPELL_EFFECT_LEARN_SPELL) : false;

                    SpellInfo const* learnSpellInfo = effect ? sSpellMgr->GetSpellInfo(effect->TriggerSpell, spellInfo->Difficulty) : nullptr;

                    bool talent = spellInfo->HasAttribute(SPELL_ATTR0_CU_IS_TALENT);
                    bool passive = spellInfo->IsPassive();
                    bool active = target && target->HasAura(spellInfo->Id);

                    // unit32 used to prevent interpreting uint8 as char at output
                    // find rank of learned spell for learning spell, or talent rank
                    uint32 rank = learn && learnSpellInfo ? learnSpellInfo->GetRank() : spellInfo->GetRank();

                    // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                    std::ostringstream ss;
                    if (handler->GetSession())
                        ss << spellInfo->Id << " - |cffffffff|Hspell:" << spellInfo->Id << "|h[" << name;
                    else
                        ss << spellInfo->Id << " - " << name;

                    // include rank in link name
                    if (rank)
                        ss << handler->GetTrinityString(LANG_SPELL_RANK) << rank;

                    if (handler->GetSession())
                        ss << "]|h|r";

                    if (talent)
                        ss << handler->GetTrinityString(LANG_TALENT);
                    if (passive)
                        ss << handler->GetTrinityString(LANG_PASSIVE);
                    if (learn)
                        ss << handler->GetTrinityString(LANG_LEARN);
                    if (known)
                        ss << handler->GetTrinityString(LANG_KNOWN);
                    if (active)
                        ss << handler->GetTrinityString(LANG_ACTIVE);

                    handler->SendSysMessage(ss.str().c_str());

                    if (!found)
                        found = true;
                }
            }
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOSPELLFOUND);

        return true;
    }

    static bool HandleLookupSpellIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL at console call
        Player* target = handler->getSelectedPlayer();

        uint32 id = atoi((char*)args);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id, DIFFICULTY_NONE))
        {
            LocaleConstant locale = handler->GetSessionDbcLocale();
            std::string name = spellInfo->SpellName->Str[locale];
            if (name.empty())
            {
                handler->SendSysMessage(LANG_COMMAND_NOSPELLFOUND);
                return true;
            }

            bool known = target && target->HasSpell(id);

            SpellEffectInfo const* effect = spellInfo->GetEffect(EFFECT_0);
            bool learn = effect? (effect->Effect == SPELL_EFFECT_LEARN_SPELL) : false;

            SpellInfo const* learnSpellInfo = effect ? sSpellMgr->GetSpellInfo(effect->TriggerSpell, DIFFICULTY_NONE) : nullptr;

            bool talent = spellInfo->HasAttribute(SPELL_ATTR0_CU_IS_TALENT);
            bool passive = spellInfo->IsPassive();
            bool active = target && target->HasAura(id);

            // unit32 used to prevent interpreting uint8 as char at output
            // find rank of learned spell for learning spell, or talent rank
            uint32 rank = learn && learnSpellInfo ? learnSpellInfo->GetRank() : spellInfo->GetRank();

            // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
            std::ostringstream ss;
            if (handler->GetSession())
                ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
            else
                ss << id << " - " << name;

            // include rank in link name
            if (rank)
                ss << handler->GetTrinityString(LANG_SPELL_RANK) << rank;

            if (handler->GetSession())
                ss << ' ' << localeNames[locale] << "]|h|r";
            else
                ss << ' ' << localeNames[locale];

            if (talent)
                ss << handler->GetTrinityString(LANG_TALENT);
            if (passive)
                ss << handler->GetTrinityString(LANG_PASSIVE);
            if (learn)
                ss << handler->GetTrinityString(LANG_LEARN);
            if (known)
                ss << handler->GetTrinityString(LANG_KNOWN);
            if (active)
                ss << handler->GetTrinityString(LANG_ACTIVE);

            handler->SendSysMessage(ss.str().c_str());
        }
        else
            handler->SendSysMessage(LANG_COMMAND_NOSPELLFOUND);

        return true;
    }

    static bool HandleLookupTaxiNodeCommand(ChatHandler* handler, const char * args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        bool found = false;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        LocaleConstant locale = handler->GetSessionDbcLocale();

        // Search in TaxiNodes.dbc
        for (TaxiNodesEntry const* nodeEntry : sTaxiNodesStore)
        {
            std::string name = nodeEntry->Name[locale];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wNamePart))
                continue;

            if (maxResults && count++ == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            // send taxinode in "id - [name] (Map:m X:x Y:y Z:z)" format
            if (handler->GetSession())
                handler->PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CHAT, nodeEntry->ID, nodeEntry->ID, name.c_str(), "",
                    uint32(nodeEntry->ContinentID), nodeEntry->Pos.X, nodeEntry->Pos.Y, nodeEntry->Pos.Z);
            else
                handler->PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CONSOLE, nodeEntry->ID, name.c_str(), "",
                    uint32(nodeEntry->ContinentID), nodeEntry->Pos.X, nodeEntry->Pos.Y, nodeEntry->Pos.Z);

            if (!found)
                found = true;
        }
        if (!found)
            handler->SendSysMessage(LANG_COMMAND_NOTAXINODEFOUND);

        return true;
    }

    // Find tele in game_tele order by name
    static bool HandleLookupTeleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char const* str = strtok((char*)args, " ");
        if (!str)
            return false;

        std::string namePart = str;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        std::ostringstream reply;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        bool limitReached = false;

        GameTeleContainer const & teleMap = sObjectMgr->GetGameTeleMap();
        for (GameTeleContainer::const_iterator itr = teleMap.begin(); itr != teleMap.end(); ++itr)
        {
            GameTele const* tele = &itr->second;

            if (tele->wnameLow.find(wNamePart) == std::wstring::npos)
                continue;

            if (maxResults && count++ == maxResults)
            {
                limitReached = true;
                break;
            }

            if (handler->GetSession())
                reply << "  |cffffffff|Htele:" << itr->first << "|h[" << tele->name << "]|h|r\n";
            else
                reply << "  " << itr->first << ' ' << tele->name << "\n";
        }

        if (reply.str().empty())
            handler->SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
        else
            handler->PSendSysMessage(LANG_COMMAND_TELE_LOCATION, reply.str().c_str());

        if (limitReached)
            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);

        return true;
    }

    static bool HandleLookupTitleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // can be NULL in console call
        Player* target = handler->getSelectedPlayer();

        // title name have single string arg for player name
        char const* targetName = target ? target->GetName().c_str() : "NAME";

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower(wNamePart);

        uint32 counter = 0;                                     // Counter for figure out that we found smth.
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // Search in CharTitles.dbc
        for (uint32 id = 0; id < sCharTitlesStore.GetNumRows(); id++)
        {
            if (CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id))
            {
                for (uint8 gender = GENDER_MALE; gender <= GENDER_FEMALE; ++gender)
                {
                    if (target && target->getGender() != gender)
                        continue;

                    LocaleConstant locale = handler->GetSessionDbcLocale();
                    std::string name = (gender == GENDER_MALE ? titleInfo->Name : titleInfo->Name1)[locale];

                    if (name.empty())
                        continue;

                    if (!Utf8FitTo(name, wNamePart))
                    {
                        locale = LOCALE_enUS;
                        for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                        {
                            if (locale == handler->GetSessionDbcLocale())
                                continue;

                            name = (gender == GENDER_MALE ? titleInfo->Name : titleInfo->Name1)[locale];
                            if (name.empty())
                                continue;

                            if (Utf8FitTo(name, wNamePart))
                                break;
                        }
                    }

                    if (locale < TOTAL_LOCALES)
                    {
                        if (maxResults && counter == maxResults)
                        {
                            handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                            return true;
                        }

                        char const* knownStr = target && target->HasTitle(titleInfo) ? handler->GetTrinityString(LANG_KNOWN) : "";

                        char const* activeStr = target && *target->m_playerData->PlayerTitle == titleInfo->MaskID
                            ? handler->GetTrinityString(LANG_ACTIVE)
                            : "";

                        std::string titleNameStr = Trinity::StringFormat(name.c_str(), targetName);

                        // send title in "id (idx:idx) - [namedlink locale]" format
                        if (handler->GetSession())
                            handler->PSendSysMessage(LANG_TITLE_LIST_CHAT, id, titleInfo->MaskID, id, titleNameStr.c_str(), "", knownStr, activeStr);
                        else
                            handler->PSendSysMessage(LANG_TITLE_LIST_CONSOLE, id, titleInfo->MaskID, titleNameStr.c_str(), "", knownStr, activeStr);

                        ++counter;
                    }
                }
            }
        }
        if (counter == 0)  // if counter == 0 then we found nth
            handler->SendSysMessage(LANG_COMMAND_NOTITLEFOUND);

        return true;
    }

    static bool HandleLookupMapCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        uint32 counter = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // search in Map.dbc
        for (uint32 id = 0; id < sMapStore.GetNumRows(); id++)
        {
            if (MapEntry const* mapInfo = sMapStore.LookupEntry(id))
            {
                LocaleConstant locale = handler->GetSessionDbcLocale();
                std::string name = mapInfo->MapName[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart) && handler->GetSession())
                {
                    locale = LOCALE_enUS;
                    for (; locale < TOTAL_LOCALES; locale = LocaleConstant(locale + 1))
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = mapInfo->MapName[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && counter == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    std::ostringstream ss;
                    ss << id << " - [" << name << ']';

                    if (mapInfo->IsContinent())
                        ss << handler->GetTrinityString(LANG_CONTINENT);

                    switch (mapInfo->InstanceType)
                    {
                        case MAP_INSTANCE:
                            ss << handler->GetTrinityString(LANG_INSTANCE);
                            break;
                        case MAP_RAID:
                            ss << handler->GetTrinityString(LANG_RAID);
                            break;
                        case MAP_BATTLEGROUND:
                            ss << handler->GetTrinityString(LANG_BATTLEGROUND);
                            break;
                        case MAP_ARENA:
                            ss << handler->GetTrinityString(LANG_ARENA);
                            break;
                    }

                    handler->SendSysMessage(ss.str().c_str());

                    ++counter;
                }
            }
        }

        if (!counter)
            handler->SendSysMessage(LANG_COMMAND_NOMAPFOUND);

        return true;
    }

    static bool HandleLookupPlayerIpCommand(ChatHandler* handler, char const* args)
    {
        //std::string ip;
		std::string nameTarget;
        int32 limit;
        char* limitStr;

        Player* target = handler->getSelectedPlayer();
        if (!*args)
        {
            // NULL only if used from console
            if (!target || target == handler->GetSession()->GetPlayer())
                return false;

            //ip = target->GetSession()->GetRemoteAddress();
			nameTarget = target->GetSession()->GetPlayer()->GetName();
            limit = -1;
        }
        else
        {
<<<<<<< HEAD
            //ip = strtok((char*)args, " ");
			nameTarget = strtok((char*)args, " ");
            limitStr = strtok(NULL, " ");
=======
            ip = strtok((char*)args, " ");
            limitStr = strtok(nullptr, " ");
>>>>>>> e26122dc54b5c5a356a97a842718168dab97a0aa
            limit = limitStr ? atoi(limitStr) : -1;
        }

		std::string ip;
		Player* player = ObjectAccessor::FindPlayerByName(nameTarget);
		if (!player)
		{
			ip = nameTarget;
		}
		else
		{
			ip = player->GetSession()->GetRemoteAddress();
		}

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_IP);
        stmt->setString(0, ip);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        return LookupPlayerSearchCommand(result, limit, handler);
    }

    static bool HandleLookupPlayerAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string account = strtok((char*)args, " ");
        char* limitStr = strtok(nullptr, " ");
        int32 limit = limitStr ? atoi(limitStr) : -1;

        if (!Utf8ToUpperOnlyLatin(account))
            return false;

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_LIST_BY_NAME);
        stmt->setString(0, account);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        return LookupPlayerSearchCommand(result, limit, handler);
    }

    static bool HandleLookupPlayerEmailCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string email = strtok((char*)args, " ");
        char* limitStr = strtok(nullptr, " ");
        int32 limit = limitStr ? atoi(limitStr) : -1;

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_LIST_BY_EMAIL);
        stmt->setString(0, email);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        return LookupPlayerSearchCommand(result, limit, handler);
    }

    static bool LookupPlayerSearchCommand(PreparedQueryResult result, int32 limit, ChatHandler* handler)
    {
        if (!result)
        {
            handler->PSendSysMessage(LANG_NO_PLAYERS_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        //Player* target;
        int32 counter = 0;
        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        do
        {
            if (maxResults && count++ == maxResults)
            {
                handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                return true;
            }

            Field* fields           = result->Fetch();
            uint32 accountId        = fields[0].GetUInt32();
            std::string accountName = fields[1].GetString();

            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_NAME_BY_ACC);
            stmt->setUInt32(0, accountId);
            PreparedQueryResult result2 = CharacterDatabase.Query(stmt);

            if (result2)
            {
                handler->PSendSysMessage(LANG_LOOKUP_PLAYER_ACCOUNT, accountName.c_str(), accountId);

                do
                {
                    Field* characterFields  = result2->Fetch();
                    ObjectGuid guid         = ObjectGuid::Create<HighGuid::Player>(characterFields[0].GetUInt64());
                    std::string name        = characterFields[1].GetString();
                    Player* target = NULL;
                    target = ObjectAccessor::FindPlayer(guid);


                    if (target && target != NULL) {
                        handler->PSendSysMessage(LANG_LOOKUP_PLAYER_CHARACTER_ONLINE, name.c_str(), guid.ToString().c_str());
                        ++counter;
                        target = NULL;
                    }
                    else {
                        handler->PSendSysMessage(LANG_LOOKUP_PLAYER_CHARACTER_OFFLINE, name.c_str(), guid.ToString().c_str());
                        ++counter;
                        target = NULL;
                    }

                }
                while (result2->NextRow() && (limit == -1 || counter < limit));
            }
        }
        while (result->NextRow());

        if (counter == 0) // empty accounts only
        {
            handler->PSendSysMessage(LANG_NO_PLAYERS_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleLookupSkyboxCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            return false;
        }

        char const* str = strtok((char*)args, " ");
        if (!str)
            return false;

        std::string requestName = str;
        if (requestName.find('\'') != std::string::npos) {
            handler->PSendSysMessage(LANG_LOOKUP_SOUND_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else {
            auto query = DB2Manager::GetMapSkyboxs();
            if (query.size() != 0) {
                for(const auto& skyboxData : query) {
                    std::string skyboxName = skyboxData.second;

                    std::transform(skyboxName.begin(), skyboxName.end(), skyboxName.begin(),
                        [](unsigned char c) { return std::tolower(c); });

                    if(skyboxName.find(requestName) != std::string::npos)
                        handler->PSendSysMessage(LANG_LOOKUP_SKYBOX, skyboxData.first, skyboxName.c_str());
                }
            }
            else {

                handler->PSendSysMessage(LANG_LOOKUP_SKYBOX_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }

            return true;
        }

    }

    static bool HandleLookupAmbianceCommand(ChatHandler* handler, char const* args) {

        if (!args)
            return false;

        char const* mapid = strtok((char*)args, " ");
        if (!mapid)
            return false;

        std::string checkIsValid = mapid;
        uint16 map_id = 0;

        if (std::all_of(checkIsValid.begin(), checkIsValid.end(), ::isdigit)) {

            map_id = atoi(mapid);

        }
        else {
            handler->PSendSysMessage(LANG_LOOKUP_AMBIANCE_INVALID_ARG);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto query = DB2Manager::GetMapLights(map_id);


        if (query.size() > 0) {

            std::stringstream s1;
            s1 << map_id;
            handler->PSendSysMessage(LANG_LOOKUP_AMBIANCE_ARE, s1.str().c_str());

            for (const auto& mapLight : query) {
                handler->PSendSysMessage(LANG_LOOKUP_AMBIANCE, mapLight->ID, mapLight->LightParamsID[0], mapLight->LightParamsID[1], mapLight->LightParamsID[2], mapLight->LightParamsID[3]);
            }          
        }
        else {
            handler->PSendSysMessage(LANG_LOOKUP_AMBIANCE_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    static bool HandleLookupSoundCommand(ChatHandler* handler, char const* args) {

        if (!args)
            return false;

        char const* name = strtok((char*)args, " ");
        if (!name)
            return false;

        std::string requestName = name;
        if (requestName.find('\'') != std::string::npos) {
            handler->PSendSysMessage(LANG_LOOKUP_SOUND_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else {
    
            auto soundKitsNames = DB2Manager::GetSoundKitsNames();
            if (soundKitsNames.size() > 0) {

                for (const auto& soundKitsName : soundKitsNames) {
                    std::string soundKitNameStr = soundKitsName.second;

                    std::transform(soundKitNameStr.begin(), soundKitNameStr.end(), soundKitNameStr.begin(),
                        [](unsigned char c) { return std::tolower(c); });

                    if (soundKitNameStr.find(requestName) != std::string::npos)
                        handler->PSendSysMessage(LANG_LOOKUP_SOUND, soundKitsName.first, soundKitNameStr);
                }
            }
            else {
                handler->PSendSysMessage(LANG_LOOKUP_SOUND_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }

            return true;
        }

    }

    static bool HandleLookupTerrainCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        uint32 counter = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        // search in Map.db2
        for (uint32 id = 0; id < sMapStore.GetNumRows(); ++id)
        {
            if (MapEntry const* mapInfo = sMapStore.LookupEntry(id))
            {
                int32 locale = handler->GetSessionDbcLocale();
                std::string name = mapInfo->MapName.Str[locale];
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wNamePart) && handler->GetSession())
                {
                    locale = 0;
                    for (; locale < TOTAL_LOCALES; ++locale)
                    {
                        if (locale == handler->GetSessionDbcLocale())
                            continue;

                        name = mapInfo->MapName.Str[locale];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wNamePart))
                            break;
                    }
                }

                if (locale < TOTAL_LOCALES)
                {
                    if (maxResults && counter == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    handler->PSendSysMessage(LANG_LOOKUP_TERRAIN, id, name.c_str());

                    ++counter;
                }
            }
        }

        if (!counter)
            handler->PSendSysMessage(LANG_LOOKUP_TERRAIN_ERROR);

        return true;
    }

    static bool HandleLookupPhaseOwnCommand(ChatHandler* handler, char const* args)
    {

        uint32 phaseIdOwner = 0;

        uint32 counter = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        bool limitReached = false;

        QueryResult query = WorldDatabase.PQuery("SELECT phaseId from phase_owner where accountOwner = %u", handler->GetSession()->GetAccountId());

        if (query) {

            do {

                Field* owner = query->Fetch();

                phaseIdOwner = owner[0].GetUInt32();

                QueryResult query3 = WorldDatabase.PQuery("SELECT map, name from phaseown_map where map = %u", phaseIdOwner);

                if (query3) {

                    do {

                        Field* result = query3->Fetch();

                        std::string phaseName = result[1].GetString();
                        uint16 mapid = result[0].GetUInt16();

                        std::ostringstream reply;

                        if (handler->GetSession())
                            reply << "  |cffffffff|Htele:" << phaseName.c_str() << "|h[" << phaseName.c_str() << "]|h|r" << "ID : " << "|cffffffff" << mapid << "|r";
                        else
                            reply << phaseName.c_str() << phaseName.c_str() << mapid;

                        handler->PSendSysMessage(LANG_LOOKUP_PHASE_OWN, reply.str().c_str());

                    } while (query3->NextRow());

                }
                else {
                    continue;
                }

            } while (query->NextRow());

        }
        else {
            handler->PSendSysMessage(LANG_LOOKUP_PHASE_OWN_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    static bool HandleLookupPhaseAutCommand(ChatHandler* handler, char const* args)
    {

        uint16 phaseIdAllow = 0;

        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);
        bool limitReached = false;

        QueryResult query = WorldDatabase.PQuery("SELECT phaseId from phase_allow l where playerId = %u AND NOT EXISTS"
            " (SELECT * from phase_owner s WHERE l.phaseId = s.phaseId AND s.accountOwner = %u)", handler->GetSession()->GetAccountId(), handler->GetSession()->GetAccountId());

        if (query) {

            do {

                Field* allow = query->Fetch();

                phaseIdAllow = allow[0].GetUInt16();

                QueryResult query3 = WorldDatabase.PQuery("SELECT map, name from phaseown_map where map = %u", phaseIdAllow);

                if (query3) {

                    do {

                        Field* result = query3->Fetch();

                        std::string phaseName = result[1].GetString();
                        uint16 mapid = result[0].GetUInt16();

                        std::ostringstream reply;

                        if (handler->GetSession())
                            reply << "  |cffffffff|Htele:" << phaseName.c_str() << "|h[" << phaseName.c_str() << "]|h|r" << "ID : " << "|cffffffff" << mapid << "|r";
                        else
                            reply << phaseName.c_str() << phaseName.c_str() << mapid;

                        if (!reply.str().empty())
                            handler->PSendSysMessage(LANG_LOOKUP_PHASE_AUT, reply.str().c_str());

                    } while (query3->NextRow());

                }
                else {
                    continue;
                }

            } while (query->NextRow());

        }
        else {
            handler->PSendSysMessage(LANG_LOOKUP_PHASE_AUT_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    static bool HandleLookupPlayerMailCommand(ChatHandler* handler, char const* args) 
    {

        if (!args)
            return false;

        char const* id = strtok((char*)args, " ");

        if (!id)
            return false;

        uint32 m_ID = 0;

        if (id) {

            std::string checkString = id;

            if (std::all_of(checkString.begin(), checkString.end(), ::isdigit)) {

                m_ID = atoi(id);

            }
            else {
                return false;
            }

        }

        QueryResult query = CharacterDatabase.PQuery("SELECT body FROM mail WHERE id = %u", m_ID);
        if (query) {
            Field* fields = query->Fetch();
            std::string str = fields[0].GetString();

            handler->PSendSysMessage(LANG_LOOKUP_PLAYER_MAIL, str.c_str());
        }
        else {
            handler->PSendSysMessage(LANG_LOOKUP_PLAYER_MAIL_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;

    }

    static bool HandleLookupItemForgeCommand(ChatHandler* handler, char const* args) {

        if (!*args)
            return false;

        std::string namePart = args;
        std::wstring wNamePart;

        // converting string that we try to find to lower case
        if (!Utf8toWStr(namePart, wNamePart))
            return false;

        wstrToLower(wNamePart);

        uint32 count = 0;
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        uint32 itemEntryId = 0;
        uint32 itemAppearanceId = 0;
        uint32 itemDisplayInfo = 0;

        for (uint32 id = 300001; id < sItemSparseStore.GetNumRows(); ++id)
        {
            ItemSparseEntry const* entryId = sItemSparseStore.LookupEntry(id);
            if (entryId) {
                int32 locale = handler->GetSessionDbcLocale();
                std::string name = entryId->Display.Str[locale];
                itemEntryId = entryId->ID;
                if (name.empty())
                    continue;

                if (Utf8FitTo(name, wNamePart))
                {

                    ItemModifiedAppearanceEntry const* modifiedAppearanceId = sItemModifiedAppearanceStore.LookupEntry(itemEntryId);
                    if (modifiedAppearanceId) {

                        itemAppearanceId = modifiedAppearanceId->ItemAppearanceID;

                    }

                    ItemAppearanceEntry const* appearanceId = sItemAppearanceStore.LookupEntry(itemAppearanceId);
                    if (appearanceId) {

                        itemDisplayInfo = appearanceId->ItemDisplayInfoID;

                    }

                    if (maxResults && ++count == maxResults)
                    {
                        handler->PSendSysMessage(LANG_COMMAND_LOOKUP_MAX_RESULTS, maxResults);
                        return true;
                    }

                    if (handler->GetSession())
                        handler->PSendSysMessage(LANG_LOOKUP_ITEM_FORGE, itemEntryId, itemEntryId, name.c_str(), itemAppearanceId, itemDisplayInfo);
                    else
                        handler->PSendSysMessage(LANG_LOOKUP_ITEM_FORGE, itemEntryId, itemEntryId, name.c_str(), itemAppearanceId, itemDisplayInfo);

                }

            }

        }

    return true;

    }

    static bool HandleLookupDupplicationCommand(ChatHandler* handler, char const* args)
    {

        if (!*args)
        {
            return false;
        }

        char const* str = strtok((char*)args, " ");
        if (!str)
            return false;

        std::string fix = str;
        if (fix.find('\'') != std::string::npos) {
            handler->PSendSysMessage(LANG_DUPPLICATION_LOOKUP_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        else {
            std::stringstream ss;
            ss << "'%" << str << "%'";
            std::string namePart = ss.str();

            QueryResult query = WorldDatabase.PQuery("SELECT entry,CASE isPrivate WHEN 0 THEN name WHEN 1 THEN CONCAT(name,' [P]') END as name, author FROM gameobject_dupplication_template WHERE LOWER(name) LIKE LOWER(%s) AND ( isPrivate = 0 OR account = %u)", namePart.c_str(), handler->GetSession()->GetAccountId());
            if (query) {
                do {
                    Field* result = query->Fetch();
                    uint32 entry = result[0].GetUInt32();
                    std::string name = result[1].GetString();
                    std::string author = result[2].GetString();
                    handler->PSendSysMessage(LANG_DUPPLICATION_LOOKUP_SUCCESS, entry, name.c_str(), author.c_str());
                    
                } while (query->NextRow());
            }
            else {

                handler->PSendSysMessage(LANG_DUPPLICATION_LOOKUP_ERROR);
                handler->SetSentErrorMessage(true);
                return false;
            }

            return true;
        }

    }

};

void AddSC_lookup_commandscript()
{
    new lookup_commandscript();
}
