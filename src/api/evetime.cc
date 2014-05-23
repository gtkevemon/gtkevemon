#include <sstream>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include "util/os.h"
#include "bits/config.h"
#include "evetime.h"

time_t EveTime::timediff = 0;
bool EveTime::initialized = false;

/* ---------------------------------------------------------------- */

void
EveTime::init_from_eveapi_string (std::string const& evetime)
{
  time_t eve = EveTime::get_time_for_string(evetime);
  time_t local = EveTime::get_local_time();

  EveTime::timediff = eve - local;
  EveTime::initialized = true;

  //std::cout << "EVE time has been set. Difference: "
  //    << EveTime::timediff << std::endl;
}

/* ---------------------------------------------------------------- */

void
EveTime::init_from_config (void)
{
  if (Config::conf.get_value("evetime.valid")->get_bool())
  {
    EveTime::timediff = (time_t)Config::conf.get_value
        ("evetime.difference")->get_int();
    EveTime::initialized = true;

    //std::cout << "EVE time has been set from config. Difference: "
    //    << EveTime::timediff << std::endl;
  }
}

/* ---------------------------------------------------------------- */

void
EveTime::store_to_config (void)
{
  Config::conf.get_value("evetime.valid")->set(EveTime::initialized);
  Config::conf.get_value("evetime.difference")->set((int)EveTime::timediff);
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_eve_time_string (void)
{
  time_t eve_time = EveTime::get_eve_time();
  return EveTime::get_gm_time_string(eve_time, false);
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_local_time_string (void)
{
  time_t local = EveTime::get_local_time();
  return EveTime::get_local_time_string(local, false);
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_time_string (struct tm *tm, bool slim)
{
  // If we wind up with an invalid time, we don't want to crash,
  // so we return a string indicating something has gone wrong.
  if (!tm)
  {
    return "INVALID TIME";
  }

  static ConfValuePtr lf = Config::conf.get_value("evetime.time_format");
  static ConfValuePtr sf = Config::conf.get_value("evetime.time_short_format");

  char buffer[128];
  strftime(buffer, 128, (slim ? **sf : **lf).c_str(), tm);
  return std::string(buffer);
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_local_time_string (time_t time, bool slim)
{
  struct tm tm;
  return EveTime::get_time_string(::localtime_r(&time, &tm), slim);
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_gm_time_string (time_t time, bool slim)
{
  struct tm tm;
  return EveTime::get_time_string(::gmtime_r(&time, &tm), slim);
}

/* ---------------------------------------------------------------- */

time_t
EveTime::adjust_local_time (time_t time)
{
  return time - EveTime::timediff;
}

/* ---------------------------------------------------------------- */

time_t
EveTime::adjust_eve_time (time_t time)
{
  return time + EveTime::timediff;
}

/* ---------------------------------------------------------------- */

time_t
EveTime::get_eve_time (void)
{
  return EveTime::get_local_time() + EveTime::timediff;
}

/* ---------------------------------------------------------------- */

time_t
EveTime::get_local_time (void)
{
  return ::time(0);
}

/* ---------------------------------------------------------------- */

time_t
EveTime::get_time_for_string (std::string const& timestr)
{
  if (timestr.empty())
    return -1;

  struct tm tm;
  ::memset(&tm, '\0', sizeof(struct tm));
  char* tmp = OS::strptime(timestr.c_str(), EVE_TIME_FORMAT, &tm);
  if (tmp == 0)
  {
    std::cout << "Warning: Unable to parse time string!" << std::endl;
    return (time_t)0;
  }
  time_t ret = OS::timegm(&tm);
  return ret;
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_string_for_timediff (time_t diff, bool slim)
{
  int slim_count = 2;
  bool ss_empty = true;

  time_t seconds = diff % 60;
  diff /= 60;
  time_t minutes = diff % 60;
  diff /= 60;
  time_t hours = diff % 24;
  diff /= 24;
  time_t days = diff;

  std::stringstream ss;
  if (days > 0)
  {
    ss << (int)days << "d";
    slim_count -= 1;
    ss_empty = false;
  }
  if (days > 0 || hours > 0)
  {
    if (!ss_empty) ss << " ";
    ss << (int)hours << "h";
    slim_count -= 1;
    ss_empty = false;
  }
  if ((!slim || slim_count > 0) && ((days > 0 && hours > 0) || minutes > 0))
  {
    if (!ss_empty) ss << " ";
    ss << (int)minutes << "m";
    slim_count -= 1;
    ss_empty = false;
  }

  if (!slim || slim_count > 0)
  {
    if (!ss_empty) ss << " ";
    ss << (int)seconds << "s";
  }

  return ss.str();
}

/* ---------------------------------------------------------------- */

std::string
EveTime::get_minute_str_for_diff (time_t diff)
{
  std::stringstream ss;
  ss << ((diff + 59) / 60) << "m";
  return ss.str();
}

/* ---------------------------------------------------------------- */

bool
EveTime::is_in_eve_downtime (time_t time)
{
  time_t hours = time % (3600 * 24);
  if (hours < EVE_DOWNTIME_START)
    return false;
  else
    return (hours - EVE_DOWNTIME_START) < EVE_DOWNTIME_DUR;
}

/* ---------------------------------------------------------------- */

bool
EveTime::is_initialized (void)
{
  return EveTime::initialized;
}
