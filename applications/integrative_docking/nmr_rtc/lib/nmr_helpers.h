/**
 *  \file nmr_helpers.h   \brief A set of helper functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_NMR_HELPERS_H
#define IMP_NMR_HELPERS_H

#include <IMP/Particle.h>
#include <IMP/algebra/Transformation3D.h>

#include <vector>
#include <string>

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms);

void read_pdb_atoms(const std::string file_name, IMP::Particles& particles);

void read_pdb_ca_atoms(const std::string file_name, IMP::Particles& particles);

void transform(IMP::Particles& particles, IMP::algebra::Transformation3D& t);

void get_atom_2_residue_map(const IMP::Particles& atom_particles,
                            const IMP::Particles& residue_particles,
                            std::vector<int>& atom_2_residue_map);

void get_residue_solvent_accessibility(const IMP::Particles& residue_particles,
                                    IMP::Floats& residue_solvent_accessibility);

void get_residue_solvent_accessibility(const IMP::Particles& atom_particles,
                                       const IMP::Particles& residue_particles,
                                    const std::vector<int>& atom_2_residue_map,
                             std::vector<float>& residue_solvent_accessibility);

#endif /* IMP_NMR_HELPERS_H */
