/*
 * This file is part of GtkEveMon.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVE_TIME_HEADER
#define EVE_TIME_HEADER

#include <ctime>
#include <string>

/* Time format present in XML files. */
#define EVE_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
/* EVE downtime is 1h = 60m = 3600s. */
#define EVE_DOWNTIME_DUR 3600
/* EVE downtime starts at 11 o'clock. */
#define EVE_DOWNTIME_START (11 * 3600)

class EveTime
{
  private:
    /* Difference between the EVE time and the local time. */
    static time_t timediff;
    static bool initialized;

    static std::string get_time_string (struct tm *tm, bool slim);

  public:
    /* Calculate the time difference from a string. */
    static void init_from_eveapi_string (std::string const& evetime);
    /* Retrieve the time difference from config file. */
    static void init_from_config (void);
    /* Stores the time difference to config file. */
    static void store_to_config (void);

    /* Returns the current EVE time as string. */
    static std::string get_eve_time_string (void);
    /* Returns the current local time as string. */
    static std::string get_local_time_string (void);
    /* Returns a formatted time string from a given time,
     * either using local time or GMT. */
    static std::string get_local_time_string (time_t time, bool slim);
    static std::string get_gm_time_string (time_t time, bool slim);

    /* Adjusts the given local time to be in EVE time. */
    static time_t adjust_local_time (time_t time);
    /* Adjusts the given EVE time to be in local time. */
    static time_t adjust_eve_time (time_t time);

    /* Returns current EVE simple time. */
    static time_t get_eve_time (void);
    /* Returns the current local simple time. */
    static time_t get_local_time (void);

    /* Returns a local simple time for a given time string. */
    static time_t get_time_for_string (std::string const& timestr);

    /* Returns a string representing a time difference. */
    static std::string get_string_for_timediff (time_t diff, bool slim);
    static std::string get_minute_str_for_diff (time_t diff);

    /* Returns true if the given EVE time is in server downtime. */
    static bool is_in_eve_downtime (time_t time);

    /* Returns if the EVE time has been initialized. Otherwise
     * the EVE time is equal to the local time. */
    static bool is_initialized (void);
};

#endif /* EVE_TIME_HEADER */
