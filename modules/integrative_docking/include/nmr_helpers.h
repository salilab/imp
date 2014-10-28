/**
 *  \file IMP/integrative_docking/nmr_helpers.h
 *  \brief A set of helper functions
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_NMR_HELPERS_H
#define IMPINTEGRATIVE_DOCKING_NMR_HELPERS_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/kernel/Particle.h>
#include <IMP/algebra/Transformation3D.h>

#include <vector>
#include <string>

IMPINTEGRATIVEDOCKING_BEGIN_NAMESPACE

IMPINTEGRATIVEDOCKINGEXPORT
void get_residue_solvent_accessibility(
    const IMP::kernel::Particles& atom_particles,
    const IMP::kernel::Particles& residue_particles,
    const std::vector<int>& atom_2_residue_map,
    std::vector<float>& residue_solvent_accessibility);

IMPINTEGRATIVEDOCKING_END_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_NMR_HELPERS_H */
