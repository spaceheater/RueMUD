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

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include "sqlite3/sqlite3.h"

#include "utils.hpp"

sqlite3 *database;
std::ifstream * fpArea;
std::string strArea("help.are");
time_t current_time = time(NULL);

int main(int argc, char** argv) {

  if(sqlite3_open("murk.db", &database)) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(database) << std::endl;
    sqlite3_close(database);
    return 1;
  }
  std::ifstream fp;
  fp.open (strArea.c_str(), std::ifstream::in | std::ifstream::binary);
  if (!fp.is_open()) {
    std::cerr << "Can't find " << strArea << std::endl;
  }
  for (;;) {
    if (fread_letter (fp) != '#') {
      std::cerr << "# not found." << std::endl;
    }
    std::string word = fread_word (fp);
    if (word[0] == '$')
      break;
    else if (word == "HELPS") {
      for (;;) {
        int level = fread_number (fp);
        std::string keyword = fread_string (fp);
        if (keyword[0] == '$')
          break;
        std::string text = fread_string (fp);
        char * z;
        if (text[0] == '.')
          z = sqlite3_mprintf("INSERT INTO 'helps' VALUES(%d,'%q','%q')",
            level, keyword.c_str(), text.substr(1).c_str());
        else
          z = sqlite3_mprintf("INSERT INTO 'helps' VALUES(%d,'%q','%q')",
            level, keyword.c_str(), text.c_str());
        sqlite3_exec(database, z, 0, 0, 0);
        sqlite3_free(z);
      }
    } else {
      std::cerr << "Load helps: bad section name." << std::endl;
    }
  }
  fp.close();
  sqlite3_close(database);
  return 0;
}

