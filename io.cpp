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
#include "database.hpp"
#include "world.hpp"

/*
print a series of warnings - do not exit
*/
void log_printf (const char * str, ...) {
  char buf[MAX_STRING_LENGTH];
  va_list args;

  va_start (args, str);
  vsnprintf (buf, sizeof buf, str, args);
  va_end (args);

  time_t tm = std::time(NULL);
  char * strtime = std::ctime (&tm);
  strtime[strlen (strtime) - 1] = '\0';
  std::cerr << strtime << " :: " << buf << std::endl;
  return;
}

/*
 * Reports a bug.
 */
void bug_printf (const char * str, ...)
{
  char buf[MAX_STRING_LENGTH];
  std::ofstream fp;
  va_list args;

  if (fpArea != NULL) {
    int iLine;
    std::ifstream::pos_type iChar;

    iChar = fpArea->tellg();
    fpArea->seekg(0);
    for (iLine = 0; fpArea->tellg() < iChar; iLine++) {
      while (fpArea->get() != '\n') ;
    }
    fpArea->seekg(iChar);

    snprintf (buf, sizeof buf, "[*****] FILE: %s LINE: %d", strArea.c_str(), iLine);
    log_printf(buf);

    fp.open ("shutdown.txt", std::ofstream::out | std::ofstream::app | std::ofstream::binary);
    if (fp.is_open()) {
      fp << buf << std::endl;
      fp.close();
    }
  }

  char buf2[MAX_STRING_LENGTH];
  snprintf (buf2, sizeof buf2, "[*****] BUG: %s", str);
  va_start (args, str);
  vsnprintf (buf, sizeof buf, buf2, args);
  va_end (args);

  log_printf(buf);

  fp.open (BUG_FILE, std::ofstream::out | std::ofstream::app | std::ofstream::binary);
  if (fp.is_open()) {
    fp << buf << std::endl;
    fp.close();
  }
  return;
}

/*
 * Reports a bug.
 */
void fatal_printf (const char * str, ...)
{
  char buf[MAX_STRING_LENGTH];
  va_list args;

  va_start (args, str);
  vsnprintf (buf, sizeof buf, str, args);
  va_end (args);
  bug_printf (buf);
  WIN32CLEANUP
//  g_db->shutdown();
  std::abort();
  return;
}

#ifdef WIN32
void win_errprint (const char * str)
{
  void* errmsg;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*) &errmsg, 0, NULL);
  bug_printf ("%s : %s.", str, (char*) errmsg);
  LocalFree(errmsg);
}
#endif

/*
 * Read and allocate space for a string from a file.
 */
std::string fread_string (std::ifstream & fp)
{
  std::string str;
  char c;

  /*
   * Skip blanks.
   * Read first char.
   */
  do {
    c = fp.get();
  }
  while (isspace (c));

  if (c == '~')
    return str;

  for (;;) {
    /*
     * Back off the char type lookup,
     *   it was too dirty for portability.
     *   -- Furey
     */
    switch (c) {
    default:
      try {
        str.append(1, c);
      } catch(...) {
        fatal_printf ("Fread_string: Maximum string size exceeded.");
      }
      break;
    case EOF:
      fatal_printf ("Fread_string: EOF");
      break;
    case '\n':
      try {
        str.append("\n\r");
      } catch(...) {
        fatal_printf ("Fread_string: Maximum string size exceeded.");
      }
      break;
    case '\r':
      break;
    case '~':
      return str;
    }
    c = fp.get();
  }
}

/*
 * Read a letter from a file.
 */
char fread_letter (std::ifstream & fp)
{
  char c;

  do {
    c = fp.get();
  }
  while (isspace (c));

  return c;
}

/*
 * Read a number from a file.
 */
int fread_number (std::ifstream & fp)
{
  int number;
  bool sign;
  char c;

  do {
    c = fp.get();
  }
  while (isspace (c));

  number = 0;

  sign = false;
  if (c == '+') {
    c = fp.get();
  } else if (c == '-') {
    sign = true;
    c = fp.get();
  }

  if (!isdigit (c)) {
    fatal_printf ("Fread_number: bad format.");
  }

  while (isdigit (c)) {
    number = number * 10 + c - '0';
    c = fp.get();
  }

  if (sign)
    number = 0 - number;

  if (c == '|')
    number += fread_number (fp);
  else if (c != ' ')
    fp.unget();

  return number;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol (std::ifstream & fp)
{
  char c;

  do {
    c = fp.get();
  }
  while (c != '\n' && c != '\r');

  do {
    c = fp.get();
  }
  while (c == '\n' || c == '\r');

  fp.unget();
  return;
}

/*
 * Read one word (into static buffer).
 */
std::string fread_word (std::ifstream & fp)
{
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  char cEnd;

  do {
    cEnd = fp.get();
  }
  while (isspace (cEnd));

  if (cEnd == '\'' || cEnd == '"') {
    pword = word;
  } else {
    word[0] = cEnd;
    pword = word + 1;
    cEnd = ' ';
  }

  for (; pword < word + MAX_INPUT_LENGTH; pword++) {
    *pword = fp.get();
    if (cEnd == ' ' ? isspace (*pword) : *pword == cEnd) {
      if (cEnd == ' ')
        fp.unget();
      *pword = '\0';
      return std::string(word);
    }
  }

  fatal_printf ("Fread_word: word too long.");
  return NULL;
}


