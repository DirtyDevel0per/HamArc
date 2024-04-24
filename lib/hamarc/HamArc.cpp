#include "HamArc.h"
#include <lib/haming/haming.h>
#include <iostream>
#include <set>

HamArc::HamArc(const std::string& arc_name) :
  arc_name_(arc_name)
, files_number_(0)
, block_size_(4) {}

HamArc::HamArc(const std::string& arc_name, std::vector<std::string>& files_arg, const uint32_t& block_size) :
  arc_name_(arc_name)
  , files_(files_arg)
  , files_number_(files_.size())
  , block_size_(block_size) {

  std::ofstream arc_file(arc_name_, std::ios_base::binary);

  if (!arc_file.is_open()) {
    std::cout << "Wrong arc path!!!!" << std::endl;
    exit(EXIT_FAILURE);
  }

  arc_file.write(reinterpret_cast<const char*>(&files_number_), HamArc::kFilesNumberBytes_);
  arc_file.write(reinterpret_cast<const char*>(&block_size_), HamArc::kBlockSizeBytes);

  for (int i = 0; i < files_number_; ++i) {
    uint32_t n = files_[i].size();
    arc_file.write(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
    arc_file.write(files_[i].c_str(), n);
  }

  for (int i = 0; i < files_number_; ++i) {
    std::ifstream file(files_[i], std::ios_base::binary);

    file.seekg(0, std::ios::end);
    uint32_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    uint32_t control_bits = log(block_size_ * 8) / log(2) + 1 + block_size_ * 8 + 1;
    uint32_t bits_per_modulo_word = log(file_size % block_size_ * 8) / log(2) + 1
    + file_size % block_size_ * 8 + 1;

    if (file_size % block_size_) {
      file_size = file_size / block_size_ * ((control_bits + (8 - control_bits % 8)) / 8)
          + (bits_per_modulo_word + (8 - bits_per_modulo_word % 8)) / 8;
    } else {
      file_size = file_size / block_size_ * ((control_bits + (8 - control_bits % 8)) / 8);
    }
    arc_file.write(reinterpret_cast<char*>(&file_size), HamArc::kFileLengthBytes_);
    std::bitset<8> bits;
    char ch;
    std::string word;
    uint32_t j = 1;

    while (file.get(ch)) {
      bits = ch;
      word += bits.to_string();
      if (j % block_size_ == 0) {
        std::vector<bool> encoded = HamEncode(word);
        WriteBits(encoded, arc_file);
        word = "";
      }
      j++;
    }

    if (!word.empty() && block_size_ != 1) {
      std::vector<bool> encoded = HamEncode(word);
      WriteBits(encoded, arc_file);
    }
  }
}

void HamArc::WriteBits(std::vector<bool>& bits_to_write, std::ofstream& f) {
  int k = 0;
  while (k < bits_to_write.size()) {
    std::bitset<CHAR_BIT> byte_to_write;
    for (int l = k; l - k < CHAR_BIT && l < bits_to_write.size(); ++l) {
      byte_to_write.set(CHAR_BIT - (l - k) - 1, bits_to_write[l]);
    }
    unsigned long n = byte_to_write.to_ulong();
    f.write(reinterpret_cast<const char*>(&n), 1);
    k += CHAR_BIT;
  }
}

HamArc HamArc::ReadArchive(const std::string& archive_name_arg) {
  HamArc ReadArchive(archive_name_arg);
  ReadArchive.arc_name_ = archive_name_arg;
  std::ifstream file(ReadArchive.arc_name_, std::ios_base::binary);

  file.read(reinterpret_cast<char*>(&ReadArchive.files_number_), sizeof(ReadArchive.files_number_));
  file.read(reinterpret_cast<char*>(&ReadArchive.block_size_), sizeof(ReadArchive.block_size_));
  for (int i = 0; i < ReadArchive.files_number_; ++i) {
    uint32_t n;
    file.read(reinterpret_cast<char*>(&n), sizeof(n));
    std::string name(n, 0);

    for (auto it = 0; it < n; ++it)
      file.read(&name[it], 1);
    ReadArchive.files_.push_back(name);
  }

  for (int i = 0; i < ReadArchive.files_number_; ++i) {
    uint32_t file_size;
    file.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));
    file.seekg(file_size, std::ios_base::cur);
  }

  return ReadArchive;
}

void HamArc::GetList() const {
  for (int i = 0; i < files_number_; ++i) {
    std::cout << files_[i] << " ";
  }
  std::cout << '\n';
}

void HamArc::Extract(const std::vector<std::string>& extract_files) const {
  std::set<uint32_t> extract_positions;

  for (int i = 0; i < extract_files.size(); ++i) {
    for (int j = 0; j < files_number_; ++j) {
      if (files_[j] == extract_files[i]) {
        extract_positions.insert(j);
        break;
      }
    }
  }

  std::ifstream fin(arc_name_, std::ios_base::binary);
  std::ofstream fout(arc_name_ + "_tmp", std::ios_base::binary);

  uint32_t n;
  fin.read(reinterpret_cast<char*>(&n), HamArc::kFilesNumberBytes_);
  n -= extract_positions.size();
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kFilesNumberBytes_);
  fin.read(reinterpret_cast<char*>(&n), HamArc::kBlockSizeBytes);
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kBlockSizeBytes);

  for (int i = 0; i < files_number_; ++i) {
    if (extract_positions.contains(i)) {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read(&ch, 1);
        fout.write(&ch, 1);
      }
    }
  }

  for (int i = 0; i < files_number_; ++i) {
    if (extract_positions.contains(i)) {
      std::ofstream extracted_fout(files_[i], std::ios_base::binary);
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileLengthBytes_);
      std::string word;
      char ch;
      uint32_t bits_per_word = log(block_size_ * CHAR_BIT) / log(2) + 1 + block_size_ * CHAR_BIT + 1;
      uint32_t bytes_per_word = (bits_per_word + (CHAR_BIT - bits_per_word % CHAR_BIT)) / CHAR_BIT;
      for (int j = 0; j < n; ++j) {
        fin.read(&ch, 1);
        std::bitset<CHAR_BIT> bits = ch;
        word += bits.to_string();
        if ((j + 1) % bytes_per_word == 0 && j != 0) {
          std::vector<bool> decoded_word = HamDecode(word, block_size_);
          WriteBits(decoded_word, extracted_fout);
          word = "";
        }
      }
      if (!word.empty() && block_size_ != 1) {
        std::vector<bool> decoded_word = HamDecode(word, block_size_);
        WriteBits(decoded_word, extracted_fout);
      }
    } else {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read(&ch, 1);
        fout.write(&ch, 1);
      }
    }
  }

  fin.close();
  fout.close();
  std::remove(arc_name_.c_str());
  std::rename((arc_name_ + "_tmp").c_str(), arc_name_.c_str());
}

void HamArc::Delete(const std::vector<std::string>& delete_files) const {
  std::set<int> delete_positions;
  for (int i = 0; i < delete_files.size(); ++i) {
    for (int j = 0; j < files_number_; ++j) {
      if (files_[j] == delete_files[i]) {
        delete_positions.insert(j);
        break;
      }
    }
  }
  std::ifstream fin(arc_name_, std::ios_base::binary);
  std::ofstream fout(arc_name_ + "_tmp", std::ios_base::binary);

  uint32_t n;
  fin.read(reinterpret_cast<char*>(&n), HamArc::kFilesNumberBytes_);
  n -= delete_positions.size();
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kFilesNumberBytes_);
  fin.read(reinterpret_cast<char*>(&n), HamArc::kBlockSizeBytes);
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kBlockSizeBytes);

  for (int i = 0; i < files_number_; ++i) {
    if (delete_positions.contains(i)) {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read(&ch, 1);
        fout.write(&ch, 1);
      }
    }
  }

  for (int i = 0; i < files_number_; ++i) {
    if (delete_positions.contains(i)) {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fin.seekg(n, std::ios::cur);
    } else {
      fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
      fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
      for (int j = 0; j < n; ++j) {
        char ch;
        fin.read(&ch, 1);
        fout.write(&ch, 1);
      }
    }
  }

  fin.close();
  fout.close();

  std::remove(arc_name_.c_str());
  std::rename((arc_name_ + "_tmp").c_str(), arc_name_.c_str());
}

void HamArc::Append(const std::vector<std::string>& append_files) const {
  std::ifstream fin(arc_name_, std::ios_base::binary);
  std::ofstream fout(arc_name_ + "_tmp", std::ios_base::binary);

  uint32_t n;
  fin.read(reinterpret_cast<char*>(&n), HamArc::kFilesNumberBytes_);
  n += append_files.size();
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kFilesNumberBytes_);
  fin.read(reinterpret_cast<char*>(&n), HamArc::kBlockSizeBytes);
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kBlockSizeBytes);

  for (int i = 0; i < files_number_; ++i) {
    fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }

  for (int i = 0; i < append_files.size(); ++i) {
    n = append_files[i].size();
    fout.write(reinterpret_cast<const char*>(&n), sizeof(n));
    fout.write(append_files[i].c_str(), n);
  }
  for (int i = 0; i < files_number_; ++i) {
    fin.read(reinterpret_cast<char*>(&n), HamArc::kFileLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }

  for (int i = 0; i < append_files.size(); ++i) {
    std::ifstream append_fin(append_files[i], std::ios_base::binary);
    if (!append_fin.is_open()) {
      std::cout << "Error opening file!" << std::endl;
      exit(1);
    }
    append_fin.seekg(0, std::ios::end);
    uint32_t file_size = append_fin.tellg();
    append_fin.seekg(0, std::ios::beg);
    int bits_per_word = log(block_size_ * CHAR_BIT) / log(2) + 1 + block_size_ * CHAR_BIT;
    if (file_size%block_size_ != 0) {
      uint32_t bits_per_modulo_word = log((file_size%block_size_)*CHAR_BIT)/log(2) + 1
      + file_size%block_size_ * CHAR_BIT;
      file_size = file_size/block_size_ * ((bits_per_word + (CHAR_BIT - bits_per_word % CHAR_BIT)) / CHAR_BIT)
          + (bits_per_modulo_word + (CHAR_BIT - bits_per_modulo_word%CHAR_BIT)) / CHAR_BIT;
    } else {
      file_size = file_size/block_size_ * ((bits_per_word + (CHAR_BIT - bits_per_word % CHAR_BIT)) / CHAR_BIT);
    }
    fout.write(reinterpret_cast<const char*>(&file_size), HamArc::kFileLengthBytes_);
    std::bitset<CHAR_BIT> bits;
    char ch;
    std::string word;
    uint32_t j = 1;
    while (append_fin.get(ch)) {
      bits = ch;
      word += bits.to_string();
      if (j % block_size_ == 0) {
        std::vector<bool> encoded_word = HamEncode(word);
        WriteBits(encoded_word, fout);
        word = "";
      }
      j++;
    }
    if (!word.empty() && block_size_ != 1) {
      std::vector<bool> encoded_word = HamEncode(word);
      WriteBits(encoded_word, fout);
    }
  }
  fin.close();
  fout.close();
  std::remove(arc_name_.c_str());
  std::rename((arc_name_ + "_tmp").c_str(), arc_name_.c_str());
}

void HamArc::Conc(const std::string& archive_to_conc, const std::string& new_archive) const {
  std::ifstream main_fin(arc_name_, std::ios_base::binary);
  std::ifstream to_conc_fin(archive_to_conc, std::ios_base::binary);
  std::ofstream fout(new_archive, std::ios_base::binary);

  uint32_t n;
  uint32_t to_conc_files_number;
  main_fin.read(reinterpret_cast<char*>(&n), HamArc::kFilesNumberBytes_);
  to_conc_fin.read(reinterpret_cast<char*>(&to_conc_files_number), HamArc::kFilesNumberBytes_);
  n += to_conc_files_number;
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kFilesNumberBytes_);
  main_fin.read(reinterpret_cast<char*>(&n), HamArc::kBlockSizeBytes);
  to_conc_fin.seekg(HamArc::kBlockSizeBytes, std::ios::cur);
  fout.write(reinterpret_cast<const char*>(&n), HamArc::kBlockSizeBytes);

  for (int i = 0; i < files_number_; ++i) {
    main_fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      main_fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }
  for (int i = 0; i < to_conc_files_number; ++i) {
    to_conc_fin.read(reinterpret_cast<char*>(&n), HamArc::kFileNameLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileNameLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      to_conc_fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }
  for (int i = 0; i < files_number_; ++i) {
    main_fin.read(reinterpret_cast<char*>(&n), HamArc::kFileLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      main_fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }
  for (int i = 0; i < to_conc_files_number; ++i) {
    to_conc_fin.read(reinterpret_cast<char*>(&n), HamArc::kFileLengthBytes_);
    fout.write(reinterpret_cast<const char*>(&n), HamArc::kFileLengthBytes_);
    for (int j = 0; j < n; ++j) {
      char ch;
      to_conc_fin.read(&ch, 1);
      fout.write(&ch, 1);
    }
  }

  main_fin.close();
  to_conc_fin.close();
  fout.close();
}
