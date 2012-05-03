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
#include "io.hpp"
#include "utils.hpp"

#include "pcdata.hpp"
#include "affect.hpp"
#include "area.hpp"
#include "room.hpp"
#include "mobproto.hpp"
#include "objproto.hpp"
#include "exit.hpp"
#include "extra.hpp"
#include "object.hpp"
#include "descriptor.hpp"
#include "ban.hpp"
#include "shop.hpp"
#include "reset.hpp"
#include "note.hpp"
#include "database.hpp"
#include "world.hpp"

// Temp externs
extern Room *find_location (Character * ch, const std::string & arg);
extern Character *find_keeper (Character * ch);
extern int get_cost (Character * keeper, Object * obj, bool fBuy);
extern std::string extra_bit_name (int extra_flags);
extern void obj_cast_spell (int sn, int level, Character * ch, Character * victim,
  Object * obj);
extern void talk_channel (Character * ch, const std::string & argument, int channel,
  const char *verb);
extern void multi_hit(Character *ch, Character *victim, int dt);
extern void say_spell (Character * ch, int sn);
extern void raw_kill (Character * victim);
extern bool is_same_group (Character * ach, Character * bch);
extern std::string affect_loc_name (int location);
extern std::string affect_bit_name (int vector);
extern void mprog_bribe_trigger (Character * mob, Character * ch, int amount);
extern void mprog_speech_trigger (const std::string & txt, Character * mob);
extern void mprog_give_trigger (Character * mob, Character * ch, Object * obj);
extern Object *create_money (int amount);
extern void note_attach (Character * ch);
extern void note_remove (Character * ch, Note * pnote);
extern void damage(Character *ch, Character *victim, int dam, int dt);
extern void check_killer (Character * ch, Character * victim);
extern std::string get_extra_descr (const std::string & name, std::list<ExtraDescription *> & ed);
extern bool is_safe (Character * ch, Character * victim);
extern void disarm (Character * ch, Character * victim);

void Character::do_areas (std::string argument)
{
  send_to_char (g_world->list_areas());
  return;
}

void Character::do_memory (std::string argument)
{
  char buf[MAX_STRING_LENGTH];

  snprintf (buf, sizeof buf, "Affects %5d\r\n", Affect::top_affect);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Areas   %5d\r\n", Area::top_area);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "ExDes   %5d\r\n", ExtraDescription::top_ed);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Exits   %5d\r\n", Exit::top_exit);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Mobs    %5d\r\n", MobPrototype::top_mob);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Objs    %5d\r\n", ObjectPrototype::top_obj);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Resets  %5d\r\n", Reset::top_reset);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Rooms   %5d\r\n", Room::top_room);
  send_to_char (buf);
  snprintf (buf, sizeof buf, "Shops   %5d\r\n", Shop::top_shop);
  send_to_char (buf);
  return;
}

void Character::do_kill (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Kill whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!victim->is_npc ()) {
    if (!IS_SET (victim->actflags, PLR_KILLER)
      && !IS_SET (victim->actflags, PLR_THIEF)) {
      send_to_char ("You must MURDER a player.\r\n");
      return;
    }
  } else {
    if (victim->is_affected (AFF_CHARM) && victim->master != NULL) {
      send_to_char ("You must MURDER a charmed creature.\r\n");
      return;
    }
  }

  if (victim == this) {
    send_to_char ("You hit yourself.  Ouch!\r\n");
    multi_hit (this, this, TYPE_UNDEFINED);
    return;
  }

  if (is_safe (this, victim))
    return;

  if (is_affected (AFF_CHARM) && master == victim) {
    act ("$N is your beloved master.", NULL, victim, TO_CHAR);
    return;
  }

  if (position == POS_FIGHTING) {
    send_to_char ("You do the best you can!\r\n");
    return;
  }

  wait_state (1 * PULSE_VIOLENCE);
  check_killer (this, victim);
  multi_hit (this, victim, TYPE_UNDEFINED);
  return;
}

void Character::do_murde (std::string argument)
{
  send_to_char ("If you want to MURDER, spell it out.\r\n");
  return;
}

void Character::do_murder (std::string argument)
{
  std::string arg, buf;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Murder whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim == this) {
    send_to_char ("Suicide is a mortal sin.\r\n");
    return;
  }

  if (is_safe (this, victim))
    return;

  if (is_affected (AFF_CHARM) && master == victim) {
    act ("$N is your beloved master.", NULL, victim, TO_CHAR);
    return;
  }

  if (position == POS_FIGHTING) {
    send_to_char ("You do the best you can!\r\n");
    return;
  }

  wait_state (1 * PULSE_VIOLENCE);
  buf += "Help!  I am being attacked by " + name + "!";
  victim->do_shout (buf);
  check_killer (this, victim);
  multi_hit (this, victim, TYPE_UNDEFINED);
  return;
}

void Character::do_backstab (std::string argument)
{
  std::string arg;

  if (!is_npc ()
    && level < skill_table[skill_lookup("backstab")].skill_level[klass]) {
    send_to_char ("You better leave the assassin trade to thieves.\r\n");
    return;
  }

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Backstab whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim == this) {
    send_to_char ("How can you sneak up on yourself?\r\n");
    return;
  }

  if (is_safe (this, victim))
    return;

  Object *obj;
  if ((obj = get_eq_char (WEAR_WIELD)) == NULL || obj->value[3] != 11) {
    send_to_char ("You need to wield a piercing weapon.\r\n");
    return;
  }

  if (victim->fighting != NULL) {
    send_to_char ("You can't backstab a fighting person.\r\n");
    return;
  }

  if (victim->hit < victim->max_hit) {
    act ("$N is hurt and suspicious ... you can't sneak up.",
      NULL, victim, TO_CHAR);
    return;
  }

  check_killer (this, victim);
  int bck = skill_lookup("backstab");
  wait_state (skill_table[bck].beats);
  if (!victim->is_awake ()
    || is_npc ()
    || number_percent () < pcdata->learned[bck])
    multi_hit (this, victim, bck);
  else
    damage (this, victim, 0, bck);

  return;
}

void Character::do_flee (std::string argument)
{
  if (fighting == NULL) {
    if (position == POS_FIGHTING)
      position = POS_STANDING;
    send_to_char ("You aren't fighting anyone.\r\n");
    return;
  }

  Room *now_in;
  Room* was_in = in_room;
  for (int attempt = 0; attempt < 6; attempt++) {
    Exit *pexit;
    int door;

    door = number_door ();
    if ((pexit = was_in->exit[door]) == 0
      || pexit->to_room == NULL || IS_SET (pexit->exit_info, EX_CLOSED)
      || (is_npc ()
        && (IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)
          || (IS_SET (actflags, ACT_STAY_AREA)
            && pexit->to_room->area != in_room->area))))
      continue;

    move_char (door);
    if ((now_in = in_room) == was_in)
      continue;

    in_room = was_in;
    act ("$n has fled!", NULL, NULL, TO_ROOM);
    in_room = now_in;

    if (!is_npc ()) {
      send_to_char ("You flee from combat!  You lose 25 exps.\r\n");
      gain_exp(-25);
    }

    stop_fighting(true);
    return;
  }

  send_to_char ("You failed!  You lose 10 exps.\r\n");
  gain_exp(-10);
  return;
}

void Character::do_rescue (std::string argument)
{
  if (!is_npc ()
    && level < skill_table[skill_lookup("rescue")].skill_level[klass]) {
    send_to_char ("You better leave the heroic acts to warriors.\r\n");
    return;
  }

  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Rescue whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim == this) {
    send_to_char ("What about fleeing instead?\r\n");
    return;
  }

  if (!is_npc () && victim->is_npc ()) {
    send_to_char ("Doesn't need your help!\r\n");
    return;
  }

  if (fighting == victim) {
    send_to_char ("Too late.\r\n");
    return;
  }

  Character *fch;
  if ((fch = victim->fighting) == NULL) {
    send_to_char ("That person is not fighting right now.\r\n");
    return;
  }

  int rsc = skill_lookup("rescue");
  wait_state (skill_table[rsc].beats);
  if (!is_npc () && number_percent () > pcdata->learned[rsc]) {
    send_to_char ("You fail the rescue.\r\n");
    return;
  }

  act ("You rescue $N!", NULL, victim, TO_CHAR);
  act ("$n rescues you!", NULL, victim, TO_VICT);
  act ("$n rescues $N!", NULL, victim, TO_NOTVICT);

  fch->stop_fighting(false);
  victim->stop_fighting(false);

  check_killer (this, fch);
  set_fighting(fch);
  fch->set_fighting(this);
  return;
}

void Character::do_kick (std::string argument)
{
  if (!is_npc ()
    && level < skill_table[skill_lookup("kick")].skill_level[klass]) {
    send_to_char ("You better leave the martial arts to fighters.\r\n");
    return;
  }

  Character *victim;

  if ((victim = fighting) == NULL) {
    send_to_char ("You aren't fighting anyone.\r\n");
    return;
  }

  int kck = skill_lookup("kick");
  wait_state (skill_table[kck].beats);
  if (is_npc () || number_percent () < pcdata->learned[kck])
    damage (this, victim, number_range (1, level), kck);
  else
    damage (this, victim, 0, kck);

  return;
}

void Character::do_disarm (std::string argument)
{
  if (!is_npc ()
    && level < skill_table[skill_lookup("disarm")].skill_level[klass]) {
    send_to_char ("You don't know how to disarm opponents.\r\n");
    return;
  }

  if (get_eq_char (WEAR_WIELD) == NULL) {
    send_to_char ("You must wield a weapon to disarm.\r\n");
    return;
  }

  Character *victim;

  if ((victim = fighting) == NULL) {
    send_to_char ("You aren't fighting anyone.\r\n");
    return;
  }

  if (victim->get_eq_char (WEAR_WIELD) == NULL) {
    send_to_char ("Your opponent is not wielding a weapon.\r\n");
    return;
  }

  wait_state (skill_table[skill_lookup("disarm")].beats);
  int percent = number_percent () + victim->level - level;
  if (is_npc () || percent < pcdata->learned[skill_lookup("disarm")] * 2 / 3)
    disarm (this, victim);
  else
    send_to_char ("You failed.\r\n");
  return;
}

void Character::do_sla (std::string argument)
{
  send_to_char ("If you want to SLAY, spell it out.\r\n");
  return;
}

void Character::do_slay (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Slay whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (this == victim) {
    send_to_char ("Suicide is a mortal sin.\r\n");
    return;
  }

  if (!victim->is_npc () && victim->level >= level) {
    send_to_char ("You failed.\r\n");
    return;
  }

  act ("You slay $M in cold blood!", NULL, victim, TO_CHAR);
  act ("$n slays you in cold blood!", NULL, victim, TO_VICT);
  act ("$n slays $N in cold blood!", NULL, victim, TO_NOTVICT);
  raw_kill (victim);
  return;
}

void Character::do_cast (std::string argument)
{
  /*
   * Only MOBprogrammed mobs not charmed can cast spells
   * like PC's
   */
  if (is_npc ()
    && (!pIndexData->progtypes || is_affected (AFF_CHARM)))
    return;

  std::string arg1, arg2;

  target_name = one_argument (argument, arg1);
  one_argument (target_name, arg2);

  if (arg1.empty()) {
    send_to_char ("Cast which what where?\r\n");
    return;
  }

  int sn;
  if ((sn = skill_lookup (arg1)) < 0
    || (!is_npc () && level < skill_table[sn].skill_level[klass])) {
    send_to_char ("You can't do that.\r\n");
    return;
  }

  if (position < skill_table[sn].minimum_position) {
    send_to_char ("You can't concentrate enough.\r\n");
    return;
  }

  int mn = mana_cost (sn);
  /*
   * Locate targets.
   */
  Character *victim = NULL;
  Object *obj;
  void *vo = NULL;

  switch (skill_table[sn].target) {
  default:
    bug_printf ("Do_cast: bad target for sn %d.", sn);
    return;

  case TAR_IGNORE:
    break;

  case TAR_CHAR_OFFENSIVE:
    if (arg2.empty()) {
      if ((victim = fighting) == NULL) {
        send_to_char ("Cast the spell on whom?\r\n");
        return;
      }
    } else {
      if ((victim = get_char_room (arg2)) == NULL) {
        send_to_char ("They aren't here.\r\n");
        return;
      }
    }
    vo = (void *) victim;
    break;

  case TAR_CHAR_DEFENSIVE:
    if (arg2.empty()) {
      victim = this;
    } else {
      if ((victim = get_char_room (arg2)) == NULL) {
        send_to_char ("They aren't here.\r\n");
        return;
      }
    }

    vo = (void *) victim;
    break;

  case TAR_CHAR_SELF:
    if (!arg2.empty() && !is_name (arg2, name)) {
      send_to_char ("You cannot cast this spell on another.\r\n");
      return;
    }

    vo = (void *) this;
    break;

  case TAR_OBJ_INV:
    if (arg2.empty()) {
      send_to_char ("What should the spell be cast upon?\r\n");
      return;
    }

    if ((obj = get_obj_carry (arg2)) == NULL) {
      send_to_char ("You are not carrying that.\r\n");
      return;
    }

    vo = (void *) obj;
    break;
  }

  if (!is_npc () && mana < mn) {
    send_to_char ("You don't have enough mana.\r\n");
    return;
  }

  if (str_cmp (skill_table[sn].name, "ventriloquate"))
    say_spell (this, sn);

  wait_state (skill_table[sn].beats);

  if (!is_npc () && number_percent () > pcdata->learned[sn]) {
    send_to_char ("You lost your concentration.\r\n");
    mana -= mn / 2;
  } else {
    mana -= mn;
    (this->*(skill_table[sn].spell_fun)) (sn, level, vo);
  }

  if (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    && victim->master != this && victim != this) {
    Character *vch;

    CharIter rch, next;
    for (rch = in_room->people.begin(); rch != in_room->people.end(); rch = next) {
      vch = *rch;
      next = ++rch;
      if (victim == vch && victim->fighting == NULL) {
        multi_hit (victim, this, TYPE_UNDEFINED);
        break;
      }
    }
  }

  return;
}

/* Date stamp idea comes from Alander of ROM */
void Character::do_note (std::string argument)
{
  if (is_npc ())
    return;

  std::string arg, buf1;

  argument = one_argument (argument, arg);
  smash_tilde (argument);

  if (arg.empty()) {
    do_note ("read");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  int vnum;
  int anum;
  if (!str_cmp (arg, "list")) {
    vnum = 0;
    for (std::list<Note*>::iterator p = note_list.begin();
      p != note_list.end(); p++) {
      if ((*p)->is_note_to (this)) {
        snprintf (buf, sizeof buf, "[%3d%s] %s: %s\r\n",
          vnum,
          ((*p)->date_stamp > last_note
            && str_cmp ((*p)->sender, name)) ? "N" : " ",
          (*p)->sender.c_str(), (*p)->subject.c_str());
        buf1.append(buf);
        vnum++;
      }
    }
    send_to_char (buf1);
    return;
  }

  if (!str_cmp (arg, "read")) {
    bool fAll;

    if (!str_cmp (argument, "all")) {
      fAll = true;
      anum = 0;
    } else if (argument.empty() || !str_prefix (argument, "next"))
      /* read next unread note */
    {
      vnum = 0;
      for (std::list<Note*>::iterator p = note_list.begin();
        p != note_list.end(); p++) {
        if ((*p)->is_note_to (this)
          && str_cmp (name, (*p)->sender)
          && last_note < (*p)->date_stamp) {
          snprintf (buf, sizeof buf, "[%3d] %s: %s\r\n%s\r\nTo: %s\r\n",
            vnum, (*p)->sender.c_str(), (*p)->subject.c_str(),
            (*p)->date.c_str(), (*p)->to_list.c_str());
          buf1.append(buf);
          buf1.append((*p)->text);
          last_note = std::max (last_note, (*p)->date_stamp);
          send_to_char (buf1);
          return;
        } else
          vnum++;
      }
      send_to_char ("You have no unread notes.\r\n");
      return;
    } else if (is_number (argument)) {
      fAll = false;
      anum = std::atoi (argument.c_str());
    } else {
      send_to_char ("Note read which number?\r\n");
      return;
    }

    vnum = 0;
    for (std::list<Note*>::iterator p = note_list.begin();
      p != note_list.end(); p++) {
      if ((*p)->is_note_to (this) && (vnum++ == anum || fAll)) {
        snprintf (buf, sizeof buf, "[%3d] %s: %s\r\n%s\r\nTo: %s\r\n",
          vnum - 1,
          (*p)->sender.c_str(), (*p)->subject.c_str(),
          (*p)->date.c_str(), (*p)->to_list.c_str());
        buf1.append(buf);
        buf1.append((*p)->text);
        send_to_char (buf1);
        last_note = std::max (last_note, (*p)->date_stamp);
        return;
      }
    }

    send_to_char ("No such note.\r\n");
    return;
  }

  if (!str_cmp (arg, "+")) {
    note_attach (this);
    strncpy (buf, pnote->text.c_str(), sizeof buf);
    if (strlen (buf) + argument.size() >= MAX_STRING_LENGTH - 200) {
      send_to_char ("Note too long.\r\n");
      return;
    }

    strncat (buf, argument.c_str(), sizeof buf - argument.size());
    strncat (buf, "\r\n", sizeof buf - strlen("\r\n"));
    pnote->text = buf;
    send_to_char ("Ok.\r\n");
    return;
  }

  if (!str_cmp (arg, "subject")) {
    note_attach (this);
    pnote->subject = argument;
    send_to_char ("Ok.\r\n");
    return;
  }

  if (!str_cmp (arg, "to")) {
    note_attach (this);
    pnote->to_list = argument;
    send_to_char ("Ok.\r\n");
    return;
  }

  if (!str_cmp (arg, "clear")) {
    if (pnote != NULL) {
      delete pnote;
      pnote = NULL;
    }

    send_to_char ("Ok.\r\n");
    return;
  }

  if (!str_cmp (arg, "show")) {
    if (pnote == NULL) {
      send_to_char ("You have no note in progress.\r\n");
      return;
    }

    snprintf (buf, sizeof buf, "%s: %s\r\nTo: %s\r\n",
      pnote->sender.c_str(), pnote->subject.c_str(), pnote->to_list.c_str());
    send_to_char (buf);
    send_to_char (pnote->text);
    return;
  }

  if (!str_cmp (arg, "post") || !str_prefix (arg, "send")) {
    if (pnote == NULL) {
      send_to_char ("You have no note in progress.\r\n");
      return;
    }

    if (!str_cmp (pnote->to_list, "")) {
      send_to_char
        ("You need to provide a recipient (name, all, or immortal).\r\n");
      return;
    }

    if (!str_cmp (pnote->subject, "")) {
      send_to_char ("You need to provide a subject.\r\n");
      return;
    }

    pnote->date = g_world->get_time_text();
    pnote->date_stamp = g_world->get_current_time();

    note_list.push_back(pnote);

    std::ofstream notefile;

    notefile.open (NOTE_FILE, std::ofstream::out | std::ofstream::app | std::ofstream::binary);
    if (!notefile.is_open()) {
      std::perror (NOTE_FILE);
    } else {
      notefile << "Sender  " << pnote->sender << "~\n";
      notefile << "Date    " << pnote->date << "~\n";
      notefile << "Stamp   " << pnote->date_stamp << "\n";
      notefile << "To      " << pnote->to_list << "~\n";
      notefile << "Subject " << pnote->subject << "~\n";
      notefile << "Text\n" << pnote->text << "~\n\n";
      notefile.close();
    }

    pnote = NULL;
    send_to_char ("Ok.\r\n");
    return;
  }

  if (!str_cmp (arg, "remove")) {
    if (!is_number (argument)) {
      send_to_char ("Note remove which number?\r\n");
      return;
    }

    anum = std::atoi (argument.c_str());
    vnum = 0;
    std::list<Note*>::iterator next;
    for (std::list<Note*>::iterator p = note_list.begin();
      p != note_list.end(); p = next) {
      Note* curr = *p;
      next = ++p;
      if (curr->is_note_to (this) && vnum++ == anum) {
        note_remove (this, curr);
        send_to_char ("Ok.\r\n");
        return;
      }
    }

    send_to_char ("No such note.\r\n");
    return;
  }

  send_to_char ("Huh?  Type 'help note' for usage.\r\n");
  return;
}

void Character::do_auction (std::string argument)
{
  talk_channel (this, argument, CHANNEL_AUCTION, "auction");
  return;
}

void Character::do_chat (std::string argument)
{
  talk_channel (this, argument, CHANNEL_CHAT, "chat");
  return;
}

/*
 * Alander's new channels.
 */
void Character::do_music (std::string argument)
{
  talk_channel (this, argument, CHANNEL_MUSIC, "music");
  return;
}

void Character::do_question (std::string argument)
{
  talk_channel (this, argument, CHANNEL_QUESTION, "question");
  return;
}

void Character::do_answer (std::string argument)
{
  talk_channel (this, argument, CHANNEL_QUESTION, "answer");
  return;
}

void Character::do_shout (std::string argument)
{
  talk_channel (this, argument, CHANNEL_SHOUT, "shout");
  wait_state (12);
  return;
}

void Character::do_yell (std::string argument)
{
  talk_channel (this, argument, CHANNEL_YELL, "yell");
  return;
}

void Character::do_immtalk (std::string argument)
{
  talk_channel (this, argument, CHANNEL_IMMTALK, "immtalk");
  return;
}

void Character::do_say (std::string argument)
{
  if (argument.empty()) {
    send_to_char ("Say what?\r\n");
    return;
  }

  act ("$n says '$T'.", NULL, argument.c_str(), TO_ROOM);
  act ("You say '$T'.", NULL, argument.c_str(), TO_CHAR);
  mprog_speech_trigger (argument, this);
  return;
}

void Character::do_tell (std::string argument)
{
  if (!is_npc () && IS_SET (actflags, PLR_SILENCE)) {
    send_to_char ("Your message didn't get through.\r\n");
    return;
  }

  std::string arg;
  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    send_to_char ("Tell whom what?\r\n");
    return;
  }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  Character *victim;
  if ((victim = get_char_world (arg)) == NULL
    || (victim->is_npc () && victim->in_room != in_room)) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!is_immortal() && !victim->is_awake ()) {
    act ("$E can't hear you.", 0, victim, TO_CHAR);
    return;
  }

  act ("You tell $N '$t'.", argument.c_str(), victim, TO_CHAR);
  int savepos = victim->position;
  victim->position = POS_STANDING;
  act ("$n tells you '$t'.", argument.c_str(), victim, TO_VICT);
  victim->position = savepos;
  victim->reply = this;

  return;
}

void Character::do_reply (std::string argument)
{
  if (!is_npc () && IS_SET (actflags, PLR_SILENCE)) {
    send_to_char ("Your message didn't get through.\r\n");
    return;
  }

  Character *victim;

  if ((victim = reply) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!is_immortal() && !victim->is_awake ()) {
    act ("$E can't hear you.", 0, victim, TO_CHAR);
    return;
  }

  act ("You tell $N '$t'.", argument.c_str(), victim, TO_CHAR);
  int savepos = victim->position;
  victim->position = POS_STANDING;
  act ("$n tells you '$t'.", argument.c_str(), victim, TO_VICT);
  victim->position = savepos;
  victim->reply = this;

  return;
}

void Character::do_emote (std::string argument)
{
  if (!is_npc () && IS_SET (actflags, PLR_NO_EMOTE)) {
    send_to_char ("You can't show your emotions.\r\n");
    return;
  }

  if (argument.empty()) {
    send_to_char ("Emote what?\r\n");
    return;
  }

  if (isalpha(argument[argument.size()-1]))
    argument += ".";

  act ("$n $T", NULL, argument.c_str(), TO_ROOM);
  act ("$n $T", NULL, argument.c_str(), TO_CHAR);
  return;
}

void Character::do_bug (std::string argument)
{
  append_file (BUG_FILE, argument);
  send_to_char ("Ok.  Thanks.\r\n");
  return;
}

void Character::do_idea (std::string argument)
{
  append_file (IDEA_FILE, argument);
  send_to_char ("Ok.  Thanks.\r\n");
  return;
}

void Character::do_typo (std::string argument)
{
  append_file (TYPO_FILE, argument);
  send_to_char ("Ok.  Thanks.\r\n");
  return;
}

void Character::do_rent (std::string argument)
{
  send_to_char ("There is no rent here.  Just save and quit.\r\n");
  return;
}

void Character::do_qui (std::string argument)
{
  send_to_char ("If you want to QUIT, you have to spell it out.\r\n");
  return;
}

void Character::do_quit (std::string argument)
{
  if (is_npc ())
    return;

  if (position == POS_FIGHTING) {
    send_to_char ("No way! You are fighting.\r\n");
    return;
  }

  if (position < POS_STUNNED) {
    send_to_char ("You're not DEAD yet.\r\n");
    return;
  }

  send_to_char
    ("Had I but time--as this fell sergeant, Death,\r\nIs strict in his arrest--O, I could tell you--\r\nBut let it be.\r\n");
  act ("$n has left the game.", NULL, NULL, TO_ROOM);
  log_printf ("%s has quit.", name.c_str());

  /*
   * After extract_char the this is no longer valid!
   */
  save_char_obj();
  Descriptor *d = desc;
  extract_char (true);
  if (d != NULL)
    d->close_socket();

  return;
}

void Character::do_save (std::string argument)
{
  if (is_npc ())
    return;

  if (level < 2) {
    send_to_char ("You must be at least second level to save.\r\n");
    return;
  }

  save_char_obj();
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_gag (std::string argument)
{
  if (is_npc ())
    return;

  std::string arg;
  one_argument (argument, arg);

  if (arg.empty()) {
    if (pcdata->gag_list.empty()) {
      send_to_char ("No gags set.\r\n");
      return;
    }
    std::string buf("Gag list\r\n");
    for (std::list<std::string>::iterator gag = pcdata->gag_list.begin();
      gag != pcdata->gag_list.end(); gag++) {
      buf.append((*gag) + "\r\n");
    }
    send_to_char (buf);
    return;
  }

  std::string gagname = capitalize(arg);
  if (!str_cmp(name, gagname)) {
    send_to_char ("You can't gag yourself.\r\n");
    return;
  }

  std::list<std::string>::iterator fnd;
  fnd = std::find(pcdata->gag_list.begin(), pcdata->gag_list.end(), gagname);
  if (fnd != pcdata->gag_list.end()) {
    pcdata->gag_list.remove(gagname);
    send_to_char ("Gag removed.\r\n");
  } else {
    pcdata->gag_list.push_back(gagname);
    send_to_char ("Gag added.\r\n");
  }
  return;
}

void Character::do_follow (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Follow whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (is_affected (AFF_CHARM) && master != NULL) {
    act ("But you'd rather follow $N!", NULL, master, TO_CHAR);
    return;
  }

  if (victim == this) {
    if (master == NULL) {
      send_to_char ("You already follow yourself.\r\n");
      return;
    }
    stop_follower();
    return;
  }

  if ((level - victim->level < -5 || level - victim->level > 5)
    && !is_hero()) {
    send_to_char ("You are not of the right caliber to follow.\r\n");
    return;
  }

  if (master != NULL)
    stop_follower();

  add_follower(victim);
  return;
}

void Character::do_order (std::string argument)
{
  std::string arg;

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    send_to_char ("Order whom to do what?\r\n");
    return;
  }

  if (is_affected (AFF_CHARM)) {
    send_to_char ("You feel like taking, not giving, orders.\r\n");
    return;
  }

  Character *victim;
  bool fAll;
  if (!str_cmp (arg, "all")) {
    fAll = true;
    victim = NULL;
  } else {
    fAll = false;
    if ((victim = get_char_room (arg)) == NULL) {
      send_to_char ("They aren't here.\r\n");
      return;
    }

    if (victim == this) {
      send_to_char ("Aye aye, right away!\r\n");
      return;
    }

    if (!victim->is_affected (AFF_CHARM) || victim->master != this) {
      send_to_char ("Do it yourself!\r\n");
      return;
    }
  }

  Character *och;
  bool found = false;
  CharIter rch, next;
  for (rch = in_room->people.begin(); rch != in_room->people.end(); rch = next) {
    och = *rch;
    next = ++rch;

    if (och->is_affected (AFF_CHARM)
      && och->master == this && (fAll || och == victim)) {
      found = true;
      act ("$n orders you to '$t'.", argument.c_str(), och, TO_VICT);
      och->interpret (argument);
    }
  }

  if (found)
    send_to_char ("Ok.\r\n");
  else
    send_to_char ("You have no followers here.\r\n");
  return;
}

void Character::do_group (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    Character *ldr = (leader != NULL) ? leader : this;
    snprintf (buf, sizeof buf, "%s's group:\r\n", ldr->describe_to(this).c_str());
    send_to_char (buf);

    CharIter c;
    for (c = char_list.begin(); c != char_list.end(); c++) {
      if (is_same_group (*c, this)) {
        snprintf (buf, sizeof buf,
          "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\r\n",
          (*c)->level,
          (*c)->is_npc () ? "Mob" : class_table[(*c)->klass].who_name,
          capitalize ((*c)->describe_to(this)).c_str(),
          (*c)->hit, (*c)->max_hit,
          (*c)->mana, (*c)->max_mana, (*c)->move, (*c)->max_move, (*c)->exp);
        send_to_char (buf);
      }
    }
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (master != NULL || (leader != NULL && leader != this)) {
    send_to_char ("But you are following someone else!\r\n");
    return;
  }

  if (victim->master != this && this != victim) {
    act ("$N isn't following you.", NULL, victim, TO_CHAR);
    return;
  }

  if (is_same_group (victim, this) && this != victim) {
    victim->leader = NULL;
    act ("$n removes $N from $s group.", NULL, victim, TO_NOTVICT);
    act ("$n removes you from $s group.", NULL, victim, TO_VICT);
    act ("You remove $N from your group.", NULL, victim, TO_CHAR);
    return;
  }

  if (level - victim->level < -5 || level - victim->level > 5) {
    act ("$N cannot join $n's group.", NULL, victim, TO_NOTVICT);
    act ("You cannot join $n's group.", NULL, victim, TO_VICT);
    act ("$N cannot join your group.", NULL, victim, TO_CHAR);
    return;
  }

  victim->leader = this;
  act ("$N joins $n's group.", NULL, victim, TO_NOTVICT);
  act ("You join $n's group.", NULL, victim, TO_VICT);
  act ("$N joins your group.", NULL, victim, TO_CHAR);
  return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void Character::do_split (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Split how much?\r\n");
    return;
  }

  int amount = std::atoi (arg.c_str());

  if (amount < 0) {
    send_to_char ("Your group wouldn't like that.\r\n");
    return;
  }

  if (amount == 0) {
    send_to_char ("You hand out zero coins, but no one notices.\r\n");
    return;
  }

  if (gold < amount) {
    send_to_char ("You don't have that much gold.\r\n");
    return;
  }

  int members = 0;
  CharIter gch;
  for (gch = in_room->people.begin(); gch != in_room->people.end(); gch++) {
    if (is_same_group (*gch, this))
      members++;
  }

  if (members < 2) {
    send_to_char ("Just keep it all.\r\n");
    return;
  }

  int share = amount / members;
  int extra = amount % members;

  if (share == 0) {
    send_to_char ("Don't even bother, cheapskate.\r\n");
    return;
  }

  gold -= amount;
  gold += share + extra;

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf,
    "You split %d gold coins.  Your share is %d gold coins.\r\n",
    amount, share + extra);
  send_to_char (buf);

  snprintf (buf, sizeof buf, "$n splits %d gold coins.  Your share is %d gold coins.",
    amount, share);

  for (gch = in_room->people.begin(); gch != in_room->people.end(); gch++) {
    if (*gch != this && is_same_group (*gch, this)) {
      act (buf, NULL, *gch, TO_VICT);
      (*gch)->gold += share;
    }
  }

  return;
}

void Character::do_gtell (std::string argument)
{
  char buf[MAX_STRING_LENGTH];

  if (argument.empty()) {
    send_to_char ("Tell your group what?\r\n");
    return;
  }

  if (IS_SET (actflags, PLR_NO_TELL)) {
    send_to_char ("Your message didn't get through!\r\n");
    return;
  }

  /*
   * Note use of send_to_char, so gtell works on sleepers.
   */
  snprintf (buf, sizeof buf, "%s tells the group '%s'.\r\n", name.c_str(), argument.c_str());
  for (CharIter c = char_list.begin(); c != char_list.end(); c++) {
    if (is_same_group (*c, this))
      (*c)->send_to_char (buf);
  }

  return;
}

void Character::do_look (std::string argument)
{
  if (!is_npc () && desc == NULL)
    return;

  if (position < POS_SLEEPING) {
    send_to_char ("You can't see anything but stars!\r\n");
    return;
  }

  if (position == POS_SLEEPING) {
    send_to_char ("You can't see anything, you're sleeping!\r\n");
    return;
  }

  if (!check_blind())
    return;

  if (!is_npc ()
    && !IS_SET (actflags, PLR_HOLYLIGHT)
    && in_room->is_dark()) {
    send_to_char ("It is pitch black ... \r\n");
    show_char_to_char (in_room->people);
    return;
  }

  std::string arg1, arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || !str_cmp (arg1, "auto")) {
    /* 'look' or 'look auto' */
    send_to_char (in_room->name);
    send_to_char ("\r\n");

    if (!is_npc () && IS_SET (actflags, PLR_AUTOEXIT))
      do_exits ("auto");

    if (arg1.empty() || (!is_npc () && !IS_SET (actflags, PLR_BRIEF)))
      send_to_char (in_room->description);

    show_list_to_char (in_room->contents, false, false);
    show_char_to_char (in_room->people);
    return;
  }

  char buf[MAX_STRING_LENGTH];
  Object *obj;
  std::string pdesc;

  if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in")) {
    /* 'look in' */
    if (arg2.empty()) {
      send_to_char ("Look in what?\r\n");
      return;
    }

    if ((obj = get_obj_here (arg2)) == NULL) {
      send_to_char ("You do not see that here.\r\n");
      return;
    }

    switch (obj->item_type) {
    default:
      send_to_char ("That is not a container.\r\n");
      break;

    case ITEM_DRINK_CON:
      if (obj->value[1] <= 0) {
        send_to_char ("It is empty.\r\n");
        break;
      }

      snprintf (buf, sizeof buf, "It's %s full of a %s liquid.\r\n",
        obj->value[1] < obj->value[0] / 4
        ? "less than" :
        obj->value[1] < 3 * obj->value[0] / 4
        ? "about" : "more than", liq_table[obj->value[2]].liq_color);

      send_to_char (buf);
      break;

    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
      if (IS_SET (obj->value[1], CONT_CLOSED)) {
        send_to_char ("It is closed.\r\n");
        break;
      }

      act ("$p contains:", obj, NULL, TO_CHAR);
      show_list_to_char (obj->contains, true, true);
      break;
    }
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg1)) != NULL) {
    show_char_to_char_1 (victim);
    return;
  }

  ObjIter o;
  for (o = carrying.begin(); o != carrying.end(); o++) {
    if (can_see_obj(*o)) {
      pdesc = get_extra_descr (arg1, (*o)->extra_descr);
      if (!pdesc.empty()) {
        send_to_char (pdesc);
        return;
      }

      pdesc = get_extra_descr (arg1, (*o)->pIndexData->extra_descr);
      if (!pdesc.empty()) {
        send_to_char (pdesc);
        return;
      }
    }

    if (is_name (arg1, (*o)->name)) {
      send_to_char ((*o)->description);
      return;
    }
  }

  for (o = in_room->contents.begin(); o != in_room->contents.end(); o++) {
    if (can_see_obj(*o)) {
      pdesc = get_extra_descr (arg1, (*o)->extra_descr);
      if (!pdesc.empty()) {
        send_to_char (pdesc);
        return;
      }

      pdesc = get_extra_descr (arg1, (*o)->pIndexData->extra_descr);
      if (!pdesc.empty()) {
        send_to_char (pdesc);
        return;
      }
    }

    if (is_name (arg1, (*o)->name)) {
      send_to_char ((*o)->description);
      return;
    }
  }

  pdesc = get_extra_descr (arg1, in_room->extra_descr);
  if (!pdesc.empty()) {
    send_to_char (pdesc);
    return;
  }

  int door;
  if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
    door = 0;
  else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
    door = 1;
  else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
    door = 2;
  else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
    door = 3;
  else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
    door = 4;
  else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
    door = 5;
  else {
    send_to_char ("You do not see that here.\r\n");
    return;
  }

  /* 'look direction' */
  Exit *pexit;
  if ((pexit = in_room->exit[door]) == NULL) {
    send_to_char ("Nothing special there.\r\n");
    return;
  }

  if (!pexit->description.empty())
    send_to_char (pexit->description);
  else
    send_to_char ("Nothing special there.\r\n");

  if (!pexit->name.empty() && pexit->name[0] != ' ') {
    if (IS_SET (pexit->exit_info, EX_CLOSED)) {
      act ("The $d is closed.", NULL, pexit->name.c_str(), TO_CHAR);
    } else if (IS_SET (pexit->exit_info, EX_ISDOOR)) {
      act ("The $d is open.", NULL, pexit->name.c_str(), TO_CHAR);
    }
  }

  return;
}

void Character::do_examine (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Examine what?\r\n");
    return;
  }

  do_look (arg);

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    switch (obj->item_type) {
    default:
      break;

    case ITEM_DRINK_CON:
    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
      send_to_char ("When you look inside, you see:\r\n");
      std::string buf = "in " + arg;
      do_look (buf);
    }
  }

  return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void Character::do_exits (std::string argument)
{
  if (!check_blind())
    return;

  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  bool fAuto = !str_cmp (argument, "auto");
  strncpy (buf, fAuto ? "[Exits:" : "Obvious exits:\r\n", sizeof buf);

  bool found = false;
  for (int door = 0; door <= 5; door++) {
    Exit *pexit;
    if ((pexit = in_room->exit[door]) != NULL
      && pexit->to_room != NULL && !IS_SET (pexit->exit_info, EX_CLOSED)) {
      found = true;
      if (fAuto) {
        strncat (buf, " ", sizeof(buf) - strlen(" "));
        strncat (buf, dir_name[door].c_str(), sizeof(buf) - dir_name[door].size());
      } else {
        snprintf (buf + strlen(buf), sizeof(buf) - strlen(buf), "%-5s - %s\r\n",
          capitalize (dir_name[door]).c_str(), pexit->to_room->is_dark()
          ? "Too dark to tell" : pexit->to_room->name.c_str());
      }
    }
  }

  if (!found)
    strncat (buf, fAuto ? " none" : "None.\r\n", sizeof(buf) - strlen("None.\r\n"));

  if (fAuto)
    strncat (buf, "]\r\n", sizeof(buf) - strlen("]\r\n"));

  send_to_char (buf);
  return;
}

void Character::do_score (std::string argument)
{
  char buf[MAX_STRING_LENGTH];

  snprintf (buf, sizeof buf,
    "You are %s%s, level %d, %d years old (%d hours).\r\n",
    name.c_str(),
    is_npc () ? "" : pcdata->title.c_str(),
    level, get_age(), (get_age() - 17) * 2);
  send_to_char (buf);

  if (get_trust () != level) {
    snprintf (buf, sizeof buf, "You are trusted at level %d.\r\n", get_trust ());
    send_to_char (buf);
  }

  snprintf (buf, sizeof buf,
    "You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\r\n",
    hit, max_hit,
    mana, max_mana, move, max_move, practice);
  send_to_char (buf);

  snprintf (buf, sizeof buf,
    "You are carrying %d/%d items with weight %d/%d kg.\r\n",
    carry_number, can_carry_n(), carry_weight, can_carry_w());
  send_to_char (buf);

  snprintf (buf, sizeof buf,
    "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d.\r\n",
    get_curr_str(), get_curr_int(), get_curr_wis(),
    get_curr_dex(), get_curr_con());
  send_to_char (buf);

  snprintf (buf, sizeof buf,
    "You have scored %d exp, and have %d gold coins.\r\n", exp, gold);
  send_to_char (buf);

  snprintf (buf, sizeof buf,
    "Autoexit: %s.  Autoloot: %s.  Autosac: %s.\r\n",
    (!is_npc () && IS_SET (actflags, PLR_AUTOEXIT)) ? "yes" : "no",
    (!is_npc () && IS_SET (actflags, PLR_AUTOLOOT)) ? "yes" : "no",
    (!is_npc () && IS_SET (actflags, PLR_AUTOSAC)) ? "yes" : "no");
  send_to_char (buf);

  snprintf (buf, sizeof buf, "Wimpy set to %d hit points.\r\n", wimpy);
  send_to_char (buf);

  if (!is_npc ()) {
    snprintf (buf, sizeof buf, "Page pausing set to %d lines of text.\r\n",
      pcdata->pagelen);
    send_to_char (buf);
  }

  if (!is_npc () && pcdata->condition[COND_DRUNK] > 10)
    send_to_char ("You are drunk.\r\n");
  if (!is_npc () && pcdata->condition[COND_THIRST] == 0)
    send_to_char ("You are thirsty.\r\n");
  if (!is_npc () && pcdata->condition[COND_FULL] == 0)
    send_to_char ("You are hungry.\r\n");

  switch (position) {
  case POS_DEAD:
    send_to_char ("You are DEAD!!\r\n");
    break;
  case POS_MORTAL:
    send_to_char ("You are mortally wounded.\r\n");
    break;
  case POS_INCAP:
    send_to_char ("You are incapacitated.\r\n");
    break;
  case POS_STUNNED:
    send_to_char ("You are stunned.\r\n");
    break;
  case POS_SLEEPING:
    send_to_char ("You are sleeping.\r\n");
    break;
  case POS_RESTING:
    send_to_char ("You are resting.\r\n");
    break;
  case POS_STANDING:
    send_to_char ("You are standing.\r\n");
    break;
  case POS_FIGHTING:
    send_to_char ("You are fighting.\r\n");
    break;
  }

  if (level >= 25) {
    snprintf (buf, sizeof buf, "AC: %d.  ", get_ac());
    send_to_char (buf);
  }

  send_to_char ("You are ");
  if (get_ac() >= 101)
    send_to_char ("WORSE than naked!\r\n");
  else if (get_ac() >= 80)
    send_to_char ("naked.\r\n");
  else if (get_ac() >= 60)
    send_to_char ("wearing clothes.\r\n");
  else if (get_ac() >= 40)
    send_to_char ("slightly armored.\r\n");
  else if (get_ac() >= 20)
    send_to_char ("somewhat armored.\r\n");
  else if (get_ac() >= 0)
    send_to_char ("armored.\r\n");
  else if (get_ac() >= -20)
    send_to_char ("well armored.\r\n");
  else if (get_ac() >= -40)
    send_to_char ("strongly armored.\r\n");
  else if (get_ac() >= -60)
    send_to_char ("heavily armored.\r\n");
  else if (get_ac() >= -80)
    send_to_char ("superbly armored.\r\n");
  else if (get_ac() >= -100)
    send_to_char ("divinely armored.\r\n");
  else
    send_to_char ("invincible!\r\n");

  if (level >= 15) {
    snprintf (buf, sizeof buf, "Hitroll: %d  Damroll: %d.\r\n",
      get_hitroll(), get_damroll());
    send_to_char (buf);
  }

  if (level >= 10) {
    snprintf (buf, sizeof buf, "Alignment: %d.  ", alignment);
    send_to_char (buf);
  }

  send_to_char ("You are ");
  if (alignment > 900)
    send_to_char ("angelic.\r\n");
  else if (alignment > 700)
    send_to_char ("saintly.\r\n");
  else if (alignment > 350)
    send_to_char ("good.\r\n");
  else if (alignment > 100)
    send_to_char ("kind.\r\n");
  else if (alignment > -100)
    send_to_char ("neutral.\r\n");
  else if (alignment > -350)
    send_to_char ("mean.\r\n");
  else if (alignment > -700)
    send_to_char ("evil.\r\n");
  else if (alignment > -900)
    send_to_char ("demonic.\r\n");
  else
    send_to_char ("satanic.\r\n");

  if (!affected.empty()) {
    send_to_char ("You are affected by:\r\n");
    AffIter af;
    for (af = affected.begin(); af != affected.end(); af++) {
      snprintf (buf, sizeof buf, "Spell: '%s'", skill_table[(*af)->type].name);
      send_to_char (buf);

      if (level >= 20) {
        snprintf (buf, sizeof buf,
          " modifies %s by %d for %d hours",
          affect_loc_name ((*af)->location).c_str(), (*af)->modifier, (*af)->duration);
        send_to_char (buf);
      }

      send_to_char (".\r\n");
    }
  }

  return;
}

void Character::do_time (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string wt = g_world->world_time();
  snprintf (buf, sizeof buf,
    "%sMerc started up at %s\r\nThe system time is %s\r\n",  wt.c_str(),
    str_boot_time.c_str(), g_world->get_time_text()
    );

  send_to_char (buf);
  return;
}

void Character::do_weather (std::string argument)
{
  if (!is_outside()) {
    send_to_char ("You can't see the weather indoors.\r\n");
    return;
  }

  send_to_char (g_world->world_weather());
  return;
}

void Character::do_help (std::string argument)
{
  sqlite3_stmt *stmt = NULL;

  if (argument.empty())
    argument = "summary";

  char *sql = sqlite3_mprintf("SELECT level, keyword, text FROM helps WHERE level <= %d", level);

  if (sqlite3_prepare(g_db->database, sql, -1, &stmt, 0) != SQLITE_OK) {
    bug_printf("Could not prepare statement: '%s' Error: %s", sql, sqlite3_errmsg(g_db->database));
    sqlite3_free(sql);
    return;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int lvl = sqlite3_column_int( stmt, 0 );
    std::string keyword = (const char*)sqlite3_column_text( stmt, 1 );
    std::string text = (const char*)sqlite3_column_text( stmt, 2 );

    if (is_name (argument, keyword)) {
      if (lvl >= 0 && str_cmp (argument, "imotd")) {
        send_to_char (keyword);
        send_to_char ("\r\n");
      }

      send_to_char (text);
      sqlite3_finalize(stmt);
      sqlite3_free(sql);
      return;
    }
  }

  sqlite3_finalize(stmt);
  sqlite3_free(sql);
  send_to_char ("No help on that word.\r\n");
  return;
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void Character::do_who (std::string argument)
{
  int iClass;
  bool rgfClass[CLASS_MAX];

  /*
   * Set default arguments.
   */
  int iLevelLower = 0;
  int iLevelUpper = MAX_LEVEL;
  bool fClassRestrict = false;
  bool fImmortalOnly = false;

  for (iClass = 0; iClass < CLASS_MAX; iClass++)
    rgfClass[iClass] = false;

  /*
   * Parse arguments.
   */
  int nNumber = 0;
  for (;;) {
    std::string arg;

    argument = one_argument (argument, arg);
    if (arg.empty())
      break;

    if (is_number (arg)) {
      switch (++nNumber) {
      case 1:
        iLevelLower = std::atoi (arg.c_str());
        break;
      case 2:
        iLevelUpper = std::atoi (arg.c_str());
        break;
      default:
        send_to_char ("Only two level numbers allowed.\r\n");
        return;
      }
    } else {
      if (arg.size() < 3) {
        send_to_char ("Classes must be longer than that.\r\n");
        return;
      }

      /*
       * Look for classes to turn on.
       */
      int iClass;

      arg.erase(3);
      if (!str_cmp (arg, "imm")) {
        fImmortalOnly = true;
      } else {
        fClassRestrict = true;
        for (iClass = 0; iClass < CLASS_MAX; iClass++) {
          if (!str_cmp (arg, class_table[iClass].who_name)) {
            rgfClass[iClass] = true;
            break;
          }
        }

        if (iClass == CLASS_MAX) {
          send_to_char ("That's not a class.\r\n");
          return;
        }
      }
    }
  }

  /*
   * Now show matching chars.
   */
  int nMatch = 0;
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  for (DescIter d = descriptor_list.begin();
    d != descriptor_list.end(); d++) {
    Character *wch;
    char const *klass;

    /*
     * Check for match against restrictions.
     * Don't use trust as that exposes trusted mortals.
     */
    if ((*d)->connected != CON_PLAYING || !can_see((*d)->character))
      continue;

    wch = ((*d)->original != NULL) ? (*d)->original : (*d)->character;
    if (wch->level < iLevelLower
      || wch->level > iLevelUpper
      || (fImmortalOnly && wch->level < LEVEL_HERO)
      || (fClassRestrict && !rgfClass[wch->klass]))
      continue;

    nMatch++;

    /*
     * Figure out what to print for class.
     */
    klass = class_table[wch->klass].who_name;
    switch (wch->level) {
    default:
      break;
    case MAX_LEVEL - 0:
      klass = "GOD";
      break;
    case MAX_LEVEL - 1:
      klass = "SUP";
      break;
    case MAX_LEVEL - 2:
      klass = "DEI";
      break;
    case MAX_LEVEL - 3:
      klass = "ANG";
      break;
    }

    /*
     * Format it up.
     */
    snprintf (buf + strlen(buf), sizeof(buf) - strlen(buf), "[%2d %s] %s%s%s%s\r\n",
      wch->level,
      klass,
      IS_SET (wch->actflags, PLR_KILLER) ? "(KILLER) " : "",
      IS_SET (wch->actflags, PLR_THIEF) ? "(THIEF) " : "",
      wch->name.c_str(), wch->pcdata->title.c_str());
  }

  char buf2[MAX_STRING_LENGTH];
  snprintf (buf2, sizeof buf2, "You see %d player%s in the game.\r\n",
    nMatch, nMatch == 1 ? "" : "s");
  strncat (buf, buf2, sizeof(buf) - sizeof(buf2));
  send_to_char (buf);
  return;
}

void Character::do_inventory (std::string argument)
{
  send_to_char ("You are carrying:\r\n");
  show_list_to_char (carrying, true, true);
  return;
}

void Character::do_equipment (std::string argument)
{

  send_to_char ("You are using:\r\n");
  bool found = false;
  for (int iWear = 0; iWear < MAX_WEAR; iWear++) {
    Object *obj;
    if ((obj = get_eq_char (iWear)) == NULL)
      continue;

    send_to_char (where_name[iWear]);
    if (can_see_obj(obj)) {
      send_to_char (obj->format_obj_to_char (this, true));
      send_to_char ("\r\n");
    } else {
      send_to_char ("something.\r\n");
    }
    found = true;
  }

  if (!found)
    send_to_char ("Nothing.\r\n");

  return;
}

void Character::do_compare (std::string argument)
{
  std::string arg1, arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1.empty()) {
    send_to_char ("Compare what to what?\r\n");
    return;
  }

  Object *obj1;
  if ((obj1 = get_obj_carry (arg1)) == NULL) {
    send_to_char ("You do not have that item.\r\n");
    return;
  }

  Object *obj2 = NULL;
  if (arg2.empty()) {
    ObjIter o;
    for (o = carrying.begin(); o != carrying.end(); o++) {
      obj2 = *o;
      if (obj2->wear_loc != WEAR_NONE && can_see_obj(obj2)
        && obj1->item_type == obj2->item_type
        && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
        break;
    }

    if (obj2 == NULL) {
      send_to_char ("You aren't wearing anything comparable.\r\n");
      return;
    }
  } else {
    if ((obj2 = get_obj_carry (arg2)) == NULL) {
      send_to_char ("You do not have that item.\r\n");
      return;
    }
  }

  const char* msg = NULL;
  int value1 = 0;
  int value2 = 0;

  if (obj1 == obj2) {
    msg = "You compare $p to itself.  It looks about the same.";
  } else if (obj1->item_type != obj2->item_type) {
    msg = "You can't compare $p and $P.";
  } else {
    switch (obj1->item_type) {
    default:
      msg = "You can't compare $p and $P.";
      break;

    case ITEM_ARMOR:
      value1 = obj1->value[0];
      value2 = obj2->value[0];
      break;

    case ITEM_WEAPON:
      value1 = obj1->value[1] + obj1->value[2];
      value2 = obj2->value[1] + obj2->value[2];
      break;
    }
  }

  if (msg == NULL) {
    if (value1 == value2)
      msg = "$p and $P look about the same.";
    else if (value1 > value2)
      msg = "$p looks better than $P.";
    else
      msg = "$p looks worse than $P.";
  }

  act (msg, obj1, obj2, TO_CHAR);
  return;
}

void Character::do_credits (std::string argument)
{
  do_help ("diku");
  return;
}

void Character::do_where (std::string argument)
{
  std::string arg;
  char buf[MAX_STRING_LENGTH];
  Character *victim;
  bool found = false;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Players near you:\r\n");
    for (DescIter d = descriptor_list.begin();
      d != descriptor_list.end(); d++) {
      if ((*d)->connected == CON_PLAYING
        && (victim = (*d)->character) != NULL && !victim->is_npc ()
        && victim->in_room != NULL
        && victim->in_room->area == in_room->area
        && can_see(victim)) {
        found = true;
        snprintf (buf, sizeof buf, "%-28s %s\r\n", victim->name.c_str(), victim->in_room->name.c_str());
        send_to_char (buf);
      }
    }
    if (!found)
      send_to_char ("None\r\n");
  } else {
    for (CharIter c = char_list.begin(); c != char_list.end(); c++) {
      victim = *c;
      if (victim->in_room != NULL
        && victim->in_room->area == in_room->area
        && !victim->is_affected (AFF_HIDE)
        && !victim->is_affected (AFF_SNEAK)
        && can_see(victim)
        && is_name (arg, victim->name)) {
        found = true;
        snprintf (buf, sizeof buf, "%-28s %s\r\n",
          victim->describe_to(this).c_str(), victim->in_room->name.c_str());
        send_to_char (buf);
        break;
      }
    }
    if (!found)
      act ("You didn't find any $T.", NULL, arg.c_str(), TO_CHAR);
  }

  return;
}

void Character::do_consider (std::string argument)
{
  std::string arg, msg, buf;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Consider killing whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They're not here.\r\n");
    return;
  }

  if (!victim->is_npc ()) {
    send_to_char ("The gods do not accept this type of sacrafice.\r\n");
    return;
  }

  int diff = victim->level - level;

  if (diff <= -10)
    msg = "You can kill $N naked and weaponless.";
  else if (diff <= -5)
    msg = "$N is no match for you.";
  else if (diff <= -2)
    msg = "$N looks like an easy kill.";
  else if (diff <= 1)
    msg = "The perfect match!";
  else if (diff <= 4)
    msg = "$N says 'Do you feel lucky, punk?'.";
  else if (diff <= 9)
    msg = "$N laughs at you mercilessly.";
  else
    msg = "Death will thank you for your gift.";

  act (msg, NULL, victim, TO_CHAR);

  /* additions by king@tinuviel.cs.wcu.edu */
  int hpdiff = (hit - victim->hit);

  if (((diff >= 0) && (hpdiff <= 0))
    || ((diff <= 0) && (hpdiff >= 0))) {
    send_to_char ("Also,");
  } else {
    send_to_char ("However,");
  }

  if (hpdiff >= 101)
    buf = " you are currently much healthier than $E.";
  if (hpdiff <= 100)
    buf = " you are currently healthier than $E.";
  if (hpdiff <= 50)
    buf = " you are currently slightly healthier than $E.";
  if (hpdiff <= 25)
    buf = " you are a teensy bit healthier than $E.";
  if (hpdiff <= 0)
    buf = " $E is a teensy bit healthier than you.";
  if (hpdiff <= -25)
    buf = " $E is slightly healthier than you.";
  if (hpdiff <= -50)
    buf = " $E is healthier than you.";
  if (hpdiff <= -100)
    buf = " $E is much healthier than you.";

  act (buf, NULL, victim, TO_CHAR);
  return;
}

void Character::do_title (std::string argument)
{
  if (is_npc ())
    return;

  if (argument.empty()) {
    send_to_char ("Change your title to what?\r\n");
    return;
  }

  if (argument.size() > 50)
    argument.erase(50);

  smash_tilde (argument);
  set_title(argument);
  send_to_char ("Ok.\r\n");
}

void Character::do_description (std::string argument)
{
  if (!argument.empty()) {
    std::string buf;
    smash_tilde (argument);
    if (argument[0] == '+') {
      if (!description.empty())
        buf = description;
      argument.erase(0,1);
      argument.erase(0, argument.find_first_not_of(" "));
    }

    if (buf.size() + argument.size() >= MAX_STRING_LENGTH - 2) {
      send_to_char ("Description too long.\r\n");
      return;
    }

    buf.append(argument);
    buf.append("\r\n");
    description = buf;
  }

  send_to_char ("Your description is:\r\n");
  send_to_char (!description.empty() ? description.c_str() : "(None).\r\n");
  return;
}

void Character::do_report (std::string argument)
{
  char buf[MAX_INPUT_LENGTH];

  snprintf (buf, sizeof buf,
    "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
    hit, max_hit,
    mana, max_mana, move, max_move, exp);

  send_to_char (buf);

  snprintf (buf, sizeof buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
    hit, max_hit,
    mana, max_mana, move, max_move, exp);

  act (buf, NULL, NULL, TO_ROOM);

  return;
}

void Character::do_practice (std::string argument)
{
  if (is_npc ())
    return;

  if (level < 3) {
    send_to_char
      ("You must be third level to practice.  Go train instead!\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  std::string buf1;
  int sn;

  if (argument.empty()) {
    int col;

    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name == NULL)
        break;
      if (level < skill_table[sn].skill_level[klass])
        continue;

      snprintf (buf, sizeof buf, "%18s %3d%%  ",
        skill_table[sn].name, pcdata->learned[sn]);
      buf1.append(buf);
      if (++col % 3 == 0)
        buf1.append("\r\n");
    }

    if (col % 3 != 0)
      buf1.append("\r\n");

    snprintf (buf, sizeof buf, "You have %d practice sessions left.\r\n", practice);
    buf1.append(buf);
    send_to_char (buf1);
  } else {
    int adept;

    if (!is_awake ()) {
      send_to_char ("In your dreams, or what?\r\n");
      return;
    }

    CharIter mob;
    for (mob = in_room->people.begin(); mob != in_room->people.end(); mob++) {
      if ((*mob)->is_npc () && IS_SET ((*mob)->actflags, ACT_PRACTICE))
        break;
    }

    if (mob == in_room->people.end()) {
      send_to_char ("You can't do that here.\r\n");
      return;
    }

    if (practice <= 0) {
      send_to_char ("You have no practice sessions left.\r\n");
      return;
    }

    if ((sn = skill_lookup (argument)) < 0 || (!is_npc ()
        && level < skill_table[sn].skill_level[klass])) {
      send_to_char ("You can't practice that.\r\n");
      return;
    }

    adept = is_npc () ? 100 : class_table[klass].skill_adept;

    if (pcdata->learned[sn] >= adept) {
      snprintf (buf, sizeof buf, "You are already an adept of %s.\r\n",
        skill_table[sn].name);
      send_to_char (buf);
    } else {
      practice--;
      pcdata->learned[sn] += int_app[get_curr_int()].learn;
      if (pcdata->learned[sn] < adept) {
        act ("You practice $T.", NULL, skill_table[sn].name, TO_CHAR);
        act ("$n practices $T.", NULL, skill_table[sn].name, TO_ROOM);
      } else {
        pcdata->learned[sn] = adept;
        act ("You are now an adept of $T.",
          NULL, skill_table[sn].name, TO_CHAR);
        act ("$n is now an adept of $T.",
          NULL, skill_table[sn].name, TO_ROOM);
      }
    }
  }
  return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void Character::do_wimpy (std::string argument)
{
  std::string arg;
  int wpy;

  one_argument (argument, arg);

  if (arg.empty())
    wpy = max_hit / 5;
  else
    wpy = std::atoi (arg.c_str());

  if (wpy < 0) {
    send_to_char ("Your courage exceeds your wisdom.\r\n");
    return;
  }

  if (wpy > max_hit) {
    send_to_char ("Such cowardice ill becomes you.\r\n");
    return;
  }

  wimpy = wpy;
  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "Wimpy set to %d hit points.\r\n", wimpy);
  send_to_char (buf);
  return;
}

void Character::do_password (std::string argument)
{
  if (is_npc ())
    return;

  std::string arg1;
  std::string arg2;
  char *pwdnew;
  char *p;
  char cEnd;

  /*
   * Can't use one_argument here because it smashes case.
   * So we just steal all its code.  Bleagh.
   */
  std::string::iterator argp = argument.begin();

  arg1.erase();
  while (argp != argument.end() && isspace (*argp))
    argp++;

  cEnd = ' ';
  if (*argp == '\'' || *argp == '"')
    cEnd = *argp++;

  while (argp != argument.end()) {
    if (*argp == cEnd) {
      break;
    }
    arg1.append(1, *argp);
    argp++;
  }

  argp = argument.begin();

  arg2.erase();
  while (argp != argument.end() && isspace (*argp))
    argp++;

  cEnd = ' ';
  if (*argp == '\'' || *argp == '"')
    cEnd = *argp++;

  while (argp != argument.end()) {
    if (*argp == cEnd) {
      break;
    }
    arg2.append(1, *argp);
    argp++;
  }

  if (arg1.empty() || arg2.empty()) {
    send_to_char ("Syntax: password <old> <new>.\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];  // Needed for Windows crypt
  strncpy(buf,arg1.c_str(), sizeof buf);
  if (strcmp (crypt (buf, pcdata->pwd.c_str()), pcdata->pwd.c_str())) {
    wait_state (40);
    send_to_char ("Wrong password.  Wait 10 seconds.\r\n");
    return;
  }

  if (arg2.size() < 5) {
    send_to_char ("New password must be at least five characters long.\r\n");
    return;
  }

  /*
   * No tilde allowed because of player file format.
   */
  strncpy(buf,arg2.c_str(), sizeof buf);
  pwdnew = crypt (buf, name.c_str());
  for (p = pwdnew; *p != '\0'; p++) {
    if (*p == '~') {
      send_to_char ("New password not acceptable, try again.\r\n");
      return;
    }
  }

  pcdata->pwd = pwdnew;
  save_char_obj();
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_socials (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  int col = 0;
  sqlite3_stmt *stmt = NULL;

  if (sqlite3_prepare(g_db->database,
      "SELECT name FROM socials ORDER BY name ASC",
      -1, &stmt, 0) != SQLITE_OK) {
    bug_printf("Could not prepare statement: %s", sqlite3_errmsg(g_db->database));
    return;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    snprintf (buf, sizeof buf, "%-12s", sqlite3_column_text( stmt, 0 ));
    send_to_char (buf);
    if (++col % 6 == 0)
      send_to_char ("\r\n");
  }

  if (col % 6 != 0)
    send_to_char ("\r\n");
  sqlite3_finalize(stmt);
  return;
}

/*
 * Contributed by Alander.
 */
void Character::do_commands (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string buf1;
  int cmd;
  int col;

  col = 0;
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
    if (cmd_table[cmd].level < LEVEL_HERO
      && cmd_table[cmd].level <= get_trust ()) {
      snprintf (buf, sizeof buf, "%-12s", cmd_table[cmd].name);
      buf1.append(buf);
      if (++col % 6 == 0)
        buf1.append("\r\n");
    }
  }

  if (col % 6 != 0)
    buf1.append("\r\n");

  send_to_char (buf1);
  return;
}

void Character::do_channels (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    if (!is_npc () && IS_SET (actflags, PLR_SILENCE)) {
      send_to_char ("You are silenced.\r\n");
      return;
    }

    send_to_char ("Channels:");

    send_to_char (!IS_SET (deaf, CHANNEL_AUCTION)
      ? " +AUCTION" : " -auction");

    send_to_char (!IS_SET (deaf, CHANNEL_CHAT)
      ? " +CHAT" : " -chat");

    if (is_hero()) {
      send_to_char (!IS_SET (deaf, CHANNEL_IMMTALK)
        ? " +IMMTALK" : " -immtalk");
    }

    send_to_char (!IS_SET (deaf, CHANNEL_MUSIC)
      ? " +MUSIC" : " -music");

    send_to_char (!IS_SET (deaf, CHANNEL_QUESTION)
      ? " +QUESTION" : " -question");

    send_to_char (!IS_SET (deaf, CHANNEL_SHOUT)
      ? " +SHOUT" : " -shout");

    send_to_char (!IS_SET (deaf, CHANNEL_YELL)
      ? " +YELL" : " -yell");

    send_to_char (".\r\n");
  } else {
    bool fClear;
    int bit;

    if (arg[0] == '+')
      fClear = true;
    else if (arg[0] == '-')
      fClear = false;
    else {
      send_to_char ("Channels -channel or +channel?\r\n");
      return;
    }

    if (!str_cmp (arg.substr(1), "auction"))
      bit = CHANNEL_AUCTION;
    else if (!str_cmp (arg.substr(1), "chat"))
      bit = CHANNEL_CHAT;
    else if (!str_cmp (arg.substr(1), "immtalk"))
      bit = CHANNEL_IMMTALK;
    else if (!str_cmp (arg.substr(1), "music"))
      bit = CHANNEL_MUSIC;
    else if (!str_cmp (arg.substr(1), "question"))
      bit = CHANNEL_QUESTION;
    else if (!str_cmp (arg.substr(1), "shout"))
      bit = CHANNEL_SHOUT;
    else if (!str_cmp (arg.substr(1), "yell"))
      bit = CHANNEL_YELL;
    else {
      send_to_char ("Set or clear which channel?\r\n");
      return;
    }

    if (fClear)
      REMOVE_BIT (deaf, bit);
    else
      SET_BIT (deaf, bit);

    send_to_char ("Ok.\r\n");
  }

  return;
}

/*
 * Contributed by Grodyn.
 */
void Character::do_config (std::string argument)
{
  if (is_npc ())
    return;

  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("[ Keyword  ] Option\r\n");

    send_to_char (IS_SET (actflags, PLR_AUTOEXIT)
      ? "[+AUTOEXIT ] You automatically see exits.\r\n"
      : "[-autoexit ] You don't automatically see exits.\r\n");

    send_to_char (IS_SET (actflags, PLR_AUTOLOOT)
      ? "[+AUTOLOOT ] You automatically loot corpses.\r\n"
      : "[-autoloot ] You don't automatically loot corpses.\r\n");

    send_to_char (IS_SET (actflags, PLR_AUTOSAC)
      ? "[+AUTOSAC  ] You automatically sacrifice corpses.\r\n"
      : "[-autosac  ] You don't automatically sacrifice corpses.\r\n");

    send_to_char (IS_SET (actflags, PLR_BLANK)
      ? "[+BLANK    ] You have a blank line before your prompt.\r\n"
      : "[-blank    ] You have no blank line before your prompt.\r\n");

    send_to_char (IS_SET (actflags, PLR_BRIEF)
      ? "[+BRIEF    ] You see brief descriptions.\r\n"
      : "[-brief    ] You see long descriptions.\r\n");

    send_to_char (IS_SET (actflags, PLR_COMBINE)
      ? "[+COMBINE  ] You see object lists in combined format.\r\n"
      : "[-combine  ] You see object lists in single format.\r\n");

    send_to_char (IS_SET (actflags, PLR_PROMPT)
      ? "[+PROMPT   ] You have a prompt.\r\n"
      : "[-prompt   ] You don't have a prompt.\r\n");

    send_to_char (IS_SET (actflags, PLR_TELNET_GA)
      ? "[+TELNETGA ] You receive a telnet GA sequence.\r\n"
      : "[-telnetga ] You don't receive a telnet GA sequence.\r\n");

    send_to_char (IS_SET (actflags, PLR_SILENCE)
      ? "[+SILENCE  ] You are silenced.\r\n" : "");

    send_to_char (!IS_SET (actflags, PLR_NO_EMOTE)
      ? "" : "[-emote    ] You can't emote.\r\n");

    send_to_char (!IS_SET (actflags, PLR_NO_TELL)
      ? "" : "[-tell     ] You can't use 'tell'.\r\n");
  } else {
    bool fSet;
    int bit;

    if (arg[0] == '+')
      fSet = true;
    else if (arg[0] == '-')
      fSet = false;
    else {
      send_to_char ("Config -option or +option?\r\n");
      return;
    }

    if (!str_cmp (arg.substr(1), "autoexit"))
      bit = PLR_AUTOEXIT;
    else if (!str_cmp (arg.substr(1), "autoloot"))
      bit = PLR_AUTOLOOT;
    else if (!str_cmp (arg.substr(1), "autosac"))
      bit = PLR_AUTOSAC;
    else if (!str_cmp (arg.substr(1), "blank"))
      bit = PLR_BLANK;
    else if (!str_cmp (arg.substr(1), "brief"))
      bit = PLR_BRIEF;
    else if (!str_cmp (arg.substr(1), "combine"))
      bit = PLR_COMBINE;
    else if (!str_cmp (arg.substr(1), "prompt"))
      bit = PLR_PROMPT;
    else if (!str_cmp (arg.substr(1), "telnetga"))
      bit = PLR_TELNET_GA;
    else {
      send_to_char ("Config which option?\r\n");
      return;
    }

    if (fSet)
      SET_BIT (actflags, bit);
    else
      REMOVE_BIT (actflags, bit);

    send_to_char ("Ok.\r\n");
  }

  return;
}

void Character::do_wizlist (std::string argument)
{
  do_help ("wizlist");
  return;
}

void Character::do_spells (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string buf1;

  if ((!is_npc () && !class_table[klass].fMana)
    || is_npc ()) {
    send_to_char ("You do not know how to cast spells!\r\n");
    return;
  }

  int col = 0;
  for (int sn = 0; sn < MAX_SKILL; sn++) {
    if (skill_table[sn].name == NULL)
      break;
    if ((level < skill_table[sn].skill_level[klass])
      || (skill_table[sn].skill_level[klass] > LEVEL_HERO))
      continue;

    snprintf (buf, sizeof buf, "%18s %3dpts ", skill_table[sn].name, mana_cost (sn));
    buf1.append(buf);
    if (++col % 3 == 0)
      buf1.append("\r\n");
  }

  if (col % 3 != 0)
    buf1.append("\r\n");

  send_to_char (buf1);
  return;

}

void Character::do_slist (std::string argument)
{
  if ((!is_npc () && !class_table[klass].fMana) || is_npc ()) {
    send_to_char ("You do not need any stinking spells!\r\n");
    return;
  }

  std::string buf1;
  buf1.append("ALL Spells available for your class.\r\n\r\n");
  buf1.append("Lv          Spells\r\n\r\n");

  for (int lvl = 1; lvl < LEVEL_IMMORTAL; lvl++) {
    int col = 0;
    bool pSpell = true;
    char buf[MAX_STRING_LENGTH];

    for (int sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name == NULL)
        break;
      if (skill_table[sn].skill_level[klass] != lvl)
        continue;

      if (pSpell) {
        snprintf (buf, sizeof buf, "%2d:", level);
        buf1.append(buf);
        pSpell = false;
      }

      if (++col % 5 == 0)
        buf1.append("   ");

      snprintf (buf, sizeof buf, "%18s", skill_table[sn].name);
      buf1.append(buf);

      if (col % 4 == 0)
        buf1.append("\r\n");

    }

    if (col % 4 != 0)
      buf1.append("\r\n");
  }
  send_to_char (buf1);
  return;
}

/* by passing the conf command - Kahn */
void Character::do_autoexit (std::string argument)
{
  (IS_SET (actflags, PLR_AUTOEXIT)
    ? do_config ("-autoexit")
    : do_config ("+autoexit"));
}

void Character::do_autoloot (std::string argument)
{
  (IS_SET (actflags, PLR_AUTOLOOT)
    ? do_config ("-autoloot")
    : do_config ("+autoloot"));
}

void Character::do_autosac (std::string argument)
{
  (IS_SET (actflags, PLR_AUTOSAC)
    ? do_config ("-autosac")
    : do_config ("+autosac"));
}

void Character::do_blank (std::string argument)
{
  (IS_SET (actflags, PLR_BLANK)
    ? do_config ("-blank")
    : do_config ("+blank"));
}

void Character::do_brief (std::string argument)
{
  (IS_SET (actflags, PLR_BRIEF)
    ? do_config ("-brief")
    : do_config ("+brief"));
}

void Character::do_combine (std::string argument)
{
  (IS_SET (actflags, PLR_COMBINE)
    ? do_config ("-combine")
    : do_config ("+combine"));
}

void Character::do_pagelen (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string arg;
  int lines;

  one_argument (argument, arg);

  if (arg.empty())
    lines = 20;
  else
    lines = std::atoi (arg.c_str());

  if (lines < 1) {
    send_to_char
      ("Negative or Zero values for a page pause is not legal.\r\n");
    return;
  }

  pcdata->pagelen = lines;
  snprintf (buf, sizeof buf, "Page pause set to %d lines.\r\n", lines);
  send_to_char (buf);
  return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void Character::do_prompt (std::string argument)
{
  if (argument.empty()) {
    (IS_SET (actflags, PLR_PROMPT)
      ? do_config ("-prompt")
      : do_config ("+prompt"));
    return;
  }

  std::string buf;

  if (!strcmp (argument.c_str(), "all"))
    buf = "<%hhp %mm %vmv> ";
  else {
    smash_tilde (argument);
    if (argument.size() > 50)
      argument.erase(50);
    buf = argument;
  }

  prompt = buf;
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_auto (std::string argument)
{
  do_config ("");
  return;
}

void Character::do_north (std::string argument)
{
  move_char (DIR_NORTH);
  return;
}

void Character::do_east (std::string argument)
{
  move_char (DIR_EAST);
  return;
}

void Character::do_south (std::string argument)
{
  move_char (DIR_SOUTH);
  return;
}

void Character::do_west (std::string argument)
{
  move_char (DIR_WEST);
  return;
}

void Character::do_up (std::string argument)
{
  move_char (DIR_UP);
  return;
}

void Character::do_down (std::string argument)
{
  move_char (DIR_DOWN);
  return;
}

void Character::do_open (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Open what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    /* 'open object' */
    if (obj->item_type != ITEM_CONTAINER) {
      send_to_char ("That's not a container.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSED)) {
      send_to_char ("It's already open.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSEABLE)) {
      send_to_char ("You can't do that.\r\n");
      return;
    }
    if (IS_SET (obj->value[1], CONT_LOCKED)) {
      send_to_char ("It's locked.\r\n");
      return;
    }

    REMOVE_BIT (obj->value[1], CONT_CLOSED);
    send_to_char ("Ok.\r\n");
    act ("$n opens $p.", obj, NULL, TO_ROOM);
    return;
  }

  int door;
  if ((door = find_door (arg)) >= 0) {
    /* 'open door' */
    Room *to_room;
    Exit *pexit;
    Exit *pexit_rev;

    pexit = in_room->exit[door];
    if (!IS_SET (pexit->exit_info, EX_CLOSED)) {
      send_to_char ("It's already open.\r\n");
      return;
    }
    if (IS_SET (pexit->exit_info, EX_LOCKED)) {
      send_to_char ("It's locked.\r\n");
      return;
    }

    REMOVE_BIT (pexit->exit_info, EX_CLOSED);
    act ("$n opens the $d.", NULL, pexit->name.c_str(), TO_ROOM);
    send_to_char ("Ok.\r\n");

    /* open the other side */
    if ((to_room = pexit->to_room) != NULL
      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
      && pexit_rev->to_room == in_room) {

      REMOVE_BIT (pexit_rev->exit_info, EX_CLOSED);
      CharIter rch;
      for (rch = to_room->people.begin(); rch != to_room->people.end(); rch++)
        (*rch)->act ("The $d opens.", NULL, pexit_rev->name.c_str(), TO_CHAR);
    }
  }

  return;
}

void Character::do_close (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Close what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    /* 'close object' */
    if (obj->item_type != ITEM_CONTAINER) {
      send_to_char ("That's not a container.\r\n");
      return;
    }
    if (IS_SET (obj->value[1], CONT_CLOSED)) {
      send_to_char ("It's already closed.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSEABLE)) {
      send_to_char ("You can't do that.\r\n");
      return;
    }

    SET_BIT (obj->value[1], CONT_CLOSED);
    send_to_char ("Ok.\r\n");
    act ("$n closes $p.", obj, NULL, TO_ROOM);
    return;
  }

  int door;
  if ((door = find_door (arg)) >= 0) {
    /* 'close door' */
    Room *to_room;
    Exit *pexit;
    Exit *pexit_rev;

    pexit = in_room->exit[door];
    if (IS_SET (pexit->exit_info, EX_CLOSED)) {
      send_to_char ("It's already closed.\r\n");
      return;
    }

    SET_BIT (pexit->exit_info, EX_CLOSED);
    act ("$n closes the $d.", NULL, pexit->name.c_str(), TO_ROOM);
    send_to_char ("Ok.\r\n");

    /* close the other side */
    if ((to_room = pexit->to_room) != NULL
      && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
      && pexit_rev->to_room == in_room) {

      SET_BIT (pexit_rev->exit_info, EX_CLOSED);
      CharIter rch;
      for (rch = to_room->people.begin(); rch != to_room->people.end(); rch++)
        (*rch)->act ("The $d closes.", NULL, pexit_rev->name.c_str(), TO_CHAR);
    }
  }

  return;
}

void Character::do_lock (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Lock what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    /* 'lock object' */
    if (obj->item_type != ITEM_CONTAINER) {
      send_to_char ("That's not a container.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (obj->value[2] < 0) {
      send_to_char ("It can't be locked.\r\n");
      return;
    }
    if (!has_key(obj->value[2])) {
      send_to_char ("You lack the key.\r\n");
      return;
    }
    if (IS_SET (obj->value[1], CONT_LOCKED)) {
      send_to_char ("It's already locked.\r\n");
      return;
    }

    SET_BIT (obj->value[1], CONT_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n locks $p.", obj, NULL, TO_ROOM);
    return;
  }

  int door;
  if ((door = find_door (arg)) >= 0) {
    /* 'lock door' */
    Room *to_room;
    Exit *pexit;
    Exit *pexit_rev;

    pexit = in_room->exit[door];
    if (!IS_SET (pexit->exit_info, EX_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (pexit->key < 0) {
      send_to_char ("It can't be locked.\r\n");
      return;
    }
    if (!has_key(pexit->key)) {
      send_to_char ("You lack the key.\r\n");
      return;
    }
    if (IS_SET (pexit->exit_info, EX_LOCKED)) {
      send_to_char ("It's already locked.\r\n");
      return;
    }

    SET_BIT (pexit->exit_info, EX_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n locks the $d.", NULL, pexit->name.c_str(), TO_ROOM);

    /* lock the other side */
    if ((to_room = pexit->to_room) != NULL
      && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
      && pexit_rev->to_room == in_room) {
      SET_BIT (pexit_rev->exit_info, EX_LOCKED);
    }
  }

  return;
}

void Character::do_unlock (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Unlock what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    /* 'unlock object' */
    if (obj->item_type != ITEM_CONTAINER) {
      send_to_char ("That's not a container.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (obj->value[2] < 0) {
      send_to_char ("It can't be unlocked.\r\n");
      return;
    }
    if (!has_key(obj->value[2])) {
      send_to_char ("You lack the key.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_LOCKED)) {
      send_to_char ("It's already unlocked.\r\n");
      return;
    }

    REMOVE_BIT (obj->value[1], CONT_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n unlocks $p.", obj, NULL, TO_ROOM);
    return;
  }

  int door;
  if ((door = find_door (arg)) >= 0) {
    /* 'unlock door' */
    Room *to_room;
    Exit *pexit;
    Exit *pexit_rev;

    pexit = in_room->exit[door];
    if (!IS_SET (pexit->exit_info, EX_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (pexit->key < 0) {
      send_to_char ("It can't be unlocked.\r\n");
      return;
    }
    if (!has_key(pexit->key)) {
      send_to_char ("You lack the key.\r\n");
      return;
    }
    if (!IS_SET (pexit->exit_info, EX_LOCKED)) {
      send_to_char ("It's already unlocked.\r\n");
      return;
    }

    REMOVE_BIT (pexit->exit_info, EX_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n unlocks the $d.", NULL, pexit->name.c_str(), TO_ROOM);

    /* unlock the other side */
    if ((to_room = pexit->to_room) != NULL
      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
      && pexit_rev->to_room == in_room) {
      REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
    }
  }

  return;
}

void Character::do_pick (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Pick what?\r\n");
    return;
  }

  wait_state (skill_table[skill_lookup("pick lock")].beats);

  /* look for guards */
  CharIter rch;
  for (rch = in_room->people.begin(); rch != in_room->people.end(); rch++) {
    if ((*rch)->is_npc () && (*rch)->is_awake () && level + 5 < (*rch)->level) {
      act ("$N is standing too close to the lock.", NULL, *rch, TO_CHAR);
      return;
    }
  }

  if (!is_npc () && number_percent () > pcdata->learned[skill_lookup("pick lock")]) {
    send_to_char ("You failed.\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_here (arg)) != NULL) {
    /* 'pick object' */
    if (obj->item_type != ITEM_CONTAINER) {
      send_to_char ("That's not a container.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (obj->value[2] < 0) {
      send_to_char ("It can't be unlocked.\r\n");
      return;
    }
    if (!IS_SET (obj->value[1], CONT_LOCKED)) {
      send_to_char ("It's already unlocked.\r\n");
      return;
    }
    if (IS_SET (obj->value[1], CONT_PICKPROOF)) {
      send_to_char ("You failed.\r\n");
      return;
    }

    REMOVE_BIT (obj->value[1], CONT_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n picks $p.", obj, NULL, TO_ROOM);
    return;
  }

  int door;
  if ((door = find_door (arg)) >= 0) {
    /* 'pick door' */
    Room *to_room;
    Exit *pexit;
    Exit *pexit_rev;

    pexit = in_room->exit[door];
    if (!IS_SET (pexit->exit_info, EX_CLOSED)) {
      send_to_char ("It's not closed.\r\n");
      return;
    }
    if (pexit->key < 0) {
      send_to_char ("It can't be picked.\r\n");
      return;
    }
    if (!IS_SET (pexit->exit_info, EX_LOCKED)) {
      send_to_char ("It's already unlocked.\r\n");
      return;
    }
    if (IS_SET (pexit->exit_info, EX_PICKPROOF)) {
      send_to_char ("You failed.\r\n");
      return;
    }

    REMOVE_BIT (pexit->exit_info, EX_LOCKED);
    send_to_char ("*Click*\r\n");
    act ("$n picks the $d.", NULL, pexit->name.c_str(), TO_ROOM);

    /* pick the other side */
    if ((to_room = pexit->to_room) != NULL
      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
      && pexit_rev->to_room == in_room) {
      REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
    }
  }

  return;
}

void Character::do_stand (std::string argument)
{
  switch (position) {
  case POS_SLEEPING:
    if (is_affected (AFF_SLEEP)) {
      send_to_char ("You can't wake up!\r\n");
      return;
    }

    send_to_char ("You wake and stand up.\r\n");
    act ("$n wakes and stands up.", NULL, NULL, TO_ROOM);
    position = POS_STANDING;
    break;

  case POS_RESTING:
    send_to_char ("You stand up.\r\n");
    act ("$n stands up.", NULL, NULL, TO_ROOM);
    position = POS_STANDING;
    break;

  case POS_STANDING:
    send_to_char ("You are already standing.\r\n");
    break;

  case POS_FIGHTING:
    send_to_char ("You are already fighting!\r\n");
    break;
  }

  return;
}

void Character::do_rest (std::string argument)
{
  switch (position) {
  case POS_SLEEPING:
    send_to_char ("You are already sleeping.\r\n");
    break;

  case POS_RESTING:
    send_to_char ("You are already resting.\r\n");
    break;

  case POS_STANDING:
    send_to_char ("You rest.\r\n");
    act ("$n rests.", NULL, NULL, TO_ROOM);
    position = POS_RESTING;
    break;

  case POS_FIGHTING:
    send_to_char ("You are already fighting!\r\n");
    break;
  }

  return;
}

void Character::do_sleep (std::string argument)
{
  switch (position) {
  case POS_SLEEPING:
    send_to_char ("You are already sleeping.\r\n");
    break;

  case POS_RESTING:
  case POS_STANDING:
    send_to_char ("You sleep.\r\n");
    act ("$n sleeps.", NULL, NULL, TO_ROOM);
    position = POS_SLEEPING;
    break;

  case POS_FIGHTING:
    send_to_char ("You are already fighting!\r\n");
    break;
  }

  return;
}

void Character::do_wake (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    do_stand (argument);
    return;
  }

  if (!is_awake ()) {
    send_to_char ("You are asleep yourself!\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_awake ()) {
    act ("$N is already awake.", NULL, victim, TO_CHAR);
    return;
  }

  if (victim->is_affected (AFF_SLEEP)) {
    act ("You can't wake $M!", NULL, victim, TO_CHAR);
    return;
  }

  act ("You wake $M.", NULL, victim, TO_CHAR);
  act ("$n wakes you.", NULL, victim, TO_VICT);
  victim->position = POS_STANDING;
  return;
}

void Character::do_sneak (std::string argument)
{
  Affect af;
  int snk = skill_lookup("sneak");

  send_to_char ("You attempt to move silently.\r\n");
  affect_strip (snk);

  if (is_npc () || number_percent () < pcdata->learned[snk]) {
    af.type = snk;
    af.duration = level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SNEAK;
    affect_to_char(&af);
  }

  return;
}

void Character::do_hide (std::string argument)
{
  send_to_char ("You attempt to hide.\r\n");

  if (is_affected (AFF_HIDE))
    REMOVE_BIT (affected_by, AFF_HIDE);

  if (is_npc () || number_percent () < pcdata->learned[skill_lookup("hide")])
    SET_BIT (affected_by, AFF_HIDE);

  return;
}

/*
 * Contributed by Alander.
 */
void Character::do_visible (std::string argument)
{
  affect_strip (skill_lookup("invis"));
  affect_strip (skill_lookup("mass invis"));
  affect_strip (skill_lookup("sneak"));
  REMOVE_BIT (affected_by, AFF_HIDE);
  REMOVE_BIT (affected_by, AFF_INVISIBLE);
  REMOVE_BIT (affected_by, AFF_SNEAK);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_recall (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  Room *location;

  act ("$n prays for transportation!", 0, 0, TO_ROOM);

  if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL) {
    send_to_char ("You are completely lost.\r\n");
    return;
  }

  if (in_room == location)
    return;

  if (IS_SET (in_room->room_flags, ROOM_NO_RECALL)
    || is_affected (AFF_CURSE)) {
    send_to_char ("God has forsaken you.\r\n");
    return;
  }

  if (fighting != NULL) {
    int lose;

    if (number_percent() <= 50) {
      wait_state (4);
      lose = (desc != NULL) ? 50 : 100;
      gain_exp(0 - lose);
      snprintf (buf, sizeof buf, "You failed!  You lose %d exps.\r\n", lose);
      send_to_char (buf);
      return;
    }

    lose = (desc != NULL) ? 100 : 200;
    gain_exp(0 - lose);
    snprintf (buf, sizeof buf, "You recall from combat!  You lose %d exps.\r\n", lose);
    send_to_char (buf);
    stop_fighting(true);
  }

  move /= 2;
  act ("$n disappears.", NULL, NULL, TO_ROOM);
  char_from_room();
  char_to_room(location);
  act ("$n appears in the room.", NULL, NULL, TO_ROOM);
  do_look ("auto");

  return;
}

void Character::do_train (std::string argument)
{
  if (is_npc ())
    return;

  std::string buf;
  sh_int ability;
  const char *pOutput;

  /*
   * Check for trainer.
   */
  CharIter mob;
  for (mob = in_room->people.begin(); mob != in_room->people.end(); mob++) {
    if ((*mob)->is_npc () && IS_SET ((*mob)->actflags, ACT_TRAIN))
      break;
  }

  if (mob == in_room->people.end()) {
    send_to_char ("You can't do that here.\r\n");
    return;
  }

  if (argument.empty()) {
    buf = "You have " + itoa(practice, 10) + " practice sessions.\r\n";
    send_to_char (buf);
    argument = "foo";
  }

  int cost = 5;

  if (!str_cmp (argument, "str")) {
    if (class_table[klass].attr_prime == APPLY_STR)
      cost = 3;
    ability = pcdata->get_perm(argument);
    pOutput = "strength";
  } else if (!str_cmp (argument, "int")) {
    if (class_table[klass].attr_prime == APPLY_INT)
      cost = 3;
    ability = pcdata->get_perm(argument);
    pOutput = "intelligence";
  } else if (!str_cmp (argument, "wis")) {
    if (class_table[klass].attr_prime == APPLY_WIS)
      cost = 3;
    ability = pcdata->get_perm(argument);
    pOutput = "wisdom";
  } else if (!str_cmp (argument, "dex")) {
    if (class_table[klass].attr_prime == APPLY_DEX)
      cost = 3;
    ability = pcdata->get_perm(argument);
    pOutput = "dexterity";
  } else if (!str_cmp (argument, "con")) {
    if (class_table[klass].attr_prime == APPLY_CON)
      cost = 3;
    ability = pcdata->get_perm(argument);
    pOutput = "constitution";
  } else {
    buf = "You can train:";
    buf.append(pcdata->trainable_list());

    if (buf[buf.size() - 1] != ':') {
      buf.append(".\r\n");
      send_to_char (buf);
    } else {
      /*
       * This message dedicated to Jordan ... you big stud!
       */
      act ("You have nothing left to train, you $T!",
        NULL,
        sex == SEX_MALE ? "big stud" :
        sex == SEX_FEMALE ? "hot babe" : "wild thing", TO_CHAR);
    }

    return;
  }

  if (ability >= 18) {
    act ("Your $T is already at maximum.", NULL, pOutput, TO_CHAR);
    return;
  }

  if (cost > practice) {
    send_to_char ("You don't have enough practices.\r\n");
    return;
  }

  practice -= cost;
  pcdata->set_perm(argument, ability+1);
  act ("Your $T increases!", NULL, pOutput, TO_CHAR);
  act ("$n's $T increases!", NULL, pOutput, TO_ROOM);
  return;
}

void Character::do_get (std::string argument)
{
  std::string arg1;
  std::string arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  /* Get type. */
  if (arg1.empty()) {
    send_to_char ("Get what?\r\n");
    return;
  }

  Object *obj;
  Object *container;
  bool found;
  if (arg2.empty()) {
    if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)) {
      /* 'get obj' */
      obj = get_obj_list (arg1, in_room->contents);
      if (obj == NULL) {
        act ("I see no $T here.", NULL, arg1.c_str(), TO_CHAR);
        return;
      }

      get_obj (obj, NULL);
    } else {
      /* 'get all' or 'get all.obj' */
      found = false;
      ObjIter o, onext;
      for (o = in_room->contents.begin(); o != in_room->contents.end(); o = onext) {
        obj = *o;
        onext = ++o;
        if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
          && can_see_obj(obj)) {
          found = true;
          get_obj (obj, NULL);
        }
      }

      if (!found) {
        if (arg1[3] == '\0')
          send_to_char ("I see nothing here.\r\n");
        else
          act ("I see no $T here.", NULL, &arg1[4], TO_CHAR);
      }
    }
  } else {
    /* 'get ... container' */
    if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2)) {
      send_to_char ("You can't do that.\r\n");
      return;
    }

    if ((container = get_obj_here (arg2)) == NULL) {
      act ("I see no $T here.", NULL, arg2.c_str(), TO_CHAR);
      return;
    }

    switch (container->item_type) {
    default:
      send_to_char ("That's not a container.\r\n");
      return;

    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
      break;

    case ITEM_CORPSE_PC:
      {
        std::string nm;
        std::string pd;

        if (is_npc ()) {
          send_to_char ("You can't do that.\r\n");
          return;
        }

        pd = container->short_descr;
        pd = one_argument (pd, nm);
        pd = one_argument (pd, nm);
        pd = one_argument (pd, nm);

        if (str_cmp (nm, name) && !is_immortal()) {
          bool fGroup;

          fGroup = false;
          CharIter c;
          for (c = char_list.begin(); c != char_list.end(); c++) {
            if (!(*c)->is_npc ()
              && is_same_group (this, *c)
              && !str_cmp (nm, (*c)->name)) {
              fGroup = true;
              break;
            }
          }

          if (!fGroup) {
            send_to_char ("You can't do that.\r\n");
            return;
          }
        }
      }
    }

    if (IS_SET (container->value[1], CONT_CLOSED)) {
      act ("The $d is closed.", NULL, container->name.c_str(), TO_CHAR);
      return;
    }

    if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)) {
      /* 'get obj container' */
      obj = get_obj_list (arg1, container->contains);
      if (obj == NULL) {
        act ("I see nothing like that in the $T.", NULL, arg2.c_str(), TO_CHAR);
        return;
      }
      get_obj (obj, container);
    } else {
      /* 'get all container' or 'get all.obj container' */
      found = false;
      ObjIter o, onext;
      for (o = container->contains.begin(); o != container->contains.end(); o = onext) {
        obj = *o;
        onext = ++o;
        if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
          && can_see_obj(obj)) {
          found = true;
          get_obj (obj, container);
        }
      }

      if (!found) {
        if (arg1[3] == '\0')
          act ("I see nothing in the $T.", NULL, arg2.c_str(), TO_CHAR);
        else
          act ("I see nothing like that in the $T.", NULL, arg2.c_str(), TO_CHAR);
      }
    }
  }

  return;
}

void Character::do_put (std::string argument)
{
  std::string arg1;
  std::string arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty()) {
    send_to_char ("Put what in what?\r\n");
    return;
  }

  if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2)) {
    send_to_char ("You can't do that.\r\n");
    return;
  }

  Object *container;
  if ((container = get_obj_here (arg2)) == NULL) {
    act ("I see no $T here.", NULL, arg2.c_str(), TO_CHAR);
    return;
  }

  if (container->item_type != ITEM_CONTAINER) {
    send_to_char ("That's not a container.\r\n");
    return;
  }

  if (IS_SET (container->value[1], CONT_CLOSED)) {
    act ("The $d is closed.", NULL, container->name.c_str(), TO_CHAR);
    return;
  }

  Object *obj;
  if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)) {
    /* 'put obj container' */
    if ((obj = get_obj_carry (arg1)) == NULL) {
      send_to_char ("You do not have that item.\r\n");
      return;
    }

    if (obj == container) {
      send_to_char ("You can't fold it into itself.\r\n");
      return;
    }

    if (!can_drop_obj (obj)) {
      send_to_char ("You can't let go of it.\r\n");
      return;
    }

    if (obj->get_obj_weight() + container->get_obj_weight()
      > container->value[0]) {
      send_to_char ("It won't fit.\r\n");
      return;
    }

    obj->obj_from_char();
    obj->obj_to_obj (container);
    act ("$n puts $p in $P.", obj, container, TO_ROOM);
    act ("You put $p in $P.", obj, container, TO_CHAR);
  } else {
    /* 'put all container' or 'put all.obj container' */
    ObjIter o, onext;
    for (o = carrying.begin(); o != carrying.end(); o = onext) {
      obj = *o;
      onext = ++o;

      if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
        && can_see_obj(obj)
        && obj->wear_loc == WEAR_NONE
        && obj != container && can_drop_obj (obj)
        && obj->get_obj_weight() + container->get_obj_weight()
        <= container->value[0]) {
        obj->obj_from_char ();
        obj->obj_to_obj (container);
        act ("$n puts $p in $P.", obj, container, TO_ROOM);
        act ("You put $p in $P.", obj, container, TO_CHAR);
      }
    }
  }

  return;
}

void Character::do_drop (std::string argument)
{
  std::string arg;

  argument = one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Drop what?\r\n");
    return;
  }

  Object *obj;
  bool found;
  if (is_number (arg)) {
    /* 'drop NNNN coins' */
    int amount;

    amount = std::atoi (arg.c_str());
    argument = one_argument (argument, arg);
    if (amount <= 0 || (str_cmp (arg, "coins") && str_cmp (arg, "coin"))) {
      send_to_char ("Sorry, you can't do that.\r\n");
      return;
    }

    if (gold < amount) {
      send_to_char ("You haven't got that many coins.\r\n");
      return;
    }

    gold -= amount;

    ObjIter o, onext;
    for (o = in_room->contents.begin(); o != in_room->contents.end(); o = onext) {
      obj = *o;
      onext = ++o;

      switch (obj->pIndexData->vnum) {
      case OBJ_VNUM_MONEY_ONE:
        amount += 1;
        obj->extract_obj ();
        break;

      case OBJ_VNUM_MONEY_SOME:
        amount += obj->value[0];
        obj->extract_obj ();
        break;
      }
    }

    create_money (amount)->obj_to_room (in_room);
    act ("$n drops some gold.", NULL, NULL, TO_ROOM);
    send_to_char ("OK.\r\n");
    return;
  }

  if (str_cmp (arg, "all") && str_prefix ("all.", arg)) {
    /* 'drop obj' */
    if ((obj = get_obj_carry (arg)) == NULL) {
      send_to_char ("You do not have that item.\r\n");
      return;
    }

    if (!can_drop_obj (obj)) {
      send_to_char ("You can't let go of it.\r\n");
      return;
    }

    obj->obj_from_char();
    obj->obj_to_room (in_room);
    act ("$n drops $p.", obj, NULL, TO_ROOM);
    act ("You drop $p.", obj, NULL, TO_CHAR);
  } else {
    /* 'drop all' or 'drop all.obj' */
    found = false;
    ObjIter o, onext;
    for (o = carrying.begin(); o != carrying.end(); o = onext) {
      obj = *o;
      onext = ++o;

      if ((arg[3] == '\0' || is_name (&arg[4], obj->name))
        && can_see_obj(obj)
        && obj->wear_loc == WEAR_NONE && can_drop_obj (obj)) {
        found = true;
        obj->obj_from_char();
        obj->obj_to_room(in_room);
        act ("$n drops $p.", obj, NULL, TO_ROOM);
        act ("You drop $p.", obj, NULL, TO_CHAR);
      }
    }

    if (!found) {
      if (arg[3] == '\0')
        act ("You are not carrying anything.", NULL, arg.c_str(), TO_CHAR);
      else
        act ("You are not carrying any $T.", NULL, &arg[4], TO_CHAR);
    }
  }

  return;
}

void Character::do_give (std::string argument)
{
  std::string arg1;
  std::string arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty()) {
    send_to_char ("Give what to whom?\r\n");
    return;
  }

  Character *victim;
  Object *obj;
  if (is_number (arg1)) {
    /* 'give NNNN coins victim' */
    int amount;

    amount = std::atoi (arg1.c_str());
    if (amount <= 0 || (str_cmp (arg2, "coins") && str_cmp (arg2, "coin"))) {
      send_to_char ("Sorry, you can't do that.\r\n");
      return;
    }

    argument = one_argument (argument, arg2);
    if (arg2.empty()) {
      send_to_char ("Give what to whom?\r\n");
      return;
    }

    if ((victim = get_char_room (arg2)) == NULL) {
      send_to_char ("They aren't here.\r\n");
      return;
    }

    if (gold < amount) {
      send_to_char ("You haven't got that much gold.\r\n");
      return;
    }

    gold -= amount;
    victim->gold += amount;
    act ("$n gives you some gold.", NULL, victim, TO_VICT);
    act ("$n gives $N some gold.", NULL, victim, TO_NOTVICT);
    act ("You give $N some gold.", NULL, victim, TO_CHAR);
    send_to_char ("OK.\r\n");
    mprog_bribe_trigger (victim, this, amount);
    return;
  }

  if ((obj = get_obj_carry (arg1)) == NULL) {
    send_to_char ("You do not have that item.\r\n");
    return;
  }

  if (obj->wear_loc != WEAR_NONE) {
    send_to_char ("You must remove it first.\r\n");
    return;
  }

  if ((victim = get_char_room (arg2)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!can_drop_obj (obj)) {
    send_to_char ("You can't let go of it.\r\n");
    return;
  }

  if (victim->carry_number + obj->get_obj_number() > victim->can_carry_n()) {
    act ("$N has $S hands full.", NULL, victim, TO_CHAR);
    return;
  }

  if (victim->carry_weight + obj->get_obj_weight() > victim->can_carry_w()) {
    act ("$N can't carry that much weight.", NULL, victim, TO_CHAR);
    return;
  }

  if (!victim->can_see_obj(obj)) {
    act ("$N can't see it.", NULL, victim, TO_CHAR);
    return;
  }

  obj->obj_from_char ();
  obj->obj_to_char (victim);
  MOBtrigger = false;
  act ("$n gives $p to $N.", obj, victim, TO_NOTVICT);
  act ("$n gives you $p.", obj, victim, TO_VICT);
  act ("You give $p to $N.", obj, victim, TO_CHAR);
  mprog_give_trigger (victim, this, obj);
  return;
}

void Character::do_fill (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Fill what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_carry (arg)) == NULL) {
    send_to_char ("You do not have that item.\r\n");
    return;
  }

  bool found = false;
  ObjIter o;
  for (o = in_room->contents.begin(); o != in_room->contents.end(); o++) {
    if ((*o)->item_type == ITEM_FOUNTAIN) {
      found = true;
      break;
    }
  }

  if (!found) {
    send_to_char ("There is no fountain here!\r\n");
    return;
  }

  if (obj->item_type != ITEM_DRINK_CON) {
    send_to_char ("You can't fill that.\r\n");
    return;
  }

  if (obj->value[1] != 0 && obj->value[2] != 0) {
    send_to_char ("There is already another liquid in it.\r\n");
    return;
  }

  if (obj->value[1] >= obj->value[0]) {
    send_to_char ("Your container is full.\r\n");
    return;
  }

  act ("You fill $p.", obj, NULL, TO_CHAR);
  obj->value[2] = 0;
  obj->value[1] = obj->value[0];
  return;
}

void Character::do_drink (std::string argument)
{
  std::string arg;
  Object *obj = NULL;
  int amount;
  int liquid;

  one_argument (argument, arg);

  if (arg.empty()) {
    ObjIter o;
    for (o = in_room->contents.begin(); o != in_room->contents.end(); o++) {
      obj = *o;
      if (obj->item_type == ITEM_FOUNTAIN)
        break;
    }

    if (obj == NULL) {
      send_to_char ("Drink what?\r\n");
      return;
    }
  } else {
    if ((obj = get_obj_here (arg)) == NULL) {
      send_to_char ("You can't find it.\r\n");
      return;
    }
  }

  if (!is_npc () && pcdata->condition[COND_DRUNK] > 10) {
    send_to_char ("You fail to reach your mouth.  *Hic*\r\n");
    return;
  }

  switch (obj->item_type) {
  default:
    send_to_char ("You can't drink from that.\r\n");
    break;

  case ITEM_FOUNTAIN:
    if (!is_npc ())
      pcdata->condition[COND_THIRST] = 48;
    act ("$n drinks from the fountain.", NULL, NULL, TO_ROOM);
    send_to_char ("You are not thirsty.\r\n");
    break;

  case ITEM_DRINK_CON:
    if (obj->value[1] <= 0) {
      send_to_char ("It is already empty.\r\n");
      return;
    }

    if ((liquid = obj->value[2]) >= LIQ_MAX) {
      bug_printf ("Do_drink: bad liquid number %d.", liquid);
      liquid = obj->value[2] = 0;
    }

    act ("$n drinks $T from $p.",
      obj, liq_table[liquid].liq_name, TO_ROOM);
    act ("You drink $T from $p.",
      obj, liq_table[liquid].liq_name, TO_CHAR);

    amount = number_range (3, 10);
    amount = std::min (amount, obj->value[1]);

    gain_condition (COND_DRUNK, amount * liq_table[liquid].liq_affect[COND_DRUNK]);
    gain_condition (COND_FULL, amount * liq_table[liquid].liq_affect[COND_FULL]);
    gain_condition (COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST]);

    if (!is_npc () && pcdata->condition[COND_DRUNK] > 10)
      send_to_char ("You feel drunk.\r\n");
    if (!is_npc () && pcdata->condition[COND_FULL] > 40)
      send_to_char ("You are full.\r\n");
    if (!is_npc () && pcdata->condition[COND_THIRST] > 40)
      send_to_char ("You do not feel thirsty.\r\n");

    if (obj->value[3] != 0) {
      /* The shit was poisoned ! */
      Affect af;

      act ("$n chokes and gags.", NULL, NULL, TO_ROOM);
      send_to_char ("You choke and gag.\r\n");
      af.type = skill_lookup("poison");
      af.duration = 3 * amount;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_POISON;
      affect_join (&af);
    }

    obj->value[1] -= amount;
    if (obj->value[1] <= 0) {
      send_to_char ("The empty container vanishes.\r\n");
      obj->extract_obj ();
    }
    break;
  }

  return;
}

void Character::do_eat (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Eat what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_carry (arg)) == NULL) {
    send_to_char ("You do not have that item.\r\n");
    return;
  }

  if (!is_immortal()) {
    if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL) {
      send_to_char ("That's not edible.\r\n");
      return;
    }

    if (!is_npc () && pcdata->condition[COND_FULL] > 40) {
      send_to_char ("You are too full to eat more.\r\n");
      return;
    }
  }

  act ("$n eats $p.", obj, NULL, TO_ROOM);
  act ("You eat $p.", obj, NULL, TO_CHAR);

  switch (obj->item_type) {

  case ITEM_FOOD:
    if (!is_npc ()) {
      int condition;

      condition = pcdata->condition[COND_FULL];
      gain_condition (COND_FULL, obj->value[0]);
      if (condition == 0 && pcdata->condition[COND_FULL] > 0)
        send_to_char ("You are no longer hungry.\r\n");
      else if (pcdata->condition[COND_FULL] > 40)
        send_to_char ("You are full.\r\n");
    }

    if (obj->value[3] != 0) {
      /* The shit was poisoned! */
      Affect af;

      act ("$n chokes and gags.", 0, 0, TO_ROOM);
      send_to_char ("You choke and gag.\r\n");

      af.type = skill_lookup("poison");
      af.duration = 2 * obj->value[0];
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_POISON;
      affect_join (&af);
    }
    break;

  case ITEM_PILL:
    obj_cast_spell (obj->value[1], obj->value[0], this, this, NULL);
    obj_cast_spell (obj->value[2], obj->value[0], this, this, NULL);
    obj_cast_spell (obj->value[3], obj->value[0], this, this, NULL);
    break;
  }

  obj->extract_obj ();
  return;
}

void Character::do_wear (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Wear, wield, or hold what?\r\n");
    return;
  }

  Object *obj;
  if (!str_cmp (arg, "all")) {
    ObjIter o, onext;
    for (o = carrying.begin(); o != carrying.end(); o = onext) {
      obj = *o;
      onext = ++o;
      if (obj->wear_loc == WEAR_NONE && can_see_obj(obj))
        wear_obj (obj, false);
    }
    return;
  } else {
    if ((obj = get_obj_carry (arg)) == NULL) {
      send_to_char ("You do not have that item.\r\n");
      return;
    }

    wear_obj (obj, true);
  }

  return;
}

void Character::do_remove (std::string argument)
{
  std::string arg;
  Object *obj;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Remove what?\r\n");
    return;
  }

  if ((obj = get_obj_wear (arg)) == NULL) {
    send_to_char ("You do not have that item.\r\n");
    return;
  }

  remove_obj (obj->wear_loc, true);
  return;
}

void Character::do_sacrifice (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty() || !str_cmp (arg, name)) {
    act ("$n offers $mself to God, who graciously declines.",
      NULL, NULL, TO_ROOM);
    send_to_char ("God appreciates your offer and may accept it later.");
    return;
  }

  Object* obj = get_obj_list (arg, in_room->contents);
  if (obj == NULL) {
    send_to_char ("You can't find it.\r\n");
    return;
  }

  if (!obj->can_wear(ITEM_TAKE)) {
    act ("$p is not an acceptable sacrifice.", obj, 0, TO_CHAR);
    return;
  }

  send_to_char ("God gives you one gold coin for your sacrifice.\r\n");
  gold += 1;

  act ("$n sacrifices $p to God.", obj, NULL, TO_ROOM);
  obj->extract_obj ();
  return;
}

void Character::do_quaff (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Quaff what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_carry (arg)) == NULL) {
    send_to_char ("You do not have that potion.\r\n");
    return;
  }

  if (obj->item_type != ITEM_POTION) {
    send_to_char ("You can quaff only potions.\r\n");
    return;
  }

  act ("$n quaffs $p.", obj, NULL, TO_ROOM);
  act ("You quaff $p.", obj, NULL, TO_CHAR);

  obj_cast_spell (obj->value[1], obj->value[0], this, this, NULL);
  obj_cast_spell (obj->value[2], obj->value[0], this, this, NULL);
  obj_cast_spell (obj->value[3], obj->value[0], this, this, NULL);

  obj->extract_obj ();
  return;
}

void Character::do_recite (std::string argument)
{
  std::string arg1;
  std::string arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  Object *scroll;
  if ((scroll = get_obj_carry (arg1)) == NULL) {
    send_to_char ("You do not have that scroll.\r\n");
    return;
  }

  if (scroll->item_type != ITEM_SCROLL) {
    send_to_char ("You can recite only scrolls.\r\n");
    return;
  }

  Character *victim;
  Object *obj = NULL;
  if (arg2.empty()) {
    victim = this;
  } else {
    if ((victim = get_char_room (arg2)) == NULL
      && (obj = get_obj_here (arg2)) == NULL) {
      send_to_char ("You can't find it.\r\n");
      return;
    }
  }

  act ("$n recites $p.", scroll, NULL, TO_ROOM);
  act ("You recite $p.", scroll, NULL, TO_CHAR);

  obj_cast_spell (scroll->value[1], scroll->value[0], this, victim, obj);
  obj_cast_spell (scroll->value[2], scroll->value[0], this, victim, obj);
  obj_cast_spell (scroll->value[3], scroll->value[0], this, victim, obj);

  scroll->extract_obj ();
  return;
}

void Character::do_brandish (std::string argument)
{
  Character *vch;
  Object *staff;

  if ((staff = get_eq_char (WEAR_HOLD)) == NULL) {
    send_to_char ("You hold nothing in your hand.\r\n");
    return;
  }

  if (staff->item_type != ITEM_STAFF) {
    send_to_char ("You can brandish only with a staff.\r\n");
    return;
  }

  int sn;
  if ((sn = staff->value[3]) < 0
    || sn >= MAX_SKILL || skill_table[sn].spell_fun == NULL) {
    bug_printf ("Do_brandish: bad sn %d.", sn);
    return;
  }

  wait_state (2 * PULSE_VIOLENCE);

  if (staff->value[2] > 0) {
    act ("$n brandishes $p.", staff, NULL, TO_ROOM);
    act ("You brandish $p.", staff, NULL, TO_CHAR);
    CharIter rch, next;
    for (rch = in_room->people.begin(); rch != in_room->people.end(); rch = next) {
      vch = *rch;
      next = ++rch;

      switch (skill_table[sn].target) {
      default:
        bug_printf ("Do_brandish: bad target for sn %d.", sn);
        return;

      case TAR_IGNORE:
        if (vch != this)
          continue;
        break;

      case TAR_CHAR_OFFENSIVE:
        if (is_npc () ? vch->is_npc () : !vch->is_npc ())
          continue;
        break;

      case TAR_CHAR_DEFENSIVE:
        if (is_npc () ? !vch->is_npc () : vch->is_npc ())
          continue;
        break;

      case TAR_CHAR_SELF:
        if (vch != this)
          continue;
        break;
      }

      obj_cast_spell (staff->value[3], staff->value[0], this, vch, NULL);
    }
  }

  if (--staff->value[2] <= 0) {
    act ("$n's $p blazes bright and is gone.", staff, NULL, TO_ROOM);
    act ("Your $p blazes bright and is gone.", staff, NULL, TO_CHAR);
    staff->extract_obj ();
  }

  return;
}

void Character::do_zap (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty() && fighting == NULL) {
    send_to_char ("Zap whom or what?\r\n");
    return;
  }

  Object *wand;
  if ((wand = get_eq_char (WEAR_HOLD)) == NULL) {
    send_to_char ("You hold nothing in your hand.\r\n");
    return;
  }

  if (wand->item_type != ITEM_WAND) {
    send_to_char ("You can zap only with a wand.\r\n");
    return;
  }

  Character *victim;
  Object *obj = NULL;
  if (arg.empty()) {
    if (fighting != NULL) {
      victim = fighting;
    } else {
      send_to_char ("Zap whom or what?\r\n");
      return;
    }
  } else {
    if ((victim = get_char_room (arg)) == NULL
      && (obj = get_obj_here (arg)) == NULL) {
      send_to_char ("You can't find it.\r\n");
      return;
    }
  }

  wait_state (2 * PULSE_VIOLENCE);

  if (wand->value[2] > 0) {
    if (victim != NULL) {
      act ("$n zaps $N with $p.", wand, victim, TO_ROOM);
      act ("You zap $N with $p.", wand, victim, TO_CHAR);
    } else {
      act ("$n zaps $P with $p.", wand, obj, TO_ROOM);
      act ("You zap $P with $p.", wand, obj, TO_CHAR);
    }

    obj_cast_spell (wand->value[3], wand->value[0], this, victim, obj);
  }

  if (--wand->value[2] <= 0) {
    act ("$n's $p explodes into fragments.", wand, NULL, TO_ROOM);
    act ("Your $p explodes into fragments.", wand, NULL, TO_CHAR);
    wand->extract_obj ();
  }

  return;
}

void Character::do_steal (std::string argument)
{
  std::string arg1, arg2, buf;
  Character *victim;
  Object *obj;
  int percent;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty()) {
    send_to_char ("Steal what from whom?\r\n");
    return;
  }

  if ((victim = get_char_room (arg2)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim == this) {
    send_to_char ("That's pointless.\r\n");
    return;
  }

  wait_state (skill_table[skill_lookup("steal")].beats);
  percent = number_percent () + (victim->is_awake () ? 10 : -50);

  if (level + 5 < victim->level
    || victim->position == POS_FIGHTING || !victim->is_npc ()
    || (!is_npc () && percent > pcdata->learned[skill_lookup("steal")])) {
    /*
     * Failure.
     */
    send_to_char ("Oops.\r\n");
    act ("$n tried to steal from you.\r\n", NULL, victim, TO_VICT);
    act ("$n tried to steal from $N.\r\n", NULL, victim, TO_NOTVICT);
    buf = name + " is a bloody thief!";
    victim->do_shout (buf);
    if (!is_npc ()) {
      if (victim->is_npc ()) {
        multi_hit (victim, this, TYPE_UNDEFINED);
      } else {
        log_printf (buf.c_str());
        if (!IS_SET (actflags, PLR_THIEF)) {
          SET_BIT (actflags, PLR_THIEF);
          send_to_char ("*** You are now a THIEF!! ***\r\n");
          save_char_obj();
        }
      }
    }

    return;
  }

  if (!str_cmp (arg1, "coin")
    || !str_cmp (arg1, "coins")
    || !str_cmp (arg1, "gold")) {
    int amount;

    amount = victim->gold * number_range (1, 10) / 100;
    if (amount <= 0) {
      send_to_char ("You couldn't get any gold.\r\n");
      return;
    }

    gold += amount;
    victim->gold -= amount;
    buf = "Bingo!  You got " + itoa(amount, 10) + " gold coins.\r\n";
    send_to_char (buf);
    return;
  }

  if ((obj = victim->get_obj_carry (arg1)) == NULL) {
    send_to_char ("You can't find it.\r\n");
    return;
  }

  if (!can_drop_obj (obj)
    || IS_SET (obj->extra_flags, ITEM_INVENTORY)
    || obj->level > level) {
    send_to_char ("You can't pry it away.\r\n");
    return;
  }

  if (carry_number + obj->get_obj_number() > can_carry_n()) {
    send_to_char ("You have your hands full.\r\n");
    return;
  }

  if (carry_weight + obj->get_obj_weight() > can_carry_w()) {
    send_to_char ("You can't carry that much weight.\r\n");
    return;
  }

  obj->obj_from_char ();
  obj->obj_to_char (this);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_buy (std::string argument)
{
  std::string arg;

  argument = one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Buy what?\r\n");
    return;
  }

  if (IS_SET (in_room->room_flags, ROOM_PET_SHOP)) {
    std::string buf;
    Character *pet;
    Room *pRoomIndexNext;
    Room *in_rm;

    if (is_npc ())
      return;

    pRoomIndexNext = get_room_index (in_room->vnum + 1);
    if (pRoomIndexNext == NULL) {
      bug_printf ("Do_buy: bad pet shop at vnum %d.", in_room->vnum);
      send_to_char ("Sorry, you can't buy that here.\r\n");
      return;
    }

    in_rm = in_room;
    in_room = pRoomIndexNext;
    pet = get_char_room (arg);
    in_room = in_rm;

    if (pet == NULL || !IS_SET (pet->actflags, ACT_PET)) {
      send_to_char ("Sorry, you can't buy that here.\r\n");
      return;
    }

    if (IS_SET (actflags, PLR_BOUGHT_PET)) {
      send_to_char ("You already bought one pet this level.\r\n");
      return;
    }

    if (gold < 10 * pet->level * pet->level) {
      send_to_char ("You can't afford it.\r\n");
      return;
    }

    if (level < pet->level) {
      send_to_char ("You're not ready for this pet.\r\n");
      return;
    }

    gold -= 10 * pet->level * pet->level;
    pet = pet->pIndexData->create_mobile();
    SET_BIT (actflags, PLR_BOUGHT_PET);
    SET_BIT (pet->actflags, ACT_PET);
    SET_BIT (pet->affected_by, AFF_CHARM);

    argument = one_argument (argument, arg);
    if (!arg.empty()) {
      buf = pet->name + " " + arg;
      pet->name = buf;
    }

    buf = pet->description + "A neck tag says 'I belong to " + name + "'.\r\n";
    pet->description = buf;

    pet->char_to_room(in_room);
    pet->add_follower(this);
    send_to_char ("Enjoy your pet.\r\n");
    act ("$n bought $N as a pet.", NULL, pet, TO_ROOM);
    return;
  } else {
    Character *keeper;
    Object *obj;
    int cost;

    if ((keeper = find_keeper (this)) == NULL)
      return;

    obj = keeper->get_obj_carry (arg);
    cost = get_cost (keeper, obj, true);

    if (cost <= 0 || !can_see_obj(obj)) {
      keeper->act ("$n tells you 'I don't sell that -- try 'list''.",
        NULL, this, TO_VICT);
      reply = keeper;
      return;
    }

    if (gold < cost) {
      keeper->act ("$n tells you 'You can't afford to buy $p'.",
        obj, this, TO_VICT);
      reply = keeper;
      return;
    }

    if (obj->level > level) {
      keeper->act ("$n tells you 'You can't use $p yet'.", obj, this, TO_VICT);
      reply = keeper;
      return;
    }

    if (carry_number + obj->get_obj_number() > can_carry_n()) {
      send_to_char ("You can't carry that many items.\r\n");
      return;
    }

    if (carry_weight + obj->get_obj_weight() > can_carry_w()) {
      send_to_char ("You can't carry that much weight.\r\n");
      return;
    }

    act ("$n buys $p.", obj, NULL, TO_ROOM);
    act ("You buy $p.", obj, NULL, TO_CHAR);
    gold -= cost;
    keeper->gold += cost;

    if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
      obj = obj->pIndexData->create_object(obj->level);
    else
      obj->obj_from_char ();

    obj->obj_to_char (this);
    return;
  }
}

void Character::do_list (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string buf1;

  if (IS_SET (in_room->room_flags, ROOM_PET_SHOP)) {
    Room *pRoomIndexNext;
    bool found;

    pRoomIndexNext = get_room_index (in_room->vnum + 1);
    if (pRoomIndexNext == NULL) {
      bug_printf ("Do_list: bad pet shop at vnum %d.", in_room->vnum);
      send_to_char ("You can't do that here.\r\n");
      return;
    }

    found = false;
    CharIter pet;
    for (pet = pRoomIndexNext->people.begin(); pet != pRoomIndexNext->people.end(); pet++) {
      if (IS_SET ((*pet)->actflags, ACT_PET)) {
        if (!found) {
          found = true;
          buf1.append("Pets for sale:\r\n");
        }
        snprintf (buf, sizeof buf, "[%2d] %8d - %s\r\n",
          (*pet)->level, 10 * (*pet)->level * (*pet)->level, (*pet)->short_descr.c_str());
        buf1.append(buf);
      }
    }
    if (!found)
      send_to_char ("Sorry, we're out of pets right now.\r\n");

    send_to_char (buf1);
    return;
  } else {
    std::string arg;
    Character *keeper;
    Object *obj;
    int cost = 0;
    bool found;

    one_argument (argument, arg);

    if ((keeper = find_keeper (this)) == NULL)
      return;

    found = false;
    ObjIter o;
    for (o = keeper->carrying.begin(); o != keeper->carrying.end(); o++) {
      obj = *o;
      if (obj->wear_loc == WEAR_NONE && can_see_obj(obj)
        && (cost = get_cost (keeper, obj, true)) > 0
        && (arg.empty() || is_name (arg, obj->name))) {
        if (!found) {
          found = true;
          buf1.append("[Lv Price] Item\r\n");
        }

        snprintf (buf, sizeof buf, "[%2d %5d] %s.\r\n",
          obj->level, cost, capitalize (obj->short_descr).c_str());
        buf1.append(buf);
      }
    }

    if (!found) {
      if (arg.empty())
        send_to_char ("You can't buy anything here.\r\n");
      else
        send_to_char ("You can't buy that here.\r\n");
      return;
    }

    send_to_char (buf1);
    return;
  }
}

void Character::do_sell (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Sell what?\r\n");
    return;
  }

  Character *keeper;
  if ((keeper = find_keeper (this)) == NULL)
    return;

  Object *obj;
  if ((obj = get_obj_carry (arg)) == NULL) {
    keeper->act ("$n tells you 'You don't have that item'.",
      NULL, this, TO_VICT);
    reply = keeper;
    return;
  }

  if (!can_drop_obj (obj)) {
    send_to_char ("You can't let go of it.\r\n");
    return;
  }

  int cost;
  if ((cost = get_cost (keeper, obj, false)) <= 0) {
    keeper->act ("$n looks uninterested in $p.", obj, this, TO_VICT);
    return;
  }

  char buf[MAX_STRING_LENGTH];
  act ("$n sells $p.", obj, NULL, TO_ROOM);
  snprintf (buf, sizeof buf, "You sell $p for %d gold piece%s.",
    cost, cost == 1 ? "" : "s");
  act (buf, obj, NULL, TO_CHAR);
  gold += cost;
  keeper->gold -= cost;
  if (keeper->gold < 0)
    keeper->gold = 0;

  if (obj->item_type == ITEM_TRASH) {
    obj->extract_obj ();
  } else {
    obj->obj_from_char ();
    obj->obj_to_char (keeper);
  }

  return;
}

void Character::do_value (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Value what?\r\n");
    return;
  }

  Character *keeper;
  if ((keeper = find_keeper (this)) == NULL)
    return;

  Object *obj;
  if ((obj = get_obj_carry (arg)) == NULL) {
    keeper->act ("$n tells you 'You don't have that item'.",
      NULL, this, TO_VICT);
    reply = keeper;
    return;
  }

  if (!can_drop_obj (obj)) {
    send_to_char ("You can't let go of it.\r\n");
    return;
  }

  int cost;
  if ((cost = get_cost (keeper, obj, false)) <= 0) {
    keeper->act ("$n looks uninterested in $p.", obj, this, TO_VICT);
    return;
  }

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost);
  keeper->act (buf, obj, this, TO_VICT);
  reply = keeper;

  return;
}

void Character::do_wizhelp (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string buf1;

  int col = 0;
  for (int cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
    if (cmd_table[cmd].level >= LEVEL_HERO
      && cmd_table[cmd].level <= get_trust ()) {
      snprintf (buf, sizeof buf, "%-12s", cmd_table[cmd].name);
      buf1.append(buf);
      if (++col % 6 == 0)
        buf1.append("\r\n");
    }
  }

  if (col % 6 != 0)
    buf1.append("\r\n");
  send_to_char (buf1);
  return;
}

void Character::do_bamfin (std::string argument)
{
  if (!is_npc ()) {
    smash_tilde (argument);
    pcdata->bamfin = argument;
    send_to_char ("Ok.\r\n");
  }
  return;
}

void Character::do_bamfout (std::string argument)
{
  if (!is_npc ()) {
    smash_tilde (argument);
    pcdata->bamfout = argument;
    send_to_char ("Ok.\r\n");
  }
  return;
}

void Character::do_deny (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Deny whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (victim->get_trust () >= get_trust ()) {
    send_to_char ("You failed.\r\n");
    return;
  }

  SET_BIT (victim->actflags, PLR_DENY);
  victim->send_to_char ("You are denied access!\r\n");
  send_to_char ("OK.\r\n");
  victim->do_quit ("");

  return;
}

void Character::do_disconnect (std::string argument)
{
  // :WARNING: There is a bug in this routine!  The mud will crash if you
  // disconnect the descriptor that immediately follows yours in
  // descriptor_list.  close_socket() invalidates the iterator in
  // 'process input' in game_loop.
  // FIXED by adding deepdenext iterator

  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Disconnect whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->desc == NULL) {
    act ("$N doesn't have a descriptor.", NULL, victim, TO_CHAR);
    return;
  }

  DescIter d = std::find(descriptor_list.begin(),descriptor_list.end(),victim->desc);
  if (d != descriptor_list.end()) {
    (*d)->close_socket();
    send_to_char ("Ok.\r\n");
    return;
  }

  bug_printf ("Do_disconnect: desc not found.");
  send_to_char ("Descriptor not found!\r\n");
  return;
}

void Character::do_pardon (std::string argument)
{
  std::string arg1, arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty()) {
    send_to_char ("Syntax: pardon <character> <killer|thief>.\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_world (arg1)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (!str_cmp (arg2, "killer")) {
    if (IS_SET (victim->actflags, PLR_KILLER)) {
      REMOVE_BIT (victim->actflags, PLR_KILLER);
      send_to_char ("Killer flag removed.\r\n");
      victim->send_to_char ("You are no longer a KILLER.\r\n");
    }
    return;
  }

  if (!str_cmp (arg2, "thief")) {
    if (IS_SET (victim->actflags, PLR_THIEF)) {
      REMOVE_BIT (victim->actflags, PLR_THIEF);
      send_to_char ("Thief flag removed.\r\n");
      victim->send_to_char ("You are no longer a THIEF.\r\n");
    }
    return;
  }

  send_to_char ("Syntax: pardon <character> <killer|thief>.\r\n");
  return;
}

void Character::do_echo (std::string argument)
{
  if (argument.empty()) {
    send_to_char ("Echo what?\r\n");
    return;
  }

  for (DescIter d = descriptor_list.begin();
    d != descriptor_list.end(); d++) {
    if ((*d)->connected == CON_PLAYING) {
      (*d)->character->send_to_char (argument + "\r\n");
    }
  }

  return;
}

void Character::do_recho (std::string argument)
{
  if (argument.empty()) {
    send_to_char ("Recho what?\r\n");
    return;
  }

  for (DescIter d = descriptor_list.begin();
    d != descriptor_list.end(); d++) {
    if ((*d)->connected == CON_PLAYING && (*d)->character->in_room == in_room) {
      (*d)->character->send_to_char (argument + "\r\n");
    }
  }

  return;
}

void Character::do_transfer (std::string argument)
{
  std::string arg1, arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty()) {
    send_to_char ("Transfer whom (and where)?\r\n");
    return;
  }

  if (!str_cmp (arg1, "all")) {
    for (DescIter d = descriptor_list.begin();
      d != descriptor_list.end(); d++) {
      if ((*d)->connected == CON_PLAYING
        && (*d)->character != this
        && (*d)->character->in_room != NULL && can_see((*d)->character)) {
        char buf[MAX_STRING_LENGTH];
        snprintf (buf, sizeof buf, "%s %s", (*d)->character->name.c_str(), arg2.c_str());
        do_transfer (buf);
      }
    }
    return;
  }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  Room *location;
  if (arg2.empty()) {
    location = in_room;
  } else {
    if ((location = find_location (this, arg2)) == NULL) {
      send_to_char ("No such location.\r\n");
      return;
    }

    if (location->is_private()) {
      send_to_char ("That room is private right now.\r\n");
      return;
    }
  }

  Character *victim;
  if ((victim = get_char_world (arg1)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->in_room == NULL) {
    send_to_char ("They are in limbo.\r\n");
    return;
  }

  if (victim->fighting != NULL)
    victim->stop_fighting(true);
  victim->act ("$n disappears in a mushroom cloud.", NULL, NULL, TO_ROOM);
  victim->char_from_room();
  victim->char_to_room(location);
  victim->act ("$n arrives from a puff of smoke.", NULL, NULL, TO_ROOM);
  if (this != victim)
    act ("$n has transferred you.", NULL, victim, TO_VICT);
  victim->do_look ("auto");
  send_to_char ("Ok.\r\n");
}

void Character::do_at (std::string argument)
{
  std::string arg;

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    send_to_char ("At where what?\r\n");
    return;
  }

  Room *location;
  if ((location = find_location (this, arg)) == NULL) {
    send_to_char ("No such location.\r\n");
    return;
  }

  if (location->is_private()) {
    send_to_char ("That room is private right now.\r\n");
    return;
  }

  Room* original = in_room;
  char_from_room();
  char_to_room(location);
  interpret (argument);

  /*
   * See if 'this' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c++) {
    if (*c == this) {
      char_from_room();
      char_to_room(original);
      break;
    }
  }

  return;
}

void Character::do_goto (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Goto where?\r\n");
    return;
  }

  Room *location;
  if ((location = find_location (this, arg)) == NULL) {
    send_to_char ("No such location.\r\n");
    return;
  }

  if (location->is_private()) {
    send_to_char ("That room is private right now.\r\n");
    return;
  }

  if (fighting != NULL)
    stop_fighting (true);
  act ("$n $T.", NULL,
    (pcdata != NULL && !pcdata->bamfout.empty())
    ? pcdata->bamfout.c_str() : "leaves in a swirling mist", TO_ROOM);

  char_from_room();
  char_to_room(location);

  act ("$n $T.", NULL,
    (pcdata != NULL && !pcdata->bamfin.empty())
    ? pcdata->bamfin.c_str() : "appears in a swirling mist", TO_ROOM);

  do_look ("auto");
  return;
}

void Character::do_rstat (std::string argument)
{
  std::string arg, buf1;

  one_argument (argument, arg);

  Room* location = arg.empty() ? in_room : find_location (this, arg);
  if (location == NULL) {
    send_to_char ("No such location.\r\n");
    return;
  }

  if (in_room != location && location->is_private()) {
    send_to_char ("That room is private right now.\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "Name: '%s.'\r\nArea: '%s'.\r\n",
    location->name.c_str(), location->area->name.c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf,
    "Vnum: %d.  Sector: %d.  Light: %d.\r\n",
    location->vnum, location->sector_type, location->light);
  buf1.append(buf);

  snprintf (buf, sizeof buf,
    "Room flags: %d.\r\nDescription:\r\n%s",
    location->room_flags, location->description.c_str());
  buf1.append(buf);

  if (!location->extra_descr.empty()) {
    buf1.append("Extra description keywords: '");
    std::list<ExtraDescription *>::iterator ed;
    for (ed = location->extra_descr.begin(); ed != location->extra_descr.end(); ed++) {
      buf1.append((*ed)->keyword);
      buf1.append(" ");
    }
    if (buf1[buf1.size() - 1] == ' ')
      buf1.erase(buf1.size() - 1);
    buf1.append("'.\r\n");
  }

  buf1.append("Characters:");
  std::string tmp;
  CharIter rch;
  for (rch = location->people.begin(); rch != location->people.end(); rch++) {
    buf1.append(" ");
    one_argument ((*rch)->name, tmp);
    buf1.append(buf);
  }

  buf1.append(".\r\nObjects:   ");
  ObjIter o;
  for (o = location->contents.begin(); o != location->contents.end(); o++) {
    buf1.append(" ");
    one_argument ((*o)->name, tmp);
    buf1.append(buf);
  }
  buf1.append(".\r\n");

  for (int door = 0; door <= 5; door++) {
    Exit *pexit;

    if ((pexit = location->exit[door]) != NULL) {
      snprintf (buf, sizeof buf,
        "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\r\nKeyword: '%s'.  Description: %s",
        door,
        pexit->to_room != NULL ? pexit->to_room->vnum : 0,
        pexit->key,
        pexit->exit_info,
        pexit->name.c_str(),
        !pexit->description.empty() ? pexit->description.c_str() : "(none).\r\n");
      buf1.append(buf);
    }
  }

  send_to_char (buf1);
  return;
}

void Character::do_ostat (std::string argument)
{
  std::string arg, buf1;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Ostat what?\r\n");
    return;
  }

  Object *obj;
  if ((obj = get_obj_world (arg)) == NULL) {
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "Name: %s.\r\n", obj->name.c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Vnum: %d.  Type: %s.\r\n",
    obj->pIndexData->vnum, obj->item_type_name().c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Short description: %s.\r\nLong description: %s\r\n",
    obj->short_descr.c_str(), obj->description.c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Wear bits: %d.  Extra bits: %s.\r\n",
    obj->wear_flags, extra_bit_name (obj->extra_flags).c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Number: %d/%d.  Weight: %d/%d.\r\n",
    1, obj->get_obj_number(), obj->weight, obj->get_obj_weight());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Cost: %d.  Timer: %d.  Level: %d.\r\n",
    obj->cost, obj->timer, obj->level);
  buf1.append(buf);

  snprintf (buf, sizeof buf,
    "In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\r\n",
    obj->in_room == NULL ? 0 : obj->in_room->vnum,
    obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr.c_str(),
    obj->carried_by == NULL ? "(none)" : obj->carried_by->name.c_str(),
    obj->wear_loc);
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Values: %d %d %d %d.\r\n",
    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
  buf1.append(buf);

  if (!obj->extra_descr.empty() || !obj->pIndexData->extra_descr.empty()) {
    buf1.append("Extra description keywords: '");
    std::list<ExtraDescription *>::iterator ed;
    for (ed = obj->extra_descr.begin(); ed != obj->extra_descr.end(); ed++) {
      buf1.append((*ed)->keyword);
      buf1.append(" ");
    }
    for (ed = obj->pIndexData->extra_descr.begin(); ed != obj->pIndexData->extra_descr.end(); ed++) {
      buf1.append((*ed)->keyword);
      buf1.append(" ");
    }
    if (buf1[buf1.size() - 1] == ' ')
      buf1.erase(buf1.size() - 1);

    buf1.append("'.\r\n");
  }

  AffIter af;
  for (af = obj->affected.begin(); af != obj->affected.end(); af++) {
    snprintf (buf, sizeof buf, "Affects %s by %d.\r\n",
      affect_loc_name ((*af)->location).c_str(), (*af)->modifier);
    buf1.append(buf);
  }

  for (af = obj->pIndexData->affected.begin(); af != obj->pIndexData->affected.end(); af++) {
    snprintf (buf, sizeof buf, "Affects %s by %d.\r\n",
      affect_loc_name ((*af)->location).c_str(), (*af)->modifier);
    buf1.append(buf);
  }

  send_to_char (buf1);
  return;
}

void Character::do_mstat (std::string argument)
{
  std::string arg, buf1;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Mstat whom?\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "Name: %s.\r\n", victim->name.c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Vnum: %d.  Sex: %s.  Room: %d.\r\n",
    victim->is_npc () ? victim->pIndexData->vnum : 0,
    victim->sex == SEX_MALE ? "male" :
    victim->sex == SEX_FEMALE ? "female" : "neutral",
    victim->in_room == NULL ? 0 : victim->in_room->vnum);
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\r\n",
    victim->get_curr_str(), victim->get_curr_int(),
    victim->get_curr_wis(), victim->get_curr_dex(),
    victim->get_curr_con());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Practices: %d.\r\n",
    victim->hit, victim->max_hit,
    victim->mana, victim->max_mana,
    victim->move, victim->max_move, victim->practice);
  buf1.append(buf);

  snprintf (buf, sizeof buf,
    "Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\r\n",
    victim->level, victim->klass, victim->alignment,
    victim->get_ac(), victim->gold, victim->exp);
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Hitroll: %d.  Damroll: %d.  Position: %d.  Wimpy: %d.\r\n",
    victim->get_hitroll(), victim->get_damroll(),
    victim->position, victim->wimpy);
  buf1.append(buf);

  if (!victim->is_npc ()) {
    snprintf (buf, sizeof buf, "Page Lines: %d.\r\n", victim->pcdata->pagelen);
    buf1.append(buf);
  }

  snprintf (buf, sizeof buf, "Fighting: %s.\r\n",
    victim->fighting ? victim->fighting->name.c_str() : "(none)");
  buf1.append(buf);

  if (!victim->is_npc ()) {
    snprintf (buf, sizeof buf,
      "Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d.\r\n",
      victim->pcdata->condition[COND_THIRST],
      victim->pcdata->condition[COND_FULL],
      victim->pcdata->condition[COND_DRUNK], victim->saving_throw);
    buf1.append(buf);
  }

  snprintf (buf, sizeof buf, "Carry number: %d.  Carry weight: %d.\r\n",
    victim->carry_number, victim->carry_weight);
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Age: %d.  Played: %d.  Timer: %d.  Act: %d.\r\n",
    victim->get_age(), (int) victim->played, victim->timer, victim->actflags);
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Master: %s.  Leader: %s.  Affected by: %s.\r\n",
    victim->master ? victim->master->name.c_str() : "(none)",
    victim->leader ? victim->leader->name.c_str() : "(none)",
    affect_bit_name (victim->affected_by).c_str());
  buf1.append(buf);

  snprintf (buf, sizeof buf, "Short description: %s.\r\nLong  description: %s",
    victim->short_descr.c_str(),
    !victim->long_descr.empty() ? victim->long_descr.c_str() : "(none).\r\n");
  buf1.append(buf);

  if (victim->is_npc () && victim->spec_fun != 0)
    buf1.append("Mobile has spec fun.\r\n");

  AffIter af;
  for (af = victim->affected.begin(); af != victim->affected.end(); af++) {
    snprintf (buf, sizeof buf,
      "Spell: '%s' modifies %s by %d for %d hours with bits %s.\r\n",
      skill_table[(int) (*af)->type].name,
      affect_loc_name ((*af)->location).c_str(),
      (*af)->modifier, (*af)->duration, affect_bit_name ((*af)->bitvector).c_str()
      );
    buf1.append(buf);
  }

  send_to_char (buf1);
  return;
}

void Character::do_mfind (std::string argument)
{
  std::string arg, buf1;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Mfind whom?\r\n");
    return;
  }

  bool fAll = !str_cmp (arg, "all");
  bool found = false;
  int nMatch = 0;
  MobPrototype *pMobIndex;
  char buf[MAX_STRING_LENGTH];

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (int vn = 0; nMatch < MobPrototype::top_mob; vn++) {
    if ((pMobIndex = get_mob_index (vn)) != NULL) {
      nMatch++;
      if (fAll || is_name (arg, pMobIndex->name)) {
        found = true;
        snprintf (buf, sizeof buf, "[%5d] %s\r\n",
          pMobIndex->vnum, capitalize (pMobIndex->short_descr).c_str());
        buf1.append(buf);
      }
    }
  }

  if (!found) {
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");
    return;
  }

  send_to_char (buf1);
  return;
}

void Character::do_ofind (std::string argument)
{
  std::string arg, buf1;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Ofind what?\r\n");
    return;
  }

  bool fAll = !str_cmp (arg, "all");
  bool found = false;
  int nMatch = 0;
  char buf[MAX_STRING_LENGTH];
  ObjectPrototype *pObjIndex;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_obj_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (int vn = 0; nMatch < ObjectPrototype::top_obj; vn++) {
    if ((pObjIndex = get_obj_index (vn)) != NULL) {
      nMatch++;
      if (fAll || is_name (arg, pObjIndex->name)) {
        found = true;
        snprintf (buf, sizeof buf, "[%5d] %s\r\n",
          pObjIndex->vnum, capitalize (pObjIndex->short_descr).c_str());
        buf1.append(buf);
      }
    }
  }

  if (!found) {
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");
    return;
  }

  send_to_char (buf1);
  return;
}

void Character::do_mwhere (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Mwhere whom?\r\n");
    return;
  }

  char buf[MAX_STRING_LENGTH];
  bool found = false;
  for (CharIter c = char_list.begin(); c != char_list.end(); c++) {
    if ((*c)->is_npc ()
      && (*c)->in_room != NULL && is_name (arg, (*c)->name)) {
      found = true;
      snprintf (buf, sizeof buf, "[%5d] %-28s [%5d] %s\r\n",
        (*c)->pIndexData->vnum,
        (*c)->short_descr.c_str(), (*c)->in_room->vnum, (*c)->in_room->name.c_str());
      send_to_char (buf);
    }
  }

  if (!found) {
    act ("You didn't find any $T.", NULL, arg.c_str(), TO_CHAR);
    return;
  }

  return;
}

void Character::do_hotboo (std::string argument)
{
  send_to_char ("If you want to HOTBOOT, spell it out.\r\n");
  return;
}

void Character::do_hotboot (std::string argument)
{
#if !defined(WIN32) || defined(__DMC__)
  send_to_char ("Hotboot not supported.\r\n");
#else
  extern bool write_to_descriptor (SOCKET desc, const char *txt, int length);

  std::string usr_msg("Hotboot by ");
  usr_msg.append(name);
  usr_msg.append(". Stand by...\r\n");

  int count_users = 0;
  for (DescIter d = descriptor_list.begin(); d != descriptor_list.end(); d++) {
    if (!(*d)->character || (*d)->connected > CON_PLAYING) {
      write_to_descriptor ((*d)->descriptor, "The server is hotbooting.  Please recreate in a few minutes.\r\n", 0);
      (*d)->close_socket();
    } else {
      count_users++;
    }
  }

  char cmd[MAX_INPUT_LENGTH];
  snprintf(cmd, sizeof cmd, "murk %d hotboot", g_port);

  STARTUPINFO start_info;
  PROCESS_INFORMATION proc_info;
  WSAPROTOCOL_INFO proto_info;

  // Get current console parameters
  GetStartupInfo(&start_info);

  // This event will be set when we are ready for copyover
  void * file_event = CreateEvent(NULL, TRUE, FALSE, "file_created");
  if (file_event == NULL) {
    win_errprint("Error creating event, file_created");
    return;
  }

  // We will wait on this event before shutting down
  void * shutdown_event = CreateEvent(NULL, TRUE, FALSE, "ok_to_shutdown");
  if (shutdown_event == NULL) {
    win_errprint("Error creating event, ok_to_shutdown");
    CloseHandle(file_event);
    return;
  }

  // Start running a new server
  if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, NULL, NULL, NULL,
      &start_info, &proc_info) == NULL) {
    win_errprint("Error creating new process");
    CloseHandle(file_event);
    CloseHandle(shutdown_event);
    return;
  }

  std::FILE* fp;
  if ((fp = std::fopen ("hotboot.$$$", "w+b")) == NULL) {
    send_to_char ("Hotboot aborted.\r\n");
    win_errprint("Error creating hotboot file");
    return;
  }

  fwrite(&count_users, sizeof(int),1,fp);  // write number of users
  // duplicate and save the listening sockets info
  if (WSADuplicateSocket(g_listen, proc_info.dwProcessId, &proto_info) == SOCKET_ERROR) {
    bug_printf ("Error duplicating listening socket : %d.", WSAGetLastError());
  }
  fwrite(&proto_info,sizeof(WSAPROTOCOL_INFO),1,fp);
  closesocket(g_listen);

  for (DescIter d = descriptor_list.begin(); d != descriptor_list.end(); d++) {
    if (WSADuplicateSocket((*d)->descriptor, proc_info.dwProcessId, &proto_info) == SOCKET_ERROR) {
      bug_printf ("Error duplicating user socket : %d.", WSAGetLastError());
    }
    Character * ch = (*d)->original ? (*d)->original : (*d)->character;
    if (ch->level == 1) {
      ch->level += 1;
      ch->advance_level();
    }
    ch->save_char_obj();
    write_to_descriptor ((*d)->descriptor, usr_msg.c_str(), 0);
    char chname[25];
    strcpy(chname,ch->name.c_str());
    fwrite(&proto_info,sizeof(WSAPROTOCOL_INFO),1,fp);
    fwrite(chname,sizeof(chname),1,fp);
    closesocket((*d)->descriptor);
  }

  fclose (fp);

  // Indicate we are done with the copyover file
  if (SetEvent(file_event) == NULL) {
    win_errprint("Error setting file_event");
  }

  // Wait for child server to tell us its ok to shutdown
  if (WaitForSingleObject(shutdown_event, INFINITE) == WAIT_FAILED) {
    win_errprint("Error waiting on shutdown_event");
  }

  // cleanup event handles
  CloseHandle(file_event);
  CloseHandle(shutdown_event);
  WIN32CLEANUP
  g_db->shutdown();
  std::exit(0);
#endif
  return;
}

void Character::do_shutdow (std::string argument)
{
  send_to_char ("If you want to SHUTDOWN, spell it out.\r\n");
  return;
}

void Character::do_shutdown (std::string argument)
{
  std::string buf("Shutdown by ");

  buf.append(name);
  buf.append(".");
  append_file (SHUTDOWN_FILE, buf);
  buf.append("\r\n");
  do_echo (buf);
  merc_down = true;
  return;
}

void Character::do_switch (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Switch into whom?\r\n");
    return;
  }

  if (desc == NULL)
    return;

  if (desc->original != NULL) {
    send_to_char ("You are already switched.\r\n");
    return;
  }

  Character *victim;
  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim == this) {
    send_to_char ("Ok.\r\n");
    return;
  }

  /*
   * Pointed out by Da Pub (What Mud)
   */
  if (!victim->is_npc ()) {
    send_to_char ("You cannot switch into a player!\r\n");
    return;
  }

  if (victim->desc != NULL) {
    send_to_char ("Character in use.\r\n");
    return;
  }

  desc->character = victim;
  desc->original = this;
  victim->desc = desc;
  desc = NULL;
  victim->send_to_char ("Ok.\r\n");
  return;
}

void Character::do_return (std::string argument)
{
  if (desc == NULL)
    return;

  if (desc->original == NULL) {
    send_to_char ("You aren't switched.\r\n");
    return;
  }

  send_to_char ("You return to your original body.\r\n");
  desc->character = desc->original;
  desc->original = NULL;
  desc->character->desc = desc;
  desc = NULL;
  return;
}

void Character::do_mload (std::string argument)
{
  std::string arg;
  MobPrototype *pMobIndex;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty() || !is_number (arg)) {
    send_to_char ("Syntax: mload <vnum>.\r\n");
    return;
  }

  if ((pMobIndex = get_mob_index (std::atoi (arg.c_str()))) == NULL) {
    send_to_char ("No mob has that vnum.\r\n");
    return;
  }

  victim = pMobIndex->create_mobile ();
  victim->char_to_room(in_room);
  act ("$n has created $N!", NULL, victim, TO_ROOM);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_oload (std::string argument)
{
  std::string arg1, arg2;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || !is_number (arg1)) {
    send_to_char ("Syntax: oload <vnum> <level>.\r\n");
    return;
  }

  int lvl;
  if (arg2.empty()) {
    lvl = get_trust ();
  } else {
    /*
     * New feature from Alander.
     */
    if (!is_number (arg2)) {
      send_to_char ("Syntax: oload <vnum> <level>.\r\n");
      return;
    }
    lvl = std::atoi (arg2.c_str());
    if (lvl < 0 || lvl > get_trust ()) {
      send_to_char ("Limited to your trust level.\r\n");
      return;
    }
  }

  ObjectPrototype *pObjIndex;
  if ((pObjIndex = get_obj_index (std::atoi (arg1.c_str()))) == NULL) {
    send_to_char ("No object has that vnum.\r\n");
    return;
  }

  Object *obj = pObjIndex->create_object(lvl);
  if (obj->can_wear(ITEM_TAKE)) {
    obj->obj_to_char (this);
  } else {
    obj->obj_to_room (in_room);
    act ("$n has created $p!", obj, NULL, TO_ROOM);
  }
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_purge (std::string argument)
{
  std::string arg;
  Character *victim;
  Object *obj;

  one_argument (argument, arg);

  if (arg.empty()) {
    /* 'purge' */

    CharIter rch, rnext;
    for (rch = in_room->people.begin(); rch != in_room->people.end(); rch = rnext) {
      victim = *rch;
      rnext = ++rch;
      if (victim->is_npc () && victim != this)
        victim->extract_char (true);
    }

    ObjIter o, onext;
    for (o = in_room->contents.begin(); o != in_room->contents.end(); o = onext) {
      obj = *o;
      onext = ++o;
      obj->extract_obj ();
    }

    act ("$n purges the room!", NULL, NULL, TO_ROOM);
    send_to_char ("Ok.\r\n");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!victim->is_npc ()) {
    send_to_char ("Not on PC's.\r\n");
    return;
  }

  act ("$n purges $N.", NULL, victim, TO_NOTVICT);
  victim->extract_char (true);
  return;
}

void Character::do_advance (std::string argument)
{
  std::string arg1, arg2;
  Character *victim;
  int lvl;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty() || !is_number (arg2)) {
    send_to_char ("Syntax: advance <char> <level>.\r\n");
    return;
  }

  if ((victim = get_char_room (arg1)) == NULL) {
    send_to_char ("That player is not here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if ((lvl = std::atoi (arg2.c_str())) < 1 || lvl > 40) {
    send_to_char ("Level must be 1 to 40.\r\n");
    return;
  }

  if (lvl > get_trust ()) {
    send_to_char ("Limited to your trust level.\r\n");
    return;
  }

  /*
   * Lower level:
   *   Reset to level 1.
   *   Then raise again.
   *   Currently, an imp can lower another imp.
   *   -- Swiftest
   */
  if (lvl <= victim->level) {
    int sn;

    send_to_char ("Lowering a player's level!\r\n");
    victim->send_to_char ("**** OOOOHHHHHHHHHH  NNNNOOOO ****\r\n");
    victim->level = 1;
    victim->exp = 1000;
    victim->max_hit = 10;
    victim->max_mana = 100;
    victim->max_move = 100;
    for (sn = 0; sn < MAX_SKILL; sn++)
      victim->pcdata->learned[sn] = 0;
    victim->practice = 0;
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    victim->advance_level();
  } else {
    send_to_char ("Raising a player's level!\r\n");
    victim->send_to_char ("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\r\n");
  }

  for (int iLevel = victim->level; iLevel < lvl; iLevel++) {
    victim->send_to_char ("You raise a level!!  ");
    victim->level += 1;
    victim->advance_level();
  }
  victim->exp = 1000 * std::max (1, victim->level);
  victim->trust = 0;
  return;
}

void Character::do_trust (std::string argument)
{
  std::string arg1, arg2;
  Character *victim;
  int lvl;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || arg2.empty() || !is_number (arg2)) {
    send_to_char ("Syntax: trust <char> <level>.\r\n");
    return;
  }

  if ((victim = get_char_room (arg1)) == NULL) {
    send_to_char ("That player is not here.\r\n");
    return;
  }

  if ((lvl = std::atoi (arg2.c_str())) < 0 || lvl > 40) {
    send_to_char ("Level must be 0 (reset) or 1 to 40.\r\n");
    return;
  }

  if (lvl > get_trust ()) {
    send_to_char ("Limited to your trust.\r\n");
    return;
  }

  victim->trust = lvl;
  return;
}

void Character::do_restore (std::string argument)
{
  std::string arg;
  Character *victim;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Restore whom?\r\n");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  victim->update_pos();
  act ("$n has restored you.", NULL, victim, TO_VICT);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_freeze (std::string argument)
{
  std::string arg;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Freeze whom?\r\n");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (victim->get_trust () >= get_trust ()) {
    send_to_char ("You failed.\r\n");
    return;
  }

  if (IS_SET (victim->actflags, PLR_FREEZE)) {
    REMOVE_BIT (victim->actflags, PLR_FREEZE);
    victim->send_to_char ("You can play again.\r\n");
    send_to_char ("FREEZE removed.\r\n");
  } else {
    SET_BIT (victim->actflags, PLR_FREEZE);
    victim->send_to_char ("You can't do ANYthing!\r\n");
    send_to_char ("FREEZE set.\r\n");
  }

  victim->save_char_obj();

  return;
}

void Character::do_noemote (std::string argument)
{
  std::string arg;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Noemote whom?\r\n");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (victim->get_trust () >= get_trust ()) {
    send_to_char ("You failed.\r\n");
    return;
  }

  if (IS_SET (victim->actflags, PLR_NO_EMOTE)) {
    REMOVE_BIT (victim->actflags, PLR_NO_EMOTE);
    victim->send_to_char ("You can emote again.\r\n");
    send_to_char ("NO_EMOTE removed.\r\n");
  } else {
    SET_BIT (victim->actflags, PLR_NO_EMOTE);
    victim->send_to_char ("You can't emote!\r\n");
    send_to_char ("NO_EMOTE set.\r\n");
  }

  return;
}

void Character::do_notell (std::string argument)
{
  std::string arg;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Notell whom?");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (victim->get_trust () >= get_trust ()) {
    send_to_char ("You failed.\r\n");
    return;
  }

  if (IS_SET (victim->actflags, PLR_NO_TELL)) {
    REMOVE_BIT (victim->actflags, PLR_NO_TELL);
    victim->send_to_char ("You can tell again.\r\n");
    send_to_char ("NO_TELL removed.\r\n");
  } else {
    SET_BIT (victim->actflags, PLR_NO_TELL);
    victim->send_to_char ("You can't tell!\r\n");
    send_to_char ("NO_TELL set.\r\n");
  }

  return;
}

void Character::do_silence (std::string argument)
{
  std::string arg;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Silence whom?");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  if (victim->get_trust () >= get_trust ()) {
    send_to_char ("You failed.\r\n");
    return;
  }

  if (IS_SET (victim->actflags, PLR_SILENCE)) {
    REMOVE_BIT (victim->actflags, PLR_SILENCE);
    victim->send_to_char ("You can use channels again.\r\n");
    send_to_char ("SILENCE removed.\r\n");
  } else {
    SET_BIT (victim->actflags, PLR_SILENCE);
    victim->send_to_char ("You can't use channels!\r\n");
    send_to_char ("SILENCE set.\r\n");
  }

  return;
}

void Character::do_peace (std::string argument)
{
  CharIter rch;
  for (rch = in_room->people.begin(); rch != in_room->people.end(); rch++) {
    if ((*rch)->fighting != NULL)
      (*rch)->stop_fighting (true);
  }

  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_ban (std::string argument)
{
  std::string buf;
  std::string arg;

  if (is_npc ())
    return;

  one_argument (argument, arg);

  if (arg.empty()) {
    buf = "Banned sites:\r\n";
    for (std::list<Ban*>::iterator p = ban_list.begin(); p != ban_list.end(); p++) {
      buf.append((*p)->name);
      buf.append("\r\n");
    }
    send_to_char (buf);
    return;
  }

  for (std::list<Ban*>::iterator p = ban_list.begin(); p != ban_list.end(); p++) {
    if (!str_cmp (arg, (*p)->name)) {
      send_to_char ("That site is already banned!\r\n");
      return;
    }
  }

  Ban *pban = new Ban(arg);
  ban_list.push_back(pban);
  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_allow (std::string argument)
{
  std::string arg;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Remove which site from the ban list?\r\n");
    return;
  }

  std::list<Ban*>::iterator next, curr;
  for (std::list<Ban*>::iterator pban = ban_list.begin();
    pban != ban_list.end(); pban = next) {
    curr = pban;
    next = ++pban;
    if (!str_cmp (arg, (*curr)->name)) {
      delete *curr;
      ban_list.erase(curr);
      send_to_char ("Ok.\r\n");
      return;
    }
  }

  send_to_char ("Site is not banned.\r\n");
  return;
}

void Character::do_wizlock (std::string argument)
{
  wizlock = !wizlock;

  if (wizlock)
    send_to_char ("Game wizlocked.\r\n");
  else
    send_to_char ("Game un-wizlocked.\r\n");

  return;
}

void Character::do_slookup (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string arg;
  int sn;

  one_argument (argument, arg);
  if (arg.empty()) {
    send_to_char ("Slookup what?\r\n");
    return;
  }

  if (!str_cmp (arg, "all")) {
    std::string buf1;
    for (sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name == NULL)
        break;
      snprintf (buf, sizeof buf, "Sn: %4d Skill/spell: '%s'\r\n",
        sn, skill_table[sn].name);
      buf1.append(buf);
    }
    send_to_char (buf1);
  } else {
    if ((sn = skill_lookup (arg)) < 0) {
      send_to_char ("No such skill or spell.\r\n");
      return;
    }

    snprintf (buf, sizeof buf, "Sn: %4d Skill/spell: '%s'\r\n",
      sn, skill_table[sn].name);
    send_to_char (buf);
  }

  return;
}

void Character::do_sset (std::string argument)
{
  std::string arg1, arg2, arg3;
  Character *victim;
  int value;
  int sn;
  bool fAll;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (arg1.empty() || arg2.empty() || arg3.empty()) {
    send_to_char ("Syntax: sset <victim> <skill> <value>\r\n");
    send_to_char ("or:     sset <victim> all     <value>\r\n");
    send_to_char ("Skill being any skill or spell.\r\n");
    return;
  }

  if ((victim = get_char_world (arg1)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (victim->is_npc ()) {
    send_to_char ("Not on NPC's.\r\n");
    return;
  }

  fAll = !str_cmp (arg2, "all");
  sn = 0;
  if (!fAll && (sn = skill_lookup (arg2)) < 0) {
    send_to_char ("No such skill or spell.\r\n");
    return;
  }

  /*
   * Snarf the value.
   */
  if (!is_number (arg3)) {
    send_to_char ("Value must be numeric.\r\n");
    return;
  }

  value = std::atoi (arg3.c_str());
  if (value < 0 || value > 100) {
    send_to_char ("Value range is 0 to 100.\r\n");
    return;
  }

  if (fAll) {
    for (sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name != NULL)
        victim->pcdata->learned[sn] = value;
    }
  } else {
    victim->pcdata->learned[sn] = value;
  }

  return;
}

void Character::do_mset (std::string argument)
{
  std::string arg1, arg2, arg3;
  char buf[MAX_STRING_LENGTH];
  Character *victim;
  int value, max;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  arg3 = argument;

  if (arg1.empty() || arg2.empty() || arg3.empty()) {
    send_to_char ("Syntax: mset <victim> <field>  <value>\r\n");
    send_to_char ("or:     mset <victim> <string> <value>\r\n");
    send_to_char ("\r\n");
    send_to_char ("Field being one of:\r\n");
    send_to_char ("  str int wis dex con sex class level\r\n");
    send_to_char ("  gold hp mana move practice align\r\n");
    send_to_char ("  thirst drunk full");
    send_to_char ("\r\n");
    send_to_char ("String being one of:\r\n");
    send_to_char ("  name short long description title spec\r\n");
    return;
  }

  if ((victim = get_char_world (arg1)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number (arg3) ? std::atoi (arg3.c_str()) : -1;

  /*
   * Set something.
   */
  if (!str_cmp (arg2, "str")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (class_table[klass].attr_prime == APPLY_STR)
      max = 25;
    else
      max = 18;

    if (value < 3 || value > max) {
      snprintf (buf, sizeof buf, "Strength range is 3 to %d.\r\n", max);
      send_to_char (buf);
      return;
    }

    victim->pcdata->set_perm("str", value);
    return;
  }

  if (!str_cmp (arg2, "int")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (class_table[klass].attr_prime == APPLY_INT)
      max = 25;
    else
      max = 18;

    if (value < 3 || value > max) {
      snprintf (buf, sizeof buf, "Intelligence range is 3 to %d.\r\n", max);
      send_to_char (buf);
      return;
    }

    victim->pcdata->set_perm("int", value);
    return;
  }

  if (!str_cmp (arg2, "wis")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (class_table[klass].attr_prime == APPLY_WIS)
      max = 25;
    else
      max = 18;

    if (value < 3 || value > max) {
      snprintf (buf, sizeof buf, "Wisdom range is 3 to %d.\r\n", max);
      send_to_char (buf);
      return;
    }

    victim->pcdata->set_perm("wis", value);
    return;
  }

  if (!str_cmp (arg2, "dex")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (class_table[klass].attr_prime == APPLY_DEX)
      max = 25;
    else
      max = 18;

    if (value < 3 || value > max) {
      snprintf (buf, sizeof buf, "Dexterity range is 3 to %d.\r\n", max);
      send_to_char (buf);
      return;
    }

    victim->pcdata->set_perm("dex", value);
    return;
  }

  if (!str_cmp (arg2, "con")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (class_table[klass].attr_prime == APPLY_CON)
      max = 25;
    else
      max = 18;

    if (value < 3 || value > max) {
      snprintf (buf, sizeof buf, "Constitution range is 3 to %d.\r\n", max);
      send_to_char (buf);
      return;
    }

    victim->pcdata->set_perm("con", value);
    return;
  }

  if (!str_cmp (arg2, "sex")) {
    if (value < 0 || value > 2) {
      send_to_char ("Sex range is 0 to 2.\r\n");
      return;
    }
    victim->sex = value;
    return;
  }

  if (!str_cmp (arg2, "class")) {
    if (value < 0 || value >= CLASS_MAX) {
      char buf[MAX_STRING_LENGTH];

      snprintf (buf, sizeof buf, "Class range is 0 to %d.\n", CLASS_MAX - 1);
      send_to_char (buf);
      return;
    }
    victim->klass = value;
    return;
  }

  if (!str_cmp (arg2, "level")) {
    if (!victim->is_npc ()) {
      send_to_char ("Not on PC's.\r\n");
      return;
    }

    if (value < 0 || value > 50) {
      send_to_char ("Level range is 0 to 50.\r\n");
      return;
    }
    victim->level = value;
    return;
  }

  if (!str_cmp (arg2, "gold")) {
    victim->gold = value;
    return;
  }

  if (!str_cmp (arg2, "hp")) {
    if (value < -10 || value > 30000) {
      send_to_char ("Hp range is -10 to 30,000 hit points.\r\n");
      return;
    }
    victim->max_hit = value;
    return;
  }

  if (!str_cmp (arg2, "mana")) {
    if (value < 0 || value > 30000) {
      send_to_char ("Mana range is 0 to 30,000 mana points.\r\n");
      return;
    }
    victim->max_mana = value;
    return;
  }

  if (!str_cmp (arg2, "move")) {
    if (value < 0 || value > 30000) {
      send_to_char ("Move range is 0 to 30,000 move points.\r\n");
      return;
    }
    victim->max_move = value;
    return;
  }

  if (!str_cmp (arg2, "practice")) {
    if (value < 0 || value > 100) {
      send_to_char ("Practice range is 0 to 100 sessions.\r\n");
      return;
    }
    victim->practice = value;
    return;
  }

  if (!str_cmp (arg2, "align")) {
    if (value < -1000 || value > 1000) {
      send_to_char ("Alignment range is -1000 to 1000.\r\n");
      return;
    }
    victim->alignment = value;
    return;
  }

  if (!str_cmp (arg2, "thirst")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (value < 0 || value > 100) {
      send_to_char ("Thirst range is 0 to 100.\r\n");
      return;
    }

    victim->pcdata->condition[COND_THIRST] = value;
    return;
  }

  if (!str_cmp (arg2, "drunk")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (value < 0 || value > 100) {
      send_to_char ("Drunk range is 0 to 100.\r\n");
      return;
    }

    victim->pcdata->condition[COND_DRUNK] = value;
    return;
  }

  if (!str_cmp (arg2, "full")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    if (value < 0 || value > 100) {
      send_to_char ("Full range is 0 to 100.\r\n");
      return;
    }

    victim->pcdata->condition[COND_FULL] = value;
    return;
  }

  if (!str_cmp (arg2, "name")) {
    if (!victim->is_npc ()) {
      send_to_char ("Not on PC's.\r\n");
      return;
    }

    victim->name = arg3;
    return;
  }

  if (!str_cmp (arg2, "short")) {
    victim->short_descr = arg3;
    return;
  }

  if (!str_cmp (arg2, "long")) {
    victim->long_descr = arg3;
    return;
  }

  if (!str_cmp (arg2, "title")) {
    if (victim->is_npc ()) {
      send_to_char ("Not on NPC's.\r\n");
      return;
    }

    victim->set_title(arg3);
    return;
  }

  if (!str_cmp (arg2, "spec")) {
    if (!victim->is_npc ()) {
      send_to_char ("Not on PC's.\r\n");
      return;
    }

    if ((victim->spec_fun = spec_lookup (arg3)) == 0) {
      send_to_char ("No such spec fun.\r\n");
      return;
    }

    return;
  }

  /*
   * Generate usage message.
   */
  do_mset ("");
  return;
}

void Character::do_oset (std::string argument)
{
  std::string arg1, arg2, arg3;
  Object *obj;
  int value;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  arg3 = argument;

  if (arg1.empty() || arg2.empty() || arg3.empty()) {
    send_to_char ("Syntax: oset <object> <field>  <value>\r\n");
    send_to_char ("or:     oset <object> <string> <value>\r\n");
    send_to_char ("\r\n");
    send_to_char ("Field being one of:\r\n");
    send_to_char ("  value0 value1 value2 value3\r\n");
    send_to_char ("  extra wear level weight cost timer\r\n");
    send_to_char ("\r\n");
    send_to_char ("String being one of:\r\n");
    send_to_char ("  name short long ed\r\n");
    return;
  }

  if ((obj = get_obj_world (arg1)) == NULL) {
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = std::atoi (arg3.c_str());

  /*
   * Set something.
   */
  if (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0")) {
    obj->value[0] = value;
    return;
  }

  if (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1")) {
    obj->value[1] = value;
    return;
  }

  if (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2")) {
    obj->value[2] = value;
    return;
  }

  if (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3")) {
    obj->value[3] = value;
    return;
  }

  if (!str_cmp (arg2, "extra")) {
    obj->extra_flags = value;
    return;
  }

  if (!str_cmp (arg2, "wear")) {
    obj->wear_flags = value;
    return;
  }

  if (!str_cmp (arg2, "level")) {
    obj->level = value;
    return;
  }

  if (!str_cmp (arg2, "weight")) {
    obj->weight = value;
    return;
  }

  if (!str_cmp (arg2, "cost")) {
    obj->cost = value;
    return;
  }

  if (!str_cmp (arg2, "timer")) {
    obj->timer = value;
    return;
  }

  if (!str_cmp (arg2, "name")) {
    obj->name = arg3;
    return;
  }

  if (!str_cmp (arg2, "short")) {
    obj->short_descr = arg3;
    return;
  }

  if (!str_cmp (arg2, "long")) {
    obj->description = arg3;
    return;
  }

  if (!str_cmp (arg2, "ed")) {
    ExtraDescription *ed;

    argument = one_argument (argument, arg3);
    if (argument.empty()) {
      send_to_char ("Syntax: oset <object> ed <keyword> <string>\r\n");
      return;
    }

    ed = new ExtraDescription();

    ed->keyword = arg3;
    ed->description = argument;
    obj->extra_descr.push_back(ed);
    return;
  }

  /*
   * Generate usage message.
   */
  do_oset ("");
  return;
}

void Character::do_rset (std::string argument)
{
  std::string arg1, arg2, arg3;
  Room *location;
  int value;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  arg3 = argument;

  if (arg1.empty() || arg2.empty() || arg3.empty()) {
    send_to_char ("Syntax: rset <location> <field> value\r\n");
    send_to_char ("\r\n");
    send_to_char ("Field being one of:\r\n");
    send_to_char ("  flags sector\r\n");
    return;
  }

  if ((location = find_location (this, arg1)) == NULL) {
    send_to_char ("No such location.\r\n");
    return;
  }

  /*
   * Snarf the value.
   */
  if (!is_number (arg3)) {
    send_to_char ("Value must be numeric.\r\n");
    return;
  }
  value = std::atoi (arg3.c_str());

  /*
   * Set something.
   */
  if (!str_cmp (arg2, "flags")) {
    location->room_flags = value;
    return;
  }

  if (!str_cmp (arg2, "sector")) {
    location->sector_type = value;
    return;
  }

  /*
   * Generate usage message.
   */
  do_rset ("");
  return;
}

void Character::do_users (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int count;

  count = 0;
  buf[0] = '\0';
  buf2[0] = '\0';
  for (DescIter d = descriptor_list.begin();
    d != descriptor_list.end(); d++) {
    if ((*d)->character != NULL && can_see((*d)->character)) {
      count++;
      snprintf (buf + strlen(buf), sizeof(buf) - strlen(buf), "[%3d %2d] %s@%s\r\n",
        (*d)->descriptor,
        (*d)->connected,
        (*d)->original ? (*d)->original->name.c_str() :
        (*d)->character ? (*d)->character->name.c_str() : "(none)", (*d)->host.c_str());
    }
  }

  snprintf (buf2, sizeof buf2, "%d user%s\r\n", count, count == 1 ? "" : "s");
  strncat (buf, buf2, sizeof buf - sizeof buf2);
  send_to_char (buf);
  return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void Character::do_force (std::string argument)
{
  std::string arg;
  int trst;
  int cmd;

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    send_to_char ("Force whom to do what?\r\n");
    return;
  }

  /*
   * Look for command in command table.
   */
  trst = get_trust ();
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
    if (argument[0] == cmd_table[cmd].name[0]
      && !str_prefix (argument, cmd_table[cmd].name)
      && (cmd_table[cmd].level > trst && cmd_table[cmd].level != 41)) {
      send_to_char ("You cant even do that yourself!\r\n");
      return;
    }
  }

  if (!str_cmp (arg, "all")) {
    Character *vch;
    CharIter c, next;
    for (c = char_list.begin(); c != char_list.end(); c = next) {
      vch = *c;
      next = ++c;
      if (!vch->is_npc () && vch->get_trust () < get_trust ()) {
        MOBtrigger = false;
        act ("$n forces you to '$t'.", argument.c_str(), vch, TO_VICT);
        vch->interpret (argument);
      }
    }
  } else {
    Character *victim;

    if ((victim = get_char_world (arg)) == NULL) {
      send_to_char ("They aren't here.\r\n");
      return;
    }

    if (victim == this) {
      send_to_char ("Aye aye, right away!\r\n");
      return;
    }

    if (victim->get_trust () >= get_trust ()) {
      send_to_char ("Do it yourself!\r\n");
      return;
    }

    MOBtrigger = false;
    act ("$n forces you to '$t'.", argument.c_str(), victim, TO_VICT);
    victim->interpret (argument);
  }

  send_to_char ("Ok.\r\n");
  return;
}

void Character::do_holylight (std::string argument)
{
  if (is_npc ())
    return;

  if (IS_SET (actflags, PLR_HOLYLIGHT)) {
    REMOVE_BIT (actflags, PLR_HOLYLIGHT);
    send_to_char ("Holy light mode off.\r\n");
  } else {
    SET_BIT (actflags, PLR_HOLYLIGHT);
    send_to_char ("Holy light mode on.\r\n");
  }

  return;
}

/* Wizify and Wizbit sent in by M. B. King */
void Character::do_wizify (std::string argument)
{
  std::string arg1;
  Character *victim;

  argument = one_argument (argument, arg1);
  if (arg1.empty()) {
    send_to_char ("Syntax: wizify <name>\r\n");
    return;
  }
  if ((victim = get_char_world (arg1)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }
  if (victim->is_npc ()) {
    send_to_char ("Not on mobs.\r\n");
    return;
  }
  victim->wizbit = !victim->wizbit;
  if (victim->wizbit) {
    act ("$N wizified.\r\n", NULL, victim, TO_CHAR);
    act ("$n has wizified you!\r\n", NULL, victim, TO_VICT);
  } else {
    act ("$N dewizzed.\r\n", NULL, victim, TO_CHAR);
    act ("$n has dewizzed you!\r\n", NULL, victim, TO_VICT);
  }

  victim->do_save ("");
  return;
}

/* Idea from Talen of Vego's do_where command */
void Character::do_owhere (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string arg;
  bool found = false;
  Object *in_obj;
  int obj_counter = 1;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("Syntax:  owhere <object>.\r\n");
    return;
  } else {
    ObjIter o;
    for (o = object_list.begin(); o != object_list.end(); o++) {
      if (!can_see_obj(*o) || !is_name (arg, (*o)->name))
        continue;

      found = true;

      for (in_obj = *o; in_obj->in_obj != NULL; in_obj = in_obj->in_obj) ;

      if (in_obj->carried_by != NULL) {
        snprintf (buf, sizeof buf, "[%2d] %s carried by %s.\r\n", obj_counter,
          (*o)->short_descr.c_str(), in_obj->carried_by->describe_to(this).c_str());
      } else {
        snprintf (buf, sizeof buf, "[%2d] %s in %s.\r\n", obj_counter,
          (*o)->short_descr.c_str(), (in_obj->in_room == NULL) ?
          "somewhere" : in_obj->in_room->name.c_str());
      }

      obj_counter++;
      buf[0] = toupper (buf[0]);
      send_to_char (buf);
    }
  }

  if (!found)
    send_to_char ("Nothing like that in hell, earth, or heaven.\r\n");

  return;
}


