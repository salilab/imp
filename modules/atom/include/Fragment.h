/**
 *  \file IMP/atom/Fragment.h
 *  \brief A decorator for associating a Hierachy piece
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_FRAGMENT_H
#define IMPATOM_FRAGMENT_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator to associate a particle with a part of a protein/DNA/RNA
/** The decorator stores an optional list of resdiue indexes.
 */
class IMPATOMEXPORT Fragment : public Hierarchy {
  static IntsKey get_begins_key();
  static IntsKey get_ends_key();
  static void set_residue_indexes(Particle *p, Ints ris);
  static void set_residue_indexes(Particle *p, const IntPairs &ris);
  static IntKey get_marker_key();

 public:
  static Fragment setup_particle(Particle *p, const Ints &ris = Ints()) {
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    set_residue_indexes(p, ris);
    p->add_attribute(get_marker_key(), 1);
    return Fragment(p);
  }

  //! Create a domain by copying from o
  static Fragment setup_particle(Particle *p, Fragment o) {
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    IntPairs ris = o.get_residue_index_ranges();
    set_residue_indexes(p, ris);
    p->add_attribute(get_marker_key(), 1);
    return Fragment(p);
  }

  virtual ~Fragment();

  static bool particle_is_instance(Particle *p) {
    return particle_is_instance(p->get_model(), p->get_index());
  }

  static bool particle_is_instance(Model *m, ParticleIndex pi) {
    return Hierarchy::particle_is_instance(m, pi) &&
           m->get_has_attribute(get_marker_key(), pi);
  }

  //! Add the residues whose indexes are listed in the passed vector
  void set_residue_indexes(Ints o) { set_residue_indexes(get_particle(), o); }

  Ints get_residue_indexes() const;

  IntPairs get_residue_index_ranges() const;

  //! Return true if this fragment contains a given residue
  /** This could be made more efficient. */
  bool get_contains_residue(int rindex) const;

  IMP_DECORATOR(Fragment, Hierarchy);
};

IMP_DECORATORS(Fragment, Fragments, Hierarchies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_FRAGMENT_H */
