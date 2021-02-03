/**
 *  \file IMP/atom/Fragment.h
 *  \brief A decorator for associating a Hierarchy piece
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_FRAGMENT_H
#define IMPATOM_FRAGMENT_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator to associate a particle with a part of a protein/DNA/RNA
/** The decorator stores an optional list of residue indexes.
 */
class IMPATOMEXPORT Fragment : public Hierarchy {
  static IntsKey get_begins_key();
  static IntsKey get_ends_key();
  static void set_residue_indexes(Model *m, ParticleIndex pi,
                                  Ints ris);
  static void set_residue_indexes(Model *m, ParticleIndex pi,
                                  const IntPairs &ris);
  static IntKey get_marker_key();

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const Ints &ris = Ints()) {
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    m->add_attribute(get_marker_key(), pi, 1);
    set_residue_indexes(m, pi, ris);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Fragment o) {
    do_setup_particle(m, pi, o.get_residue_indexes());
  }

  IntPairs get_residue_index_ranges() const;

 public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return Hierarchy::get_is_setup(m, pi) &&
           m->get_has_attribute(get_marker_key(), pi);
  }

  //! Add the residues whose indexes are listed in the passed vector
  void set_residue_indexes(Ints o) {
    set_residue_indexes(get_model(), get_particle_index(), o);
  }

  Ints get_residue_indexes() const;

  //! Return true if this fragment contains a given residue
  /** This could be made more efficient. */
  bool get_contains_residue(int rindex) const;

  IMP_DECORATOR_METHODS(Fragment, Hierarchy);
  IMP_DECORATOR_SETUP_0(Fragment);
  IMP_DECORATOR_SETUP_1(Fragment, Fragment, other);
  /** Setup a fragment with the passed residue indexes. */
  IMP_DECORATOR_SETUP_1(Fragment, Ints, residue_indexes);
};

IMP_DECORATORS(Fragment, Fragments, Hierarchies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FRAGMENT_H */
