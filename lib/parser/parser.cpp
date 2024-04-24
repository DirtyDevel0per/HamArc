#include "parser.h"

Arguments Parse(const size_t& argc, char** argv) {
  Arguments options;
  uint32_t i = 1;
  while (i < argc) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        std::string command_name;
        int it = 2;
        while (argv[i][it] != '=') {
          command_name += argv[i][it];
          ++it;
        }
        if (!strcmp(command_name.c_str(), "create"))
          options.create = true;
        else if (!strcmp(command_name.c_str(), "block_size"))
          options.block_size = std::stoi(argv[i] + it);
        else if (!strcmp(command_name.c_str(), "delete"))
          options.del = true;
        else if (!strcmp(command_name.c_str(), "append"))
          options.append = true;
        else if (!strcmp(command_name.c_str(), "extract"))
          options.extract = true;
        else if (!strcmp(command_name.c_str(), "list"))
          options.list = true;
        else if (!strcmp(command_name.c_str(), "concatenate")) {
          options.concatenate = true;
          options.conc_archive1 = argv[++i];
          options.conc_archive2 = argv[++i];
        }
      } else if (!strcmp(argv[i], "-c"))
        options.create = true;
      else if (!strcmp(argv[i], "-s"))
        options.block_size = std::stoi(argv[++i]);
      else if (!strcmp(argv[i], "-l"))
        options.list = true;
      else if (!strcmp(argv[i], "-x"))
        options.extract = true;
      else if (!strcmp(argv[i], "-a"))
        options.append = true;
      else if (!strcmp(argv[i], "-d"))
        options.del = true;
      else if (!strcmp(argv[i], "-A")) {
        options.concatenate = true;
        options.conc_archive1 = argv[++i];
        options.conc_archive2 = argv[++i];
      } else if (!strcmp(argv[i], "-f"))
        options.archive_name = argv[++i];
    } else if (std::string(argv[i]).substr(0, 6) == "--file")
        options.archive_name = std::string(argv[i]).substr(7);
    else
      options.files.emplace_back(argv[i]);
    i++;
  }

  return options;
}