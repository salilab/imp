/**
 * \file IMP/atom/CAAngleRestraint.h
 * \brief Angle restraint between three residues in CA-only representation.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CA_ANGLE_RESTRAINT_H
#define IMPATOM_CA_ANGLE_RESTRAINT_H

#include "atom_config.h"

#include <IMP/kernel/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/generic.h>
#include <map>

IMPATOM_BEGIN_NAMESPACE

//! Angle restraint between three residues in CA-only representation
/** This restraint is designed to be used in conjunction with a CA-only
    representation; each restraint is applied to three CA atoms that
    correspond to three residues that are adjacent in sequence.
    A simple statistical potential is applied to score the angle.
    Currently, no splining or other smoothing is done and no derivatives
    are calculated.
 */
class IMPATOMEXPORT CAAngleRestraint : public kernel::Restraint
{
public:
  //! Create the angle restraint.
  /** The value of the restraint is simply score[n] where phi0[n]
      is the closest angle in phi0 to that between p1-p2-p3.
      \param[in] p1 First CA atom (in previous residue)
      \param[in] p2 Second CA atom (in current residue)
      \param[in] p3 Third CA atom (in next residue)
      \param[in] phi0 List of angles for which scores are available
      \param[in] score List of scores that correspond to phi0
   */
  CAAngleRestraint(Particle* p1, Particle* p2, Particle* p3,
                   Floats phi0,  Floats score);

  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CAAngleRestraint);

private:
  base::PointerMember<Particle> p_[3];
  Floats phi0_;
  Floats score_;

  double get_distance(double v0, double v1) const;
  int get_closest(std::vector<double> const& vec, double value) const;

};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CA_ANGLE_RESTRAINT_H */
