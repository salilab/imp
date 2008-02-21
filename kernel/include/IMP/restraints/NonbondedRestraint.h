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
#include "../internal/ObjectPointer.h"

namespace IMP
{

class NonbondedListScoreState;
class PairScore;

//! Restrain all pairs of non-bonded particles
/**
   \ingroup restraint
 */
class IMPDLLEXPORT NonbondedRestraint : public Restraint
{
public:
  //! Create the nonbonded restraint.
  /** \param[in] nbl The non-bonded list to use to get the list
      of particles.
      \param[in] ps The pair score function to apply to the pairs. This 
      object is deleted upon destruction.
      \param[in] max_dist Pairs beyond this distance may be dropped.
   */
  NonbondedRestraint(NonbondedListScoreState *nbl, PairScore *ps,
                     Float max_dist= std::numeric_limits<Float>::max());
  virtual ~NonbondedRestraint(){}

  IMP_RESTRAINT("0.5", "Daniel Russel");

protected:
  NonbondedListScoreState *nbl_;
  internal::ObjectPointer<PairScore, true> sf_;
  Float max_dist_;
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_RESTRAINT_H */
