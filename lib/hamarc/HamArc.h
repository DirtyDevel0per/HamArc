#include <fstream>
#include <string>
#include <vector>

class HamArc {
  public:

  HamArc(const std::string& arc_name, std::vector<std::string>& files_arg, const uint32_t& block_size);

  explicit HamArc(const std::string& arc_name);

  void GetList() const;

  void Extract(const std::vector<std::string>& extract_files) const;

  void Delete(const std::vector<std::string>& delete_files) const;

  void Append(const std::vector<std::string>& append_files) const;

  void Conc(const std::string& archive_to_conc, const std::string& new_archive) const;

  static HamArc ReadArchive(const std::string& archive_name_arg);

  private:
  const uint8_t kFilesNumberBytes_ = 4;
  const uint8_t kBlockSizeBytes = 4;
  const uint8_t kFileNameLengthBytes_ = 4;
  const uint8_t kFileLengthBytes_ = 4;
  std::string arc_name_;
  std::vector<std::string> files_;
  uint32_t files_number_;
  uint32_t block_size_;

  static void WriteBits(std::vector<bool>& bits_to_write, std::ofstream& stream);
};

