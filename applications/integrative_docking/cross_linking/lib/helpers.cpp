/**
 *  \file helpers.cpp  \brief A set of helper functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "helpers.h"

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/algebra/constants.h>
#include <fstream>
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

void transform(IMP::Particles& ps, IMP::algebra::Transformation3D& t) {
  for(IMP::Particles::iterator it = ps.begin(); it != ps.end(); it++) {
    IMP::core::XYZ d(*it);
    d.set_coordinates(t * d.get_coordinates());
  }
}

void get_atom_2_residue_map(const IMP::Particles& atom_particles,
                            const IMP::Particles& residue_particles,
                            std::vector<int>& atom_2_residue_map) {
  atom_2_residue_map.resize(atom_particles.size());
  unsigned int residue_index=0;
  for(unsigned int atom_index=0; atom_index<atom_particles.size(); ) {
    if(get_residue(IMP::atom::Atom(atom_particles[atom_index])).get_particle()
       == residue_particles[residue_index]) {
      atom_2_residue_map[atom_index] = residue_index;
      atom_index++;
    } else {
      residue_index++;
    }
  }
}

IMP::algebra::Vector3D get_ca_coordinate(const IMP::Particles& ca_atoms,
                                         int residue_index, char chain_id) {
  IMP::algebra::Vector3D v(0,0,0);
  for(unsigned int i=0; i<ca_atoms.size(); i++) {
    IMP::atom::Residue r = IMP::atom::get_residue(IMP::atom::Atom(ca_atoms[i]));
    int curr_residue_index = r.get_index();
    char curr_chain_id =
      IMP::atom::get_chain(IMP::atom::Atom(ca_atoms[i])).get_id();
    if(curr_residue_index == residue_index && curr_chain_id == chain_id) {
      IMP::algebra::Vector3D v = IMP::core::XYZ(ca_atoms[i]).get_coordinates();
      return v;
    }
  }
  std::cerr << "Residue not found " << residue_index
            << " " << chain_id << std::endl;
  exit(1);
  //  return v;
}

void get_residue_solvent_accessibility(const IMP::Particles& residue_particles,
                                  IMP::Floats& residue_solvent_accessibility) {
  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  IMP::saxs::FormFactorType ff_type = IMP::saxs::CA_ATOMS;
  for(unsigned int p_index=0; p_index<residue_particles.size(); p_index++) {
    float radius = ft->get_radius(residue_particles[p_index], ff_type);
    IMP::core::XYZR::setup_particle(residue_particles[p_index], radius);
  }
  IMP::saxs::SolventAccessibleSurface s;
  residue_solvent_accessibility =
    s.get_solvent_accessibility(IMP::core::XYZRs(residue_particles));
}
