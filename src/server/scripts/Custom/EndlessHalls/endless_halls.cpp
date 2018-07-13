/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "GameObject.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "endless_halls.h"
#include "TemporarySummon.h"
#include "PhasingHandler.h"

uint32 SpellOrbEntries[] =
{
    SPELL_BLUE_ORB,
    SPELL_RED_ORB,
    SPELL_GREEN_ORB,
    SPELL_YELLOW_ORB,
    SPELL_VIOLET_ORB
};


class spell_endlesshalls_direction : public SpellScriptLoader
{
public:
    spell_endlesshalls_direction() : SpellScriptLoader("spell_endlesshalls_direction") { }

    class spell_endlesshalls_direction_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_endlesshalls_direction_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit* target = GetHitPlayer();
            Player* player = GetHitPlayer();
            if (player)
            { 
                target->CastSpell(target, SPELL_DIRECT_BLACKOUT, true);
                uint32 direction = 0;

                switch (GetSpellInfo()->Id)
                {
                    case SPELL_DIRECT_NORTH:
                        direction = SPELL_DIRECT_NORTH;
                        break;
                    case SPELL_DIRECT_SOUTH:
                        direction = SPELL_DIRECT_SOUTH;
                        break;
                    case SPELL_DIRECT_EAST:
                        direction = SPELL_DIRECT_EAST;
                        break;
                    case SPELL_DIRECT_WEST:
                        direction = SPELL_DIRECT_WEST;
                        break;
                    default:
                        return;
                }

                if (InstanceScript* instance = player->GetInstanceScript())
                {
                    instance->SetData(DATA_DIRECTION, direction);
                }
            }
            
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_endlesshalls_direction_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_endlesshalls_direction_SpellScript();
    }
};

class spell_endlesshalls_teleportout : public SpellScriptLoader
{
public:
    spell_endlesshalls_teleportout() : SpellScriptLoader("spell_endlesshalls_teleportout") { }

    class spell_endlesshalls_teleportout_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_endlesshalls_teleportout_SpellScript);

        void OnCast(SpellMissInfo missInfo)
        {
            if (GetHitPlayer())
            {
                GetHitPlayer()->RemoveAllAuras();
				PhasingHandler::RemovePhase(GetHitPlayer()->ToPlayer(), PHASE_GAME, true);
            }
        }

        void Register() override
        {
            BeforeHit += BeforeSpellHitFn(spell_endlesshalls_teleportout_SpellScript::OnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_endlesshalls_teleportout_SpellScript();
    }
};

class spell_endlesshalls_auracheck : public SpellScriptLoader
{
public:
    spell_endlesshalls_auracheck() : SpellScriptLoader("spell_endlesshalls_auracheck") { }

    class spell_endlesshalls_auracheck_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_endlesshalls_auracheck_AuraScript);

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (GetCaster()->ToPlayer() && GetCaster()->ToPlayer()->GetInstanceSave(1764) && GetCaster()->ToPlayer()->GetMapId() != (uint32)1764)
            {
                // Whisper to player ?
                GetCaster()->ToPlayer()->TeleportTo(TAKE_ME_BACK);
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_endlesshalls_auracheck_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_endlesshalls_auracheck_AuraScript();
    }
};

class go_endlesshalls_orbe : public GameObjectScript
{
public:
    go_endlesshalls_orbe() : GameObjectScript("go_endlesshalls_orbe") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        // Keep only 1 orb when picking orb.
        uint32 checkSpell = go->GetGOInfo()->goober.spell;
        for (uint32 spellid : SpellOrbEntries)
        {
            if (checkSpell != spellid)
                player->RemoveAurasDueToSpell(spellid);
            else
                player->AddAura(spellid, player);
        }

        if (InstanceScript* instance = go->GetInstanceScript())
        {
            instance->SetData(DATA_PICK_ORBE, go->GetEntry());
            return false;
        }

        return false;
    }
};

class go_endlesshalls_rune : public GameObjectScript
{
public:
    go_endlesshalls_rune() : GameObjectScript("go_endlesshalls_rune") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
        go->SetGoState(GO_STATE_ACTIVE);

        if (InstanceScript* instance = go->GetInstanceScript())
        {
            instance->SetData(DATA_PICK_RUNE, go->GetEntry());
            return false;
        }

        return false;
    }
};


void AddSC_endless_halls()
{
    new spell_endlesshalls_direction();
    new spell_endlesshalls_teleportout();
    new spell_endlesshalls_auracheck();
    new go_endlesshalls_orbe();
    new go_endlesshalls_rune();
}
