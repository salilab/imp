/**
 *  \file IMP/atom/Domain.h
 *  \brief A decorator for associating an atom::Hierarchy piece with a domain
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_DOMAIN_H
#define IMPATOM_DOMAIN_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator to associate a particle with a part of a protein
/** The decorator stores the indexes of the first and last residues
    in this domain.
 */
class IMPATOMEXPORT Domain : public Hierarchy {
  struct Data {
    Data() : begin("domain_begin"), end("domain_end") {}
    IntKey begin, end;
  };
  static const Data &get_data();

 public:
#ifndef IMP_DOXYGEN
  //! Create a domain covering the range [b, e)
  static Domain setup_particle(Particle *p, Int b, Int e) {
    p->add_attribute(get_data().begin, b);
    p->add_attribute(get_data().end, e);
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    return Domain(p);
  }
#endif
  //! Create a domain covering the range [b, e)
  static Domain setup_particle(Particle *p, IntRange r) {
    p->add_attribute(get_data().begin, r.first);
    p->add_attribute(get_data().end, r.second);
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    return Domain(p);
  }
  //! Create a domain by copying from o
  static Domain setup_particle(Particle *p, Domain o) {
    p->add_attribute(get_data().begin, o.get_begin_index());
    p->add_attribute(get_data().end, o.get_end_index());
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    return Domain(p);
  }

  virtual ~Domain();

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_data().begin) &&
           p->has_attribute(get_data().end) &&
           Hierarchy::particle_is_instance(p);
  }

  void set_index_range(IntRange ir) {
    IMP_USAGE_CHECK(ir.first < ir.second,
                    "Bad range passed: " << ir.first << "..." << ir.second);
    get_particle()->set_value(get_data().begin, ir.first);
    get_particle()->set_value(get_data().end, ir.second);
  }

  IntRange get_index_range() const {
    return IntRange(get_begin_index(), get_end_index());
  }

  //! Get the index of the first residue in the domain
  Int get_begin_index() const {
    return get_particle()->get_value(get_data().begin);
  }

  //! Get the index of the first residue not in the domain
  Int get_end_index() const {
    return get_particle()->get_value(get_data().end);
  }

  IMP_DECORATOR(Domain, Hierarchy);
};

IMP_DECORATORS(Domain, Domains, Hierarchies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DOMAIN_H */
