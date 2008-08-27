/**
 *  \file MolecularHierarchyDecorator.h   \brief Decorator for helping deal with
 *                                               a hierarchy of molecules.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
#include "bond_decorators.h"

namespace IMP
{

//! A decorator for helping deal with a hierarchy of molecules
/** \ingroup hierarchy
 */
class IMPDLLEXPORT MolecularHierarchyDecorator: public HierarchyDecorator
{
  IMP_DECORATOR(MolecularHierarchyDecorator,
                HierarchyDecorator,
                return P::has_required_attributes(p)
                       && p->has_attribute(type_key_),
                {p->add_attribute(type_key_, UNKNOWN);});
protected:
  typedef HierarchyDecorator P;
  static IntKey type_key_;


public:

  //! The various values for levels of the hierarchy
  /** - UNIVERSE is all the molecules in existance at once.
      - UNIVERSES is a set of universes
      - TRAJECTORY is an ordered set of UNIVERSES
   */
  enum Type {UNKNOWN=-1, ATOM, RESIDUE, NUCLEICACID, FRAGMENT,
             CHAIN, PROTEIN, NUCLEOTIDE, MOLECULE, ASSEMBLY,
             COLLECTION, UNIVERSE, UNIVERSES, TRAJECTORY
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
    case FRAGMENT:
      return "fragment";
    case PROTEIN:
      return "protein";
    case NUCLEOTIDE:
      return "nucleotide";
    case MOLECULE:
      return "molecule";
    case ASSEMBLY:
      return "assembly";
    case COLLECTION:
      return "collection";
    case UNIVERSE:
      return "universe";
    case UNIVERSES:
      return "universes";
    case TRAJECTORY:
      return "trajectory";
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
              InvalidStateException);
    IMP_check(get_type() != UNKNOWN, "Parent must have known type",
              InvalidStateException);
    IMP_check(o.get_type() != UNKNOWN, "Child must have known type",
              InvalidStateException);
    return P::add_child(o);
  }

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  void add_child_at(This o, unsigned int i) {
    IMP_check(get_type() > o.get_type(),
              "Parent type must subsume child type",
              InvalidStateException);
    IMP_check(get_type() != UNKNOWN, "Parent must have known type",
              InvalidStateException);
    IMP_check(o.get_type() != UNKNOWN, "Child must have known type",
              InvalidStateException);
    P::add_child_at(o, i);
  }

  //! Get a child
  MolecularHierarchyDecorator get_child(unsigned int i) const {
    HierarchyDecorator hd= P::get_child(i);
    return cast(hd.get_particle());
  }

  //! Get the parent
  MolecularHierarchyDecorator get_parent() const {
    HierarchyDecorator hd= P::get_parent();
    if (hd == HierarchyDecorator()) {
      return MolecularHierarchyDecorator();
    } else {
      return cast(hd.get_particle());
    }
  }


  static IntKey get_type_key() {
    decorator_initialize_static_data();
    return type_key_;
  }

};

typedef std::vector<MolecularHierarchyDecorator> MolecularHierarchyDecorators;


/**
   Gather all the molecular particles of a certain level
   in the molecular hierarchy
   \ingroup hierarchy
*/
IMPDLLEXPORT Particles
molecular_hierarchy_get_by_type(MolecularHierarchyDecorator mhd,
                                MolecularHierarchyDecorator::Type t);

class ResidueDecorator;

//! Get the residue with the specified index
/** Find the residue with the appropriate index. This is the PDB index,
    not the offset in the chain (if they are different).

    \throw ValueException if mhd's type is not one of CHAIN, PROTEIN, NUCLEOTIDE
    \return MolecularHierarchyDecorator() if that residue is not found.

    \todo We can make this method much more clever in its search since
    most proteins consist of a few contiguous blocks of indices.

    \ingroup hierarchy
 */
IMPDLLEXPORT ResidueDecorator
molecular_hierarchy_get_residue(MolecularHierarchyDecorator mhd,
                                unsigned int index);


//! Create a fragment containing the specified nodes
/** A particle representing the frament is created and initialized.

    The Fragment is inserted as a child of the parent (and the particles are
    removed). The particles become children of the frament.

    \throw ValueException If all the particles do not have the same parent.
 */
IMPDLLEXPORT MolecularHierarchyDecorator
create_fragment(const MolecularHierarchyDecorators &ps);


} // namespace IMP

#endif  /* __IMP_MOLECULAR_HIERARCHY_DECORATOR_H */
