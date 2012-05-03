/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.diku' as well the Merc      *
 *  license in 'license.merc'.  In particular, you may not remove either   *
 *  of these copyright notices.                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 08/30/2006
 \version 1.4
 \remarks
  This source code copyright (C) 2005, 2006 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "baseobject.hpp"

/*
 * One character (PC or NPC).
 */
class Character : public BaseObject {
public:
  Character *master;
  Character *leader;
  Character *fighting;
  Character *reply;
  SPEC_FUN *spec_fun;
  MobPrototype *pIndexData;
  Descriptor *desc;
  std::list<Affect *> affected;
  Note *pnote;
  std::list<Object *> carrying;
  Room *in_room;
  Room *was_in_room;
  PCData *pcdata;
  std::string long_descr;
  std::string prompt;
  sh_int sex;
  sh_int klass;
  sh_int race;
  int level;
  sh_int trust;
  bool wizbit;
  int played;
  time_t logon;
  time_t save_time;
  time_t last_note;
  sh_int timer;
  int wait;
  int hit;
  int max_hit;
  int mana;
  int max_mana;
  int move;
  int max_move;
  int gold;
  int exp;
  int actflags;
  int affected_by;
  sh_int position;
  sh_int practice;
  sh_int carry_weight;
  sh_int carry_number;
  sh_int saving_throw;
  sh_int alignment;
  sh_int hitroll;
  sh_int damroll;
  sh_int armor;
  sh_int wimpy;
  sh_int deaf;
  MobProgramActList *mpact;        /* Used by MOBprogram */
  int mpactnum;                 /* Used by MOBprogram */

  Character();
  ~Character();

#define CMD_DECL
#include "cmd_list.hpp"

  void do_mpstat(std::string argument);
  void do_mpasound(std::string argument);
  void do_mpkill(std::string argument);
  void do_mpjunk(std::string argument);
  void do_mpechoaround(std::string argument);
  void do_mpechoat(std::string argument);
  void do_mpecho(std::string argument);
  void do_mpmload(std::string argument);
  void do_mpoload(std::string argument);
  void do_mppurge(std::string argument);
  void do_mpgoto(std::string argument);
  void do_mpat(std::string argument);
  void do_mptransfer(std::string argument);
  void do_mpforce(std::string argument);

#define SPELL_DECL
#include "spell_list.hpp"

  bool is_npc();
  bool is_awake();
  bool is_good();
  bool is_evil();
  bool is_neutral();
  bool is_affected(int flg);
  int get_ac();
  int get_hitroll();
  int get_damroll();
  int get_curr_str();
  int get_curr_int();
  int get_curr_wis();
  int get_curr_dex();
  int get_curr_con();
  int get_age();
  int can_carry_n();
  int can_carry_w();
  int get_trust();
  bool is_immortal();
  bool is_hero();
  int is_outside();
  void wait_state(int npulse);
  int mana_cost(int sn);
  bool saves_spell (int lvl);
  std::string describe_to (Character* looker);
  Object * get_eq_char (int iWear);
  void affect_modify (Affect * paf, bool fAdd);
  bool can_see (Character * victim);
  bool can_see_obj (Object * obj);
  void unequip_char (Object * obj);
  void char_from_room ();
  void char_to_room (Room * pRoomIndex);
  void send_to_char (const std::string & txt);
  void interpret (std::string argument);
  bool check_social (const std::string & command, const std::string & argument);
  void set_title (const std::string & title);
  bool is_switched ();
  void advance_level ();
  bool mp_commands ();
  void gain_exp(int gain);
  int hit_gain ();
  int mana_gain ();
  int move_gain ();
  void add_follower (Character * master);
  void stop_follower();
  void die_follower();
  void update_pos ();
  void set_fighting (Character * victim);
  bool check_blind ();
  bool has_key (int key);
  void affect_to_char (Affect * paf);
  void affect_remove (Affect * paf);
  void affect_strip (int sn);
  bool has_affect (int sn);
  void affect_join (Affect * paf);
  bool remove_obj (int iWear, bool fReplace);
  void wear_obj (Object * obj, bool fReplace);
  void equip_char (Object * obj, int iWear);
  void act (const std::string & format, const void *arg1, const void *arg2, int type);
  bool can_drop_obj (Object * obj);
  Object * get_obj_wear (const std::string & argument);
  Object * get_obj_carry (const std::string & argument);
  Object * get_obj_here (const std::string & argument);
  void fwrite_char (std::ofstream & fp);
  void append_file (const char *file, const std::string & str);
  Character * get_char_room (const std::string & argument);
  Character * get_char_world (const std::string & argument);
  Object * get_obj_list (const std::string & argument, std::list<Object *> & list);
  Object * get_obj_world (const std::string & argument);
  void save_char_obj ();
  void fread_char (std::ifstream & fp);
  void gain_condition (int iCond, int value);
  void stop_fighting (bool fBoth);
  int find_door (const std::string & arg);
  void get_obj (Object * obj, Object * container);
  void extract_char (bool fPull);
  void extract_char_old (bool fPull);
  void stop_idling ();
  void show_list_to_char (std::list<Object *> & list, bool fShort, bool fShowNothing);
  void show_char_to_char_0 (Character * victim);
  void show_char_to_char_1 (Character * victim);
  void show_char_to_char (std::list<Character *> & list);
  void move_char (int door);
  bool is_gagged(std::string & nm);

};

#endif // CHARACTER_HPP
