/**
 *  \file NonbondedRestraint.h   
 *  \brief Apply a PairScore to all nonbonded pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_NONBONDED_RESTRAINT_H
#define __IMP_NONBONDED_RESTRAINT_H

#include <iostream>

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"
#include "../Pointer.h"
#include "../score_states/NonbondedListScoreState.h"

namespace IMP
{

class PairScore;

//! Apply a PairScore to all nonbonded pairs of particles
/**
   \ingroup restraint
 */
class IMPDLLEXPORT NonbondedRestraint : public Restraint
{
public:
  //! Create the nonbonded restraint.
  /** \param[in] ps The pair score function to apply to the pairs. This 
      object is deleted upon destruction.
      \param[in] nbl The non-bonded list to use to get the list
      of particles.
   */
  NonbondedRestraint(PairScore *ps, NonbondedListScoreState *nbl);
  virtual ~NonbondedRestraint(){}

  IMP_RESTRAINT(internal::kernel_version_info)

protected:
  Pointer<NonbondedListScoreState> nbl_;
  Pointer<PairScore> sf_;
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_RESTRAINT_H */
