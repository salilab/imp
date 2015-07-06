/**
   This is the program for validation of SAXS profiles.
 */

#include <IMP/saxs/Profile.h>

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;


int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  float percentage = 0.0;
  std::string out_file_name  = "weighted.dat";

  po::options_description desc("Usage: <profile_file1> <w1> <profile_file2> <w2> ...");
  desc.add_options()
    ("help", "Any number of input profiles is supported. \
Each profile is read and written back, with simulated error added if necessary")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("percentage,p", po::value<float>(&percentage)->default_value(0.00),
     "percentage of noise (default = 0.00 (0%))")
     ("output_file,o",
      po::value<std::string>(&out_file_name)->default_value("weighted.dat"),
      "output file name, default name weighted.dat")
    ;
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> arguments, dat_files;
  IMP::Vector<double> weights;
  if(vm.count("input-files")) {
    arguments = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || arguments.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }

  IMP::Vector<IMP::saxs::Profile *> profiles;
  for(unsigned int i=0; i<arguments.size(); i+=2) {
    // check if file exists
    std::ifstream in_file(arguments[i].c_str());
    if(!in_file) {
      std::cerr << "Can't open file " << arguments[i] << std::endl;
      exit(1);
    }

    IMP::saxs::Profile *profile = new IMP::saxs::Profile(arguments[i]);
    if(profile->size() == 0) {
      std::cerr << "can't parse input file " << arguments[i] << std::endl;
      return 1;
    } else {
      dat_files.push_back(arguments[i]);
      profiles.push_back(profile);
      std::cout << "Profile read from file " << arguments[i] << " size = "
                << profile->size() << std::endl;
      weights.push_back(atof(arguments[i+1].c_str()));
    }
  }

  if(profiles.size() == 0) return 0;

  IMP::saxs::Profile *weighted_profile =
    new IMP::saxs::Profile(profiles[0]->get_min_q(),
                           profiles[0]->get_max_q(),
                           profiles[0]->get_delta_q());
  weighted_profile->add(profiles, weights);
  if(percentage > 0.0) weighted_profile->add_noise(percentage);
  weighted_profile->write_SAXS_file(out_file_name);
  return 0;
}
