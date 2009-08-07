/**
 *  \file covers.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/helper/covers.h"
#include <IMP/SingletonContainer.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/CentroidOfRefined.h>
#include <IMP/core/DerivativesToRefined.h>
#include <IMP/core/SingletonsScoreState.h>
#include <IMP/core/SingletonScoreState.h>

IMPHELPER_BEGIN_NAMESPACE




ScoreState* create_covers(SingletonContainer *sc,
                  Refiner *pr,
                  FloatKey radius_key, Float slack) {
  IMP_check(sc->get_number_of_particles() >0,
            "Need some particles to set up as centroid",
            ValueException);
  Model *m= sc->get_particle(0)->get_model();
  for (SingletonContainer::ParticleIterator pit= sc->particles_begin();
       pit != sc->particles_end(); ++pit) {
    core::XYZR d;
    if (!core::XYZR::particle_is_instance(*pit)) {
      d= core::XYZR::setup_particle(*pit,
                            algebra::Sphere3D(algebra::Vector3D(0,0,0),0),
                            radius_key);
    } else {
      d= core::XYZR(*pit, radius_key);
    }
    d.set_coordinates_are_optimized(false);
  }

  core::CoverRefined *cr= new core::CoverRefined(pr, radius_key, slack);
  core::DerivativesToRefined *dtr= new core::DerivativesToRefined(pr,
                                    core::XYZ::get_xyz_keys());
  core::SingletonsScoreState *sss= new core::SingletonsScoreState(sc, cr, dtr);
  m->add_score_state(sss);
  return sss;
}


ScoreState *create_cover(Particle *p, Refiner *pr,
                         FloatKey radius_key, Float slack) {
  if (!core::XYZ::particle_is_instance(p)) {
    core::XYZ::setup_particle(p, algebra::Vector3D(0,0,0));
  }
  if (!p->has_attribute(radius_key)) {
    p->add_attribute(radius_key, 0);
  }
  core::XYZR d(p, radius_key);
  d.set_coordinates_are_optimized(false);

  core::CoverRefined *cr= new core::CoverRefined(pr, radius_key, slack);
  core::DerivativesToRefined *dtr= new core::DerivativesToRefined(pr,
                                    core::XYZ::get_xyz_keys());
  core::SingletonScoreState *sss= new core::SingletonScoreState(cr, dtr, p);
  p->get_model()->add_score_state(sss);
  return sss;
}




ScoreState* create_centroids(SingletonContainer *sc,
                      Refiner *pr,
                      FloatKey weight,
                      FloatKeys ks) {
  IMP_check(sc->get_number_of_particles() >0,
            "Need some particles to set up as centroid",
            ValueException);
  for (SingletonContainer::ParticleIterator pit= sc->particles_begin();
       pit != sc->particles_end(); ++pit) {
    for (unsigned int i=0; i< ks.size(); ++i) {
      if (!(*pit)->has_attribute(ks[i])) {
        (*pit)->add_attribute(ks[i], 0, false);
      }
    }
  }

  core::CentroidOfRefined *cr= new core::CentroidOfRefined(pr, weight, ks);
  core::DerivativesToRefined *dtr= new core::DerivativesToRefined(pr,
                                                      ks);
  core::SingletonsScoreState *sss= new core::SingletonsScoreState(sc, cr, dtr);
  return sss;
}



ScoreState* create_centroid(Particle *p, Refiner *pr,
                     FloatKey weight,
                     FloatKeys ks) {
  for (unsigned int i=0; i< ks.size(); ++i) {
    if (!p->has_attribute(ks[i])) {
      p->add_attribute(ks[i], 0, false);
    }
  }

  core::CentroidOfRefined *cr= new core::CentroidOfRefined(pr, weight, ks);
  core::DerivativesToRefined *dtr= new core::DerivativesToRefined(pr,ks);
  core::SingletonScoreState *sss= new core::SingletonScoreState(cr, dtr, p);
  IMP_check(pr->get_refined(p).size()>0,
             "Need particles to compute the centroid of",
             ValueException);
  return sss;
}

IMPHELPER_END_NAMESPACE
