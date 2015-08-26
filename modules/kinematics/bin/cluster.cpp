/**
   RMSD clustering
   Author: Dina Schneidman
*/

#include <IMP/kinematics/RMSDClustering.h>

#include <IMP/atom/pdb.h>
#include <IMP/saxs/utility.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <fstream>
#include <vector>
#include <string>

namespace {
std::string trim_extension(const std::string file_name) {
  if(file_name[file_name.size()-4] == '.')
    return file_name.substr(0, file_name.size() - 4);
  return file_name;
}

void read_pdb(const std::string file,
              std::vector<std::string>& pdb_file_names,
              std::vector<IMP::algebra::Vector3Ds>& particles_vec,
              int multi_model_pdb) {

  IMP::Model *model = new IMP::Model();

  IMP::atom::Hierarchies mhds;
  IMP::atom::PDBSelector* selector = new IMP::atom::CAlphaPDBSelector();

  if(multi_model_pdb == 2) {
    mhds = read_multimodel_pdb(file, model, selector, true);
  } else {
    if(multi_model_pdb == 3) {
      IMP::atom::Hierarchy mhd =
        IMP::atom::read_pdb(file, model, selector, false, true);
      mhds.push_back(mhd);
    } else {
      IMP::atom::Hierarchy mhd =
        IMP::atom::read_pdb(file, model, selector, true, true);
      mhds.push_back(mhd);
    }
  }

  for(unsigned int h_index=0; h_index<mhds.size(); h_index++) {
    IMP::ParticlesTemp ps = get_by_type(mhds[h_index], IMP::atom::ATOM_TYPE);
    if(ps.size() > 0) { // pdb file
      std::string pdb_id = file;
      if(mhds.size() > 1) {
        pdb_id = trim_extension(file) + "_m" +
          std::string(boost::lexical_cast<std::string>(h_index+1)) + ".pdb";
      }
      pdb_file_names.push_back(pdb_id);
      IMP::algebra::Vector3Ds coords;
      IMP::saxs::get_coordinates(ps, coords);
      particles_vec.push_back(coords);
      std::cout << ps.size() << " atoms were read from PDB file " << file;
      if(mhds.size() > 1) std::cout << " MODEL " << h_index+1;
      std::cout << std::endl;
    }
  }
}


void read_filenames(const std::string filenames_file,
                    std::vector<std::pair<double,
                                          std::string> >& file_name_score_pairs) {
  // open input file with profile file names
  std::ifstream in_file(filenames_file.c_str());
  if(!in_file) {
    IMP_THROW("Can't find input file " << filenames_file, IMP::IOException);
  }
  std::string curr_file_name;
  unsigned int counter=0;
  while (!in_file.eof()) {
    getline(in_file, curr_file_name);
    boost::trim(curr_file_name); // remove all spaces
    std::vector < std::string > split_results;
    boost::split(split_results, curr_file_name, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() < 1 || split_results.size() > 2) continue;
    curr_file_name = split_results[0].c_str();
    if(curr_file_name.length() <= 0) continue;
    double score = 0;
    if(split_results.size() > 1) {
      score = atof(split_results[1].c_str());
    }
    //file_names.push_back(curr_file_name);
    //if(split_results.size() > 1) scores.push_back(score);
    file_name_score_pairs.push_back(std::make_pair(score, curr_file_name));
    counter++;
  }
  std::cout << counter << " filenames were read" << std::endl;
}

}


int main(int argc, char* argv[]) {
  // output arguments
  for (int i = 0; i < argc; i++) std::cerr << argv[i] << " ";
  std::cerr << std::endl;

  float rmsd_threshold = 2.0;
  bool transformation = true;
  po::options_description
    desc("Usage: <filenames> <pdb_file1> <pdb_file2> ...");
  desc.add_options()
    ("help", "filenames file includes filenames and their scores")
    ("input-files", po::value< std::vector<std::string> >(),
     "input profile files")
    ("rmsd-threshold,r", po::value<float>(&rmsd_threshold)->default_value(2.0),
     "rmsd threshold for structure similarity (default = 2.0)")
    ("transformation,t",
     "calculate transformation that minimizes RMSD (default = true)")
    ;

   po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || files.size() == 0) {
    std::cout << desc << "\n";
    return 0;
  }
  if (vm.count("transformation")) transformation = false;

  // read the scores
  std::vector<std::pair<double, std::string> > file_name_score_pairs;
  read_filenames(files[0], file_name_score_pairs);
  std::sort(file_name_score_pairs.begin(), file_name_score_pairs.end());

  // read the pdbs
  std::vector<std::string> pdb_file_names;
  std::vector<IMP::algebra::Vector3Ds> coords_vec;
  for (unsigned int i=1; i<files.size(); i++) {
    read_pdb(files[i], pdb_file_names, coords_vec, 2);
  }

  if (pdb_file_names.size() != file_name_score_pairs.size()) {
    std::cerr << "Different number of files in " << files[0] << " "
              << file_name_score_pairs.size() << " vs. number of pdbs "
              << pdb_file_names.size() << std::endl;
    exit(0);
  }

  // map pdb_file_name to index
  std::map<std::string, int> pdb_to_index;
  for (unsigned int i=0; i<pdb_file_names.size(); i++) {
    pdb_to_index[pdb_file_names[i]] = i;
  }

  // sort the structures
  std::vector<IMP::algebra::Vector3Ds> sorted_coords_vec(coords_vec.size());
  for (unsigned int i=0; i<file_name_score_pairs.size(); i++) {
    int index = pdb_to_index[file_name_score_pairs[i].second];
    sorted_coords_vec[i] = coords_vec[index];
  }

  coords_vec.clear();

  // cluster
  std::vector<int> out_cluster_numbers;
  unsigned int number_of_clusters =
    IMP::kinematics::rmsd_clustering(sorted_coords_vec,
                                     out_cluster_numbers,
                                     rmsd_threshold,
                                     transformation);

  std::cout << "Number of clusters with RMSD " << rmsd_threshold << ": "
            << number_of_clusters << std::endl;

  // print representatives
  int curr_cluster = -1;
  std::ofstream out_file("structural_clusters.txt");
  for (unsigned int i=0; i<out_cluster_numbers.size(); i++) {
    if(out_cluster_numbers[i] > curr_cluster) { // first cluster member
      curr_cluster = out_cluster_numbers[i];
      out_file << file_name_score_pairs[i].second << std::endl;
    }
  }
  out_file.close();
}
