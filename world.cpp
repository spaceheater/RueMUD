/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 01/20/2007
 \version 1.5
 \remarks
  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#include "os.hpp"
#include "config.hpp"
#include "io.hpp"
#include "utils.hpp"
#include "globals.hpp"
#include "room.hpp"
#include "area.hpp"
#include "world.hpp"

enum {SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET };
enum {SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING};

std::string day_name[] = {
  "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
  "the Great Gods", "the Sun"
};

std::string month_name[] = {
  "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
  "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
  "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
  "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

World* World::_instance = 0;

World* World::instance() {
  if (_instance == 0)
    _instance = new World;
  return _instance;
}

World::World() {
  current_time = std::time(NULL);
  /*
   * Set time and weather.
   */
  long lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
  time_info.hour = lhour % 24;
  long lday = lhour / 24;
  time_info.day = lday % 35;
  long lmonth = lday / 35;
  time_info.month = lmonth % 17;
  time_info.year = lmonth / 17;

  if (time_info.hour < 5)
    weather_info.sunlight = SUN_DARK;
  else if (time_info.hour < 6)
    weather_info.sunlight = SUN_RISE;
  else if (time_info.hour < 19)
    weather_info.sunlight = SUN_LIGHT;
  else if (time_info.hour < 20)
    weather_info.sunlight = SUN_SET;
  else
    weather_info.sunlight = SUN_DARK;

  weather_info.change = 0;
  weather_info.mmhg = 960;
  if (time_info.month >= 7 && time_info.month <= 12)
    weather_info.mmhg += number_range (1, 50);
  else
    weather_info.mmhg += number_range (1, 80);

  if (weather_info.mmhg <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.mmhg <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.mmhg <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;

}

World::~World() {
}


time_t World::get_current_time(void) {
  return current_time;
}

void World::set_current_time(time_t tsecs) {
  current_time = tsecs;
}

char* World::get_time_text(void) {
  char * strtime = std::ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  return strtime;
}

/*
 * Update the weather.
 */
std::string World::weather_update (void)
{
  std::string buf;
  int diff;

  switch (++time_info.hour) {
  case 5:
    weather_info.sunlight = SUN_LIGHT;
    buf.append("The day has begun.\r\n");
    break;
  case 6:
    weather_info.sunlight = SUN_RISE;
    buf.append("The sun rises in the east.\r\n");
    break;
  case 19:
    weather_info.sunlight = SUN_SET;
    buf.append("The sun slowly disappears in the west.\r\n");
    break;
  case 20:
    weather_info.sunlight = SUN_DARK;
    buf.append("The night has begun.\r\n");
    break;
  case 24:
    time_info.hour = 0;
    time_info.day++;
    break;
  }

  if (time_info.day >= 35) {
    time_info.day = 0;
    time_info.month++;
  }

  if (time_info.month >= 17) {
    time_info.month = 0;
    time_info.year++;
  }

  /*
   * Weather change.
   */
  if (time_info.month >= 9 && time_info.month <= 16)
    diff = weather_info.mmhg > 985 ? -2 : 2;
  else
    diff = weather_info.mmhg > 1015 ? -2 : 2;

  weather_info.change += diff * dice (1, 4) + dice (2, 6) - dice (2, 6);
  weather_info.change = std::max (weather_info.change, -12);
  weather_info.change = std::min (weather_info.change, 12);

  weather_info.mmhg += weather_info.change;
  weather_info.mmhg = std::max (weather_info.mmhg, 960);
  weather_info.mmhg = std::min (weather_info.mmhg, 1040);

  switch (weather_info.sky) {
  default:
    bug_printf ("Weather_update: bad sky %d.", weather_info.sky);
    weather_info.sky = SKY_CLOUDLESS;
    break;

  case SKY_CLOUDLESS:
    if (weather_info.mmhg < 990
      || (weather_info.mmhg < 1010 && number_percent() <= 25)) {
      buf.append("The sky is getting cloudy.\r\n");
      weather_info.sky = SKY_CLOUDY;
    }
    break;

  case SKY_CLOUDY:
    if (weather_info.mmhg < 970
      || (weather_info.mmhg < 990 && number_percent() <= 25)) {
      buf.append("It starts to rain.\r\n");
      weather_info.sky = SKY_RAINING;
    }

    if (weather_info.mmhg > 1030 && number_percent() <= 25) {
      buf.append("The clouds disappear.\r\n");
      weather_info.sky = SKY_CLOUDLESS;
    }
    break;

  case SKY_RAINING:
    if (weather_info.mmhg < 970 && number_percent() <= 25) {
      buf.append("Lightning flashes in the sky.\r\n");
      weather_info.sky = SKY_LIGHTNING;
    }

    if (weather_info.mmhg > 1030
      || (weather_info.mmhg > 1010 && number_percent() <= 25)) {
      buf.append("The rain stopped.\r\n");
      weather_info.sky = SKY_CLOUDY;
    }
    break;

  case SKY_LIGHTNING:
    if (weather_info.mmhg > 1010
      || (weather_info.mmhg > 990 && number_percent() <= 25)) {
      buf.append("The lightning has stopped.\r\n");
      weather_info.sky = SKY_RAINING;
      break;
    }
    break;
  }

  return buf;
}

int World::hour(void) {
  return time_info.hour;
}

std::string World::world_time(void) {
  std::string buf;

  const char *suf;

  int day = time_info.day + 1;

  if (day > 4 && day < 20)
    suf = "th";
  else if (day % 10 == 1)
    suf = "st";
  else if (day % 10 == 2)
    suf = "nd";
  else if (day % 10 == 3)
    suf = "rd";
  else
    suf = "th";

  int h = (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12;

  buf.append("It is ");
  buf.append(itoa(h, 10));
  buf.append(" o'clock ");
  buf.append(time_info.hour >= 12 ? "pm" : "am");
  buf.append(", Day of ");
  buf.append(day_name[day % 7]);
  buf.append(", ");
  buf.append(itoa(day, 10));
  buf.append(suf);
  buf.append(" the Month of ");
  buf.append(month_name[time_info.month]);
  buf.append(".\r\n");

  return buf;
}

std::string World::world_weather(void) {
  static const char * sky_look[4] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"
  };

  std::string buf;

  buf.append("The sky is ");
  buf.append(sky_look[weather_info.sky]);
  buf.append(" and ");
  buf.append(weather_info.change >= 0 ? "a warm southerly breeze blows" :
    "a cold northern gust blows");
  buf.append(".\r\n");

  return buf;
}

bool World::is_dark(void) {
  return (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK);
}

bool World::is_raining(void) {
  return (weather_info.sky >= SKY_RAINING);
}

void World::change_weather(int chg) {
  weather_info.change += chg;
}

/*
 * Repopulate areas periodically.
 */
void World::area_update (void)
{
  Area *pArea;

  std::list<Area*>::iterator a;
  for (a = area_list.begin(); a != area_list.end(); a++) {
    pArea = *a;

    if (++pArea->age < 3)
      continue;

    /*
     * Check for PC's.
     */
    if (pArea->nplayer > 0 && pArea->age == 15 - 1) {
      CharIter c;
      for (c = char_list.begin(); c != char_list.end(); c++) {
        if (!(*c)->is_npc ()
          && (*c)->is_awake ()
          && (*c)->in_room != NULL && (*c)->in_room->area == pArea) {
          (*c)->send_to_char ("You hear the patter of little feet.\r\n");
        }
      }
    }

    /*
     * Check age and reset.
     * Note: Mud School resets every 3 minutes (not 15).
     */
    if (pArea->nplayer == 0 || pArea->age >= 15) {
      pArea->reset_area ();
      pArea->age = number_range (0, 3);
      Room* pRoomIndex = get_room_index (ROOM_VNUM_SCHOOL);
      if (pRoomIndex != NULL && pArea == pRoomIndex->area)
        pArea->age = 15 - 3;
    }
  }

  return;
}

std::string World::list_areas(void) {
  char buf[MAX_STRING_LENGTH];
  std::list<Area *>::iterator pArea1;
  std::list<Area *>::iterator pArea2;
  int iArea;
  int iAreaHalf;
  std::string listing;

  iAreaHalf = (Area::top_area + 1) / 2;
  pArea1 = pArea2 = area_list.begin();

  for (iArea = 0; iArea < iAreaHalf; iArea++, pArea2++) ;

  for (iArea = 0; iArea < iAreaHalf; iArea++, pArea1++, pArea2++) {
    snprintf (buf, sizeof buf, "%-39s%-39s\r\n",
      (*pArea1)->name.c_str(), (pArea2 != area_list.end()) ? (*pArea2)->name.c_str() : "");
    listing.append(buf);
  }

  return listing;
}

void World::add_area(Area* area) {
  area_list.push_back(area);
}
