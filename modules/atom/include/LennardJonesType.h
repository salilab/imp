/**
 *  \file IMP/atom/LennardJonesType.h
 *  \brief Parameters for a Lennard-Jones interaction.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_LENNARD_JONES_TYPE_H
#define IMPATOM_LENNARD_JONES_TYPE_H

#include <IMP/atom/atom_config.h>
#include <IMP/Pointer.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

//! Parameters for a Lennard-Jones interaction.
/** This type stores a well depth and radius, which can be applied to
    any number of particles using the LennardJonesTyped decorator and then
    used by LennardJonesTypedPairScore. The parameters can be changed at
    any time, which will change the interaction of all particles using this
    type.

    Note that there is currently no support for overriding the interaction
    between two specific types (as is done in the CHARMM forcefield with the
    NBFIX directive, for example) but that could be added if needed.
 */
class IMPATOMEXPORT LennardJonesType : public Object {
  double well_depth_;
  double radius_;
  int index_;
public:
  LennardJonesType(double well_depth, double radius,
                   std::string name="LennardJonesType%1%");
 
  double get_well_depth() const { return well_depth_; }

  void set_well_depth(double d);

  double get_radius() const { return radius_; }

  void set_radius(double r);

  //! Get the globally unique identifier for this type.
  int get_index() const { return index_; }

  IMP_OBJECT_METHODS(LennardJonesType);
};

IMP_OBJECTS(LennardJonesType, LennardJonesTypes);

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {

class IMPATOMEXPORT LennardJonesParameters : public Object {
  LennardJonesTypes types_;

public:
  // Mapping from LJ types for a particle pair to A factor
  std::vector<double> aij_;

  // Mapping from LJ types for a particle pair to B factor
  std::vector<double> bij_;

  LennardJonesParameters() : Object("LennardJonesParameters %1%") {}

  int add(LennardJonesType *typ);

  LennardJonesType *get(int index);

  // Calculate aij, bij factors for all types interacting with type i
  void precalculate(int i);

  int get_parameter_index(int i, int j) const {
    // matrix is symmetric so we only need to store half. Roll maxij
    // first so that we don't have to rearrange the vectors when we add
    // a new type.
    int maxij = std::max(i, j);
    int minij = std::min(i, j);
    return (maxij+1)*maxij / 2 + minij;
  }
};

//! Get the singleton object that stores all parameters
IMPATOMEXPORT LennardJonesParameters* get_lj_params();

} // namespace internal
#endif

//! A decorator for a particle that has a Lennard-Jones potential well.
/** Such particles must be XYZ particles (they must have a position)
    but need not be true atoms. Note that if the particle is an XYZR
    particle, the radius used for Lennard-Jones is the LennardJonesTyped
    radius, not the XYZR radius.

    \ingroup helper
    \ingroup decorators
    \see LennardJonesType
    \see LennardJonesTypedPairScore
 */
class IMPATOMEXPORT LennardJonesTyped : public core::XYZ {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                LennardJonesType *type) {
    IMP_USAGE_CHECK(XYZ::get_is_setup(m, pi),
                    "Particle must already be an XYZ particle");
    m->add_attribute(get_type_key(), pi, type->get_index());
  }
 public:
  IMP_DECORATOR_METHODS(LennardJonesTyped, core::XYZ);
  IMP_DECORATOR_SETUP_1(LennardJonesTyped, LennardJonesType*, type);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return XYZ::get_is_setup(m, pi) &&
           m->get_has_attribute(get_type_key(), pi);
  }

  void set_type(LennardJonesType *type) {
    get_model()->set_attribute(get_type_key(), get_particle_index(),
                               type->get_index());
  }

  LennardJonesType* get_type() const {
    int ind = get_index();
    return internal::get_lj_params()->get(ind);
  }

  int get_index() const {
    return get_model()->get_attribute(get_type_key(), get_particle_index());
  }

  double get_well_depth() const {
    return get_type()->get_well_depth();
  }

  double get_radius() const {
    return get_type()->get_radius();
  }

  //! Get the key used to store the type.
  static IntKey get_type_key();
};

IMP_DECORATORS(LennardJonesTyped, LennardJonesTypedList, core::XYZs);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_TYPE_H */
