/**
 *  \file IMP/isd/PenalizedComplexityPrior.h
 *  \brief Penalized complexity prior.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_PENALIZED_COMPLEXITY_PRIOR_H
#define IMPISD_PENALIZED_COMPLEXITY_PRIOR_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/distribution.h>

IMPISD_BEGIN_NAMESPACE
//! Penalized complexity prior.
/** Given a parameter \f$\xi\f$ of a model (distribution)
    \f$f(x \mid \xi)\f$ with a simpler base model \f$g(x)\f$
    (\f$f(x \mid \xi=0)\f$ if \f$\xi\f$ is a flexibility parameter), the
    Penalized Complexity (PC) prior on \f$\xi\f$ minimizes the complexity
    introduced by deviation from the base model. It corresponds to an
    exponential distribution on the Kullback–Leibler divergence between the
    flexible and base models, in terms of \f$\xi\f$.

    For ease of parameterization, a 'tail event' is defined as an upper bound
    on an interpretable transformation \f$Q(\xi)\f$ with tail density
    \f$\alpha\f$, such that \f$P(Q(\xi) > U) = \alpha\f$. With KL divergence
    defined as

    \f[ \mathrm{KLD}(f(x \mid \xi) || g(x)) =
            \int f(x \mid \xi) \log\left(\frac{f(x \mid \xi)}{g(x)}\right) dx,\f]

    KL divergence at \f$\xi\f$ normalized by KL divergence at the tail

    \f[ D(\xi) = \frac{\mathrm{KLD}(f(x \mid \xi) || g(x))}{\mathrm{KLD}(f(x \mid \xi=Q^{-1}(U)) || g(x))} ,\f]

    and

    \f[ J(\xi) = \frac{\partial D(\xi)}{\partial \xi} ,\f]

    the density of the PC prior is

    \f[p(\xi) = \frac{-\log\alpha}{2 \sqrt{D(\xi)}} \alpha^{\sqrt{D(\xi)}} |J(\xi)|.\f]

    \see NormalSigmaPCRestraint

    \note For more details, see Simpson et al. Stat Sci. (2017) 32(1): 1. arXiv: 1403.4630
    \note The log-density with this parameterization overflows when
          \f$D(\xi)=0\f$. The value of the PC prior at the base model is
          unique to each case and cannot be provided by this general
          implementation.
 */
class IMPISDEXPORT PenalizedComplexityPrior : public OneDimensionalSufficientDistribution {
  protected:
    virtual void do_update_sufficient_statistics(Floats Dxis) IMP_OVERRIDE;
    virtual void do_update_sufficient_statistics(Floats Dxis, Floats Jxis);
    virtual Floats do_get_sufficient_statistics() const IMP_OVERRIDE;
    virtual double do_evaluate() const IMP_OVERRIDE;

  public:
    //! Create from normalized KL divergence.
    /** \param [in] Dxi Normalized KL divergence \f$D(\xi)\f$.
        \param [in] Jxi Derivative \f$J(\xi)\f$ of normalized KL divergence.
        \param [in] alpha Density in tail.
        \param [in] name Name of prior.
     */
    PenalizedComplexityPrior(double Dxi, double Jxi, double alpha,
                             std::string name = "PenalizedComplexityPrior %1%");

    //! Update sufficient statistics with values and derivatives.
    void update_sufficient_statistics(Floats Dxis, Floats Jxis) {
      do_update_sufficient_statistics(Dxis, Jxis);
    }

    //! Evaluate derivative of negative log-density wrt \f$D(\xi)\f$.
    double evaluate_derivative_Dxi() const;

    //! Evaluate derivative of negative log-density wrt derivative of \f$D(\xi)\f$.
    double evaluate_derivative_Jxi() const;

    //! Evaluate derivative of negative log-density wrt alpha.
    double evaluate_derivative_alpha() const;

    //! Evaluate derivative of negative log-density wrt parameter \f$\xi\f$.
    /** \param [in] JJxi Second derivative of normalized KL divergence wrt \f$\xi\f$.
     */
    double evaluate_derivative_xi(double JJxi) const;

    //! Set normalized KL divergence \f$D(\xi)\f$.
    void set_Dxi(double v);

    //! Set derivative of normalized KL divergence \f$D(\xi)\f$ wrt \f$\xi\f$.
    void set_Jxi(double v);

    //! Set density in tail.
    void set_alpha(double v);

    //! Get normalized KL divergence \f$D(\xi)\f$.
    double get_Dxi() const;

    //! Get derivative of normalized KL divergence \f$D(\xi)\f$ wrt \f$\xi\f$.
    double get_Jxi() const;

    //! Get density in tail.
    double get_alpha() const;

  IMP_OBJECT_METHODS(PenalizedComplexityPrior);

  private:
    double sqrtDxi_, Jxi_, nloga_;
};
IMP_OBJECTS(PenalizedComplexityPrior, PenalizedComplexityPriors);

IMPISD_END_NAMESPACE

#endif /* IMPISD_PENALIZED_COMPLEXITY_PRIOR_H */
