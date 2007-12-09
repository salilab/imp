/**
 *  \file MolecularHierarchyDecorator.h   \brief Decorator for helping deal with
 *                                               a hierarchy of molecules.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MOLECULAR_HIERARCHY_DECORATOR_H
#define __IMP_MOLECULAR_HIERARCHY_DECORATOR_H

#include <vector>
#include <deque>

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include "HierarchyDecorator.h"

namespace IMP
{

//! A decorator for helping deal with a hierarchy of molecules
class IMPDLLEXPORT MolecularHierarchyDecorator: public HierarchyDecorator
{
  IMP_DECORATOR(MolecularHierarchyDecorator,
                HierarchyDecorator,
                return P::has_required_attributes(p)
                       && p->has_attribute(type_key_),
                { P::add_required_attributes(p);
                  p->add_attribute(type_key_, UNKNOWN);
                });
protected:
  typedef HierarchyDecorator P;
  static IntKey type_key_;


public:

  //! The various values for levels of the hierarchy
  /** - UNIVERSE is all the molecules in existance at once.
      - UNIVERSES is a set of universes
      - TRAJECTORY is an ordered set of UNIVERSES
   */
  enum Type {UNKNOWN=-1, ATOM, RESIDUE, NUCLEICACID,
             CHAIN, PROTEIN, NUCLEOTIDE, MOLECULE,
             UNIVERSE, UNIVERSES, TRAJECTORY
            };

  Type get_type() const {
    return Type(get_particle()->get_value(type_key_));
  }

  void set_type(Type t) {
    get_particle()->set_value(type_key_, t);
  }

  //! Return a string representation of the current level of the hierarchy
  std::string get_type_string() const {
    switch (get_type()) {
    case UNKNOWN:
      return "unknown";
    case ATOM:
      return "atom";
    case RESIDUE:
      return "residue";
    case NUCLEICACID:
      return "nucleic acid";
    case CHAIN:
      return "chain";
    case PROTEIN:
      return "protein";
    case NUCLEOTIDE:
      return "nucleotide";
    case MOLECULE:
      return "molecule";
    case UNIVERSE:
      return "universe";
    case UNIVERSES:
      return "universes";
    default:
      IMP_assert(0, "Invalid MolecularHierarchyDecorator type");
      return std::string();
    }
  }

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  unsigned int add_child(This o) {
    IMP_check(get_type() > o.get_type(),
              "Parent type must subsume child type",
              InvalidStateException());
    IMP_check(get_type() != UNKNOWN, "Parent must have known type",
              InvalidStateException());
    IMP_check(o.get_type() != UNKNOWN, "Child must have known type",
              InvalidStateException());
    return P::add_child(o);
  }

  //! Get a child
  MolecularHierarchyDecorator get_child(unsigned int i) const {
    HierarchyDecorator hd= P::get_child(i);
    return cast(hd.get_particle());
  }

  //! Get the parent
  MolecularHierarchyDecorator get_parent() const {
    HierarchyDecorator hd= P::get_parent();
    if (hd != HierarchyDecorator()) {
      return MolecularHierarchyDecorator();
    } else {
      return cast(hd.get_particle());
    }
  }

};



  /**
     Gather all the molecular particles of a certain level 
     in the moleuclar hierarchy
  */
  IMPDLLEXPORT Particles get_particles(MolecularHierarchyDecorator mhd, 
                                       MolecularHierarchyDecorator::Type t);
} // namespace IMP

#endif  /* __IMP_MOLECULAR_HIERARCHY_DECORATOR_H */
