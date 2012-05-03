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

#ifndef WORLD_HPP
#define WORLD_HPP

class World {
public:
  static World* instance();
  time_t get_current_time(void);
  void set_current_time(time_t tsecs);
  char* get_time_text(void);
  std::string weather_update (void);
  int hour(void);
  std::string world_time(void);
  std::string world_weather(void);
  bool is_dark(void);
  bool is_raining(void);
  void change_weather(int chg);
  void area_update(void);
  std::string list_areas(void);
  void add_area(Area* area);

protected:
  World();
  ~World();
  World(const World&);
  World& operator= (const World&);

private:
  static World* _instance;

  time_t current_time;
  struct time_info_data {
    int hour;
    int day;
    int month;
    int year;
  } time_info;

  struct weather_data {
    int mmhg;
    int change;
    int sky;
    int sunlight;
  } weather_info;
  std::list<Area *> area_list;

};

#endif // WORLD_HPP
