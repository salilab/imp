/**
 *  \file covers.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/helper/optimization.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/IncrementalBallMover.h>
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/core/model_io.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>

IMPHELPER_BEGIN_NAMESPACE

double optimize_spheres(Model *m, const algebra::BoundingBox3D &bb,
                        double cutoff,
                        const OptimizerStates &oss) {
  bool done=false;
  IMP_NEW(core::ConjugateGradients, cg, (m));
  cg->set_optimizer_states(oss);
  unsigned int count=0;
  while (!done) {
    try {
      for (Model::ParticleIterator pit= m->particles_begin();
           pit != m->particles_end(); ++pit) {
        if (core::XYZ::particle_is_instance(*pit)) {
          core::XYZ d(*pit);
          if (d.get_coordinates_are_optimized()) {
            algebra::Vector3D v= algebra::random_vector_in_box(bb);
            d.set_coordinates(v);
          }
        }
      }
      double e=cg->optimize(1000);
      if ( e< cutoff) {
        done=true;
      }
    } catch (const ModelException &e) {
      IMP_LOG(TERSE, "Restarting optimization due to model exception."
              << std::endl);
      ++count;
    }
    if (count ==20) {
      IMP_THROW("Too many failures in optimize_spheres", UsageException);
    }
  };
  return m->evaluate(false);
}


IMPHELPER_END_NAMESPACE
