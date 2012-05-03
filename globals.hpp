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

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

// Global variables
extern World *g_world;
extern Database *g_db;
extern Area *area_last;
extern std::list<Ban *> ban_list;
extern std::list<Character *> char_list;
extern std::list<Descriptor *> descriptor_list;       /* All open descriptors     */
extern std::list<Note *> note_list;
extern std::list<Object *> object_list;
extern std::list<Shop *> shop_list;

// These iterators used on loops where the next iterator can be invalidated
// because of a nested method that erases an object in the list.
extern CharIter deepchnext, deeprmnext;
extern ObjIter deepobnext;
extern DescIter deepdenext;
//bool character_invalidated = false;  // This is set in Mprogs if we

extern std::map<int, MobPrototype *> mob_table;
extern std::map<int, ObjectPrototype *> obj_table;
extern std::map<int, Room *> room_table;

extern struct cmd_type cmd_table[];
extern struct skill_type skill_table[MAX_SKILL];
extern struct class_type class_table[CLASS_MAX];
extern struct liq_type liq_table[LIQ_MAX];
extern std::string where_name[];
extern struct kill_data kill_table[];

extern struct int_app_type int_app[26];
extern struct str_app_type str_app[26];
extern struct dex_app_type dex_app[26];
extern struct con_app_type con_app[26];
extern struct wis_app_type wis_app[26];

extern bool merc_down;
extern bool wizlock;
extern std::string str_boot_time;
extern std::string help_greeting;
extern bool MOBtrigger;
extern std::ifstream * fpArea;
extern std::string strArea;
extern short g_port;
extern SOCKET g_listen;
extern bool extract_chars;

extern std::string target_name;
extern std::string dir_name[];
extern sh_int rev_dir[];
extern Object *rgObjNest[MAX_NEST];

// Global functions
extern ObjectPrototype *get_obj_index (int vnum);
extern Room *get_room_index (int vnum);
extern MobPrototype *get_mob_index (int vnum);
extern int skill_lookup (const std::string & name);
extern void load_mobprogs (std::ifstream & fp);
extern SPEC_FUN *spec_lookup (const std::string & name);
extern void mprog_read_programs (std::ifstream & fp, MobPrototype *pMobIndex);

#endif
