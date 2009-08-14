/**
 *  \file atom/Hierarchy.h
 *  \brief Decorator for helping deal with a hierarchy of molecules.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_HIERARCHY_H
#define IMPATOM_HIERARCHY_H

#include "config.h"
#include <IMP/core/utility.h>
#include <IMP/core/Hierarchy.h>
#include "bond_decorators.h"
#include <IMP/core/rigid_bodies.h>
#include "macros.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE


class Hierarchy;
/** A collecton of Hierarchies. */
typedef Decorators<Hierarchy,
                   IMP::DecoratorsWithImplicitTraits< Hierarchy,
                              IMP::core::GenericHierarchies> > Hierarchies;


//! A decorator for helping deal with a hierarchy of molecules
/** A hierarchy can have any tree structure as long as child nodes
    are valid subtypes of their parents (for example, an ATOM can be
    a child of a RESIDUE or CHAIN or PROTEIN, but a RESIDUE can't
    be a child of an ATOM).

    Layers of the hierarchy are associated with other decorators as
    follows:
    - ATOM: Atom
    - AMINOACID, NUCLEICACID, LIGAND: Residue
    - FRAGMENT: Fragment
    - CHAIN: Chain
    - PROTEIN: no decorator, but the protein name should be stored as
    the Particle::get_name().

    A hierarchy is considered "valid" if:
    - all the leaves have coordinates (or none of the nodes have
    coordinates) and all leaves have mass
    - for any protein in the Hierarchy, each residue in the protein
    either has a Residue particle, or is part of a leaf Domain or
    Fragment
    - all bounding spheres in the hierachy bound their subtrees
    - Any atom that is part of a protein or DNA or RNA has a residue
    as a parent.
    - All Hierarchy nodes which have associated decorators are set
    up for the corresponding decorator.

    The get_is_valid() method checks some of these.

    \ingroup hierarchy
    \ingroup decorators
    \see Atom
    \see Chain
    \see Domain
    \see Fragment
    \see Residue
    \see write_pdb
    \see read_pdb
 */
class IMPATOMEXPORT Hierarchy:
  public IMP::core::Hierarchy
{
  typedef IMP::core::Hierarchy P;
public:

  //! The various values for levels of the hierarchy
  /**
      - ATOM (0) an atom
      - RESIDUE (1) a residue
      - NUCLEICACID (2) a nucleic acid
      - FRAGMENT (3) an arbitrary fragment
      - DOMAIN (4) a chain of a protein
      - CHAIN (5) a chain of a protein
      - PROTEIN (6) a protein
      - NUCLEOTIDE (7) a nucleotide
      - MOLECULE (8) an arbitrary molecule
      - ASSEMBLY (9) an assembly
      - COLLECTION (10) a group of assemblies
      - UNIVERSE is all the molecules in existance at once.
      - UNIVERSES is a set of universes
      - TRAJECTORY is an ordered set of UNIVERSES

      \note These values may change.
   */
  enum Type {UNKNOWN=-1, ATOM, AMINOACID, NUCLEICACID, LIGAND,
             FRAGMENT, CHAIN, PROTEIN, NUCLEOTIDE, MOLECULE, ASSEMBLY,
             COLLECTION, UNIVERSE, UNIVERSES, TRAJECTORY
            };

  // swig gets unhappy if it is private
  IMP_NO_DOXYGEN(typedef Hierarchy This;)

  //! The traits must match
  Hierarchy(IMP::core::Hierarchy h): P(h) {
    IMP_check(h.get_traits() == get_traits(),
              "Cannot construct a IMP.atom.Hierarchy from a general "
              " IMP.core.Hierarchy",
              ValueException);
  }

  //! Create a Hierarchy on the Particle
  /** A traits class can be specified if the default one is not desired.
   */
  Hierarchy(Particle *p):
    P(p,get_traits()){
    IMP_assert(particle_is_instance(p), "Missing required attributes for "
               << "Hierarchy" << *p);
  }

  //! null constructor
  Hierarchy() {}

  //! cast a particle which has the needed attributes
  static Hierarchy decorate_particle(Particle *p) {
    IMP::core::Hierarchy::decorate_particle(p, get_traits());
    IMP_check(p->has_attribute(get_type_key()), "Particle is missing attribute "
              << get_type_key(),
              InvalidStateException);
    return Hierarchy(p);
  }

  /** Create a Hierarchy of level t by adding the needed
      attributes. */
  static Hierarchy setup_particle(Particle *p,
                                            Type t= UNKNOWN) {
    IMP::core::Hierarchy::setup_particle(p, get_traits());
    p->add_attribute(get_type_key(), t);
    return Hierarchy(p);
  }

  /** Check if the particle has the needed attributes for a
   cast to succeed */
  static bool particle_is_instance(Particle *p){
    return P::particle_is_instance(p, get_traits())
      && p->has_attribute(get_type_key());
  }


  /** Write information about this decorator to out.*/
  void show(std::ostream &out=std::cout) const;



  Type get_type() const {
    return Type(get_particle()->get_value(get_type_key()));
  }
  void set_type(Type t) {
    get_particle()->set_value(get_type_key(), t);
  }

  //! Return true if the hierarchy is valid.
  /** Print information about the hierarchy if print_info is
      true and things are invalid.
      \note Returning true only means that no problems were
      found, it can't check everything.*/
  bool get_is_valid(bool print_info) const;

  //! Return a string representation of the current level of the hierarchy
  std::string get_type_string() const {
    switch (get_type()) {
    case UNKNOWN:
      return "unknown";
    case ATOM:
      return "atom";
    case AMINOACID:
      return "amino acid";
    case NUCLEICACID:
      return "nucleic acid";
    case LIGAND:
      return "ligand";
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
      IMP_assert(0, "Invalid Hierarchy type");
      return std::string();
    }
  }

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  unsigned int add_child(Hierarchy o) {
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
  void add_child_at(Hierarchy o, unsigned int i) {
    IMP_check(get_type() > o.get_type(),
              "Parent type must subsume child type",
              InvalidStateException);
    IMP_check(get_type() != UNKNOWN, "Parent must have known type",
              InvalidStateException);
    IMP_check(o.get_type() != UNKNOWN, "Child must have known type",
              InvalidStateException);
    P::add_child_at(o, i);
  }

  /** Get the ith child */
  Hierarchy get_child(unsigned int i) const {
    IMP::core::Hierarchy hd= P::get_child(i);
    return decorate_particle(hd.get_particle());
  }

  Hierarchies get_children() const {
    Hierarchies ps;
    for (unsigned int i=0; i< get_number_of_children(); ++i) {
      ps.push_back( get_child(i) );
    }
    return ps;
  }


  /** Get the parent particle. */
  Hierarchy get_parent() const {
    IMP::core::Hierarchy hd= P::get_parent();
    if (hd == Hierarchy()) {
      return Hierarchy();
    } else {
      return decorate_particle(hd.get_particle());
    }
  }

  /** Gets the key used to store the type. */
  static IntKey get_type_key() ;
  //! Get the molecular hierarchy HierararchyTraits.
  static const IMP::core::HierarchyTraits& get_traits();

};

IMP_OUTPUT_OPERATOR(Hierarchy);


/**
   Gather all the molecular particles of a certain level
   in the molecular hierarchy
   \ingroup hierarchy
   \relatesalso Hierarchy
*/
IMPATOMEXPORT Hierarchies
get_by_type(Hierarchy mhd,
            Hierarchy::Type t);


class Residue;

//! Get the residue with the specified index
/** Find the leaf containing the residue with the appropriate index.
    This is the PDB index, not the offset in the chain (if they are different).

    The function returns a Hierarchy, rather than a Residue since the
    residue may not be explicitly represented and may just be part of some
    fragment.

    \throw ValueException if mhd's type is not one of CHAIN, PROTEIN, NUCLEOTIDE
    \return Hierarchy() if that residue is not found.

    \ingroup hierarchy
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
get_residue(Hierarchy mhd, unsigned int index);


//! Create a fragment containing the specified nodes
/** A particle representing the frament is created and initialized.

    The Fragment is inserted as a child of the parent (and the particles are
    removed). The particles become children of the frament.

    \throw ValueException If all the particles do not have the same parent.
    \ingroup hierarchy
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
create_fragment(const Hierarchies &ps);

//! Get the bonds internal to this tree
/**     \relatesalso Hierarchy
        \see Bond
 */
IMPATOMEXPORT Bonds
get_internal_bonds(Hierarchy mhd);


//! Return the root of the hierarchy
/** \relatesalso Hierarchy */
inline Hierarchy get_root(Hierarchy h) {
  while (h.has_parent()) {
    h= h.get_parent();
  }
  return h;
}

inline Hierarchies get_leaves(Hierarchy h) {
  return Hierarchies(IMP::core::get_leaves(h));
}

//! Print out a molecular hierarchy
inline void show(Hierarchy h, std::ostream &out=std::cout) {
   IMP::core::show<Hierarchy>(h, out);
}

IMPATOM_END_NAMESPACE



#endif  /* IMPATOM_HIERARCHY_H */
