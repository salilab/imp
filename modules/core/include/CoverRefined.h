/**
 *  \file IMP/core/CoverRefined.h
 *  \brief Set the position and radius of a particle to enclose the refined.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_COVER_REFINED_H
#define IMPCORE_COVER_REFINED_H

#include <IMP/core/core_config.h>

#include "XYZR.h"
#include <IMP/Refiner.h>
#include <IMP/Pointer.h>
#include <IMP/Decorator.h>
#include <IMP/SingletonModifier.h>
#include <IMP/SingletonDerivativeModifier.h>

IMPCORE_BEGIN_NAMESPACE

// for swig
class XYZR;

//! Set the position and radius of a particle to enclose the refined.
/** Set the coordinates and radius of the passed particle to cover the
    particles listed by the particle refiner.
    An example showing a how to use such a score state to maintain a cover
    of the atoms of a protein by a sphere per residue:
    \include cover_particles.py

    \note The particle passed must be an XYZR.

    \see DerivativesFromRefined
    \see DerivativesToRefined
    \see CentroidOfRefined

    \see atom::CoverBond

    \uses{class CoverRefined, CGAL}
 */
class IMPCOREEXPORT CoverRefined : public SingletonModifier {
  IMP::PointerMember<Refiner> refiner_;
  Float slack_;

 public:
  //! Create with the given refiner and radius key
  /** Slack is the amount added to the radius. */
  CoverRefined(Refiner *ref, Float slack = 0);

  //! Set how much extra to add to the radius.
  void set_slack(Float slack) { slack_ = slack; }

  virtual void apply_index(Model *m, ParticleIndex a) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(CoverRefined);
  IMP_OBJECT_METHODS(CoverRefined);
};

//! A particle which covers a set of other particles.
/** A decorator which sets up a particle to be a cover of a
    set of other particles. That is, the sphere defined by
    its XYZR decorator will always contain the passed
    particles.

    \usesconstraint
*/
IMP_SUMMARIZE_DECORATOR_DECL(Cover, XYZR, XYZs,);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_COVER_REFINED_H */
