/**
 *  \file IMP/isd/Nuisance.h
 *  \brief A decorator for nuisance parameters particles
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_NUISANCE_H
#define IMPISD_NUISANCE_H

#include "isd_config.h"

#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Store types of nuisance transformations.
enum TransformationType {
  NONE,
  LOG_LOWER,
  LOG_UPPER,
  LOGIT_LOWER_UPPER
};


//! Add nuisance parameter to particle
/** The value of the nuisance parameter may express data
    or theory uncertainty. It can be initialized with or without
    specifying its value. Default is 1. On construction, the Nuisance is
    unbounded. It can be bounded with set_upper and set_lower. Setting it
    to values outside of bounds results in setting it to the bound value.
 */
class IMPISDEXPORT Nuisance : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                double nuisance = 1.0);

 public:
  IMP_DECORATOR_METHODS(Nuisance, Decorator);
  IMP_DECORATOR_SETUP_0(Nuisance);
  IMP_DECORATOR_SETUP_1(Nuisance, double, nuisance);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_transformed_nuisance_key(), pi);
  }

  //! Get type of transformation applied to nuisance.
  int get_transformation_type() const {
    return get_has_lower() + 2 * get_has_upper();
  }

  //! Get key for the transformed nuisance.
  static FloatKey get_transformed_nuisance_key();

  //! Get key for the transformed nuisance.
  IMPISD_DEPRECATED_METHOD_DECL(2.10)
  static FloatKey get_nuisance_key() {
    IMPISD_DEPRECATED_METHOD_DEF(2.10, "Use get_transformed_nuisance_key() instead. See documentation.");
    return get_transformed_nuisance_key();
  }

  //! Get value of transformed nuisance.
  Float get_transformed_nuisance() const {
    return get_particle()->get_value(get_transformed_nuisance_key());
  }

  //! Get value of untransformed nuisance.
  Float get_nuisance() const;

  //! Set value of transformed nuisance.
  void set_transformed_nuisance(Float y);

  //! Set value of transformed nuisance from untransformed nuisance.
  void set_nuisance(Float x);

  /** set upper and lower bound of nuisance by specifying
   * either a float or another nuisance. Both can be set at the same
   * time in which case the upper bound is the minimum of the two values.
   * This constraint is enforced with the help of a ScoreState that will be
   * created on-the-fly.
   */
  bool get_has_lower() const;
  Float get_lower() const;
  static FloatKey get_lower_key();
  static ParticleIndexKey get_lower_particle_key();
  void set_lower(Float d);
  void set_lower(Particle *d);
  void remove_lower();

  bool get_has_upper() const;
  Float get_upper() const;
  static FloatKey get_upper_key();
  static ParticleIndexKey get_upper_particle_key();
  void set_upper(Float d);
  void set_upper(Particle *d);
  void remove_upper();

  Float get_transformed_nuisance_derivative() const {
    return get_particle()->get_derivative(get_transformed_nuisance_key());
  }

  void add_to_transformed_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_transformed_nuisance_key(), d, accum);
  }

  bool get_transformed_nuisance_is_optimized() const {
    return get_particle()->get_is_optimized(get_transformed_nuisance_key());
  }

  void set_transformed_nuisance_is_optimized(bool val) {
    get_particle()->set_is_optimized(get_transformed_nuisance_key(), val);
  }

  //! Get derivative of score wrt nuisance
  /** Get derivative of score wrt nuisance as though score was parameterized
      in terms of the nuisance and not the nuisance transformation.
      \note this is only guaranteed to be accurate after model update.
   */
  Float get_nuisance_derivative() const;

  //! Add to derivative of score wrt nuisance
  /** The derivative is automatically propagated to transformed nuisance with
      with the necessary modification.
   */
  void add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum);

  bool get_nuisance_is_optimized() const {
    return get_transformed_nuisance_is_optimized();
  }

  void set_nuisance_is_optimized(bool val) {
    set_transformed_nuisance_is_optimized(val);
  }

  //! Get Jacobian of the transformation.
  /** The value is  \f$\frac{df^{-1}(y)}{dy} \f$ .*/
  double get_jacobian_of_transformation() const;

  //! Get negative log of the absolute value of the Jacobian of the transformation.
  /** The value is \f$-\log \left| \frac{df^{-1}(y)}{dy} \right| \f$. It must
      be added to the score of restraints that represent a probability
      distribution on the Nuisance, due to change of variables.
   */
  double get_negative_log_absolute_jacobian_of_transformation() const;

  //! Get derivative of negative log of the absolute value of the Jacobian of the transformation.
  /** The value is \f$ \frac{d}{dy} -\log \left| \frac{df^{-1}(y)}{dy} \right| \f$.
      A score state is used to add it to the derivative of the transformed
      nuisance to account for change of variables.
   */
  double get_derivative_of_negative_log_absolute_jacobian_of_transformation() const;

  friend class NuisanceScoreState;

 private:
  // scorestate-related bookkeeping
  static ObjectKey get_ss_key();
  void setup_score_state();
  void remove_score_state();
};

IMP_DECORATORS(Nuisance, Nuisances, ParticlesTemp);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPISDEXPORT NuisanceScoreState : public ScoreState {
 private:
  IMP::WeakPointer<Particle> p_;

 private:
  NuisanceScoreState(Particle *p)
      : ScoreState(p->get_model(), "NuisanceScoreState%1%"), p_(p) {}

 public:
  friend class Nuisance;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(NuisanceScoreState);
};
#endif

IMPISD_END_NAMESPACE

#endif /* IMPISD_NUISANCE_H */
