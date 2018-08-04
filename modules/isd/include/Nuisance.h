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
/** The value of the nuisance parameter may express data or theory uncertainty.
    It can be initialized with or without specifying its value. Default is 1.
    On construction, the Nuisance is unbounded. It can be bounded with
    set_upper and set_lower.

    The bounds must be non-equal. They are enforced by maintaining an
    unconstrained transformation \f$y=f(x)\f$ of the nuisance \f$x\f$. To
    reduce sampling bias, the bounds on \f$x\f$ are enforced by only directly
    setting and evolving \f$y\f$.

    The transformations, inverse transformations, and Jacobian of the
    transformations are described below.

    For an unbounded nuisance \f$x \in \mathbb{R}\f$:

    \f[ f(x) = x \\
        f^{-1}(y) = y \\
        \frac{d}{dy} f^{-1}(y) = 1
    \f]

    For a lower-bounded nuisance \f$x > a\f$:

    \f[ f(x) = \log(x - a) \\
        f^{-1}(y) = a + e^y \\
        \frac{d}{dy} f^{-1}(y) = e^y
    \f]

    For an upper-bounded nuisance \f$x < b\f$:

    \f[ f(x) = \log(b - x) \\
        f^{-1}(y) = b - e^y \\
        \frac{d}{dy} f^{-1}(y) = -e^y
    \f]

    For a lower- and upper-bounded nuisance \f$a < x < b\f$:

    \f[ f(x) = \text{logit}\left(\frac{x - a}{b - a}\right)
             = \log\left(\frac{x - a}{b - x}\right) \\
        f^{-1}(y) = a + \frac{b - a}{1 + e^{-y}} \\
        \frac{d}{dy} f^{-1}(y) = \frac{(b - a) e^{-y}}{(1 + e^{-y})^2}
    \f]

    \note Defining the nuisance in terms of the transformed nuisance requires
          a change of variables on all prior distributions, including the
          default uniform distribution. This is handled for the user by adding
          the negative log absolute value of the Jacobian of the inverse
          transformation to the score. A score state is used to add the
          derivative of this term to the transformed nuisance.
    \note Bounds should only be used when they reflect true
          physical/mathematical constraints on the nuisance (e.g. Scale).
          Prior information regarding realistic values of the nuisance should
          be expressed in a prior restraint, *not* in the use of bounds.
    \note Bounds may be a double, another Nuisance or both. If both are set,
          the tighter of the two bounds at a given evaluation is enforced.
          A change in the value of a particle bound changes the untransformed
          nuisance but not the transformed nuisance.

    \see Scale
    \see Switching
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

  //! Get whether nuisance has lower bound.
  bool get_has_lower() const;

  //! Get lower bound of nuisance.
  Float get_lower() const;

  //! Get key for lower bound attribute.
  static FloatKey get_lower_key();

  //! Get key for lower bound Nuisance attribute.
  static ParticleIndexKey get_lower_particle_key();

  //! Set lower bound to value.
  void set_lower(Float d);

  //! Set lower bound to Nuisance.
  void set_lower(Particle *d);

  //! Remove lower bound.
  void remove_lower();

  //! Get whether nuisance has upper bound.
  bool get_has_upper() const;

  //! Get upper bound of nuisance.
  Float get_upper() const;

  //! Get key for upper bound attribute.
  static FloatKey get_upper_key();

  //! Get key for upper bound Nuisance attribute.
  static ParticleIndexKey get_upper_particle_key();

  //! Set upper bound to value.
  void set_upper(Float d);

  //! Set upper bound to Nuisance.
  void set_upper(Particle *d);

  //! Remove upper bound.
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
  /** The value is \f$\frac{df^{-1}(y)}{dy} \f$ .*/
  double get_jacobian_of_inverse_transformation() const;

  //! Get negative log of the absolute value of the Jacobian of the transformation.
  /** The value is \f$-\log \left| \frac{df^{-1}(y)}{dy} \right| \f$. It must
      be added to the score of restraints that represent a probability
      distribution on the Nuisance, due to change of variables.
   */
  double get_negative_log_absolute_jacobian_of_inverse_transformation() const;

  //! Get derivative of negative log of the absolute value of the Jacobian of the transformation.
  /** The value is \f$ \frac{d}{dy} -\log \left| \frac{df^{-1}(y)}{dy} \right| \f$.
      A score state is used to add it to the derivative of the transformed
      nuisance to account for change of variables.
   */
  double get_derivative_of_negative_log_absolute_jacobian_of_inverse_transformation() const;

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
