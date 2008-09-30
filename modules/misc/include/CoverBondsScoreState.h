/**
 *  \file CoverBondsScoreState.h
 *  \brief Covers a set of bonds with spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPMISC_COVER_BONDS_SCORE_STATE_H
#define __IMPMISC_COVER_BONDS_SCORE_STATE_H

#include "IMP/ScoreState.h"
#include "IMP/Pointer.h"
#include "IMP/score_states/BondDecoratorListScoreState.h"
#include "misc_version_info.h"

namespace IMP
{

namespace misc
{

//! This class sets the position and radius of each bond to cover the endpoints.
/** For each bond in the list of particles set the x,y,z to be the
    average of that of the bonds endpoints and the radius to be the
    sqrt of half the distance.

    Note that the ends of the bond are treated as points so any radius
    they might have is ignored. This would be easy to change if
    desired.

    This is designed to be used in conjunction with a bonded list. Make sure
    it is updated first.
 */
class IMPMISCEXPORT CoverBondsScoreState: public ScoreState
{
  Pointer<BondDecoratorListScoreState> bl_;
  FloatKey rk_;
public:
  /** Get the list of bonds from the BondDecoratorListScoreState. This list is
      not owned and update is not called on this list automatically.
   */
  CoverBondsScoreState(BondDecoratorListScoreState *bl,
                       FloatKey rk=FloatKey("radius"));
  ~CoverBondsScoreState();

  IMP_SCORE_STATE(internal::misc_version_info);

  //! apply the derivatives to the endpoints
  void after_evaluate(DerivativeAccumulator *accpt);

};

} // namespace misc

} // namespace IMP

#endif  /* __IMPMISC_COVER_BONDS_SCORE_STATE_H */
