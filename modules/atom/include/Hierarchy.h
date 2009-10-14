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
#include <IMP/core/XYZR.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>


#define IMP_GET_AS_DECL(UCName, lcname, CAPSNAME)       \
  UCName get_as_##lcname() const;

// figure out how to inline
#define IMP_GET_AS_DEF(UCName, lcname, CAPSNAME)        \
  UCName Hierarchy::get_as_##lcname() const {           \
    if (UCName::particle_is_instance(get_particle())) { \
      return UCName(get_particle());                    \
    } else {                                            \
      return UCName();                                  \
    }                                                   \
  }


// DOMAIN is defined to be 1 by a fedora math header
#define IMP_FOREACH_HIERARCHY_TYPE(macro)      \
  macro(Atom, atom, ATOM_TYPE)                 \
  macro(Residue, residue, RESIDUE_TYPE)        \
  macro(Chain, chain, CHAIN_TYPE)              \
  macro(Domain, domain, DOMAIN_TYPE)           \
  macro(Fragment, fragment, FRAGMENT_TYPE)     \
  macro(core::XYZ, xyz, XYZ_TYPE)              \
  macro(core::XYZR, xyzr, XYZR_TYPE)           \
  macro(Mass, mass, MASS_TYPE)

#define IMP_CAPS_NAME(UCName, lcname, CAPSNAME) \
  CAPSNAME,


IMPATOM_BEGIN_NAMESPACE
class Atom;
class Residue;
class Domain;
class Fragment;
class Chain;
class Mass;

class Hierarchy;
/** A collecton of Hierarchies. */
typedef Decorators<Hierarchy,
                   IMP::DecoratorsWithImplicitTraits< Hierarchy,
                              IMP::core::GenericHierarchies> > Hierarchies;


//! A decorator for helping deal with a hierarchy of molecules
/** A hierarchy can have any tree structure as long as:
    - the type of the parent makes sense for the child: eg a Residue
    cannot be the parent of a Chain.
    - the leaves always have coordinates and mass
    - all particles in hierarchy are from the same model
    - any Atom that is part of a protein, DNA or RNA has a Residue
    for as parent

    The get_is_valid() method checks some of these.

    A number of decorator types are associated with the Hierarchy.
    We provide a get_as_x() function for each such decorator which
    returns either X() (a null type) if the node is not a particle
    of type x, or an X decorator wrapping the current particle if
    it is.

    \ingroup hierarchy
    \ingroup decorators
    \see write_pdb
    \see read_pdb
 */
class IMPATOMEXPORT Hierarchy:
  public ::IMP::core::Hierarchy
{
  typedef ::IMP::core::Hierarchy P;
public:

  // swig gets unhappy if it is private
  IMP_NO_DOXYGEN(typedef Hierarchy This;)

  //! The traits must match
  Hierarchy(IMP::core::Hierarchy h): P(h) {
    IMP_USAGE_CHECK(h.get_traits() == get_traits(),
              "Cannot construct a IMP.atom.Hierarchy from a general "
              " IMP.core.Hierarchy",
              ValueException);
  }

  //! Create a Hierarchy on the Particle
  /** A traits class can be specified if the default one is not desired.
   */
  Hierarchy(Particle *p):
    P(p,get_traits()){
    IMP_INTERNAL_CHECK(particle_is_instance(p),
                       "Missing required attributes for "
               << "Hierarchy" << *p);
  }

  //! null constructor
  Hierarchy() {}

  //! cast a particle which has the needed attributes
  static Hierarchy decorate_particle(Particle *p) {
    IMP::core::Hierarchy::decorate_particle(p, get_traits());
    return Hierarchy(p);
  }

  /** Create a Hierarchy of level t by adding the needed
      attributes. */
  static Hierarchy setup_particle(Particle *p) {
    IMP::core::Hierarchy::setup_particle(p, get_traits());
    return Hierarchy(p);
  }

  /** Check if the particle has the needed attributes for a
   cast to succeed */
  static bool particle_is_instance(Particle *p){
    return P::particle_is_instance(p, get_traits());
  }


  /** Write information about this decorator to out.*/
  void show(std::ostream &out=std::cout) const;

  //! Return true if the hierarchy is valid.
  /** Print information about the hierarchy if print_info is
      true and things are invalid.
      \note Returning true only means that no problems were
      found, it can't check everything.*/
  bool get_is_valid(bool print_info) const;

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  unsigned int add_child(Hierarchy o) {
    IMP_USAGE_CHECK(o.get_particle()->get_model()
                    == get_particle()->get_model(),
                    "All particles in hierarchy must have same Model",
                    ValueException);
    unsigned int ret= P::add_child(o);
    return ret;
  }

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  void add_child_at(Hierarchy o, unsigned int i) {
    IMP_USAGE_CHECK(o.get_particle()->get_model()
                    == get_particle()->get_model(),
                    "All particles in hierarchy must have same Model",
                    ValueException);
    P::add_child_at(o, i);
  }

  /** Get the ith child */
  Hierarchy get_child(unsigned int i) const {
    IMP::core::Hierarchy hd= P::get_child(i);
    return decorate_particle(hd.get_particle());
  }

  Hierarchies get_children() const {
    return Hierarchies(IMP::core::Hierarchy::get_children());
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

  /** \name Methods to get associated decorators

      We provide a number of helper methods to get associated
      decorators for the current node in the hierarchy. As an
      example, if the particle decorated by this decorator is
      a Residue particle, then get_as_residue() return
      Residue(get_particle()), if not it returns Residue().
      @{
   */
  IMP_FOREACH_HIERARCHY_TYPE(IMP_GET_AS_DECL)
  /** @} */

  //! Get the molecular hierarchy HierararchyTraits.
  static const IMP::core::HierarchyTraits& get_traits();

};

IMP_OUTPUT_OPERATOR(Hierarchy);

enum GetByType {
  IMP_FOREACH_HIERARCHY_TYPE(IMP_CAPS_NAME)
};

/**
   Gather all the molecular particles of a certain level
   in the molecular hierarchy
   \ingroup hierarchy
   \relatesalso Hierarchy
*/
IMPATOMEXPORT Hierarchies
get_by_type(Hierarchy mhd,
            GetByType t);


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
