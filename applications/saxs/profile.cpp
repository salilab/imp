/**
   This is the program for SAXS profile computation and fitting.
   see FOXS for webserver (salilab.org/foxs)
 */
#include <IMP/Model.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <IMP/atom/CharmmParameters.h>

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/Score.h>
#include <IMP/saxs/SolventAccessibleSurface.h>

#include "Gnuplot.h"

#include <fstream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#if defined(_WIN32) || defined(_WIN64)
// Simple basename implementation on platforms that don't have libgen.h
const char *basename(const char *path)
{
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

int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  int profile_size = 500;
  float max_q = 0.5;
  float background_adjustment_q = 0.0;
  bool use_offset = false;
  bool fit = true;
  bool hydration_layer = true;
  bool autocorrelation = true;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB and profile files")
    ("max_q,q", po::value<float>(&max_q)->default_value(0.5),
     "maximal q value (default = 0.5)")
    ("profile_size,s", po::value<int>(&profile_size)->default_value(500),
     "number of points in the profile (default = 500)")
    ("parameter_fit,p",
     "fit by adjusting excluded volume and hydration layer density parameters \
 (default = true)")
    ("hydration_layer,h", "compute hydration layer (default = true)")
    ("background_q,b",
     po::value<float>(&background_adjustment_q)->default_value(0.0),
     "background adjustment, not used by default. if enabled, \
 recommended q value is 0.2")
    ("offset,o", "use offset in fitting (default = false)")
    ;
  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }
  std::vector<std::string> files, pdb_files, dat_files;
  if (vm.count("input-files"))   {
    files = vm["input-files"].as< std::vector<std::string> >();
    // std::cout << "Input files are: ";
    // for(unsigned int i=0; i<files.size(); i++) std::cout << files[i]<<  "\n";
  }
  if (vm.count("parameter_fit")) fit=false;
  if (vm.count("hydration_layer")) hydration_layer=false;
  if (vm.count("offset")) use_offset=true;

  float delta_q = max_q / profile_size;
  bool interactive_gnuplot = false; // for server

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
                            IMP::atom::NonWaterNonHydrogenPDBSelector());
      IMP::atom::add_radii(mhd); //, cp);
      IMP::Particles particles = get_by_type(mhd, IMP::atom::ATOM_TYPE);
      if(particles.size() > 0) { // pdb file
        pdb_files.push_back(files[i]);
        particles_vec.push_back(particles);
        std::cout << particles.size() << " atoms were read from PDB file "
                  << files[i] << std::endl;
      }
    } catch(IMP::IOException e) { // not a pdb file
      // B. try as dat file
      IMP::saxs::Profile *profile = new IMP::saxs::Profile(files[i]);
      if(profile->size() == 0) {
        std::cerr << "can't parse input file " << files[i] << std::endl;
        return 1;
      } else {
        dat_files.push_back(files[i]);
        if(background_adjustment_q > 0.0) {
          profile->background_adjust(background_adjustment_q);
        }
        exp_profiles.push_back(profile);
        std::cout << "Profile read from file " << files[i] << " size = "
                  << profile->size() << std::endl;
      }
    }
  }

  // 2. compute profiles for input pdbs
  std::vector<IMP::saxs::Profile *> profiles;
  for(unsigned int i=0; i<pdb_files.size(); i++) {
    // compute surface accessibility
    IMP::Floats surface_area;
    IMP::saxs::SolventAccessibleSurface s;
    if(hydration_layer)
      surface_area =
        s.get_solvent_accessibility(IMP::core::XYZRs(particles_vec[i]));

    // compute profile
    IMP::saxs::Profile *partial_profile = NULL;
    std::cerr << "Computing profile for " << pdb_files[i] << " min = 0.0 max = "
              << max_q << " delta=" << delta_q << std::endl;
    partial_profile = new IMP::saxs::Profile(0.0, max_q, delta_q);
    if(dat_files.size() > 0) {
      partial_profile->calculate_profile_partial(particles_vec[i],
                                                 surface_area, autocorrelation);
    } else {
      partial_profile->calculate_profile(particles_vec[i],
                                         false, autocorrelation);
    }
    profiles.push_back(partial_profile);
    // write profile file
    std::string profile_file_name = std::string(pdb_files[i]) + ".dat";
    partial_profile->add_errors();
    partial_profile->write_SAXS_file(profile_file_name);
    Gnuplot::print_profile_script(pdb_files[i], interactive_gnuplot);

    // 3. fit experimental profiles
    for(unsigned int j=0; j<dat_files.size(); j++) {
      IMP::saxs::Profile* exp_saxs_profile = exp_profiles[j];
      IMP::Pointer<IMP::saxs::Score> saxs_score =
        new IMP::saxs::Score(*exp_saxs_profile);
      partial_profile->sum_partial_profiles(1.0, 0.0, *partial_profile);
      float chi = saxs_score->compute_chi_score(*partial_profile, use_offset);
      std::cout << "Chi  = " << chi << std::endl;

      // try to fit exp data with two parameters
      if(fit) {
        float best_c1 = 1.0, best_c2 = 0.0;
        float best_chi = chi;
        for(float c1 = 0.8; c1<=1.2; c1+= 0.005) {
          for(float c2 = 0.0; c2<=4.0; c2+= 0.1) {
            IMP::saxs::Profile p(0.0, max_q, delta_q);
            partial_profile->sum_partial_profiles(c1, c2, p);
            float curr_chi = saxs_score->compute_chi_score(p, use_offset);
            if(curr_chi < best_chi) {
              best_chi = curr_chi;
              best_c1 = c1;
              best_c2 = c2;
              saxs_score->compute_chi_score(p, use_offset);
            }
            //std::cerr << "c1 = " << c1 << " c2 = " << c2
            // << " chi = " << curr_chi << std::endl;
            if(surface_area.size() == 0) break;
          }
        }

        // store best fit into partial_profile and print
        std::cout << pdb_files[i] << " Chi = " << best_chi << " c1 = "
                  << best_c1 << " c2 = " << best_c2 << std::endl;
        partial_profile->sum_partial_profiles(best_c1, best_c2,
                                              *partial_profile);
      }
      std::string fit_file_name2 = trim_extension(pdb_files[i]) + "_" +
        trim_extension(basename(const_cast<char *>(dat_files[j].c_str())))
        + ".dat";
      saxs_score->compute_chi_score(*partial_profile,use_offset,fit_file_name2);
      Gnuplot::print_fit_script(pdb_files[i], dat_files[j],interactive_gnuplot);
    }
  }

  if(pdb_files.size() > 1) {
    Gnuplot::print_profile_script(pdb_files, interactive_gnuplot);
    if(dat_files.size() > 0) Gnuplot::print_fit_script(pdb_files, dat_files[0],
                                                       interactive_gnuplot);
  }
  return 0;
}
