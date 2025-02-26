/**
 *  \file IMP/atom/LennardJonesTypedPairScore.h
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H
#define IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/generic.h>
#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/atom/LennardJones.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

//! Parameters for a Lennard-Jones interaction.
/** This type stores a well depth and radius, which can be applied to
    any number of particles using the LennardJonesTyped decorator and then
    used by LennardJonesTypedPairScore. The parameters can be changed at
    any time, which will change the interaction of all particles using this
    type.
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

#ifndef SWIG
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

//! Lennard-Jones score between a pair of particles.
/** The two particles in the pair must be LennardJonesTyped particles.
    The form of the potential is \f[
       -\epsilon \left[ w_{rep} \left(\frac{r_{min}}{r}\right)^{12}
                       - 2 w_{att} \left(\frac{r_{min}}{r}\right)^{6}\right]
    \f] where \f$\epsilon\f$ is the depth of the well between the
    two particles, \f$r_{min}\f$ the distance corresponding to the minimum
    score, \f$r\f$ the inter-particle distance, and \f$w_{rep}\f$ and
    \f$w_{att}\f$ the weights on the repulsive and attractive parts of the
    potential respectively; both weights are 1.0 by default.

    The well depth is the geometric mean of the individual particles' well
    depths (as extracted by LennardJonesType::get_well_depth) and the minimum
    distance the sum of the particles' Lennard-Jones radii (as extracted
    by LennardJonesType::get_radius; note that this is not necessarily the
    same as the core::XYZR radius).
 */
class IMPATOMEXPORT LennardJonesTypedPairScore : public PairScore {
  IMP::PointerMember<atom::SmoothingFunction> smoothing_function_;
  double repulsive_weight_, attractive_weight_;
  IMP::PointerMember<internal::LennardJonesParameters> params_;

public:
  LennardJonesTypedPairScore(atom::SmoothingFunction *f)
      : smoothing_function_(f),
        repulsive_weight_(1.0),
        attractive_weight_(1.0) {
    params_ = internal::get_lj_params();
  }

  void set_repulsive_weight(double repulsive_weight) {
    repulsive_weight_ = repulsive_weight;
  }

  double get_repulsive_weight() const { return repulsive_weight_; }

  void set_attractive_weight(double attractive_weight) {
    attractive_weight_ = attractive_weight;
  }

  double get_attractive_weight() const { return attractive_weight_; }

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_PAIR_SCORE_METHODS(LennardJonesTypedPairScore);
  IMP_OBJECT_METHODS(LennardJonesTypedPairScore);
  ;
};

IMP_OBJECTS(LennardJonesTypedPairScore, LennardJonesTypedPairScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H */
