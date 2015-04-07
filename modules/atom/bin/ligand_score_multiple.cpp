/**
 *  \file ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/mol2.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/particle_index.h>
#include <IMP/kernel/Model.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

namespace {

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms) {
  std::ifstream trans_file(file_name.c_str());
  if(!trans_file) {
    std::cerr << "Can't find Transformation file " << file_name << std::endl;
    exit(1);
  }

  IMP::algebra::Vector3D rotation_vec, translation;
  int trans_number;

  while(trans_file >> trans_number >> rotation_vec >> translation) {
    IMP::algebra::Rotation3D rotation =
      IMP::algebra::get_rotation_from_fixed_xyz(rotation_vec[0],
                                                rotation_vec[1],
                                                rotation_vec[2]);
    IMP::algebra::Transformation3D trans(rotation, translation);
    transforms.push_back(trans);
  }
  trans_file.close();
  std::cout << transforms.size() << " transforms were read from "
            << file_name << std::endl;
}

}

int main(int argc, char *argv[]) {
  // print command
  for(int i=0; i<argc; i++) std::cerr << argv[i] << " "; std::cerr << std::endl;

  IMP::base::set_log_level(IMP::base::SILENT);
  std::string mol2name, pdbname, trans_file, out_file_name;

  po::options_description desc("Usage: <pdb> <mol2> [trans file]");
  desc.add_options()
    ("help", "static and transformed molecules from docking with \
transformation file.")
    ("input-files", po::value< std::vector<std::string> >(),
     "input PDB, transformation and profile files")
    ("output_file,o",
     po::value<std::string>(&out_file_name)->default_value("mol2_score.res"),
     "output file name, default name mol2_score.res");

  po::positional_options_description p;
  p.add("input-files", -1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  // parse filenames
  std::vector<std::string> files;
  if(vm.count("input-files")) {
    files = vm["input-files"].as< std::vector<std::string> >();
  }
  if(vm.count("help") || (files.size() !=2 && files.size() != 3)) {
    std::cout << desc << "\n"; return 0;
  }
  pdbname = files[0];
  mol2name = files[1];
  trans_file = files[2];

 
  IMP::base::TextInput lib;
  /*
  if (argc==4) {
    lib= IMP::base::TextInput(argv[3]);
  }
  */

  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::atom::Hierarchy protein, ligand;
  {
    IMP::base::SetLogState ss(IMP::base::SILENT);
    protein = IMP::atom::read_pdb(pdbname, m, new IMP::atom::ATOMPDBSelector());
    IMP::atom::add_protein_ligand_score_data(protein);
    ligand = IMP::atom::read_mol2(mol2name, m);
    IMP::atom::add_protein_ligand_score_data(ligand);
  }
  IMP::atom::Hierarchies mols
    = IMP::atom::get_by_type(ligand, IMP::atom::RESIDUE_TYPE);
  IMP::base::Pointer<IMP::atom::ProteinLigandAtomPairScore> ps;
  if (lib) {
    ps = new IMP::atom::ProteinLigandAtomPairScore(100000, lib);
  } else {
    ps = new IMP::atom::ProteinLigandAtomPairScore();
  }
  double d= ps->get_maximum_distance();
  IMP_NEW(IMP::core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(d);

  IMP::kernel::ParticlesTemp patoms= IMP::atom::get_leaves(protein);
  IMP::kernel::ParticleIndexes ipatoms= IMP::get_indexes(patoms);


  std::vector<IMP::kernel::ParticlesTemp> latoms(mols.size());
  std::vector<IMP::kernel::ParticleIndexes> ilatoms(mols.size());
  for (unsigned int i=0; i< mols.size(); ++i) {
    latoms[i] = IMP::atom::get_leaves(mols[i]);
    ilatoms[i] = IMP::get_indexes(latoms[i]);
  }

  // read tranformations
  std::vector<IMP::algebra::Transformation3D> transforms;
  if (trans_file.length() > 0)
    read_trans_file(trans_file, transforms);
  else
    transforms.push_back(IMP::algebra::get_identity_transformation_3d());

  // save original coordinates
  std::vector<IMP::algebra::Vector3Ds> mol2_coordinates(mols.size());
  for (unsigned int i=0; i< mols.size(); ++i) {
    for (unsigned int l_index=0; l_index<ilatoms[i].size(); l_index++) {
      IMP::core::XYZ d(m, ilatoms[i][l_index]);
      IMP::algebra::Vector3D v(d.get_coordinates());
      mol2_coordinates[i].push_back(v);
    }
  }

  // iterate transformations
  std::ofstream out_file(out_file_name.c_str());
  for (unsigned int t=0; t<transforms.size(); t++) {

    // score
    for (unsigned int i=0; i< mols.size(); ++i) {
      // apply transformation on each particle
      for (unsigned int l_index=0; l_index<ilatoms[i].size(); l_index++) {
	IMP::core::XYZ d(m, ilatoms[i][l_index]);
	d.set_coordinates(transforms[t]*d.get_coordinates());
      }

      //IMP::SetLogState ss(i==0? TERSE: IMP::SILENT);
      IMP::kernel::ParticleIndexPairs ppt= gcpf->get_close_pairs(m, ipatoms, ilatoms[i]);
      double score=ps->evaluate_indexes(m, ppt, NULL, 0, ppt.size());
      out_file << "Score for " << mols[i]->get_name() << " trans " << t << " is "
		<< score << std::endl;
      
      // return the ligand back
      for (unsigned int l_index=0; l_index<ilatoms[i].size(); l_index++) {
	IMP::core::XYZ d(m, ilatoms[i][l_index]);
	d.set_coordinates(mol2_coordinates[i][l_index]);
      }
    }

  }
  out_file.close();
  ps->set_was_used(true);
  return EXIT_SUCCESS;
}
