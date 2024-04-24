#pragma once
#pragma pack(1)
#include <__filesystem/path.h>

struct HafHeader{
  char type[4] = "HAF";
  uint64_t amount_of_files{};
  uint64_t block_size = 1;
  HafHeader();
  HafHeader(char* bytes);
  uint64_t GetAmountOfFiles();
  uint64_t GetBlockSize();
  char* Serialize();
};

struct FileHeader {
  uint64_t header_size;
  const char* filename = "";
  uint64_t file_size;
  uint64_t begin;
  uint64_t end;
  FileHeader(uint64_t size, const std::filesystem::path& filename);
  char* Serialize() const;
};
#pragma pop