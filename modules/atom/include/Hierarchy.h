/**
 *  \file IMP/atom/Hierarchy.h
 *  \brief Decorator for helping deal with a hierarchy of molecules.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_HIERARCHY_H
#define IMPATOM_HIERARCHY_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/utility.h>
#include <IMP/core/Hierarchy.h>
#include "bond_decorators.h"
#include "atom_macros.h"
#include <IMP/core/XYZR.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/Gaussian.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <deque>

// DOMAIN is defined to be 1 by a fedora math header
#define IMP_ATOM_FOREACH_HIERARCHY_TYPE_LIST(macro)                        \
  macro(Atom, atom, ATOM_TYPE), macro(Residue, residue, RESIDUE_TYPE),     \
      macro(Chain, chain, CHAIN_TYPE),                                     \
      macro(Molecule, molecule, MOLECULE_TYPE),                            \
      macro(Domain, domain, DOMAIN_TYPE),                                  \
      macro(Fragment, fragment, FRAGMENT_TYPE),                            \
      macro(core::XYZ, xyz, XYZ_TYPE), macro(core::XYZR, xyzr, XYZR_TYPE), \
      macro(Mass, mass, MASS_TYPE),                                        \
      macro(State, state, STATE_TYPE)

// DOMAIN is defined to be 1 by a fedora math header
#define IMP_ATOM_FOREACH_HIERARCHY_TYPE_STATEMENTS(macro) \
  macro(Atom, atom, ATOM_TYPE);                           \
  macro(Residue, residue, RESIDUE_TYPE);                  \
  macro(Chain, chain, CHAIN_TYPE);                        \
  macro(Molecule, molecule, MOLECULE_TYPE);               \
  macro(Domain, domain, DOMAIN_TYPE);                     \
  macro(Fragment, fragment, FRAGMENT_TYPE);               \
  macro(core::XYZ, xyz, XYZ_TYPE);                        \
  macro(core::XYZR, xyzr, XYZR_TYPE);                     \
  macro(Mass, mass, MASS_TYPE);                           \
  macro(State, state, STATE_TYPE)

// DOMAIN is defined to be 1 by a fedora math header
#define IMP_ATOM_FOREACH_HIERARCHY_TYPE_FUNCTIONS(macro)                       \
  macro(Atom, atom, ATOM_TYPE) macro(Residue, residue, RESIDUE_TYPE)           \
      macro(Chain, chain, CHAIN_TYPE) macro(Molecule, molecule, MOLECULE_TYPE) \
      macro(Domain, domain, DOMAIN_TYPE)                                       \
      macro(Fragment, fragment, FRAGMENT_TYPE) macro(core::XYZ, xyz, XYZ_TYPE) \
      macro(core::XYZR, xyzr, XYZR_TYPE) macro(Mass, mass, MASS_TYPE)          \
      macro(State, state, STATE_TYPE)
      IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_ATOM_CAPS_NAME(UCName, lcname, CAPSNAME) CAPSNAME

IMPATOM_BEGIN_NAMESPACE
class Atom;
class Residue;
class Domain;
class Fragment;
class Chain;
class Molecule;
class Mass;
class State;
class Representation;

IMP_DECORATORS_DECL(Hierarchy, Hierarchies);

//! The standard decorator for manipulating molecular structures.
/** \imp represents molecular structures using the Hierarchy decorator.
    Molecules and collections of molecules each are stored as a
    hierarchy (or tree) where the resolution of the representation increases
    as you move further from the root. That is, if a parent has
    some particular property (eg, marks out a volume by having
    x,y,z coordinates and a radius), then the children should have
    a higher resolution form of that information (eg, mark out a more
    detailed excluded volume by defining a set of balls which have
    approximately the same total volume).

    \section tree_basics Tree Basics
    In a tree you have a set of nodes, represented by Hierarchy particles.
    Each node can have at most one parent. The node with no
    parent is known as the root of the tree.

    Here is a simple example with a protein with three residues. Two of the
    residues have atoms, where as the third is coarse grained.
    \dotgraph{\dot
    digraph example {
    node [shape=record\, fontname= Helvetica\, fontsize=10]
      a [label="Protein A (the root)"];
      b [label="Residue 0"\, URL="Residue"];
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


    The nodes in the hierarchy can correspond to arbitrary bits of a
    molecule and do not need to have any biological significance. For
    example we could introduce a fragment containing residues 0 and 1:
    \dotgraph{\dot
    digraph example {
    node [shape=record\, fontname= Helvetica\, fontsize=10]
      a [label="Protein A (the root)"];
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
    - the leaves always have coordinates, radius and mass
    - all particles in the hierarchy are from the same model
    - all Atoms have a Residue as the parent
    - any Atom with a non-heterogen atom type is part of a protein,
    DNA or RNA molecule
    - all Residue children of a particle appear in order based
      on their index
    - all Atom children of a particle appear in order of their
      AtomType
    - if a node has residue indexes, all its descendants down to the
      residue level also do.

    The get_is_valid() method checks some of these properties. Any
    method taking a hierarchy as an argument should do
    \code
    IMP_USAGE_CHECK(h.get_is_valid(), "Invalid hierarchy as input");
    \endcode
    to make sure the hierarchy makes sense.

    A number of decorator types are associated with the Hierarchy
    to store the information associated with that node in the
    hierarchy. Examples include Residue, Atom, XYZ, Chain, XYZR,
    Mass, Domain, Molecule etc.

    \note Deleting a Hierarchy, like deleting any decorator, will not
          delete the underlying Particle or remove any children. To do
          that, call destroy().

    \see Atom
    \see Residue
    \see Chain
    \see Molecule
    \see Domain
    \see Fragment
    \see Mass
    \see State
    \see Representation
 */
class IMPATOMEXPORT Hierarchy : public core::Hierarchy {
  typedef core::Hierarchy H;

 public:
#ifndef IMP_DOXYGEN
  typedef boost::false_type DecoratorHasTraits;

  //! Setup the particle as a hierarchy; add the passed particles as children.
  static Hierarchy setup_particle(Particle *p,
                                  ParticleIndexesAdaptor children) {
    H::setup_particle(p, get_traits());
    Hierarchy ret(p);
    for (unsigned int i = 0; i < children.size(); ++i) {
      if (!get_is_setup(p->get_model(), children[i])) {
        setup_particle(p->get_model(), children[i]);
      }
      ret.add_child(Hierarchy(p->get_model(), children[i]));
    }
    return ret;
  }

  static Hierarchy setup_particle(Particle *p) {
    return setup_particle(p->get_model(), p->get_index());
  }

  static bool get_is_setup(Particle *p) {
    return H::get_is_setup(p, get_traits());
  }
#endif

  Hierarchy(Model *m, ParticleIndex pi)
      : H(m, pi, get_traits()) {}

  Hierarchy(ParticleAdaptor pi)
      : H(pi.get_model(), pi.get_particle_index(), get_traits()) {}

  //! Null constructor
  Hierarchy() {}

  //! The traits must match
  explicit Hierarchy(IMP::core::Hierarchy h) : H(h) {
    IMP_USAGE_CHECK(
        h != IMP::core::Hierarchy() || h.get_decorator_traits() == get_traits(),
        "Cannot construct a IMP.atom.Hierarchy from a general "
        " IMP.core.Hierarchy");
  }

  //! Create a Hierarchy of level t by adding the needed attributes.
  static Hierarchy setup_particle(Model *m, ParticleIndex pi,
                                  ParticleIndexesAdaptor children =
                                      ParticleIndexesAdaptor()) {
    H::setup_particle(m, pi, get_traits());
    Hierarchy ret(m, pi);
    for (unsigned int i = 0; i < children.size(); ++i) {
      if (!get_is_setup(m, children[i])) {
        setup_particle(m, children[i]);
      }
      ret.add_child(Hierarchy(m, children[i]));
    }
    return ret;
  }

  //! Check if the particle has the needed attributes for a cast to succeed
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return H::get_is_setup(m->get_particle(p), get_traits());
  }

  //! Return true if the hierarchy is valid.
  /** Print information about the hierarchy if print_info is
      true and things are invalid.
      \note Returning true only means that no problems were
      found; it can't check everything.*/
  bool get_is_valid(bool print_info=false) const;

  //! Add a child and check that the types are appropriate
  /** A child must have a type that is listed before the parent in the
      Type enum list.
   */
  void add_child(Hierarchy o) {
    IMP_USAGE_CHECK(o != *this, "Can't add something as its own child");
    H::add_child(o);
  }

#ifndef IMP_DOXYGEN
  void show(std::ostream &out, std::string delimiter) const;
#endif

  //! Get the ith child based on the order they were added.
  Hierarchy get_child(unsigned int i) const {
    H hd = H::get_child(i);
    return Hierarchy(hd);
  }
  //! Return the children in the order they were added
  Hierarchies get_children() const {
    Hierarchies ret(get_number_of_children());
    for (unsigned int i = 0; i < get_number_of_children(); ++i) {
      ret[i] = get_child(i);
    }
    return ret;
  }

  //! Get the children in a container of your choosing, eg ParticlesTemp
  template <class C>
  C get_children() const {
    C ret(get_number_of_children());
    for (unsigned int i = 0; i < get_number_of_children(); ++i) {
      ret[i] = get_child(i);
    }
    return ret;
  }

  //! Get the parent particle.
  Hierarchy get_parent() const {
    H hd = H::get_parent();
    if (hd == H()) {
      return Hierarchy();
    } else {
      return Hierarchy(hd);
    }
  }

  //! Get the molecular hierarchy HierarchyTraits.
  static const IMP::core::HierarchyTraits &get_traits();

  // swig overwrites __repr__ if it is inherited
  IMP_SHOWABLE(Hierarchy);
};

IMP_DECORATORS_DEF(Hierarchy, Hierarchies);

#ifdef IMP_DOXYGEN
//! The different types which can be passed to get_by_type()
enum GetByType {
  ATOM_TYPE,
  RESIDUE_TYPE,
  CHAIN_TYPE,
  MOLECULE_TYPE,
  DOMAIN_TYPE,
  FRAGMENT_TYPE,
  XYZ_TYPE,
  XYZR_TYPE,
  MASS_TYPE,
  STATE_TYPE
};
#else
enum GetByType {
  IMP_ATOM_FOREACH_HIERARCHY_TYPE_LIST(IMP_ATOM_CAPS_NAME)
};
#endif

//! Gather all the molecular particles of a certain level in the hierarchy.
/** \relates Hierarchy */
IMPATOMEXPORT Hierarchies get_by_type(Hierarchy mhd, GetByType t);

//! Get the residue with the specified index
/** Find the leaf containing the residue with the appropriate index.
    This is the PDB index, not the offset in the chain (if they are different).

    The function returns a Hierarchy, rather than a Residue since the
    residue may not be explicitly represented and may just be part of some
    fragment.

    \throw ValueException if mhd's type is not one of CHAIN, PROTEIN, NUCLEOTIDE
    \return Hierarchy() if that residue is not found.

    \relates Hierarchy
 */
IMPATOMEXPORT Hierarchy get_residue(Hierarchy mhd, unsigned int index);

//! Create a fragment containing the specified nodes
/** A particle representing the fragment is created and initialized.

    The Fragment is inserted as a child of the parent (and the particles are
    removed). The particles become children of the fragment.

    \throw ValueException If all the particles do not have the same parent.
    \relates Hierarchy
 */
IMPATOMEXPORT Hierarchy create_fragment(const Hierarchies &ps);

//! Get the bonds internal to this tree
/** \relates Hierarchy
    \see Bond
 */
IMPATOMEXPORT Bonds get_internal_bonds(Hierarchy mhd);

//! Return the root of the hierarchy
/** \relates Hierarchy */
inline Hierarchy get_root(Hierarchy h) {
  while (h.get_parent()) {
    h = h.get_parent();
  }
  return h;
}

/** \relates Hierarchy */
inline Hierarchies get_leaves(Hierarchy h) {
  return Hierarchies(IMP::core::get_leaves(h));
}

/** \relates Hierarchy */
inline Hierarchies get_leaves(const Hierarchies &h) {
  ParticlesTemp ret;
  for (unsigned int i = 0; i < h.size(); ++i) {
    core::GenericHierarchies cur = IMP::core::get_leaves(h[i]);
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  return get_as<Hierarchies>(ret);
}

//! Print out a molecular hierarchy
/** \relates Hierarchy
 */
inline void show(Hierarchy h, std::ostream &out = std::cout) {
  IMP::core::show<Hierarchy>(h, out);
}

//! Rigidify a molecule or collection of molecules.
/** The rigid body created has all the leaves as members and a
    member rigid body for each internal node in the tree. The
    particle created to be the rigid body is returned.

    A name can be passed as it is not easy to automatically pick
    a decent name.
    \see create_aligned_rigid_body()
    \see Hierarchy
    \see IMP::core::RigidBody
*/
IMPATOMEXPORT IMP::core::RigidBody create_rigid_body(
    const Hierarchies &h, std::string name = std::string("created rigid body"));

/** \see create_rigid_body(const Hierarchies&)
 */
IMPATOMEXPORT IMP::core::RigidBody create_rigid_body(Hierarchy h);

//! Rigidify a molecule or collection of molecules.
/** This method is identical to create_rigid_body() except that
    the chosen reference frame is aligned with that of reference
    (which must have exactly the same set of particles). This allows
    one to make sure the rigid body is equivalent when you have several
    copies of the same molecule.

    \see Hierarchy
    \see IMP::core::RigidBody
*/
IMPATOMEXPORT IMP::core::RigidBody create_compatible_rigid_body(
    Hierarchy h, Hierarchy reference);

#ifndef IMP_DOXYGEN
IMPATOMEXPORT IMP::core::RigidBody setup_as_rigid_body(Hierarchy h);
#endif

//! Return true if the piece of hierarchy should be classified as a heterogen
/** For the purposes of classification, a heterogen is anything that
    - is a heterogen atom (one whose name starts with HET:)
    - is or is part of a Residue that is not a normal protein, rna or
      dna residue
    - or is not part of a Chain

    For the moment, this can only be called on residues or atoms.
    \relates Hierarchy
*/
IMPATOMEXPORT bool get_is_heterogen(Hierarchy h);

//! Clone the Hierarchy
/** This method copies the Bond, Bonded, Atom,
    Residue, and Domain data and the particle name to the
    new copies in addition to the Hierarchy relationships.

    \relates Hierarchy
*/
IMPATOMEXPORT Hierarchy create_clone(Hierarchy d);

//! Clone the node in the Hierarchy
/** This method copies the Atom,
    Residue, Chain and Domain data and the particle name.

    \relates Hierarchy
*/
IMPATOMEXPORT Hierarchy create_clone_one(Hierarchy d);

//! Delete the Hierarchy
/** All bonds connecting to these atoms are destroyed as are
    hierarchy links in the Hierarchy and the particles are
    removed from the Model. If this particle has a parent, it is
    removed from the parent.
    \relates Hierarchy
*/
IMPATOMEXPORT void destroy(Hierarchy d);

//! Get a bounding box for the Hierarchy
/** This bounding box is that of the highest (in the CS sense of a tree
    growing down from the root) cut
    through the tree where each node in the cut has x,y,z, and r.
    That is, if the root has x,y,z,r then it is the bounding box
    of that sphere. If only the leaves have radii, it is the bounding
    box of the leaves. If no such cut exists, the behavior is undefined.
    \relates Hierarchy
    \see IMP::algebra::BoundingBoxD
 */
IMPATOMEXPORT algebra::BoundingBoxD<3> get_bounding_box(const Hierarchy &h);

/** See get_bounding_box() for more details.
    \relates Hierarchy
 */
IMPATOMEXPORT algebra::Sphere3D get_bounding_sphere(const Hierarchy &h);


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
// Get the parent, or if non-tree Representation get the fake parent
inline atom::Hierarchy get_parent_representation(Hierarchy h){
  if (h.get_model()->get_has_attribute(
     Hierarchy::get_traits().get_parent_key(),h.get_particle_index())){
    ParticleIndex pidx = h.get_model()->get_attribute(
          Hierarchy::get_traits().get_parent_key(),h.get_particle_index());
      return Hierarchy(h.get_model(),pidx);
  }
  else return Hierarchy();
}
#endif

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_HIERARCHY_H */
