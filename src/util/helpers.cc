#include <cstring>
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <zlib.h>

#include "exception.h"
#include "helpers.h"

std::string
Helpers::get_string_from_int (int value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_string_from_uint (unsigned int value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_string_from_sizet (std::size_t value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_string_from_float (float value, int digits)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(digits) << value;
  return ss.str();

}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_string_from_double (double value, int digits)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(digits) << value;
  return ss.str();

}

/* ---------------------------------------------------------------- */

int
Helpers::get_int_from_string (std::string const& value)
{
  std::stringstream ss(value);
  int ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

unsigned int
Helpers::get_uint_from_string (std::string const& value)
{
  std::stringstream ss(value);
  unsigned int ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

double
Helpers::get_double_from_string (std::string const& value)
{
  std::stringstream ss(value);
  double ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

float
Helpers::get_float_from_string (std::string const& value)
{
  std::stringstream ss(value);
  float ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_roman_from_int (int value)
{
  switch (value)
  {
    case 0:
      return "";
    case 1:
      return "I";
    case 2:
      return "II";
    case 3:
      return "III";
    case 4:
      return "IV";
    case 5:
      return "V";
    default:
      return "?";
  }
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_dotted_str_from_int (int value)
{
  std::stringstream ss;
  ss << value;
  return Helpers::get_dotted_str_from_str(ss.str());
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_dotted_str_from_uint (unsigned int value)
{
  std::stringstream ss;
  ss << value;
  return Helpers::get_dotted_str_from_str(ss.str());
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_dotted_str_from_str (std::string const& str)
{
  std::string ret;

  int cnt = 0;
  for (int i = (int)str.size() - 1; i >= 0; --i)
  {
    if (cnt % 3 == 0 && cnt > 0)
      ret.insert(ret.begin(), 1, ',');
    ret.insert(ret.begin(), 1, str[i]);
    cnt += 1;
  }
  return ret;
}

/* ---------------------------------------------------------------- */

std::string
Helpers::get_dotted_isk (std::string const& isk_string)
{
  size_t pos = isk_string.find_first_of('.');
  if (pos == std::string::npos)
    return isk_string;

  std::string tmp = isk_string.substr(0, pos);
  tmp = Helpers::get_dotted_str_from_str(tmp);
  tmp += isk_string.substr(pos);

  return tmp;
}

/* ---------------------------------------------------------------- */

std::string
Helpers::trunc_string (std::string const& str, int len)
{
  if ((int)str.size() > len + 3)
    return str.substr(0, len - 3).append("...");

  return str;
}

/* ---------------------------------------------------------------- */

StringVector
Helpers::split_string (std::string const& str, char delim)
{
  StringVector parts;

  unsigned int last = 0;
  unsigned int cur = 0;
  for (; cur < str.size(); ++cur)
    if (str[cur] == delim)
    {
      parts.push_back(str.substr(last, cur - last));
      last = cur + 1;
    }

  if (last < str.size())
    parts.push_back(str.substr(last));

  return parts;
}

/* ---------------------------------------------------------------- */

StringVector
Helpers::tokenize_cmd (std::string const& str)
{
  std::vector<std::string> result;

  /* Tokenize command. Delimiter is ' ', remove and handle '"' gracefully. */
  bool in_quote = false;
  std::string token;
  for (unsigned int i = 0; i < str.size(); ++i)
  {
    char chr = str[i];

    if (chr == ' ' && !in_quote)
    {
      result.push_back(token);
      token.clear();
    }
    else if (chr == '"')
      in_quote = !in_quote;
    else
      token += chr;
  }
  result.push_back(token);

  return result;
}

/* ---------------------------------------------------------------- */

char**
Helpers::create_argv (const std::vector<std::string>& cmd)
{
  char** args = new char*[cmd.size() + 1];
  for (unsigned int i = 0; i < cmd.size(); ++i)
  {
    char* cmd_cstr = new char[cmd[i].size() + 1];
    ::strcpy(cmd_cstr, cmd[i].c_str());
    args[i] = cmd_cstr;
  }
  args[cmd.size()] = 0;
  return args;
}

/* ---------------------------------------------------------------- */

void
Helpers::delete_argv (char** argv)
{
  for (std::size_t i = 0; argv[i] != 0; ++i)
    delete [] argv[i];
  delete [] argv;
}

/* ---------------------------------------------------------------- */

void
Helpers::read_file (std::string const& filename, std::string* data,
    bool auto_gunzip)
{
  if (auto_gunzip == false)
  {
    /* Read standard binary file. */
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in)
      throw FileException(filename, ::strerror(errno));

    in.seekg(0, std::ios::end);
    data->resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&data->at(0), data->size());
    in.close();
  }
  else
  {
    /* Read file and transparently uncompress if gzipped. */
    ::gzFile file = ::gzopen(filename.c_str(), "r");
    if (file == NULL)
     throw FileException(filename, ::strerror(errno));

    while (true)
    {
      unsigned char buffer[1024];
      int bytes_read = ::gzread(file, buffer, 1024);
      if (bytes_read > 0)
          data->append(buffer, buffer + bytes_read);
      else
        break;
    }
    ::gzclose(file);
  }
}

/* ---------------------------------------------------------------- */

void
Helpers::write_file (std::string const& filename, std::string const& data)
{
    std::ofstream out(filename.c_str(), std::ios::binary);
    if (!out)
        throw FileException(filename, ::strerror(errno));
    out.write(data.c_str(), data.size());
    out.close();
}
