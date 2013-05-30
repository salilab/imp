/**
   Profile clustering program
   Author: Dina Schneidman
*/
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ChiScore.h>

#include <vector>
#include <string>
#include <map>

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;


int main(int argc, char **argv)
{
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  float threshold = 1.0;
  po::options_description desc("Usage: <mes_input_file>\n \
Provide a text file with profile filenames:\n \
exp_profile_file\nfit_file1\nfit_file2\n...\nfit_filen\n");
  desc.add_options()
    ("help", "Clusters input profiles that were previously fitted \
to exp_profile. Please profile the exp_profile and at least two fit files.\n")
    ("input-files", po::value< std::vector<std::string> >(),
     "input profile files")
    ("threshold,t", po::value<float>(&threshold)->default_value(1.0),
     "chi value for profile similarity (default = 1.0)")
    ;

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files;
  if (vm.count("input-files"))   {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() != 1) {
    std::cout << desc << "\n";
    return 0;
  }

  // read the profiles
  IMP::saxs::Profile* exp_profile;
  std::map<int , std::pair<std::string, IMP::saxs::Profile *> > fit_profiles;
  bool fit_file = false;
  // open input file with profile file names - mes format
  std::ifstream in_file(files[0].c_str());
  if(!in_file) {
    IMP_THROW("Can't find input file " << files[0], IOException);
  }
  std::string curr_file_name;
  unsigned int profile_counter=0;
  while (!in_file.eof()) {
    getline(in_file, curr_file_name);
    boost::trim(curr_file_name); // remove all spaces
    if(curr_file_name.length() == 0) continue;

    if(profile_counter > 0) fit_file = true;

    // read a profile from current file
    IMP::saxs::Profile *profile =
      new IMP::saxs::Profile(curr_file_name, fit_file);
    if(profile->size() == 0) {
      IMP_THROW("Can't parse input file " << curr_file_name, IOException);
    } else {
      if(profile_counter == 0) {
        exp_profile = profile;
      } else {
        profile->copy_errors(*exp_profile);
        fit_profiles[profile_counter] = std::make_pair(curr_file_name, profile);
      }
      profile_counter++;
      std::cout << "Profile read from file " << curr_file_name << " size = "
                << profile->size() << std::endl;
    }
  }

  // compute Chi values
  std::multimap<float, int> scored_profiles;
  IMP::saxs::ChiScore chi_score;
  std::map<int , std::pair<std::string, IMP::saxs::Profile *> >::iterator it;
  for(it = fit_profiles.begin(); it != fit_profiles.end(); it++) {
    IMP::saxs::Profile *curr_profile = it->second.second;
    float score = chi_score.compute_score(*exp_profile, *curr_profile);
    scored_profiles.insert(std::make_pair(score, it->first));
  }

  // cluster
  std::multimap<float, int> clustered_profiles; //needed?
  std::multimap<float, int> &temp_profiles(scored_profiles);
  int cluster_number = 1;
  while(!temp_profiles.empty()) {
    std::cerr << "Cluster_Number = " << cluster_number << std::endl;
    int cluster_profile_id = temp_profiles.begin()->second;
    IMP::saxs::Profile *cluster_profile =
      fit_profiles[cluster_profile_id].second;
    std::string cluster_file_name = fit_profiles[cluster_profile_id].first;
    std::cerr << cluster_profile_id << " score " << temp_profiles.begin()->first
              << " file " << cluster_file_name << std::endl;

    // remove first
    temp_profiles.erase(temp_profiles.begin());

    std::multimap<float, int>::iterator it = temp_profiles.begin();
    // iterate over the rest of the profiles and erase similar ones
    while(it != temp_profiles.end()) {
      int curr_profile_id = it->second;
      IMP::saxs::Profile *curr_profile = fit_profiles[curr_profile_id].second;
      std::string curr_file_name = fit_profiles[curr_profile_id].first;

      float score = chi_score.compute_score(*cluster_profile, *curr_profile);
      if(score < threshold) {
        std::cerr << curr_profile_id << " score " << score
                  << " file " << curr_file_name << std::endl;
        temp_profiles.erase(it++);
      } else {
        it++;
      }
    }
    cluster_number++;
  }
}
