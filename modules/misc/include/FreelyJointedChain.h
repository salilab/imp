/**
 *  \file IMP/misc/FreelyJointedChain.h
 *  \brief Score on end-to-end distance of freely jointed chain
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_FREELY_JOINTED_CHAIN_H
#define IMPMISC_FREELY_JOINTED_CHAIN_H

#include <IMP/misc/misc_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/constants.h>

IMPMISC_BEGIN_NAMESPACE

//! Score on end-to-end distance of freely jointed chain
/** A flexible linker can be represented by the freely jointed chain model as a
    polymer under a Gaussian random walk. The end-to-end distance \f$ z \f$ then
    follows the distribution
        \f[p(z | \beta) = 4 \pi z^2 (\frac{\beta}{\pi})^{3/2)} \exp{(-\beta z^2)},\f]
    where \f$ \beta = \frac{3}{2 N b^2} \f$, \f$ N \f$ is the number of links,
    and \f$ b \f$ is the average length of a single link. The score reaches its
    minimum value when \f$ z = \frac{1}{\sqrt{\beta}} \f$.

    See KA Dill, S Bromberg. Molecular Driving Forces. 2nd Edition. 2010.
    Eqs 33.26, 33.27.

    \note While the Gaussian approximation breaks down when \f$z > N b\f$, the
    score naturally increases as a harmonic restraint with force constant
    \f$2 \beta\f$.

    \note The resulting score blows up as the \f$ z \f$ approaches 0.
    Therefore, in this implementation, when \f$ z = .01 z_{min} \f$, where
    \f$ z_{min} \f$ is the value of \f$ z \f$ where the score is minimized,
    then the score increases linearly as \f$ z \f$ decreases.

    \see WormLikeChain
    \see Harmonic
*/
class FreelyJointedChain : public UnaryFunction {

 private:
  void initialize() {
    double N = (double) N_;
    beta_ = 3. / 2. / N / b_ / b_;
    z_min_ = 0.01 / sqrt(beta_);
    z_min_deriv_ = -199.98 * sqrt(beta_);
    z_min_int_ = 10.39631095 - std::log(beta_) / 2.;
  }

 public:
  FreelyJointedChain(int link_num, double link_length)
    : N_(link_num), b_(link_length) {
      IMP_USAGE_CHECK(N_ > 0, "Number of links must be positive.");
      IMP_USAGE_CHECK(b_ > 0, "Link length must be positive.");
      initialize();
    }

  virtual DerivativePair evaluate_with_derivative(double feature) const {
    if (feature < 0) feature = 0;
    if (feature < z_min_) {
      return DerivativePair(z_min_deriv_ * feature + z_min_int_,
                            z_min_deriv_);
    } else {
      return DerivativePair(beta_ * feature * feature -
                            2 * std::log(2 * feature) +
                            std::log(PI / std::pow(beta_, 3)) / 2.,
                            2. * beta_ * feature - 2.0 / feature);
    }
  }

  virtual double evaluate(double feature) const {
    return evaluate_with_derivative(feature).first;
  }

  IMP_OBJECT_METHODS(FreelyJointedChain);

  //! Get number of links in chain.
  int get_link_number() const { return N_; }

  //! Get length of each chain link.
  double get_link_length() const { return b_; }

  //! Set the number of links in chain.
  void set_link_number(int N) {
    N_ = N;
    IMP_USAGE_CHECK(N_ > 0, "Number of links must be positive.");
    initialize();
  }

  //! Set the length of each chain link.
  void set_link_length(double b) {
    b_ = b;
    IMP_USAGE_CHECK(b_ > 0, "Link length must be positive.");
    initialize();
  }

  //! Get the distance at which the score is at its minimum value.
  double get_distance_at_minimum() const {
    return 1. / sqrt(beta_);
  }

  //! Get the average distance from the surface.
  double get_average_distance() const {
    return 2. / sqrt(PI * beta_);
  }

 private:
  int N_;
  double b_;
  double beta_, z_min_, z_min_deriv_, z_min_int_;
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_FREELY_JOINTED_CHAIN_H */
