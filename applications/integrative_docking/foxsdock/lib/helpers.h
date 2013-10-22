/**
 *  \file helpers.h   \brief A set of helper functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_HELPERS_H
#define IMP_HELPERS_H

#include <IMP/kernel/Particle.h>
#include <IMP/algebra/Transformation3D.h>

#include <vector>
#include <string>

void read_trans_file(const std::string file_name,
                     std::vector<IMP::algebra::Transformation3D>& transforms);

void read_pdb_atoms(const std::string file_name, IMP::kernel::Particles& particles);

void read_pdb_ca_atoms(const std::string file_name, IMP::kernel::Particles& particles);

void transform(IMP::kernel::Particles& particles, IMP::algebra::Transformation3D& t);

#endif /* IMP_HELPERS_H */
