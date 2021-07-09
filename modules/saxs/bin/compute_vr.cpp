/**
   This is the program for validation of SAXS profiles.
 */
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/RatioVolatilityScore.h>

#include <IMP/Vector.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv) {
  bool use_offset = false;

  po::options_description desc(
      "Usage: compute_vr <experimental_profile> <profile_file1> <profile_file2> ...\n\n"
      "Any number of input profiles is supported.\n"
      "The chi value is computed relative to the first profile using\n"
      "its error column.\n\n"
      "Options");
  desc.add_options()
    ("help,h", "Show command line arguments and exit.")
    ("version", "Show version info and exit.")
    ("offset,o", "use offset in fitting (default = false)");

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
  if (vm.count("offset")) use_offset = true;

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
    }
  }

  IMP::saxs::Profile *exp_saxs_profile = exp_profiles[0];
  IMP::Pointer<IMP::saxs::ProfileFitter<IMP::saxs::RatioVolatilityScore> >
      saxs_score =
          new IMP::saxs::ProfileFitter<IMP::saxs::RatioVolatilityScore>(exp_saxs_profile);
  for (unsigned int i = 1; i < exp_profiles.size(); i++) {
    std::string fit_file =
        "fit" + std::string(boost::lexical_cast<std::string>(i)) + ".dat";
    double vr = saxs_score->compute_score(exp_profiles[i], use_offset, fit_file);
    std::cout << "File " << files[i] << " vr=" << vr << std::endl;
  }
  return 0;
}
