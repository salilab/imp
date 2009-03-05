/**
 *  \file atom/MolecularHierarchyDecorator.h
 *  \brief Decorator for helping deal with a hierarchy of molecules.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_MOLECULAR_HIERARCHY_DECORATOR_H
#define IMPATOM_MOLECULAR_HIERARCHY_DECORATOR_H

#include "config.h"
#include <IMP/core/utility.h>
#include <IMP/core/HierarchyDecorator.h>
#include "bond_decorators.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for helping deal with a hierarchy of molecules
/** \ingroup hierarchy
    \ingroup decorators
    \see ResidueDecorator
    \see AtomDecorator
    \see write_pdb
    \see read_pdb
 */
class IMPATOMEXPORT MolecularHierarchyDecorator:
  public IMP::core::HierarchyDecorator
{
  typedef IMP::core::HierarchyDecorator P;
public:

  //! The various values for levels of the hierarchy
  /**
      - ATOM (0) an atom
      - RESIDUE (1) a residue
      - NUCLEICACID (2) a nucleic acid
      - FRAGMENT (3) an arbitrary fragment
      - CHAIN (4) a chain of a protein
      - PROTEIN (5) a protein
      - NUCLEOTIDE (6) a nucleotide
      - MOLECULE (7) an arbitrary molecule
      - ASSEMBLY (8) an assembly
      - COLLECTION (9) a group of assemblies
      - UNIVERSE is all the molecules in existance at once.
      - UNIVERSES is a set of universes
      - TRAJECTORY is an ordered set of UNIVERSES
   */
  enum Type {UNKNOWN=-1, ATOM, RESIDUE, NUCLEICACID, FRAGMENT,
             CHAIN, PROTEIN, NUCLEOTIDE, MOLECULE, ASSEMBLY,
             COLLECTION, UNIVERSE, UNIVERSES, TRAJECTORY
            };

  // swig gets unhappy if it is private
  typedef MolecularHierarchyDecorator This;


  //! Create a HiearchyDecorator on the Particle
  /** A traits class can be specified if the default one is not desired.
   */
  MolecularHierarchyDecorator(Particle *p):
    P(p,get_traits()){
    IMP_assert(is_instance_of(p), "Missing required attributes for "
               << "MolecularHierarchyDecorator" << *p);
  }

  //! null constructor
  MolecularHierarchyDecorator() {}

  //! cast a particle which has the needed attributes
  static MolecularHierarchyDecorator cast(Particle *p) {
    IMP::core::HierarchyDecorator::cast(p, get_traits());
    IMP_check(p->has_attribute(get_type_key()), "Particle is missing attribute "
              << get_type_key(),
              InvalidStateException);
    return MolecularHierarchyDecorator(p);
  }

  /** Create a MolecularHierarchyDecorator of level t by adding the needed
      attributes. */
  static MolecularHierarchyDecorator create(Particle *p,
                                            Type t= UNKNOWN) {
    HierarchyDecorator::create(p, get_traits());
    p->add_attribute(get_type_key(), t);
    return MolecularHierarchyDecorator(p);
  }

  /** Check if the particle has the needed attributes for a
   cast to succeed */
  static bool is_instance_of(Particle *p){
    return P::is_instance_of(p, get_traits())
      && p->has_attribute(get_type_key());
  }


  /** Write information about this decorator to out. Each line should
   prefixed by prefix*/
  void show(std::ostream &out=std::cout,
            std::string prefix=std::string()) const;



  /** */
  Type get_type() const {
    return Type(get_particle()->get_value(get_type_key()));
  }
  /** */
  void set_type(Type t) {
    get_particle()->set_value(get_type_key(), t);
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

  /** Get the ith child
      \throw IndexException if there is no such child. */
  MolecularHierarchyDecorator get_child(unsigned int i) const {
    HierarchyDecorator hd= P::get_child(i);
    return cast(hd.get_particle());
  }

  /** Get the parent particle. */
  MolecularHierarchyDecorator get_parent() const {
    HierarchyDecorator hd= P::get_parent();
    if (hd == HierarchyDecorator()) {
      return MolecularHierarchyDecorator();
    } else {
      return cast(hd.get_particle());
    }
  }

  /** Gets the key used to store the type. */
  static IntKey get_type_key() ;
  //! Get the molecular hierarchy HierararchyTraits.
  static const IMP::core::HierarchyTraits& get_traits();

};

/** A colleciton of MolecularHierarchyDecorators. */
typedef std::vector<MolecularHierarchyDecorator> MolecularHierarchyDecorators;


/**
   Gather all the molecular particles of a certain level
   in the molecular hierarchy
   \ingroup hierarchy
   \relates MolecularHierarchyDecorator
*/
IMPATOMEXPORT Particles
get_by_type(MolecularHierarchyDecorator mhd,
                                MolecularHierarchyDecorator::Type t);

class ResidueDecorator;

//! Get the residue with the specified index
/** Find the residue with the appropriate index. This is the PDB index,
    not the offset in the chain (if they are different).

    \throw ValueException if mhd's type is not one of CHAIN, PROTEIN, NUCLEOTIDE
    \return MolecularHierarchyDecorator() if that residue is not found.

    \ingroup hierarchy
    \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT ResidueDecorator
get_residue(MolecularHierarchyDecorator mhd,
            unsigned int index);


//! Create a fragment containing the specified nodes
/** A particle representing the frament is created and initialized.

    The Fragment is inserted as a child of the parent (and the particles are
    removed). The particles become children of the frament.

    \throw ValueException If all the particles do not have the same parent.
    \ingroup hierarchy
    \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT MolecularHierarchyDecorator
create_fragment(const MolecularHierarchyDecorators &ps);

//! Get the bonds internal to this tree
/**     \relates MolecularHierarchyDecorator
 */
IMPATOMEXPORT atom::BondDecorators
get_internal_bonds(MolecularHierarchyDecorator mhd);


//! Clone the MolecularHiearchy
/** This method copies the bonds, AtomDecorator data, ResidueDecorator
    data, DomainDecorator data and NameDecorator data to the new copies
    in addition to the MolecularHierarchyDecorator relationships.

    \note This method has not been tested at all
    \relates MolecularHierarchyDecorator
*/
IMPATOMEXPORT
MolecularHierarchyDecorator clone(MolecularHierarchyDecorator d);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_MOLECULAR_HIERARCHY_DECORATOR_H */
