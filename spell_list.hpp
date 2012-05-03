/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 01/02/2007
 \version 1.5
 \remarks
  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#if defined(SPELL_LIST)
  #define SPELL(spell)  { #spell, spell },
  #undef SPELL_LIST
#elif defined(SPELL_DECL)
  #define SPELL(spell)  void spell(int sn, int level, void *vo);
  #undef SPELL_DECL
#else
  #error
#endif

SPELL (spell_acid_blast)
SPELL (spell_armor)
SPELL (spell_bless)
SPELL (spell_blindness)
SPELL (spell_burning_hands)
SPELL (spell_call_lightning)
SPELL (spell_cause_light)
SPELL (spell_cause_critical)
SPELL (spell_cause_serious)
SPELL (spell_change_sex)
SPELL (spell_charm_person)
SPELL (spell_chill_touch)
SPELL (spell_colour_spray)
SPELL (spell_continual_light)
SPELL (spell_control_weather)
SPELL (spell_create_food)
SPELL (spell_create_spring)
SPELL (spell_create_water)
SPELL (spell_cure_blindness)
SPELL (spell_cure_critical)
SPELL (spell_cure_light)
SPELL (spell_cure_poison)
SPELL (spell_cure_serious)
SPELL (spell_curse)
SPELL (spell_detect_evil)
SPELL (spell_detect_hidden)
SPELL (spell_detect_invis)
SPELL (spell_detect_magic)
SPELL (spell_detect_poison)
SPELL (spell_dispel_magic)
SPELL (spell_dispel_evil)
SPELL (spell_earthquake)
SPELL (spell_enchant_weapon)
SPELL (spell_energy_drain)
SPELL (spell_fireball)
SPELL (spell_flamestrike)
SPELL (spell_faerie_fire)
SPELL (spell_faerie_fog)
SPELL (spell_fly)
SPELL (spell_gate)
SPELL (spell_general_purpose)
SPELL (spell_giant_strength)
SPELL (spell_harm)
SPELL (spell_heal)
SPELL (spell_high_explosive)
SPELL (spell_identify)
SPELL (spell_infravision)
SPELL (spell_invis)
SPELL (spell_know_alignment)
SPELL (spell_lightning_bolt)
SPELL (spell_locate_object)
SPELL (spell_magic_missile)
SPELL (spell_mass_invis)
SPELL (spell_null)
SPELL (spell_pass_door)
SPELL (spell_poison)
SPELL (spell_protection)
SPELL (spell_refresh)
SPELL (spell_remove_curse)
SPELL (spell_sanctuary)
SPELL (spell_shield)
SPELL (spell_shocking_grasp)
SPELL (spell_sleep)
SPELL (spell_stone_skin)
SPELL (spell_summon)
SPELL (spell_teleport)
SPELL (spell_ventriloquate)
SPELL (spell_weaken)
SPELL (spell_word_of_recall)
SPELL (spell_acid_breath)
SPELL (spell_fire_breath)
SPELL (spell_frost_breath)
SPELL (spell_gas_breath)
SPELL (spell_lightning_breath)

#undef SPELL

