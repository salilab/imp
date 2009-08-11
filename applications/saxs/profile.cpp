/**
   This is a basic program for SAXS profile computation.
   The program computed the profile for an input PDB.
   If experimental profile is given, it is compared against the
   computational one and the chi value for the fit is printed.
 */

#include <IMP/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Atom.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Score.h>
#include <IMP/saxs/FormFactorTable.h>

#include <fstream>
#include <vector>
#include <string>
#include <libgen.h>     // Use basename()

std::string trim_extension(const std::string file_name) {
 if(file_name[file_name.size()-4] == '.')
   return file_name.substr(0, file_name.size() - 4);
 return file_name;
}

int main(int argc, char **argv)
{
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  if(argc < 2) {
    std::cerr <<"Usage: " <<"<pdb file> [Optional: profile files]" << std::endl;
    exit(1);
  }

  int profile_size = 100;
  float max_q = 0.5;
  bool use_offset = false;

  //const char* ff_file_name = argv[1];
  const char* pdb = argv[1];
  std::vector<std::string> dat_files;
  for(int i=2 ; i< argc; i++) {
    char* dat_file = argv[i];
    dat_files.push_back(dat_file);
  }

  float delta_q = max_q / profile_size;
  IMP::saxs::FormFactorTable* f_table = IMP::saxs::default_form_factor_table();

  // read pdb, prepare particles
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd =
    IMP::atom::read_pdb(pdb, model, IMP::atom::NonWaterNonHydrogenSelector());
  IMP::Particles particles = get_by_type(mhd, IMP::atom::Hierarchy::ATOM);

  // either computed or read from dat file
  IMP::saxs::Profile *computational_profile = NULL;

  if(particles.size() > 0) { // compute profile
    computational_profile = new IMP::saxs::Profile(f_table, 0.0, max_q,delta_q);
    std::cerr << "Computing profile for " << pdb << " min = 0.0 max = " << max_q
              << " delta=" << delta_q << std::endl;
    computational_profile->calculate_profile(particles);
    std::string profile_file_name = std::string(pdb) + ".dat";
    computational_profile->write_SAXS_file(profile_file_name);
  } else { // maybe already computed, try to read as profile file
    computational_profile = new IMP::saxs::Profile(pdb);
    if(computational_profile->size() == 0) {
      std::cerr << "invalid input " << std::endl;
      return 1;
    }
  }

  // fit profiles
  for(unsigned int i=0; i<dat_files.size(); i++) {
    IMP::saxs::Profile exp_saxs_profile(dat_files[i]);
    IMP::saxs::Score saxs_score(&exp_saxs_profile);
    std::string fit_file_name = trim_extension(pdb) + "_" +
      trim_extension(basename(const_cast<char *>(dat_files[i].c_str())))
      + ".dat";
    float chi = saxs_score.compute_chi_score(*computational_profile,
                                             use_offset, fit_file_name);
    std::cout << "Chi  = " << chi << std::endl;

    // print gnuplot script
    std::string plt_file_name = trim_extension(pdb) + "_" +
      trim_extension(basename(const_cast<char *>(dat_files[i].c_str())))
      + ".plt";
    std::ofstream plt_file(plt_file_name.c_str());
    plt_file << "plot '" << fit_file_name << "' u 1:2 thru log(y), '"
             << fit_file_name << "' u 1:3 thru log(y)" << std::endl;
    plt_file.close();
  }
  return 0;
}
