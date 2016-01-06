#include <cstdlib>
#include <string>
#include <iostream>

#include "defines.h"
#include "argumentsettings.h"

int ArgumentSettings::argc = 0;
char** ArgumentSettings::argv = 0;
bool ArgumentSettings::start_minimized = false;
std::string ArgumentSettings::config_dir = "";

/* ---------------------------------------------------------------- */

void
ArgumentSettings::show_help (void)
{
  std::cout << "Usage: gtkevemon [ options ]" << std::endl
      << "Options:" << std::endl
      << "  -c DIR, --config-dir DIR  Use DIR as config directory" << std::endl
      << "  -h, --help                Display this helpful text" << std::endl
      << "  -m, --start-minimized     Start gtkevemon minimized" << std::endl;
}

/* ---------------------------------------------------------------- */

void
ArgumentSettings::init (int argc, char** argv)
{
  ArgumentSettings::argc = argc;
  ArgumentSettings::argv = argv;
  for (int optind = 1; optind < argc; ++optind)
  {
    if (argv[optind][0] != '-')
      continue;

    std::string sw = argv[optind];
    if (sw == "-m" || sw == "--start-minimized")
    {
      ArgumentSettings::start_minimized = true;
    }
    else if (sw == "-h" || sw == "--help")
    {
      ArgumentSettings::show_help();
      std::exit(EXIT_SUCCESS);
    }
    else if (sw == "-c" || sw == "--config-dir")
    {
      if (argc <= optind + 1 || argv[optind + 1][0] == '\0'
          || argv[optind + 1][0] == '-')
      {
        std::cout << sw << ": Expecting directory argument" << std::endl;
      }
      else
      {
        ArgumentSettings::config_dir = argv[optind + 1];
        optind += 1;
      }
    }
    else
    {
      std::cout << "Unrecognized option: " << sw << std::endl;
    }
  }
}
