/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/CoverRefined.h"

#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZRDecorator.h"
#include "IMP/core/FixedParticleRefiner.h"
#include "IMP/core/SingletonScoreState.h"
#include "IMP/core/SingletonsScoreState.h"
#include "IMP/core/DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

CoverRefined
::CoverRefined(ParticleRefiner *ref,
               FloatKey rk,
               Float slack): ref_(ref),
                             rk_(rk),
                             slack_(slack)
{
}

CoverRefined::~CoverRefined()
{
}

void CoverRefined::apply(Particle *p) const
{
  XYZRDecorator dp(p, rk_);
  IMP_CHECK_OBJECT(ref_.get());
  IMP_check(ref_->get_can_refine(p), "Passed particles cannot be refined",
            ValueException);
  Particles ps= ref_->get_refined(p);
  set_enclosing_sphere(dp, ps);
  dp.set_radius(dp.get_radius()+slack_);
  ref_->cleanup_refined(p, ps);
}

void CoverRefined::show(std::ostream &out) const
{
  out << "CoverRefined with "
      << *ref_ << " and " << rk_ << std::endl;
}

ScoreState* create_covers(SingletonContainer *sc,
                  ParticleRefiner *pr,
                  FloatKey radius_key, Float slack) {
  IMP_check(sc->get_number_of_particles() >0,
            "Need some particles to set up as centroid",
            ValueException);
  Model *m= sc->get_particle(0)->get_model();
  for (SingletonContainer::ParticleIterator pit= sc->particles_begin();
       pit != sc->particles_end(); ++pit) {
    XYZRDecorator d;
    if (!XYZRDecorator::is_instance_of(*pit)) {
      d= XYZRDecorator::create(*pit,
                            algebra::Sphere3D(algebra::Vector3D(0,0,0),0),
                            radius_key);
    } else {
      d= XYZRDecorator(*pit, radius_key);
    }
    d.set_coordinates_are_optimized(false);
  }

  CoverRefined *cr= new CoverRefined(pr, radius_key, slack);
  DerivativesToRefined *dtr= new DerivativesToRefined(pr,
                                    XYZDecorator::get_xyz_keys());
  SingletonsScoreState *sss= new SingletonsScoreState(sc, cr, dtr);
  m->add_score_state(sss);
  return sss;
}


ScoreState *create_cover(Particle *p, ParticleRefiner *pr,
                         FloatKey radius_key, Float slack) {
  if (!XYZDecorator::is_instance_of(p)) {
    XYZDecorator::create(p, algebra::Vector3D(0,0,0));
  }
  if (!p->has_attribute(radius_key)) {
    p->add_attribute(radius_key, 0);
  }
  XYZRDecorator d(p, radius_key);
  d.set_coordinates_are_optimized(false);

  CoverRefined *cr= new CoverRefined(pr, radius_key, slack);
  DerivativesToRefined *dtr= new DerivativesToRefined(pr,
                                    XYZDecorator::get_xyz_keys());
  SingletonScoreState *sss= new SingletonScoreState(cr, dtr, p);
  p->get_model()->add_score_state(sss);
  return sss;
}


IMPCORE_END_NAMESPACE
