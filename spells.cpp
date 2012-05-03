/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

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

#include "os.hpp"
#include "config.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include "object.hpp"
#include "room.hpp"
#include "affect.hpp"
#include "objproto.hpp"
#include "mobproto.hpp"
#include "world.hpp"

// temp externs
extern void damage(Character *ch, Character *victim, int dam, int dt);
extern std::string extra_bit_name (int extra_flags);
extern std::string affect_loc_name (int location);
extern bool is_same_group (Character * ach, Character * bch);

/*
 * Spell functions.
 */
void Character::spell_acid_blast (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  int dam;

  dam = dice (lvl, 6);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_armor (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->has_affect(sn))
    return;
  af.type = sn;
  af.duration = 24;
  af.modifier = -20;
  af.location = APPLY_AC;
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel someone protecting you.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_bless (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->position == POS_FIGHTING || victim->has_affect(sn))
    return;
  af.type = sn;
  af.duration = 6 + lvl;
  af.location = APPLY_HITROLL;
  af.modifier = lvl / 8;
  af.bitvector = 0;
  victim->affect_to_char(&af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 0 - lvl / 8;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel righteous.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_blindness (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_BLIND) || victim->saves_spell (lvl))
    return;

  af.type = sn;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.duration = 1 + lvl;
  af.bitvector = AFF_BLIND;
  victim->affect_to_char(&af);
  victim->send_to_char ("You are blinded!\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_burning_hands (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 14, 17, 20, 23, 26, 29,
    29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
    34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
    39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
    44, 44, 45, 45, 46, 46, 47, 47, 48, 48
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_call_lightning (int sn, int lvl, void *vo)
{
  Character *vch;

  if (!is_outside()) {
    send_to_char ("You must be out of doors.\r\n");
    return;
  }

  if (!g_world->is_raining()) {
    send_to_char ("You need bad weather.\r\n");
    return;
  }

  int dam = dice (lvl / 2, 8);

  send_to_char ("God's lightning strikes your foes!\r\n");
  act ("$n calls God's lightning to strike $s foes!",
    NULL, NULL, TO_ROOM);

  CharIter c, next;
  for (c = char_list.begin(); c != char_list.end(); c = next) {
    vch = *c;
    next = ++c;
    if (vch->in_room == NULL)
      continue;
    if (vch->in_room == in_room) {
      if (vch != this && (is_npc () ? !vch->is_npc () : vch->is_npc ()))
        damage (this, vch, vch->saves_spell (lvl) ? dam / 2 : dam, sn);
      continue;
    }

    if (vch->in_room->area == in_room->area && vch->is_outside()
      && vch->is_awake ())
      vch->send_to_char ("Lightning flashes in the sky.\r\n");
  }

  return;
}

void Character::spell_cause_light (int sn, int lvl, void *vo)
{
  damage (this, (Character *) vo, dice (1, 8) + lvl / 3, sn);
  return;
}

void Character::spell_cause_critical (int sn, int lvl, void *vo)
{
  damage (this, (Character *) vo, dice (3, 8) + lvl - 6, sn);
  return;
}

void Character::spell_cause_serious (int sn, int lvl, void *vo)
{
  damage (this, (Character *) vo, dice (2, 8) + lvl / 2, sn);
  return;
}

void Character::spell_change_sex (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->has_affect(sn))
    return;
  af.type = sn;
  af.duration = 10 * lvl;
  af.location = APPLY_SEX;
  do {
    af.modifier = number_range (0, 2) - victim->sex;
  }
  while (af.modifier == 0);
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel different.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_charm_person (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim == this) {
    send_to_char ("You like yourself even better!\r\n");
    return;
  }

  if (victim->is_affected (AFF_CHARM)
    || is_affected (AFF_CHARM)
    || lvl < victim->level || victim->saves_spell (lvl))
    return;

  if (victim->master)
    victim->stop_follower();
  victim->add_follower(this);
  af.type = sn;
  af.duration = number_fuzzy (lvl / 4);
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_CHARM;
  victim->affect_to_char(&af);
  act ("Isn't $n just so nice?", NULL, victim, TO_VICT);
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_chill_touch (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 6, 7, 8, 9, 12, 13, 13, 13,
    14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
    17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
    20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
    24, 24, 24, 25, 25, 25, 26, 26, 26, 27
  };
  Affect af;

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (!victim->saves_spell (lvl)) {
    af.type = sn;
    af.duration = 6;
    af.location = APPLY_STR;
    af.modifier = -1;
    af.bitvector = 0;
    victim->affect_join (&af);
  } else {
    dam /= 2;
  }

  damage (this, victim, dam, sn);
  return;
}

void Character::spell_colour_spray (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
    58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
    65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
    73, 73, 74, 75, 76, 76, 77, 78, 79, 79
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;

  damage (this, victim, dam, sn);
  return;
}

void Character::spell_continual_light (int sn, int lvl, void *vo)
{
  Object *light;

  light = get_obj_index(OBJ_VNUM_LIGHT_BALL)->create_object(0);
  light->obj_to_room (in_room);
  act ("$n twiddles $s thumbs and $p appears.", light, NULL, TO_ROOM);
  act ("You twiddle your thumbs and $p appears.", light, NULL, TO_CHAR);
  return;
}

void Character::spell_control_weather (int sn, int lvl, void *vo)
{
  int change = dice (lvl / 3, 4);
  if (!str_cmp (target_name, "better"))
    change *= 1;
  else if (!str_cmp (target_name, "worse"))
    change *= -1;
  else
    send_to_char ("Do you want it to get better or worse?\r\n");

  g_world->change_weather(change);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_create_food (int sn, int lvl, void *vo)
{
  Object* mushroom = get_obj_index(OBJ_VNUM_MUSHROOM)->create_object(0);
  mushroom->value[0] = 5 + lvl;
  mushroom->obj_to_room (in_room);
  act ("$p suddenly appears.", mushroom, NULL, TO_ROOM);
  act ("$p suddenly appears.", mushroom, NULL, TO_CHAR);
  return;
}

void Character::spell_create_spring (int sn, int lvl, void *vo)
{
  Object* spring = get_obj_index(OBJ_VNUM_SPRING)->create_object(0);
  spring->timer = lvl;
  spring->obj_to_room (in_room);
  act ("$p flows from the ground.", spring, NULL, TO_ROOM);
  act ("$p flows from the ground.", spring, NULL, TO_CHAR);
  return;
}

void Character::spell_create_water (int sn, int lvl, void *vo)
{
  Object *obj = (Object *) vo;

  if (obj->item_type != ITEM_DRINK_CON) {
    send_to_char ("It is unable to hold water.\r\n");
    return;
  }

  if (obj->value[2] != LIQ_WATER && obj->value[1] != 0) {
    send_to_char ("It contains some other liquid.\r\n");
    return;
  }

  int water = std::min (lvl * (g_world->is_raining() ? 4 : 2),
    obj->value[0] - obj->value[1]
    );

  if (water > 0) {
    obj->value[2] = LIQ_WATER;
    obj->value[1] += water;
    if (!is_name ("water", obj->name)) {
      obj->name = obj->name + " water";
    }
    act ("$p is filled.", obj, NULL, TO_CHAR);
  }

  return;
}

void Character::spell_cure_blindness (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  if (!victim->has_affect(skill_lookup("blindness")))
    return;
  victim->affect_strip (skill_lookup("blindness"));
  victim->send_to_char ("Your vision returns!\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_cure_critical (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int heal = dice (3, 8) + lvl - 6;
  victim->hit = std::min (victim->hit + heal, victim->max_hit);
  victim->update_pos();
  victim->send_to_char ("You feel better!\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_cure_light (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int heal = dice (1, 8) + lvl / 3;
  victim->hit = std::min (victim->hit + heal, victim->max_hit);
  victim->update_pos();
  victim->send_to_char ("You feel better!\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_cure_poison (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  int pois = skill_lookup("poison");

  if (victim->has_affect(pois)) {
    victim->affect_strip (pois);
    act ("$N looks better.", NULL, victim, TO_NOTVICT);
    victim->send_to_char ("A warm feeling runs through your body.\r\n");
    send_to_char ("Ok.\r\n");
  }
  return;
}

void Character::spell_cure_serious (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int heal = dice (2, 8) + lvl / 2;
  victim->hit = std::min (victim->hit + heal, victim->max_hit);
  victim->update_pos();
  victim->send_to_char ("You feel better!\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_curse (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_CURSE) || victim->saves_spell (lvl))
    return;
  af.type = sn;
  af.duration = 4 * lvl;
  af.location = APPLY_HITROLL;
  af.modifier = -1;
  af.bitvector = AFF_CURSE;
  victim->affect_to_char(&af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 1;
  victim->affect_to_char(&af);

  victim->send_to_char ("You feel unclean.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_detect_evil (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_DETECT_EVIL))
    return;
  af.type = sn;
  af.duration = lvl;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your eyes tingle.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_detect_hidden (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_DETECT_HIDDEN))
    return;
  af.type = sn;
  af.duration = lvl;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your awareness improves.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_detect_invis (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_DETECT_INVIS))
    return;
  af.type = sn;
  af.duration = lvl;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your eyes tingle.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_detect_magic (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_DETECT_MAGIC))
    return;
  af.type = sn;
  af.duration = lvl;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your eyes tingle.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_detect_poison (int sn, int lvl, void *vo)
{
  Object *obj = (Object *) vo;

  if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD) {
    if (obj->value[3] != 0)
      send_to_char ("You smell poisonous fumes.\r\n");
    else
      send_to_char ("It looks very delicious.\r\n");
  } else {
    send_to_char ("It doesn't look poisoned.\r\n");
  }

  return;
}

void Character::spell_dispel_magic (int sn, int lvl, void *vo)
{
  send_to_char ("Sorry but this spell has been disabled.\r\n");
  return;
}

void Character::spell_dispel_evil (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  if (!is_npc () && is_evil ())
    victim = this;

  if (victim->is_good ()) {
    act ("God protects $N.", NULL, victim, TO_ROOM);
    return;
  }

  if (victim->is_neutral ()) {
    act ("$N does not seem to be affected.", NULL, victim, TO_CHAR);
    return;
  }

  int dam = dice (lvl, 4);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_earthquake (int sn, int lvl, void *vo)
{
  send_to_char ("The earth trembles beneath your feet!\r\n");
  act ("$n makes the earth tremble and shiver.", NULL, NULL, TO_ROOM);

  CharIter c, next;
  for (c = char_list.begin(); c != char_list.end(); c = next) {
    Character* vch = *c;
    next = ++c;
    if (vch->in_room == NULL)
      continue;
    if (vch->in_room == in_room) {
      if (vch != this && (is_npc () ? !vch->is_npc () : vch->is_npc ()))
        damage (this, vch, lvl + dice (2, 8), sn);
      continue;
    }

    if (vch->in_room->area == in_room->area)
      vch->send_to_char ("The earth trembles and shivers.\r\n");
  }

  return;
}

void Character::spell_enchant_weapon (int sn, int lvl, void *vo)
{
  Object *obj = (Object *) vo;
  Affect *paf;

  if (obj->item_type != ITEM_WEAPON || obj->is_obj_stat(ITEM_MAGIC)
    || !obj->affected.empty())
    return;

  paf = new Affect();

  paf->type = sn;
  paf->duration = -1;
  paf->location = APPLY_HITROLL;
  paf->modifier = lvl / 5;
  paf->bitvector = 0;
  obj->affected.push_back(paf);

  paf = new Affect();

  paf->type = -1;
  paf->duration = -1;
  paf->location = APPLY_DAMROLL;
  paf->modifier = lvl / 10;
  paf->bitvector = 0;
  obj->affected.push_back(paf);
  obj->level = number_fuzzy (level - 5);

  if (is_good ()) {
    SET_BIT (obj->extra_flags, ITEM_ANTI_EVIL);
    act ("$p glows blue.", obj, NULL, TO_CHAR);
  } else if (is_evil ()) {
    SET_BIT (obj->extra_flags, ITEM_ANTI_GOOD);
    act ("$p glows red.", obj, NULL, TO_CHAR);
  } else {
    SET_BIT (obj->extra_flags, ITEM_ANTI_EVIL);
    SET_BIT (obj->extra_flags, ITEM_ANTI_GOOD);
    act ("$p glows yellow.", obj, NULL, TO_CHAR);
  }

  send_to_char ("Ok.\r\n");
  return;
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void Character::spell_energy_drain (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  int dam;

  if (victim->saves_spell (lvl))
    return;

  alignment = std::max (-1000, alignment - 200);
  if (victim->level <= 2) {
    dam = hit + 1;
  } else {
    victim->gain_exp(0 - number_range (lvl / 2, 3 * lvl / 2));
    victim->mana /= 2;
    victim->move /= 2;
    dam = dice (1, lvl);
    hit += dam;
  }

  damage (this, victim, dam, sn);

  return;
}

void Character::spell_fireball (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
    60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
    92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
    112, 114, 116, 118, 120, 122, 124, 126, 128, 130
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_flamestrike (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int dam = dice (6, lvl);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_faerie_fire (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_FAERIE_FIRE))
    return;
  af.type = sn;
  af.duration = lvl;
  af.location = APPLY_AC;
  af.modifier = 2 * lvl;
  af.bitvector = AFF_FAERIE_FIRE;
  victim->affect_to_char(&af);
  victim->send_to_char ("You are surrounded by a pink outline.\r\n");
  victim->act ("$n is surrounded by a pink outline.", NULL, NULL, TO_ROOM);
  return;
}

void Character::spell_faerie_fog (int sn, int lvl, void *vo)
{
  act ("$n conjures a cloud of purple smoke.", NULL, NULL, TO_ROOM);
  send_to_char ("You conjure a cloud of purple smoke.\r\n");

  CharIter ich;
  for (ich = in_room->people.begin(); ich != in_room->people.end(); ich++) {
    if (*ich == this || (*ich)->saves_spell (lvl))
      continue;

    (*ich)->affect_strip (skill_lookup("invis"));
    (*ich)->affect_strip (skill_lookup("mass invis"));
    (*ich)->affect_strip (skill_lookup("sneak"));
    REMOVE_BIT ((*ich)->affected_by, AFF_HIDE);
    REMOVE_BIT ((*ich)->affected_by, AFF_INVISIBLE);
    REMOVE_BIT ((*ich)->affected_by, AFF_SNEAK);
    (*ich)->act ("$n is revealed!", NULL, NULL, TO_ROOM);
    (*ich)->send_to_char ("You are revealed!\r\n");
  }

  return;
}

void Character::spell_fly (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_FLYING))
    return;
  af.type = sn;
  af.duration = lvl + 3;
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_FLYING;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your feet rise off the ground.\r\n");
  victim->act ("$n's feet rise off the ground.", NULL, NULL, TO_ROOM);
  return;
}

void Character::spell_gate (int sn, int lvl, void *vo)
{
  get_mob_index(MOB_VNUM_VAMPIRE)->create_mobile()->char_to_room(in_room);
  return;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void Character::spell_general_purpose (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int dam = number_range (25, 100);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_giant_strength (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->has_affect(sn))
    return;
  af.type = sn;
  af.duration = lvl;
  af.location = APPLY_STR;
  af.modifier = 1 + (lvl >= 18) + (lvl >= 25);
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel stronger.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_harm (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  int dam;

  dam = std::max (20, victim->hit - dice (1, 4));
  if (victim->saves_spell (lvl))
    dam = std::min (50, dam / 4);
  dam = std::min (100, dam);
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_heal (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  victim->hit = std::min (victim->hit + 100, victim->max_hit);
  victim->update_pos();
  victim->send_to_char ("A warm feeling fills your body.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void Character::spell_high_explosive (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int dam = number_range (30, 120);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_identify (int sn, int lvl, void *vo)
{
  Object *obj = (Object *) vo;
  char buf[MAX_STRING_LENGTH];

  snprintf (buf, sizeof buf,
    "Object '%s' is type %s, extra flags %s.\r\nWeight is %d, value is %d, lvl is %d.\r\n",
    obj->name.c_str(),
    obj->item_type_name().c_str(),
    extra_bit_name (obj->extra_flags).c_str(), obj->weight, obj->cost, obj->level);
  send_to_char (buf);

  switch (obj->item_type) {
  case ITEM_SCROLL:
  case ITEM_POTION:
    snprintf (buf, sizeof buf, "Level %d spells of:", obj->value[0]);
    send_to_char (buf);

    if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL) {
      send_to_char (" '");
      send_to_char (skill_table[obj->value[1]].name);
      send_to_char ("'");
    }

    if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL) {
      send_to_char (" '");
      send_to_char (skill_table[obj->value[2]].name);
      send_to_char ("'");
    }

    if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
      send_to_char (" '");
      send_to_char (skill_table[obj->value[3]].name);
      send_to_char ("'");
    }

    send_to_char (".\r\n");
    break;

  case ITEM_WAND:
  case ITEM_STAFF:
    snprintf (buf, sizeof buf, "Has %d(%d) charges of level %d",
      obj->value[1], obj->value[2], obj->value[0]);
    send_to_char (buf);

    if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
      send_to_char (" '");
      send_to_char (skill_table[obj->value[3]].name);
      send_to_char ("'");
    }

    send_to_char (".\r\n");
    break;

  case ITEM_WEAPON:
    snprintf (buf, sizeof buf, "Damage is %d to %d (average %d).\r\n",
      obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2);
    send_to_char (buf);
    break;

  case ITEM_ARMOR:
    snprintf (buf, sizeof buf, "Armor class is %d.\r\n", obj->value[0]);
    send_to_char (buf);
    break;
  }

  AffIter af;
  for (af = obj->pIndexData->affected.begin(); af != obj->pIndexData->affected.end(); af++) {
    if ((*af)->location != APPLY_NONE && (*af)->modifier != 0) {
      snprintf (buf, sizeof buf, "Affects %s by %d.\r\n",
        affect_loc_name ((*af)->location).c_str(), (*af)->modifier);
      send_to_char (buf);
    }
  }

  for (af = obj->affected.begin(); af != obj->affected.end(); af++) {
    if ((*af)->location != APPLY_NONE && (*af)->modifier != 0) {
      snprintf (buf, sizeof buf, "Affects %s by %d.\r\n",
        affect_loc_name ((*af)->location).c_str(), (*af)->modifier);
      send_to_char (buf);
    }
  }

  return;
}

void Character::spell_infravision (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_INFRARED))
    return;
  act ("$n's eyes glow red.\r\n", NULL, NULL, TO_ROOM);
  af.type = sn;
  af.duration = 2 * lvl;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INFRARED;
  victim->affect_to_char(&af);
  victim->send_to_char ("Your eyes glow red.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_invis (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_INVISIBLE))
    return;

  victim->act ("$n fades out of existence.", NULL, NULL, TO_ROOM);
  af.type = sn;
  af.duration = 24;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INVISIBLE;
  victim->affect_to_char(&af);
  victim->send_to_char ("You fade out of existence.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_know_alignment (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  const char *msg;

  int ap = victim->alignment;

  if (ap > 700)
    msg = "$N has an aura as white as the driven snow.";
  else if (ap > 350)
    msg = "$N is of excellent moral character.";
  else if (ap > 100)
    msg = "$N is often kind and thoughtful.";
  else if (ap > -100)
    msg = "$N doesn't have a firm moral commitment.";
  else if (ap > -350)
    msg = "$N lies to $S friends.";
  else if (ap > -700)
    msg = "$N's slash DISEMBOWELS you!";
  else
    msg = "I'd rather just not say anything at all about $N.";

  act (msg, NULL, victim, TO_CHAR);
  return;
}

void Character::spell_lightning_bolt (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 25, 28,
    31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
    44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
    51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
    58, 58, 59, 60, 60, 61, 62, 62, 63, 64
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_locate_object (int sn, int lvl, void *vo)
{
  std::string buf;
  bool found = false;
  ObjIter o;
  for (o = object_list.begin(); o != object_list.end(); o++) {
    Object *in_obj;

    if (!can_see_obj(*o) || !is_name (target_name, (*o)->name))
      continue;

    found = true;

    for (in_obj = *o; in_obj->in_obj != NULL; in_obj = in_obj->in_obj) ;

    if (in_obj->carried_by != NULL) {
      buf += (*o)->short_descr + " carried by " + in_obj->carried_by->describe_to(this) + "\r\n";
    } else {
      buf += (*o)->short_descr + " in " +
        (in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name.c_str()) +
        ".\r\n";
    }

    buf[0] = toupper (buf[0]);
    send_to_char (buf);
  }

  if (!found)
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");

  return;
}

void Character::spell_magic_missile (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const sh_int dam_each[] = {
    0,
    3, 3, 4, 4, 5, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 14, 14, 14, 14, 14
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_mass_invis (int sn, int lvl, void *vo)
{
  Affect af;

  CharIter gch;
  for (gch = in_room->people.begin(); gch != in_room->people.end(); gch++) {
    if (!is_same_group (*gch, this) || (*gch)->is_affected (AFF_INVISIBLE))
      continue;
    (*gch)->act ("$n slowly fades out of existence.", NULL, NULL, TO_ROOM);
    (*gch)->send_to_char ("You slowly fade out of existence.\r\n");
    af.type = sn;
    af.duration = 24;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INVISIBLE;
    (*gch)->affect_to_char(&af);
  }
  send_to_char ("Ok.\r\n");

  return;
}

void Character::spell_null (int sn, int lvl, void *vo)
{
  send_to_char ("That's not a spell!\r\n");
  return;
}

void Character::spell_pass_door (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_PASS_DOOR))
    return;
  af.type = sn;
  af.duration = number_fuzzy (lvl / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PASS_DOOR;
  victim->affect_to_char(&af);
  victim->act ("$n turns translucent.", NULL, NULL, TO_ROOM);
  victim->send_to_char ("You turn translucent.\r\n");
  return;
}

void Character::spell_poison (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->saves_spell (lvl))
    return;
  af.type = sn;
  af.duration = lvl;
  af.location = APPLY_STR;
  af.modifier = -2;
  af.bitvector = AFF_POISON;
  victim->affect_join (&af);
  victim->send_to_char ("You feel very sick.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_protection (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_PROTECT))
    return;
  af.type = sn;
  af.duration = 24;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PROTECT;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel protected.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_refresh (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  victim->move = std::min (victim->move + lvl, victim->max_move);
  victim->send_to_char ("You feel less tired.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

void Character::spell_remove_curse (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  if (victim->has_affect(skill_lookup("curse"))) {
    victim->affect_strip (skill_lookup("curse"));
    victim->send_to_char ("You feel better.\r\n");
    if (this != victim)
      send_to_char ("Ok.\r\n");
  }

  return;
}

void Character::spell_sanctuary (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_SANCTUARY))
    return;
  af.type = sn;
  af.duration = number_fuzzy (lvl / 8);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SANCTUARY;
  victim->affect_to_char(&af);
  victim->act ("$n is surrounded by a white aura.", NULL, NULL, TO_ROOM);
  victim->send_to_char ("You are surrounded by a white aura.\r\n");
  return;
}

void Character::spell_shield (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->has_affect(sn))
    return;
  af.type = sn;
  af.duration = 8 + lvl;
  af.location = APPLY_AC;
  af.modifier = -20;
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->act ("$n is surrounded by a force shield.", NULL, NULL, TO_ROOM);
  victim->send_to_char ("You are surrounded by a force shield.\r\n");
  return;
}

void Character::spell_shocking_grasp (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  static const int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 20, 25, 29, 33,
    36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
    43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
    48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
    53, 53, 54, 54, 55, 55, 56, 56, 57, 57
  };

  lvl = std::min (lvl, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  lvl = std::max (0, lvl);
  int dam = number_range (dam_each[lvl] / 2, dam_each[lvl] * 2);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_sleep (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->is_affected (AFF_SLEEP)
    || lvl < victim->level || victim->saves_spell (lvl))
    return;

  af.type = sn;
  af.duration = 4 + lvl;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SLEEP;
  victim->affect_join (&af);

  if (victim->is_awake ()) {
    victim->send_to_char ("You feel very sleepy ..... zzzzzz.\r\n");
    victim->act ("$n goes to sleep.", NULL, NULL, TO_ROOM);
    victim->position = POS_SLEEPING;
  }

  return;
}

void Character::spell_stone_skin (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (has_affect(sn))
    return;
  af.type = sn;
  af.duration = lvl;
  af.location = APPLY_AC;
  af.modifier = -40;
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->act ("$n's skin turns to stone.", NULL, NULL, TO_ROOM);
  victim->send_to_char ("Your skin turns to stone.\r\n");
  return;
}

void Character::spell_summon (int sn, int lvl, void *vo)
{
  Character *victim;

  if ((victim = get_char_world (target_name)) == NULL
    || victim == this
    || victim->in_room == NULL
    || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
    || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
    || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
    || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
    || victim->level >= lvl + 3
    || victim->fighting != NULL
    || victim->in_room->area != in_room->area
    || (victim->is_npc () && victim->saves_spell (lvl))) {
    send_to_char ("You failed.\r\n");
    return;
  }

  victim->act ("$n disappears suddenly.", NULL, NULL, TO_ROOM);
  victim->char_from_room();
  victim->char_to_room(in_room);
  victim->act ("$n arrives suddenly.", NULL, NULL, TO_ROOM);
  act ("$N has summoned you!", NULL, victim, TO_VICT);
  victim->do_look ("auto");
  return;
}

void Character::spell_teleport (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  if (victim->in_room == NULL
    || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
    || (!is_npc () && victim->fighting != NULL)
    || (victim != this
      && (victim->saves_spell (lvl) || victim->saves_spell (lvl)))) {
    send_to_char ("You failed.\r\n");
    return;
  }

  Room *pRoomIndex;
  for (;;) {
    pRoomIndex = get_room_index (number_range (0, 65535));
    if (pRoomIndex != NULL)
      if (!IS_SET (pRoomIndex->room_flags, ROOM_PRIVATE)
        && !IS_SET (pRoomIndex->room_flags, ROOM_SOLITARY))
        break;
  }

  victim->act ("$n slowly fades out of existence.", NULL, NULL, TO_ROOM);
  victim->char_from_room();
  victim->char_to_room(pRoomIndex);
  victim->act ("$n slowly fades into existence.", NULL, NULL, TO_ROOM);
  victim->do_look ("auto");
  return;
}

void Character::spell_ventriloquate (int sn, int lvl, void *vo)
{
  std::string speaker;

  target_name = one_argument (target_name, speaker);

  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  snprintf (buf1, sizeof buf1, "%s says '%s'.\r\n", speaker.c_str(), target_name.c_str());
  snprintf (buf2, sizeof buf2, "Someone makes %s say '%s'.\r\n", speaker.c_str(), target_name.c_str());
  buf1[0] = toupper (buf1[0]);

  CharIter vch;
  for (vch = in_room->people.begin(); vch != in_room->people.end(); vch++) {
    if (!is_name (speaker, (*vch)->name))
      (*vch)->send_to_char ((*vch)->saves_spell (lvl) ? buf2 : buf1);
  }

  return;
}

void Character::spell_weaken (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;
  Affect af;

  if (victim->has_affect(sn) || victim->saves_spell (lvl))
    return;
  af.type = sn;
  af.duration = lvl / 2;
  af.location = APPLY_STR;
  af.modifier = -2;
  af.bitvector = 0;
  victim->affect_to_char(&af);
  victim->send_to_char ("You feel weaker.\r\n");
  if (this != victim)
    send_to_char ("Ok.\r\n");
  return;
}

/*
 * This is for muds that _want_ scrolls of recall.
 * Ick.
 */
void Character::spell_word_of_recall (int sn, int lvl, void *vo)
{
  ((Character *) vo)->do_recall ("");
  return;
}

/*
 * NPC spells.
 */
void Character::spell_acid_breath (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  if (number_percent () < 2 * lvl && !victim->saves_spell (lvl)) {
    Object *obj_lose;
    ObjIter o, onext;
    for (o = victim->carrying.begin(); o != victim->carrying.end(); o = onext) {
      obj_lose = *o;
      onext = ++o;
      int iWear;

      if (number_percent() <= 75)
        continue;

      switch (obj_lose->item_type) {
      case ITEM_ARMOR:
        if (obj_lose->value[0] > 0) {
          victim->act ("$p is pitted and etched!", obj_lose, NULL, TO_CHAR);
          if ((iWear = obj_lose->wear_loc) != WEAR_NONE)
            victim->armor -= obj_lose->apply_ac (iWear);
          obj_lose->value[0] -= 1;
          obj_lose->cost = 0;
          if (iWear != WEAR_NONE)
            victim->armor += obj_lose->apply_ac (iWear);
        }
        break;

      case ITEM_CONTAINER:
        victim->act ("$p fumes and dissolves!", obj_lose, NULL, TO_CHAR);
        obj_lose->extract_obj ();
        break;
      }
    }
  }

  int hpch = std::max (10, hit);
  int dam = number_range (hpch / 16 + 1, hpch / 8);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_fire_breath (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  if (number_percent () < 2 * lvl && !victim->saves_spell (lvl)) {
    Object *obj_lose;
    ObjIter o, onext;
    for (o = victim->carrying.begin(); o != victim->carrying.end(); o = onext) {
      obj_lose = *o;
      onext = ++o;
      const char *msg;

      if (number_percent() <= 75)
        continue;

      switch (obj_lose->item_type) {
      default:
        continue;
      case ITEM_CONTAINER:
        msg = "$p ignites and burns!";
        break;
      case ITEM_POTION:
        msg = "$p bubbles and boils!";
        break;
      case ITEM_SCROLL:
        msg = "$p crackles and burns!";
        break;
      case ITEM_STAFF:
        msg = "$p smokes and chars!";
        break;
      case ITEM_WAND:
        msg = "$p sparks and sputters!";
        break;
      case ITEM_FOOD:
        msg = "$p blackens and crisps!";
        break;
      case ITEM_PILL:
        msg = "$p melts and drips!";
        break;
      }

      victim->act (msg, obj_lose, NULL, TO_CHAR);
      obj_lose->extract_obj ();
    }
  }

  int hpch = std::max (10, hit);
  int dam = number_range (hpch / 16 + 1, hpch / 8);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_frost_breath (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  if (number_percent () < 2 * lvl && !victim->saves_spell (lvl)) {
    Object *obj_lose;
    ObjIter o, onext;
    for (o = victim->carrying.begin(); o != victim->carrying.end(); o = onext) {
      obj_lose = *o;
      onext = ++o;
      const char *msg;

      if (number_percent() <= 75)
        continue;

      switch (obj_lose->item_type) {
      default:
        continue;
      case ITEM_CONTAINER:
      case ITEM_DRINK_CON:
      case ITEM_POTION:
        msg = "$p freezes and shatters!";
        break;
      }

      victim->act (msg, obj_lose, NULL, TO_CHAR);
      obj_lose->extract_obj ();
    }
  }

  int hpch = std::max (10, hit);
  int dam = number_range (hpch / 16 + 1, hpch / 8);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

void Character::spell_gas_breath (int sn, int lvl, void *vo)
{
  Character *vch;

  CharIter rch, next;
  for (rch = in_room->people.begin(); rch != in_room->people.end(); rch = next) {
    vch = *rch;
    next = ++rch;
    if (is_npc () ? !vch->is_npc () : vch->is_npc ()) {
      int hpch = std::max (10, hit);
      int dam = number_range (hpch / 16 + 1, hpch / 8);
      if (vch->saves_spell (lvl))
        dam /= 2;
      damage (this, vch, dam, sn);
    }
  }
  return;
}

void Character::spell_lightning_breath (int sn, int lvl, void *vo)
{
  Character *victim = (Character *) vo;

  int hpch = std::max (10, hit);
  int dam = number_range (hpch / 16 + 1, hpch / 8);
  if (victim->saves_spell (lvl))
    dam /= 2;
  damage (this, victim, dam, sn);
  return;
}

