/**
 *  \file CoverRefined.h
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_COVER_REFINED_H
#define IMPCORE_COVER_REFINED_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZRDecorator.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/SingletonContainer.h>


IMPCORE_BEGIN_NAMESPACE

/** \brief This class sets the position and radius of each particle to
 enclose the refined.

 \see DerivativesFromRefined
 \see DerivativesToRefined
 \see CentroidOfRefined

 Set the coordinates and radius of the passed particle to cover the particles
 listed by the particle refiner.
 An example showing a how to use such a score state to maintain a cover
 of the atoms of a protein by a sphere per residue.
 \verbinclude simple_examples/cover_particles.py

 \note The particle passed must be an XYZRDecorator with the given radius key.
 \note This used the set_enclosing_sphere function and so produces
  better results if the CGAL library is found.
 */
class IMPCOREEXPORT CoverRefined: public SingletonModifier
{
  Pointer<ParticleRefiner> ref_;
  FloatKey rk_;
  Float slack_;
public:
  //! Create with the given refiner and radius key
  /** Slack is the amount added to the radius.*/
  CoverRefined(ParticleRefiner *ref,
                                FloatKey rk
                                =XYZRDecorator::get_default_radius_key(),
                                Float slack=0);
  ~CoverRefined();

  IMP_SINGLETON_MODIFIER(internal::version_info);

  //! Set how nmuch extra to add to the radius.
  void set_slack(Float slack) {
    slack_=slack;
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_COVER_REFINED_H */
