/**
 *  \file helpers.cpp  \brief A set of helper functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "helpers.h"

#include <IMP/atom/pdb.h>

#include <vector>
#include <string>

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

void read_pdb_atoms(const std::string file_name, IMP::Particles& particles) {
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(file_name, model,
                             new IMP::atom::NonWaterNonHydrogenPDBSelector(),
                                                 true, true);
  particles=IMP::get_as<IMP::Particles>(get_by_type(mhd, IMP::atom::ATOM_TYPE));
  std::cout << "Number of atom particles " << particles.size() << std::endl;
}

void read_pdb_ca_atoms(const std::string file_name, IMP::Particles& particles) {
  IMP::Model *model = new IMP::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(file_name, model,
                             new IMP::atom::CAlphaPDBSelector(), true, true);
  particles=IMP::get_as<IMP::Particles>(get_by_type(mhd, IMP::atom::ATOM_TYPE));
  std::cout << "Number of CA atom particles " << particles.size() << std::endl;
}

void transform(IMP::Particles& ps, IMP::algebra::Transformation3D& t) {
  for(IMP::Particles::iterator it = ps.begin(); it != ps.end(); it++) {
    IMP::core::XYZ d(*it);
    d.set_coordinates(t * d.get_coordinates());
  }
}
