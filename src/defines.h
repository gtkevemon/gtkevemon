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

#ifndef DEFINES_HEADER
#define DEFINES_HEADER

/* The version string used in the application. */
#define GTKEVEMON_VERSION_STR "Version 1.10"

/* General compile-time configuration. */
#define LAUNCHER_CMD_AMOUNT 5

/* OS dependent defines. */
#ifndef WIN32
# define CONF_HOME_DIR ".gtkevemon"
# define OS_TEMP_DIR "/tmp"
#else
# define CONF_HOME_DIR "GtkEveMon"
# define OS_TEMP_DIR "c:\\windows\\temp"
#endif

#endif /* DEFINES_HEADER */
