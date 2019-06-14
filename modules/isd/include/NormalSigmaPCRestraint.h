/**
 *  \file IMP/isd/NormalSigmaPCRestraint.h
 *  \brief A Penalized Complexity prior on sigma of a normal distribution.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_NORMAL_SIGMA_PC_RESTRAINT_H
#define IMPISD_NORMAL_SIGMA_PC_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/PenalizedComplexityPrior.h>
#include <IMP/isd/Scale.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! A Penalized Complexity prior on sigma of a normal distribution.
/** The Penalized Complexity (PC) prior on sigma minimizes the complexity (and
    resulting risk of over-fitting) introduced by deviation from a delta
    function (\f$\sigma=0\f$) to a normal distribution. Its density is

    \f[ p(\sigma) = \frac{-\log\alpha}{\sigma_u} \alpha^{\sigma / \sigma_u},\f]

    where \f$\sigma_u\f$ is a user-specified value of \f$\sigma\f$ that
    indicates a 'tail event', a value that it is unexpected to exceed, and
    \f$\alpha = p(\sigma > \sigma_u)\f$ is the density in the tail.

    \see PenalizedComplexityPrior

    \note The prior is related by a change of variables to the PC prior on the
          precision tau in Simpson et al. Stat Sci. (2017) 32(1): 1. arXiv: 1403.4630,
          which is a type 2 Gumbel distribution.
 */
class IMPISDEXPORT NormalSigmaPCRestraint : public Restraint {

  private:
    Pointer<PenalizedComplexityPrior> pcp_;
    ParticleIndex spi_;
    double su_, nloga_;

    void create_distribution(double alpha);

  public:
    NormalSigmaPCRestraint(Model *m, ParticleIndex spi, double su, double alpha,
                           std::string name = "NormalSigmaPCRestraint%1%");

    NormalSigmaPCRestraint(Scale s, double su, double alpha,
                           std::string name = "NormalSigmaPCRestraint%1%");

    Scale get_sigma() const;

    double get_alpha() const { return std::exp(-nloga_); }

    double get_sigma_upper() const { return su_; }

    double get_probability() const { return std::exp(-unprotected_evaluate(nullptr)); }

    virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
        const IMP_OVERRIDE;

    virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

    IMP_OBJECT_METHODS(NormalSigmaPCRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_NORMAL_SIGMA_PC_RESTRAINT_H */
