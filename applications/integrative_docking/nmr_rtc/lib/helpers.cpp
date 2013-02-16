/**
 *  \file helpers.cpp  \brief A set of helper functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "nmr_helpers.h"

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

void get_residue_solvent_accessibility(const IMP::Particles& atom_particles,
                                       const IMP::Particles& residue_particles,
                                     const std::vector<int>& atom_2_residue_map,
                            std::vector<float>& residue_solvent_accessibility) {

  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  IMP::saxs::FormFactorType ff_type = IMP::saxs::HEAVY_ATOMS;
  for(unsigned int i=0; i<atom_particles.size(); i++) {
    float radius = ft->get_radius(atom_particles[i], ff_type);
    IMP::core::XYZR::setup_particle(atom_particles[i], 2.0*radius);
  }
  IMP::Floats surface_area;
  IMP::saxs::SolventAccessibleSurface s;
  surface_area = s.get_solvent_accessibility(IMP::core::XYZRs(atom_particles));

  // sum up individual atom areas into residue area
  residue_solvent_accessibility.clear();
  residue_solvent_accessibility.insert(residue_solvent_accessibility.begin(),
                                       residue_particles.size(), 0.0);
  for(unsigned int i=0; i<atom_particles.size(); i++) {
    // convert from 0-1 range to actual area in A^2
    float radius = IMP::core::XYZR(atom_particles[i]).get_radius();
    float area = surface_area[i] * 4*IMP::algebra::PI*radius*radius;
    residue_solvent_accessibility[atom_2_residue_map[i]] += area;
  }

  // compute solvent accessability percentage
  std::map<IMP::atom::ResidueType, float> residue_type_area_map_;
  residue_type_area_map_[IMP::atom::ALA] = 113.0;
  residue_type_area_map_[IMP::atom::ARG] = 241.0;
  residue_type_area_map_[IMP::atom::ASP] = 151.0;
  residue_type_area_map_[IMP::atom::ASN] = 158.0;
  residue_type_area_map_[IMP::atom::CYS] = 140.0;
  residue_type_area_map_[IMP::atom::GLN] = 189.0;
  residue_type_area_map_[IMP::atom::GLU] = 183.0;
  residue_type_area_map_[IMP::atom::GLY] = 85.0;
  residue_type_area_map_[IMP::atom::HIS] = 194.0;
  residue_type_area_map_[IMP::atom::ILE] = 182.0;
  residue_type_area_map_[IMP::atom::LEU] = 180.0;
  residue_type_area_map_[IMP::atom::LYS] = 211.0;
  residue_type_area_map_[IMP::atom::MET] = 204.0;
  residue_type_area_map_[IMP::atom::PHE] = 218.0;
  residue_type_area_map_[IMP::atom::PRO] = 143.0;
  residue_type_area_map_[IMP::atom::SER] = 122.0;
  residue_type_area_map_[IMP::atom::THR] = 146.0;
  residue_type_area_map_[IMP::atom::TYR] = 229.0;
  residue_type_area_map_[IMP::atom::TRP] = 259.0;
  residue_type_area_map_[IMP::atom::VAL] = 160.0;
  residue_type_area_map_[IMP::atom::UNK] = 113.0;

  for(unsigned int i=0; i<residue_particles.size(); i++) {
    IMP::atom::ResidueType residue_type =
      IMP::atom::Residue(residue_particles[i]).get_residue_type();
    float residue_area = residue_type_area_map_.find(residue_type)->second;
    std::cerr << residue_type.get_string() << " "
              << residue_solvent_accessibility[i]
              << " " << residue_area << std::endl;
    residue_solvent_accessibility[i] /= residue_area;
  }
}
