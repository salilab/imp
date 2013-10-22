/**
 *  \file IMP/isd/vonMisesSufficient.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_VON_MISES_SUFFICIENT_H
#define IMPISD_VON_MISES_SUFFICIENT_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/kernel/Model.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/math/special_functions/bessel.hpp>

IMPISD_BEGIN_NAMESPACE

//! vonMisesSufficient
/** Probability density function and -log(p) of von Mises distribution
    of N iid von Mises observations, provided through their sufficient
    statistics.
    This is much more efficient than multiplying N von Mises densities.
    \f[ f(\chi|N, R_0, \chi_{exp}, \kappa) =
       \frac{\exp \left(R_0 \kappa \cos (\chi - \chi_{exp})\right)}
       {2\pi I_0(\kappa)^N} \f]
    where
    \f[ R = \sqrt{\left(\sum_{i=1}^N \cos \chi_{exp}^i\right)^2
                  + \left(\sum_{i=1}^N \cos \chi_{exp}^i\right)^2} \f]
    \f[ \exp (i \chi_{exp}) = \frac{1}{R} \sum_{j=1}^N \exp(i \chi_{exp}^j) \f]
    If \f$N=1\f$ and \f$\mu_1=\mu_2\f$ this reduces to the original von Mises
    distribution with known mean and concentration.
    \note derivative with respect to the mean \f$\chi_{exp}\f$ is not provided.
 */

class vonMisesSufficient : public base::Object
{
 public:
  /** compute von Mises given the sufficient statistics
    \param[in] chi
    \param[in] N number of observations
    \param[in] R0 component of N observations on the x axis (\f$R_0\f$)
    \param[in] chiexp mean (\f$\chi_{exp}\f$)
    \param[in] kappa concentration
  */
  vonMisesSufficient(double chi, unsigned N, double R0, double chiexp,
                     double kappa):
  base::Object("von Mises sufficient %1%"), x_(chi), R0_(R0), chiexp_(chiexp)

    {
        N_=N;
        force_set_kappa(kappa);
    }

  /** compute von Mises given the raw observations
   * this is equivalent to calling get_sufficient_statistics and then the other
   * constructor.
    \param[in] chi
    \param[in] obs a list of observed angles (in radians).
    \param[in] kappa concentration
  */
  vonMisesSufficient(double chi, Floats obs, double kappa) :
  base::Object("von Mises sufficient %1%"), x_(chi)
    {
        Floats stats = get_sufficient_statistics(obs);
        N_= (unsigned) stats[0];
        R0_ = stats[1];
        chiexp_ = stats[2];
        force_set_kappa(kappa);
    }

  /* energy (score) functions, aka -log(p) */
  virtual double evaluate() const
  {
      return logterm_ - R0_*kappa_*cos(x_-chiexp_);
  }

  virtual double evaluate_derivative_x() const
  {
      return R0_*kappa_*sin(x_-chiexp_) ;
  }

  virtual double evaluate_derivative_kappa() const
  {
     return - R0_ * cos(x_-chiexp_) + double(N_) * I1_/I0_ ;
  }

  /* probability density function */
  virtual double density() const
  {
      return exp(R0_*kappa_*cos(x_-chiexp_))/(2*IMP::PI*I0N_);
  }

  /* getting parameters */
  double get_x() { return x_; }
  double get_R0() { return R0_; }
  double get_chiexp() { return chiexp_; }
  double get_N() { return N_; }
  double get_kappa() { return kappa_; }

  /* change of parameters */
  void set_x(double x) {
    x_=x;
  }

  void set_R0(double R0) {
    R0_=R0;
  }

  void set_chiexp(double chiexp) {
    chiexp_=chiexp;
  }

  void set_N(unsigned N){
    N_=N;
    I0N_=pow(I0_, static_cast<int>(N_));
    logterm_ = log(2*IMP::PI*I0N_);
  }

  void set_kappa(double kappa) {
    if (kappa_ != kappa) {
      force_set_kappa(kappa);
    }
  }

  //! Compute sufficient statistics from a list of observations.
  /** See Mardia and El-Atoum, "Bayesian inference for the von Mises-Fisher
      distribution ", Biometrika, 1967.
      \return the number of observations, \f$R_0\f$ (the component on the
              x axis) and \f$\chi_{exp}\f$
   */
  static Floats get_sufficient_statistics(Floats data)
    {
        unsigned N = data.size();
        //mean cosine
        double cosbar=0;
        double sinbar=0;
        for (unsigned i=0; i<N; ++i){
            cosbar += cos(data[i]);
            sinbar += sin(data[i]);
        }
        double R=sqrt(cosbar*cosbar + sinbar*sinbar);
        double chi=acos(cosbar/R);
        if (sinbar < 0) chi=-chi;
        Floats retval (3);
        retval[0]=N;
        retval[1]=R;
        retval[2]=chi;
        return retval;
    }

  IMP_OBJECT_METHODS(vonMisesSufficient);
  /*IMP_OBJECT_INLINE(vonMisesSufficient, out << "vonMisesSufficient: " << x_
                    << ", " << N_ << ", " << R0_ << ", " << chiexp_
                    << ", " << kappa_  <<std::endl, {});*/

 private:
  double x_,R0_,chiexp_,kappa_,I0_,I1_,logterm_,I0N_;
  unsigned N_;
  void force_set_kappa(double kappa) {
    kappa_ = kappa;
    I0_ = double(boost::math::cyl_bessel_i(0, kappa));
    I1_ = double(boost::math::cyl_bessel_i(1, kappa));
    I0N_=pow(I0_, static_cast<int>(N_));
    logterm_ = log(2*IMP::PI*I0N_);
  }
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_VON_MISES_SUFFICIENT_H */
