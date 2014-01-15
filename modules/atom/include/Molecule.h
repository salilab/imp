/**
 *  \file IMP/atom/Molecule.h     \brief A decorator for Molecules.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MOLECULE_H
#define IMPATOM_MOLECULE_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"

#include <IMP/base_types.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a molecule.
/** */
class IMPATOMEXPORT Molecule : public Hierarchy {
  static IntKey key();
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                Molecule = Molecule()) {
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    m->add_attribute(key(), pi, 1);
  }

 public:
  IMP_DECORATOR_METHODS(Molecule, Hierarchy);
  /** Mark the particle as denoting a molecule. */
  IMP_DECORATOR_SETUP_0(Molecule);
  IMP_DECORATOR_SETUP_1(Molecule, Molecule, other);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(key(), pi);
  }
};

IMP_DECORATORS(Molecule, Molecules, Hierarchies);

/** Walk up the hierarchy to determine the molecule name. */
IMPATOMEXPORT std::string get_molecule_name(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MOLECULE_H */
