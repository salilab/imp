/**
   This is the program for validation of SAXS profiles.
 */

#include <IMP/Vector.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/utility.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#if defined(_WIN32) || defined(_WIN64)
// Simple basename implementation on platforms that don't have libgen.h
const char *basename(const char *path) {
  int i;
  for (i = path ? strlen(path) : 0; i > 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      return &path[i + 1];
    }
  }
  return path;
}
#else
#include <libgen.h>
#endif


int main(int argc, char **argv) {
  double max_q = 0.0;
  po::options_description desc("Usage: validate_profile <profile_file1> <profile_file2> ...\n\n"
          "Any number of input profiles is supported.\n"
          "Each profile is read and written back, with simulated error\n"
          "added if necessary\n\n"
          "Options");
  desc.add_options()
    ("help,h", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")
    ("max_q,q", po::value<double>(&max_q)->default_value(0.0),
      "maximal q value");

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files", po::value<std::vector<std::string> >(), "input files");

  po::options_description allopt;
  allopt.add(desc).add(hidden);

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(allopt).positional(p).run(),
      vm);
  po::notify(vm);
  if (vm.count("version")) {
    std::cerr << "Version: \"" << IMP::saxs::get_module_version()
              << "\"" << std::endl;
    return 0;
  }

  std::vector<std::string> files, dat_files;
  if (vm.count("input-files")) {
    files = vm["input-files"].as<std::vector<std::string> >();
  }
  if (vm.count("help") || files.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }

  IMP::saxs::Profiles exp_profiles;
  for (unsigned int i = 0; i < files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if (!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl;
      exit(1);
    }

    IMP_NEW(IMP::saxs::Profile, profile, (files[i]));
    if (profile->size() == 0) {
      std::cerr << "can't parse input file " << files[i] << std::endl;
      return 1;
    } else {
      dat_files.push_back(files[i]);
      exp_profiles.push_back(profile);
      std::cout << "Profile read from file " << files[i]
                << " size = " << profile->size() << std::endl;
      // write back
      std::string profile_name =
        IMP::saxs::trim_extension(basename(const_cast<char *>(files[i].c_str())));
      std::string file_name = profile_name + "_v.dat";
      profile->write_SAXS_file(file_name, max_q);
      std::cout << "Profile written to file " << file_name << std::endl;
    }
  }
  return 0;
}
