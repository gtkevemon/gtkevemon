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

#ifndef HELPERS_HEADER
#define HELPERS_HEADER

#include <string>
#include <vector>

typedef std::vector<std::string> StringVector;

class Helpers
{
  public:
    static std::string get_string_from_int (int value);
    static std::string get_string_from_uint (unsigned int value);
    static std::string get_string_from_sizet (std::size_t value);
    static std::string get_string_from_float (float value, int digits);
    static std::string get_string_from_double (double value, int digits);

    static int get_int_from_string (std::string const& value);
    static unsigned int get_uint_from_string (std::string const& value);
    static double get_double_from_string (std::string const& value);
    static float get_float_from_string (std::string const& value);
    static std::string get_roman_from_int (int value);

    static std::string get_dotted_str_from_int (int value);
    static std::string get_dotted_str_from_uint (unsigned int value);
    static std::string get_dotted_str_from_str (std::string const& str);
    static std::string get_dotted_isk (std::string const& isk_string);

    static std::string trunc_string (std::string const& str, int len);

    static StringVector split_string (std::string const& str, char delim);
    static StringVector tokenize_cmd (std::string const& str);
    static char** create_argv (std::vector<std::string> const& cmd);
    static void delete_argv (char** argv);
    static void read_file (std::string const& filename, std::string* data,
        bool auto_gunzip = false);
    static void write_file (std::string const& filename,
        std::string const& data);
};

#endif /* HELPERS_HEADER */
