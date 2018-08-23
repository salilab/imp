/**
 *  \file IMP/isd/TransformedNuisance.h
 *  \brief Transform a bounded Nuisance for sampling in an unconstrained space.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_TRANSFORMED_NUISANCE_H
#define IMPISD_TRANSFORMED_NUISANCE_H

#include "isd_config.h"
#include <IMP/isd/Nuisance.h>
#include <IMP/Object.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Store types of nuisance transformations.
enum NuisanceTransformationType {
  NONE,
  LOG_LOWER,
  LOG_UPPER,
  LOGIT_LOWER_UPPER
};

//! Transform a bounded Nuisance for sampling in an unconstrained space.
/** The bounds must be non-equal. They are enforced by maintaining an
    transformation \f$y=f(x)\f$ of the nuisance \f$x\f$ to an unconstrained
    space. To reduce sampling bias, the bounds on \f$x\f$ are enforced by only
    directly setting and evolving \f$y\f$.

    The transformations, inverse transformations, and Jacobian of the
    transformations are described below.

    For an unbounded nuisance \f$x \in \mathbb{R}\f$:

    \f[ f(x) = x \
        f^{-1}(y) = y \
        \frac{d}{dy} f^{-1}(y) = 1
    \f]

    For a lower-bounded nuisance \f$x > a\f$:

    \f[ f(x) = \log(x - a) \
        f^{-1}(y) = a + e^y \
        \frac{d}{dy} f^{-1}(y) = e^y
    \f]

    For an upper-bounded nuisance \f$x < b\f$:

    \f[ f(x) = \log(b - x) \
        f^{-1}(y) = b - e^y \
        \frac{d}{dy} f^{-1}(y) = -e^y
    \f]

    For a lower- and upper-bounded nuisance \f$a < x < b\f$:

    \f[ f(x) = \text{logit}\left(\frac{x - a}{b - a}\right)
             = \log\left(\frac{x - a}{b - x}\right) \
        f^{-1}(y) = a + \frac{b - a}{1 + e^{-y}} \
        \frac{d}{dy} f^{-1}(y) = \frac{(b - a) e^{-y}}{(1 + e^{-y})^2}
    \f]

    \note Defining the nuisance in terms of the transformed nuisance requires
          a change of variables on all prior distributions. This is handled by
          adjusting the score and its gradient. This is handled for the user.
    \note Bounds should only be used when they reflect true
          physical/mathematical constraints on the nuisance (e.g. Scale).
          Prior information regarding realistic values of the nuisance should
          be expressed in a prior restraint, *not* in the use of bounds.

    \see Nuisance
    \see Scale
    \see Switching
 */
class IMPISDEXPORT TransformedNuisance : public Nuisance {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                double nuisance = 1.0);

 public:
  IMP_DECORATOR_METHODS(TransformedNuisance, Nuisance);
  IMP_DECORATOR_SETUP_0(TransformedNuisance);
  IMP_DECORATOR_SETUP_1(TransformedNuisance, double, nuisance);

  static bool get_is_setup(Model *m, ParticleIndex pi);

  //! Get type of transformation applied to nuisance.
  int get_transformation_type() const {
    return get_has_lower() + 2 * get_has_upper();
  }

  //! Set value of transformed nuisance from untransformed nuisance.
  /** Automatically triggers update of cached values. */
  void set_from_nuisance(Float x);

  //! Get key for the transformed nuisance.
  static FloatKey get_transformed_nuisance_key();

  //! Get value of transformed nuisance.
  Float get_transformed_nuisance() const {
    return get_particle()->get_value(get_transformed_nuisance_key());
  }

  //! Set value of transformed nuisance.
  void set_transformed_nuisance(Float y);

  Float get_transformed_nuisance_derivative() const {
    return get_particle()->get_derivative(get_transformed_nuisance_key());
  }

  void add_to_transformed_nuisance_derivative(Float d,
                                              DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_transformed_nuisance_key(), d, accum);
  }

  bool get_transformed_nuisance_is_optimized() const {
    return get_particle()->get_is_optimized(get_transformed_nuisance_key());
  }

  void set_transformed_nuisance_is_optimized(bool val) {
    if (val) {
      IMP_USAGE_CHECK(!get_nuisance_is_optimized(),
                      "Nuisance and TransformedNuisance cannot be "
                      "simultaneously optimized.");
    }
    get_particle()->set_is_optimized(get_transformed_nuisance_key(), val);
  }

  void set_lower(Float d);
  void set_lower(Particle *d);
  void remove_lower();

  void set_upper(Float d);
  void set_upper(Particle *d);
  void remove_upper();

  //! Get Jacobian of the transformation.
  /** The value is \f$\frac{df^{-1}(y)}{dy} \f$ .*/
  double get_jacobian() const;

  //! Get negative log of the absolute value of the Jacobian of the
  //! transformation.
  /** The value is \f$-\log \left| \frac{df^{-1}(y)}{dy} \right| \f$. It must
      be added to the score of restraints that represent a probability
      distribution on the TransformedNuisance, due to change of variables.
   */
  double get_score_adjustment() const;

  //! Get derivative of negative log of the absolute value of the Jacobian of
  //! the transformation.
  /** The value is \f$ \frac{d}{dy} -\log \left| \frac{df^{-1}(y)}{dy} \right|
     \f$. A score state is used to add it to the derivative of the transformed
      nuisance to account for change of variables.
   */
  double get_gradient_adjustment() const;

  friend class TransformedNuisanceScoreState;

  void update_nuisance() const;
  void update_derivative();

 private:
  static ObjectKey get_ss_key();
  void setup_score_state();
  void remove_score_state();
};

IMP_DECORATORS(TransformedNuisance, TransformedNuisances, Nuisances);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPISDEXPORT TransformedNuisanceScoreState : public ScoreState {
 private:
  IMP::WeakPointer<Particle> p_;

 private:
  TransformedNuisanceScoreState(Particle *p)
      : ScoreState(p->get_model(), "TransformedNuisanceScoreState%1%"), p_(p) {}

 public:
  friend class TransformedNuisance;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TransformedNuisanceScoreState);
};
#endif

IMPISD_END_NAMESPACE

#endif /* IMPISD_TRANSFORMED_NUISANCE_H */
