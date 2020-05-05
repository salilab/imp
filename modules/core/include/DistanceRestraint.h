/**
 *  \file IMP/core/DistanceRestraint.h
 *  \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DISTANCE_RESTRAINT_H
#define IMPCORE_DISTANCE_RESTRAINT_H

#include <IMP/core/core_config.h>
#include "DistancePairScore.h"
#include "XYZ.h"
#include <IMP/generic.h>

#include <IMP/Restraint.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Distance restraint between two particles
/**
   \note If the particles are closer than a certain distance, then
   the contributions to the derivatives are set to 0.

   \see PairRestraint
   \see DistancePairScore
   \see SphereDistancePairScore
 */
class IMPCOREEXPORT DistanceRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
    public Restraint
#else
    public IMP::internal::TupleRestraint<DistancePairScore>
#endif
    {
 public:
  //! Create the distance restraint.
  /** \param[in] m Model.
      \param[in] score_func Scoring function for the restraint.
      \param[in] a First particle in distance restraint.
      \param[in] b Second particle in distance restraint.
      \param[in] name restraint name
   */
  DistanceRestraint(Model *m, UnaryFunction *score_func,
                    ParticleIndexAdaptor a,
                    ParticleIndexAdaptor b,
                    std::string name = "DistanceRestraint %1%");

#ifdef SWIG
 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(DistanceRestraint);
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DISTANCE_RESTRAINT_H */
