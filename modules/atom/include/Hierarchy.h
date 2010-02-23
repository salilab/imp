/**
 *  \file atom/Hierarchy.h
 *  \brief Decorator for helping deal with a hierarchy of molecules.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_HIERARCHY_H
#define IMPATOM_HIERARCHY_H

#include "config.h"
#include <IMP/core/utility.h>
#include <IMP/core/Hierarchy.h>
#include "bond_decorators.h"
#include "macros.h"
#include <IMP/core/XYZR.h>
#include <IMP/core/rigid_bodies.h>

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

#ifndef SWIG
#ifdef IMP_DOXYGEN
/**  A type to store a collection of Hierarchy decorators.

     It looks like a \c std::vector<Hierarchy> in C++ and a \c list in Python.
     See \ref tempornot "when to use Temp" for details on using this type.
*/
class Hierarchies: public HierarchiesTemp {};

/**  A type to store a collection of Hierarchy decorators.

     It looks like a \c std::vector<Hierarchy> in C++ and a \c list in Python.
     See \ref tempornot "when to use Temp" for details on using this type.
*/
class HierarchiesTemp: public IMP::core::GenericHierarchiesTemp {};

#else
typedef IMP::DecoratorsWithImplicitTraits< Hierarchy,
                              IMP::core::GenericHierarchies> Hierarchies;
typedef IMP::DecoratorsWithImplicitTraits< Hierarchy,
                         IMP::core::GenericHierarchiesTemp> HierarchiesTemp;
#endif

#else
class Hierarchies: public IMP::core::GenericHierarchies {
public:
  Hierarchies(Hierarchy h);
  Hierarchies();
  Hierarchies(const Particles &ps);
  const Particles &get_particles() const;
  void push_back(Hierarchy d);
  void push_back(Particle *p);
  Hierarchy back() const;
  Hierarchy front() const;
};
class HierarchiesTemp: public IMP::core::GenericHierarchiesTemp {
public:
  HierarchiesTemp(Hierarchy h);
  HierarchiesTemp(Hierarchies h);
  HierarchiesTemp();
  HierarchiesTemp(const Particles &ps);
  const Particles &get_particles() const;
  void push_back(Hierarchy d);
  void push_back(Particle *p);
  Hierarchy back() const;
  Hierarchy front() const;
};
#endif

//! The standard decorator for manipulating molecular structures.
/** \imp represents molecular structures using the Hierachy decorator.
    Using it, molecules and collections of molecules are represented
    using a hierarchy (or tree) where the level of detail increases
    as you move further from the root. The leaves of the tree have
    coordinates, as can internal nodes if the molecules are represented
    at multiple resolutions.

    \section tree_basics Tree Basics
    In a tree you have a set of nodes, represented by Hierarchy particles.
    Each node can have a node can have at most one parent. The node with no
    parent is known as the root of the tree.

    Here is a simple example with a protein with three residues. Two of the
    residues have atoms, where as the third is coarse grained.
    \dotgraph{\dot
    digraph example {
    node [shape=record\, fontname= Helvetica\, fontsize=10]
      a [label="Protein A (the root)"\, URL="\ref B"];
      b [label="Residue 0"];
      c [label="Residue 1"];
      cp [label="Residue 2"];
      d0 [label="CA"];
      e0 [label="CA"];
      d1 [label="C"];
      e1 [label="C"];
      d2 [label="N"];
      e2 [label="N"];
      a -> b [arrowhead="open"];
      a -> c [arrowhead="open"]
      a -> cp [arrowhead="open"];
      b -> d0 [arrowhead="open"];
      c -> e0 [arrowhead="open"];
      b -> d1 [arrowhead="open"];
      c -> e1 [arrowhead="open"];
      b -> d2 [arrowhead="open"];
      c -> e2 [arrowhead="open"];
    }
    \enddot
    }


    The hierarchy can be used to add extra, not-necessarily
    biological, structure such as domains or other convenient
    divisions of the molecule. For example, the protein node could
    have an intermediate layer between it and the first two residues:
    \dotgraph{\dot
    digraph example {
    node [shape=record\, fontname= Helvetica\, fontsize=10]
      a [label="Protein A (the root)"\, URL="\ref B"];
      aa [label="Fragment 0"];
      b [label="Residue 0"];
      c [label="Residue 1"];
      cp [label="Residue 2"];
      d0 [label="CA"];
      e0 [label="CA"];
      d1 [label="C"];
      e1 [label="C"];
      d2 [label="N"];
      e2 [label="N"];
      a -> aa [arrowhead="open"];
      aa -> b [arrowhead="open"];
      aa -> c [arrowhead="open"]
      a -> cp [arrowhead="open"];
      b -> d0 [arrowhead="open"];
      c -> e0 [arrowhead="open"];
      b -> d1 [arrowhead="open"];
      c -> e1 [arrowhead="open"];
      b -> d2 [arrowhead="open"];
      c -> e2 [arrowhead="open"];
    }
    \enddot}


    A hierarchy can have any tree structure as long as:
    - the type of the parent makes sense for the child: eg a Residue
    cannot be the parent of a Chain.
    - the leaves always have coordinates and mass
    - all particles in hierarchy are from the same model
    - all Atoms has a Residue for as parent
    - any Atom with a non-heterogen atom type is part of a protein,
    DNA or RNA molecule.

    The get_is_valid() method checks some of these.

    A number of decorator types are associated with the Hierarchy
    to store the information associated with that node in the
    hierarchy. Examples include Residue, Atom, XYZ, Chain, XYZR,
    Mass, Domain etc.
    We provide a get_as_x() function for each such decorator which
    returns either X() (a null type) if the node is not a particle
    of type x, or an X decorator wrapping the current particle if
    it is.

    \ingroup hierarchy
    \ingroup decorators
 */
class IMPATOMEXPORT Hierarchy: public ::IMP::core::Hierarchy
{
  typedef ::IMP::core::Hierarchy P;
public:

  // swig gets unhappy if it is private
  IMP_NO_DOXYGEN(typedef Hierarchy This;)

  //! The traits must match
  Hierarchy(IMP::core::Hierarchy h): P(h) {
    IMP_USAGE_CHECK(h.get_traits() == get_traits(),
              "Cannot construct a IMP.atom.Hierarchy from a general "
              " IMP.core.Hierarchy");
  }

  explicit Hierarchy(Particle *p):
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
                    "All particles in hierarchy must have same Model");
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
                    "All particles in hierarchy must have same Model");
    P::add_child_at(o, i);
  }

  /** Get the ith child */
  Hierarchy get_child(unsigned int i) const {
    IMP::core::Hierarchy hd= P::get_child(i);
    return decorate_particle(hd.get_particle());
  }
  HierarchiesTemp get_children() const {
    return HierarchiesTemp(IMP::core::Hierarchy::get_children());
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
IMPATOMEXPORT HierarchiesTemp
get_by_type(Hierarchy mhd, GetByType t);


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
create_fragment(const HierarchiesTemp &ps);

//! Get the bonds internal to this tree
/**     \relatesalso Hierarchy
        \see Bond
        \relatesalso Bond
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

/** \relatesalso Hierarchy */
inline HierarchiesTemp get_leaves(Hierarchy h) {
  return HierarchiesTemp(IMP::core::get_leaves(h));
}

//! Print out a molecular hierarchy
/** \relatesalso Hierarchy
 */
inline void show(Hierarchy h, std::ostream &out=std::cout) {
   IMP::core::show<Hierarchy>(h, out);
}

//! make the given hierarchy move rigidly
/** The rigid body created has all the leaves as members and a
    member rigid body for each internal node in the tree.

    \note any existing coordinates and radii for internal nodes
    are overwritten.

    \relatesalso Hierarchy
    \relatesalso IMP::core::RigidBody
*/
IMPATOMEXPORT IMP::core::RigidBody rigid_body_setup_hierarchy(Hierarchy h);

//! Return true if the piece of hierarchy should be classified as a heterogen
/** For the purposes of classification, a heterogen is anything that
    - is a heterogen atom (one whose name starts with HET_)
    - is or is part of a Residue that is not a normal protein, rna or
      dna residue
    - or is not part of a Chain
    For the moment, this can only be called on residues or atoms.
*/
IMPATOMEXPORT bool get_is_heterogen(Hierarchy h);

//! Clone the Hierarchy
/** This method copies the Bond, Bonded, Atom,
    Residue, and Domain data and the particle name to the
    new copies in addition to the Hierarchy relationships.

    \relatesalso Hierarchy
*/
IMPATOMEXPORT
Hierarchy create_clone(Hierarchy d);

//! Clone the node in the Hierarchy
/** This method copies the  Atom,
    Residue, Chain and Domain data and the particle name.

    \relatesalso Hierarchy
*/
IMPATOMEXPORT
Hierarchy create_clone_one(Hierarchy d);


//! Delete the Hierarchy
/** All bonds connecting to these atoms are destroyed as are
    hierarchy links in the Hierarchy and the particles are
    removed from the Model.
    \relatesalso Hierarchy
*/
IMPATOMEXPORT
void destroy(Hierarchy d);



//! Get a bounding box for the Hierarchy
/** This bounding box is that of the highest (in the CS sense of a tree
    growning down from the root) cut
    through the tree where each node in the cut has x,y,z, and r.
    That is, if the root has x,y,z,r then it is the bounding box
    if that sphere. If only the leaves have radii, it is the bounding
    box of the leaves. If no such cut exists, the behavior is undefined.
    \relatesalso Hierarchy
    \relatesalso IMP::algebra::BoundingBoxD
 */
IMPATOMEXPORT
algebra::BoundingBoxD<3> get_bounding_box(const Hierarchy &h);


/** See get_bounding_box() for more details.
    \relatesalso Hierarchy
 */
IMPATOMEXPORT
algebra::SphereD<3> get_bounding_sphere(const Hierarchy &h);


IMPATOM_END_NAMESPACE


#endif  /* IMPATOM_HIERARCHY_H */
