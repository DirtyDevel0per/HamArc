#pragma once
#include <iostream>

struct Arguments {
  bool append = false;
  bool create = false;
  bool extract = false;
  bool list = false;
  bool del = false;
  bool concatenate = false;
  uint32_t block_size = 1;
  std::vector<std::string> files;
  std::string archive_name;
  std::string conc_archive1;
  std::string conc_archive2;
};

Arguments Parse(const size_t& argc, char** argv);