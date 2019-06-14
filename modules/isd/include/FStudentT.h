/**
 *  \file IMP/isd/FStudentT.h
 *  \brief Joint Student's t-distribution of Function
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_F_STUDENT_T_H
#define IMPISD_F_STUDENT_T_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/distribution.h>

IMPISD_BEGIN_NAMESPACE

//! Joint Student's t-distribution of Function.
/** If a vector of values \f$F(X)\f$ is jointly distributed according to the t
    distribution with shared center \f$F(M)\f$, scale \f$\sigma\f$, and degrees
    of freedom \f$\nu\f$, then the vector \f$X\f$ is distributed according to
    the F-t distribution, where \f$F\f$ is a one-to-one (strictly monotonic)
    function.

    For a vector \f$X=\{x_1,...,x_N\}\f$ containing \f$N\f$ observations, the
    joint F-t density is given by
    \f[
      p(X \mid M, \sigma, \nu) = \frac{
        \Gamma[(N + \nu) / 2]}{\Gamma[\nu / 2]}
        (\pi \nu)^{-N / 2} \sigma^{-N} J(X)
        \left[1 + \frac{t(X, M, \sigma)^2}{\nu} \right]^{-(N + \nu)/2},
    \f]

    where
    \f[
      J(X) = \left| \prod_{i=1}^N F'(x_i) \right|,
    \f]
    is a normalization factor according to the change of variables technique. 

    \f[
      t(X, M, \sigma)^2 = \frac{S_2(X) - 2 F(M) S_1(X) + N F(M)^2}{\sigma^2},
    \f]

    and the minimally sufficient statistics are
    \f{align*}{
      S_1(X) &= \sum_{i=1}^N F(x_i) \\
      S_2(X) &= \sum_{i=1}^N F(x_i)^2.
    \f}

    The degrees of freedom \f$\nu\f$ controls the heaviness of the tails. When
    \f$\nu = 1\f$, the F-t distribution becomes the F-Cauchy distribution. For
    very large \f$\nu\f$, the F-t distribution approaches the F-Normal
    distribution. Set \f$J(X)\f$ values to 1 for the joint t distribution.

    \note The joint t distribution is derived by marginalizing a joint normal
          distribution with the scaled inverse chi-square distribution as a
          prior on the variance. The same distribution is not produced by
          multiplying t distributions.
          For more details, see Gelmen et al. Bayesian Data Analaysis. 3rd edition.

    \see FNormal
 */
class IMPISDEXPORT FStudentT : public OneDimensionalSufficientDistribution {
  protected:
    virtual void do_update_sufficient_statistics(Floats FXs) IMP_OVERRIDE;
    virtual void do_update_sufficient_statistics(Floats FXs, Floats JXs);
    virtual Floats do_get_sufficient_statistics() const IMP_OVERRIDE;
    virtual double do_evaluate() const IMP_OVERRIDE;

  public:
    //! Create from observations vectors.
    /** \param [in] FXs Vector of N \f$F(x)\f$ observations drawn
                        independently from the same F-t distribution.
        \param [in] JXs Vector of N derivatives of \f$F(x)\f$ with respect to
                        \f$X\f$ vector.
        \param [in] FM Center of F-Student t with respect to \f$F(x)\f$.
        \param [in] sigma Scale of F-Student t with respect to \f$F(x)\f$.
        \param [in] nu Degrees of freedom of Student t distribution.
        \param [in] name Name.
     */
    FStudentT(Floats FXs, Floats JXs, double FM, double sigma, double nu,
              std::string name = "FStudentT %1%");

    //! Create from sufficient statistics.
    /** \param [in] sumFX Sum of observations of \f$F(x)\f$.
        \param [in] sumFX2 Sum of observations of \f$F(x)^2\f$.
        \param [in] N Number of observations.
        \param [in] LogJX Log of \f$J(X)\f$.
        \param [in] FM Center of F-t with respect to \f$F(x)\f$.
        \param [in] sigma Scale of F-t with respect to \f$F(x)\f$.
        \param [in] nu Degrees of freedom of Student t distribution.
        \param [in] name Name.
     */
    FStudentT(double sumFX, double sumFX2, unsigned N, double LogJX,
              double FM, double sigma, double nu,
              std::string name = "FStudentT %1%");

    //! Update sufficient statistics with values and derivatives.
    void update_sufficient_statistics(Floats FXs, Floats JXs) {
      do_update_sufficient_statistics(FXs, JXs);
    }

    //! Evaluate derivative of negative log-density wrt single \f$F(x_i)\f$.
    /** \f$F(x_i)\f$ represents an element of \f$F(X)\f$. Since only the
        sufficient statistics are stored, this value must be provided.
     */
    virtual double evaluate_derivative_Fx(double Fx) const;

    //! Evaluate derivative of negative log-density wrt elements of \f$F(X)\f$.
    /** This is equivalent to though faster than running
        evaluate_derivative_Fx() on every element of \f$F(X)\f$.
    */
    Floats evaluate_derivative_FX(const Floats FXs) const;

    //! Evaluate derivative of negative log-density wrt \f$\log J(X)\f$.
    virtual double evaluate_derivative_LogJX() const;

    //! Evaluate derivative of negative log-density wrt \f$F(M)\f$.
    virtual double evaluate_derivative_FM() const;

    //! Evaluate derivative of negative log-density wrt \f$\sigma\f$.
    virtual double evaluate_derivative_sigma() const;

    //! Evaluate derivative of negative log-density wrt \f$\nu\f$.
    virtual double evaluate_derivative_nu() const;

    //! Update cached intermediate values.
    /** This method is automatically called during evaluation. If evaluating
        derivatives after modifying parameters but before evaluation, this
        method must be manually called.
     */
    void update_cached_values() const;

    void set_sumFX(double v) { sumFX_ = v; }
    void set_sumFX2(double v) { sumFX2_ = v; }
    void set_N(unsigned v) { N_ = v; }
    void set_LogJX(double v) { LogJX_ = v; }
    void set_FM(double v) { FM_ = v; }
    void set_sigma(double v) { sigma_ = v; }
    void set_nu(double v) { nu_ = v; }

    //! Get sufficient statistic for passed \f$J(X)\f$ values.
    double get_sumFX() { return sumFX_; }
    double get_sumFX2() { return sumFX2_; }
    unsigned get_N() { return N_; }
    double get_LogJX() { return LogJX_; }
    double get_FM() { return FM_; }
    double get_sigma() { return sigma_; }
    double get_nu() { return nu_; }

  IMP_OBJECT_METHODS(FStudentT);

  private:
    double sumFX_, sumFX2_, LogJX_, FM_, sigma_, nu_;
    unsigned N_;
    mutable double t2_; // square of t-statistic
};
IMP_OBJECTS(FStudentT, FStudentTs);

IMPISD_END_NAMESPACE

#endif /* IMPISD_F_STUDENT_T_H */
