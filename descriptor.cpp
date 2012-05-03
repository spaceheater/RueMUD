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

#include "os.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "globals.hpp"
#include "io.hpp"

#include "descriptor.hpp"
#include "pcdata.hpp"
#include "object.hpp"
#include "room.hpp"
#include "area.hpp"
#include "note.hpp"
#include "objproto.hpp"
#include "mobproto.hpp"

/*
 * constants for Telnet.
 */
#define IAC 255     /* interpret as command: */
#define WONT    252     /* I won't use option */
#define WILL    251     /* I will use option */
#define GA  249     /* you may reverse the line */
#define TELOPT_ECHO 1   /* echo */

const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { IAC, GA, '\0' };


// Temporary externs
extern bool write_to_descriptor (SOCKET desc, const char *txt, int length);
extern std::string get_title (int klass, int level, int sex);
extern bool check_parse_name (const std::string & name);

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
static void bust_a_prompt (Character * ch)
{
  std::string buf;
  char buf2[MAX_STRING_LENGTH];
  std::string::iterator str;

  if (ch->prompt.empty()) {
    ch->send_to_char ("\r\n\r\n");
    return;
  }

  str = ch->prompt.begin();
  while (str != ch->prompt.end()) {
    if (*str != '%') {
      buf.append(1,*str);
      str++;
      continue;
    }
    ++str;
    switch (*str) {
    default:
      buf.append(" ");
      break;
    case 'h':
      snprintf (buf2, sizeof buf2, "%d", ch->hit);
      buf.append(buf2);
      break;
    case 'H':
      snprintf (buf2, sizeof buf2, "%d", ch->max_hit);
      buf.append(buf2);
      break;
    case 'm':
      snprintf (buf2, sizeof buf2, "%d", ch->mana);
      buf.append(buf2);
      break;
    case 'M':
      snprintf (buf2, sizeof buf2, "%d", ch->max_mana);
      buf.append(buf2);
      break;
    case 'v':
      snprintf (buf2, sizeof buf2, "%d", ch->move);
      buf.append(buf2);
      break;
    case 'V':
      snprintf (buf2, sizeof buf2, "%d", ch->max_move);
      buf.append(buf2);
      break;
    case 'x':
      snprintf (buf2, sizeof buf2, "%d", ch->exp);
      buf.append(buf2);
      break;
    case 'g':
      snprintf (buf2, sizeof buf2, "%d", ch->gold);
      buf.append(buf2);
      break;
    case 'a':
      if (ch->level < 5)
        snprintf (buf2, sizeof buf2, "%d", ch->alignment);
      else
        snprintf (buf2, sizeof buf2, "%s", ch->is_good () ? "good" : ch->is_evil () ?
          "evil" : "neutral");
      buf.append(buf2);
      break;
    case 'r':
      if (ch->in_room != NULL)
        buf.append(ch->in_room->name);
      else
        buf.append(" ");
      break;
    case 'R':
      if (ch->is_immortal() && ch->in_room != NULL)
        snprintf (buf2, sizeof buf2, "%d", ch->in_room->vnum);
      else
        snprintf (buf2, sizeof buf2, " ");
      buf.append(buf2);
      break;
    case 'z':
      if (ch->is_immortal() && ch->in_room != NULL)
        buf.append(ch->in_room->area->name);
      else
        buf.append(" ");
      break;
    case '%':
      buf.append("%%");
      break;
    }
    ++str;
  }
  ch->desc->write_to_buffer (buf);
  return;
}


Descriptor::Descriptor(SOCKET desc) :
  character(NULL), original(NULL),
  descriptor(desc), connected(CON_GET_NAME), fcommand(false),
  repeat(0), showstr_head(NULL), showstr_point(NULL) {
  memset(inbuf, 0, sizeof inbuf);
}



/*
 * Append onto an output buffer.
 */
void Descriptor::write_to_buffer (const std::string & txt) {
  /*
   * Initial \r\n if needed.
   */
  if (outbuf.empty() && !fcommand)
    outbuf = "\r\n";

  /*
   * Copy.
   */
  outbuf.append(txt);
  return;
}


/* The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom
   for porting this SillyMud code for MERC 2.0 and laying down the groundwork.
   Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which
   the improvements to the pager was modeled from.  - Kahn */
void Descriptor::show_string (const std::string & input) {
  char buffer[MAX_STRING_LENGTH];
  std::string buf;
  register char *scan, *chk;
  int lines = 0, toggle = 1;

  one_argument (input, buf);
  incomm.erase();

  if (!buf.empty()) {
    switch (toupper (buf[0])) {
    case 'C':                    /* show next page of text */
      lines = 0;
      break;

    case 'R':                    /* refresh current page of text */
      lines = -1 - (character->pcdata->pagelen);
      break;

    case 'B':                    /* scroll back a page of text */
      lines = -(2 * character->pcdata->pagelen);
      break;

    case 'H':                    /* Show some help */
      write_to_buffer ("C, or Return = continue, R = redraw this page,\r\n");
      write_to_buffer (
        "B = back one page, H = this help, Q or other keys = exit.\r\n\r\n");
      lines = -1 - (character->pcdata->pagelen);
      break;

    default:                     /*otherwise, stop the text viewing */
      if (showstr_head) {
        std::free(showstr_head);
        showstr_head = NULL;
      }
      showstr_point = 0;
      return;

    }
  }

  /* do any backing up necessary */
  if (lines < 0) {
    for (scan = showstr_point; scan > showstr_head; scan--)
      if ((*scan == '\n') || (*scan == '\r')) {
        toggle = -toggle;
        if (toggle < 0)
          if (!(++lines))
            break;
      }
    showstr_point = scan;
  }

  /* show a chunk */
  lines = 0;
  toggle = 1;
  for (scan = buffer;; scan++, showstr_point++) {
    *scan = *showstr_point;
    if ((*scan == '\n' || *scan == '\r') && (toggle = -toggle) < 0) {
      lines++;
    } else if (!*scan || (character && !character->is_npc ()
        && lines >= character->pcdata->pagelen)) {

      *scan = '\0';
      write_to_buffer (buffer);

      /* See if this is the end (or near the end) of the string */
      for (chk = showstr_point; isspace (*chk); chk++) ;
      if (!*chk) {
        if (showstr_head) {
          std::free(showstr_head);
          showstr_head = NULL;
        }
        showstr_point = 0;
      }
      return;
    }
  }
}

/*
 * Look for link-dead player to reconnect.
 */
bool Descriptor::check_reconnect (const std::string & name, bool fConn)
{
  Object *obj;

  CharIter c;
  for (c = char_list.begin(); c != char_list.end(); c++) {
    if (!(*c)->is_npc ()
      && (!fConn || (*c)->desc == NULL)
      && !str_cmp (character->name, (*c)->name)) {
      if (fConn == false) {
        character->pcdata->pwd = (*c)->pcdata->pwd;
      } else {
        delete character;
        character = *c;
        (*c)->desc = this;
        (*c)->timer = 0;
        (*c)->send_to_char ("Reconnecting.\r\n");
        (*c)->act ("$n has reconnected.", NULL, NULL, TO_ROOM);
        log_printf ("%s@%s reconnected.", (*c)->name.c_str(), host.c_str());
        connected = CON_PLAYING;

        /*
         * Contributed by Gene Choi
         */
        if ((obj = (*c)->get_eq_char (WEAR_LIGHT)) != NULL
          && obj->item_type == ITEM_LIGHT
          && obj->value[2] != 0 && (*c)->in_room != NULL)
          ++(*c)->in_room->light;
      }
      return true;
    }
  }

  return false;
}

/*
 * Check if already playing.
 */
bool Descriptor::check_playing (const std::string & name)
{
  DescIter dold;

  for (dold = descriptor_list.begin(); dold != descriptor_list.end(); dold++) {
    if (*dold != this
      && (*dold)->character != NULL
      && (*dold)->connected != CON_GET_NAME
      && (*dold)->connected != CON_GET_OLD_PASSWORD
      && !str_cmp (name, (*dold)->original
        ? (*dold)->original->name : (*dold)->character->name)) {
      write_to_buffer ("Already playing.\r\nName: ");
      connected = CON_GET_NAME;
      if (character != NULL) {
        delete character;
        character = NULL;
      }
      return true;
    }
  }

  return false;
}


/*
 * Transfer one line from input buffer to input line.
 */
void Descriptor::read_from_buffer ()
{
  int i, j, k;

  /*
   * Hold horses if pending command already.
   */
  if (!incomm.empty())
    return;

  /*
   * Look for at least one new line.
   */
  for (i = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++) {
    if (inbuf[i] == '\0')
      return;
  }

  /*
   * Canonical input processing.
   */
  for (i = 0, k = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++) {
    if (k >= MAX_INPUT_LENGTH - 2) {
      write_to_descriptor (descriptor, "Line too long.\r\n", 0);

      /* skip the rest of the line */
      for (; inbuf[i] != '\0'; i++) {
        if (inbuf[i] == '\n' || inbuf[i] == '\r')
          break;
      }
      inbuf[i] = '\n';
      inbuf[i + 1] = '\0';
      break;
    }

    if (inbuf[i] == '\b' && k > 0) {
      --k;
    } else if ( ((unsigned)inbuf[i] <= 0177)
      && isprint (inbuf[i])) {
      incomm.append(1, inbuf[i]);
      k++;
    }
  }

  /*
   * Finish off the line.
   */
  if (k == 0) {
    incomm.append(" ");
    k++;
  }

  /*
   * Deal with bozos with #repeat 1000 ...
   */
  if (k > 1 || incomm[0] == '!') {
    if (incomm[0] != '!' && incomm != inlast) {
      repeat = 0;
    } else {
      if (++repeat >= 20) {
        log_printf ("%s input spamming!", host.c_str());
        write_to_descriptor (descriptor,
          "\r\n*** PUT A LID ON IT!!! ***\r\n", 0);
        incomm = "quit";
      }
    }
  }

  /*
   * Do '!' substitution.
   */
  if (incomm[0] == '!')
    incomm = inlast;
  else
    inlast = incomm;

  /*
   * Shift the input buffer.
   */
  while (inbuf[i] == '\n' || inbuf[i] == '\r')
    i++;
  for (j = 0; (inbuf[j] = inbuf[i + j]) != '\0'; j++) ;
  return;
}

/*
 * Low level output function.
 */
bool Descriptor::process_output (bool fPrompt)
{
  /*
   * Bust a prompt.
   */
  if (fPrompt && !merc_down && connected == CON_PLAYING) {
    if (showstr_point)
      write_to_buffer (
        "[Please type (c)ontinue, (r)efresh, (b)ack, (h)elp, (q)uit, or RETURN]:  ");
    else {
      Character *ch;

      ch = original ? original : character;
      if (IS_SET (ch->actflags, PLR_BLANK))
        write_to_buffer ("\r\n");

      if (IS_SET (ch->actflags, PLR_PROMPT))
        bust_a_prompt (ch);

      if (IS_SET (ch->actflags, PLR_TELNET_GA))
        write_to_buffer (go_ahead_str);
    }
  }

  /*
   * Short-circuit if nothing to write.
   */
  if (outbuf.empty())
    return true;

  /*
   * OS-dependent output.
   */
  if (!write_to_descriptor (descriptor, outbuf.c_str(), outbuf.size())) {
    outbuf.erase();
    return false;
  } else {
    outbuf.erase();
    return true;
  }
}

void Descriptor::close_socket ()
{
  if (!outbuf.empty())
    process_output(false);

  if (character != NULL) {
    log_printf ("Closing link to %s.", character->name.c_str());
    if (connected == CON_PLAYING) {
      character->act ("$n has lost $s link.", NULL, NULL, TO_ROOM);
      character->desc = NULL;
    } else {
      delete character;
    }
  }

  deepdenext = descriptor_list.erase(
     std::find(descriptor_list.begin(),descriptor_list.end(),this));
  closesocket (descriptor);
  delete this;
  return;
}

bool Descriptor::read_from_descriptor ()
{
  unsigned int iStart;

  /* Hold horses if pending command already. */
  if (!incomm.empty())
    return true;

  /* Check for overflow. */
  iStart = strlen (inbuf);
  if (iStart >= sizeof (inbuf) - 10) {
    log_printf ("%s input overflow!", host.c_str());
    write_to_descriptor (descriptor,
      "\r\n*** PUT A LID ON IT!!! ***\r\n", 0);
    return false;
  }

  /* Snarf input. */
  for (;;) {
    int nRead;

    nRead = recv (descriptor, inbuf + iStart,
      sizeof (inbuf) - 10 - iStart, 0);
    if (nRead > 0) {
      iStart += nRead;
      if (inbuf[iStart - 1] == '\n' || inbuf[iStart - 1] == '\r')
        break;
    } else if (nRead == 0) {
      log_printf ("EOF encountered on read.");
      return false;
    } else if (GETERROR == EWOULDBLOCK)
      break;
    else {
      std::perror ("Read_from_descriptor");
      return false;
    }
  }

  inbuf[iStart] = '\0';
  return true;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void Descriptor::nanny (std::string argument)
{
  std::string buf;
  char cbuf[MAX_STRING_LENGTH];  // Needed for Windows crypt
  Character *ch;
  char *pwdnew;
  char *p;
  int iClass;
  int lines;
  int notes;
  bool fOld;

  incomm.erase();
  argument.erase(0, argument.find_first_not_of(" "));

  ch = character;

  switch (connected) {

  default:
    bug_printf ("Nanny: bad connected %d.", connected);
    close_socket();
    return;

  case CON_GET_NAME:
    if (argument.empty()) {
      close_socket();
      return;
    }

    argument[0] = toupper(argument[0]);
    if (!check_parse_name (argument)) {
      write_to_buffer ("Illegal name, try another.\r\nName: ");
      return;
    }

    fOld = load_char_obj (argument);
    ch = character;

    if (IS_SET (ch->actflags, PLR_DENY)) {
      log_printf ("Denying access to %s@%s.", argument.c_str(), host.c_str());
      write_to_buffer ("You are denied access.\r\n");
      close_socket();
      return;
    }

    if (check_reconnect (argument, false)) {
      fOld = true;
    } else {
      if (wizlock && !ch->is_hero() && !ch->wizbit) {
        write_to_buffer ("The game is wizlocked.\r\n");
        close_socket();
        return;
      }
    }

    if (fOld) {
      /* Old player */
      write_to_buffer ("Password: ");
      write_to_buffer (echo_off_str);
      connected = CON_GET_OLD_PASSWORD;
    } else {
      /* New player */
      /* New characters with same name fix by Salem's Lot */
      if (check_playing (ch->name))
        return;
      buf = "Did I get that right, " + argument + " (Y/N)? ";
      write_to_buffer (buf);
      connected = CON_CONFIRM_NEW_NAME;
    }
    break;

  case CON_GET_OLD_PASSWORD:
    write_to_buffer ("\r\n");

    strncpy(cbuf,argument.c_str(), sizeof cbuf);
    if (strcmp (crypt (cbuf, ch->pcdata->pwd.c_str()), ch->pcdata->pwd.c_str())) {
      write_to_buffer ("Wrong password.\r\n");
      close_socket();
      return;
    }

    write_to_buffer (echo_on_str);

    if (check_reconnect (ch->name, true))
      return;

    if (check_playing (ch->name))
      return;

    log_printf ("%s@%s has connected.", ch->name.c_str(), host.c_str());
    lines = ch->pcdata->pagelen;
    ch->pcdata->pagelen = 20;
    if (ch->is_hero())
      ch->do_help ("imotd");
    ch->do_help ("motd");
    ch->pcdata->pagelen = lines;
    connected = CON_READ_MOTD;
    break;

  case CON_CONFIRM_NEW_NAME:
    switch (argument[0]) {
    case 'y':
    case 'Y':
      buf = "New character.\r\nGive me a password for " + ch->name + ": " + echo_off_str;
      write_to_buffer (buf);
      connected = CON_GET_NEW_PASSWORD;
      break;

    case 'n':
    case 'N':
      write_to_buffer ("Ok, what IS it, then? ");
      delete character;
      character = NULL;
      connected = CON_GET_NAME;
      break;

    default:
      write_to_buffer ("Please type Yes or No? ");
      break;
    }
    break;

  case CON_GET_NEW_PASSWORD:
    write_to_buffer ("\r\n");

    if (argument.size() < 5) {
      write_to_buffer (
        "Password must be at least five characters long.\r\nPassword: ");
      return;
    }

    strncpy(cbuf,argument.c_str(), sizeof cbuf);
    pwdnew = crypt (cbuf, ch->name.c_str());
    for (p = pwdnew; *p != '\0'; p++) {
      if (*p == '~') {
        write_to_buffer (
          "New password not acceptable, try again.\r\nPassword: ");
        return;
      }
    }

    ch->pcdata->pwd = pwdnew;
    write_to_buffer ("Please retype password: ");
    connected = CON_CONFIRM_NEW_PASSWORD;
    break;

  case CON_CONFIRM_NEW_PASSWORD:
    write_to_buffer ("\r\n");

    strncpy(cbuf,argument.c_str(), sizeof cbuf);
    if (strcmp (crypt (cbuf, ch->pcdata->pwd.c_str()), ch->pcdata->pwd.c_str())) {
      write_to_buffer ("Passwords don't match.\r\nRetype password: ");
      connected = CON_GET_NEW_PASSWORD;
      return;
    }

    write_to_buffer (echo_on_str);
    write_to_buffer ("What is your sex (M/F/N)? ");
    connected = CON_GET_NEW_SEX;
    break;

  case CON_GET_NEW_SEX:
    switch (argument[0]) {
    case 'm':
    case 'M':
      ch->sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      ch->sex = SEX_FEMALE;
      break;
    case 'n':
    case 'N':
      ch->sex = SEX_NEUTRAL;
      break;
    default:
      write_to_buffer ("That's not a sex.\r\nWhat IS your sex? ");
      return;
    }

    buf = "Select a class [";
    for (iClass = 0; iClass < CLASS_MAX; iClass++) {
      if (iClass > 0)
        buf.append(" ");
      buf.append(class_table[iClass].who_name);
    }
    buf.append("]: ");
    write_to_buffer (buf);
    connected = CON_GET_NEW_CLASS;
    break;

  case CON_GET_NEW_CLASS:
    for (iClass = 0; iClass < CLASS_MAX; iClass++) {
      if (!str_cmp (argument, class_table[iClass].who_name)) {
        ch->klass = iClass;
        break;
      }
    }

    if (iClass == CLASS_MAX) {
      write_to_buffer ("That's not a class.\r\nWhat IS your class? ");
      return;
    }

    log_printf ("%s@%s new player.", ch->name.c_str(), host.c_str());
    write_to_buffer ("\r\n");
    ch->pcdata->pagelen = 20;
    ch->prompt = "<%hhp %mm %vmv> ";
    ch->do_help ("motd");
    connected = CON_READ_MOTD;
    break;

  case CON_READ_MOTD:
    char_list.push_back(ch);
    connected = CON_PLAYING;

    ch->send_to_char
      ("\r\nWelcome to Merc Diku Mud.  May your visit here be ... Mercenary.\r\n");

    if (ch->level == 0) {
      Object *obj;

      ch->pcdata->set_prime (class_table[ch->klass].attr_prime);

      ch->level = 1;
      ch->exp = 1000;
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
      buf = "the ";
      buf.append(get_title(ch->klass, ch->level, ch->sex));
      ch->set_title(buf);

      obj = get_obj_index(OBJ_VNUM_SCHOOL_BANNER)->create_object(0);
      obj->obj_to_char (ch);
      ch->equip_char (obj, WEAR_LIGHT);

      obj = get_obj_index(OBJ_VNUM_SCHOOL_VEST)->create_object(0);
      obj->obj_to_char (ch);
      ch->equip_char (obj, WEAR_BODY);

      obj = get_obj_index(OBJ_VNUM_SCHOOL_SHIELD)->create_object(0);
      obj->obj_to_char (ch);
      ch->equip_char (obj, WEAR_SHIELD);

      obj = get_obj_index(class_table[ch->klass].weapon)->create_object(0);
      obj->obj_to_char (ch);
      ch->equip_char (obj, WEAR_WIELD);

      ch->char_to_room(get_room_index (ROOM_VNUM_SCHOOL));
    } else if (ch->in_room != NULL) {
      ch->char_to_room(ch->in_room);
    } else if (ch->is_immortal()) {
      ch->char_to_room(get_room_index (ROOM_VNUM_CHAT));
    } else {
      ch->char_to_room(get_room_index (ROOM_VNUM_TEMPLE));
    }

    ch->act ("$n has entered the game.", NULL, NULL, TO_ROOM);
    ch->do_look ("auto");
    /* check for new notes */
    notes = 0;

    for (std::list<Note*>::iterator p = note_list.begin();
      p != note_list.end(); p++)
      if ((*p)->is_note_to (ch) && str_cmp (ch->name, (*p)->sender)
        && (*p)->date_stamp > ch->last_note)
        notes++;

    if (notes == 1)
      ch->send_to_char ("\r\nYou have one new note waiting.\r\n");
    else if (notes > 1) {
      buf = "\r\nYou have " + itoa(notes, 10) + " new notes waiting.\r\n";
      ch->send_to_char (buf);
    }

    break;
  }

  return;
}

void Descriptor::assign_hostname (void)
{
  struct sockaddr_in sock;
  char buf[MAX_STRING_LENGTH];
  struct hostent *from;
#ifndef WIN32
  socklen_t size = sizeof (sock);
#else
  int size = sizeof (sock);
#endif

  if (getpeername (descriptor, (struct sockaddr *) &sock, &size) < 0) {
    std::perror ("New_descriptor: getpeername");
    host = "(unknown)";
  } else {
    int addr = ntohl (sock.sin_addr.s_addr);
    snprintf (buf, sizeof buf, "%d.%d.%d.%d", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
      (addr >> 8) & 0xFF, (addr) & 0xFF);
    log_printf ("Sock.sinaddr:  %s", buf);
    from = gethostbyaddr ((char *) &sock.sin_addr, sizeof (sock.sin_addr), AF_INET);
    host = from ? from->h_name : buf;
  }

}

/*
 * Load a char and inventory into a new ch structure.
 */
bool Descriptor::load_char_obj (const std::string & name)
{
  Character* ch = new Character();
  ch->pcdata = new PCData();
  character = ch;
  ch->desc = this;
  ch->name = name;
  ch->prompt = "<%hhp %mm %vmv> ";
  ch->last_note = 0;
  ch->actflags = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
  ch->pcdata->condition[COND_THIRST] = 48;
  ch->pcdata->condition[COND_FULL] = 48;

  bool found = false;

  char strsave[MAX_INPUT_LENGTH];
  std::ifstream fp;

  snprintf (strsave, sizeof strsave, "%s%s", PLAYER_DIR, capitalize (name).c_str());
  fp.open (strsave, std::ifstream::in | std::ifstream::binary);
  if (fp.is_open()) {
    for (int iNest = 0; iNest < MAX_NEST; iNest++)
      rgObjNest[iNest] = NULL;

    found = true;
    for (;;) {
      char letter;
      std::string word;

      letter = fread_letter (fp);
      if (letter == '*') {
        fread_to_eol (fp);
        continue;
      }

      if (letter != '#') {
        bug_printf ("Load_char_obj: # not found.");
        break;
      }

      word = fread_word (fp);
      if (!str_cmp (word, "PLAYER"))
        ch->fread_char (fp);
      else if (!str_cmp (word, "OBJECT")) {
        Object* obj = new Object;
        if (!obj->fread_obj (ch, fp)) {
          delete obj;
          bug_printf ("fread_obj: bad object.");
        }
      } else if (!str_cmp (word, "END"))
        break;
      else {
        bug_printf ("Load_char_obj: bad section.");
        break;
      }
    }
    fp.close();
  }

  return found;
}

