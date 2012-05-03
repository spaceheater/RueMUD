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

/*-----------------------------------------------------------------------*/
/* OS DEPENDENT INCLUDES AND DEFINITIONS                                 */
/*-----------------------------------------------------------------------*/
#include "os.hpp"

/*-----------------------------------------------------------------------*/
/* MurkMUD++ BEGINS HERE                                                 */
/*-----------------------------------------------------------------------*/

#include "config.hpp"
#include "utils.hpp"
#include "io.hpp"
#include "database.hpp"
#include "symbols.hpp"
#include "world.hpp"

/*
 * Globals.
 */
World *g_world = NULL;
Database *g_db = NULL;
Area *area_last = NULL;
std::list<Ban *> ban_list;
std::list<Character *> char_list;
std::list<Descriptor *> descriptor_list;       /* All open descriptors     */
std::list<Note *> note_list;
std::list<Object *> object_list;
std::list<Shop *> shop_list;

// These iterators used on loops where the next iterator can be invalidated
// because of a nested method that erases an object in the list.
CharIter deepchnext, deeprmnext;
ObjIter deepobnext;
DescIter deepdenext;
//bool character_invalidated = false;  // This is set in Mprogs if we

std::map<int, MobPrototype *> mob_table;
std::map<int, ObjectPrototype *> obj_table;
std::map<int, Room *> room_table;

struct kill_data kill_table[MAX_LEVEL];

/*
 * Global variables.
 */
bool merc_down;                 /* Shutdown         */
bool wizlock;                   /* Game is wizlocked        */
std::string str_boot_time;
std::string help_greeting;
short g_port;
SOCKET g_listen;
bool extract_chars;

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
std::string target_name;

std::ifstream * fpArea;
std::string strArea;

/*
 * Array of containers read for proper re-nesting of objects.
 */
Object *rgObjNest[MAX_NEST];

bool MOBtrigger;

std::string dir_name[] = {
  "north", "east", "south", "west", "up", "down"
};

sh_int rev_dir[] = {
  2, 3, 0, 1, 5, 4
};

std::string where_name[] = {
  "<used as light>     ",
  "<worn on finger>    ",
  "<worn on finger>    ",
  "<worn around neck>  ",
  "<worn around neck>  ",
  "<worn on body>      ",
  "<worn on head>      ",
  "<worn on legs>      ",
  "<worn on feet>      ",
  "<worn on hands>     ",
  "<worn on arms>      ",
  "<worn as shield>    ",
  "<worn about body>   ",
  "<worn about waist>  ",
  "<worn around wrist> ",
  "<worn around wrist> ",
  "<wielded>           ",
  "<held>              "
};

/*
 * Class table.
 */
struct class_type class_table[] = {
  { "Mag", APPLY_INT, OBJ_VNUM_SCHOOL_DAGGER,
    3018, 95, 18, 10, 6, 8, true},
  { "Cle", APPLY_WIS, OBJ_VNUM_SCHOOL_MACE,
    3003, 95, 18, 12, 7, 10, true},
  { "Thi", APPLY_DEX, OBJ_VNUM_SCHOOL_DAGGER,
    3028, 85, 18, 8, 8, 13, false},
  { "War", APPLY_STR, OBJ_VNUM_SCHOOL_SWORD,
    3022, 85, 18, 6, 11, 15, false}
};

/*
 * Attribute bonus structures.
 */
struct str_app_type str_app[26] = {
  {-5, -4, 0, 0}, {-5, -4, 3, 1}, {-3, -2, 3, 2}, {-3, -1, 10, 3},
  {-2, -1, 25, 4}, {-2, -1, 55, 5}, {-1, 0, 80, 6}, {-1, 0, 90, 7},
  {0, 0, 100, 8}, {0, 0, 100, 9}, {0, 0, 115, 10},  {0, 0, 115, 11},
  {0, 0, 140, 12}, {0, 0, 140, 13}, {0, 1, 170, 14}, {1, 1, 170, 15}, /* 15  */
  {1, 2, 195, 16}, {2, 3, 220, 22}, {2, 4, 250, 25}, {3, 5, 400, 30},
  {3, 6, 500, 35}, {4, 7, 600, 40}, {5, 7, 700, 45}, {6, 8, 800, 50},
  {8, 10, 900, 55}, {10, 12, 999, 60}      /* 25   */
};

struct int_app_type int_app[26] = {
  {3}, {5}, {7}, {8}, {9}, {10},      /*  5 */
  {11}, {12}, {13}, {15}, {17},       /* 10 */
  {19}, {22}, {25}, {28}, {31},       /* 15 */
  {34}, {37}, {40}, {44}, {49},       /* 20 */
  {55}, {60}, {70}, {85}, {99}        /* 25 */
};

struct wis_app_type wis_app[26] = {
  {0}, {0}, {0}, {0}, {0}, {1},       /*  5 */
  {1}, {1}, {1}, {2}, {2},            /* 10 */
  {2}, {2}, {2}, {2}, {3},            /* 15 */
  {3}, {4}, {4}, {5}, {5},            /* 20 */
  {6}, {7}, {7}, {7}, {8}             /* 25 */
};

struct dex_app_type dex_app[26] = {
  {60}, {50}, {50}, {40}, {30}, {20}, /* 5 */
  {10}, {0}, {0}, {0}, {0},           /* 10 */
  {0}, {0}, {0}, {0}, {-10},          /* 15 */
  {-15}, {-20}, {-30}, {-40}, {-50},  /* 20 */
  {-65}, {-75}, {-90}, {-105}, {-120} /* 25 */
};

struct con_app_type con_app[26] = {
  {-4, 20}, {-3, 25}, {-2, 30}, {-2, 35}, {-1, 40}, {-1, 45}, /*  5 */
  {-1, 50}, {0, 55}, {0, 60}, {0, 65}, {0, 70},               /* 10 */
  {0, 75}, {0, 80}, {0, 85}, {0, 88}, {1, 90},                /* 15 */
  {2, 95}, {2, 97}, {3, 99}, {3, 99}, {4, 99},                /* 20 */
  {4, 99}, {5, 99}, {6, 99}, {7, 99}, {8, 99}                 /* 25 */
};

/*
 * Liquid properties.
 */
struct liq_type liq_table[LIQ_MAX] = {
  {"water", "clear", {0, 1, 10}},       /*  0 */
  {"beer", "amber", {3, 2, 5}},
  {"wine", "rose", {5, 2, 5}},
  {"ale", "brown", {2, 2, 5}},
  {"dark ale", "dark", {1, 2, 5}},

  {"whisky", "golden", {6, 1, 4}},      /*  5 */
  {"lemonade", "pink", {0, 1, 8}},
  {"firebreather", "boiling", {10, 0, 0}},
  {"local specialty", "everclear", {3, 3, 3}},
  {"slime mold juice", "green", {0, 4, -8}},

  {"milk", "white", {0, 3, 6}}, /* 10 */
  {"tea", "tan", {0, 1, 6}},
  {"coffee", "black", {0, 1, 6}},
  {"blood", "red", {0, 2, -1}},
  {"salt water", "clear", {0, 1, -2}},

  {"cola", "cherry", {0, 1, 5}} /* 15 */
};

/* prototypes */
void multi_hit(Character *ch, Character *victim, int dt);
void damage(Character *ch, Character *victim, int dam, int dt);

bool dragon(Character *ch, const char *spell_name);

void mprog_act_trigger(const std::string & buf, Character *mob, Character *ch, Object *obj, void *vo);
void mprog_bribe_trigger(Character *mob, Character *ch, int amount);
void mprog_death_trigger(Character *mob);
void mprog_entry_trigger(Character *mob);
void mprog_fight_trigger(Character *mob, Character *ch);
void mprog_give_trigger(Character *mob, Character *ch, Object *obj);
void mprog_greet_trigger(Character *mob);
void mprog_hitprcnt_trigger(Character *mob, Character *ch);
void mprog_random_trigger(Character *mob);
void mprog_speech_trigger(char *txt, Character *mob);

bool spec_breath_any(Character *ch);
bool spec_breath_acid(Character *ch);
bool spec_breath_fire(Character *ch);
bool spec_breath_frost(Character *ch);
bool spec_breath_gas(Character *ch);
bool spec_breath_lightning(Character *ch);
bool spec_cast_adept(Character *ch);
bool spec_cast_cleric(Character *ch);
bool spec_cast_judge(Character *ch);
bool spec_cast_mage(Character *ch);
bool spec_cast_undead(Character *ch);
bool spec_executioner(Character *ch);
bool spec_fido(Character *ch);
bool spec_guard(Character *ch);
bool spec_janitor(Character *ch);
bool spec_mayor(Character *ch);
bool spec_poison(Character *ch);
bool spec_thief(Character *ch);

/* Structs */
/*
 * MOBprogram block
*/
class MobProgramActList {
public:
  MobProgramActList *next;
  std::string buf;
  Character *ch;
  Object *obj;
  void *vo;

  MobProgramActList() :
    next(NULL), ch(NULL), obj(NULL), vo(NULL) {
  }

};

class MobProgram {
public:
  MobProgram *next;
  int type;
  std::string arglist;
  std::string comlist;

  MobProgram() :
    next(NULL), type(0) {
  }

};

#include "shop.hpp"
#include "note.hpp"
#include "affect.hpp"
#include "ban.hpp"
#include "exit.hpp"
#include "reset.hpp"
#include "area.hpp"
#include "room.hpp"
#include "objproto.hpp"
#include "object.hpp"
#include "mobproto.hpp"
#include "descriptor.hpp"
#include "pcdata.hpp"
#include "extra.hpp"

struct cmd_type cmd_table[] = {
  /*
   * Common movement commands.
   */
  {"north", &Character::do_north, POS_STANDING, 0},
  {"east", &Character::do_east, POS_STANDING, 0},
  {"south", &Character::do_south, POS_STANDING, 0},
  {"west", &Character::do_west, POS_STANDING, 0},
  {"up", &Character::do_up, POS_STANDING, 0},
  {"down", &Character::do_down, POS_STANDING, 0},

  /*
   * Common other commands.
   * Placed here so one and two letter abbreviations work.
   */
  {"buy", &Character::do_buy, POS_RESTING, 0},
  {"cast", &Character::do_cast, POS_FIGHTING, 0},
  {"exits", &Character::do_exits, POS_RESTING, 0},
  {"get", &Character::do_get, POS_RESTING, 0},
  {"inventory", &Character::do_inventory, POS_DEAD, 0},
  {"kill", &Character::do_kill, POS_FIGHTING, 0},
  {"look", &Character::do_look, POS_RESTING, 0},
  {"order", &Character::do_order, POS_RESTING, 0},
  {"rest", &Character::do_rest, POS_RESTING, 0},
  {"sleep", &Character::do_sleep, POS_SLEEPING, 0},
  {"stand", &Character::do_stand, POS_SLEEPING, 0},
  {"tell", &Character::do_tell, POS_RESTING, 0},
  {"wield", &Character::do_wear, POS_RESTING, 0},
  {"wizhelp", &Character::do_wizhelp, POS_DEAD, L_HER},

  /*
   * Informational commands.
   */
  {"areas", &Character::do_areas, POS_DEAD, 0},
  {"bug", &Character::do_bug, POS_DEAD, 0},
  {"commands", &Character::do_commands, POS_DEAD, 0},
  {"compare", &Character::do_compare, POS_RESTING, 0},
  {"consider", &Character::do_consider, POS_RESTING, 0},
  {"credits", &Character::do_credits, POS_DEAD, 0},
  {"equipment", &Character::do_equipment, POS_DEAD, 0},
  {"examine", &Character::do_examine, POS_RESTING, 0},
  {"help", &Character::do_help, POS_DEAD, 0},
  {"idea", &Character::do_idea, POS_DEAD, 0},
  {"report", &Character::do_report, POS_DEAD, 0},
  {"pagelength", &Character::do_pagelen, POS_DEAD, 0},
  {"score", &Character::do_score, POS_DEAD, 0},
  {"slist", &Character::do_slist, POS_DEAD, 0},
  {"socials", &Character::do_socials, POS_DEAD, 0},
  {"time", &Character::do_time, POS_DEAD, 0},
  {"typo", &Character::do_typo, POS_DEAD, 0},
  {"weather", &Character::do_weather, POS_RESTING, 0},
  {"who", &Character::do_who, POS_DEAD, 0},
  {"wizlist", &Character::do_wizlist, POS_DEAD, 0},

  /*
   * Configuration commands.
   */
  {"auto", &Character::do_auto, POS_DEAD, 0},
  {"autoexit", &Character::do_autoexit, POS_DEAD, 0},
  {"autoloot", &Character::do_autoloot, POS_DEAD, 0},
  {"autosac", &Character::do_autosac, POS_DEAD, 0},
  {"blank", &Character::do_blank, POS_DEAD, 0},
  {"brief", &Character::do_brief, POS_DEAD, 0},
  {"channels", &Character::do_channels, POS_DEAD, 0},
  {"combine", &Character::do_combine, POS_DEAD, 0},
  {"config", &Character::do_config, POS_DEAD, 0},
  {"description", &Character::do_description, POS_DEAD, 0},
  {"password", &Character::do_password, POS_DEAD, 0},
  {"prompt", &Character::do_prompt, POS_DEAD, 0},
  {"title", &Character::do_title, POS_DEAD, 0},
  {"wimpy", &Character::do_wimpy, POS_DEAD, 0},

  /*
   * Communication commands.
   */
  {"answer", &Character::do_answer, POS_SLEEPING, 0},
  {"auction", &Character::do_auction, POS_SLEEPING, 0},
  {"chat", &Character::do_chat, POS_SLEEPING, 0},
  {".", &Character::do_chat, POS_SLEEPING, 0},
  {"emote", &Character::do_emote, POS_RESTING, 0},
  {",", &Character::do_emote, POS_RESTING, 0},
  {"gtell", &Character::do_gtell, POS_DEAD, 0},
  {";", &Character::do_gtell, POS_DEAD, 0},
  {"music", &Character::do_music, POS_SLEEPING, 0},
  {"note", &Character::do_note, POS_SLEEPING, 0},
  {"question", &Character::do_question, POS_SLEEPING, 0},
  {"reply", &Character::do_reply, POS_RESTING, 0},
  {"say", &Character::do_say, POS_RESTING, 0},
  {"'", &Character::do_say, POS_RESTING, 0},
  {"shout", &Character::do_shout, POS_RESTING, 3},
  {"yell", &Character::do_yell, POS_RESTING, 0},
  {"gag", &Character::do_gag, POS_DEAD, 0},
  {"ignore", &Character::do_gag, POS_DEAD, 0},

  /*
   * Object manipulation commands.
   */
  {"brandish", &Character::do_brandish, POS_RESTING, 0},
  {"close", &Character::do_close, POS_RESTING, 0},
  {"drink", &Character::do_drink, POS_RESTING, 0},
  {"drop", &Character::do_drop, POS_RESTING, 0},
  {"eat", &Character::do_eat, POS_RESTING, 0},
  {"fill", &Character::do_fill, POS_RESTING, 0},
  {"give", &Character::do_give, POS_RESTING, 0},
  {"hold", &Character::do_wear, POS_RESTING, 0},
  {"list", &Character::do_list, POS_RESTING, 0},
  {"lock", &Character::do_lock, POS_RESTING, 0},
  {"open", &Character::do_open, POS_RESTING, 0},
  {"pick", &Character::do_pick, POS_RESTING, 0},
  {"put", &Character::do_put, POS_RESTING, 0},
  {"quaff", &Character::do_quaff, POS_RESTING, 0},
  {"recite", &Character::do_recite, POS_RESTING, 0},
  {"remove", &Character::do_remove, POS_RESTING, 0},
  {"sell", &Character::do_sell, POS_RESTING, 0},
  {"take", &Character::do_get, POS_RESTING, 0},
  {"sacrifice", &Character::do_sacrifice, POS_RESTING, 0},
  {"unlock", &Character::do_unlock, POS_RESTING, 0},
  {"value", &Character::do_value, POS_RESTING, 0},
  {"wear", &Character::do_wear, POS_RESTING, 0},
  {"zap", &Character::do_zap, POS_RESTING, 0},

  /*
   * Combat commands.
   */
  {"backstab", &Character::do_backstab, POS_STANDING, 0},
  {"bs", &Character::do_backstab, POS_STANDING, 0},
  {"disarm", &Character::do_disarm, POS_FIGHTING, 0},
  {"flee", &Character::do_flee, POS_FIGHTING, 0},
  {"kick", &Character::do_kick, POS_FIGHTING, 0},
  {"murde", &Character::do_murde, POS_FIGHTING, 5},
  {"murder", &Character::do_murder, POS_FIGHTING, 5},
  {"rescue", &Character::do_rescue, POS_FIGHTING, 0},

  /*
   * Miscellaneous commands.
   */
  {"follow", &Character::do_follow, POS_RESTING, 0},
  {"group", &Character::do_group, POS_SLEEPING, 0},
  {"hide", &Character::do_hide, POS_RESTING, 0},
  {"practice", &Character::do_practice, POS_SLEEPING, 0},
  {"qui", &Character::do_qui, POS_DEAD, 0},
  {"quit", &Character::do_quit, POS_DEAD, 0},
  {"recall", &Character::do_recall, POS_FIGHTING, 0},
  {"/", &Character::do_recall, POS_FIGHTING, 0},
  {"rent", &Character::do_rent, POS_DEAD, 0},
  {"save", &Character::do_save, POS_DEAD, 0},
  {"sleep", &Character::do_sleep, POS_SLEEPING, 0},
  {"sneak", &Character::do_sneak, POS_STANDING, 0},
  {"spells", &Character::do_spells, POS_SLEEPING, 0},
  {"split", &Character::do_split, POS_RESTING, 0},
  {"steal", &Character::do_steal, POS_STANDING, 0},
  {"train", &Character::do_train, POS_RESTING, 0},
  {"visible", &Character::do_visible, POS_SLEEPING, 0},
  {"wake", &Character::do_wake, POS_SLEEPING, 0},
  {"where", &Character::do_where, POS_RESTING, 0},

  /*
   * Immortal commands.
   */
  {"advance", &Character::do_advance, POS_DEAD, L_GOD},
  {"trust", &Character::do_trust, POS_DEAD, L_GOD},

  {"allow", &Character::do_allow, POS_DEAD, L_SUP},
  {"ban", &Character::do_ban, POS_DEAD, L_SUP},
  {"deny", &Character::do_deny, POS_DEAD, L_SUP},
  {"disconnect", &Character::do_disconnect, POS_DEAD, L_SUP},
  {"freeze", &Character::do_freeze, POS_DEAD, L_SUP},
  {"hotboo", &Character::do_hotboo, POS_DEAD, L_SUP},
  {"hotboot", &Character::do_hotboot, POS_DEAD, L_SUP},
  {"shutdow", &Character::do_shutdow, POS_DEAD, L_SUP},
  {"shutdown", &Character::do_shutdown, POS_DEAD, L_SUP},
  {"users", &Character::do_users, POS_DEAD, L_SUP},
  {"wizify", &Character::do_wizify, POS_DEAD, L_SUP},
  {"wizlock", &Character::do_wizlock, POS_DEAD, L_SUP},

  {"force", &Character::do_force, POS_DEAD, L_DEI},
  {"mload", &Character::do_mload, POS_DEAD, L_DEI},
  {"mset", &Character::do_mset, POS_DEAD, L_DEI},
  {"noemote", &Character::do_noemote, POS_DEAD, L_DEI},
  {"notell", &Character::do_notell, POS_DEAD, L_DEI},
  {"oload", &Character::do_oload, POS_DEAD, L_DEI},
  {"oset", &Character::do_oset, POS_DEAD, L_DEI},
  {"owhere", &Character::do_owhere, POS_DEAD, L_DEI},
  {"pardon", &Character::do_pardon, POS_DEAD, L_DEI},
  {"peace", &Character::do_peace, POS_DEAD, L_DEI},
  {"purge", &Character::do_purge, POS_DEAD, L_DEI},
  {"restore", &Character::do_restore, POS_DEAD, L_DEI},
  {"rset", &Character::do_rset, POS_DEAD, L_DEI},
  {"silence", &Character::do_silence, POS_DEAD, L_DEI},
  {"sla", &Character::do_sla, POS_DEAD, L_DEI},
  {"slay", &Character::do_slay, POS_DEAD, L_DEI},
  {"sset", &Character::do_sset, POS_DEAD, L_DEI},
  {"transfer", &Character::do_transfer, POS_DEAD, L_DEI},
  {"mpstat", &Character::do_mpstat, POS_DEAD, L_DEI},

  {"at", &Character::do_at, POS_DEAD, L_ANG},
  {"bamfin", &Character::do_bamfin, POS_DEAD, L_ANG},
  {"bamfout", &Character::do_bamfout, POS_DEAD, L_ANG},
  {"echo", &Character::do_echo, POS_DEAD, L_ANG},
  {"goto", &Character::do_goto, POS_DEAD, L_ANG},
  {"holylight", &Character::do_holylight, POS_DEAD, L_ANG},
  {"memory", &Character::do_memory, POS_DEAD, L_ANG},
  {"mfind", &Character::do_mfind, POS_DEAD, L_ANG},
  {"mstat", &Character::do_mstat, POS_DEAD, L_ANG},
  {"mwhere", &Character::do_mwhere, POS_DEAD, L_ANG},
  {"ofind", &Character::do_ofind, POS_DEAD, L_ANG},
  {"ostat", &Character::do_ostat, POS_DEAD, L_ANG},
  {"recho", &Character::do_recho, POS_DEAD, L_ANG},
  {"return", &Character::do_return, POS_DEAD, L_ANG},
  {"rstat", &Character::do_rstat, POS_DEAD, L_ANG},
  {"slookup", &Character::do_slookup, POS_DEAD, L_ANG},
  {"switch", &Character::do_switch, POS_DEAD, L_ANG},

  {"immtalk", &Character::do_immtalk, POS_DEAD, L_ANG},
  {":", &Character::do_immtalk, POS_DEAD, L_ANG},

  /*
   * MOBprogram commands.
   */
  {"mpasound", &Character::do_mpasound, POS_DEAD, 41},
  {"mpjunk", &Character::do_mpjunk, POS_DEAD, 41},
  {"mpecho", &Character::do_mpecho, POS_DEAD, 41},
  {"mpechoat", &Character::do_mpechoat, POS_DEAD, 41},
  {"mpechoaround", &Character::do_mpechoaround, POS_DEAD, 41},
  {"mpkill", &Character::do_mpkill, POS_DEAD, 41},
  {"mpmload", &Character::do_mpmload, POS_DEAD, 41},
  {"mpoload", &Character::do_mpoload, POS_DEAD, 41},
  {"mppurge", &Character::do_mppurge, POS_DEAD, 41},
  {"mpgoto", &Character::do_mpgoto, POS_DEAD, 41},
  {"mpat", &Character::do_mpat, POS_DEAD, 41},
  {"mptransfer", &Character::do_mptransfer, POS_DEAD, 41},
  {"mpforce", &Character::do_mpforce, POS_DEAD, 41},

  /*
   * End of list.
   */
  {"", 0, POS_DEAD, 0}
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 * Skills include spells as a particular case.
 */
struct skill_type skill_table[MAX_SKILL] = {

/*
 * Magic spells.
 */

  {   "reserved", {99, 99, 99, 99},
      NULL, TAR_IGNORE, POS_STANDING,
      0, 0, "", ""},
  {   "acid blast", {20, 37, 37, 37},
      &Character::spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      20, 12, "acid blast", "!Acid Blast!"},
  {   "armor", {5, 1, 37, 37},
      &Character::spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 12, "", "You feel less protected."},
  {   "bless", {37, 5, 37, 37},
      &Character::spell_bless, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 12, "", "You feel less righteous."},
  {   "blindness", {8, 5, 37, 37},
      &Character::spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      5, 12, "", "You can see again."},
  {   "burning hands", {5, 37, 37, 37},
      &Character::spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "burning hands", "!Burning Hands!"},
  {   "call lightning", {37, 12, 37, 37},
      &Character::spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
      15, 12, "lightning bolt", "!Call Lightning!"},
  {   "cause critical", {37, 9, 37, 37},
      &Character::spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      20, 12, "spell", "!Cause Critical!"},
  {   "cause light", {37, 1, 37, 37},
      &Character::spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "spell", "!Cause Light!"},
  {   "cause serious", {37, 5, 37, 37},
      &Character::spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      17, 12, "spell", "!Cause Serious!"},
  {   "change sex", {37, 37, 37, 37},
      &Character::spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      15, 12, "", "Your body feels familiar again."},
  {   "charm person", {14, 37, 37, 37},
      &Character::spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING,
      5, 12, "", "You feel more self-confident."},
  {   "chill touch", {3, 37, 37, 37},
      &Character::spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "chilling touch", "You feel less cold."},
  {   "colour spray", {11, 37, 37, 37},
      &Character::spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "colour spray", "!Colour Spray!"},
  {   "continual light", {4, 2, 37, 37},
      &Character::spell_continual_light, TAR_IGNORE, POS_STANDING,
      7, 12, "", "!Continual Light!"},
  {   "control weather", {10, 13, 37, 37},
      &Character::spell_control_weather, TAR_IGNORE, POS_STANDING,
      25, 12, "", "!Control Weather!"},
  {   "create food", {37, 3, 37, 37},
      &Character::spell_create_food, TAR_IGNORE, POS_STANDING,
      5, 12, "", "!Create Food!"},
  {   "create spring", {10, 37, 37, 37},
      &Character::spell_create_spring, TAR_IGNORE, POS_STANDING,
      20, 12, "", "!Create Spring!"},
  {   "create water", {37, 2, 37, 37},
      &Character::spell_create_water, TAR_OBJ_INV, POS_STANDING,
      5, 12, "", "!Create Water!"},
  {   "cure blindness", {37, 4, 37, 37},
      &Character::spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      5, 12, "", "!Cure Blindness!"},
  {   "cure critical", {37, 9, 37, 37},
      &Character::spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      20, 12, "", "!Cure Critical!"},
  {   "cure light", {37, 1, 37, 37},
      &Character::spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      10, 12, "", "!Cure Light!"},
  {   "cure poison", {37, 9, 37, 37},
      &Character::spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 12, "", "!Cure Poison!"},
  {   "cure serious", {37, 5, 37, 37},
      &Character::spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      15, 12, "", "!Cure Serious!"},
  {   "curse", {12, 12, 37, 37},
      &Character::spell_curse, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      20, 12, "curse", "The curse wears off."},
  {   "detect evil", {37, 4, 37, 37},
      &Character::spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
      5, 12, "", "The red in your vision disappears."},
  {   "detect hidden", {37, 7, 37, 37},
      &Character::spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
      5, 12, "", "You feel less aware of your suroundings."},
  {   "detect invis", {2, 5, 37, 37},
      &Character::spell_detect_invis, TAR_CHAR_SELF, POS_STANDING,
      5, 12, "", "You no longer see invisible objects."},
  {   "detect magic", {2, 3, 37, 37},
      &Character::spell_detect_magic, TAR_CHAR_SELF, POS_STANDING,
      5, 12, "", "The detect magic wears off."},
  {   "detect poison", {37, 5, 37, 37},
      &Character::spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
      5, 12, "", "!Detect Poison!"},
  {   "dispel evil", {37, 10, 37, 37},
      &Character::spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "dispel evil", "!Dispel Evil!"},
  {   "dispel magic", {26, 31, 37, 37},
      &Character::spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_STANDING,
      15, 12, "", "!Dispel Magic!"},
  {   "earthquake", {37, 7, 37, 37},
      &Character::spell_earthquake, TAR_IGNORE, POS_FIGHTING,
      15, 12, "earthquake", "!Earthquake!"},
  {   "enchant weapon", {12, 37, 37, 37},
      &Character::spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
      100, 24, "", "!Enchant Weapon!"},
  {   "energy drain", {13, 37, 37, 37},
      &Character::spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      35, 12, "energy drain", "!Energy Drain!"},
  {   "faerie fire", {4, 2, 37, 37},
      &Character::spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      5, 12, "faerie fire", "The pink aura around you fades away."},
  {   "faerie fog", {10, 14, 37, 37},
      &Character::spell_faerie_fog, TAR_IGNORE, POS_STANDING,
      12, 12, "faerie fog", "!Faerie Fog!"},
  {   "fireball", {15, 37, 37, 37},
      &Character::spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "fireball", "!Fireball!"},
  {   "flamestrike", {37, 13, 37, 37},
      &Character::spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      20, 12, "flamestrike", "!Flamestrike!"},
  {   "fly", {7, 12, 37, 37},
      &Character::spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
      10, 18, "", "You slowly float to the ground."},
  {   "gate", {37, 37, 37, 37},
      &Character::spell_gate, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      50, 12, "", "!Gate!"},
  {   "giant strength", {7, 37, 37, 37},
      &Character::spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
      20, 12, "", "You feel weaker."},
  {   "harm", {37, 15, 37, 37},
      &Character::spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      35, 12, "harm spell", "!Harm!"},
  {   "heal", {37, 14, 37, 37},
      &Character::spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
      50, 12, "", "!Heal!"},
  {   "identify", {10, 10, 37, 37},
      &Character::spell_identify, TAR_OBJ_INV, POS_STANDING,
      12, 24, "", "!Identify!"},
  {   "infravision", {6, 9, 37, 37},
      &Character::spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 18, "", "You no longer see in the dark."},
  {   "invis", {4, 37, 37, 37},
      &Character::spell_invis, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 12, "", "You are no longer invisible."},
  {   "know alignment", {8, 5, 37, 37},
      &Character::spell_know_alignment, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      9, 12, "", "!Know Alignment!"},
  {   "lightning bolt", {9, 37, 37, 37},
      &Character::spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "lightning bolt", "!Lightning Bolt!"},
  {   "locate object", {6, 10, 37, 37},
      &Character::spell_locate_object, TAR_IGNORE, POS_STANDING,
      20, 18, "", "!Locate Object!"},
  {   "magic missile", {1, 37, 37, 37},
      &Character::spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "magic missile", "!Magic Missile!"},
  {   "mass invis", {15, 17, 37, 37},
      &Character::spell_mass_invis, TAR_IGNORE, POS_STANDING,
      20, 24, "", "!Mass Invis!"},
  {   "pass door", {18, 37, 37, 37},
      &Character::spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
      20, 12, "", "You feel solid again."},
  {   "poison", {37, 8, 37, 37},
      &Character::spell_poison, TAR_CHAR_OFFENSIVE, POS_STANDING,
      10, 12, "poison", "You feel less sick."},
  {   "protection", {37, 6, 37, 37},
      &Character::spell_protection, TAR_CHAR_SELF, POS_STANDING,
      5, 12, "", "You feel less protected."},
  {   "refresh", {5, 3, 37, 37},
      &Character::spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
      12, 18, "refresh", "!Refresh!"},
  {   "remove curse", {37, 12, 37, 37},
      &Character::spell_remove_curse, TAR_CHAR_DEFENSIVE, POS_STANDING,
      5, 12, "", "!Remove Curse!"},
  {   "sanctuary", {37, 13, 37, 37},
      &Character::spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
      75, 12, "", "The white aura around your body fades."},
  {   "shield", {13, 37, 37, 37},
      &Character::spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
      12, 18, "", "Your force shield shimmers then fades away."},
  {   "shocking grasp", {7, 37, 37, 37},
      &Character::spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      15, 12, "shocking grasp", "!Shocking Grasp!"},
  {   "sleep", {14, 37, 37, 37},
      &Character::spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
      15, 12, "", "You feel less tired."},
  {   "stone skin", {17, 37, 37, 37},
      &Character::spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
      12, 18, "", "Your skin feels soft again."},
  {   "summon", {37, 8, 37, 37},
      &Character::spell_summon, TAR_IGNORE, POS_STANDING,
      50, 12, "", "!Summon!"},
  {   "teleport", {8, 37, 37, 37},
      &Character::spell_teleport, TAR_CHAR_SELF, POS_FIGHTING,
      35, 12, "", "!Teleport!"},
  {   "ventriloquate", {1, 37, 37, 37},
      &Character::spell_ventriloquate, TAR_IGNORE, POS_STANDING,
      5, 12, "", "!Ventriloquate!"},
  {   "weaken", {7, 37, 37, 37},
      &Character::spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      20, 12, "spell", "You feel stronger."},
  {   "word of recall", {37, 37, 37, 37},
      &Character::spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
      5, 12, "", "!Word of Recall!"},
/*
 * Dragon breath
 */
  {   "acid breath", {33, 37, 37, 37},
      &Character::spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 4, "blast of acid", "!Acid Breath!"},
  {   "fire breath", {34, 37, 37, 37},
      &Character::spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 4, "blast of flame", "!Fire Breath!"},
  {   "frost breath", {31, 37, 37, 37},
      &Character::spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 4, "blast of frost", "!Frost Breath!"},
  {   "gas breath", {35, 37, 37, 37},
      &Character::spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
      0, 4, "blast of gas", "!Gas Breath!"},
  {   "lightning breath", {32, 37, 37, 37},
      &Character::spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 4, "blast of lightning", "!Lightning Breath!"},
/*
 * Fighter and thief skills.
 */
  {   "backstab", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_STANDING,
      0, 24, "backstab", "!Backstab!"},
  {   "disarm", {37, 37, 10, 37},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 24, "", "!Disarm!"},
  {   "dodge", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 0, "", "!Dodge!"},
  {   "enhanced damage", {37, 37, 37, 1},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 0, "", "!Enhanced Damage!"},
  {   "hide", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_RESTING,
      0, 12, "", "!Hide!"},
  {   "kick", {37, 37, 37, 1},
      &Character::spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 8, "kick", "!Kick!"},
  {   "parry", {37, 37, 37, 1},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 0, "", "!Parry!"},
  {   "peek", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_STANDING,
      0, 0, "", "!Peek!"},
  {   "pick lock", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_STANDING,
      0, 12, "", "!Pick!"},
  {   "rescue", {37, 37, 37, 1},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 12, "", "!Rescue!"},
  {   "second attack", {37, 37, 1, 1},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 0, "", "!Second Attack!"},
  {   "sneak", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_STANDING,
      0, 12, "", NULL},
  {   "steal", {37, 37, 1, 37},
      &Character::spell_null, TAR_IGNORE, POS_STANDING,
      0, 24, "", "!Steal!"},
  {   "third attack", {37, 37, 37, 1},
      &Character::spell_null, TAR_IGNORE, POS_FIGHTING,
      0, 0, "", "!Third Attack!"},
/*
 *  Spells for mega1.are from Glop/Erkenbrand.
*/
  {   "general purpose", {37, 37, 37, 37},
      &Character::spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 12, "general purpose ammo", "!General Purpose Ammo!"},
  {   "high explosive", {37, 37, 37, 37},
      &Character::spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
      0, 12, "high explosive ammo", "!High Explosive Ammo!"}
};

///////////////////
// start of code //
///////////////////

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor (SOCKET desc, const char *txt, int length)
{
  int iStart;
  int nWrite;
  int nBlock;

  if (length <= 0)
    length = strlen (txt);

  for (iStart = 0; iStart < length; iStart += nWrite) {
    nBlock = std::min (length - iStart, 4096);
    if ((nWrite = send (desc, txt + iStart, nBlock, 0)) == SOCKET_ERROR) {
      std::perror ("Write_to_descriptor");
      return false;
    }
  }

  return true;
}

/*
 * Return ascii name of an affect location.
 */
std::string affect_loc_name (int location)
{
  switch (location) {
  case APPLY_NONE:
    return "none";
  case APPLY_STR:
    return "strength";
  case APPLY_DEX:
    return "dexterity";
  case APPLY_INT:
    return "intelligence";
  case APPLY_WIS:
    return "wisdom";
  case APPLY_CON:
    return "constitution";
  case APPLY_SEX:
    return "sex";
  case APPLY_CLASS:
    return "class";
  case APPLY_LEVEL:
    return "level";
  case APPLY_AGE:
    return "age";
  case APPLY_MANA:
    return "mana";
  case APPLY_HIT:
    return "hp";
  case APPLY_MOVE:
    return "moves";
  case APPLY_GOLD:
    return "gold";
  case APPLY_EXP:
    return "experience";
  case APPLY_AC:
    return "armor class";
  case APPLY_HITROLL:
    return "hit roll";
  case APPLY_DAMROLL:
    return "damage roll";
  case APPLY_SAVING_PARA:
    return "save vs paralysis";
  case APPLY_SAVING_ROD:
    return "save vs rod";
  case APPLY_SAVING_PETRI:
    return "save vs petrification";
  case APPLY_SAVING_BREATH:
    return "save vs breath";
  case APPLY_SAVING_SPELL:
    return "save vs spell";
  }

  bug_printf ("Affect_location_name: unknown location %d.", location);
  return "(unknown)";
}

/*
 * Return ascii name of an affect bit vector.
 */
std::string affect_bit_name (int vector)
{
  std::string buf;

  if (vector & AFF_BLIND)
    buf.append(" blind");
  if (vector & AFF_INVISIBLE)
    buf.append(" invisible");
  if (vector & AFF_DETECT_EVIL)
    buf.append(" detect_evil");
  if (vector & AFF_DETECT_INVIS)
    buf.append(" detect_invis");
  if (vector & AFF_DETECT_MAGIC)
    buf.append(" detect_magic");
  if (vector & AFF_DETECT_HIDDEN)
    buf.append(" detect_hidden");
  if (vector & AFF_SANCTUARY)
    buf.append(" sanctuary");
  if (vector & AFF_FAERIE_FIRE)
    buf.append(" faerie_fire");
  if (vector & AFF_INFRARED)
    buf.append(" infrared");
  if (vector & AFF_CURSE)
    buf.append(" curse");
  if (vector & AFF_POISON)
    buf.append(" poison");
  if (vector & AFF_PROTECT)
    buf.append(" protect");
  if (vector & AFF_SLEEP)
    buf.append(" sleep");
  if (vector & AFF_SNEAK)
    buf.append(" sneak");
  if (vector & AFF_HIDE)
    buf.append(" hide");
  if (vector & AFF_CHARM)
    buf.append(" charm");
  if (vector & AFF_FLYING)
    buf.append(" flying");
  if (vector & AFF_PASS_DOOR)
    buf.append(" pass_door");
  if (buf.empty())
    buf.append("none");
  else
    buf.erase(0,1);
  return buf;
}

/*
 * Return ascii name of extra flags vector.
 */
std::string extra_bit_name (int extra_flags)
{
  std::string buf;

  if (extra_flags & ITEM_GLOW)
    buf.append(" glow");
  if (extra_flags & ITEM_HUM)
    buf.append(" hum");
  if (extra_flags & ITEM_DARK)
    buf.append(" dark");
  if (extra_flags & ITEM_LOCK)
    buf.append(" lock");
  if (extra_flags & ITEM_EVIL)
    buf.append(" evil");
  if (extra_flags & ITEM_INVIS)
    buf.append(" invis");
  if (extra_flags & ITEM_MAGIC)
    buf.append(" magic");
  if (extra_flags & ITEM_NODROP)
    buf.append(" nodrop");
  if (extra_flags & ITEM_BLESS)
    buf.append(" bless");
  if (extra_flags & ITEM_ANTI_GOOD)
    buf.append(" anti-good");
  if (extra_flags & ITEM_ANTI_EVIL)
    buf.append(" anti-evil");
  if (extra_flags & ITEM_ANTI_NEUTRAL)
    buf.append(" anti-neutral");
  if (extra_flags & ITEM_NOREMOVE)
    buf.append(" noremove");
  if (extra_flags & ITEM_INVENTORY)
    buf.append(" inventory");
  if (buf.empty())
    buf.append("none");
  else
    buf.erase(0,1);
  return buf;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup (const std::string & name)
{
  int sn;

  for (sn = 0; sn < MAX_SKILL; sn++) {
    if (skill_table[sn].name == NULL)
      break;
    if (tolower(name[0]) == tolower(skill_table[sn].name[0])
      && !str_prefix (name, skill_table[sn].name))
      return sn;
  }

  return -1;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MobPrototype *get_mob_index (int vnum)
{
  std::map<int,MobPrototype*>::iterator pMobIndex;

  pMobIndex = mob_table.find(vnum);

  if (pMobIndex != mob_table.end())
      return (*pMobIndex).second;

  if (g_db->fBootDb) {
    fatal_printf ("Get_mob_index: bad vnum %d.", vnum);
  }

  return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
ObjectPrototype *get_obj_index (int vnum)
{
  std::map<int,ObjectPrototype*>::iterator pObjIndex;

  pObjIndex = obj_table.find(vnum);

  if (pObjIndex != obj_table.end())
      return (*pObjIndex).second;

  if (g_db->fBootDb) {
    fatal_printf ("Get_obj_index: bad vnum %d.", vnum);
  }

  return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
Room *get_room_index (int vnum)
{
  std::map<int,Room*>::iterator pRoomIndex;

  pRoomIndex = room_table.find(vnum);

  if (pRoomIndex != room_table.end())
      return (*pRoomIndex).second;

  if (g_db->fBootDb) {
    fatal_printf ("Get_room_index: bad vnum %d.", vnum);
  }

  return NULL;
}

std::string get_title (int klass, int level, int sex)
{
  sqlite3_stmt *stmt = NULL;
  char *sql = sqlite3_mprintf(
    "SELECT title FROM titles WHERE class = %d AND level = %d and sex = %d",
    klass, level, sex == SEX_FEMALE ? 1 : 0);
  std::string str("");
  Database* db = Database::instance();

  if (sqlite3_prepare(db->database, sql, -1, &stmt, 0) != SQLITE_OK) {
    bug_printf("Could not prepare statement: %s", sqlite3_errmsg(db->database));
    sqlite3_free(sql);
    return str;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    str.assign((const char*)sqlite3_column_text( stmt, 0 ));
  }

  sqlite3_finalize(stmt);
  sqlite3_free(sql);
  return str;
}

/*
 * Get an extra description from a list.
 */
std::string get_extra_descr (const std::string & name, std::list<ExtraDescription *> & ed)
{
  std::list<ExtraDescription *>::iterator e;
  for (e = ed.begin(); e != ed.end(); e++) {
    if (is_name (name, (*e)->keyword))
      return (*e)->description;
  }
  return "";
}

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup (const std::string & name)
{
  if (!str_cmp (name, "spec_breath_any"))
    return spec_breath_any;
  if (!str_cmp (name, "spec_breath_acid"))
    return spec_breath_acid;
  if (!str_cmp (name, "spec_breath_fire"))
    return spec_breath_fire;
  if (!str_cmp (name, "spec_breath_frost"))
    return spec_breath_frost;
  if (!str_cmp (name, "spec_breath_gas"))
    return spec_breath_gas;
  if (!str_cmp (name, "spec_breath_lightning"))
    return spec_breath_lightning;
  if (!str_cmp (name, "spec_cast_adept"))
    return spec_cast_adept;
  if (!str_cmp (name, "spec_cast_cleric"))
    return spec_cast_cleric;
  if (!str_cmp (name, "spec_cast_judge"))
    return spec_cast_judge;
  if (!str_cmp (name, "spec_cast_mage"))
    return spec_cast_mage;
  if (!str_cmp (name, "spec_cast_undead"))
    return spec_cast_undead;
  if (!str_cmp (name, "spec_executioner"))
    return spec_executioner;
  if (!str_cmp (name, "spec_fido"))
    return spec_fido;
  if (!str_cmp (name, "spec_guard"))
    return spec_guard;
  if (!str_cmp (name, "spec_janitor"))
    return spec_janitor;
  if (!str_cmp (name, "spec_mayor"))
    return spec_mayor;
  if (!str_cmp (name, "spec_poison"))
    return spec_poison;
  if (!str_cmp (name, "spec_thief"))
    return spec_thief;
  return 0;
}

/*
 * MOBprogram code block
*/
/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
int mprog_name_to_type (const std::string & name)
{
  if (!str_cmp (name, "in_file_prog"))
    return IN_FILE_PROG;
  if (!str_cmp (name, "act_prog"))
    return ACT_PROG;
  if (!str_cmp (name, "speech_prog"))
    return SPEECH_PROG;
  if (!str_cmp (name, "rand_prog"))
    return RAND_PROG;
  if (!str_cmp (name, "fight_prog"))
    return FIGHT_PROG;
  if (!str_cmp (name, "hitprcnt_prog"))
    return HITPRCNT_PROG;
  if (!str_cmp (name, "death_prog"))
    return DEATH_PROG;
  if (!str_cmp (name, "entry_prog"))
    return ENTRY_PROG;
  if (!str_cmp (name, "greet_prog"))
    return GREET_PROG;
  if (!str_cmp (name, "all_greet_prog"))
    return ALL_GREET_PROG;
  if (!str_cmp (name, "give_prog"))
    return GIVE_PROG;
  if (!str_cmp (name, "bribe_prog"))
    return BRIBE_PROG;
  return (ERROR_PROG);
}

/* This routine reads in scripts of MOBprograms from a file */
MobProgram *mprog_file_read (const std::string & f, MobProgram *mprg, MobPrototype *pMobIndex)
{
  MobProgram *mprg2;
  std::ifstream progfile;
  bool done = false;
  char MOBProgfile[MAX_INPUT_LENGTH];

  snprintf (MOBProgfile, sizeof MOBProgfile, "%s%s", MOB_DIR, f.c_str());
  progfile.open (MOBProgfile, std::ifstream::in | std::ifstream::binary);
  if (!progfile.is_open()) {
    fatal_printf ("Mob:%d couldnt open mobprog file", pMobIndex->vnum);
  }
  mprg2 = mprg;
  switch (fread_letter (progfile)) {
  case '>':
    break;
  case '|':
    fatal_printf ("empty mobprog file.");
    break;
  default:
    fatal_printf ("in mobprog file syntax error.");
    break;
  }
  while (!done) {
    mprg2->type = mprog_name_to_type (fread_word (progfile));
    switch (mprg2->type) {
    case ERROR_PROG:
      fatal_printf ("mobprog file type error");
      break;
    case IN_FILE_PROG:
      fatal_printf ("mprog file contains a call to file.");
      break;
    default:
      pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
      mprg2->arglist = fread_string (progfile);
      mprg2->comlist = fread_string (progfile);
      switch (fread_letter (progfile)) {
      case '>':
        mprg2->next = new MobProgram();
        mprg2 = mprg2->next;
        mprg2->next = NULL;
        break;
      case '|':
        done = true;
        break;
      default:
        fatal_printf ("in mobprog file syntax error.");
        break;
      }
      break;
    }
  }
  progfile.close();
  return mprg2;
}

/* This procedure is responsible for reading any in_file MOBprograms.
 */
void mprog_read_programs (std::ifstream & fp, MobPrototype *pMobIndex)
{
  MobProgram *mprg;
  bool done = false;

  if ((fread_letter (fp)) != '>') {
    fatal_printf ("Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum);
  }
  pMobIndex->mobprogs = new MobProgram();
  mprg = pMobIndex->mobprogs;
  while (!done) {
    mprg->type = mprog_name_to_type (fread_word (fp));
    switch (mprg->type) {
    case ERROR_PROG:
      fatal_printf ("Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum);
      break;
    case IN_FILE_PROG:
      mprg = mprog_file_read (fread_string (fp), mprg, pMobIndex);
      fread_to_eol (fp);
      switch (fread_letter (fp)) {
      case '>':
        mprg->next = new MobProgram();
        mprg = mprg->next;
        mprg->next = NULL;
        break;
      case '|':
        mprg->next = NULL;
        fread_to_eol (fp);
        done = true;
        break;
      default:
        fatal_printf ("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
        break;
      }
      break;
    default:
      pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
      mprg->arglist = fread_string (fp);
      fread_to_eol (fp);
      mprg->comlist = fread_string (fp);
      fread_to_eol (fp);
      switch (fread_letter (fp)) {
      case '>':
        mprg->next = new MobProgram();
        mprg = mprg->next;
        mprg->next = NULL;
        break;
      case '|':
        mprg->next = NULL;
        fread_to_eol (fp);
        done = true;
        break;
      default:
        fatal_printf ("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
        break;
      }
      break;
    }
  }
}

/*
 * Create a 'money' obj.
 */
Object *create_money (int amount)
{
  char buf[MAX_STRING_LENGTH];
  Object *obj;

  if (amount <= 0) {
    bug_printf ("Create_money: zero or negative money %d.", amount);
    amount = 1;
  }

  if (amount == 1) {
    obj = get_obj_index(OBJ_VNUM_MONEY_ONE)->create_object(0);
  } else {
    obj = get_obj_index(OBJ_VNUM_MONEY_SOME)->create_object(0);
    snprintf (buf, sizeof buf, obj->short_descr.c_str(), amount);
    obj->short_descr = buf;
    obj->value[0] = amount;
  }

  return obj;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group (Character * ach, Character * bch)
{
  if (ach->leader != NULL)
    ach = ach->leader;
  if (bch->leader != NULL)
    bch = bch->leader;
  return ach == bch;
}

bool Note::is_note_to (Character * ch)
{
  if (!str_cmp (ch->name, sender))
    return true;

  if (is_name ("all", to_list))
    return true;

  if (ch->is_hero() && is_name ("immortal", to_list))
    return true;

  if (is_name (ch->name, to_list))
    return true;

  return false;
}

void note_attach (Character * ch)
{
  Note *pnote;

  if (ch->pnote != NULL)
    return;

  pnote = new Note();

  pnote->sender = ch->name;
  ch->pnote = pnote;
  return;
}

void note_remove (Character * ch, Note * pnote)
{
  std::string to_new, to_one, to_list;

  /*
   * Build a new to_list.
   * Strip out this recipient.
   */
  to_list = pnote->to_list;
  while (!to_list.empty()) {
    to_list = one_argument (to_list, to_one);
    if (!to_list.empty() && str_cmp (ch->name, to_one)) {
      to_new.append(" ");
      to_new.append(to_one);
    }
  }

  /*
   * Just a simple recipient removal?
   */
  if (str_cmp (ch->name, pnote->sender) && !to_new.empty()) {
    pnote->to_list = to_new.substr(1);
    return;
  }

  /*
   * Remove note from linked list.
   */
  note_list.erase(std::find(note_list.begin(),note_list.end(),pnote));
  delete pnote;

  /*
   * Rewrite entire list.
   */
  std::ofstream notefile;

  notefile.open (NOTE_FILE, std::ofstream::out | std::ofstream::binary);
  if (!notefile.is_open()) {
    std::perror (NOTE_FILE);
  } else {
    for (std::list<Note*>::iterator p = note_list.begin();
      p != note_list.end(); p++) {
      notefile << "Sender  " << (*p)->sender << "~\n";
      notefile << "Date    " << (*p)->date << "~\n";
      notefile << "Stamp   " << (*p)->date_stamp << "\n";
      notefile << "To      " << (*p)->to_list << "~\n";
      notefile << "Subject " << (*p)->subject << "~\n";
      notefile << "Text\n" << (*p)->text << "~\n\n";
    }
    notefile.close();
  }
  return;
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
void mprog_translate (char ch, std::string & t, Character * mob, Character * actor,
  Object * obj, void *vo, Character * rndm)
{
  static const char *he_she[] = { "it", "he", "she" };
  static const char *him_her[] = { "it", "him", "her" };
  static const char *his_her[] = { "its", "his", "her" };
  Character *vict = (Character *) vo;
  Object *v_obj = (Object *) vo;

  t.erase();
  switch (ch) {
  case 'i':
    one_argument (mob->name, t);
    break;

  case 'I':
    t = mob->short_descr;
    break;

  case 'n':
    if (actor && mob->can_see(actor)) {
      one_argument (actor->name, t);
      if (!actor->is_npc ())
        t[0] = toupper(t[0]);
    }
    break;

  case 'N':
    if (actor) {
      if (mob->can_see(actor)) {
        if (actor->is_npc ())
          t = actor->short_descr;
        else
          t = actor->name + " " + actor->pcdata->title;
      } else {
        t = "someone";
      }
    }
    break;

  case 't':
    if (vict && mob->can_see(vict)) {
      one_argument (vict->name, t);
      if (!vict->is_npc ())
        t[0] = toupper(t[0]);
    }
    break;

  case 'T':
    if (vict) {
      if (mob->can_see(vict)) {
        if (vict->is_npc ())
          t = vict->short_descr;
        else
          t = vict->name + " " + vict->pcdata->title;
      } else {
        t = "someone";
      }
    }
    break;

  case 'r':
    if (rndm && mob->can_see(rndm)) {
      one_argument (rndm->name, t);
      if (!rndm->is_npc ())
        t[0] = toupper(t[0]);
    }
    break;

  case 'R':
    if (rndm) {
      if (mob->can_see(rndm)) {
        if (rndm->is_npc ())
          t = rndm->short_descr;
        else
          t = rndm->name + " " + rndm->pcdata->title;
      } else {
        t = "someone";
      }
    }
    break;

  case 'e':
    if (actor) {
      if (mob->can_see(actor))
        t = he_she[actor->sex];
      else
        t = "someone";
    }
    break;

  case 'm':
    if (actor) {
      if (mob->can_see(actor))
        t = him_her[actor->sex];
      else
        t = "someone";
    }
    break;

  case 's':
    if (actor) {
      if (mob->can_see(actor))
        t = his_her[actor->sex];
      else
        t = "someone's";
    }
    break;

  case 'E':
    if (vict) {
      if (mob->can_see(vict))
        t = he_she[vict->sex];
      else
        t = "someone";
    }
    break;

  case 'M':
    if (vict) {
      if (mob->can_see(vict))
        t = him_her[vict->sex];
      else
        t = "someone";
    }
    break;

  case 'S':
    if (vict) {
      if (mob->can_see(vict))
        t = his_her[vict->sex];
      else
        t = "someone's";
    }
    break;

  case 'j':
    t = he_she[mob->sex];
    break;

  case 'k':
    t = him_her[mob->sex];
    break;

  case 'l':
    t = his_her[mob->sex];
    break;

  case 'J':
    if (rndm) {
      if (mob->can_see(rndm))
        t = he_she[rndm->sex];
      else
        t = "someone";
    }
    break;

  case 'K':
    if (rndm) {
      if (mob->can_see(rndm))
        t = him_her[rndm->sex];
      else
        t = "someone";
    }
    break;

  case 'L':
    if (rndm) {
      if (mob->can_see(rndm))
        t = his_her[rndm->sex];
      else
        t = "someone's";
    }
    break;

  case 'o':
    if (obj) {
      if (mob->can_see_obj(obj))
        one_argument (obj->name, t);
      else
        t = "something";
    }
    break;

  case 'O':
    if (obj) {
      if (mob->can_see_obj(obj))
        t = obj->short_descr;
      else
        t = "something";
    }
    break;

  case 'p':
    if (v_obj) {
      if (mob->can_see_obj(v_obj))
        one_argument (v_obj->name, t);
      else
        t = "something";
    }
    break;

  case 'P':
    if (v_obj) {
      if (mob->can_see_obj(v_obj))
        t = v_obj->short_descr;
      else
        t = "something";
    }
    break;

  case 'a':
    if (obj)
      switch (obj->name[0]) {
      case 'a':
      case 'e':
      case 'i':
      case 'o':
      case 'u':
        t = "an";
        break;
      default:
        t = "a";
      }
    break;

  case 'A':
    if (v_obj)
      switch (v_obj->name[0]) {
      case 'a':
      case 'e':
      case 'i':
      case 'o':
      case 'u':
        t = "an";
        break;
      default:
        t = "a";
      }
    break;

  case '$':
    t = "$";
    break;

  default:
    bug_printf ("Mob: %d bad $var", mob->pIndexData->vnum);
    break;
  }

  return;

}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd (const std::string & cmnd, Character * mob,
  Character * actor, Object * obj, void *vo, Character * rndm)
{
  std::string buf;
  std::string tmp;
  std::string::const_iterator str;
  str = cmnd.begin();

  while (str != cmnd.end()) {
    if (*str != '$') {
      buf.append(1, *str);
      str++;
      continue;
    }
    str++;
    mprog_translate (*str, tmp, mob, actor, obj, vo, rndm);
    buf.append(tmp);
    str++;
  }
  mob->interpret (buf);

  return;

}

/* Used to get sequential lines of a multi line string (separated by "\r\n")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
std::string mprog_next_command (std::string & clist, std::string & cmd)
{
  std::string::iterator pointer = clist.begin();

  while (*pointer != '\n' && *pointer != '\r' && pointer != clist.end())
    pointer++;
  cmd.assign(clist.begin(), pointer);
  while ((*pointer == '\n' || *pointer == '\r') && pointer != clist.end())
    pointer++;
  return std::string(pointer, clist.end());
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval (const std::string & lhs, const std::string & opr, const std::string & rhs)
{

  if (!str_cmp (opr, "=="))
    return (bool) (!str_cmp (lhs, rhs));
  if (!str_cmp (opr, "!="))
    return (bool) (str_cmp (lhs, rhs));
  if (!str_cmp (opr, "/"))
    return (bool) (!str_infix (rhs, lhs));
  if (!str_cmp (opr, "!/"))
    return (bool) (str_infix (rhs, lhs));

  bug_printf ("Improper MOBprog operator");
  return 0;

}

bool mprog_veval (int lhs, const std::string & opr, int rhs)
{

  if (!str_cmp (opr, "=="))
    return (lhs == rhs);
  if (!str_cmp (opr, "!="))
    return (lhs != rhs);
  if (!str_cmp (opr, ">"))
    return (lhs > rhs);
  if (!str_cmp (opr, "<"))
    return (lhs < rhs);
  if (!str_cmp (opr, ">="))
    return (lhs <= rhs);
  if (!str_cmp (opr, ">="))
    return (lhs >= rhs);
  if (!str_cmp (opr, "&"))
    return (lhs & rhs);
  if (!str_cmp (opr, "|"))
    return (lhs | rhs);

  bug_printf ("Improper MOBprog operator\r\n");
  return 0;

}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
int mprog_do_ifchck (const std::string & ifchck, Character * mob,
  Character * actor, Object * obj, void *vo, Character * rndm)
{

  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char opr[MAX_INPUT_LENGTH];
  char val[MAX_INPUT_LENGTH];
  Character *vict = (Character *) vo;
  Object *v_obj = (Object *) vo;
  char *bufpt = buf;
  char *argpt = arg;
  char *oprpt = opr;
  char *valpt = val;
  std::string::const_iterator point = ifchck.begin();
  int lhsvl;
  int rhsvl;

  if (ifchck.empty()) {
    bug_printf ("Mob: %d null ifchck", mob->pIndexData->vnum);
    return -1;
  }
  /* skip leading spaces */
  while (*point == ' ')
    point++;

  /* get whatever comes before the left paren.. ignore spaces */
  while (*point != '(')
    if (point == ifchck.end()) {
      bug_printf ("Mob: %d ifchck syntax error", mob->pIndexData->vnum);
      return -1;
    } else if (*point == ' ')
      point++;
    else
      *bufpt++ = *point++;

  *bufpt = '\0';
  point++;

  /* get whatever is in between the parens.. ignore spaces */
  while (*point != ')')
    if (point == ifchck.end()) {
      bug_printf ("Mob: %d ifchck syntax error", mob->pIndexData->vnum);
      return -1;
    } else if (*point == ' ')
      point++;
    else
      *argpt++ = *point++;

  *argpt = '\0';
  point++;

  /* check to see if there is an operator */
  while (*point == ' ')
    point++;
  if (point == ifchck.end()) {
    *opr = '\0';
    *val = '\0';
  } else {                      /* there should be an operator and value, so get them */

    while ((*point != ' ') && (!isalnum (*point)))
      if (point == ifchck.end()) {
        bug_printf ("Mob: %d ifchck operator without value", mob->pIndexData->vnum);
        return -1;
      } else
        *oprpt++ = *point++;

    *oprpt = '\0';

    /* finished with operator, skip spaces and then get the value */
    while (*point == ' ')
      point++;
    for (;;) {
      if ((*point != ' ') && (point == ifchck.end()))
        break;
      else
        *valpt++ = *point++;
    }

    *valpt = '\0';
  }
//  bufpt = buf;
//  argpt = arg;
//  oprpt = opr;
//  valpt = val;

  /* Ok... now buf contains the ifchck, arg contains the inside of the
   *  parentheses, opr contains an operator if one is present, and val
   *  has the value if an operator was present.
   *  So.. basically use if statements and run over all known ifchecks
   *  Once inside, use the argument and expand the lhs. Then if need be
   *  send the lhs,opr,rhs off to be evaluated.
   */

  if (!str_cmp (buf, "rand")) {
    return (number_percent () <= std::atoi (arg));
  }

  if (!str_cmp (buf, "ispc")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return 0;
    case 'n':
      if (actor)
        return (!actor->is_npc ());
      else
        return -1;
    case 't':
      if (vict)
        return (!vict->is_npc ());
      else
        return -1;
    case 'r':
      if (rndm)
        return (!rndm->is_npc ());
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'ispc'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isnpc")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return 1;
    case 'n':
      if (actor)
        return actor->is_npc ();
      else
        return -1;
    case 't':
      if (vict)
        return vict->is_npc ();
      else
        return -1;
    case 'r':
      if (rndm)
        return rndm->is_npc ();
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isnpc'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isgood")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return mob->is_good ();
    case 'n':
      if (actor)
        return actor->is_good ();
      else
        return -1;
    case 't':
      if (vict)
        return vict->is_good ();
      else
        return -1;
    case 'r':
      if (rndm)
        return rndm->is_good ();
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isgood'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isfight")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return (mob->fighting) ? 1 : 0;
    case 'n':
      if (actor)
        return (actor->fighting) ? 1 : 0;
      else
        return -1;
    case 't':
      if (vict)
        return (vict->fighting) ? 1 : 0;
      else
        return -1;
    case 'r':
      if (rndm)
        return (rndm->fighting) ? 1 : 0;
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isfight'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isimmort")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return (mob->get_trust () > LEVEL_IMMORTAL);
    case 'n':
      if (actor)
        return (actor->get_trust () > LEVEL_IMMORTAL);
      else
        return -1;
    case 't':
      if (vict)
        return (vict->get_trust () > LEVEL_IMMORTAL);
      else
        return -1;
    case 'r':
      if (rndm)
        return (rndm->get_trust () > LEVEL_IMMORTAL);
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isimmort'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "ischarmed")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return mob->is_affected (AFF_CHARM);
    case 'n':
      if (actor)
        return actor->is_affected (AFF_CHARM);
      else
        return -1;
    case 't':
      if (vict)
        return vict->is_affected (AFF_CHARM);
      else
        return -1;
    case 'r':
      if (rndm)
        return rndm->is_affected (AFF_CHARM);
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'ischarmed'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isfollow")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return (mob->master != NULL && mob->master->in_room == mob->in_room);
    case 'n':
      if (actor)
        return (actor->master != NULL
          && actor->master->in_room == actor->in_room);
      else
        return -1;
    case 't':
      if (vict)
        return (vict->master != NULL
          && vict->master->in_room == vict->in_room);
      else
        return -1;
    case 'r':
      if (rndm)
        return (rndm->master != NULL
          && rndm->master->in_room == rndm->in_room);
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isfollow'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "isaffected")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return (mob->affected_by & std::atoi (arg));
    case 'n':
      if (actor)
        return (actor->affected_by & std::atoi (arg));
      else
        return -1;
    case 't':
      if (vict)
        return (vict->affected_by & std::atoi (arg));
      else
        return -1;
    case 'r':
      if (rndm)
        return (rndm->affected_by & std::atoi (arg));
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'isaffected'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "hitprcnt")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->hit / mob->max_hit;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->hit / actor->max_hit;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->hit / vict->max_hit;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->hit / rndm->max_hit;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'hitprcnt'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "inroom")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->in_room->vnum;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->in_room->vnum;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->in_room->vnum;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->in_room->vnum;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'inroom'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "sex")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->sex;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->sex;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->sex;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->sex;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'sex'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "position")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->position;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->position;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->position;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->position;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'position'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "level")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->get_trust ();
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->get_trust ();
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->get_trust ();
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->get_trust ();
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'level'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "class")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->klass;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->klass;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->klass;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->klass;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'class'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "goldamt")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->gold;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        lhsvl = actor->gold;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 't':
      if (vict) {
        lhsvl = vict->gold;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'r':
      if (rndm) {
        lhsvl = rndm->gold;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'goldamt'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "objtype")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'o':
      if (obj) {
        lhsvl = obj->item_type;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->item_type;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'objtype'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "objval0")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'o':
      if (obj) {
        lhsvl = obj->value[0];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->value[0];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'objval0'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "objval1")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'o':
      if (obj) {
        lhsvl = obj->value[1];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->value[1];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'objval1'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "objval2")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'o':
      if (obj) {
        lhsvl = obj->value[2];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->value[2];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'objval2'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "objval3")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'o':
      if (obj) {
        lhsvl = obj->value[3];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->value[3];
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'objval3'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "number")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      lhsvl = mob->gold;
      rhsvl = std::atoi (val);
      return mprog_veval (lhsvl, opr, rhsvl);
    case 'n':
      if (actor) {
        if (actor->is_npc ()) {
          lhsvl = actor->pIndexData->vnum;
          rhsvl = std::atoi (val);
          return mprog_veval (lhsvl, opr, rhsvl);
          }
      } else
        return -1;
    case 't':
      if (vict) {
        if (actor->is_npc ()) {
          lhsvl = vict->pIndexData->vnum;
          rhsvl = std::atoi (val);
          return mprog_veval (lhsvl, opr, rhsvl);
          }
      } else
        return -1;
    case 'r':
      if (rndm) {
        if (actor->is_npc ()) {
          lhsvl = rndm->pIndexData->vnum;
          rhsvl = std::atoi (val);
          return mprog_veval (lhsvl, opr, rhsvl);
          }
      } else
        return -1;
    case 'o':
      if (obj) {
        lhsvl = obj->pIndexData->vnum;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    case 'p':
      if (v_obj) {
        lhsvl = v_obj->pIndexData->vnum;
        rhsvl = std::atoi (val);
        return mprog_veval (lhsvl, opr, rhsvl);
      } else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'number'", mob->pIndexData->vnum);
      return -1;
    }
  }

  if (!str_cmp (buf, "name")) {
    switch (arg[1]) {           /* arg should be "$*" so just get the letter */
    case 'i':
      return mprog_seval (mob->name, opr, val);
    case 'n':
      if (actor)
        return mprog_seval (actor->name, opr, val);
      else
        return -1;
    case 't':
      if (vict)
        return mprog_seval (vict->name, opr, val);
      else
        return -1;
    case 'r':
      if (rndm)
        return mprog_seval (rndm->name, opr, val);
      else
        return -1;
    case 'o':
      if (obj)
        return mprog_seval (obj->name, opr, val);
      else
        return -1;
    case 'p':
      if (v_obj)
        return mprog_seval (v_obj->name, opr, val);
      else
        return -1;
    default:
      bug_printf ("Mob: %d bad argument to 'name'", mob->pIndexData->vnum);
      return -1;
    }
  }

  /* Ok... all the ifchcks are done, so if we didnt find ours then something
   * odd happened.  So report the bug and abort the MOBprogram (return error)
   */
  bug_printf ("Mob: %d unknown ifchck", mob->pIndexData->vnum);
  return -1;

}

/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
std::string mprog_process_if (const std::string & ifchck, std::string & com_list,
  Character * mob, Character * actor, Object * obj, void *vo, Character * rndm)
{
  std::string buf;
  std::string morebuf;
  std::string cmnd;
  bool loopdone = false;
  bool flag = false;
  int legal;

  /* check for trueness of the ifcheck */
  if ((legal = mprog_do_ifchck (ifchck, mob, actor, obj, vo, rndm))) {
    if (legal == 1)
      flag = true;
    else
      return "";
  }

  while (loopdone == false) {   /*scan over any existing or statements */
    com_list = mprog_next_command (com_list, cmnd);
    cmnd.erase(0, cmnd.find_first_not_of(" "));
    if (cmnd.empty()) {
      bug_printf ("Mob: %d no commands after IF/OR", mob->pIndexData->vnum);
      return "";
    }
    morebuf = one_argument (cmnd, buf);
    if (!str_cmp (buf, "or")) {
      if ((legal = mprog_do_ifchck (morebuf, mob, actor, obj, vo, rndm))) {
        if (legal == 1)
          flag = true;
        else
          return "";
      }
    } else
      loopdone = true;
  }

  if (flag)
    for (;;) {                  /*ifcheck was true, do commands but ignore else to endif */
      if (!str_cmp (buf, "if")) {
        com_list =
          mprog_process_if (morebuf, com_list, mob, actor, obj, vo, rndm);
        cmnd.erase(0, cmnd.find_first_not_of(" "));
        if (com_list.empty())
          return "";
        com_list = mprog_next_command (com_list, cmnd);
        morebuf = one_argument (cmnd, buf);
        continue;
      }
      if (!str_cmp (buf, "break"))
        return "";
      if (!str_cmp (buf, "endif"))
        return com_list;
      if (!str_cmp (buf, "else")) {
        while (str_cmp (buf, "endif")) {
          com_list = mprog_next_command (com_list, cmnd);
          cmnd.erase(0, cmnd.find_first_not_of(" "));
          if (cmnd.empty()) {
            bug_printf ("Mob: %d missing endif after else", mob->pIndexData->vnum);
            return "";
          }
          morebuf = one_argument (cmnd, buf);
        }
        return com_list;
      }
      mprog_process_cmnd (cmnd, mob, actor, obj, vo, rndm);
      com_list = mprog_next_command (com_list, cmnd);
      cmnd.erase(0, cmnd.find_first_not_of(" "));
      if (cmnd.empty()) {
        bug_printf ("Mob: %d missing else or endif", mob->pIndexData->vnum);
        return "";
      }
      morebuf = one_argument (cmnd, buf);
  } else {                      /*false ifcheck, find else and do existing commands or quit at endif */

    while ((str_cmp (buf, "else")) && (str_cmp (buf, "endif"))) {
      com_list = mprog_next_command (com_list, cmnd);
      cmnd.erase(0, cmnd.find_first_not_of(" "));
      if (cmnd.empty()) {
        bug_printf ("Mob: %d missing an else or endif", mob->pIndexData->vnum);
        return "";
      }
      morebuf = one_argument (cmnd, buf);
    }

    /* found either an else or an endif.. act accordingly */
    if (!str_cmp (buf, "endif"))
      return com_list;
    com_list = mprog_next_command (com_list, cmnd);
    cmnd.erase(0, cmnd.find_first_not_of(" "));
    if (cmnd.empty()) {
      bug_printf ("Mob: %d missing endif", mob->pIndexData->vnum);
      return "";
    }
    morebuf = one_argument (cmnd, buf);

    for (;;) {                  /*process the post-else commands until an endif is found. */
      if (!str_cmp (buf, "if")) {
        com_list = mprog_process_if (morebuf, com_list, mob, actor,
          obj, vo, rndm);
        cmnd.erase(0, cmnd.find_first_not_of(" "));
        if (com_list.empty())
          return "";
        com_list = mprog_next_command (com_list, cmnd);
        morebuf = one_argument (cmnd, buf);
        continue;
      }
      if (!str_cmp (buf, "else")) {
        bug_printf ("Mob: %d found else in an else section", mob->pIndexData->vnum);
        return "";
      }
      if (!str_cmp (buf, "break"))
        return "";
      if (!str_cmp (buf, "endif"))
        return com_list;
      mprog_process_cmnd (cmnd, mob, actor, obj, vo, rndm);
      com_list = mprog_next_command (com_list, cmnd);
      cmnd.erase(0, cmnd.find_first_not_of(" "));
      if (cmnd.empty()) {
        bug_printf ("Mob:%d missing endif in else section", mob->pIndexData->vnum);
        return "";
      }
      morebuf = one_argument (cmnd, buf);
    }
  }
}

/* The main focus of the MOBprograms.  This routine is called
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver (const std::string & com_list, Character * mob,
  Character * actor, Object * obj, void *vo)
{

  std::string tmpcmndlst;
  std::string buf;
  std::string morebuf;
  std::string command_list;
  std::string cmnd;
  Character *rndm = NULL;
  int count = 0;

  if (mob->is_affected (AFF_CHARM))
      return;

  /* get a random visable mortal player who is in the room with the mob */
  CharIter vch;
  for (vch = mob->in_room->people.begin(); vch != mob->in_room->people.end(); vch++)
    if (!(*vch)->is_npc ()
      && (*vch)->level < LEVEL_IMMORTAL && mob->can_see(*vch)) {
      if (number_range (0, count) == 0)
        rndm = *vch;
      count++;
    }

  tmpcmndlst = com_list;
  command_list = tmpcmndlst;
  command_list = mprog_next_command (command_list, cmnd);
  while (!cmnd.empty()) {
    morebuf = one_argument (cmnd, buf);
    if (!str_cmp (buf, "if"))
      command_list = mprog_process_if (morebuf, command_list, mob,
        actor, obj, vo, rndm);
    else
      mprog_process_cmnd (cmnd, mob, actor, obj, vo, rndm);
    command_list = mprog_next_command (command_list, cmnd);
  }

  return;

}

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check (const std::string & arg, Character * mob,
  Character * actor, Object * obj, void *vo, int type)
{
  std::string list;
  std::string dupl;
  std::string word;
  MobProgram *mprg;

  std::string::size_type start;
  std::string::size_type end;

  unsigned int i;

  for (mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next)
    if (mprg->type & type) {
      list = mprg->arglist;
      for (i = 0; i < list.size(); i++)
        list[i] = tolower (list[i]);
      dupl = arg;
      for (i = 0; i < dupl.size(); i++)
        dupl[i] = tolower (dupl[i]);

      if (list.substr(0,2) == "p ") {
        list = list.substr(2);
        while ((start = dupl.find(list)) != std::string::npos)
          if ((start == 0 || dupl[start - 1] == ' ')
            && (dupl[end = start + list.size()] == ' '
              || dupl[end] == '\n' || dupl[end] == '\r' || dupl[end] == '\0')) {
            mprog_driver (mprg->comlist, mob, actor, obj, vo);
            break;
          } else
            dupl = dupl.substr(start + 1);
      } else {
        list = one_argument (list, word);
        for (; !word.empty(); list = one_argument (list, word))
          while ((start = dupl.find(word)) != std::string::npos)
            if ((start == 0 || dupl[start - 1] == ' ')
              && (dupl[end = start + word.size()] == ' '
              || dupl[end] == '\n' || dupl[end] == '\r' || dupl[end] == '\0')) {
              mprog_driver (mprg->comlist, mob, actor, obj, vo);
              break;
            } else
              dupl = dupl.substr(start + 1);
      }
    }

  return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update (void)
{
  Character *ch;
  Exit *pexit;
  int door;
try {
  /* Examine all mobs. */
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c = deepchnext) {
    ch = *c;
    deepchnext = ++c;

    if (!ch->is_npc () || ch->in_room == NULL || ch->is_affected (AFF_CHARM))
      continue;

    /* Examine call for special procedure */
    if (ch->spec_fun != 0) {
      if ((*ch->spec_fun) (ch))
        continue;
    }

    /* That's all for sleeping / busy monster */
    if (ch->position < POS_STANDING)
      continue;

    /* MOBprogram random trigger */
    if (ch->in_room->area->nplayer > 0) {
      mprog_random_trigger (ch);
      /* If ch dies or changes
         position due to it's random
         trigger continue - Kahn */
      if (ch->position < POS_STANDING)
        continue;
    }

    /* Scavenge */
    if (IS_SET (ch->actflags, ACT_SCAVENGER)
      && !ch->in_room->contents.empty() && number_percent() <= 25) {
      Object *obj_best;
      int max;

      max = 1;
      obj_best = 0;
      ObjIter obj;
      for (obj = ch->in_room->contents.begin(); obj != ch->in_room->contents.end(); obj++) {
        if ((*obj)->can_wear(ITEM_TAKE) && (*obj)->cost > max) {
          obj_best = *obj;
          max = (*obj)->cost;
        }
      }

      if (obj_best) {
        obj_best->obj_from_room ();
        obj_best->obj_to_char (ch);
        ch->act ("$n gets $p.", obj_best, NULL, TO_ROOM);
      }
    }

    /* Wander */
    if (!IS_SET (ch->actflags, ACT_SENTINEL)
      && (door = number_range (0, 31)) <= 5
      && (pexit = ch->in_room->exit[door]) != NULL
      && pexit->to_room != NULL && !IS_SET (pexit->exit_info, EX_CLOSED)
      && !IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)
      && (!IS_SET (ch->actflags, ACT_STAY_AREA)
        || pexit->to_room->area == ch->in_room->area)) {
      ch->move_char (door);
      /* If ch changes position due
         to it's or someother mob's
         movement via MOBProgs,
         continue - Kahn */
      if (ch->position < POS_STANDING)
        continue;
    }

    /* Flee */
    if (ch->hit < (ch->max_hit / 2)
      && (door = number_range (0, 7)) <= 5
      && (pexit = ch->in_room->exit[door]) != NULL
      && pexit->to_room != NULL && !IS_SET (pexit->exit_info, EX_CLOSED)
      && !IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)) {
      bool found;

      found = false;
      CharIter rch;
      for (rch = pexit->to_room->people.begin(); rch != pexit->to_room->people.end(); rch++) {
        if (!(*rch)->is_npc ()) {
          found = true;
          break;
        }
      }
      if (!found)
        ch->move_char (door);
    }

  }

} catch (...) {
  fatal_printf("mobile_update() exception");
}
  return;
}

/*
 * Update the weather.
 */
void weather_update (void)
{
  std::string buf = g_world->weather_update();

  if (!buf.empty()) {
    for (DescIter d = descriptor_list.begin();
      d != descriptor_list.end(); d++) {
      if ((*d)->connected == CON_PLAYING && (*d)->character->is_outside()
        && (*d)->character->is_awake ())
        (*d)->character->send_to_char (buf);
    }
  }

  return;
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update (void)
{
  Character *ch;
  Character *ch_save;
  Character *ch_quit;
  time_t save_time;

try {
  save_time = g_world->get_current_time();
  ch_save = NULL;
  ch_quit = NULL;
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c = deepchnext) {
    Affect *paf;
    ch = *c;
    deepchnext = ++c;

    /*
     * Find dude with oldest save time.
     */
    if (!ch->is_npc ()
      && (ch->desc == NULL || ch->desc->connected == CON_PLAYING)
      && ch->level >= 2 && ch->save_time < save_time) {
      ch_save = ch;
      save_time = ch->save_time;
    }

    if (ch->position >= POS_STUNNED) {
      if (ch->hit < ch->max_hit)
        ch->hit += ch->hit_gain();

      if (ch->mana < ch->max_mana)
        ch->mana += ch->mana_gain ();

      if (ch->move < ch->max_move)
        ch->move += ch->move_gain();
    }

    if (ch->position == POS_STUNNED)
      ch->update_pos();

    if (!ch->is_npc () && ch->level < LEVEL_IMMORTAL) {
      Object *obj;

      if ((obj = ch->get_eq_char (WEAR_LIGHT)) != NULL
        && obj->item_type == ITEM_LIGHT && obj->value[2] > 0) {
        if (--obj->value[2] == 0 && ch->in_room != NULL) {
          --ch->in_room->light;
          ch->act ("$p goes out.", obj, NULL, TO_ROOM);
          ch->act ("$p goes out.", obj, NULL, TO_CHAR);
          obj->extract_obj ();
        }
      }

      if (++ch->timer >= 12) {
        if (ch->was_in_room == NULL && ch->in_room != NULL) {
          ch->was_in_room = ch->in_room;
          if (ch->fighting != NULL)
            ch->stop_fighting (true);
          ch->act ("$n disappears into the void.", NULL, NULL, TO_ROOM);
          ch->send_to_char ("You disappear into the void.\r\n");
          ch->save_char_obj();
          ch->char_from_room();
          ch->char_to_room(get_room_index (ROOM_VNUM_LIMBO));
        }
      }

      if (ch->timer > 30)
        ch_quit = ch;

      ch->gain_condition (COND_DRUNK, -1);
      ch->gain_condition (COND_FULL, -1);
      ch->gain_condition (COND_THIRST, -1);
    }

    AffIter af, next;
    for (af = ch->affected.begin(); af != ch->affected.end(); af = next) {
      paf = *af;
      next = ++af;
      if (paf->duration > 0)
        paf->duration--;
      else if (paf->duration < 0);
      else {
        if (next == ch->affected.end()
          || (*next)->type != paf->type || (*next)->duration > 0) {
          if (paf->type > 0 && skill_table[paf->type].msg_off[0] != '\0') {
            ch->send_to_char (skill_table[paf->type].msg_off);
            ch->send_to_char ("\r\n");
          }
        }

        ch->affect_remove (paf);
      }
    }

    /*
     * Careful with the damages here,
     *   MUST NOT refer to ch after damage taken,
     *   as it may be lethal damage (on NPC).
     */
    if (ch->is_affected (AFF_POISON)) {
      ch->act ("$n shivers and suffers.", NULL, NULL, TO_ROOM);
      ch->send_to_char ("You shiver and suffer.\r\n");
      damage (ch, ch, 2, skill_lookup("poison"));
    } else if (ch->position == POS_INCAP) {
      damage (ch, ch, 1, TYPE_UNDEFINED);
    } else if (ch->position == POS_MORTAL) {
      damage (ch, ch, 2, TYPE_UNDEFINED);
    }
  }

  /*
   * Autosave and autoquit.
   * Check that these chars still exist.
   */
  if (ch_save != NULL || ch_quit != NULL) {
    CharIter cnext;
    for (c = char_list.begin(); c != char_list.end(); c = cnext) {
      ch = *c;
      cnext = ++c;
      if (ch == ch_save)
        ch->save_char_obj();
      if (ch == ch_quit)
        ch->do_quit ("");
    }
  }
} catch (...) {
  fatal_printf("char_update() exception");
}

  return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update (void)
{
  Object *obj;
  ObjIter o;

try {
  for (o = object_list.begin(); o != object_list.end(); o = deepobnext) {
    const char *message;
    obj = *o;
    deepobnext = ++o;

    if (obj->timer <= 0 || --obj->timer > 0)
      continue;

    switch (obj->item_type) {
    default:
      message = "$p vanishes.";
      break;
    case ITEM_FOUNTAIN:
      message = "$p dries up.";
      break;
    case ITEM_CORPSE_NPC:
      message = "$p decays into dust.";
      break;
    case ITEM_CORPSE_PC:
      message = "$p decays into dust.";
      break;
    case ITEM_FOOD:
      message = "$p decomposes.";
      break;
    }

    if (obj->carried_by != NULL) {
      obj->carried_by->act (message, obj, NULL, TO_CHAR);
    } else if (obj->in_room != NULL && !obj->in_room->people.empty()) {
      Character *rch = obj->in_room->people.front();
      rch->act (message, obj, NULL, TO_ROOM);
      rch->act (message, obj, NULL, TO_CHAR);
    }

    obj->extract_obj ();
  }
} catch (...) {
  fatal_printf("obj_update() exception");
}

  return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update (void)
{
  Character *wch;
  Character *ch;
  Character *vch;
  Character *victim;

try {
  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c = deepchnext) {
    wch = *c;
    deepchnext = ++c;

    /* MOBProgram ACT_PROG trigger */
    if (wch->is_npc () && wch->mpactnum > 0 && wch->in_room->area->nplayer > 0) {
      MobProgramActList *tmp_act, *tmp2_act;
      for (tmp_act = wch->mpact; tmp_act != NULL; tmp_act = tmp_act->next) {
        mprog_wordlist_check (tmp_act->buf, wch, tmp_act->ch,
          tmp_act->obj, tmp_act->vo, ACT_PROG);
      }
      for (tmp_act = wch->mpact; tmp_act != NULL; tmp_act = tmp2_act) {
        tmp2_act = tmp_act->next;
        delete tmp_act;
      }
      wch->mpactnum = 0;
      wch->mpact = NULL;
    }

    if (wch->is_npc ()
      || wch->level >= LEVEL_IMMORTAL || wch->in_room == NULL)
      continue;

    Room* rlist = wch->in_room;

    CharIter rch, rcnext;
    for (rch = rlist->people.begin(); rch != rlist->people.end(); rch = deeprmnext) {
      ch = *rch;
      deeprmnext = ++rch;
      int count;

      if (!ch->is_npc ()
        || !IS_SET (ch->actflags, ACT_AGGRESSIVE)
        || ch->fighting != NULL || ch->is_affected (AFF_CHARM)
        || !ch->is_awake ()
        || (IS_SET (ch->actflags, ACT_WIMPY) && wch->is_awake ())
        || !ch->can_see(wch))
        continue;

      /*
       * Ok we have a 'wch' player character and a 'ch' npc aggressor.
       * Now make the aggressor fight a RANDOM pc victim in the room,
       *   giving each 'vch' an equal chance of selection.
       */
      count = 0;
      victim = NULL;
      for (CharIter vc = rlist->people.begin(); vc != rlist->people.end(); vc++) {
        if (!(*vc)->is_npc () && (*vc)->level < LEVEL_IMMORTAL
          && (!IS_SET (ch->actflags, ACT_WIMPY) || !(*vc)->is_awake ())
          && ch->can_see(*vc)) {
          if (number_range (0, count) == 0)
            victim = *vc;
          count++;
        }
      }

      if (victim == NULL) {
        bug_printf ("Aggr_update: null victim.", count);
        continue;
      }

      multi_hit (ch, victim, TYPE_UNDEFINED);
    }
  }

} catch (...) {
  fatal_printf("aggr_update() exception");
}

  return;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update (void)
{
  Character *ch;
  Character *victim;
  Character *rch;

try {

  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c = deepchnext) {
    ch = *c;
    deepchnext = ++c;

    victim = ch->fighting;
    if (victim == NULL || ch->in_room == NULL)
      continue;

    if (ch->is_awake () && ch->in_room == victim->in_room)
      multi_hit (ch, victim, TYPE_UNDEFINED);
    else
      ch->stop_fighting (false);

    victim = ch->fighting;
    if (victim == NULL)
      continue;

    mprog_hitprcnt_trigger (ch, victim);
    mprog_fight_trigger (ch, victim);

    /*
     * Fun for the whole family!
     */

    if (ch == NULL || ch->in_room == NULL)
      continue;
    victim = ch->fighting;
    if (victim == NULL)
      continue;
    Room * rlist = ch->in_room;

    CharIter rc, rnext;
    for (rc = rlist->people.begin(); rc != rlist->people.end(); rc = deeprmnext) {
      rch = *rc;
      deeprmnext = ++rc;

      if (rch->fighting == NULL && rch->is_awake ()) {
        /*
         * PC's auto-assist others in their group.
         */
        if (!ch->is_npc () || ch->is_affected (AFF_CHARM)) {
          if ((!rch->is_npc () || rch->is_affected (AFF_CHARM))
            && is_same_group (ch, rch))
            multi_hit (rch, victim, TYPE_UNDEFINED);
          continue;
        }

        /*
         * NPC's assist NPC's of same type or 12.5% chance regardless.
         */
        if (rch->is_npc () && !rch->is_affected (AFF_CHARM)) {
          if (rch->pIndexData == ch->pIndexData || number_range (0, 7) == 0) {
            Character *target;
            int number;

            target = NULL;
            number = 0;
            CharIter vch;
            for (vch = rlist->people.begin(); vch != rlist->people.end(); vch++) {
              if (rch->can_see(*vch)
                && is_same_group (*vch, victim)
                && number_range (0, number) == 0) {
                target = *vch;
                number++;
              }
            }

            if (target != NULL) {
              if ((((target->level - rch->level <= 4)
                    && (target->level - rch->level >= -4))
                  && !(rch->is_good () && target->is_good ()))
                || (rch->is_evil () || target->is_evil ()))
                multi_hit (rch, target, TYPE_UNDEFINED);
            }
          }
        }
      }
    }
  }

} catch (...) {
  fatal_printf("violence_update() exception");
}

  return;
}


void extract_dead_characters()
{
  Character *curr;
  CharIter c, next;
  for (c = char_list.begin(); c != char_list.end(); c = next) {
	curr = *c;
    next = ++c;
    if (curr->is_npc()) {
      if (IS_SET(curr->actflags, ACT_EXTRACT)) {
		  REMOVE_BIT(curr->actflags, ACT_EXTRACT);
		  curr->extract_char_old(true);
	  }
    } else {
      if (IS_SET(curr->actflags, PLR_EXTRACT)) {
		  REMOVE_BIT(curr->actflags, PLR_EXTRACT);
		  curr->extract_char_old(false);
 	  }
	}
  }
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler (void)
{
  static int pulse_area;
  static int pulse_mobile;
  static int pulse_violence;
  static int pulse_point;

try {
  if (--pulse_area <= 0) {
    pulse_area = number_range (PULSE_AREA / 2, 3 * PULSE_AREA / 2);
    g_world->area_update ();
  }

  if (--pulse_violence <= 0) {
    pulse_violence = PULSE_VIOLENCE;
    violence_update ();
  }

  if (--pulse_mobile <= 0) {
    pulse_mobile = PULSE_MOBILE;
    mobile_update ();
  }

  if (--pulse_point <= 0) {
    pulse_point = number_range (PULSE_TICK / 2, 3 * PULSE_TICK / 2);
    weather_update ();
    char_update ();
    obj_update ();
  }

  aggr_update ();
  if (extract_chars) {
	  extract_dead_characters();
	  extract_chars = false;
  }

} catch (...) {
  fatal_printf("update_handler() exception");
}
  return;
}

/*
 * Shopping commands.
 */
Character *find_keeper (Character * ch)
{
  char buf[MAX_STRING_LENGTH];
  Shop *pShop;

  pShop = NULL;
  CharIter keeper;
  for (keeper = ch->in_room->people.begin(); keeper != ch->in_room->people.end(); keeper++) {
    if ((*keeper)->is_npc () && (pShop = (*keeper)->pIndexData->pShop) != NULL)
      break;
  }

  if (pShop == NULL) {
    ch->send_to_char ("You can't do that here.\r\n");
    return NULL;
  }

  /*
   * Undesirables.
   */
  if (!ch->is_npc () && IS_SET (ch->actflags, PLR_KILLER)) {
    (*keeper)->do_say ("Killers are not welcome!");
    snprintf (buf, sizeof buf, "%s the KILLER is over here!\r\n", ch->name.c_str());
    (*keeper)->do_shout (buf);
    return NULL;
  }

  if (!ch->is_npc () && IS_SET (ch->actflags, PLR_THIEF)) {
    (*keeper)->do_say ("Thieves are not welcome!");
    snprintf (buf, sizeof buf, "%s the THIEF is over here!\r\n", ch->name.c_str());
    (*keeper)->do_shout (buf);
    return NULL;
  }

  /*
   * Shop hours.
   */
  if (g_world->hour() < pShop->open_hour) {
    (*keeper)->do_say ("Sorry, come back later.");
    return NULL;
  }

  if (g_world->hour() > pShop->close_hour) {
    (*keeper)->do_say ("Sorry, come back tomorrow.");
    return NULL;
  }

  /*
   * Invisible or hidden people.
   */
  if (!(*keeper)->can_see(ch)) {
    (*keeper)->do_say ("I don't trade with folks I can't see.");
    return NULL;
  }

  return *keeper;
}

int get_cost (Character * keeper, Object * obj, bool fBuy)
{
  Shop *pShop;
  int cost;

  if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
    return 0;

  if (fBuy) {
    cost = obj->cost * pShop->profit_buy / 100;
  } else {
    int itype;

    cost = 0;
    for (itype = 0; itype < MAX_TRADE; itype++) {
      if (obj->item_type == pShop->buy_type[itype]) {
        cost = obj->cost * pShop->profit_sell / 100;
        break;
      }
    }

    ObjIter o;
    for (o = keeper->carrying.begin(); o != keeper->carrying.end(); o++) {
      if (obj->pIndexData == (*o)->pIndexData)
        cost /= 2;
    }
  }

  if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    cost = cost * obj->value[2] / obj->value[1];

  return cost;
}

/*
 * Generic channel function.
 */
void talk_channel (Character * ch, const std::string & argument, int channel,
  const char *verb)
{
  char buf[MAX_STRING_LENGTH];
  int position;

  if (argument.empty()) {
    snprintf (buf, sizeof buf, "%s what?\r\n", verb);
    buf[0] = toupper (buf[0]);
    return;
  }

  if (!ch->is_npc () && IS_SET (ch->actflags, PLR_SILENCE)) {
    snprintf (buf, sizeof buf, "You can't %s.\r\n", verb);
    ch->send_to_char (buf);
    return;
  }

  REMOVE_BIT (ch->deaf, channel);

  switch (channel) {
  default:
    snprintf (buf, sizeof buf, "You %s '%s'.\r\n", verb, argument.c_str());
    ch->send_to_char (buf);
    snprintf (buf, sizeof buf, "$n %ss '$t'.", verb);
    break;

  case CHANNEL_IMMTALK:
    snprintf (buf, sizeof buf, "$n: $t.");
    position = ch->position;
    ch->position = POS_STANDING;
    ch->act (buf, argument.c_str(), NULL, TO_CHAR);
    ch->position = position;
    break;
  }

  for (DescIter d = descriptor_list.begin();
    d != descriptor_list.end(); d++) {
    Character *och;
    Character *vch;

    och = (*d)->original ? (*d)->original : (*d)->character;
    vch = (*d)->character;

    if ((*d)->connected == CON_PLAYING
      && vch != ch && !IS_SET (och->deaf, channel)) {
      if (channel == CHANNEL_IMMTALK && !och->is_hero())
        continue;
      if (channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area)
        continue;

      position = vch->position;
      if (channel != CHANNEL_SHOUT && channel != CHANNEL_YELL)
        vch->position = POS_STANDING;
      ch->act (buf, argument.c_str(), vch, TO_VICT);
      vch->position = position;
    }
  }

  return;
}

Room *find_location (Character * ch, const std::string & arg)
{
  Character *victim;
  Object *obj;

  if (is_number (arg))
    return get_room_index (std::atoi (arg.c_str()));

  if ((victim = ch->get_char_world (arg)) != NULL)
    return victim->in_room;

  if ((obj = ch->get_obj_world (arg)) != NULL)
    return obj->in_room;

  return NULL;
}

bool is_safe (Character * ch, Character * victim)
{
  if (ch->is_npc () || victim->is_npc ())
    return false;

  if (ch->get_age() < 21) {
    ch->send_to_char ("You aren't old enough.\r\n");
    return true;
  }

  if (IS_SET (victim->actflags, PLR_KILLER))
    return false;

  if (ch->level >= victim->level) {
    ch->send_to_char ("You may not attack a lower level player.\r\n");
    return true;
  }

  return false;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer (Character * ch, Character * victim)
{
  /*
   * Follow charm thread to responsible character.
   * Attacking someone's charmed char is hostile!
   */
  while (victim->is_affected (AFF_CHARM) && victim->master != NULL)
    victim = victim->master;

  /*
   * NPC's are fair game.
   * So are killers and thieves.
   */
  if (victim->is_npc ()
    || IS_SET (victim->actflags, PLR_KILLER)
    || IS_SET (victim->actflags, PLR_THIEF))
    return;

  /*
   * Charm-o-rama.
   */
  if (IS_SET (ch->affected_by, AFF_CHARM)) {
    if (ch->master == NULL) {
      bug_printf ("Check_killer: %s bad AFF_CHARM",
        ch->is_npc () ? ch->short_descr.c_str() : ch->name.c_str());
      ch->affect_strip (skill_lookup("charm person"));
      REMOVE_BIT (ch->affected_by, AFF_CHARM);
      return;
    }

    ch->master->send_to_char ("*** You are now a KILLER!! ***\r\n");
    SET_BIT (ch->master->actflags, PLR_KILLER);
    ch->stop_follower();
    return;
  }

  /*
   * NPC's are cool of course (as long as not charmed).
   * Hitting yourself is cool too (bleeding).
   * So is being immortal (Alander's idea).
   * And current killers stay as they are.
   */
  if (ch->is_npc ()
    || ch == victim
    || ch->level >= LEVEL_IMMORTAL || IS_SET (ch->actflags, PLR_KILLER))
    return;

  ch->send_to_char ("*** You are now a KILLER!! ***\r\n");
  SET_BIT (ch->actflags, PLR_KILLER);
  ch->save_char_obj();
  return;
}

/*
 * Check for parry.
 */
bool check_parry (Character * ch, Character * victim)
{
  int chance;

  if (!victim->is_awake ())
    return false;

  if (victim->is_npc ()) {
    /* Tuan was here.  :) */
    chance = std::min (60, 2 * victim->level);
  } else {
    if (victim->get_eq_char (WEAR_WIELD) == NULL)
      return false;
    chance = victim->pcdata->learned[skill_lookup("parry")] / 2;
  }

  if (number_percent () >= chance + victim->level - ch->level)
    return false;

  ch->act ("You parry $n's attack.", NULL, victim, TO_VICT);
  ch->act ("$N parries your attack.", NULL, victim, TO_CHAR);
  return true;
}

/*
 * Check for dodge.
 */
bool check_dodge (Character * ch, Character * victim)
{
  int chance;

  if (!victim->is_awake ())
    return false;

  if (victim->is_npc ())
    /* Tuan was here.  :) */
    chance = std::min (60, 2 * victim->level);
  else
    chance = victim->pcdata->learned[skill_lookup("dodge")] / 2;

  if (number_percent () >= chance + victim->level - ch->level)
    return false;

  ch->act ("You dodge $n's attack.", NULL, victim, TO_VICT);
  ch->act ("$N dodges your attack.", NULL, victim, TO_CHAR);
  return true;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse (Character * ch)
{
  char buf[MAX_STRING_LENGTH];
  Object *corpse;
  Object *obj;
  std::string name;

  if (ch->is_npc ()) {
    name = ch->short_descr;
    corpse = get_obj_index(OBJ_VNUM_CORPSE_NPC)->create_object(0);
    corpse->timer = number_range (2, 4);
    if (ch->gold > 0) {
      create_money(ch->gold)->obj_to_obj(corpse);
      ch->gold = 0;
    }
  } else {
    name = ch->name;
    corpse = get_obj_index (OBJ_VNUM_CORPSE_PC)->create_object(0);
    corpse->timer = number_range (25, 40);
  }

  snprintf (buf, sizeof buf, corpse->short_descr.c_str(), name.c_str());
  corpse->short_descr = buf;

  snprintf (buf, sizeof buf, corpse->description.c_str(), name.c_str());
  corpse->description = buf;

  ObjIter o, onext;
  for (o = ch->carrying.begin(); o != ch->carrying.end(); o = onext) {
    obj = *o;
    onext = ++o;
    obj->obj_from_char();
    if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
      obj->extract_obj ();
    else
      obj->obj_to_obj(corpse);
  }

  corpse->obj_to_room (ch->in_room);
  return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry (Character * ch)
{
  Room *was_in_room;
  const char *msg;
  int door;
  int vnum;

  vnum = 0;
  switch (number_range (0, 15)) {
  default:
    msg = "You hear $n's death cry.";
    break;
  case 0:
    msg = "$n hits the ground ... DEAD.";
    break;
  case 1:
    msg = "$n splatters blood on your armor.";
    break;
  case 2:
    msg = "You smell $n's sphincter releasing in death.";
    vnum = OBJ_VNUM_FINAL_TURD;
    break;
  case 3:
    msg = "$n's severed head plops on the ground.";
    vnum = OBJ_VNUM_SEVERED_HEAD;
    break;
  case 4:
    msg = "$n's heart is torn from $s chest.";
    vnum = OBJ_VNUM_TORN_HEART;
    break;
  case 5:
    msg = "$n's arm is sliced from $s dead body.";
    vnum = OBJ_VNUM_SLICED_ARM;
    break;
  case 6:
    msg = "$n's leg is sliced from $s dead body.";
    vnum = OBJ_VNUM_SLICED_LEG;
    break;
  }

  ch->act (msg, NULL, NULL, TO_ROOM);

  if (vnum != 0) {
    char buf[MAX_STRING_LENGTH];
    Object *obj;
    std::string name;

    name = ch->is_npc () ? ch->short_descr : ch->name;
    obj = get_obj_index(vnum)->create_object(0);
    obj->timer = number_range (4, 7);

    snprintf (buf, sizeof buf, obj->short_descr.c_str(), name.c_str());
    obj->short_descr = buf;

    snprintf (buf, sizeof buf, obj->description.c_str(), name.c_str());
    obj->description = buf;

    obj->obj_to_room (ch->in_room);
  }

  if (ch->is_npc ())
    msg = "You hear something's death cry.";
  else
    msg = "You hear someone's death cry.";

  was_in_room = ch->in_room;
  for (door = 0; door <= 5; door++) {
    Exit *pexit;

    if ((pexit = was_in_room->exit[door]) != NULL
      && pexit->to_room != NULL && pexit->to_room != was_in_room) {
      ch->in_room = pexit->to_room;
      ch->act (msg, NULL, NULL, TO_ROOM);
    }
  }
  ch->in_room = was_in_room;

  return;
}

void raw_kill (Character * victim)
{
  victim->stop_fighting(true);
  mprog_death_trigger (victim);
  make_corpse (victim);

  if (victim->is_npc ()) {
    victim->pIndexData->killed++;
    kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
    victim->extract_char (true);
    return;
  }

  victim->extract_char (false);
  while (victim->affected.begin() != victim->affected.end())
    victim->affect_remove (*victim->affected.begin());
  victim->affected_by = 0;
  victim->armor = 100;
  victim->position = POS_RESTING;
  victim->hit = std::max (1, victim->hit);
  victim->mana = std::max (1, victim->mana);
  victim->move = std::max (1, victim->move);
  victim->save_char_obj();
  return;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute (Character * gch, Character * victim)
{
  int align;
  int xp;
  int extra;
  int level;
  int number;

  xp = 300 - URANGE (-3, gch->level - victim->level, 6) * 50;
  align = gch->alignment - victim->alignment;

  if (align > 500) {
    gch->alignment = std::min (gch->alignment + (align - 500) / 4, 1000);
    xp = 5 * xp / 4;
  } else if (align < -500) {
    gch->alignment = std::max (gch->alignment + (align + 500) / 4, -1000);
  } else {
    gch->alignment -= gch->alignment / 4;
    xp = 3 * xp / 4;
  }

  /*
   * Adjust for popularity of target:
   *   -1/8 for each target over  'par' (down to -100%)
   *   +1/8 for each target under 'par' (  up to + 25%)
   */
  level = URANGE (0, victim->level, MAX_LEVEL - 1);
  number = std::max (1, kill_table[level].number);
  extra = victim->pIndexData->killed - kill_table[level].killed / number;
  xp -= xp * URANGE (-2, extra, 8) / 8;

  xp = number_range (xp * 3 / 4, xp * 5 / 4);
  xp = std::max (0, xp);

  return xp;
}

void group_gain (Character * ch, Character * victim)
{
  char buf[MAX_STRING_LENGTH];
  Character *lch;
  int xp;
  int members;

  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if (ch->is_npc () || !victim->is_npc () || victim == ch)
    return;

  members = 0;
  CharIter gch;
  for (gch = ch->in_room->people.begin(); gch != ch->in_room->people.end(); gch++) {
    if (is_same_group (*gch, ch))
      members++;
  }

  if (members == 0) {
    bug_printf ("Group_gain: members.", members);
    members = 1;
  }

  lch = (ch->leader != NULL) ? ch->leader : ch;

  for (gch = ch->in_room->people.begin(); gch != ch->in_room->people.end(); gch++) {
    Object *obj;

    if (!is_same_group (*gch, ch))
      continue;

    if ((*gch)->level - lch->level >= 6) {
      (*gch)->send_to_char ("You are too high for this group.\r\n");
      continue;
    }

    if ((*gch)->level - lch->level <= -6) {
      (*gch)->send_to_char ("You are too low for this group.\r\n");
      continue;
    }

    xp = xp_compute (*gch, victim) / members;
    snprintf (buf, sizeof buf, "You receive %d experience points.\r\n", xp);
    (*gch)->send_to_char (buf);
    (*gch)->gain_exp(xp);

    ObjIter o, onext;
    for (o = ch->carrying.begin(); o != ch->carrying.end(); o = onext) {
      obj = *o;
      onext = ++o;
      if (obj->wear_loc == WEAR_NONE)
        continue;

      if ((obj->is_obj_stat(ITEM_ANTI_EVIL) && ch->is_evil ())
        || (obj->is_obj_stat(ITEM_ANTI_GOOD) && ch->is_good ())
        || (obj->is_obj_stat(ITEM_ANTI_NEUTRAL) && ch->is_neutral ())) {
        ch->act ("You are zapped by $p.", obj, NULL, TO_CHAR);
        ch->act ("$n is zapped by $p.", obj, NULL, TO_ROOM);
        obj->obj_from_char();
        obj->obj_to_room(ch->in_room);
      }
    }
  }

  return;
}

void dam_message (Character * ch, Character * victim, int dam, int dt)
{
  static const char * attack_table[] = {
    "hit",
    "slice", "stab", "slash", "whip", "claw",
    "blast", "pound", "crush", "grep", "bite",
    "pierce", "suction"
  };

  char buf1[256], buf2[256], buf3[256];
  const char *vs;
  const char *vp;
  std::string attack;
  char punct;

  if (dam == 0) {
    vs = "miss";
    vp = "misses";
  } else if (dam <= 4) {
    vs = "scratch";
    vp = "scratches";
  } else if (dam <= 8) {
    vs = "graze";
    vp = "grazes";
  } else if (dam <= 12) {
    vs = "hit";
    vp = "hits";
  } else if (dam <= 16) {
    vs = "injure";
    vp = "injures";
  } else if (dam <= 20) {
    vs = "wound";
    vp = "wounds";
  } else if (dam <= 24) {
    vs = "maul";
    vp = "mauls";
  } else if (dam <= 28) {
    vs = "decimate";
    vp = "decimates";
  } else if (dam <= 32) {
    vs = "devastate";
    vp = "devastates";
  } else if (dam <= 36) {
    vs = "maim";
    vp = "maims";
  } else if (dam <= 40) {
    vs = "MUTILATE";
    vp = "MUTILATES";
  } else if (dam <= 44) {
    vs = "DISEMBOWEL";
    vp = "DISEMBOWELS";
  } else if (dam <= 48) {
    vs = "EVISCERATE";
    vp = "EVISCERATES";
  } else if (dam <= 52) {
    vs = "MASSACRE";
    vp = "MASSACRES";
  } else if (dam <= 100) {
    vs = "*** DEMOLISH ***";
    vp = "*** DEMOLISHES ***";
  } else {
    vs = "*** ANNIHILATE ***";
    vp = "*** ANNIHILATES ***";
  }

  punct = (dam <= 24) ? '.' : '!';

  if (dt == TYPE_HIT) {
    snprintf (buf1, sizeof buf1, "$n %s $N%c", vp, punct);
    snprintf (buf2, sizeof buf2, "You %s $N%c", vs, punct);
    snprintf (buf3, sizeof buf3, "$n %s you%c", vp, punct);
  } else {
    if (dt >= 0 && dt < MAX_SKILL)
      attack = skill_table[dt].noun_damage;
    else if (dt >= TYPE_HIT
      && dt < (int) (TYPE_HIT + sizeof (attack_table) / sizeof (attack_table[0])))
      attack = attack_table[dt - TYPE_HIT];
    else {
      bug_printf ("Dam_message: bad dt %d.", dt);
      attack = attack_table[0];
    }

    snprintf (buf1, sizeof buf1, "$n's %s %s $N%c", attack.c_str(), vp, punct);
    snprintf (buf2, sizeof buf2, "Your %s %s $N%c", attack.c_str(), vp, punct);
    snprintf (buf3, sizeof buf3, "$n's %s %s you%c", attack.c_str(), vp, punct);
  }

  ch->act (buf1, NULL, victim, TO_NOTVICT);
  ch->act (buf2, NULL, victim, TO_CHAR);
  ch->act (buf3, NULL, victim, TO_VICT);

  return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm (Character * ch, Character * victim)
{
  Object *obj;

  if ((obj = victim->get_eq_char (WEAR_WIELD)) == NULL)
    return;

  if (ch->get_eq_char (WEAR_WIELD) == NULL && number_percent() <= 50)
    return;

  ch->act ("$n DISARMS you!", NULL, victim, TO_VICT);
  ch->act ("You disarm $N!", NULL, victim, TO_CHAR);
  ch->act ("$n DISARMS $N!", NULL, victim, TO_NOTVICT);

  obj->obj_from_char();
  if (victim->is_npc ())
    obj->obj_to_char (victim);
  else
    obj->obj_to_room (victim->in_room);

  return;
}

/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip (Character * ch, Character * victim)
{
  if (victim->wait == 0) {
    ch->act ("$n trips you and you go down!", NULL, victim, TO_VICT);
    ch->act ("You trip $N and $N goes down!", NULL, victim, TO_CHAR);
    ch->act ("$n trips $N and $N goes down!", NULL, victim, TO_NOTVICT);

    ch->wait_state (2 * PULSE_VIOLENCE);
    victim->wait_state (2 * PULSE_VIOLENCE);
    victim->position = POS_RESTING;
  }

  return;
}

/*
 * Inflict damage from a hit.
 */
void damage (Character * ch, Character * victim, int dam, int dt)
{
  if (victim->position == POS_DEAD)
    return;

  /*
   * Stop up any residual loopholes.
   */
  if (dam > 1000) {
    bug_printf ("Damage: %d: more than 1000 points!", dam);
    dam = 1000;
  }

  if (victim != ch) {
    /*
     * Certain attacks are forbidden.
     * Most other attacks are returned.
     */
    if (is_safe (ch, victim))
      return;
    check_killer (ch, victim);

    if (victim->position > POS_STUNNED) {
      if (victim->fighting == NULL)
        victim->set_fighting(ch);
      victim->position = POS_FIGHTING;
    }

    if (victim->position > POS_STUNNED) {
      if (ch->fighting == NULL)
        ch->set_fighting(victim);

      /*
       * If victim is charmed, ch might attack victim's master.
       */
      if (ch->is_npc ()
        && victim->is_npc ()
        && victim->is_affected (AFF_CHARM)
        && victim->master != NULL
        && victim->master->in_room == ch->in_room && number_range(0, 7) == 0) {
        ch->stop_fighting(false);
        multi_hit (ch, victim->master, TYPE_UNDEFINED);
        return;
      }
    }

    /*
     * More charm stuff.
     */
    if (victim->master == ch)
      victim->stop_follower();

    /*
     * Inviso attacks ... not.
     */
    if (ch->is_affected (AFF_INVISIBLE)) {
      ch->affect_strip (skill_lookup("invis"));
      ch->affect_strip (skill_lookup("mass invis"));
      REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
      ch->act ("$n fades into existence.", NULL, NULL, TO_ROOM);
    }

    /*
     * Damage modifiers.
     */
    if (victim->is_affected (AFF_SANCTUARY))
      dam /= 2;

    if (victim->is_affected (AFF_PROTECT) && ch->is_evil ())
      dam -= dam / 4;

    if (dam < 0)
      dam = 0;

    /*
     * Check for disarm, trip, parry, and dodge.
     */
    if (dt >= TYPE_HIT) {
      if (ch->is_npc () && number_percent () < ch->level / 2)
        disarm (ch, victim);
      if (ch->is_npc () && number_percent () < ch->level / 2)
        trip (ch, victim);
      if (check_parry (ch, victim))
        return;
      if (check_dodge (ch, victim))
        return;
    }

    dam_message (ch, victim, dam, dt);
  }

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
  victim->hit -= dam;
  if (!victim->is_npc ()
    && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  victim->update_pos();

  switch (victim->position) {
  case POS_MORTAL:
    victim->act ("$n is mortally wounded, and will die soon, if not aided.",
      NULL, NULL, TO_ROOM);
    victim->send_to_char
      ("You are mortally wounded, and will die soon, if not aided.\r\n");
    break;

  case POS_INCAP:
    victim->act ("$n is incapacitated and will slowly die, if not aided.",
      NULL, NULL, TO_ROOM);
    victim->send_to_char
      ("You are incapacitated and will slowly die, if not aided.\r\n");
    break;

  case POS_STUNNED:
    victim->act ("$n is stunned, but will probably recover.",
      NULL, NULL, TO_ROOM);
    victim->send_to_char ("You are stunned, but will probably recover.\r\n");
    break;

  case POS_DEAD:
    victim->act ("$n is DEAD!!", 0, 0, TO_ROOM);
    victim->send_to_char ("You have been KILLED!!\r\n\r\n");
    break;

  default:
    if (dam > victim->max_hit / 4)
      victim->send_to_char ("That really did HURT!\r\n");
    if (victim->hit < victim->max_hit / 4)
      victim->send_to_char ("You sure are BLEEDING!\r\n");
    break;
  }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if (!victim->is_awake ())
    victim->stop_fighting(false);

  /*
   * Payoff for killing things.
   */
  if (victim->position == POS_DEAD) {
    group_gain (ch, victim);

    if (!victim->is_npc ()) {
      log_printf ("%s killed by %s at %d", victim->name.c_str(),
        (ch->is_npc () ? ch->short_descr.c_str() : ch->name.c_str()), victim->in_room->vnum);

      /*
       * Dying penalty:
       * 1/2 way back to previous level.
       */
      if (victim->exp > 1000 * victim->level)
        victim->gain_exp((1000 * victim->level - victim->exp) / 2);
    }

    raw_kill (victim);

    if (!ch->is_npc () && victim->is_npc ()) {
      if (IS_SET (ch->actflags, PLR_AUTOLOOT))
        ch->do_get ("all corpse");
      else
        ch->do_look ("in corpse");

      if (IS_SET (ch->actflags, PLR_AUTOSAC))
        ch->do_sacrifice ("corpse");
    }

    return;
  }

  if (victim == ch)
    return;

  /*
   * Take care of link dead people.
   */
  if (!victim->is_npc () && victim->desc == NULL) {
    if (number_range (0, victim->wait) == 0) {
      victim->do_recall ("");
      return;
    }
  }

  /*
   * Wimp out?
   */
  if (victim->is_npc () && dam > 0) {
    if ((IS_SET (victim->actflags, ACT_WIMPY) && number_percent() <= 50
        && victim->hit < victim->max_hit / 2)
      || (victim->is_affected (AFF_CHARM) && victim->master != NULL
        && victim->master->in_room != victim->in_room))
      victim->do_flee ("");
  }

  if (!victim->is_npc ()
    && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0)
    victim->do_flee ("");

  return;
}

/*
 * Hit one guy once.
 */
void one_hit (Character * ch, Character * victim, int dt)
{
  Object *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

  /*
   * Figure out the type of damage message.
   */
  wield = ch->get_eq_char (WEAR_WIELD);
  if (dt == TYPE_UNDEFINED) {
    dt = TYPE_HIT;
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dt += wield->value[3];
  }

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if (ch->is_npc ()) {
    thac0_00 = 20;
    thac0_32 = 0;
  } else {
    thac0_00 = class_table[ch->klass].thac0_00;
    thac0_32 = class_table[ch->klass].thac0_32;
  }
  thac0 = interpolate (ch->level, thac0_00, thac0_32) - ch->get_hitroll();
  victim_ac = std::max (-15, victim->get_ac() / 10);
  if (!ch->can_see(victim))
    victim_ac -= 4;

  /*
   * The moment of excitement!
   */
  int diceroll = number_range(0, 19);
  if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac)) {
    /* Miss. */
    damage (ch, victim, 0, dt);
    return;
  }

  /*
   * Hit.
   * Calc damage.
   */
  if (ch->is_npc ()) {
    dam = number_range (ch->level / 2, ch->level * 3 / 2);
    if (wield != NULL)
      dam += dam / 2;
  } else {
    if (wield != NULL)
      dam = number_range (wield->value[1], wield->value[2]);
    else
      dam = number_range (1, 4);
  }

  /*
   * Bonuses.
   */
  dam += ch->get_damroll();
  int enh = skill_lookup("enhanced damage");
  if (!ch->is_npc () && ch->pcdata->learned[enh] > 0)
    dam += dam * ch->pcdata->learned[enh] / 150;
  if (!victim->is_awake ())
    dam *= 2;
  if (dt == skill_lookup("backstab"))
    dam *= 2 + ch->level / 8;

  if (dam <= 0)
    dam = 1;

  damage (ch, victim, dam, dt);
  return;
}

/*
 * Do one group of attacks.
 */
void multi_hit (Character * ch, Character * victim, int dt)
{
  int chance;

  one_hit (ch, victim, dt);
  if (ch->fighting != victim || dt == skill_lookup("backstab"))
    return;

  chance =
    ch->is_npc () ? ch->level : ch->pcdata->learned[skill_lookup("second attack")] / 2;
  if (number_percent () < chance) {
    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
      return;
  }

  chance =
    ch->is_npc () ? ch->level : ch->pcdata->learned[skill_lookup("third attack")] / 4;
  if (number_percent () < chance) {
    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
      return;
  }

  chance = ch->is_npc () ? ch->level / 2 : 0;
  if (number_percent () < chance)
    one_hit (ch, victim, dt);

  return;
}

/*
 * Utter mystical words for an sn.
 */
void say_spell (Character * ch, int sn)
{
  std::string mwords, buf, buf2;
  const char *pName;
  int iSyl;
  int length;

  struct syl_type {
    const char *old;
    const char *newsyl;
  };

  static const struct syl_type syl_table[] = {
    {" ", " "},
    {"ar", "abra"},
    {"au", "kada"},
    {"bless", "fido"},
    {"blind", "nose"},
    {"bur", "mosa"},
    {"cu", "judi"},
    {"de", "oculo"},
    {"en", "unso"},
    {"light", "dies"},
    {"lo", "hi"},
    {"mor", "zak"},
    {"move", "sido"},
    {"ness", "lacri"},
    {"ning", "illa"},
    {"per", "duda"},
    {"ra", "gru"},
    {"re", "candus"},
    {"son", "sabru"},
    {"tect", "infra"},
    {"tri", "cula"},
    {"ven", "nofo"},
    {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
    {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
    {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
    {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
    {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
    {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
    {"y", "l"}, {"z", "k"},
    {"", ""}
  };

  for (pName = skill_table[sn].name; *pName != '\0'; pName += length) {
    for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++) {
      if (!str_prefix (syl_table[iSyl].old, pName)) {
        mwords.append(syl_table[iSyl].newsyl);
        break;
      }
    }

    if (length == 0)
      length = 1;
  }

  buf = "$n utters the words, '";
  buf.append(skill_table[sn].name);
  buf.append("'.");
  buf2 = "$n utters the words, '";
  buf2.append(mwords);
  buf2.append("'.");

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if (*rch != ch)
      ch->act ( (ch->klass == (*rch)->klass ? buf : buf2).c_str(), NULL, *rch, TO_VICT);
  }

  return;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell (int sn, int level, Character * ch, Character * victim,
  Object * obj)
{
  void *vo;

  if (sn <= 0)
    return;

  if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0) {
    bug_printf ("Obj_cast_spell: bad sn %d.", sn);
    return;
  }

  switch (skill_table[sn].target) {
  default:
    bug_printf ("Obj_cast_spell: bad target for sn %d.", sn);
    return;

  case TAR_IGNORE:
    vo = NULL;
    break;

  case TAR_CHAR_OFFENSIVE:
    if (victim == NULL)
      victim = ch->fighting;
    if (victim == NULL || !victim->is_npc ()) {
      ch->send_to_char ("You can't do that.\r\n");
      return;
    }
    vo = (void *) victim;
    break;

  case TAR_CHAR_DEFENSIVE:
    if (victim == NULL)
      victim = ch;
    vo = (void *) victim;
    break;

  case TAR_CHAR_SELF:
    vo = (void *) ch;
    break;

  case TAR_OBJ_INV:
    if (obj == NULL) {
      ch->send_to_char ("You can't do that.\r\n");
      return;
    }
    vo = (void *) obj;
    break;
  }

  target_name = "";
  (ch->*(skill_table[sn].spell_fun)) (sn, level, vo);

  if (skill_table[sn].target == TAR_CHAR_OFFENSIVE && victim->master != ch) {
    Character *vch;

    CharIter rch, next;
    for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch = next) {
      vch = *rch;
      next = ++rch;
      if (victim == vch && victim->fighting == NULL) {
        multi_hit (victim, ch, TYPE_UNDEFINED);
        break;
      }
    }
  }

  return;
}

/* Snarf a MOBprogram section from the area file.
 */
void load_mobprogs (std::ifstream & fp)
{
  char letter;
  MobPrototype *iMob;
  int value;
  MobProgram *original;
  MobProgram *working;

  for (;;)
    switch (letter = fread_letter (fp)) {
    default:
      fatal_printf ("Load_mobprogs: bad command '%c'.", letter);
      break;
    case 'S':
    case 's':
      fread_to_eol (fp);
      return;
    case '*':
      fread_to_eol (fp);
      break;
    case 'M':
    case 'm':
      value = fread_number (fp);
      if ((iMob = get_mob_index (value)) == NULL) {
        fatal_printf ("Load_mobprogs: vnum %d doesnt exist", value);
      }

      if ((original = iMob->mobprogs) != NULL)
        for ( ; original->next != NULL; original = original->next) ;
      working = new MobProgram();
      if (original)
        original->next = working;
      else
        iMob->mobprogs = working;
      working = mprog_file_read (fread_word (fp), working, iMob);
      working->next = NULL;
      fread_to_eol (fp);
      break;
    }
}

void mprog_percent_check (Character * mob, Character * actor, Object * obj,
  void *vo, int type)
{
  MobProgram *mprg;

  for (mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next)
    if ((mprg->type & type)
      && (number_percent () < std::atoi (mprg->arglist.c_str()))) {
      mprog_driver (mprg->comlist, mob, actor, obj, vo);
      if (type != GREET_PROG && type != ALL_GREET_PROG)
        break;
    }

  return;

}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger (const std::string & buf, Character * mob, Character * ch,
  Object * obj, void *vo)
{
  if (mob == NULL || ch == NULL)
    return;

  MobProgramActList *tmp_act;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & ACT_PROG)) {
    tmp_act = new MobProgramActList();
    if (mob->mpactnum > 0)
      tmp_act->next = mob->mpact->next;
    else
      tmp_act->next = NULL;

    mob->mpact = tmp_act;
    mob->mpact->buf = buf;
    mob->mpact->ch = ch;
    mob->mpact->obj = obj;
    mob->mpact->vo = vo;
    mob->mpactnum++;

  }
  return;

}

void mprog_bribe_trigger (Character * mob, Character * ch, int amount)
{
  if (mob == NULL || ch == NULL)
    return;

  char buf[MAX_STRING_LENGTH];
  MobProgram *mprg;
  Object *obj;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & BRIBE_PROG)) {
    obj = get_obj_index (OBJ_VNUM_MONEY_SOME)->create_object(0);
    snprintf (buf, sizeof buf, obj->short_descr.c_str(), amount);
    obj->short_descr = buf;
    obj->value[0] = amount;
    obj->obj_to_char (mob);
    mob->gold -= amount;

    for (mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next)
      if ((mprg->type & BRIBE_PROG)
        && (amount >= std::atoi (mprg->arglist.c_str()))) {
        mprog_driver (mprg->comlist, mob, ch, obj, NULL);
        break;
      }
  }

  return;

}

void mprog_death_trigger (Character * mob)
{
  if (mob == NULL)
    return;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & DEATH_PROG)) {
    mprog_percent_check (mob, NULL, NULL, NULL, DEATH_PROG);
  }

  death_cry (mob);
  return;

}

void mprog_entry_trigger (Character * mob)
{
  if (mob == NULL)
    return;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & ENTRY_PROG))
    mprog_percent_check (mob, NULL, NULL, NULL, ENTRY_PROG);

  return;

}

void mprog_fight_trigger (Character * mob, Character * ch)
{
  if (mob == NULL || ch == NULL)
    return;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & FIGHT_PROG))
    mprog_percent_check (mob, ch, NULL, NULL, FIGHT_PROG);

  return;

}

void mprog_give_trigger (Character * mob, Character * ch, Object * obj)
{
  if (mob == NULL || ch == NULL || obj == NULL)
    return;

  std::string buf;
  MobProgram *mprg;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & GIVE_PROG))
    for (mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next) {
      one_argument (mprg->arglist, buf);
      if ((mprg->type & GIVE_PROG)
        && ((!str_cmp (obj->name, mprg->arglist))
          || (!str_cmp ("all", buf)))) {
        mprog_driver (mprg->comlist, mob, ch, obj, NULL);
        break;
      }
    }

  return;

}

void mprog_greet_trigger (Character * mob)
{
  if (mob == NULL)
    return;

  Room* rm = mob->in_room;
  Character* vmob;
  CharIter v;
  for (v = rm->people.begin(); v != rm->people.end(); v++) {
    vmob = *v;
    if (vmob->is_npc ()
      && (vmob->fighting == NULL)
      && vmob->is_awake ()) {
      if (mob != vmob && vmob->can_see(mob)
        && (vmob->pIndexData->progtypes & GREET_PROG)) {
        mprog_percent_check (vmob, mob, NULL, NULL, GREET_PROG);
      } else if (vmob->pIndexData->progtypes & ALL_GREET_PROG) {
        mprog_percent_check (vmob, mob, NULL, NULL, ALL_GREET_PROG);
      }
    }
  }
  return;
}

void mprog_hitprcnt_trigger (Character * mob, Character * ch)
{
  if (mob == NULL || ch == NULL)
    return;
  MobProgram *mprg;

  if (mob->is_npc ()
    && (mob->pIndexData->progtypes & HITPRCNT_PROG))
    for (mprg = mob->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next)
      if ((mprg->type & HITPRCNT_PROG)
        && ((100 * mob->hit / mob->max_hit) < std::atoi (mprg->arglist.c_str()))) {
        mprog_driver (mprg->comlist, mob, ch, NULL, NULL);
        break;
      }

  return;

}

void mprog_random_trigger (Character * mob)
{
  if (mob == NULL)
    return;

  if (mob->pIndexData->progtypes & RAND_PROG)
    mprog_percent_check (mob, NULL, NULL, NULL, RAND_PROG);

  return;

}

void mprog_speech_trigger (const std::string & txt, Character * mob)
{
  if (mob == NULL)
    return;

  Room* rm = mob->in_room;
  Character* vmob;
  CharIter v;
  for (v = rm->people.begin(); v != rm->people.end(); v++) {
    vmob = *v;
    if (vmob->is_npc () && (vmob->pIndexData->progtypes & SPEECH_PROG))
      mprog_wordlist_check (txt, vmob, mob, NULL, NULL, SPEECH_PROG);
  }

  return;

}

/*
 * Core procedure for dragons.
 */
bool dragon (Character * ch, const char *spell_name)
{
  Character *victim = NULL;
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if ((*rch)->fighting == ch && number_percent() <= 25) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  if ((sn = skill_lookup (spell_name)) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, victim);
  return true;
}

/*
 * Special procedures for mobiles.
 */
bool spec_breath_any (Character * ch)
{
  if (ch->position != POS_FIGHTING)
    return false;

  switch (number_range(0, 7)) {
  case 0:
    return spec_breath_fire (ch);
  case 1:
  case 2:
    return spec_breath_lightning (ch);
  case 3:
    return spec_breath_gas (ch);
  case 4:
    return spec_breath_acid (ch);
  case 5:
  case 6:
  case 7:
    return spec_breath_frost (ch);
  }

  return false;
}

bool spec_breath_acid (Character * ch)
{
  return dragon (ch, "acid breath");
}

bool spec_breath_fire (Character * ch)
{
  return dragon (ch, "fire breath");
}

bool spec_breath_frost (Character * ch)
{
  return dragon (ch, "frost breath");
}

bool spec_breath_gas (Character * ch)
{
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  if ((sn = skill_lookup ("gas breath")) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, NULL);
  return true;
}

bool spec_breath_lightning (Character * ch)
{
  return dragon (ch, "lightning breath");
}

bool spec_cast_adept (Character * ch)
{
  Character *victim = NULL;

  if (!ch->is_awake ())
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if (*rch != ch && ch->can_see(*rch) && number_percent() <= 50) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  switch (number_range(0, 7)) {
  case 0:
    ch->act ("$n utters the word 'tehctah'.", NULL, NULL, TO_ROOM);
    ch->spell_armor (skill_lookup ("armor"), ch->level, victim);
    return true;

  case 1:
    ch->act ("$n utters the word 'nhak'.", NULL, NULL, TO_ROOM);
    ch->spell_bless (skill_lookup ("bless"), ch->level, victim);
    return true;

  case 2:
    ch->act ("$n utters the word 'yeruf'.", NULL, NULL, TO_ROOM);
    ch->spell_cure_blindness (skill_lookup ("cure blindness"),
      ch->level, victim);
    return true;

  case 3:
    ch->act ("$n utters the word 'garf'.", NULL, NULL, TO_ROOM);
    ch->spell_cure_light (skill_lookup ("cure light"), ch->level, victim);
    return true;

  case 4:
    ch->act ("$n utters the words 'rozar'.", NULL, NULL, TO_ROOM);
    ch->spell_cure_poison (skill_lookup ("cure poison"), ch->level, victim);
    return true;

  case 5:
    ch->act ("$n utters the words 'nadroj'.", NULL, NULL, TO_ROOM);
    ch->spell_refresh (skill_lookup ("refresh"), ch->level, victim);
    return true;

  }

  return false;
}

bool spec_cast_cleric (Character * ch)
{
  Character *victim = NULL;
  const char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if ((*rch)->fighting == ch && number_percent() <= 50) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  for (;;) {
    int min_level;

    switch (number_range(0, 15)) {
    case 0:
      min_level = 0;
      spell = "blindness";
      break;
    case 1:
      min_level = 3;
      spell = "cause serious";
      break;
    case 2:
      min_level = 7;
      spell = "earthquake";
      break;
    case 3:
      min_level = 9;
      spell = "cause critical";
      break;
    case 4:
      min_level = 10;
      spell = "dispel evil";
      break;
    case 5:
      min_level = 12;
      spell = "curse";
      break;
    case 6:
      min_level = 12;
      spell = "change sex";
      break;
    case 7:
      min_level = 13;
      spell = "flamestrike";
      break;
    case 8:
    case 9:
    case 10:
      min_level = 15;
      spell = "harm";
      break;
    default:
      min_level = 16;
      spell = "dispel magic";
      break;
    }

    if (ch->level >= min_level)
      break;
  }

  if ((sn = skill_lookup (spell)) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, victim);
  return true;
}

bool spec_cast_judge (Character * ch)
{
  Character *victim = NULL;
  const char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if ((*rch)->fighting == ch && number_percent() <= 50) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  spell = "high explosive";
  if ((sn = skill_lookup (spell)) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, victim);
  return true;
}

bool spec_cast_mage (Character * ch)
{
  Character *victim = NULL;
  const char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if ((*rch)->fighting == ch && number_percent() <= 50) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  for (;;) {
    int min_level;

    switch (number_range(0, 15)) {
    case 0:
      min_level = 0;
      spell = "blindness";
      break;
    case 1:
      min_level = 3;
      spell = "chill touch";
      break;
    case 2:
      min_level = 7;
      spell = "weaken";
      break;
    case 3:
      min_level = 8;
      spell = "teleport";
      break;
    case 4:
      min_level = 11;
      spell = "colour spray";
      break;
    case 5:
      min_level = 12;
      spell = "change sex";
      break;
    case 6:
      min_level = 13;
      spell = "energy drain";
      break;
    case 7:
    case 8:
    case 9:
      min_level = 15;
      spell = "fireball";
      break;
    default:
      min_level = 20;
      spell = "acid blast";
      break;
    }

    if (ch->level >= min_level)
      break;
  }

  if ((sn = skill_lookup (spell)) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, victim);
  return true;
}

bool spec_cast_undead (Character * ch)
{
  Character *victim = NULL;
  const char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    if ((*rch)->fighting == ch && number_percent() <= 50) {
      victim = *rch;
      break;
    }
  }

  if (victim == NULL)
    return false;

  for (;;) {
    int min_level;

    switch (number_range(0, 15)) {
    case 0:
      min_level = 0;
      spell = "curse";
      break;
    case 1:
      min_level = 3;
      spell = "weaken";
      break;
    case 2:
      min_level = 6;
      spell = "chill touch";
      break;
    case 3:
      min_level = 9;
      spell = "blindness";
      break;
    case 4:
      min_level = 12;
      spell = "poison";
      break;
    case 5:
      min_level = 15;
      spell = "energy drain";
      break;
    case 6:
      min_level = 18;
      spell = "harm";
      break;
    case 7:
      min_level = 21;
      spell = "teleport";
      break;
    default:
      min_level = 24;
      spell = "gate";
      break;
    }

    if (ch->level >= min_level)
      break;
  }

  if ((sn = skill_lookup (spell)) < 0)
    return false;
  (ch->*(skill_table[sn].spell_fun)) (sn, ch->level, victim);
  return true;
}

bool spec_executioner (Character * ch)
{
  if (!ch->is_awake () || ch->fighting != NULL)
    return false;

  Character *victim = NULL;
  const char *crime = "";
  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {

    if (!(*rch)->is_npc () && IS_SET ((*rch)->actflags, PLR_KILLER)) {
      victim = *rch;
      crime = "KILLER";
      break;
    }

    if (!(*rch)->is_npc () && IS_SET ((*rch)->actflags, PLR_THIEF)) {
      victim = *rch;
      crime = "THIEF";
      break;
    }
  }

  if (victim == NULL)
    return false;

  char buf[MAX_STRING_LENGTH];
  snprintf (buf, sizeof buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
    victim->name.c_str(), crime);
  ch->do_shout (buf);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  get_mob_index(MOB_VNUM_CITYGUARD)->create_mobile()->char_to_room(ch->in_room);
  get_mob_index(MOB_VNUM_CITYGUARD)->create_mobile()->char_to_room(ch->in_room);
  return true;
}

bool spec_fido (Character * ch)
{
  if (!ch->is_awake ())
    return false;

  Object *corpse;
  Object *obj;
  ObjIter c, cnext;
  for (c = ch->in_room->contents.begin(); c != ch->in_room->contents.end(); c = cnext) {
    corpse = *c;
    cnext = ++c;
    if (corpse->item_type != ITEM_CORPSE_NPC)
      continue;

    ch->act ("$n savagely devours a corpse.", NULL, NULL, TO_ROOM);
    ObjIter o, onext;
    for (o = corpse->contains.begin(); o != corpse->contains.end(); o = onext) {
      obj = *o;
      onext = ++o;
      obj->obj_from_obj ();
      obj->obj_to_room (ch->in_room);
    }
    corpse->extract_obj ();
    return true;
  }

  return false;
}

bool spec_guard (Character * ch)
{
  if (!ch->is_awake () || ch->fighting != NULL)
    return false;

  Character *victim = NULL;
  int max_evil = 300;
  Character* ech = NULL;
  const char* crime = "";
  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {

    if (!(*rch)->is_npc () && IS_SET ((*rch)->actflags, PLR_KILLER)) {
      victim = *rch;
      crime = "KILLER";
      break;
    }

    if (!(*rch)->is_npc () && IS_SET ((*rch)->actflags, PLR_THIEF)) {
      victim = *rch;
      crime = "THIEF";
      break;
    }

    if ((*rch)->fighting != NULL
      && (*rch)->fighting != ch && (*rch)->alignment < max_evil) {
      max_evil = (*rch)->alignment;
      ech = *rch;
    }
  }

  if (victim != NULL) {
    char buf[MAX_STRING_LENGTH];
    snprintf (buf, sizeof buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
      victim->name.c_str(), crime);
    ch->do_shout (buf);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return true;
  }

  if (ech != NULL) {
    ch->act ("$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
      NULL, NULL, TO_ROOM);
    multi_hit (ch, ech, TYPE_UNDEFINED);
    return true;
  }

  return false;
}

bool spec_janitor (Character * ch)
{
  Object *trash;

  if (!ch->is_awake ())
    return false;

  ObjIter o, onext;
  for (o = ch->in_room->contents.begin(); o != ch->in_room->contents.end(); o = onext) {
    trash = *o;
    onext = ++o;
    if (!IS_SET (trash->wear_flags, ITEM_TAKE))
      continue;
    if (trash->item_type == ITEM_DRINK_CON
      || trash->item_type == ITEM_TRASH || trash->cost < 10) {
      ch->act ("$n picks up some trash.", NULL, NULL, TO_ROOM);
      trash->obj_from_room ();
      trash->obj_to_char (ch);
      return true;
    }
  }

  return false;
}

bool spec_mayor (Character * ch)
{
  static const char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static const char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path;
  static int pos;
  static bool move;

  if (!move) {
    if (g_world->hour() == 6) {
      path = open_path;
      move = true;
      pos = 0;
    }

    if (g_world->hour() == 20) {
      path = close_path;
      move = true;
      pos = 0;
    }
  }

  if (ch->fighting != NULL)
    return spec_cast_cleric (ch);
  if (!move || ch->position < POS_SLEEPING)
    return false;

  switch (path[pos]) {
  case '0':
  case '1':
  case '2':
  case '3':
    ch->move_char (path[pos] - '0');
    break;

  case 'W':
    ch->position = POS_STANDING;
    ch->act ("$n awakens and groans loudly.", NULL, NULL, TO_ROOM);
    break;

  case 'S':
    ch->position = POS_SLEEPING;
    ch->act ("$n lies down and falls asleep.", NULL, NULL, TO_ROOM);
    break;

  case 'a':
    ch->act ("$n says 'Hello Honey!'", NULL, NULL, TO_ROOM);
    break;

  case 'b':
    ch->act ("$n says 'What a view!  I must do something about that dump!'",
      NULL, NULL, TO_ROOM);
    break;

  case 'c':
    ch->act ("$n says 'Vandals!  Youngsters have no respect for anything!'",
      NULL, NULL, TO_ROOM);
    break;

  case 'd':
    ch->act ("$n says 'Good day, citizens!'", NULL, NULL, TO_ROOM);
    break;

  case 'e':
    ch->act ("$n says 'I hereby declare the city of Midgaard open!'",
      NULL, NULL, TO_ROOM);
    break;

  case 'E':
    ch->act ("$n says 'I hereby declare the city of Midgaard closed!'",
      NULL, NULL, TO_ROOM);
    break;

  case 'O':
    ch->do_unlock ("gate");
    ch->do_open ("gate");
    break;

  case 'C':
    ch->do_close ("gate");
    ch->do_lock ("gate");
    break;

  case '.':
    move = false;
    break;
  }

  pos++;
  return false;
}

bool spec_poison (Character * ch)
{
  Character *victim;

  if (ch->position != POS_FIGHTING
    || (victim = ch->fighting) == NULL || number_percent () > 2 * ch->level)
    return false;

  ch->act ("You bite $N!", NULL, victim, TO_CHAR);
  ch->act ("$n bites $N!", NULL, victim, TO_NOTVICT);
  ch->act ("$n bites you!", NULL, victim, TO_VICT);
  ch->spell_poison (skill_lookup("poison"), ch->level, victim);
  return true;
}

bool spec_thief (Character * ch)
{
  if (ch->position != POS_STANDING)
    return false;

  CharIter rch;
  for (rch = ch->in_room->people.begin(); rch != ch->in_room->people.end(); rch++) {
    Character* victim = *rch;

    if (victim->is_npc() || victim->level >= LEVEL_IMMORTAL ||
      number_percent() <= 75 || !ch->can_see(victim))      /* Thx Glop */
      continue;

    if (victim->is_awake () && number_range (0, ch->level) == 0) {
      ch->act ("You discover $n's hands in your wallet!",
        NULL, victim, TO_VICT);
      ch->act ("$N discovers $n's hands in $S wallet!",
        NULL, victim, TO_NOTVICT);
      return true;
    } else {
      int gold = victim->gold * number_range (1, 20) / 100;
      ch->gold += 7 * gold / 8;
      victim->gold -= gold;
      return true;
    }
  }

  return false;
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */
const char *mprog_type_to_name (int type)
{
  switch (type) {
  case IN_FILE_PROG:
    return "in_file_prog";
  case ACT_PROG:
    return "act_prog";
  case SPEECH_PROG:
    return "speech_prog";
  case RAND_PROG:
    return "rand_prog";
  case FIGHT_PROG:
    return "fight_prog";
  case HITPRCNT_PROG:
    return "hitprcnt_prog";
  case DEATH_PROG:
    return "death_prog";
  case ENTRY_PROG:
    return "entry_prog";
  case GREET_PROG:
    return "greet_prog";
  case ALL_GREET_PROG:
    return "all_greet_prog";
  case GIVE_PROG:
    return "give_prog";
  case BRIBE_PROG:
    return "bribe_prog";
  default:
    return "ERROR_PROG";
  }
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MOBprograms which are set.
 */
void Character::do_mpstat (std::string argument)
{
  char buf[MAX_STRING_LENGTH];
  std::string arg;
  MobProgram *mprg;
  Character *victim;

  one_argument (argument, arg);

  if (arg.empty()) {
    send_to_char ("MobProg stat whom?\r\n");
    return;
  }

  if ((victim = get_char_world (arg)) == NULL) {
    send_to_char ("They aren't here.\r\n");
    return;
  }

  if (!victim->is_npc ()) {
    send_to_char ("Only Mobiles can have Programs!\r\n");
    return;
  }

  if (!(victim->pIndexData->progtypes)) {
    send_to_char ("That Mobile has no Programs set.\r\n");
    return;
  }

  snprintf (buf, sizeof buf, "Name: %s.  Vnum: %d.\r\n",
    victim->name.c_str(), victim->pIndexData->vnum);
  send_to_char (buf);

  snprintf (buf, sizeof buf, "Short description: %s.\r\nLong  description: %s",
    victim->short_descr.c_str(),
    !victim->long_descr.empty() ? victim->long_descr.c_str() : "(none).\r\n");
  send_to_char (buf);

  snprintf (buf, sizeof buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \r\n",
    victim->hit, victim->max_hit,
    victim->mana, victim->max_mana, victim->move, victim->max_move);
  send_to_char (buf);

  snprintf (buf, sizeof buf,
    "Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\r\n",
    victim->level, victim->klass, victim->alignment,
    victim->get_ac(), victim->gold, victim->exp);
  send_to_char (buf);

  for (mprg = victim->pIndexData->mobprogs; mprg != NULL; mprg = mprg->next) {
    snprintf (buf, sizeof buf, ">%s %s\r\n%s\r\n",
      mprog_type_to_name (mprg->type), mprg->arglist.c_str(), mprg->comlist.c_str());
    send_to_char (buf);
  }

  return;

}

/* prints the argument to all the rooms aroud the mobile */
void Character::do_mpasound (std::string argument)
{

  Room *was_in_rm;
  int door;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  if (argument.empty()) {
    bug_printf ("Mpasound - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  was_in_rm = in_room;
  for (door = 0; door <= 5; door++) {
    Exit *pexit;

    if ((pexit = was_in_rm->exit[door]) != NULL
      && pexit->to_room != NULL && pexit->to_room != was_in_rm) {
      in_room = pexit->to_room;
      MOBtrigger = false;
      act (argument, NULL, NULL, TO_ROOM);
    }
  }

  in_room = was_in_rm;
  return;

}

/* lets the mobile kill any player or mobile without murder*/
void Character::do_mpkill (std::string argument)
{
  std::string arg;
  Character *victim;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  one_argument (argument, arg);

  if (arg.empty()) {
    bug_printf ("MpKill - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if ((victim = get_char_room (arg)) == NULL) {
    bug_printf ("MpKill - Victim not in room from vnum %d.", pIndexData->vnum);
    return;
  }

  if (victim == this) {
    bug_printf ("MpKill - Victim is self from vnum %d.", pIndexData->vnum);
    return;
  }

  if (is_affected (AFF_CHARM) && master == victim) {
    bug_printf ("MpKill - Charmed mob attacking master from vnum %d.",
      pIndexData->vnum);
    return;
  }

  if (position == POS_FIGHTING) {
    bug_printf ("MpKill - Already fighting from vnum %d", pIndexData->vnum);
    return;
  }

  multi_hit (this, victim, TYPE_UNDEFINED);
  return;
}

/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */
void Character::do_mpjunk (std::string argument)
{
  std::string arg;
  Object *obj;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  one_argument (argument, arg);

  if (arg.empty()) {
    bug_printf ("Mpjunk - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if (str_cmp (arg, "all") && str_prefix ("all.", arg)) {
    if ((obj = get_obj_wear (arg)) != NULL) {
      unequip_char(obj);
      obj->extract_obj ();
      return;
    }
    if ((obj = get_obj_carry (arg)) == NULL)
      return;
    obj->extract_obj ();
  } else {
    ObjIter o, onext;
    for (o = carrying.begin(); o != carrying.end(); o = onext) {
      obj = *o;
      onext = ++o;
      if (arg[3] == '\0' || is_name (&arg[4], obj->name)) {
        if (obj->wear_loc != WEAR_NONE)
          unequip_char(obj);
        obj->extract_obj ();
      }
    }
  }

  return;

}

/* prints the message to everyone in the room other than the mob and victim */
void Character::do_mpechoaround (std::string argument)
{
  std::string arg;
  Character *victim;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  argument = one_argument (argument, arg);

  if (arg.empty()) {
    bug_printf ("Mpechoaround - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if (!(victim = get_char_room (arg))) {
    bug_printf ("Mpechoaround - Victim does not exist from vnum %d.",
      pIndexData->vnum);
    return;
  }

  act (argument, NULL, victim, TO_NOTVICT);
  return;
}

/* prints the message to only the victim */
void Character::do_mpechoat (std::string argument)
{
  std::string arg;
  Character *victim;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    bug_printf ("Mpechoat - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if (!(victim = get_char_room (arg))) {
    bug_printf ("Mpechoat - Victim does not exist from vnum %d.",
      pIndexData->vnum);
    return;
  }

  act (argument, NULL, victim, TO_VICT);
  return;
}

/* prints the message to the room at large */
void Character::do_mpecho (std::string argument)
{
  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  if (argument.empty()) {
    bug_printf ("Mpecho - Called w/o argument from vnum %d.", pIndexData->vnum);
    return;
  }

  act (argument, NULL, NULL, TO_ROOM);
  return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */
void Character::do_mpmload (std::string argument)
{
  std::string arg;
  MobPrototype *pMobIndex;
  Character *victim;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  one_argument (argument, arg);

  if (arg.empty() || !is_number (arg)) {
    bug_printf ("Mpmload - Bad vnum as arg from vnum %d.", pIndexData->vnum);
    return;
  }

  if ((pMobIndex = get_mob_index (std::atoi (arg.c_str()))) == NULL) {
    bug_printf ("Mpmload - Bad mob vnum from vnum %d.", pIndexData->vnum);
    return;
  }

  victim = pMobIndex->create_mobile ();
  victim->char_to_room(in_room);
  return;
}

void Character::do_mpoload (std::string argument)
{
  std::string arg1, arg2;
  ObjectPrototype *pObjIndex;
  Object *obj;
  int lvl;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty() || !is_number (arg1)) {
    bug_printf ("Mpoload - Bad syntax from vnum %d.", pIndexData->vnum);
    return;
  }

  if (arg2.empty()) {
    lvl = get_trust ();
  } else {
    /*
     * New feature from Alander.
     */
    if (!is_number (arg2)) {
      bug_printf ("Mpoload - Bad syntax from vnum %d.", pIndexData->vnum);
      return;
    }
    lvl = std::atoi (arg2.c_str());
    if (lvl < 0 || lvl > get_trust ()) {
      bug_printf ("Mpoload - Bad level from vnum %d.", pIndexData->vnum);
      return;
    }
  }

  if ((pObjIndex = get_obj_index (std::atoi (arg1.c_str()))) == NULL) {
    bug_printf ("Mpoload - Bad vnum arg from vnum %d.", pIndexData->vnum);
    return;
  }

  obj = pObjIndex->create_object (lvl);
  if (obj->can_wear(ITEM_TAKE)) {
    obj->obj_to_char (this);
  } else {
    obj->obj_to_room (in_room);
  }

  return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */
void Character::do_mppurge (std::string argument)
{
  std::string arg;
  Character *victim;
  Object *obj;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

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

    return;
  }

  if ((victim = get_char_room (arg)) == NULL) {
    if ((obj = get_obj_here (arg))) {
      obj->extract_obj ();
    } else {
      bug_printf ("Mppurge - Bad argument from vnum %d.", pIndexData->vnum);
    }
    return;
  }

  if (!victim->is_npc ()) {
    bug_printf ("Mppurge - Purging a PC from vnum %d.", pIndexData->vnum);
    return;
  }

  victim->extract_char (true);
  return;
}

/* lets the mobile goto any location it wishes that is not private */
void Character::do_mpgoto (std::string argument)
{
  std::string arg;
  Room *location;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  one_argument (argument, arg);
  if (arg.empty()) {
    bug_printf ("Mpgoto - No argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if ((location = find_location (this, arg)) == NULL) {
    bug_printf ("Mpgoto - No such location from vnum %d.", pIndexData->vnum);
    return;
  }

  if (fighting != NULL)
    stop_fighting (true);

  char_from_room();
  char_to_room(location);

  return;
}

/* lets the mobile do a command at another location. Very useful */
void Character::do_mpat (std::string argument)
{
  std::string arg;
  Room *location;
  Room *original;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    bug_printf ("Mpat - Bad argument from vnum %d.", pIndexData->vnum);
    return;
  }

  if ((location = find_location (this, arg)) == NULL) {
    bug_printf ("Mpat - No such location from vnum %d.", pIndexData->vnum);
    return;
  }

  original = in_room;
  char_from_room();
  char_to_room(location);
  interpret (argument);

  /*
   * See if 'this' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  for (CharIter c = char_list.begin(); c != char_list.end(); c++) {
    if (*c == this) {
      char_from_room();
      char_to_room(original);
      break;
    }
  }

  return;
}

/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */
void Character::do_mptransfer (std::string argument)
{
  std::string arg1, arg2;
  Room *location;
  Character *victim;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1.empty()) {
    bug_printf ("Mptransfer - Bad syntax from vnum %d.", pIndexData->vnum);
    return;
  }

  if (!str_cmp (arg1, "all")) {
    for (DescIter d = descriptor_list.begin(); d != descriptor_list.end(); d++) {
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
  if (arg2.empty()) {
    location = in_room;
  } else {
    if ((location = find_location (this, arg2)) == NULL) {
      bug_printf ("Mptransfer - No such location from vnum %d.",
        pIndexData->vnum);
      return;
    }

    if (location->is_private()) {
      bug_printf ("Mptransfer - Private room from vnum %d.", pIndexData->vnum);
      return;
    }
  }

  if ((victim = get_char_world (arg1)) == NULL) {
    bug_printf ("Mptransfer - No such person from vnum %d.", pIndexData->vnum);
    return;
  }

  if (victim->in_room == NULL) {
    bug_printf ("Mptransfer - Victim in Limbo from vnum %d.", pIndexData->vnum);
    return;
  }

  if (victim->fighting != NULL)
    victim->stop_fighting (true);

  victim->char_from_room();
  victim->char_to_room(location);

  return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */
void Character::do_mpforce (std::string argument)
{
  std::string arg;

  if (!is_npc ()) {
    send_to_char ("Huh?\r\n");
    return;
  }

  argument = one_argument (argument, arg);

  if (arg.empty() || argument.empty()) {
    bug_printf ("Mpforce - Bad syntax from vnum %d.", pIndexData->vnum);
    return;
  }

  if (!str_cmp (arg, "all")) {
    Character *vch;

    CharIter c, next;
    for (c = char_list.begin(); c != char_list.end(); c = next) {
      vch = *c;
      next = ++c;
      if (vch->in_room == in_room && vch->get_trust () < get_trust ()
        && can_see(vch)) {
        vch->interpret (argument);
      }
    }
  } else {
    Character *victim;

    if ((victim = get_char_room (arg)) == NULL) {
      bug_printf ("Mpforce - No such victim from vnum %d.", pIndexData->vnum);
      return;
    }

    if (victim == this) {
      bug_printf ("Mpforce - Forcing oneself from vnum %d.", pIndexData->vnum);
      return;
    }

    victim->interpret (argument);
  }

  return;
}

void new_descriptor (void)
{
  char buf[MAX_STRING_LENGTH];
  Descriptor *dnew;
  struct sockaddr_in sock;
  struct hostent *from;
  SOCKET desc;
#ifndef WIN32
  socklen_t size;
#else
  int size;
  unsigned long flags = 1;
#endif

  size = sizeof (sock);
  getsockname (g_listen, (struct sockaddr *) &sock, &size);
  if ((desc = accept (g_listen, (struct sockaddr *) &sock, &size)) == INVALID_SOCKET) {
    std::perror ("New_descriptor: accept");
    return;
  }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

#ifdef WIN32
  if (ioctlsocket (desc, FIONBIO, &flags)) {
#else
  if (fcntl (desc, F_SETFL, FNDELAY) == -1) {
#endif
    std::perror ("New_descriptor: fcntl: FNDELAY");
    return;
  }

  /*
   * Cons a new descriptor.
   */
  dnew = new Descriptor(desc);
  dnew->assign_hostname();

  /*
   * Swiftest: I added the following to ban sites.  I don't
   * endorse banning of sites, but Copper has few descriptors now
   * and some people from certain sites keep abusing access by
   * using automated 'autodialers' and leaving connections hanging.
   *
   * Furey: added suffix check by request of Nickel of HiddenWorlds.
   */
  for (std::list<Ban*>::iterator pban = ban_list.begin();
    pban != ban_list.end(); pban++) {
    if (str_suffix ((*pban)->name, dnew->host)) {
      write_to_descriptor (desc,
        "Your site has been banned from this Mud.\r\n", 0);
      closesocket (desc);
      delete dnew;
      return;
    }
  }

  descriptor_list.push_back(dnew);

  /*
   * Send the greeting.
   */
  dnew->write_to_buffer (help_greeting);
  return;
}

void game_loop (void)
{
  static struct timeval null_time;
  struct timeval last_time;

#ifndef WIN32
  signal (SIGPIPE, SIG_IGN);
#endif
  gettimeofday (&last_time, NULL);
  g_world->set_current_time(last_time.tv_sec);

  /* Main loop */
  while (!merc_down) {
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
#ifdef WIN32
    fd_set dummy_set;
#endif
    SOCKET maxdesc;

    /*
     * Poll all active descriptors.
     */
    FD_ZERO (&in_set);
    FD_ZERO (&out_set);
    FD_ZERO (&exc_set);
    FD_SET (g_listen, &in_set);
#ifdef WIN32
    FD_ZERO (&dummy_set);
    FD_SET (g_listen, &dummy_set);
#endif
    maxdesc = g_listen;
    DescIter d;
    for (d = descriptor_list.begin(); d != descriptor_list.end(); d++) {
      maxdesc = std::max (maxdesc, (*d)->descriptor);
      FD_SET ((*d)->descriptor, &in_set);
      FD_SET ((*d)->descriptor, &out_set);
      FD_SET ((*d)->descriptor, &exc_set);
    }

    if (select (maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) == SOCKET_ERROR) {
      fatal_printf ("Game_loop: select: poll");
    }

    /*
     * New connection?
     */
    if (FD_ISSET (g_listen, &in_set))
      new_descriptor ();

    /*
     * Kick out the freaky folks.
     */
    for (d = descriptor_list.begin(); d != descriptor_list.end(); d = deepdenext) {
      Descriptor* d_this = *d;
      deepdenext = ++d;
      if (FD_ISSET (d_this->descriptor, &exc_set)) {
        FD_CLR (d_this->descriptor, &in_set);
        FD_CLR (d_this->descriptor, &out_set);
        if (d_this->character)
          d_this->character->save_char_obj();
        d_this->outbuf.erase();
        d_this->close_socket();
      }
    }

    /*
     * Process input.
     */
    for (d = descriptor_list.begin(); d != descriptor_list.end(); d = deepdenext) {
      Descriptor* d_this = *d;
      deepdenext = ++d;
      d_this->fcommand = false;

      if (FD_ISSET (d_this->descriptor, &in_set)) {
        if (d_this->character != NULL)
          d_this->character->timer = 0;
        if (!d_this->read_from_descriptor()) {
          FD_CLR (d_this->descriptor, &out_set);
          if (d_this->character != NULL)
            d_this->character->save_char_obj();
          d_this->outbuf.erase();
          d_this->close_socket();
          continue;
        }
      }

      if (d_this->character != NULL && d_this->character->wait > 0) {
        --d_this->character->wait;
        continue;
      }

      d_this->read_from_buffer();
      if (!d_this->incomm.empty()) {
        d_this->fcommand = true;
        if (d_this->character)
          d_this->character->stop_idling();

        if (d_this->connected == CON_PLAYING)
          if (d_this->showstr_point)
            d_this->show_string (d_this->incomm);
          else
            d_this->character->interpret (d_this->incomm);
        else
          d_this->nanny (d_this->incomm);

      }
    }

    /*
     * Autonomous game motion.
     */
    update_handler ();

    /*
     * Output.
     */
    for (d = descriptor_list.begin(); d != descriptor_list.end(); d = deepdenext) {
      Descriptor* d_this = *d;
      deepdenext = ++d;
      if ((d_this->fcommand || !d_this->outbuf.empty())
        && FD_ISSET (d_this->descriptor, &out_set)) {
        if (!d_this->process_output(true)) {
          if (d_this->character != NULL)
            d_this->character->save_char_obj();
          d_this->outbuf.erase();
          d_this->close_socket();
        }
      }
    }

    /*
     * Synchronize to a clock.
     * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
     * Careful here of signed versus unsigned arithmetic.
     */
    {
      struct timeval now_time;
      long secDelta;
      long usecDelta;

      gettimeofday (&now_time, NULL);
      usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
        + 1000000 / PULSE_PER_SECOND;
      secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
      while (usecDelta < 0) {
        usecDelta += 1000000;
        secDelta -= 1;
      }

      while (usecDelta >= 1000000) {
        usecDelta -= 1000000;
        secDelta += 1;
      }

      if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
        struct timeval stall_time;

        stall_time.tv_usec = usecDelta;
        stall_time.tv_sec = secDelta;
#ifdef WIN32   /* windows select demands a valid fd_set */
        if (select (0, NULL, NULL, &dummy_set, &stall_time) == SOCKET_ERROR) {
#else
        if (select (0, NULL, NULL, NULL, &stall_time) == SOCKET_ERROR) {
#endif
          fatal_printf ("Game_loop: select: stall");
        }
      }
    }

    gettimeofday (&last_time, NULL);
    g_world->set_current_time(last_time.tv_sec);
  }

  return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name (const std::string & name)
{
  /*
   * Reserved words.
   */
  if (is_name (name, "all auto immortal self someone"))
    return false;

  /*
   * Length restrictions.
   */
  if (name.size() < 3 || name.size() > 12)
    return false;

  /*
   * Alphanumerics only.
   * Lock out IllIll twits.
   */
  std::string::const_iterator pc;
  bool fIll = true;
  for (pc = name.begin(); pc != name.end(); pc++) {
    if (!isalpha (*pc))
      return false;
    if (tolower (*pc) != 'i' && tolower (*pc) != 'l')
      fIll = false;
  }

  if (fIll)
    return false;

  /*
   * Prevent players from naming themselves after mobs.
   */
  std::map<int,MobPrototype*>::iterator pmob;
  for (pmob = mob_table.begin(); pmob != mob_table.end(); pmob++) {
    if (is_name (name, (*pmob).second->name))
      return false;
  }

  return true;
}

int init_server_socket (void)
{
  SOCKET fd;

  if ((fd = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    fatal_printf("Init_socket: socket");
  }

  int x = 1;
  if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof (x)) == SOCKET_ERROR) {
    closesocket (fd);
    fatal_printf ("Init_socket: SO_REUSEADDR");
  }

#ifdef SO_DONTLINGER
  struct linger ld;
  ld.l_onoff = 1;
  ld.l_linger = 1000;
  if (setsockopt (fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof (ld)) == SOCKET_ERROR) {
    closesocket (fd);
    fatal_printf ("Init_socket: SO_DONTLINGER");
  }
#endif

  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  sa = sa_zero;
  sa.sin_family = AF_INET;
  sa.sin_port = htons (g_port);

  if (bind (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
    closesocket (fd);
    fatal_printf ("Init_socket: bind");
  }

  if (listen (fd, 3) < 0) {
    closesocket (fd);
    fatal_printf ("Init_socket: listen");
  }

  return fd;
}

#if defined(WIN32) && !defined(__DMC__)
void hotboot(void) {
  WSAPROTOCOL_INFO proto_info;
  int count_users = 0;
  std::FILE* fp;
  void* errmsg;

  // Open events created by parent server
  void * file_event = OpenEvent(SYNCHRONIZE, FALSE, "file_created");
  if (file_event == NULL) {
    win_errprint("Error opening event, file_created");
    return;
  }

  void * shutdown_event = OpenEvent(EVENT_MODIFY_STATE, FALSE, "ok_to_shutdown");
  if (shutdown_event == NULL) {
    win_errprint("Error opening event, ok_to_shutdown");
    CloseHandle(file_event);
    return;
  }

  // Wait for parent server to build copyover file
  if (WaitForSingleObject(file_event, INFINITE) == WAIT_FAILED) {
    win_errprint("Error waiting on file_event");
    CloseHandle(file_event);
    CloseHandle(shutdown_event);
    return;
  }

  if ((fp = std::fopen ("hotboot.$$$", "r+b")) == NULL) {
    win_errprint("Error opening hotboot file");
    CloseHandle(file_event);
    CloseHandle(shutdown_event);
    return;
  }

  fread(&count_users, sizeof(int),1,fp); // how many users?
  // read in info about listening socket and build it
  fread(&proto_info,sizeof(WSAPROTOCOL_INFO),1,fp);
  g_listen = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
    &proto_info, 0, 0);
  if (g_listen == INVALID_SOCKET) {
    bug_printf ("Error opening listening socket : %d.", WSAGetLastError());
  }

  // read in info about users and build sockets for them
  for (int i = 0; i < count_users; i++) {
    char chname[25];
    fread(&proto_info,sizeof(WSAPROTOCOL_INFO),1,fp);
    fread(chname,sizeof(chname),1,fp);

    SOCKET sock = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
      &proto_info, 0, 0);
    if (sock == INVALID_SOCKET) {
      bug_printf ("Error opening user socket : %d.", WSAGetLastError());
      continue;
    }
    if (!write_to_descriptor (sock, "Returning from hotboot.\r\n", 0)) {
      bug_printf ("Error writing user socket");
      closesocket(sock);
      continue;
    }

    Descriptor * d = new Descriptor(sock);
    d->assign_hostname();
    descriptor_list.push_back(d);
    if (!d->load_char_obj (chname)) {
      write_to_descriptor (sock, "Your character vanished during hotboot. Bye.\r\n", 0);
      bug_printf ("Error loading character %s after hotboot", chname);
      d->close_socket();
      continue;
    }
    d->connected = CON_PLAYING;
    char_list.push_back(d->character);

    write_to_descriptor (sock, "Hotboot recovery complete.\r\n", 0);

    if (!d->character->in_room)
      d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

    d->character->char_to_room (d->character->in_room);
    d->character->do_look("");
    d->character->act ("$n materializes!", NULL, NULL, TO_ROOM);
  }
  fclose(fp);
//  if (remove("hotboot.$$$")) {
//    win_errprint("Error removing hotboot file");
//  }

  // tell parent its ok to go away
  if (SetEvent(shutdown_event) == NULL) {
    win_errprint("Error setting ok_to_shutdown event");
  }

  // cleanup event handles
  CloseHandle(file_event);
  CloseHandle(shutdown_event);
}
#endif

int main (int argc, char **argv)
{
  g_world = World::instance();
  g_db = Database::instance();

  str_boot_time = g_world->get_time_text();

  g_db->initialize("murk.db");

  // Get the port number.
  g_port = 1234;
  if (argc > 1) {
    if (!is_number (argv[1])) {
      fatal_printf ("Usage: %s [port #]");
    } else if ((g_port = std::atoi (argv[1])) <= 1024) {
      fatal_printf("Port number must be above 1024.");
    }
  }

  WIN32STARTUP

  // Run the game.
#if defined(WIN32) && !defined(__DMC__)
  if (argc < 3) g_listen = init_server_socket();
  g_db->boot();
  if (argc > 2) hotboot();
#else
  g_listen = init_server_socket();
  g_db->boot();
#endif

  g_world->area_update();

  log_printf ("Merc is ready to rock on port %d.", g_port);
  game_loop ();

  // Normal exit
  closesocket (g_listen);
  log_printf ("Normal termination of game.");
  WIN32CLEANUP
  g_db->shutdown();
  return 0;
}
