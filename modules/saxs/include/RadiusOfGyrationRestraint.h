/**
 *  \file IMP/saxs/RadiusOfGyrationRestraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H
#define IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H

#include <IMP/saxs/saxs_config.h>

#include <IMP/saxs/Profile.h>

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>

IMPSAXS_BEGIN_NAMESPACE

//! Calculate score based on radius of gyration, taken from saxs profile
/** \ingroup exp_restraint

 */
class IMPSAXSEXPORT RadiusOfGyrationRestraint : public IMP::Restraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles participating in the fitting score
     \param[in] exp_profile  The experimental profile used in the fitting score
     \param[in] end_q_rg The range of profile used for approximation:
      i.e. q*rg < end_q_rg. Use 1.3 for globular proteins, 0.8 for elongated
  */
  RadiusOfGyrationRestraint(const Particles& particles,
          const Profile* exp_profile, const double end_q_rg=1.3);

  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RadiusOfGyrationRestraint);

 private:
  Particles particles_; // non-rigid bodies particles
  double exp_rg_; //radius of gyration from experimental profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H */
