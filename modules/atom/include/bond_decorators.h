/**
 *  \file IMP/atom/bond_decorators.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BOND_DECORATORS_H
#define IMPATOM_BOND_DECORATORS_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/internal/graph_base.h>
#include "internal/bond_helpers.h"
#include <IMP/display/particle_geometry.h>
#include <IMP/display/primitive_geometries.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>
#include <IMP/core/XYZ.h>

#include <IMP/kernel/internal/IndexingIterator.h>
IMPATOM_BEGIN_NAMESPACE

class Bond;
class Bonded;

/** \defgroup bond Creating and restraining bonds
    A set of classes and functions for manipulating bonds.
 */

//! A decorator for wrapping a particle representing a molecular bond
/**
   As with Atom, the types of bonds will eventually be run-time
   expandable.

   \ingroup bond
   \see Bonded
   \see IMP::atom::get_internal_bonds()
 */
class IMPATOMEXPORT Bond : public Decorator {
  friend class Bonded;

 public:
  IMP_DECORATOR_METHODS(Bond, Decorator);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return IMP::core::internal::graph_is_edge(m->get_particle(pi),
                                              internal::get_bond_data().graph_);
  }

  //! The types a bond can have right now
  enum Type {
    UNKNOWN = -1,
    NONBIOLOGICAL,
    SINGLE = 1,
    DOUBLE = 2,
    TRIPLE = 3,
    HYDROGEN,
    SALT,
    PEPTIDE,
    AMIDE,
    AROMATIC
  };

  //! Get the atom i of the bond
  /** \param[in] i 0 or 1
      \return Bonded for the atom in question
  */
  Bonded get_bonded(unsigned int i) const;

  IMP_DECORATOR_GET_SET_OPT(type, internal::get_bond_data().type_, Int, Int,
                            UNKNOWN);

  IMP_DECORATOR_GET_SET_OPT(order, internal::get_bond_data().order_, Int, Int,
                            1);

  IMP_DECORATOR_GET_SET_OPT(length, internal::get_bond_data().length_, Float,
                            Float, -1);
  IMP_DECORATOR_GET_SET_OPT(stiffness, internal::get_bond_data().stiffness_,
                            Float, Float, -1);

  static FloatKey get_length_key() { return internal::get_bond_data().length_; }
};

//! A decorator for a particle which has bonds.
/** \ingroup bond
    \see Bond
 */
class IMPATOMEXPORT Bonded : public Decorator {
  struct GetBond {
    typedef Bond result_type;
    kernel::Particle *d_;
    GetBond() : d_(nullptr) {}
    GetBond(kernel::Particle *d) : d_(d) {}
    Bond operator()(unsigned int i) const;
    bool operator==(const GetBond &o) const { return d_ == o.d_; }
  };
  struct GetBonded {
    typedef Bonded result_type;
    kernel::Particle *d_;
    GetBonded() : d_(nullptr) {}
    GetBonded(kernel::Particle *d) : d_(d) {}
    Bonded operator()(unsigned int i) const;
    bool operator==(const GetBonded &o) const { return d_ == o.d_; }
  };
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi) {
    graph_initialize_node(m->get_particle(pi),
                          internal::get_bond_data().graph_);
  }

 public:
  IMP_DECORATOR_METHODS(Bonded, Decorator);
  IMP_DECORATOR_SETUP_0(Bonded);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return IMP::core::internal::graph_is_node(m->get_particle(pi),
                                              internal::get_bond_data().graph_);
  }

  /** */
  unsigned int get_number_of_bonds() const {
    return graph_get_number_of_edges(get_particle(),
                                     internal::get_bond_data().graph_);
  }

  kernel::ParticleIndexes get_bond_indexes() const {
    return graph_get_edges(get_particle(), internal::get_bond_data().graph_);
  }

  //! Get a Bond of the ith bond
  /** \return decorator of the ith child, or throw an exception if there
              is no such bond
  */
  Bond get_bond(unsigned int i) const {
    kernel::Particle *p =
        graph_get_edge(get_particle(), i, internal::get_bond_data().graph_);
    return Bond(p);
  }

  //! Get a Bonded of the ith bonded particle
  /** \return decorator of the ith child, or throw an exception if there
              is no such bond

      \note I don't entirely like having this here as it duplicates
      functionality available otherwise, however it is such a fundamental
      operation and kind of a pain to write. It also means that we
      could later pull the edge endpoints into the vertex if
      desired.
  */
  Bonded get_bonded(unsigned int i) const {
    kernel::Particle *p =
        graph_get_edge(get_particle(), i, internal::get_bond_data().graph_);
    Bond bd(p);
    if (bd.get_bonded(0) == *this)
      return bd.get_bonded(1);
    else
      return bd.get_bonded(0);
  }

/** @name Iterate through the bonds
    @{
*/
#ifdef IMP_DOXYGEN
  class BondIterator;
#else
  typedef IMP::internal::IndexingIterator<GetBond> BondIterator;
#endif
#ifndef SWIG
  BondIterator bonds_begin() const {
    return BondIterator(GetBond(get_particle()), 0);
  }
  BondIterator bonds_end() const {
    return BondIterator(GetBond(get_particle()), get_number_of_bonds());
  }
#endif
/** @} */
/** @name Iterate through the bondeds
    @{
*/
#ifdef IMP_DOXYGEN
  class BondedIterator;
#else
  typedef IMP::internal::IndexingIterator<GetBonded> BondedIterator;
#endif
#ifndef SWIG
  BondedIterator bondeds_begin() const {
    return BondedIterator(GetBonded(get_particle()), 0);
  }
  BondedIterator bondeds_end() const {
    return BondedIterator(GetBonded(get_particle()), get_number_of_bonds());
  }
#endif
  /** @} */
};

IMP_DECORATORS(Bonded, Bondeds, kernel::ParticlesTemp);
IMP_DECORATORS(Bond, Bonds, kernel::ParticlesTemp);

inline Bonded Bond::get_bonded(unsigned int i) const {
  kernel::Particle *p =
      graph_get_node(get_particle(), i, internal::get_bond_data().graph_);
  return Bonded(p);
}

#ifndef IMP_DOXYGEN
inline Bond Bonded::GetBond::operator()(unsigned int i) const {
  return Bonded(d_).get_bond(i);
}
inline Bonded Bonded::GetBonded::operator()(unsigned int i) const {
  return Bonded(d_).get_bonded(i);
}
#endif

//! Connect the two wrapped particles by a bond.
/** \param[in] a The first kernel::Particle as a Bonded
    \param[in] b The second kernel::Particle as a Bonded
    \param[in] t The type to use for the bond
    \return Bond of the bond kernel::Particle.

    \ingroup bond
    \see Bond
    \see Bonded
 */
IMPATOMEXPORT Bond create_bond(Bonded a, Bonded b, Int t);

//! Connect the two wrapped particles by a custom bond.
/** \param[in] a The first kernel::Particle as a Bonded
    \param[in] b The second kernel::Particle as a Bonded
    \param[in] length The length of the bond.
    \param[in] stiffness The stiffness of the bond.
    \return Bond of the bond kernel::Particle.

    \ingroup bond
    \see Bond
    \see Bonded
 */
IMPATOMEXPORT inline Bond create_custom_bond(Bonded a, Bonded b, Float length,
                                             Float stiffness = -1) {
  IMP_INTERNAL_CHECK(length >= 0, "Length must be positive");
  Bond bd = create_bond(a, b, Bond::NONBIOLOGICAL);
  bd.set_length(length);
  bd.get_particle()->set_name(std::string("bond ") +
                              a.get_particle()->get_name() + " and " +
                              b.get_particle()->get_name());
  if (stiffness >= 0) bd.set_stiffness(stiffness);
  return bd;
}

//! Connect the two wrapped particles by a custom bond.
/** Create a bond by copying the information from the other bond

    \ingroup bond
    \see Bond
    \see Bonded
 */
IMPATOMEXPORT inline Bond create_bond(Bonded a, Bonded b, Bond o) {
  Bond bd = create_bond(a, b, o.get_type());
  if (o.get_length() > 0) bd.set_length(o.get_length());
  bd.get_particle()->set_name(std::string("bond ") +
                              a.get_particle()->get_name() + " and " +
                              b.get_particle()->get_name());
  if (o.get_stiffness() >= 0) bd.set_stiffness(o.get_stiffness());
  return bd;
}

//! Destroy the bond connecting two particles.
/** \param[in] b The bond.
    \ingroup bond
    \see Bond
    \see Bonded
 */
IMPATOMEXPORT void destroy_bond(Bond b);

//! Get the bond between two particles.
/** Bond() is returned if the particles are not bonded.
    \ingroup bond
    \see Bond
    \see Bonded
 */
IMPATOMEXPORT Bond get_bond(Bonded a, Bonded b);

/** \class BondGeometry
    \brief Display a Bond particle as a segment.

    \class BondsGeometry
    \brief Display an IMP::SingletonContainer of Bond particles
    as segments.
*/
IMP_PARTICLE_GEOMETRY(Bond, Bond, {
  atom::Bonded ep0 = d.get_bonded(0);
  core::XYZ epi0(ep0.get_particle());
  atom::Bonded ep1 = d.get_bonded(1);
  core::XYZ epi1(ep1.get_particle());
  algebra::Segment3D s(epi0.get_coordinates(), epi1.get_coordinates());
  display::Geometry *g = new display::SegmentGeometry(s);
  ret.push_back(g);
});

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BOND_DECORATORS_H */
