/**
 *  \file IMP/isd/StudentTRestraint.h
 *  \brief A Student-t distribution restraint
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_STUDENT_T_RESTRAINT_H
#define IMPISD_STUDENT_T_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/FStudentT.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! A Student-t distribution restraint
/** It restraints 1 to 3 particles, with the mean \f$\mu\f$, standard deviation
    \f$\sigma\f$ and degrees of freedom \f$\nu\f$ able to be constants or
    Nuisance particles.

    \see FStudentT
*/
class IMPISDEXPORT StudentTRestraint : public Restraint {
  private:
    Pointer<FStudentT> studentt_;
    ParticleIndex px_, pmu_, psigma_, pnu_;
    double x_, mu_, sigma_, nu_;
    bool isx_, ismu_, issigma_, isnu_;  // true if it's a particle

  private:
    double get_x() const;
    double get_mu() const;
    double get_sigma() const;
    double get_nu() const;
    void initialize();
    void update_distribution();

  public:
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, ParticleIndexAdaptor mu,
                      ParticleIndexAdaptor sigma, ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, double mu,
                      ParticleIndexAdaptor sigma, ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, ParticleIndexAdaptor mu,
                      double sigma, ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, ParticleIndexAdaptor mu,
                      ParticleIndexAdaptor sigma, double nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, double mu, double sigma,
                      ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, double mu,
                      ParticleIndexAdaptor sigma, double nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, ParticleIndexAdaptor mu,
                      double sigma, double nu);
    StudentTRestraint(Model *m, ParticleIndexAdaptor x, double mu, double sigma,
                      double nu);
    StudentTRestraint(Model *m, double x, ParticleIndexAdaptor mu,
                      ParticleIndexAdaptor sigma, ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, double x, double mu, ParticleIndexAdaptor sigma,
                      ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, double x, ParticleIndexAdaptor mu, double sigma,
                      ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, double x, ParticleIndexAdaptor mu,
                      ParticleIndexAdaptor sigma, double nu);
    StudentTRestraint(Model *m, double x, double mu, double sigma,
                      ParticleIndexAdaptor nu);
    StudentTRestraint(Model *m, double x, double mu, ParticleIndexAdaptor sigma,
                      double nu);
    StudentTRestraint(Model *m, double x, ParticleIndexAdaptor mu, double sigma,
                      double nu);

    //! Get probability (-log score)
    double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

    virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
        const IMP_OVERRIDE;

    virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(StudentTRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_STUDENT_T_RESTRAINT_H */
