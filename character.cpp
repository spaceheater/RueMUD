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

#include "descriptor.hpp"
#include "pcdata.hpp"
#include "object.hpp"
#include "room.hpp"
#include "mobproto.hpp"
#include "objproto.hpp"
#include "affect.hpp"
#include "area.hpp"
#include "exit.hpp"
#include "database.hpp"
#include "world.hpp"

// temp globals
extern void multi_hit(Character *ch, Character *victim, int dt);
extern void mprog_act_trigger (const std::string & buf, Character * mob, Character * ch,
  Object * obj, void *vo);
extern void mprog_entry_trigger (Character * mob);
extern void mprog_greet_trigger (Character * mob);
extern std::string get_title (int klass, int level, int sex);

sh_int movement_loss[SECT_MAX] = {
  1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};


Character::Character() :
  master(NULL), leader(NULL), fighting(NULL),
  reply(NULL), spec_fun(NULL), pIndexData(NULL), desc(NULL),
  pnote(NULL), in_room(NULL), was_in_room(NULL),
  pcdata(NULL), sex(0), klass(0), race(0), level(0), trust(0), wizbit(false),
  played(0), save_time(0), last_note(0), timer(0),
  wait(0), hit(20), max_hit(20), mana(100), max_mana(100), move(100),
  max_move(100), gold(0), exp(0), actflags(0), affected_by(0),
  position(POS_STANDING), practice(21), carry_weight(0), carry_number(0),
  saving_throw(0), alignment(0), hitroll(0), damroll(0), armor(100),
  wimpy(0), deaf(0), mpact(NULL), mpactnum(0) {
  logon = g_world->get_current_time();
}

/*
 * Free a character.
 */
Character::~Character()
{
  Object *obj;
  Affect *paf;

  ObjIter o, onext;
  for (o = carrying.begin(); o != carrying.end(); o = onext) {
    obj = *o;
    onext = ++o;
    obj->extract_obj ();
  }

  AffIter af, anext;
  for (af = affected.begin(); af != affected.end(); af = anext) {
    paf = *af;
    anext = ++af;
    affect_remove (paf);
  }

  if (pcdata != NULL) {
    delete pcdata;
  }

  return;
}

int Character::mana_cost(int sn) {
  if (is_npc())
    return 0;
  else
    return std::max(skill_table[sn].min_mana, 100 /
      (2 + level - skill_table[sn].skill_level[klass]));
}

/*
 * Write to one char.
 */
void Character::send_to_char (const std::string & txt)
{
  if (txt.empty() || desc == NULL)
    return;
  desc->showstr_head = (char*)std::malloc(txt.size()+1);
  strncpy (desc->showstr_head, txt.c_str(), txt.size()+1);
  desc->showstr_point = desc->showstr_head;
  desc->show_string ("");

}

/*
 * Append a string to a file.
 */
void Character::append_file (const char *file, const std::string & str)
{
  if (is_npc () || str.empty())
    return;

  std::ofstream outfile;

  outfile.open (file, std::ofstream::out | std::ofstream::app | std::ofstream::binary);
  if (outfile.is_open()) {
    outfile << "[" << std::setw(5) << (in_room ? in_room->vnum : 0) <<
      "] " << name << ": " << str << std::endl;
    outfile.close();
  } else {
    std::perror (file);
    send_to_char ("Could not open the file!\r\n");
  }
  return;
}

/*
 * True if char can see victim.
 */
bool Character::can_see (Character * victim)
{
  if (this == victim)
    return true;

  if (!is_npc () && IS_SET (actflags, PLR_HOLYLIGHT))
    return true;

  if (is_affected (AFF_BLIND))
    return false;

  if (in_room->is_dark() && !is_affected (AFF_INFRARED))
    return false;

  if (victim->is_affected (AFF_INVISIBLE)
    && !is_affected (AFF_DETECT_INVIS))
    return false;

  if (victim->is_affected (AFF_HIDE)
    && !is_affected (AFF_DETECT_HIDDEN)
    && victim->fighting == NULL
    && (is_npc () ? !victim->is_npc () : victim->is_npc ()))
    return false;

  return true;
}

/*
 * True if char can see obj.
 */
bool Character::can_see_obj (Object * obj)
{
  if (!is_npc () && IS_SET (actflags, PLR_HOLYLIGHT))
    return true;

  if (obj->item_type == ITEM_POTION)
    return true;

  if (is_affected (AFF_BLIND))
    return false;

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    return true;

  if (in_room->is_dark() && !is_affected (AFF_INFRARED))
    return false;

  if (IS_SET (obj->extra_flags, ITEM_INVIS)
    && !is_affected (AFF_DETECT_INVIS))
    return false;

  return true;
}

/*
 * True if char can drop obj.
 */
bool Character::can_drop_obj (Object * obj)
{
  if (!IS_SET (obj->extra_flags, ITEM_NODROP))
    return true;

  if (!is_npc () && level >= LEVEL_IMMORTAL)
    return true;

  return false;
}

/*
 * The primary output interface for formatted output.
 */
void Character::act (const std::string & format, const void *arg1, const void *arg2, int type)
{
  static const char * he_she[] = { "it", "he", "she" };
  static const char * him_her[] = { "it", "him", "her" };
  static const char * his_her[] = { "its", "his", "her" };

  Character *vch = (Character *) arg2;
  Object *obj1 = (Object *) arg1;
  Object *obj2 = (Object *) arg2;

  /*
   * Discard null and zero-length messages.
   */
  if (format.empty())
    return;

  CharIter to = in_room->people.begin();
  CharIter tend = in_room->people.end();

  if (type == TO_VICT) {
    if (vch == NULL) {
      bug_printf ("Act: null vch with TO_VICT.");
      return;
    }
    to = vch->in_room->people.begin();
    tend = vch->in_room->people.end();
  }

  for (; to != tend; to++) {
    if (((*to)->desc == NULL
        && ((*to)->is_npc () && !((*to)->pIndexData->progtypes & ACT_PROG)))
      || !(*to)->is_awake ())
      continue;

    if (type == TO_CHAR && *to != this)
      continue;
    if (type == TO_VICT && (*to != vch || *to == this))
      continue;
    if (type == TO_ROOM && *to == this)
      continue;
    if (type == TO_NOTVICT && (*to == this || *to == vch))
      continue;
    if (!is_npc() && (*to)->is_gagged(name))
      continue;

    std::string buf;

    std::string::const_iterator str = format.begin();
    while (str != format.end()) {
      if (*str != '$') {
        buf.append(1, *str);
        str++;
        continue;
      }
      ++str;

      if (arg2 == NULL && *str >= 'A' && *str <= 'Z') {
        bug_printf ("Act: missing arg2 for code %d.", *str);
        buf.append(" <@@@> ");
      } else {
        switch (*str) {
        default:
          bug_printf ("Act: bad code %d.", *str);
          buf.append(" <@@@> ");
          break;
          /* Thx alex for 't' idea */
        case 't':
          buf.append((char *) arg1);
          break;
        case 'T':
          buf.append((char *) arg2);
          break;
        case 'n':
          buf.append(describe_to(*to));
          break;
        case 'N':
          buf.append(vch->describe_to(*to));
          break;
        case 'e':
          buf.append(he_she[URANGE (0, sex, 2)]);
          break;
        case 'E':
          buf.append(he_she[URANGE (0, vch->sex, 2)]);
          break;
        case 'm':
          buf.append(him_her[URANGE (0, sex, 2)]);
          break;
        case 'M':
          buf.append(him_her[URANGE (0, vch->sex, 2)]);
          break;
        case 's':
          buf.append(his_her[URANGE (0, sex, 2)]);
          break;
        case 'S':
          buf.append(his_her[URANGE (0, vch->sex, 2)]);
          break;

        case 'p':
          buf.append((*to)->can_see_obj(obj1)
            ? obj1->short_descr.c_str() : "something");
          break;

        case 'P':
          buf.append((*to)->can_see_obj(obj2)
            ? obj2->short_descr.c_str() : "something");
          break;

        case 'd':
          if (arg2 == NULL || ((char *) arg2)[0] == '\0') {
            buf.append("door");
          } else {
            std::string fname;
            one_argument ((char *) arg2, fname);
            buf.append(fname);
          }
          break;
        }
      }

      ++str;
    }

    buf.append("\r\n");
    buf[0] = toupper(buf[0]);
    if ((*to)->desc)
      (*to)->desc->write_to_buffer (buf);
    if (MOBtrigger)
      mprog_act_trigger (buf, *to, this, obj1, vch);
    /* Added by Kahn */
  }

  MOBtrigger = true;
  return;
}

bool Character::is_npc() {
  return actflags & ACT_IS_NPC;
}

bool Character::is_awake() {
  return position > POS_SLEEPING;
}

bool Character::is_good() {
  return alignment >= 350;
}

bool Character::is_evil() {
  return alignment <= -350;
}

bool Character::is_neutral() {
  return !is_good() && !is_evil();
}

bool Character::is_affected(int flg) {
  return affected_by & flg;
}

int Character::get_ac() {
  if (is_awake())
    return armor + dex_app[get_curr_dex()].defensive;
  else
    return armor;
}

int Character::get_hitroll() {
  return hitroll + str_app[get_curr_str()].tohit;
}

int Character::get_damroll() {
  return damroll + str_app[get_curr_str()].todam;
}

/*
 * Retrieve character's current strength.
 */
int Character::get_curr_str() {
  int max;

  if (is_npc ())
    return 13;

  if (class_table[klass].attr_prime == APPLY_STR)
    max = 25;
  else
    max = 22;

  return URANGE (3, pcdata->get_curr("str"), max);
}

/*
 * Retrieve character's current intelligence.
 */
int Character::get_curr_int() {
  int max;

  if (is_npc ())
    return 13;

  if (class_table[klass].attr_prime == APPLY_INT)
    max = 25;
  else
    max = 22;

  return URANGE (3, pcdata->get_curr("int"), max);
}

/*
 * Retrieve character's current wisdom.
 */
int Character::get_curr_wis() {
  int max;

  if (is_npc ())
    return 13;

  if (class_table[klass].attr_prime == APPLY_WIS)
    max = 25;
  else
    max = 22;

  return URANGE (3, pcdata->get_curr("wis"), max);
}

/*
 * Retrieve character's current dexterity.
 */
int Character::get_curr_dex() {
  int max;

  if (is_npc ())
    return 13;

  if (class_table[klass].attr_prime == APPLY_DEX)
    max = 25;
  else
    max = 22;

  return URANGE (3, pcdata->get_curr("dex"), max);
}

/*
 * Retrieve character's current constitution.
 */
int Character::get_curr_con() {
  int max;

  if (is_npc ())
    return 13;

  if (class_table[klass].attr_prime == APPLY_CON)
    max = 25;
  else
    max = 22;

  return URANGE (3, pcdata->get_curr("con"), max);
}

/*
 * Retrieve a character's age.
 */
int Character::get_age() {
  return 17 + (played + (int) (g_world->get_current_time() - logon)) / 14400;
  /* 12240 assumes 30 second hours, 24 hours a day, 20 day - Kahn */
}

/*
 * Retrieve a character's carry capacity.
 */
int Character::can_carry_n() {
  if (!is_npc () && level >= LEVEL_IMMORTAL)
    return 1000;

  if (is_npc () && IS_SET (actflags, ACT_PET))
    return 0;

  return MAX_WEAR + 2 * get_curr_dex() / 2;
}

/*
 * Retrieve a character's carry capacity.
 */
int Character::can_carry_w() {
  if (!is_npc () && level >= LEVEL_IMMORTAL)
    return 1000000;

  if (is_npc () && IS_SET (actflags, ACT_PET))
    return 0;

  return str_app[get_curr_str()].carry;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int Character::get_trust() {
  Character *ch;

  if (desc != NULL && desc->original != NULL)
    ch = desc->original;
  else
    ch = this;

  if (ch->trust != 0)
    return ch->trust;

  if (ch->is_npc () && ch->level >= LEVEL_HERO)
    return LEVEL_HERO - 1;
  else
    return ch->level;
}

bool Character::is_immortal() {
  return get_trust() >= LEVEL_IMMORTAL;
}

bool Character::is_hero() {
  return get_trust() >= LEVEL_HERO;
}

int Character::is_outside() {
  return !(in_room->room_flags & ROOM_INDOORS);
}

void Character::wait_state(int npulse) {
  wait = std::max(wait, npulse);
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool Character::saves_spell (int lvl) {
  int save;

  save = 50 + (level - lvl - saving_throw) * 5;
  save = URANGE (5, save, 95);
  return number_percent () < save;
}

std::string Character::describe_to (Character* looker) {
  if (looker->can_see(this)) {
    if (is_npc())
      return short_descr;
    else
      return name;
  } else {
    return "someone";
  }
}

/*
 * Find a piece of eq on a character.
 */
Object * Character::get_eq_char (int iWear)
{
  for (ObjIter o = carrying.begin(); o != carrying.end(); o++) {
    if ((*o)->wear_loc == iWear)
      return *o;
  }

  return NULL;
}

/*
 * Apply or remove an affect to a character.
 */
void Character::affect_modify (Affect * paf, bool fAdd)
{
  Object *wield;

  int mod = paf->modifier;

  if (fAdd) {
    SET_BIT (affected_by, paf->bitvector);
  } else {
    REMOVE_BIT (affected_by, paf->bitvector);
    mod = 0 - mod;
  }

  if (is_npc ())
    return;

  switch (paf->location) {
  default:
    bug_printf ("Affect_modify: unknown location %d.", paf->location);
    return;

  case APPLY_NONE:
    break;
  case APPLY_STR:
    pcdata->set_mod("str", pcdata->get_mod("str") + mod);
    break;
  case APPLY_DEX:
    pcdata->set_mod("dex", pcdata->get_mod("dex") + mod);
    break;
  case APPLY_INT:
    pcdata->set_mod("int", pcdata->get_mod("int") + mod);
    break;
  case APPLY_WIS:
    pcdata->set_mod("wis", pcdata->get_mod("wis") + mod);
    break;
  case APPLY_CON:
    pcdata->set_mod("con", pcdata->get_mod("con") + mod);
    break;
  case APPLY_SEX:
    sex += mod;
    break;
  case APPLY_CLASS:
    break;
  case APPLY_LEVEL:
    break;
  case APPLY_AGE:
    break;
  case APPLY_HEIGHT:
    break;
  case APPLY_WEIGHT:
    break;
  case APPLY_MANA:
    max_mana += mod;
    break;
  case APPLY_HIT:
    max_hit += mod;
    break;
  case APPLY_MOVE:
    max_move += mod;
    break;
  case APPLY_GOLD:
    break;
  case APPLY_EXP:
    break;
  case APPLY_AC:
    armor += mod;
    break;
  case APPLY_HITROLL:
    hitroll += mod;
    break;
  case APPLY_DAMROLL:
    damroll += mod;
    break;
  case APPLY_SAVING_PARA:
    saving_throw += mod;
    break;
  case APPLY_SAVING_ROD:
    saving_throw += mod;
    break;
  case APPLY_SAVING_PETRI:
    saving_throw += mod;
    break;
  case APPLY_SAVING_BREATH:
    saving_throw += mod;
    break;
  case APPLY_SAVING_SPELL:
    saving_throw += mod;
    break;
  }

  /*
   * Check for weapon wielding.
   * Guard against recursion (for weapons with affects).
   */
  if ((wield = get_eq_char (WEAR_WIELD)) != NULL
    && wield->get_obj_weight() > str_app[get_curr_str()].wield) {
    static int depth;

    if (depth == 0) {
      depth++;
      act ("You drop $p.", wield, NULL, TO_CHAR);
      act ("$n drops $p.", wield, NULL, TO_ROOM);
      wield->obj_from_char();
      wield->obj_to_room (in_room);
      depth--;
    }
  }

  return;
}

/*
 * Unequip a char with an obj.
 */
void Character::unequip_char (Object * obj)
{
  if (obj->wear_loc == WEAR_NONE) {
    bug_printf ("Unequip_char: already unequipped.");
    return;
  }

  armor += obj->apply_ac (obj->wear_loc);
  obj->wear_loc = -1;

  AffIter paf;
  for (paf = obj->pIndexData->affected.begin(); paf != obj->pIndexData->affected.end(); paf++)
    affect_modify (*paf, false);
  for (paf = obj->affected.begin(); paf != obj->affected.end(); paf++)
    affect_modify (*paf, false);

  if (obj->item_type == ITEM_LIGHT
    && obj->value[2] != 0 && in_room != NULL && in_room->light > 0)
    --in_room->light;

  return;
}

/*
 * Give an affect to a char.
 */
void Character::affect_to_char (Affect * paf)
{
  Affect *paf_new = new Affect();

  *paf_new = *paf;
  affected.push_back(paf_new);

  affect_modify (paf_new, true);
  return;
}

/*
 * Remove an affect from a char.
 */
void Character::affect_remove (Affect * paf)
{
  if (affected.empty()) {
    bug_printf ("Affect_remove: no affect.");
    return;
  }

  affect_modify (paf, false);

  affected.erase(find(affected.begin(), affected.end(), paf));

  delete paf;
  return;
}

/*
 * Strip all affects of a given sn.
 */
void Character::affect_strip (int sn)
{
  Affect *paf;

  AffIter af, next;
  for (af = affected.begin(); af != affected.end(); af = next) {
    paf = *af;
    next = ++af;
    if (paf->type == sn)
      affect_remove (paf);
  }

  return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool Character::has_affect (int sn)
{
  AffIter af;
  for (af = affected.begin(); af != affected.end(); af++) {
    if ((*af)->type == sn)
      return true;
  }

  return false;
}

/*
 * Add or enhance an affect.
 */
void Character::affect_join (Affect * paf)
{
  AffIter af;
  for (af = affected.begin(); af != affected.end(); af++) {
    if ((*af)->type == paf->type) {
      paf->duration += (*af)->duration;
      paf->modifier += (*af)->modifier;
      affect_remove (*af);
      break;
    }
  }

  affect_to_char (paf);
  return;
}

/*
 * Find a char in the room.
 */
Character * Character::get_char_room (const std::string & argument)
{
  std::string arg;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  if (!str_cmp (arg, "self"))
    return this;

  CharIter rch;
  for (rch = in_room->people.begin(); rch != in_room->people.end(); rch++) {
    if (!can_see(*rch) || !is_name (arg, (*rch)->name))
      continue;
    if (++count == number)
      return *rch;
  }

  return NULL;
}

/*
 * Find a char in the world.
 */
Character * Character::get_char_world (const std::string & argument)
{
  std::string arg;
  Character *wch;
  int number;
  int count;

  if ((wch = get_char_room (argument)) != NULL)
    return wch;

  number = number_argument (argument, arg);
  count = 0;
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c++) {
    if (!can_see(*c) || !is_name (arg, (*c)->name))
      continue;
    if (++count == number)
      return *c;
  }

  return NULL;
}

/*
 * Find an obj in a list.
 */
Object * Character::get_obj_list (const std::string & argument, std::list<Object *> & list)
{
  std::string arg;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  ObjIter obj;
  for (obj = list.begin(); obj != list.end(); obj++) {
    if (can_see_obj(*obj) && is_name (arg, (*obj)->name)) {
      if (++count == number)
        return *obj;
    }
  }

  return NULL;
}

/*
 * Find an obj in player's inventory.
 */
Object * Character::get_obj_carry (const std::string & argument)
{
  std::string arg;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  ObjIter o;
  for (o = carrying.begin(); o != carrying.end(); o++) {
    if ((*o)->wear_loc == WEAR_NONE && can_see_obj(*o)
      && is_name (arg, (*o)->name)) {
      if (++count == number)
        return *o;
    }
  }

  return NULL;
}

/*
 * Find an obj in player's equipment.
 */
Object * Character::get_obj_wear (const std::string & argument)
{
  std::string arg;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  ObjIter o;
  for (o = carrying.begin(); o != carrying.end(); o++) {
    if ((*o)->wear_loc != WEAR_NONE && can_see_obj(*o)
      && is_name (arg, (*o)->name)) {
      if (++count == number)
        return *o;
    }
  }

  return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
Object * Character::get_obj_here (const std::string & argument)
{
  Object *obj;

  obj = get_obj_list (argument, in_room->contents);
  if (obj != NULL)
    return obj;

  if ((obj = get_obj_carry (argument)) != NULL)
    return obj;

  if ((obj = get_obj_wear (argument)) != NULL)
    return obj;

  return NULL;
}

/*
 * Find an obj in the world.
 */
Object * Character::get_obj_world (const std::string & argument)
{
  std::string arg;
  Object *obj;

  if ((obj = get_obj_here (argument)) != NULL)
    return obj;

  int number = number_argument (argument, arg);
  int count = 0;
  for (ObjIter o = object_list.begin();
    o != object_list.end(); o++) {
    if (can_see_obj(*o) && is_name (arg, (*o)->name)) {
      if (++count == number)
        return *o;
    }
  }

  return NULL;
}

/*
 * Write the char.
 */
void Character::fwrite_char (std::ofstream & fp)
{

  fp << "#" << (is_npc () ? "MOB" : "PLAYER") << "\n";

  fp << "Name         " << name << "~\n";
  fp << "ShortDescr   " << short_descr << "~\n";
  fp << "LongDescr    " << long_descr << "~\n";
  fp << "Description  " << description << "~\n";
  fp << "Prompt       " << prompt << "~\n";
  fp << "Sex          " << sex << "\n";
  fp << "Class        " << klass << "\n";
  fp << "Race         " << race << "\n";
  fp << "Level        " << level << "\n";
  fp << "Trust        " << trust << "\n";
  fp << "Wizbit       " << wizbit << "\n";
  fp << "Played       " << played + (int) (g_world->get_current_time() - logon) << "\n";
  fp << "Note         " << last_note << "\n";
  fp << "Room         " <<
    ((in_room == get_room_index (ROOM_VNUM_LIMBO)
      && was_in_room != NULL)
    ? was_in_room->vnum : in_room->vnum) << "\n";

  fp << "HpManaMove   " << hit << " " << max_hit << " " <<
       mana << " " << max_mana << " " << move << " " <<
       max_move << "\n";
  fp << "Gold         " << gold << "\n";
  fp << "Exp          " << exp << "\n";
  fp << "Act          " << actflags << "\n";
  fp << "AffectedBy   " << affected_by << "\n";
  /* Bug fix from Alander */
  if (position == POS_FIGHTING)
    fp << "Position     " <<  POS_STANDING<< "\n";
  else
    fp << "Position     " <<  position << "\n";

  fp << "Practice     " << practice << "\n";
  fp << "SavingThrow  " << saving_throw << "\n";
  fp << "Alignment    " << alignment << "\n";
  fp << "Hitroll      " << hitroll << "\n";
  fp << "Damroll      " << damroll << "\n";
  fp << "Armor        " << armor << "\n";
  fp << "Wimpy        " << wimpy << "\n";
  fp << "Deaf         " << deaf << "\n";

  if (is_npc ()) {
    fp << "Vnum         " << pIndexData->vnum << "\n";
  } else {
    fp << "Password     " << pcdata->pwd << "~\n";
    fp << "Bamfin       " << pcdata->bamfin << "~\n";
    fp << "Bamfout      " << pcdata->bamfout << "~\n";
    fp << "Title        " << pcdata->title << "~\n";
    fp << "AttrPerm     " << pcdata->get_perm("str") << " " << pcdata->get_perm("int") <<
      " " << pcdata->get_perm("wis") << " " << pcdata->get_perm("dex") << " " <<
      pcdata->get_perm("con") << "\n";

    fp << "AttrMod      " << pcdata->get_mod("str") << " " << pcdata->get_mod("int") << " "
      << pcdata->get_mod("wis") << " " << pcdata->get_mod("dex") << " " <<
      pcdata->get_mod("con") << "\n";

    fp << "Condition    " << pcdata->condition[0] << " " <<
      pcdata->condition[1] << " " << pcdata->condition[2] << "\n";

    fp << "Pagelen      " << pcdata->pagelen << "\n";

    for (int sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name != NULL && pcdata->learned[sn] > 0) {
        fp << "Skill        " << pcdata->learned[sn] << "'" <<
          skill_table[sn].name << "'\n";
      }
    }
    for (std::list<std::string>::iterator gag = pcdata->gag_list.begin();
      gag != pcdata->gag_list.end(); gag++) {
      fp << "Gag          " << *gag << "~\n";
    }
  }

  for (AffIter af = affected.begin(); af != affected.end(); af++) {
    fp << "Affect " << (*af)->type << " " << (*af)->duration << " " <<
      (*af)->modifier << " " << (*af)->location << " " <<
      (*af)->bitvector << "\n";
  }

  fp << "End\n\n";
  return;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void Character::save_char_obj ()
{
  char strsave[MAX_INPUT_LENGTH];
  std::ofstream fp;

  if (is_npc () || level < 2)
    return;

  Character * ch = this;
  if (desc != NULL && desc->original != NULL)
    ch = desc->original;

  ch->save_time = g_world->get_current_time();

  /* player files parsed directories by Yaz 4th Realm */
  snprintf (strsave, sizeof strsave, "%s%s", PLAYER_DIR, capitalize(ch->name).c_str());
  fp.open (strsave, std::ofstream::out | std::ofstream::binary);
  if (!fp.is_open()) {
    bug_printf ("Save_char_obj: fopen");
    std::perror (strsave);
  } else {
    ch->fwrite_char (fp);
    std::list<Object*>::reverse_iterator o;
    for (o = ch->carrying.rbegin(); o != ch->carrying.rend(); o++)
      (*o)->fwrite_obj (ch, fp, 0);
    fp << "#END\n";
  }
  fp.close();
  return;
}

/*
 * Read in a char.
 */
void Character::fread_char (std::ifstream & fp)
{
  std::string word;
  bool fMatch;

  for (;;) {
    word = fp.eof() ? std::string("End") : fread_word (fp);
    fMatch = false;

    switch (toupper (word[0])) {
    case '*':
      fMatch = true;
      fread_to_eol (fp);
      break;

    case 'A':
      KEY ("Act", actflags, fread_number (fp));
      KEY ("AffectedBy", affected_by, fread_number (fp));
      KEY ("Alignment", alignment, fread_number (fp));
      KEY ("Armor", armor, fread_number (fp));

      if (!str_cmp (word, "Affect")) {
        Affect *paf;

        paf = new Affect();

        paf->type = fread_number (fp);
        paf->duration = fread_number (fp);
        paf->modifier = fread_number (fp);
        paf->location = fread_number (fp);
        paf->bitvector = fread_number (fp);
        affected.push_back(paf);
        fMatch = true;
        break;
      }

      if (!str_cmp (word, "AttrMod")) {
        pcdata->set_mod("str", fread_number (fp));
        pcdata->set_mod("int", fread_number (fp));
        pcdata->set_mod("wis", fread_number (fp));
        pcdata->set_mod("dex", fread_number (fp));
        pcdata->set_mod("con", fread_number (fp));
        fMatch = true;
        break;
      }

      if (!str_cmp (word, "AttrPerm")) {
        pcdata->set_perm("str", fread_number (fp));
        pcdata->set_perm("int", fread_number (fp));
        pcdata->set_perm("wis", fread_number (fp));
        pcdata->set_perm("dex", fread_number (fp));
        pcdata->set_perm("con", fread_number (fp));
        fMatch = true;
        break;
      }
      break;

    case 'B':
      KEY ("Bamfin", pcdata->bamfin, fread_string (fp));
      KEY ("Bamfout", pcdata->bamfout, fread_string (fp));
      break;

    case 'C':
      KEY ("Class", klass, fread_number (fp));

      if (!str_cmp (word, "Condition")) {
        pcdata->condition[0] = fread_number (fp);
        pcdata->condition[1] = fread_number (fp);
        pcdata->condition[2] = fread_number (fp);
        fMatch = true;
        break;
      }
      break;

    case 'D':
      KEY ("Damroll", damroll, fread_number (fp));
      KEY ("Deaf", deaf, fread_number (fp));
      KEY ("Description", description, fread_string (fp));
      break;

    case 'E':
      if (!str_cmp (word, "End"))
        return;
      KEY ("Exp", exp, fread_number (fp));
      break;

    case 'G':
      KEY ("Gold", gold, fread_number (fp));
      if (!str_cmp (word, "Gag")) {
        pcdata->gag_list.push_back(fread_string (fp));
        fMatch = true;
        break;
      }
      break;

    case 'H':
      KEY ("Hitroll", hitroll, fread_number (fp));

      if (!str_cmp (word, "HpManaMove")) {
        hit = fread_number (fp);
        max_hit = fread_number (fp);
        mana = fread_number (fp);
        max_mana = fread_number (fp);
        move = fread_number (fp);
        max_move = fread_number (fp);
        fMatch = true;
        break;
      }
      break;

    case 'L':
      KEY ("Level", level, fread_number (fp));
      KEY ("LongDescr", long_descr, fread_string (fp));
      break;

    case 'N':
      if (!str_cmp (word, "Name")) {
        /*
         * Name already set externally.
         */
        fread_to_eol (fp);
        fMatch = true;
        break;
      }
      KEY ("Note", last_note, fread_number (fp));
      break;

    case 'P':
      KEY ("Pagelen", pcdata->pagelen, fread_number (fp));
      KEY ("Password", pcdata->pwd, fread_string (fp));
      KEY ("Played", played, fread_number (fp));
      KEY ("Position", position, fread_number (fp));
      KEY ("Practice", practice, fread_number (fp));
      KEY ("Prompt", prompt, fread_string (fp));
      break;

    case 'R':
      KEY ("Race", race, fread_number (fp));

      if (!str_cmp (word, "Room")) {
        in_room = get_room_index (fread_number (fp));
        if (in_room == NULL)
          in_room = get_room_index (ROOM_VNUM_LIMBO);
        fMatch = true;
        break;
      }

      break;

    case 'S':
      KEY ("SavingThrow", saving_throw, fread_number (fp));
      KEY ("Sex", sex, fread_number (fp));
      KEY ("ShortDescr", short_descr, fread_string (fp));

      if (!str_cmp (word, "Skill")) {
        int sn;
        int value;

        value = fread_number (fp);
        sn = skill_lookup (fread_word (fp));
        if (sn < 0)
          bug_printf ("Fread_char: unknown skill.");
        else
          pcdata->learned[sn] = value;
        fMatch = true;
      }

      break;

    case 'T':
      KEY ("Trust", trust, fread_number (fp));

      if (!str_cmp (word, "Title")) {
        pcdata->title = fread_string (fp);
        if (isalpha (pcdata->title[0]) || isdigit (pcdata->title[0])) {
          pcdata->title = " " + pcdata->title;
        }
        fMatch = true;
        break;
      }

      break;

    case 'V':
      if (!str_cmp (word, "Vnum")) {
        pIndexData = get_mob_index (fread_number (fp));
        fMatch = true;
        break;
      }
      break;

    case 'W':
      KEY ("Wimpy", wimpy, fread_number (fp));
      KEY ("Wizbit", wizbit, fread_number (fp));
      break;
    }

    /* Make sure old chars have this field - Kahn */
    if (!pcdata->pagelen)
      pcdata->pagelen = 20;
    if (prompt.empty())
      prompt = "<%h %m %mv> ";

    if (!fMatch) {
      bug_printf ("Fread_char: no match.");
      fread_to_eol (fp);
    }
  }
}

/*
 * Equip a char with an obj.
 */
void Character::equip_char (Object * obj, int iWear)
{

  if (get_eq_char (iWear) != NULL) {
    bug_printf ("Equip_char: already equipped (%d).", iWear);
    return;
  }

  if ((obj->is_obj_stat(ITEM_ANTI_EVIL) && is_evil ())
    || (obj->is_obj_stat(ITEM_ANTI_GOOD) && is_good ())
    || (obj->is_obj_stat(ITEM_ANTI_NEUTRAL) && is_neutral ())) {
    /*
     * Thanks to Morgenes for the bug fix here!
     */
    act ("You are zapped by $p and drop it.", obj, NULL, TO_CHAR);
    act ("$n is zapped by $p and drops it.", obj, NULL, TO_ROOM);
    obj->obj_from_char();
    obj->obj_to_room (in_room);
    return;
  }

  armor -= obj->apply_ac (iWear);
  obj->wear_loc = iWear;

  AffIter af;
  for (af = obj->pIndexData->affected.begin(); af != obj->pIndexData->affected.end(); af++)
    affect_modify (*af, true);
  for (af = obj->affected.begin(); af != obj->affected.end(); af++)
    affect_modify (*af, true);

  if (obj->item_type == ITEM_LIGHT
    && obj->value[2] != 0 && in_room != NULL)
    ++in_room->light;

  return;
}

/*
 * Move a char out of a room.
 */
void Character::char_from_room ()
{
  Object *obj;

  if (in_room == NULL) {
    bug_printf ("Char_from_room: NULL.");
    return;
  }

  if (!is_npc ())
    --in_room->area->nplayer;

  if ((obj = get_eq_char (WEAR_LIGHT)) != NULL
    && obj->item_type == ITEM_LIGHT
    && obj->value[2] != 0 && in_room->light > 0)
    --in_room->light;

  deeprmnext = in_room->people.erase(
    find(in_room->people.begin(), in_room->people.end(), this));
  in_room = NULL;
  return;
}

/*
 * Move a char into a room.
 */
void Character::char_to_room (Room * pRoomIndex)
{
  Object *obj;

  if (pRoomIndex == NULL) {
    bug_printf ("Char_to_room: NULL.");
    return;
  }

  in_room = pRoomIndex;
  pRoomIndex->people.push_back(this);

  if (!is_npc ())
    ++in_room->area->nplayer;

  if ((obj = get_eq_char (WEAR_LIGHT)) != NULL
    && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    ++in_room->light;

  return;
}

void Character::set_title (const std::string & title)
{
  if (is_npc ()) {
    bug_printf ("Set_title: NPC.");
    return;
  }

  if (isalpha (title[0]) || isdigit (title[0])) {
    pcdata->title = " " + title;
  } else {
    pcdata->title = title;
  }

  return;
}

bool Character::is_switched ()
{
  if (!is_npc () || desc == NULL)
    return false;
  return true;
}

bool Character::mp_commands ()
{                               /* Can MOBProged mobs
                                   use mpcommands? true if yes.
                                   - Kahn */
  if (is_switched())
    return false;

  if (is_npc ()
    && pIndexData->progtypes && !is_affected (AFF_CHARM))
    return true;

  return false;

}

/*
 * Advancement stuff.
 */
void Character::advance_level ()
{
  char buf[MAX_STRING_LENGTH];
  int add_hp, add_mana, add_move, add_prac;

  snprintf (buf, sizeof buf, "the %s", get_title(klass, level, sex).c_str());
  set_title(buf);

  add_hp = con_app[get_curr_con()].hitp +
    number_range (class_table[klass].hp_min, class_table[klass].hp_max);
  add_mana = class_table[klass].fMana ? number_range (2,
    (2 * get_curr_int() + get_curr_wis()) / 8)
    : 0;
  add_move = number_range (5, (get_curr_con() + get_curr_dex()) / 4);
  add_prac = wis_app[get_curr_wis()].practice;

  add_hp = std::max (1, add_hp);
  add_mana = std::max (0, add_mana);
  add_move = std::max (10, add_move);

  max_hit += add_hp;
  max_mana += add_mana;
  max_move += add_move;
  practice += add_prac;

  if (!is_npc ())
    REMOVE_BIT (actflags, PLR_BOUGHT_PET);

  snprintf (buf, sizeof buf,
    "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\r\n",
    add_hp, max_hit, add_mana, max_mana, add_move, max_move, add_prac,
    practice);
  send_to_char (buf);
  return;
}

void Character::gain_exp(int gain)
{
  if (is_npc () || level >= LEVEL_HERO)
    return;

  exp = std::max (1000, exp + gain);
  while (level < LEVEL_HERO && exp >= 1000 * (level + 1)) {
    send_to_char ("You raise a level!!  ");
    level += 1;
    advance_level();
  }

  return;
}

/*
 * Regeneration stuff.
 */
int Character::hit_gain ()
{
  int gain;

  if (is_npc ()) {
    gain = level * 3 / 2;
  } else {
    gain = std::min (5, level);

    switch (position) {
    case POS_SLEEPING:
      gain += get_curr_con();
      break;
    case POS_RESTING:
      gain += get_curr_con() / 2;
      break;
    }

    if (pcdata->condition[COND_FULL] == 0)
      gain /= 2;

    if (pcdata->condition[COND_THIRST] == 0)
      gain /= 2;

  }

  if (is_affected (AFF_POISON))
    gain /= 4;

  return std::min (gain, max_hit - hit);
}

int Character::mana_gain ()
{
  int gain;

  if (is_npc ()) {
    gain = level;
  } else {
    gain = std::min (5, level / 2);

    switch (position) {
    case POS_SLEEPING:
      gain += get_curr_int() * 2;
      break;
    case POS_RESTING:
      gain += get_curr_int();
      break;
    }

    if (pcdata->condition[COND_FULL] == 0)
      gain /= 2;

    if (pcdata->condition[COND_THIRST] == 0)
      gain /= 2;

  }

  if (is_affected (AFF_POISON))
    gain /= 4;

  return std::min (gain, max_mana - mana);
}

int Character::move_gain ()
{
  int gain;

  if (is_npc ()) {
    gain = level;
  } else {
    gain = std::max (15, 2 * level);

    switch (position) {
    case POS_SLEEPING:
      gain += get_curr_dex();
      break;
    case POS_RESTING:
      gain += get_curr_dex() / 2;
      break;
    }

    if (pcdata->condition[COND_FULL] == 0)
      gain /= 2;

    if (pcdata->condition[COND_THIRST] == 0)
      gain /= 2;
  }

  if (is_affected (AFF_POISON))
    gain /= 4;

  return std::min (gain, max_move - move);
}

void Character::gain_condition (int iCond, int value)
{
  if (value == 0 || is_npc () || level >= LEVEL_HERO)
    return;

  int condition = pcdata->condition[iCond];
  pcdata->condition[iCond] = URANGE (0, condition + value, 48);

  if (pcdata->condition[iCond] == 0) {
    switch (iCond) {
    case COND_FULL:
      send_to_char ("You are hungry.\r\n");
      break;

    case COND_THIRST:
      send_to_char ("You are thirsty.\r\n");
      break;

    case COND_DRUNK:
      if (condition != 0)
        send_to_char ("You are sober.\r\n");
      break;
    }
  }

  return;
}

void Character::add_follower (Character * master)
{

  if (master != NULL) {
    bug_printf ("Add_follower: non-null master.");
    return;
  }

  master = master;
  leader = NULL;

  if (master->can_see(this))
    act ("$n now follows you.", NULL, master, TO_VICT);

  act ("You now follow $N.", NULL, master, TO_CHAR);

  return;
}

void Character::stop_follower()
{

  if (master == NULL) {
    bug_printf ("Stop_follower: null master.");
    return;
  }

  if (is_affected (AFF_CHARM)) {
    REMOVE_BIT (affected_by, AFF_CHARM);
    affect_strip (skill_lookup("charm person"));
  }

  if (master->can_see(this))
    act ("$n stops following you.", NULL, master, TO_VICT);
  act ("You stop following $N.", NULL, master, TO_CHAR);

  master = NULL;
  leader = NULL;
  return;
}

void Character::die_follower ()
{
  if (master != NULL)
    stop_follower();

  leader = NULL;

  for (CharIter c = char_list.begin(); c != char_list.end(); c++) {
    if ((*c)->master == this)
      (*c)->stop_follower();
    if ((*c)->leader == this)
      (*c)->leader = *c;
  }

  return;
}

/*
 * Set position of a victim.
 */
void Character::update_pos ()
{
  if (hit > 0) {
    if (position <= POS_STUNNED)
      position = POS_STANDING;
    return;
  }

  if (is_npc () || hit <= -11) {
    position = POS_DEAD;
    return;
  }

  if (hit <= -6)
    position = POS_MORTAL;
  else if (hit <= -3)
    position = POS_INCAP;
  else
    position = POS_STUNNED;

  return;
}

/*
 * Start fights.
 */
void Character::set_fighting (Character * victim)
{
  if (fighting != NULL) {
    bug_printf ("Set_fighting: already fighting");
    return;
  }

  if (is_affected (AFF_SLEEP))
    affect_strip (skill_lookup("sleep"));

  fighting = victim;
  position = POS_FIGHTING;

  return;
}

/*
 * Stop fights.
 */
void Character::stop_fighting (bool fBoth)
{
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c++) {
    if (*c == this || (fBoth && (*c)->fighting == this)) {
      (*c)->fighting = NULL;
      (*c)->position = POS_STANDING;
      (*c)->update_pos();
    }
  }

  return;
}

bool Character::check_blind ()
{
  if (!is_npc () && IS_SET (actflags, PLR_HOLYLIGHT))
    return true;

  if (is_affected (AFF_BLIND)) {
    send_to_char ("You can't see a thing!\r\n");
    return false;
  }

  return true;
}

int Character::find_door (const std::string & arg)
{
  Exit *pexit;
  int door;

  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = 0;
  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = 1;
  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = 2;
  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = 3;
  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = 4;
  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = 5;
  else {
    for (door = 0; door <= 5; door++) {
      if ((pexit = in_room->exit[door]) != NULL
        && IS_SET (pexit->exit_info, EX_ISDOOR)
        && !pexit->name.empty() && is_name (arg, pexit->name))
        return door;
    }
    act ("I see no $T here.", NULL, arg.c_str(), TO_CHAR);
    return -1;
  }

  if ((pexit = in_room->exit[door]) == NULL) {
    act ("I see no door $T here.", NULL, arg.c_str(), TO_CHAR);
    return -1;
  }

  if (!IS_SET (pexit->exit_info, EX_ISDOOR)) {
    send_to_char ("You can't do that.\r\n");
    return -1;
  }

  return door;
}

bool Character::has_key (int key)
{
  ObjIter o;
  for (o = carrying.begin(); o != carrying.end(); o++) {
    if ((*o)->pIndexData->vnum == key)
      return true;
  }

  return false;
}

void Character::get_obj (Object * obj, Object * container)
{
  if (!obj->can_wear(ITEM_TAKE)) {
    send_to_char ("You can't take that.\r\n");
    return;
  }

  if (carry_number + obj->get_obj_number() > can_carry_n()) {
    act ("$d: you can't carry that many items.",
      NULL, obj->name.c_str(), TO_CHAR);
    return;
  }

  if (carry_weight + obj->get_obj_weight() > can_carry_w()) {
    act ("$d: you can't carry that much weight.",
      NULL, obj->name.c_str(), TO_CHAR);
    return;
  }

  if (container != NULL) {
    act ("You get $p from $P.", obj, container, TO_CHAR);
    act ("$n gets $p from $P.", obj, container, TO_ROOM);
    obj->obj_from_obj ();
  } else {
    act ("You get $p.", obj, container, TO_CHAR);
    act ("$n gets $p.", obj, container, TO_ROOM);
    obj->obj_from_room ();
  }

  if (obj->item_type == ITEM_MONEY) {
    gold += obj->value[0];
    obj->extract_obj ();
  } else {
    obj->obj_to_char (this);
  }

  return;
}

/*
 * Flag a character as extractable
 */
void Character::extract_char (bool fPull)
{
  if (is_npc())
    SET_BIT(actflags, ACT_EXTRACT);
  else
    SET_BIT(actflags, PLR_EXTRACT);

  extract_chars = true;
}

/*
 * Extract a char from the world.
 */
void Character::extract_char_old (bool fPull)
{
  Object *obj;

  if (in_room == NULL) {
    bug_printf ("Extract_char: NULL.");
    return;
  }

  if (fPull)
    die_follower();

  stop_fighting (true);

  ObjIter o, onext;
  for (o = carrying.begin(); o != carrying.end(); o = onext) {
    obj = *o;
    onext = ++o;
    obj->extract_obj();
  }

  char_from_room ();

  if (!fPull) {
    char_to_room(get_room_index (ROOM_VNUM_ALTAR));
    return;
  }

  if (is_npc ())
    --pIndexData->count;

  if (desc != NULL && desc->original != NULL)
    do_return ("");

  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c++) {
    if ((*c)->reply == this)
      (*c)->reply = NULL;
  }

  deepchnext = char_list.erase(find(char_list.begin(), char_list.end(), this));

  if (desc)
    desc->character = NULL;
  delete this;
  return;
}

void Character::stop_idling ()
{
  if (desc == NULL || desc->connected != CON_PLAYING
    || was_in_room == NULL || in_room != get_room_index (ROOM_VNUM_LIMBO))
    return;

  timer = 0;
  char_from_room();
  char_to_room(was_in_room);
  was_in_room = NULL;
  act ("$n has returned from the void.", NULL, NULL, TO_ROOM);
  return;
}

/*
 * Remove an object.
 */
bool Character::remove_obj (int iWear, bool fReplace)
{
  Object *obj;

  if ((obj = get_eq_char (iWear)) == NULL)
    return true;

  if (!fReplace)
    return false;

  if (IS_SET (obj->extra_flags, ITEM_NOREMOVE)) {
    act ("You can't remove $p.", obj, NULL, TO_CHAR);
    return false;
  }

  unequip_char(obj);
  act ("$n stops using $p.", obj, NULL, TO_ROOM);
  act ("You stop using $p.", obj, NULL, TO_CHAR);
  return true;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void Character::wear_obj (Object * obj, bool fReplace)
{
  char buf[MAX_STRING_LENGTH];

  if (level < obj->level) {
    snprintf (buf, sizeof buf, "You must be level %d to use this object.\r\n", obj->level);
    send_to_char (buf);
    act ("$n tries to use $p, but is too inexperienced.",
      obj, NULL, TO_ROOM);
    return;
  }

  if (obj->item_type == ITEM_LIGHT) {
    if (!remove_obj (WEAR_LIGHT, fReplace))
      return;
    act ("$n lights $p and holds it.", obj, NULL, TO_ROOM);
    act ("You light $p and hold it.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_LIGHT);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_FINGER)) {
    if (get_eq_char (WEAR_FINGER_L) != NULL
      && get_eq_char (WEAR_FINGER_R) != NULL
      && !remove_obj (WEAR_FINGER_L, fReplace)
      && !remove_obj (WEAR_FINGER_R, fReplace))
      return;

    if (get_eq_char (WEAR_FINGER_L) == NULL) {
      act ("$n wears $p on $s left finger.", obj, NULL, TO_ROOM);
      act ("You wear $p on your left finger.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_FINGER_L);
      return;
    }

    if (get_eq_char (WEAR_FINGER_R) == NULL) {
      act ("$n wears $p on $s right finger.", obj, NULL, TO_ROOM);
      act ("You wear $p on your right finger.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_FINGER_R);
      return;
    }

    bug_printf ("Wear_obj: no free finger.");
    send_to_char ("You already wear two rings.\r\n");
    return;
  }

  if (obj->can_wear(ITEM_WEAR_NECK)) {
    if (get_eq_char (WEAR_NECK_1) != NULL
      && get_eq_char (WEAR_NECK_2) != NULL
      && !remove_obj (WEAR_NECK_1, fReplace)
      && !remove_obj (WEAR_NECK_2, fReplace))
      return;

    if (get_eq_char (WEAR_NECK_1) == NULL) {
      act ("$n wears $p around $s neck.", obj, NULL, TO_ROOM);
      act ("You wear $p around your neck.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_NECK_1);
      return;
    }

    if (get_eq_char (WEAR_NECK_2) == NULL) {
      act ("$n wears $p around $s neck.", obj, NULL, TO_ROOM);
      act ("You wear $p around your neck.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_NECK_2);
      return;
    }

    bug_printf ("Wear_obj: no free neck.");
    send_to_char ("You already wear two neck items.\r\n");
    return;
  }

  if (obj->can_wear(ITEM_WEAR_BODY)) {
    if (!remove_obj (WEAR_BODY, fReplace))
      return;
    act ("$n wears $p on $s body.", obj, NULL, TO_ROOM);
    act ("You wear $p on your body.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_BODY);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_HEAD)) {
    if (!remove_obj (WEAR_HEAD, fReplace))
      return;
    act ("$n wears $p on $s head.", obj, NULL, TO_ROOM);
    act ("You wear $p on your head.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_HEAD);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_LEGS)) {
    if (!remove_obj (WEAR_LEGS, fReplace))
      return;
    act ("$n wears $p on $s legs.", obj, NULL, TO_ROOM);
    act ("You wear $p on your legs.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_LEGS);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_FEET)) {
    if (!remove_obj (WEAR_FEET, fReplace))
      return;
    act ("$n wears $p on $s feet.", obj, NULL, TO_ROOM);
    act ("You wear $p on your feet.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_FEET);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_HANDS)) {
    if (!remove_obj (WEAR_HANDS, fReplace))
      return;
    act ("$n wears $p on $s hands.", obj, NULL, TO_ROOM);
    act ("You wear $p on your hands.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_HANDS);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_ARMS)) {
    if (!remove_obj (WEAR_ARMS, fReplace))
      return;
    act ("$n wears $p on $s arms.", obj, NULL, TO_ROOM);
    act ("You wear $p on your arms.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_ARMS);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_ABOUT)) {
    if (!remove_obj (WEAR_ABOUT, fReplace))
      return;
    act ("$n wears $p about $s body.", obj, NULL, TO_ROOM);
    act ("You wear $p about your body.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_ABOUT);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_WAIST)) {
    if (!remove_obj (WEAR_WAIST, fReplace))
      return;
    act ("$n wears $p about $s waist.", obj, NULL, TO_ROOM);
    act ("You wear $p about your waist.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_WAIST);
    return;
  }

  if (obj->can_wear(ITEM_WEAR_WRIST)) {
    if (get_eq_char (WEAR_WRIST_L) != NULL
      && get_eq_char (WEAR_WRIST_R) != NULL
      && !remove_obj (WEAR_WRIST_L, fReplace)
      && !remove_obj (WEAR_WRIST_R, fReplace))
      return;

    if (get_eq_char (WEAR_WRIST_L) == NULL) {
      act ("$n wears $p around $s left wrist.", obj, NULL, TO_ROOM);
      act ("You wear $p around your left wrist.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_WRIST_L);
      return;
    }

    if (get_eq_char (WEAR_WRIST_R) == NULL) {
      act ("$n wears $p around $s right wrist.", obj, NULL, TO_ROOM);
      act ("You wear $p around your right wrist.", obj, NULL, TO_CHAR);
      equip_char (obj, WEAR_WRIST_R);
      return;
    }

    bug_printf ("Wear_obj: no free wrist.");
    send_to_char ("You already wear two wrist items.\r\n");
    return;
  }

  if (obj->can_wear( ITEM_WEAR_SHIELD)) {
    if (!remove_obj (WEAR_SHIELD, fReplace))
      return;
    act ("$n wears $p as a shield.", obj, NULL, TO_ROOM);
    act ("You wear $p as a shield.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_SHIELD);
    return;
  }

  if (obj->can_wear(ITEM_WIELD)) {
    if (!remove_obj (WEAR_WIELD, fReplace))
      return;

    if (obj->get_obj_weight() > str_app[get_curr_str()].wield) {
      send_to_char ("It is too heavy for you to wield.\r\n");
      return;
    }

    act ("$n wields $p.", obj, NULL, TO_ROOM);
    act ("You wield $p.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_WIELD);
    return;
  }

  if (obj->can_wear(ITEM_HOLD)) {
    if (!remove_obj (WEAR_HOLD, fReplace))
      return;
    act ("$n holds $p in $s hands.", obj, NULL, TO_ROOM);
    act ("You hold $p in your hands.", obj, NULL, TO_CHAR);
    equip_char (obj, WEAR_HOLD);
    return;
  }

  if (fReplace)
    send_to_char ("You can't wear, wield, or hold that.\r\n");

  return;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void Character::show_list_to_char (std::list<Object *> & list, bool fShort,
  bool fShowNothing)
{
  char buf[MAX_STRING_LENGTH];
  int nShow;
  int iShow;
  int count;
  bool fCombine;

  if (desc == NULL)
    return;

  /*
   * Alloc space for output lines.
   */
  std::string * prgpstrShow = new std::string[list.size()];
  int * prgnShow = new int[list.size()];
  nShow = 0;

  /*
   * Format the list of objects.
   */
  ObjIter obj;
  for (obj = list.begin(); obj != list.end(); obj++) {
    if ((*obj)->wear_loc == WEAR_NONE && can_see_obj(*obj)) {
      std::string pstrShow = (*obj)->format_obj_to_char (this, fShort);
      fCombine = false;

      if (is_npc () || IS_SET (actflags, PLR_COMBINE)) {
        /*
         * Look for duplicates, case sensitive.
         * Matches tend to be near end so run loop backwords.
         */
        for (iShow = nShow - 1; iShow >= 0; iShow--) {
          if (!strcmp (prgpstrShow[iShow].c_str(), pstrShow.c_str())) {
            prgnShow[iShow]++;
            fCombine = true;
            break;
          }
        }
      }

      /*
       * Couldn't combine, or didn't want to.
       */
      if (!fCombine) {
        prgpstrShow[nShow] = pstrShow;
        prgnShow[nShow] = 1;
        nShow++;
      }
    }
  }

  /*
   * Output the formatted list.
   */
  for (iShow = 0; iShow < nShow; iShow++) {
    if (is_npc () || IS_SET (actflags, PLR_COMBINE)) {
      if (prgnShow[iShow] != 1) {
        snprintf (buf, sizeof buf, "(%2d) ", prgnShow[iShow]);
        send_to_char (buf);
      } else {
        send_to_char ("     ");
      }
    }
    send_to_char (prgpstrShow[iShow]);
    send_to_char ("\r\n");
  }

  if (fShowNothing && nShow == 0) {
    if (is_npc () || IS_SET (actflags, PLR_COMBINE))
      send_to_char ("     ");
    send_to_char ("Nothing.\r\n");
  }

  /*
   * Clean up.
   */
  delete [] prgnShow;
  delete [] prgpstrShow;

  return;
}

void Character::show_char_to_char_0 (Character * victim)
{
  std::string buf;

  if (victim->is_affected (AFF_INVISIBLE))
    buf.append("(Invis) ");
  if (victim->is_affected (AFF_HIDE))
    buf.append("(Hide) ");
  if (victim->is_affected (AFF_CHARM))
    buf.append("(Charmed) ");
  if (victim->is_affected (AFF_PASS_DOOR))
    buf.append("(Translucent) ");
  if (victim->is_affected (AFF_FAERIE_FIRE))
    buf.append("(Pink Aura) ");
  if (victim->is_evil () && is_affected (AFF_DETECT_EVIL))
    buf.append("(Red Aura) ");
  if (victim->is_affected (AFF_SANCTUARY))
    buf.append("(White Aura) ");
  if (!victim->is_npc () && IS_SET (victim->actflags, PLR_KILLER))
    buf.append("(KILLER) ");
  if (!victim->is_npc () && IS_SET (victim->actflags, PLR_THIEF))
    buf.append("(THIEF) ");

  if (victim->position == POS_STANDING && !victim->long_descr.empty()) {
    buf.append(victim->long_descr);
    send_to_char (buf);
    return;
  }

  buf.append(victim->describe_to(this));
  if (!victim->is_npc () && !IS_SET (actflags, PLR_BRIEF))
    buf.append(victim->pcdata->title);

  switch (victim->position) {
  case POS_DEAD:
    buf.append(" is DEAD!!");
    break;
  case POS_MORTAL:
    buf.append(" is mortally wounded.");
    break;
  case POS_INCAP:
    buf.append(" is incapacitated.");
    break;
  case POS_STUNNED:
    buf.append(" is lying here stunned.");
    break;
  case POS_SLEEPING:
    buf.append(" is sleeping here.");
    break;
  case POS_RESTING:
    buf.append(" is resting here.");
    break;
  case POS_STANDING:
    buf.append(" is here.");
    break;
  case POS_FIGHTING:
    buf.append(" is here, fighting ");
    if (victim->fighting == NULL)
      buf.append("thin air??");
    else if (victim->fighting == this)
      buf.append("YOU!");
    else if (victim->in_room == victim->fighting->in_room) {
      buf.append(victim->fighting->describe_to(this));
      buf.append(".");
    } else
      buf.append("somone who left??");
    break;
  }

  buf.append("\r\n");
  buf[0] = toupper(buf[0]);
  send_to_char (buf);
  return;
}

void Character::show_char_to_char_1 (Character * victim)
{
  std::string buf;
  Object *obj;
  int iWear;
  int percent;
  bool found;

  if (victim->can_see(this)) {
    act ("$n looks at you.", NULL, victim, TO_VICT);
    act ("$n looks at $N.", NULL, victim, TO_NOTVICT);
  }

  if (victim->description[0] != '\0') {
    send_to_char (victim->description);
  } else {
    act ("You see nothing special about $M.", NULL, victim, TO_CHAR);
  }

  if (victim->max_hit > 0)
    percent = (100 * victim->hit) / victim->max_hit;
  else
    percent = -1;

  buf = victim->describe_to(this);

  if (percent >= 100)
    buf.append(" is in perfect health.\r\n");
  else if (percent >= 90)
    buf.append(" is slightly scratched.\r\n");
  else if (percent >= 80)
    buf.append(" has a few bruises.\r\n");
  else if (percent >= 70)
    buf.append(" has some cuts.\r\n");
  else if (percent >= 60)
    buf.append(" has several wounds.\r\n");
  else if (percent >= 50)
    buf.append(" has many nasty wounds.\r\n");
  else if (percent >= 40)
    buf.append(" is bleeding freely.\r\n");
  else if (percent >= 30)
    buf.append(" is covered in blood.\r\n");
  else if (percent >= 20)
    buf.append(" is leaking guts.\r\n");
  else if (percent >= 10)
    buf.append(" is almost dead.\r\n");
  else
    buf.append(" is DYING.\r\n");

  buf[0] = toupper (buf[0]);
  send_to_char (buf);

  found = false;
  for (iWear = 0; iWear < MAX_WEAR; iWear++) {
    if ((obj = victim->get_eq_char (iWear)) != NULL && can_see_obj(obj)) {
      if (!found) {
        send_to_char ("\r\n");
        act ("$N is using:", NULL, victim, TO_CHAR);
        found = true;
      }
      send_to_char (where_name[iWear]);
      send_to_char (obj->format_obj_to_char (this, true));
      send_to_char ("\r\n");
    }
  }

  if (victim != this && !is_npc ()
    && number_percent () < pcdata->learned[skill_lookup("peek")]) {
    send_to_char ("\r\nYou peek at the inventory:\r\n");
    show_list_to_char (victim->carrying, true, true);
  }

  return;
}

void Character::show_char_to_char (std::list<Character *> & list)
{
  CharIter rch;

  for (rch = list.begin(); rch != list.end(); rch++) {
    if (*rch == this)
      continue;

    if (can_see(*rch)) {
      show_char_to_char_0 (*rch);
    } else if (in_room->is_dark()
      && (*rch)->is_affected (AFF_INFRARED)) {
      send_to_char ("You see glowing red eyes watching YOU!\r\n");
    }
  }

  return;
}

void Character::move_char (int door)
{
  Character *fch;
  Room *in_rm;
  Room *to_room;
  Exit *pexit;

  if (door < 0 || door > 5) {
    bug_printf ("Do_move: bad door %d.", door);
    return;
  }

  in_rm = in_room;
  if ((pexit = in_rm->exit[door]) == NULL
    || (to_room = pexit->to_room) == NULL) {
    send_to_char ("Alas, you cannot go that way.\r\n");
    return;
  }

  if (IS_SET (pexit->exit_info, EX_CLOSED)
    && !is_affected (AFF_PASS_DOOR)) {
    act ("The $d is closed.", NULL, pexit->name.c_str(), TO_CHAR);
    return;
  }

  if (is_affected (AFF_CHARM)
    && master != NULL && in_rm == master->in_room) {
    send_to_char ("What?  And leave your beloved master?\r\n");
    return;
  }

  if (to_room->is_private()) {
    send_to_char ("That room is private right now.\r\n");
    return;
  }

  if (!is_npc ()) {
    int iClass;
    int mv;

    for (iClass = 0; iClass < CLASS_MAX; iClass++) {
      if (iClass != klass && to_room->vnum == class_table[iClass].guild) {
        send_to_char ("You aren't allowed in there.\r\n");
        return;
      }
    }

    if (in_rm->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR) {
      if (!is_affected (AFF_FLYING)) {
        send_to_char ("You can't fly.\r\n");
        return;
      }
    }

    if (in_rm->sector_type == SECT_WATER_NOSWIM
      || to_room->sector_type == SECT_WATER_NOSWIM) {
      /*
       * Look for a boat.
       */
      bool found = false;

      /*
       * Suggestion for flying above water by Sludge
       */
      if (is_affected (AFF_FLYING))
        found = true;

      for (ObjIter o = carrying.begin(); o != carrying.end(); o++) {
        if ((*o)->item_type == ITEM_BOAT) {
          found = true;
          break;
        }
      }
      if (!found) {
        send_to_char ("You need a boat to go there.\r\n");
        return;
      }
    }

    mv = movement_loss[std::min ((int)SECT_MAX - 1, in_rm->sector_type)]
      + movement_loss[std::min ((int)SECT_MAX - 1, to_room->sector_type)];

    if (move < mv) {
      send_to_char ("You are too exhausted.\r\n");
      return;
    }

    wait_state (1);
    move -= mv;
  }

  if (!is_affected (AFF_SNEAK))
    act ("$n leaves $T.", NULL, dir_name[door].c_str(), TO_ROOM);

  char_from_room();
  char_to_room(to_room);
  if (!is_affected (AFF_SNEAK))
    act ("$n has arrived.", NULL, NULL, TO_ROOM);

  do_look ("auto");

  CharIter rch, next;
  for (rch = in_rm->people.begin(); rch != in_rm->people.end(); rch = next) {
    fch = *rch;
    next = ++rch;
    if (fch->master == this && fch->position == POS_STANDING) {
      fch->act ("You follow $N.", NULL, this, TO_CHAR);
      fch->move_char (door);
    }
  }

  if (this)
    mprog_entry_trigger (this);
  if (this)
    mprog_greet_trigger (this);
  return;
}

bool Character::check_social (const std::string & command, const std::string & argument)
{
  std::string arg;
  Character *victim;
  int cmd;
  char *sql = sqlite3_mprintf(
    "SELECT name, char_no_arg, others_no_arg, char_found, others_found, vict_found, char_auto, others_auto FROM socials WHERE NAME LIKE '%q%%'",
    command.c_str());
  sqlite3_stmt *stmt = NULL;

  if (sqlite3_prepare(g_db->database, sql, -1, &stmt, 0) != SQLITE_OK) {
    bug_printf("Could not prepare statement: '%s' Error: %s", sql, sqlite3_errmsg(g_db->database));
    sqlite3_free(sql);
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return false;
  }

  if (!is_npc () && IS_SET (actflags, PLR_NO_EMOTE)) {
    send_to_char ("You are anti-social!\r\n");
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return true;
  }

  switch (position) {
  case POS_DEAD:
    send_to_char ("Lie still; you are DEAD.\r\n");
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return true;

  case POS_INCAP:
  case POS_MORTAL:
    send_to_char ("You are hurt far too bad for that.\r\n");
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return true;

  case POS_STUNNED:
    send_to_char ("You are too stunned to do that.\r\n");
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return true;

  case POS_SLEEPING:
    /*
     * I just know this is the path to a 12" 'if' statement.  :(
     * But two players asked for it already!  -- Furey
     */
    if (!str_cmp ((const char*)sqlite3_column_text( stmt, 0 ), "snore"))
      break;
    send_to_char ("In your dreams, or what?\r\n");
    sqlite3_finalize(stmt);
    sqlite3_free(sql);
    return true;

  }

  one_argument (argument, arg);
  victim = NULL;
  if (arg.empty()) {
    act ((const char*)sqlite3_column_text( stmt, 2 ), NULL, victim, TO_ROOM);
    act ((const char*)sqlite3_column_text( stmt, 1 ), NULL, victim, TO_CHAR);
  } else if ((victim = get_char_room (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
  } else if (victim == this) {
    act ((const char*)sqlite3_column_text( stmt, 7 ), NULL, victim, TO_ROOM);
    act ((const char*)sqlite3_column_text( stmt, 6 ), NULL, victim, TO_CHAR);
  } else {
    act ((const char*)sqlite3_column_text( stmt, 4 ), NULL, victim, TO_NOTVICT);
    act ((const char*)sqlite3_column_text( stmt, 3 ), NULL, victim, TO_CHAR);
    act ((const char*)sqlite3_column_text( stmt, 5 ), NULL, victim, TO_VICT);

    if (!is_npc () && victim->is_npc ()
      && !victim->is_affected (AFF_CHARM)
      && victim->is_awake ()) {
      switch (number_range (0, 15)) {
      case 0:
        multi_hit (victim, this, TYPE_UNDEFINED);
        break;

      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
        victim->act ((const char*)sqlite3_column_text( stmt, 4 ), NULL, this, TO_NOTVICT);
        victim->act ((const char*)sqlite3_column_text( stmt, 3 ), NULL, this, TO_CHAR);
        victim->act ((const char*)sqlite3_column_text( stmt, 5 ), NULL, this, TO_VICT);
        break;

      case 9:
      case 10:
      case 11:
      case 12:
        victim->act ("$n slaps $N.", NULL, this, TO_NOTVICT);
        victim->act ("You slap $N.", NULL, this, TO_CHAR);
        victim->act ("$n slaps you.", NULL, this, TO_VICT);
        break;
      }
    }
  }

  sqlite3_finalize(stmt);
  sqlite3_free(sql);
  return true;
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void Character::interpret (std::string argument)
{
  std::string command;
  int cmd;
  bool found;

  if (desc != NULL)
    desc->incomm.erase();
  /*
   * Strip leading spaces.
   */
  argument.erase(0, argument.find_first_not_of(" "));
  if (argument.empty())
    return;

  /*
   * No hiding.
   */
  REMOVE_BIT (affected_by, AFF_HIDE);

  /*
   * Implement freeze command.
   */
  if (!is_npc () && IS_SET (actflags, PLR_FREEZE)) {
    send_to_char ("You're totally frozen!\r\n");
    return;
  }

  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  if (!isalpha (argument[0]) && !isdigit (argument[0])) {
    command.assign(argument, 0, 1);
    argument.erase(0, 1);
    argument.erase(0, argument.find_first_not_of(" "));
  } else {
    argument = one_argument(argument, command);
  }

  /*
   * Look for command in command table.
   */
  found = false;
  int trst = get_trust ();
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
    if (command[0] == cmd_table[cmd].name[0]
      && !str_prefix (command, cmd_table[cmd].name)
      && (cmd_table[cmd].level <= trst || mp_commands())) {
      found = true;
      break;
    }
  }

  if (!found) {
    /*
     * Look for command in socials table.
     */
    if (!check_social (command, argument))
      send_to_char ("Huh?\r\n");
    return;
  }

  /*
   * Character not in position for command?
   */
  if (position < cmd_table[cmd].position) {
    switch (position) {
    case POS_DEAD:
      send_to_char ("Lie still; you are DEAD.\r\n");
      break;

    case POS_MORTAL:
    case POS_INCAP:
      send_to_char ("You are hurt far too bad for that.\r\n");
      break;

    case POS_STUNNED:
      send_to_char ("You are too stunned to do that.\r\n");
      break;

    case POS_SLEEPING:
      send_to_char ("In your dreams, or what?\r\n");
      break;

    case POS_RESTING:
      send_to_char ("Nah... You feel too relaxed...\r\n");
      break;

    case POS_FIGHTING:
      send_to_char ("No way!  You are still fighting!\r\n");
      break;

    }
    return;
  }

  /*
   * Dispatch the command.
   */
  (this->*(cmd_table[cmd].do_fun)) (argument);

  return;
}

bool Character::is_gagged(std::string & nm) {
  if (is_npc ())
    return false;

  std::string gagname = capitalize(nm);
  std::list<std::string>::iterator fnd;
  fnd = find(pcdata->gag_list.begin(), pcdata->gag_list.end(), gagname);
  if (fnd != pcdata->gag_list.end())
    return true;
  return false;
}
