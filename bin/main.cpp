#include <lib/hamarc/HamArc.h>
#include <lib/parser/parser.h>


int main(int argc, char** argv) {
  Arguments options = Parse(argc, argv);

  if (options.create) {
    HamArc arc(options.archive_name, options.files, options.block_size);
  } else if(options.append) {
    HamArc arc = HamArc::ReadArchive(options.archive_name);
    arc.Append(options.files);
  } else if(options.del) {
    HamArc arc = HamArc::ReadArchive(options.archive_name);
    arc.Delete(options.files);
  } else if(options.concatenate) {
    HamArc arc = HamArc::ReadArchive(options.conc_archive1);
    arc.Conc(options.conc_archive2, options.archive_name);
  } else if(options.list) {
    HamArc arc = HamArc::ReadArchive(options.archive_name);
    arc.GetList();
  } else if(options.extract) {
    HamArc arc = HamArc::ReadArchive(options.archive_name);
    arc.Extract(options.files);
  }

  return 0;
}
