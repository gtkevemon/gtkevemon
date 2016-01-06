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

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <pwd.h>

#include "defines.h"

void
usage (char** argv, bool slim = false)
{
  std::cerr << "Usage: " << argv[0] << " [ options ]" << std::endl
      << "Options:" << std::endl
      << "  -c DIR, --config-dir DIR  Use DIR as config directory" << std::endl
      << "  -h, --help                Display this helpful text" << std::endl
      << "  --char-sheet CID          Show sheet for character CID" << std::endl
      << "  --train-sheet CID         Show training sheet for CID" << std::endl
      << "  --skill-queue CID         Show skill queue for CID" << std::endl;

  if (!slim)
  {
    std::cerr << std::endl << "This application shows XML "
        << "sheets for a certain character. " << std::endl
        << "These sheets are retreived from "
        << "the GtkEveMon cache and are NOT " << std::endl
        << "requested from the API. If there "
        << "is no such sheet available an error " << std::endl
        << "is indicated. It's up to the user "
        << "to check if the sheets are outdated."
        << std::endl << std::endl;
  }
}

/* ---------------------------------------------------------------- */

std::string
get_default_config_dir (void)
{
  std::string user_conf_dir;

  uid_t user_id = ::geteuid();
  struct passwd* user_info = ::getpwuid(user_id);

  if (user_info == 0 || user_info->pw_dir == 0)
  {
    std::cerr << "Error: Couldn't determine home directory!" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  user_conf_dir = user_info->pw_dir;
  user_conf_dir += "/";
  user_conf_dir += CONF_HOME_DIR;

  return user_conf_dir;
}

/* ---------------------------------------------------------------- */

void
print_file (std::string const& filename, std::string const& conf_dir)
{
  /* Get path of config dir. */
  std::string fullpath;
  if (!conf_dir.empty())
    fullpath = conf_dir + "/sheets/" + filename;
  else
    fullpath = ::get_default_config_dir() + "/sheets/" + filename;

  /* Read file and dump to stdout. */
  std::ifstream in(fullpath.c_str());
  if (in.fail())
  {
    std::cerr << "Error accessing " << fullpath << ": "
        << ::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::cout << in.rdbuf() << std::endl;
  in.close();
}

/* ---------------------------------------------------------------- */

int
main (int argc, char** argv)
{
  if (argc <= 1)
  {
    usage(argv);
    std::exit(EXIT_FAILURE);
  }

  std::string config_dir;
  std::string char_sheet_cid;
  std::string train_sheet_cid;
  std::string skill_queue_cid;

  for (int i = 1; i < argc; ++i)
  {
    /* Arguments without parameters. */
    std::string argi(argv[i]);
    if (argi == "-h" || argi == "--help")
    {
      usage(argv);
      std::exit(EXIT_SUCCESS);
    }

    /* Arguments with exactly two parameters. */
    if (i + 1 >= argc)
    {
      usage(argv);
      std::exit(EXIT_FAILURE);
    }

    if (argi == "-c" || argi == "--config-dir")
    {
      config_dir = argv[i + 1];
      i += 1;
      continue;
    }
    else if (argi == "--char-sheet")
    {
      char_sheet_cid = argv[i + 1];
      i += 1;
      continue;
    }
    else if (argi == "--train-sheet")
    {
      train_sheet_cid = argv[i + 1];
      i += 1;
      continue;
    }
    else if (argi == "--skill-queue")
    {
      skill_queue_cid = argv[i + 1];
      i += 1;
      continue;
    }

    /* If code flow reaches here, argument is not recognized. */
    std::cout << "Argument \"" << argi << "\" not recognized!"
        << std::endl << std::endl;
    usage(argv, true);
    std::exit(EXIT_FAILURE);
  }

  if (!char_sheet_cid.empty())
    ::print_file(char_sheet_cid + "_CharacterSheet.xml", config_dir);

  if (!train_sheet_cid.empty())
    ::print_file(train_sheet_cid + "_SkillInTraining.xml", config_dir);

  if (!skill_queue_cid.empty())
    ::print_file(skill_queue_cid + "_SkillQueue.xml", config_dir);

  return 0;
}
