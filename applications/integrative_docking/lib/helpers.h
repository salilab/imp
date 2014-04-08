/**
 *  \file helpers.h   \brief A set of helper functions
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_HELPERS_H
#define IMP_HELPERS_H

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/saxs/FormFactorTable.h>
#include <IMP/algebra/constants.h>
#include <IMP/kernel/Particle.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/DopePairScore.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMStereochemistryRestraint.h>

#include <vector>
#include <string>

namespace {

IMP::atom::Hierarchy read_pdb(const std::string pdb_file_name,
                              IMP::kernel::Model* model,
                              IMP::kernel::ParticleIndexes& pis) {

  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      pdb_file_name, model, new IMP::atom::NonWaterNonHydrogenPDBSelector(),
      true, true);

  pis = IMP::get_as<IMP::kernel::ParticleIndexes>(
      get_by_type(mhd, IMP::atom::ATOM_TYPE));
  std::cout << pis.size() << " atoms read from " << pdb_file_name << std::endl;
  IMP::atom::add_dope_score_data(mhd);
  return mhd;
}

IMP::ParticlesTemp add_bonds(IMP::atom::Hierarchy mhd) {
  IMP::atom::CHARMMParameters *ff =
    IMP::atom::get_heavy_atom_CHARMM_parameters();
  IMP::base::Pointer<IMP::atom::CHARMMTopology> topology =
    ff->create_topology(mhd);
  topology->add_atom_types(mhd);

  IMP::ParticlesTemp bonds =  topology->add_bonds(mhd);
  return bonds;
}

void read_pdb_atoms(const std::string file_name,
                    IMP::kernel::Particles& particles) {
  IMP::kernel::Model* model = new IMP::kernel::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      file_name, model, new IMP::atom::NonWaterNonHydrogenPDBSelector(), true,
      true);
  particles = IMP::get_as<IMP::kernel::Particles>(
      get_by_type(mhd, IMP::atom::ATOM_TYPE));
  std::cout << "Number of atom particles " << particles.size() << std::endl;
}

void read_pdb_ca_atoms(const std::string file_name,
                       IMP::kernel::Particles& particles) {
  IMP::kernel::Model* model = new IMP::kernel::Model();
  IMP::atom::Hierarchy mhd = IMP::atom::read_pdb(
      file_name, model, new IMP::atom::CAlphaPDBSelector(), true, true);
  particles = IMP::get_as<IMP::kernel::Particles>(
      get_by_type(mhd, IMP::atom::ATOM_TYPE));
  std::cout << "Number of CA atom particles " << particles.size() << std::endl;
}

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms) {
  std::ifstream trans_file(file_name.c_str());
  if (!trans_file) {
    std::cerr << "Can't find Transformation file " << file_name << std::endl;
    exit(1);
  }

  IMP::algebra::Vector3D rotation_vec, translation;
  int trans_number;

  while (trans_file >> trans_number >> rotation_vec >> translation) {
    IMP::algebra::Rotation3D rotation =
        IMP::algebra::get_rotation_from_fixed_xyz(
            rotation_vec[0], rotation_vec[1], rotation_vec[2]);
    IMP::algebra::Transformation3D trans(rotation, translation);
    transforms.push_back(trans);
  }
  trans_file.close();
  std::cout << transforms.size() << " transforms were read from " << file_name
            << std::endl;
}

void transform(IMP::Particles& ps, IMP::algebra::Transformation3D& t) {
  for (IMP::Particles::iterator it = ps.begin(); it != ps.end(); it++) {
    IMP::core::XYZ d(*it);
    d.set_coordinates(t * d.get_coordinates());
  }
}

void transform(IMP::kernel::Model* model, IMP::kernel::ParticleIndexes& pis,
               const IMP::algebra::Transformation3D& t) {
  for (unsigned int i = 0; i < pis.size(); i++) {
    IMP::core::XYZ d(model, pis[i]);
    d.set_coordinates(t * d.get_coordinates());
  }
}

void get_atom_2_residue_map(const IMP::Particles& atom_particles,
                            const IMP::Particles& residue_particles,
                            std::vector<int>& atom_2_residue_map) {
  atom_2_residue_map.resize(atom_particles.size());
  unsigned int residue_index = 0;
  for (unsigned int atom_index = 0; atom_index < atom_particles.size();) {
    if (get_residue(IMP::atom::Atom(atom_particles[atom_index]))
            .get_particle() == residue_particles[residue_index]) {
      atom_2_residue_map[atom_index] = residue_index;
      atom_index++;
    } else {
      residue_index++;
    }
  }
}

IMP::algebra::Vector3D get_ca_coordinate(const IMP::kernel::Particles& ca_atoms,
                                         int residue_index,
                                         std::string chain_id) {
  IMP::algebra::Vector3D v(0, 0, 0);
  for (unsigned int i = 0; i < ca_atoms.size(); i++) {
    IMP::atom::Residue r = IMP::atom::get_residue(IMP::atom::Atom(ca_atoms[i]));
    int curr_residue_index = r.get_index();
    std::string curr_chain_id =
        IMP::atom::get_chain_id(IMP::atom::Atom(ca_atoms[i]));
    if (curr_residue_index == residue_index && curr_chain_id == chain_id) {
      IMP::algebra::Vector3D v = IMP::core::XYZ(ca_atoms[i]).get_coordinates();
      return v;
    }
  }
  std::cerr << "Residue not found " << residue_index << " " << chain_id
            << std::endl;
  exit(1);
  //  return v;
}

void get_residue_solvent_accessibility(
    const IMP::kernel::Particles& residue_particles,
    IMP::Floats& residue_solvent_accessibility) {
  IMP::saxs::FormFactorTable* ft = IMP::saxs::default_form_factor_table();
  IMP::saxs::FormFactorType ff_type = IMP::saxs::CA_ATOMS;
  for (unsigned int p_index = 0; p_index < residue_particles.size();
       p_index++) {
    float radius = ft->get_radius(residue_particles[p_index], ff_type);
    IMP::core::XYZR::setup_particle(residue_particles[p_index], radius);
  }
  IMP::saxs::SolventAccessibleSurface s;
  residue_solvent_accessibility =
      s.get_solvent_accessibility(IMP::core::XYZRs(residue_particles));
}


IMP::atom::Residue find_residue(const IMP::ParticlesTemp& residues,
                                int res_index, std::string chain) {
  for(unsigned int i=0; i<residues.size(); i++) {
    IMP::atom::Residue rd = IMP::atom::Residue(residues[i]);
    if(rd.get_index() == res_index &&
       IMP::atom::get_chain(rd).get_id() == chain) return rd;
  }
  std::cerr << "Residue not found " << res_index << chain << std::endl;
  exit(0);
}

}

#endif /* IMP_HELPERS_H */
