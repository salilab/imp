/**
 *  \file IMP/container/MinimumClassnameScore.h
 *  \brief Define ClassnameScore.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_MINIMUM_CLASSNAME_SCORE_H
#define IMPCONTAINER_MINIMUM_CLASSNAME_SCORE_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameScore.h>
#include <IMP/classname_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Evaluate the min or max n FUNCTIONNAME scores of the passed set
/** Each of the set of ClassnameScores is evaluated and the sum of the
    minimum n is returned.
*/
class IMPCONTAINEREXPORT MinimumClassnameScore : public ClassnameScore {
  ClassnameScores scores_;
  unsigned int n_;

 public:
  MinimumClassnameScore(const ClassnameScoresTemp &scores, unsigned int n = 1,
                        std::string name = "ClassnameScore %1%");
  virtual double evaluate_index(Model *m, PASSINDEXTYPE vt,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_CLASSNAME_SCORE_METHODS(MinimumClassnameScore);
  IMP_OBJECT_METHODS(MinimumClassnameScore);

  Restraints do_create_current_decomposition(Model *m,
                                             PASSINDEXTYPE vt) const
      IMP_OVERRIDE;
};

IMP_OBJECTS(MinimumClassnameScore, MinimumClassnameScores);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_MINIMUM_CLASSNAME_SCORE_H */
