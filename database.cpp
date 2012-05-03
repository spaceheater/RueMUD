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
#include "database.hpp"

#include "shop.hpp"
#include "note.hpp"
#include "affect.hpp"
#include "exit.hpp"
#include "reset.hpp"
#include "area.hpp"
#include "room.hpp"
#include "objproto.hpp"
#include "object.hpp"
#include "mobproto.hpp"
#include "extra.hpp"
#include "world.hpp"

Database* Database::_instance = 0;

Database* Database::instance() {
  if (_instance == 0)
    _instance = new Database;
  return _instance;
}

Database::Database() {
  database = NULL;
  fBootDb = false;
}

Database::~Database() {
}

bool Database::initialize(std::string name)
{
  if (database != NULL)
    return true;
  if(sqlite3_open(name.c_str(), &database)) {
    fatal_printf("Can't open database: %s.", sqlite3_errmsg(database));
    return false;
  }
  return true;
}

void Database::shutdown(void) {
  if (database != NULL) {
    sqlite3_close(database);
    database = NULL;
  }
}

void Database::boot(void) {

  fBootDb = true;

  /*
   * Seed random number generator.
   */
  OS_SRAND (std::time (NULL));

  // Set welcome screen
  load_greeting();

  /*
   * Read in all the area files.
   */
  std::ifstream fpList;
  std::ifstream fp;

  fpList.open (AREA_LIST, std::ifstream::in | std::ifstream::binary);
  if (!fpList.is_open()) {
    fatal_printf (AREA_LIST);
  }

  for (;;) {
    strArea = fread_word (fpList);
    if (strArea[0] == '$')
      break;

    fp.open (strArea.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!fp.is_open()) {
      fatal_printf (strArea.c_str());
    }
    fpArea = &fp;
    for (;;) {
      std::string word;

      if (fread_letter (fp) != '#') {
        fatal_printf ("Boot_db: # not found.");
      }

      word = fread_word (fp);

      if (word[0] == '$')
        break;
      else if (!str_cmp (word, "AREA"))
        load_area (fp);
      else if (!str_cmp (word, "MOBILES"))
        load_mobiles (fp);
      else if (!str_cmp (word, "MOBPROGS"))
        load_mobprogs (fp);
      else if (!str_cmp (word, "OBJECTS"))
        load_objects (fp);
      else if (!str_cmp (word, "RESETS"))
        load_resets (fp);
      else if (!str_cmp (word, "ROOMS"))
        load_rooms (fp);
      else if (!str_cmp (word, "SHOPS"))
        load_shops (fp);
      else if (!str_cmp (word, "SPECIALS"))
        load_specials (fp);
      else {
        fatal_printf ("Boot_db: bad section name.");
      }
    }

    fp.close();
    fpArea = NULL;
  }
  fpList.close();

  /*
   * Fix up exits.
   * Declare db booting over.
   * Reset all areas once.
   * Load up the notes file.
   * Set the MOBtrigger.
   */
  fix_exits ();
  fBootDb = false;
  load_notes ();
  MOBtrigger = true;

  return;
}

void Database::load_area (std::ifstream & fp)
{
  Area *pArea = new Area();
  pArea->name = fread_string (fp);
  g_world->add_area(pArea);
  area_last = pArea;
  return;
}

void Database::load_greeting(void)
{
  sqlite3_stmt *stmt = NULL;
  Database* db = Database::instance();

  char *sql = sqlite3_mprintf("SELECT text FROM helps WHERE keyword = 'GREETING'");
  if (sqlite3_prepare(db->database, sql, -1, &stmt, 0) != SQLITE_OK) {
    bug_printf("Could not prepare statement: '%s' Error: %s", sql, sqlite3_errmsg(db->database));
    sqlite3_free(sql);
    return;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    help_greeting.assign((const char*)sqlite3_column_text( stmt, 0 ));
  }

  sqlite3_finalize(stmt);
  sqlite3_free(sql);
  return;
}

void Database::load_mobiles (std::ifstream & fp)
{
  MobPrototype *pMobIndex;

  for (;;) {
    sh_int vnum;
    char letter;

    letter = fread_letter (fp);
    if (letter != '#') {
      fatal_printf ("Load_mobiles: # not found.");
    }

    vnum = fread_number (fp);
    if (vnum == 0)
      break;

    fBootDb = false;
    if (get_mob_index (vnum) != NULL) {
      fatal_printf ("Load_mobiles: vnum %d duplicated.", vnum);
    }
    fBootDb = true;

    pMobIndex = new MobPrototype();
    pMobIndex->vnum = vnum;
    pMobIndex->name = fread_string (fp);
    pMobIndex->short_descr = fread_string (fp);
    pMobIndex->long_descr = fread_string (fp);
    pMobIndex->description = fread_string (fp);

    pMobIndex->long_descr[0] = toupper (pMobIndex->long_descr[0]);
    pMobIndex->description[0] = toupper (pMobIndex->description[0]);

    pMobIndex->actflags = fread_number (fp) | ACT_IS_NPC;
    pMobIndex->affected_by = fread_number (fp);
    pMobIndex->pShop = NULL;
    pMobIndex->alignment = fread_number (fp);
    letter = fread_letter (fp);
    pMobIndex->level = number_fuzzy (fread_number (fp));
    pMobIndex->sex = fread_number (fp);

    if (letter != 'S') {
      fatal_printf ("Load_mobiles: vnum %d non-S.", vnum);
    }

    letter = fread_letter (fp);
    if (letter == '>') {
      fp.unget();
      mprog_read_programs (fp, pMobIndex);
    } else
      fp.unget();
    mob_table.insert (std::map<int, MobPrototype*>::value_type (vnum, pMobIndex));
    kill_table[URANGE (0, pMobIndex->level, MAX_LEVEL - 1)].number++;
  }

  return;
}

void Database::load_objects (std::ifstream & fp)
{
  ObjectPrototype *pObjIndex;

  for (;;) {
    sh_int vnum;
    char letter;

    letter = fread_letter (fp);
    if (letter != '#') {
      fatal_printf ("Load_objects: # not found.");
    }

    vnum = fread_number (fp);
    if (vnum == 0)
      break;

    fBootDb = false;
    if (get_obj_index (vnum) != NULL) {
      fatal_printf ("Load_objects: vnum %d duplicated.", vnum);
    }
    fBootDb = true;

    pObjIndex = new ObjectPrototype();
    pObjIndex->vnum = vnum;
    pObjIndex->name = fread_string (fp);
    pObjIndex->short_descr = fread_string (fp);
    pObjIndex->description = fread_string (fp);
    /* Action description */ fread_string (fp);

    pObjIndex->short_descr[0] = tolower (pObjIndex->short_descr[0]);
    pObjIndex->description[0] = toupper (pObjIndex->description[0]);

    pObjIndex->item_type = fread_number (fp);
    pObjIndex->extra_flags = fread_number (fp);
    pObjIndex->wear_flags = fread_number (fp);
    pObjIndex->value[0] = fread_number (fp);

    // Translate spells into internal "skill numbers."
    switch (pObjIndex->item_type) {
    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SCROLL:
      pObjIndex->value[1] = skill_lookup (fread_word (fp));
      pObjIndex->value[2] = skill_lookup (fread_word (fp));
      pObjIndex->value[3] = skill_lookup (fread_word (fp));
      break;

    case ITEM_STAFF:
    case ITEM_WAND:
      pObjIndex->value[1] = fread_number (fp);
      pObjIndex->value[2] = fread_number (fp);
      pObjIndex->value[3] = skill_lookup (fread_word (fp));
      break;
    default:
      pObjIndex->value[1] = fread_number (fp);
      pObjIndex->value[2] = fread_number (fp);
      pObjIndex->value[3] = fread_number (fp);
    }


    pObjIndex->weight = fread_number (fp);
    pObjIndex->cost = fread_number (fp);        /* Unused */
    /* Cost per day */ fread_number (fp);

    if (pObjIndex->item_type == ITEM_POTION)
      SET_BIT (pObjIndex->extra_flags, ITEM_NODROP);

    for (;;) {
      char letter;

      letter = fread_letter (fp);

      if (letter == 'A') {
        Affect* paf = new Affect();
        paf->type = -1;
        paf->duration = -1;
        paf->location = fread_number (fp);
        paf->modifier = fread_number (fp);
        paf->bitvector = 0;
        pObjIndex->affected.push_back(paf);
      } else if (letter == 'E') {
        ExtraDescription* ed = new ExtraDescription();
        ed->keyword = fread_string (fp);
        ed->description = fread_string (fp);
        pObjIndex->extra_descr.push_back(ed);
      } else {
        fp.unget();
        break;
      }
    }

    obj_table.insert (std::map<int, ObjectPrototype*>::value_type (vnum, pObjIndex));
  }

  return;
}

void Database::load_resets (std::ifstream & fp)
{
  Reset *pReset;

  if (area_last == NULL) {
    fatal_printf ("Load_resets: no #AREA seen yet.");
  }

  for (;;) {
    Room *pRoomIndex;
    Exit *pexit;
    char letter;

    if ((letter = fread_letter (fp)) == 'S')
      break;

    if (letter == '*') {
      fread_to_eol (fp);
      continue;
    }

    pReset = new Reset();
    pReset->command = letter;
    /* if_flag */ fread_number (fp);
    pReset->arg1 = fread_number (fp);
    pReset->arg2 = fread_number (fp);
    pReset->arg3 = (letter == 'G' || letter == 'R')
      ? 0 : fread_number (fp);
    fread_to_eol (fp);

    /*
     * Validate parameters.
     * We're calling the index functions for the side effect.
     */
    switch (letter) {
    default:
      fatal_printf ("Load_resets: bad command '%c'.", letter);
      break;

    case 'M':
      get_mob_index (pReset->arg1);
      get_room_index (pReset->arg3);
      break;

    case 'O':
      get_obj_index (pReset->arg1);
      get_room_index (pReset->arg3);
      break;

    case 'P':
      get_obj_index (pReset->arg1);
      get_obj_index (pReset->arg3);
      break;

    case 'G':
    case 'E':
      get_obj_index (pReset->arg1);
      break;

    case 'D':
      pRoomIndex = get_room_index (pReset->arg1);

      if (pReset->arg2 < 0
        || pReset->arg2 > 5
        || (pexit = pRoomIndex->exit[pReset->arg2]) == NULL
        || !IS_SET (pexit->exit_info, EX_ISDOOR)) {
        fatal_printf ("Load_resets: 'D': exit %d not door.", pReset->arg2);
      }

      if (pReset->arg3 < 0 || pReset->arg3 > 2) {
        fatal_printf ("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
      }

      break;

    case 'R':
//      pRoomIndex = get_room_index (pReset->arg1);
      get_room_index (pReset->arg1);

      if (pReset->arg2 < 0 || pReset->arg2 > 6) {
        fatal_printf ("Load_resets: 'R': bad exit %d.", pReset->arg2);
      }

      break;
    }

    area_last->reset_list.push_back(pReset);
  }

  return;
}

void Database::load_rooms (std::ifstream & fp)
{
  Room *pRoomIndex;

  if (area_last == NULL) {
    fatal_printf ("Load_resets: no #AREA seen yet.");
  }

  for (;;) {
    sh_int vnum;
    char letter;
    int door;

    letter = fread_letter (fp);
    if (letter != '#') {
      fatal_printf ("Load_rooms: # not found.");
    }

    vnum = fread_number (fp);
    if (vnum == 0)
      break;

    fBootDb = false;
    if (get_room_index (vnum) != NULL) {
      fatal_printf ("Load_rooms: vnum %d duplicated.", vnum);
    }
    fBootDb = true;

    pRoomIndex = new Room();
    pRoomIndex->area = area_last;
    pRoomIndex->vnum = vnum;
    pRoomIndex->name = fread_string (fp);
    pRoomIndex->description = fread_string (fp);
    /* Area number */ fread_number (fp);
    pRoomIndex->room_flags = fread_number (fp);
    pRoomIndex->sector_type = fread_number (fp);
    pRoomIndex->light = 0;
    for (door = 0; door <= 5; door++)
      pRoomIndex->exit[door] = NULL;

    for (;;) {
      letter = fread_letter (fp);

      if (letter == 'S')
        break;

      if (letter == 'D') {
        Exit *pexit;
        int locks;

        door = fread_number (fp);
        if (door < 0 || door > 5) {
          fatal_printf ("Fread_rooms: vnum %d has bad door number.", vnum);
        }

        pexit = new Exit();
        pexit->description = fread_string (fp);
        pexit->name = fread_string (fp);
        pexit->exit_info = 0;
        locks = fread_number (fp);
        pexit->key = fread_number (fp);
        pexit->vnum = fread_number (fp);

        switch (locks) {
        case 1:
          pexit->exit_info = EX_ISDOOR;
          break;
        case 2:
          pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
          break;
        }

        pRoomIndex->exit[door] = pexit;
      } else if (letter == 'E') {
        ExtraDescription *ed;

        ed = new ExtraDescription();
        ed->keyword = fread_string (fp);
        ed->description = fread_string (fp);
        pRoomIndex->extra_descr.push_back(ed);
      } else {
        fatal_printf ("Load_rooms: vnum %d has flag not 'DES'.", vnum);
      }
    }

    room_table.insert (std::map<int, Room*>::value_type (vnum, pRoomIndex));
  }

  return;
}

void Database::load_shops (std::ifstream & fp)
{
  Shop *pShop;

  for (;;) {
    MobPrototype *pMobIndex;
    int iTrade;

    pShop = new Shop();
    pShop->keeper = fread_number (fp);
    if (pShop->keeper == 0)
      break;
    for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
      pShop->buy_type[iTrade] = fread_number (fp);
    pShop->profit_buy = fread_number (fp);
    pShop->profit_sell = fread_number (fp);
    pShop->open_hour = fread_number (fp);
    pShop->close_hour = fread_number (fp);
    fread_to_eol (fp);
    pMobIndex = get_mob_index (pShop->keeper);
    pMobIndex->pShop = pShop;

    shop_list.push_back(pShop);
  }

  return;
}

void Database::load_specials (std::ifstream & fp)
{
  for (;;) {
    MobPrototype *pMobIndex;
    char letter;

    switch (letter = fread_letter (fp)) {
    default:
      fatal_printf ("Load_specials: letter '%c' not *MS.", letter);

    case 'S':
      return;

    case '*':
      break;

    case 'M':
      pMobIndex = get_mob_index (fread_number (fp));
      pMobIndex->spec_fun = spec_lookup (fread_word (fp));
      if (pMobIndex->spec_fun == 0) {
        fatal_printf ("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
      }
      break;
    }

    fread_to_eol (fp);
  }
}

void Database::load_notes (void)
{
  std::ifstream fp;

  fp.open (NOTE_FILE, std::ifstream::in | std::ifstream::binary);
  if (!fp.is_open())
    return;

  for (;;) {
    Note *pnote;
    char letter;

    do {
      letter = fp.get();
      if (fp.eof()) {
        fp.close();
        return;
      }
    } while (isspace (letter));
    fp.unget();

    pnote = new Note();

    if (str_cmp (fread_word (fp), "sender"))
      break;
    pnote->sender = fread_string (fp);

    if (str_cmp (fread_word (fp), "date"))
      break;
    pnote->date = fread_string (fp);

    if (str_cmp (fread_word (fp), "stamp"))
      break;
    pnote->date_stamp = fread_number (fp);

    if (str_cmp (fread_word (fp), "to"))
      break;
    pnote->to_list = fread_string (fp);

    if (str_cmp (fread_word (fp), "subject"))
      break;
    pnote->subject = fread_string (fp);

    if (str_cmp (fread_word (fp), "text"))
      break;
    pnote->text = fread_string (fp);

    note_list.push_back(pnote);
  }

  strArea = NOTE_FILE;
  fpArea = &fp;
  fatal_printf ("Load_notes: bad key word.");
  return;
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void Database::fix_exits (void)
{
  Room *to_room;
  Exit *pexit;
  Exit *pexit_rev;
  int door;

  std::map<int,Room*>::iterator proom;
  for (proom = room_table.begin(); proom != room_table.end(); proom++) {
    bool fexit;

    fexit = false;
    for (door = 0; door <= 5; door++) {
      if ((pexit = (*proom).second->exit[door]) != NULL) {
        fexit = true;
        if (pexit->vnum <= 0)
          pexit->to_room = NULL;
        else
          pexit->to_room = get_room_index (pexit->vnum);
      }
    }

    if (!fexit)
      SET_BIT ((*proom).second->room_flags, ROOM_NO_MOB);
  }

  for (proom = room_table.begin(); proom != room_table.end(); proom++) {
    for (door = 0; door <= 5; door++) {
      if ((pexit = (*proom).second->exit[door]) != NULL
        && (to_room = pexit->to_room) != NULL
        && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
        && pexit_rev->to_room != (*proom).second) {
        bug_printf ("Fix_exits: %d:%d -> %d:%d -> %d.",
          (*proom).second->vnum, door,
          to_room->vnum, rev_dir[door], (pexit_rev->to_room == NULL)
          ? 0 : pexit_rev->to_room->vnum);
      }
    }
  }

  return;
}

