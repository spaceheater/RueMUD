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

#ifndef CONFIG_HPP
#define CONFIG_HPP

/*
 * Structure types.
 */
class Affect;
class Area;
class Ban;
class Character;
class Descriptor;
class Exit;
class ExtraDescription;
class MobPrototype;
class Note;
class Object;
class ObjectPrototype;
class PCData;
class Reset;
class Room;
class Shop;
class MobProgram;        /* MOBprogram */
class MobProgramActList;        /* MOBprogram */
class World;
class Database;

/*
 * Function types.
 */
typedef bool SPEC_FUN (Character * ch);

typedef short int sh_int;

/*
 * String and memory management parameters.
 */
#define MAX_STRING_LENGTH    4096
#define MAX_INPUT_LENGTH      160

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SKILL          90
#define MAX_LEVEL          40
#define LEVEL_HERO         (MAX_LEVEL - 4)
#define LEVEL_IMMORTAL         (MAX_LEVEL - 3)

#define PULSE_PER_SECOND        4
#define PULSE_VIOLENCE        ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE          ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK        (30 * PULSE_PER_SECOND)
#define PULSE_AREA        (60 * PULSE_PER_SECOND)

/* Container nesting */
#define MAX_NEST    100

/*
 * Connected state for a channel.
 */
enum {CON_PLAYING, CON_GET_NAME, CON_GET_OLD_PASSWORD, CON_CONFIRM_NEW_NAME,
      CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD, CON_GET_NEW_SEX,
      CON_GET_NEW_CLASS, CON_READ_MOTD};

/*
 * TO types for actflags.
 */
enum {TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR };

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/
/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD     3060
#define MOB_VNUM_VAMPIRE       3404

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC      1 << 0  /* Auto set for mobs    */
#define ACT_SENTINEL    1 << 1  /* Stays in one room    */
#define ACT_SCAVENGER   1 << 2  /* Picks up objects */
#define ACT_AGGRESSIVE  1 << 5  /* Attacks PC's     */
#define ACT_STAY_AREA   1 << 6  /* Won't leave area */
#define ACT_WIMPY       1 << 7 /* Flees when hurt  */
#define ACT_PET         1 << 8 /* Auto set for pets    */
#define ACT_TRAIN       1 << 9 /* Can train PC's   */
#define ACT_PRACTICE    1 << 10     /* Can practice PC's    */
#define ACT_EXTRACT     1 << 11    /* Mob dead...wating for extraction */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND          1 << 0
#define AFF_INVISIBLE      1 << 1
#define AFF_DETECT_EVIL    1 << 2
#define AFF_DETECT_INVIS   1 << 3
#define AFF_DETECT_MAGIC   1 << 4
#define AFF_DETECT_HIDDEN  1 << 5

#define AFF_SANCTUARY      1 << 7
#define AFF_FAERIE_FIRE    1 << 8
#define AFF_INFRARED       1 << 9
#define AFF_CURSE          1 << 10

#define AFF_POISON         1 << 12
#define AFF_PROTECT        1 << 13

#define AFF_SNEAK          1 << 15
#define AFF_HIDE           1 << 16
#define AFF_SLEEP          1 << 17
#define AFF_CHARM          1 << 18
#define AFF_FLYING         1 << 19
#define AFF_PASS_DOOR      1 << 20

/*
 * Sex.
 * Used in #MOBILES.
 */
enum {SEX_NEUTRAL, SEX_MALE, SEX_FEMALE};

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE        2
#define OBJ_VNUM_MONEY_SOME       3

#define OBJ_VNUM_CORPSE_NPC      10
#define OBJ_VNUM_CORPSE_PC       11
#define OBJ_VNUM_SEVERED_HEAD    12
#define OBJ_VNUM_TORN_HEART      13
#define OBJ_VNUM_SLICED_ARM      14
#define OBJ_VNUM_SLICED_LEG      15
#define OBJ_VNUM_FINAL_TURD      16

#define OBJ_VNUM_MUSHROOM        20
#define OBJ_VNUM_LIGHT_BALL      21
#define OBJ_VNUM_SPRING          22

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT            1
#define ITEM_SCROLL           2
#define ITEM_WAND             3
#define ITEM_STAFF            4
#define ITEM_WEAPON           5
#define ITEM_TREASURE         8
#define ITEM_ARMOR            9
#define ITEM_POTION          10
#define ITEM_FURNITURE       12
#define ITEM_TRASH           13
#define ITEM_CONTAINER       15
#define ITEM_DRINK_CON       17
#define ITEM_KEY             18
#define ITEM_FOOD            19
#define ITEM_MONEY           20
#define ITEM_BOAT            22
#define ITEM_CORPSE_NPC      23
#define ITEM_CORPSE_PC       24
#define ITEM_FOUNTAIN        25
#define ITEM_PILL            26

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW          1 << 0
#define ITEM_HUM           1 << 1
#define ITEM_DARK          1 << 2
#define ITEM_LOCK          1 << 3
#define ITEM_EVIL          1 << 4
#define ITEM_INVIS         1 << 5
#define ITEM_MAGIC         1 << 6
#define ITEM_NODROP        1 << 7
#define ITEM_BLESS         1 << 8
#define ITEM_ANTI_GOOD     1 << 9
#define ITEM_ANTI_EVIL     1 << 10
#define ITEM_ANTI_NEUTRAL  1 << 11
#define ITEM_NOREMOVE      1 << 12
#define ITEM_INVENTORY     1 << 13

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE          1 << 0
#define ITEM_WEAR_FINGER   1 << 1
#define ITEM_WEAR_NECK     1 << 2
#define ITEM_WEAR_BODY     1 << 3
#define ITEM_WEAR_HEAD     1 << 4
#define ITEM_WEAR_LEGS     1 << 5
#define ITEM_WEAR_FEET     1 << 6
#define ITEM_WEAR_HANDS    1 << 7
#define ITEM_WEAR_ARMS     1 << 8
#define ITEM_WEAR_SHIELD   1 << 9
#define ITEM_WEAR_ABOUT    1 << 10
#define ITEM_WEAR_WAIST    1 << 11
#define ITEM_WEAR_WRIST    1 << 12
#define ITEM_WIELD         1 << 13
#define ITEM_HOLD          1 << 14

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
enum {APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
      APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT,
      APPLY_WEIGHT, APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP,
      APPLY_AC, APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_PARA,
      APPLY_SAVING_ROD, APPLY_SAVING_PETRI, APPLY_SAVING_BREATH,
      APPLY_SAVING_SPELL};

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE   1 << 0
#define CONT_PICKPROOF   1 << 1
#define CONT_CLOSED      1 << 2
#define CONT_LOCKED      1 << 3

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO           2
#define ROOM_VNUM_CHAT         1200
#define ROOM_VNUM_TEMPLE       3001
#define ROOM_VNUM_ALTAR        3054
#define ROOM_VNUM_SCHOOL       3700

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK      1 << 0
#define ROOM_NO_MOB    1 << 2
#define ROOM_INDOORS   1 << 3
#define ROOM_PRIVATE   1 << 9
#define ROOM_SAFE      1 << 10
#define ROOM_SOLITARY  1 << 11
#define ROOM_PET_SHOP  1 << 12
#define ROOM_NO_RECALL 1 << 13

/*
 * Directions.
 * Used in #ROOMS.
 */
enum {DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN };

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR     1 << 0
#define EX_CLOSED     1 << 1
#define EX_LOCKED     1 << 2
#define EX_PICKPROOF  1 << 5

/*
 * Sector types.
 * Used in #ROOMS.
 */
enum {SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS,
      SECT_MOUNTAIN, SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNUSED,
      SECT_AIR, SECT_DESERT, SECT_MAX};

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
enum {WEAR_NONE=-1, WEAR_LIGHT=0, WEAR_FINGER_L, WEAR_FINGER_R,
      WEAR_NECK_1, WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS,
      WEAR_FEET, WEAR_HANDS, WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT,
      WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R, WEAR_WIELD,
      WEAR_HOLD, MAX_WEAR};

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Conditions.
 */
enum {COND_DRUNK, COND_FULL, COND_THIRST};

/*
 * Positions.
 */
enum {POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING,
      POS_RESTING, POS_FIGHTING, POS_STANDING};

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC      1 << 0 /* Don't EVER set.  */
#define PLR_BOUGHT_PET  1 << 1

#define PLR_AUTOEXIT    1 << 3
#define PLR_AUTOLOOT    1 << 4
#define PLR_AUTOSAC     1 << 5
#define PLR_BLANK       1 << 6
#define PLR_BRIEF       1 << 7
#define PLR_COMBINE     1 << 9
#define PLR_PROMPT      1 << 10
#define PLR_TELNET_GA   1 << 11
#define PLR_HOLYLIGHT   1 << 12

#define PLR_SILENCE     1 << 15
#define PLR_NO_EMOTE    1 << 16
#define PLR_NO_TELL     1 << 18
#define PLR_DENY        1 << 20
#define PLR_FREEZE      1 << 21
#define PLR_THIEF       1 << 22
#define PLR_KILLER      1 << 23
#define PLR_EXTRACT     1 << 24

/*
 * Channel bits.
 */
#define CHANNEL_AUCTION    1 << 0
#define CHANNEL_CHAT       1 << 1
#define CHANNEL_HACKER     1 << 2
#define CHANNEL_IMMTALK    1 << 3
#define CHANNEL_MUSIC      1 << 4
#define CHANNEL_QUESTION   1 << 5
#define CHANNEL_SHOUT      1 << 6
#define CHANNEL_YELL       1 << 7

#define ERROR_PROG        -1
#define IN_FILE_PROG       0

#define ACT_PROG        1 << 0
#define SPEECH_PROG     1 << 1
#define RAND_PROG       1 << 2
#define FIGHT_PROG      1 << 3
#define DEATH_PROG      1 << 4
#define HITPRCNT_PROG   1 << 5
#define ENTRY_PROG      1 << 6
#define GREET_PROG      1 << 7
#define ALL_GREET_PROG  1 << 8
#define GIVE_PROG       1 << 9
#define BRIBE_PROG      1 << 10

/*
 * Liquids.
 */
#define LIQ_WATER    0
#define LIQ_MAX     16

#define MAX_TRADE    5
/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000

/*
 *  Target types.
 */
enum {TAR_IGNORE, TAR_CHAR_OFFENSIVE,
  TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF, TAR_OBJ_INV};

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 */
#if defined WIN32
#define PLAYER_DIR  ".\\"    /* Player files                 */
#define MOB_DIR     ".\\"    /* MOBProg files                */
#else
#define PLAYER_DIR  "./"     /* Player files         */
#define MOB_DIR     "./"     /* MOBProg files                */
#endif

#define AREA_LIST   "area.lst"  /* List of areas        */

#define BUG_FILE    "bugs.txt"  /* For 'bug' and bug( )     */
#define IDEA_FILE   "ideas.txt" /* For 'idea'           */
#define TYPO_FILE   "typos.txt" /* For 'typo'           */
#define NOTE_FILE   "notes.txt" /* For 'notes'          */
#define SHUTDOWN_FILE   "shutdown.txt"  /* For 'shutdown'       */

/*
 * God Levels
 */
#define L_GOD       MAX_LEVEL
#define L_SUP       L_GOD - 1
#define L_DEI       L_SUP - 1
#define L_ANG       L_DEI - 1
#define L_HER       L_ANG - 1

enum {CLASS_MAGE, CLASS_CLERIC, CLASS_THIEF, CLASS_WARRIOR, CLASS_MAX};

/*
 * Utility macros.
 */
#define URANGE(a, b, c)     ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define IS_SET(flag, bit)   ((flag) & (bit))
#define SET_BIT(var, bit)   ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))

/* file read macro */
#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )                    \
                if ( !str_cmp( word, literal ) )    \
                {                   \
                    field  = value;         \
                    fMatch = true;          \
                    break;              \
                }


typedef std::list<Affect *>::iterator AffIter;
typedef std::list<Character *>::iterator CharIter;
typedef std::list<Descriptor *>::iterator DescIter;
typedef std::list<Object *>::iterator ObjIter;

struct class_type {
  char who_name[4];             /* Three-letter name for 'who'  */
  sh_int attr_prime;            /* Prime attribute      */
  sh_int weapon;                /* First weapon         */
  sh_int guild;                 /* Vnum of guild room       */
  sh_int skill_adept;           /* Maximum skill level      */
  sh_int thac0_00;              /* Thac0 for level  0       */
  sh_int thac0_32;              /* Thac0 for level 32       */
  sh_int hp_min;                /* Min hp gained on leveling    */
  sh_int hp_max;                /* Max hp gained on leveling    */
  bool fMana;                   /* Class gains mana on level    */
};

struct liq_type {
  const char * liq_name;
  const char * liq_color;
  sh_int liq_affect[3];
};

/*
 * A kill structure (indexed by level).
 */
struct kill_data {
  int number;
  int killed;
};

struct str_app_type {
  sh_int tohit;
  sh_int todam;
  sh_int carry;
  sh_int wield;
};

struct int_app_type {
  sh_int learn;
};

struct wis_app_type {
  sh_int practice;
};

struct dex_app_type {
  sh_int defensive;
};

struct con_app_type {
  sh_int hitp;
  sh_int shock;
};


#include "character.hpp"

/*
 * Command table.
 */
typedef void (Character::*cmdfun_T) (std::string);

struct cmd_type {
  const char * name;
  cmdfun_T do_fun;
  sh_int position;
  sh_int level;
};

typedef void (Character::*spellfun_T) (int sn, int lvl, void *vo);

struct skill_type {
  const char * name;                   /* Name of skill        */
  sh_int skill_level[CLASS_MAX];        /* Level needed by class    */
  spellfun_T spell_fun;         /* Spell pointer (for spells)   */
  sh_int target;                /* Legal targets        */
  sh_int minimum_position;      /* Position for caster / user   */
  int min_mana;              /* Minimum mana used        */
  int beats;                 /* Waiting time after use   */
  const char * noun_damage;            /* Damage message       */
  const char * msg_off;                /* Wear off message     */
};


#endif // CONFIG_HPP

