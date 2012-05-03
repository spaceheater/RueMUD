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

#ifndef DATABASE_HPP
#define DATABASE_HPP

class Database {
public:
  sqlite3 *database;
  bool fBootDb;

  static Database* instance();
  bool initialize(std::string name);
  void shutdown(void);
  void boot(void);

protected:
  Database();
  ~Database();
  Database(const Database&);
  Database& operator= (const Database&);

private:
  static Database* _instance;

  void load_area (std::ifstream & fp);
  void load_greeting(void);
  void load_mobiles (std::ifstream & fp);
  void load_objects (std::ifstream & fp);
  void load_resets (std::ifstream & fp);
  void load_rooms (std::ifstream & fp);
  void load_shops (std::ifstream & fp);
  void load_specials (std::ifstream & fp);
  void load_notes (void);
  void fix_exits (void);

};

#endif // DATABASE_HPP
