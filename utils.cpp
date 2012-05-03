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
 \date 08/30/2006
 \version 1.4
 \remarks
  This source code copyright (C) 2005, 2006 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#include "os.hpp"
#include "config.hpp"

/*
 * Generate a random number in an inclusive range.
 */
int number_range (int from, int to)
{
  if (to <= from)
    return from;
  return (from + (OS_RAND () % (1 + to - from)));
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy (int number)
{
  switch (number_range (0, 3)) {
  case 0:
    number -= 1;
    break;
  case 3:
    number += 1;
    break;
  }

  return std::max (1, number);
}

/*
 * Roll some dice.
 */
int dice (int number, int size)
{
  int idice;
  int sum;

  switch (size) {
  case 0:
    return 0;
  case 1:
    return number;
  }

  for (idice = 0, sum = 0; idice < number; idice++)
    sum += number_range (1, size);

  return sum;
}

/*
 * Generate a percentile roll.
 */
int number_percent (void)
{
  return number_range (1, 100);
}

/*
 * Generate a random door.
 */
int number_door (void)
{
  return number_range (0, 5);
}

/*
 * Simple linear interpolation.
 */
int interpolate (int level, int value_00, int value_32)
{
  return value_00 + level * (value_32 - value_00) / 32;
}

// Case insensitive compare
bool str_cmp(const std::string & s1, const std::string & s2)
{
  if (s1.size() != s2.size())
    return true;

  std::string::const_iterator p1 = s1.begin(), p2 = s2.begin();
  while(p1 != s1.end() && p2 != s2.end()) {
    if(tolower(*p1) != tolower(*p2))
      return true;
    p1++;
    p2++;
  }
  return false;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if s1 not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const std::string & s1, const std::string & s2)
{
  if (s1.size() > s2.size())
    return true;

  std::string::const_iterator p1 = s1.begin(), p2 = s2.begin();
  while(p1 != s1.end() && p2 != s2.end()) {
    if(tolower(*p1) != tolower(*p2))
      return true;
    p1++;
    p2++;
  }
  return false;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns true is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix (const std::string & astr, const std::string & bstr)
{
  if (astr.empty())
    return false;

  int cmpsz = bstr.size() - astr.size();
  char c0 = tolower(astr[0]);

  for (int ichar = 0; ichar <= cmpsz; ichar++) {
    if (c0 == tolower(bstr[ichar]) && !str_prefix(astr, bstr.substr(ichar)))
      return false;
  }

  return true;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return true if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const std::string& s1, const std::string& s2)
{
  if (s1.size() <= s2.size() && !str_cmp(s1, s2.substr(s2.size() - s1.size())))
    return false;
  else
    return true;
}

/*
 * Returns an initial-capped string.
 */
std::string capitalize (const std::string & str)
{
  std::string strcap;

  std::string::const_iterator p = str.begin();
  while(p != str.end()) {
    if (p == str.begin())
      strcap.append(1, (char)toupper(*p));
    else
      strcap.append(1, (char)tolower(*p));
    p++;
  }
  return strcap;
}

// replaces all occurances of a s1 in str with s2
void global_replace (std::string & str, const std::string & s1, const std::string & s2)
{
  std::string::size_type pos = 0;

  while ((pos = str.find(s1, pos)) != std::string::npos) {
    str.replace(pos, s1.size(), s2);
    pos += s2.size();
  }
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number (const std::string & arg)
{
  if (arg.empty())
    return false;

  std::string::const_iterator p = arg.begin();
  if (*p == '+' || *p == '-')
    p++;

  for (; p != arg.end(); p++) {
    if (!isdigit (*p))
      return false;
  }

  return true;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument (const std::string & argument, std::string & arg)
{
  std::string::size_type pos;

  arg.erase();
  if ((pos = argument.find(".")) != std::string::npos) {
    arg = argument.substr(pos+1);
    return std::atoi (argument.substr(0, pos).c_str());
  }
  arg = argument;
  return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
std::string one_argument (const std::string & argument, std::string & arg_first)
{
  char cEnd;
  std::string::const_iterator argp = argument.begin();

  arg_first.erase();
  while (argp != argument.end() && isspace (*argp))
    argp++;

  cEnd = ' ';
  if (*argp == '\'' || *argp == '"')
    cEnd = *argp++;

  while (argp != argument.end()) {
    if (*argp == cEnd) {
      argp++;
      break;
    }
    arg_first.append(1, (char)tolower(*argp));
    argp++;
  }

  while (argp != argument.end() && isspace (*argp))
    argp++;

  return std::string(argp, argument.end());
}

/*
 * See if a string is one of the names of an object.
 */
/*
 * New is_name sent in by Alander.
 */
bool is_name (const std::string & str, std::string namelist)
{
  std::string name;

  for (;;) {
    namelist = one_argument (namelist, name);
    if (name.empty())
      return false;
    if (!str_cmp (str, name))
      return true;
  }
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde (std::string & str)
{
  global_replace(str, "~", "-");
}


