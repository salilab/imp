/**
 *  \file IMP/atom/Hierarchy.h
 *  \brief Decorator for helping deal with a hierarchy of molecules.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#define IMP_FOREACH_HIERARCHY_TYPE_LIST(macro)      \
  macro(Atom, atom, ATOM_TYPE),                     \
    macro(Residue, residue, RESIDUE_TYPE),          \
    macro(Chain, chain, CHAIN_TYPE),                \
    macro(Molecule, molecule, MOLECULE_TYPE),       \
    macro(Domain, domain, DOMAIN_TYPE),             \
    macro(Fragment, fragment, FRAGMENT_TYPE),       \
    macro(core::XYZ, xyz, XYZ_TYPE),                \
    macro(core::XYZR, xyzr, XYZR_TYPE),             \
    macro(Mass, mass, MASS_TYPE)

// DOMAIN is defined to be 1 by a fedora math header
#define IMP_FOREACH_HIERARCHY_TYPE_STATEMENTS(macro)  \
  macro(Atom, atom, ATOM_TYPE);                \
  macro(Residue, residue, RESIDUE_TYPE);       \
  macro(Chain, chain, CHAIN_TYPE);             \
  macro(Molecule, molecule, MOLECULE_TYPE);    \
  macro(Domain, domain, DOMAIN_TYPE);          \
  macro(Fragment, fragment, FRAGMENT_TYPE);    \
  macro(core::XYZ, xyz, XYZ_TYPE);             \
  macro(core::XYZR, xyzr, XYZR_TYPE);          \
  macro(Mass, mass, MASS_TYPE)

// DOMAIN is defined to be 1 by a fedora math header
#define IMP_FOREACH_HIERARCHY_TYPE_FUNCTIONS(macro)    \
  macro(Atom, atom, ATOM_TYPE)                         \
  macro(Residue, residue, RESIDUE_TYPE)                \
  macro(Chain, chain, CHAIN_TYPE)                      \
  macro(Molecule, molecule, MOLECULE_TYPE)             \
  macro(Domain, domain, DOMAIN_TYPE)                   \
  macro(Fragment, fragment, FRAGMENT_TYPE)             \
  macro(core::XYZ, xyz, XYZ_TYPE)                      \
  macro(core::XYZR, xyzr, XYZR_TYPE)                   \
  macro(Mass, mass, MASS_TYPE)                         \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_CAPS_NAME(UCName, lcname, CAPSNAME) \
  CAPSNAME


IMPATOM_BEGIN_NAMESPACE
class Atom;
class Residue;
class Domain;
class Fragment;
class Chain;
class Molecule;
class Mass;

IMP_DECORATORS_DECL(Hierarchy, Hierarchies);


//! The standard decorator for manipulating molecular structures.
/** \imp represents molecular structures using the Hierachy decorator.
    Molecules and collections of molecules each are stored as a
    hierarchy (or tree) where the resolution of the representation increases
    as you move further from the root. That is, if a parent has
    some particular property (eg, marks out a volume by having
    a x,y,z coordinates and a radius), then the children should have
    a higher resolution form of that information (eg, mark out a more
    detailed excluded volume by defining a set of balls which having
    approximately the same total volume).

    \section tree_basics Tree Basics
    In a tree you have a set of nodes, represented by Hierarchy particles.
    Each node can have a node can have at most one parent. The node with no
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
    molecule and do not need to have any biological significant. For
    example we could introduce a fragment containing residues 0 and 1:
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
    - the leaves always have coordinates, radius and mass
    - all particles in hierarchy are from the same model
    - all Atoms has a Residue for as parent
    - any Atom with a non-heterogen atom type is part of a protein,
    DNA or RNA molecule.
    - all Residue children of a particle appear in order based
      on their index
    - all Atom children in of a particle appear in order of their
      AtomType

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
    We provide a get_as_x() function for each such decorator which
    returns either X() (a null type) if the node is not a particle
    of type x, or an X decorator wrapping the current particle if
    it is.

    \ingroup hierarchy
    \ingroup decorators
    \see Atom
    \see Residue
    \see Chain
    \see Molecule
    \see Domain
    \see Fragment
    \see Mass
 */
class IMPATOMEXPORT Hierarchy: public core::Hierarchy
{
  typedef core::Hierarchy H;
public:
  IMP_NO_DOXYGEN(typedef boost::false_type DecoratorHasTraits);
  explicit Hierarchy(Particle *p): H(p, get_traits()) {
  }

  Hierarchy(Model *m, ParticleIndex pi): H(m, pi, get_traits()) {
  }

  //! null constructor
  Hierarchy() {}

  //! cast a particle which has the needed attributes
  static Hierarchy decorate_particle(Particle *p) {
    H::decorate_particle(p, get_traits());
    return Hierarchy(p);
  }

  //! The traits must match
  explicit Hierarchy(IMP::core::Hierarchy h): H(h) {
    IMP_USAGE_CHECK(h != IMP::core::Hierarchy()
                    || h.get_traits() == get_traits(),
              "Cannot construct a IMP.atom.Hierarchy from a general "
              " IMP.core.Hierarchy");
  }

  static Hierarchy setup_particle(Particle* p,
                                  const ParticlesTemp &children) {
    H::setup_particle(p, get_traits());
    Hierarchy ret(p);
    for (unsigned int i=0; i< children.size(); ++i) {
      if (!particle_is_instance(children[i])) {
        setup_particle(children[i]);
      }
      ret.add_child(Hierarchy(children[i]));
    }
    return ret;
  }

  /** Create a Hierarchy of level t by adding the needed
      attributes. */
  static Hierarchy setup_particle(Model *m, ParticleIndex pi,
                                  const ParticlesTemp &children
                                  = ParticlesTemp()) {
    Particle *p= m->get_particle(pi);
    H::setup_particle(p, get_traits());
    Hierarchy ret(p);
    for (unsigned int i=0; i< children.size(); ++i) {
      if (!particle_is_instance(children[i])) {
        setup_particle(children[i]);
      }
      ret.add_child(Hierarchy(children[i]));
    }
    return ret;
  }

 static Hierarchy setup_particle(Particle *p) {
    return setup_particle(p->get_model(),
                          p->get_index());
  }

  /** Check if the particle has the needed attributes for a
   cast to succeed */
  static bool particle_is_instance(Particle *p){
    return H::particle_is_instance(p, get_traits());
  }

  static bool particle_is_instance(Model *m, ParticleIndex p){
    return H::particle_is_instance(m->get_particle(p), get_traits());
  }

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
  void add_child(Hierarchy o) {
    IMP_USAGE_CHECK(o != *this, "Can't add something as its own child");
    H::add_child(o);
  }

  /** Get the ith child based on the order they were added. */
  Hierarchy get_child(unsigned int i) const {
    H hd=  H::get_child(i);
    return Hierarchy(hd);
  }
  //! Return the children in the order they were added
  Hierarchies get_children() const {
    Hierarchies ret(get_number_of_children());
    for (unsigned int i=0; i< get_number_of_children(); ++i) {
      ret[i]= get_child(i);
    }
    return ret;
  }

  //! Get the children in a container of your choosing, eg ParticlesTemp
  template <class C>
      C get_children() const {
    C ret(get_number_of_children());
    for (unsigned int i=0; i< get_number_of_children(); ++i) {
      ret[i]= get_child(i);
    }
    return ret;
  }

  /** Get the parent particle. */
  Hierarchy get_parent() const {
    H hd= H::get_parent();
    if (hd == H()) {
      return Hierarchy();
    } else {
      return Hierarchy(hd);
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
  IMP_FOREACH_HIERARCHY_TYPE_FUNCTIONS(IMP_GET_AS_DECL);
  /** @} */

  //! Get the molecular hierarchy HierararchyTraits.
  static const IMP::core::HierarchyTraits& get_traits();

  // swig overwrites __repr__ if it is inherited
  IMP_SHOWABLE(Hierarchy);
};

IMP_DECORATORS_DEF(Hierarchy, Hierarchies);


#ifdef IMP_DOXYGEN
/** The different types which can be passed to get_by_type()
 */
enum GetByType {ATOM_TYPE, RESIDUE_TYPE, CHAIN_TYPE, MOLECULE_TYPE.
                DOMAIN_TYPE, FRAGMENT_TYPE,
                XYZ_TYPE,XYZR_TYPE,MASS_TYPE};
#else
enum GetByType {
  IMP_FOREACH_HIERARCHY_TYPE_LIST(IMP_CAPS_NAME)
};
#endif

/**
   Gather all the molecular particles of a certain level
   in the molecular hierarchy.
   \ingroup hierarchy
   \relatesalso Hierarchy
*/
IMPATOMEXPORT Hierarchies
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
/** A particle representing the fragment is created and initialized.

    The Fragment is inserted as a child of the parent (and the particles are
    removed). The particles become children of the fragment.

    \throw ValueException If all the particles do not have the same parent.
    \ingroup hierarchy
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
create_fragment(const Hierarchies &ps);

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
  while (h.get_parent()) {
    h= h.get_parent();
  }
  return h;
}

/** \relatesalso Hierarchy */
inline Hierarchies get_leaves(Hierarchy h) {
  return Hierarchies(IMP::core::get_leaves(h));
}

/** \relatesalso Hierarchy */
inline Hierarchies get_leaves(const Hierarchies& h) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< h.size(); ++i) {
    core::GenericHierarchies cur=IMP::core::get_leaves(h[i]);
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  return get_as<Hierarchies>(ret);
}

//! Print out a molecular hierarchy
/** \relatesalso Hierarchy
 */
inline void show(Hierarchy h, std::ostream &out=std::cout) {
   IMP::core::show<Hierarchy>(h, out);
}

//! Rigidify a molecule or collection of molecules.
/** The rigid body created has all the leaves as members and a
    member rigid body for each internal node in the tree. The
    particle created to be the rigid body is returned.

    A name can be passed as it is not easy to automatically pick
    a decent name.
    \see create_aligned_rigid_body()
    \relatesalso Hierarchy
    \relatesalso IMP::core::RigidBody
*/
IMPATOMEXPORT IMP::core::RigidBody create_rigid_body(const Hierarchies& h,
                        std::string name=std::string("created rigid body"));

/** \see create_rigid_body(const Hierarchies&)
 */
IMPATOMEXPORT IMP::core::RigidBody create_rigid_body(Hierarchy h);

//! Rigidify a molecule or collection of molecules.
/** This method is identical to create_rigid_body() except that
    the chosen reference frame is aligned with that of reference
    (which must have exactly the same set of particles). This allows
    one to make sure the rigid body is equivalent when you have several
    copies of the same molecule.

    \relatesalso Hierarchy
    \relatesalso IMP::core::RigidBody
*/
IMPATOMEXPORT IMP::core::RigidBody create_compatible_rigid_body(Hierarchy h,
                                                     Hierarchy reference);


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
    removed from the Model. If this particle has a parent, it is
    removed from the parent.
    \relatesalso Hierarchy
*/
IMPATOMEXPORT
void destroy(Hierarchy d);



//! Get a bounding box for the Hierarchy
/** This bounding box is that of the highest (in the CS sense of a tree
    growing down from the root) cut
    through the tree where each node in the cut has x,y,z, and r.
    That is, if the root has x,y,z,r then it is the bounding box
    of that sphere. If only the leaves have radii, it is the bounding
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
algebra::Sphere3D get_bounding_sphere(const Hierarchy &h);


IMPATOM_END_NAMESPACE


#endif  /* IMPATOM_HIERARCHY_H */
