/**
 * \file RangeSearchScoreState.h
 * \brief Search over ranges of values.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSEARCH_RANGE_SEARCH_SCORE_STATE_H
#define IMPSEARCH_RANGE_SEARCH_SCORE_STATE_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/ScoreState.h>
#include <IMP/SingletonContainer.h>

IMPSEARCH_BEGIN_NAMESPACE

//! Build a search structure which allows fast searches over rangesw.
/** \note This class is not implemented yet. It it checked in for comments.
    \note This class will require CGAL in order to be efficient.

    Currently all arguments are in the forms of vectors of values. This can be
    done since all values are the same type. And it is a good thing in this case
    since otherwise telling the lower bound and the upper bound appart is
    annoying (is it lb0, ub0, lb1, ub1 or lb0, lb1, ub0, ub1?).
 */
class RangeSearchScoreState:
public ScoreState
{
  Pointer<IMP::SingletonContainer> pc_;
  FloatKeys keys_;
public:

  //! Create a keys.size()-D search structure
  RangeSearchScoreState(IMP::SingletonContainer* pc,
                        const FloatKeys &keys);

  //! Get all particles whose attributes fall in the interval
  /** A list of particles is returned containing particle, p, in the
      passed SingletonContainer such that for all i in get_keys().size(),
      p->get_value(keys[i]) >= lb[i] && p->get_value(keys[i]) < ub[ub].

      \throws ValueException if ub.size() != lb.size() != get_keys().size()
   */
  Particles get_contained(const Floats &lb,
                          const Floats &ub);

  //! Return the keys being searched over
  const FloatKeys &get_keys() const {
    return keys_;
  }

  IMP_SCORE_STATE(RangeSearchScoreState, internal::version_info);
};

IMPSEARCH_END_NAMESPACE

#endif  /* IMPSEARCH_RANGE_SEARCH_SCORE_STATE_H */
