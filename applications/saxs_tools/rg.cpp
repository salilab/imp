/**
   This is the program for computation of radius of gyration from SAXS profiles.
 */
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/utility.h>

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

  float end_q_rg = 1.3;
  po::options_description desc("Usage: <pdb_file1> <pdb_file2> \
... <profile_file1> <profile_file2> ...");
  desc.add_options()
    ("help", "Any number of input PDBs and profiles is supported. \
Each PDB will be fitted against each profile.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("end_q_rg,q*rg", po::value<float>(&end_q_rg)->default_value(1.3),
     "end q*rg value for rg computation, q*rg<end_q_rg (default = 1.3), \
use 0.8 for elongated proteins")
    ;
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files, pdb_files, dat_files;
  if (vm.count("input-files"))   {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }

  // 1. read pdbs and profiles, prepare particles
  IMP::Model *model = new IMP::Model();
  std::vector<IMP::Particles> particles_vec;
  std::vector<IMP::saxs::Profile *> exp_profiles;
  for(unsigned int i=0; i<files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if(!in_file) {
      std::cerr << "Can't open file " << files[i] << std::endl;
      exit(1);
    }
    // A. try as pdb
    try {
      IMP::atom::Hierarchy mhd =
        IMP::atom::read_pdb(files[i], model,
                            new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                            // don't add radii
                            true, true);
      IMP::Particles particles
        = IMP::get_as<IMP::Particles>(get_by_type(mhd, IMP::atom::ATOM_TYPE));
      if(particles.size() > 0) { // pdb file
        pdb_files.push_back(files[i]);
        particles_vec.push_back(particles);
        std::cout << particles.size() << " atoms were read from PDB file "
                  << files[i] << std::endl;
      }
    } catch(IMP::base::ValueException e) { // not a pdb file
      // B. try as dat file
      IMP::saxs::Profile *profile = new IMP::saxs::Profile(files[i]);
      if(profile->size() == 0) {
        std::cerr << "can't parse input file " << files[i] << std::endl;
        return 1;
      } else {
        dat_files.push_back(files[i]);
        exp_profiles.push_back(profile);
        std::cout << "Profile read from file " << files[i] << " size = "
                  << profile->size() << std::endl;
      }
    }
  }

  // 2. compute rg for input profiles
  for(unsigned int i=0; i<dat_files.size(); i++) {
    IMP::saxs::Profile* exp_saxs_profile = exp_profiles[i];
    double rg = exp_saxs_profile->radius_of_gyration(end_q_rg);
    std::cout << dat_files[i] << " Rg= " << rg << std::endl;
  }

  // 3. compute rg for input pdbs
  for(unsigned int i=0; i<pdb_files.size(); i++) {
    double rg = IMP::saxs::radius_of_gyration(particles_vec[i]);
    std::cout << pdb_files[i] << " Rg= " << rg << std::endl;
  }
  return 0;
}
