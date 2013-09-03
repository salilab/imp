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
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi, IntRange r) {
    m->add_attribute(get_data().begin, pi, r.first);
    m->add_attribute(get_data().end, pi, r.second);
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi, Int b, Int e) {
    do_setup_particle(m, pi, IntRange(b,e));
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi, Domain o) {
    do_setup_particle(m, pi, o.get_index_range());
  }

 public:

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_data().begin, pi) &&
           m->get_has_attribute(get_data().end, pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  void set_index_range(IntRange ir) {
    IMP_USAGE_CHECK(ir.first < ir.second,
                    "Bad range passed: " << ir.first << "..." << ir.second);
    get_particle()->set_value(get_data().begin, ir.first);
    get_particle()->set_value(get_data().end, ir.second);
  }
  /** Get the range of indexes in the domain `[begin...end)` */
  IntRange get_index_range() const {
    return IntRange(get_model()->get_attribute(get_data().begin,
                                               get_particle_index()),
                    get_model()->get_attribute(get_data().end,
                                               get_particle_index()));
  }

  IMP_DECORATOR_METHODS(Domain, Hierarchy);
  IMP_DECORATOR_SETUP_1(Domain, Domain, other);
  IMP_DECORATOR_SETUP_1(Domain, IntRange, residues);

  /** \deprecated_at{2.1} Use get_index_range() instead.*/
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
    Int get_begin_index() const;
  /** \deprecated_at{2.1} Use get_index_range() instead.*/
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
    Int get_end_index() const;
  /** \deprecated_at{2.1} Use the IntRange setup_particle.*/
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
        static Domain setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                     Int residues_begin, Int residues_end);
  /** \deprecated_at{2.1} Use the IntRange setup_particle.*/
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
  static Domain setup_particle(IMP::kernel::ParticleAdaptor decorator,
                               Int residues_begin, Int residues_end);
};

IMP_DECORATORS(Domain, Domains, Hierarchies);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DOMAIN_H */
