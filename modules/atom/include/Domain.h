/**
 *  \file Domain.h
 *  \brief A decorator for associating a MolecularHiearchy piece with a domain
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_DOMAIN_H
#define IMPATOM_DOMAIN_H

#include "config.h"
#include "Hierarchy.h"
#include "internal/version_info.h"
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator to associate a particle with a part of a protein
/** The decorator stores the indexes of the first and last residues
    in this domain.
 */
class IMPATOMEXPORT Domain: public Hierarchy
{
  struct Data {
    Data(): begin("domain_begin"),
            end("domain_end"){}
    IntKey begin, end;
  };
  static const Data &get_data();
public:
  //! Create a domain covering the range [b, e)
  static Domain create(Particle *p, Int b, Int e) {
    p->add_attribute(get_data().begin, b);
    p->add_attribute(get_data().end, e);
    if (!Hierarchy::is_instance_of(p)) {
      Hierarchy::create(p,
                     Hierarchy::FRAGMENT);
    }
    return Domain(p);
  }

  //! Create a domain by copying from o
  static Domain create(Particle *p, Domain o) {
    p->add_attribute(get_data().begin, o.get_begin_index());
    p->add_attribute(get_data().end, o.get_end_index());
    if (!Hierarchy::is_instance_of(p)) {
      Hierarchy::create(p,
               Hierarchy::FRAGMENT);
    }
    return Domain(p);
  }

  virtual ~Domain();

  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_data().begin)
      && p->has_attribute(get_data().end)
      && Hierarchy::is_instance_of(p);
  }

  //! Get the index of the first residue in the domain
  Int get_begin_index() const {
    return get_particle()->get_value(get_data().begin);
  }

  //! Get the index of the first residue not in the domain
  Int get_end_index() const {
    return get_particle()->get_value(get_data().end);
  }

  IMP_DECORATOR(Domain, Hierarchy)
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_DOMAIN_H */
