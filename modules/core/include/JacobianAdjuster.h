/**
 *  \file IMP/core/JacobianAdjuster.h
 *  \brief Adjust score and gradients with Jacobian of parameter transformation.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_JACOBIAN_ADJUSTER_H
#define IMPCORE_JACOBIAN_ADJUSTER_H

#include <IMP/core/core_config.h>
#include <IMP/ModelObject.h>
#include <IMP/tuple_macros.h>
#include <IMP/set_map_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Store Jacobian factors of a univariate transformation
/** For a univariate transformation, \f$y = f(x)\f$, it stores the Jacobian
    of the inverse transformation \f$f^{-1}(y)\f$ and the necessary
    Jacobian-derived adjustment to the score and its gradient.

    \param[in] jacobian Jacobian value \f$J(y) = \frac{d}{dy}f^{-1}(y)\f$.
    \param[in] score_adjustment Adjustment to score due to transformation,
                                \f$-\log |J(y)|\f$.
    \param[in] gradient_adjustment Adjustment to gradient of score due to
                                   transformation,
                                   \f$-\frac{d}{dy} \log |J(y)|\f$.
 */
IMP_NAMED_TUPLE_3(UnivariateJacobian, UnivariateJacobians, Float, jacobian,
                  Float, score_adjustment, Float, gradient_adjustment,);

//! Store Jacobian factors of a multivariate transformation
/** For a multivariate transformation, \f$Y = f(X)\f$, it stores the Jacobian
    of the inverse transformation \f$f^{-1}(X)\f$ and the necessary
    Jacobian-derived adjustment to the score and its gradient.

    \param[in] jacobian Jacobian matrix \f$J(Y)\f$.
    \param[in] score_adjustment Adjustment to score due to transformation,
                                \f$-\log |\det J(y)|\f$.
    \param[in] gradient_adjustment Adjustment to gradient of score due to
                                   transformation,
                                   \f$-\nabla_Y \log |\det J(y)|\f$.
 */
IMP_NAMED_TUPLE_3(MultivariateJacobian, MultivariateJacobians,
                  FloatsList, jacobian, Float, score_adjustment, Floats,
                  gradient_adjustment,{
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    int N = gradient_adjustment_.size();
    IMP_USAGE_CHECK(N > 0, "Number of variables must be positive.");
    IMP_USAGE_CHECK(jacobian_.size() == N,
      "Jacobian number of rows must be equal to gradient adjustment size.");
    for (unsigned int n = 0; n < N; ++n) {
      IMP_USAGE_CHECK(jacobian_[n].size() == N, "Jacobian must be square.");
    }
  }
});

//! Adjust score and gradients with Jacobian of parameter transformation.
/** It stores UnivariateJacobians and MultivariateJacobians as well as the
    particle and float attributes to which they are related.

    It computes the total necessary Jacobian adjustment to the score due to
    transformation with get_score_adjustment(). It also adds the necessary
    Jacobian gradient modification to all derivatives with
    apply_gradient_adjustment().

    \note This class should not be created directly. Instead,
          get_jacobian_adjuster() should be used so that only one is
          associated with each module.
 */
class IMPCOREEXPORT JacobianAdjuster : public IMP::ModelObject {
  typedef IMP::FloatIndex FloatIndex;
  IMP_NAMED_TUPLE_2(FloatsIndex, FloatsIndexes, ParticleIndex, particle,
                    FloatKeys, keys, );
  typedef std::pair<FloatIndex, UnivariateJacobian> UP;
  typedef std::pair<FloatsIndex, MultivariateJacobian> MP;
  typedef IMP_KERNEL_SMALL_UNORDERED_MAP<FloatIndex, UnivariateJacobian>
      UnivariateJacobianMap;
  typedef IMP_KERNEL_SMALL_UNORDERED_MAP<FloatsIndex, MultivariateJacobian>
      MultivariateJacobianMap;
  UnivariateJacobianMap uni_map_;
  MultivariateJacobianMap multi_map_;

 public:
  JacobianAdjuster(Model* m, const std::string name = "JacobianAdjuster%1%");

  //! Set Jacobian for univariate transformation corresponding to attribute.
  /** \param[in] ks Attribute of transformed parameter.
      \param[in] pi Index of decorated particle.
      \param[in] j Container of Jacobian, score adjustment, and gradient
                   adjustment due to transformation.
   */
  void set_jacobian(FloatKey k, ParticleIndex pi, const UnivariateJacobian& j);

  //! Set Jacobian for multivariate transformation corresponding to attribute.
  /** \param[in] ks Attributes of transformed parameter.
      \param[in] pi Index of decorated particle.
      \param[in] j Container of Jacobian matrix, score adjustment, and gradient
                   adjustment due to transformation.
   */
  void set_jacobian(FloatKeys ks, ParticleIndex pi,
                    const MultivariateJacobian& j);

  //! Get stored Jacobian for univariate transformation.
  const UnivariateJacobian& get_jacobian(FloatKey k, ParticleIndex pi) const;

  //! Get stored Jacobian for multivariate transformation.
  const MultivariateJacobian& get_jacobian(FloatKeys ks,
                                           ParticleIndex pi) const;

#ifndef SWIG
  UnivariateJacobian& access_jacobian(FloatKey k, ParticleIndex pi);

  MultivariateJacobian& access_jacobian(FloatKeys ks, ParticleIndex pi);
#endif

  //! Get total adjustment to score from stored Jacobians.
  /** Only optimized attributes are considered. */
  double get_score_adjustment() const;

  //! Adjust derivatives of particles using stored Jacobians.
  /** Only derivatives of optimized attributes are adjusted. */
  void apply_gradient_adjustment();

  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;

 private:
  static ModelKey get_score_state_key();
  void create_score_state();

  IMP_OBJECT_METHODS(JacobianAdjuster);
};

IMP_OBJECTS(JacobianAdjuster, JacobianAdjusters);

//! Get adjuster of Model's Jacobian
/** If one does not yet exist, it is created. */
IMPCOREEXPORT JacobianAdjuster* get_jacobian_adjuster(Model* m);



#if !defined(IMP_DOXYGEN) && !defined(SWIG)
//! Adjust all transformed parameter gradients with JacobianAdjuster.
class IMPCOREEXPORT JacobianAdjustGradient : public ScoreState {
 public:
  JacobianAdjustGradient(Model *m)
      : ScoreState(m, "JacobianAdjustGradient%1%") {}
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(JacobianAdjustGradient);
};
#endif

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_JACOBIAN_ADJUSTER_H */
