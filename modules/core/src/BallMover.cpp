/**
 *  \file BallMover.cpp  \brief A modifier which perturbs a discrete variable.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/BallMover.h>

#include <IMP/random.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/vector_generators.h>
#include <boost/random/uniform_real.hpp>

IMPCORE_BEGIN_NAMESPACE


BallMover::BallMover(const ParticlesTemp &sc,
                     const FloatKeys &vars,
                     double max):
  MoverBase(sc, vars, "BallMover%1%")
{
  set_radius(max);
}

BallMover::BallMover(const ParticlesTemp &sc,
                     double max):
  MoverBase(sc, XYZ::get_xyz_keys(), "XYZBallMover%1%")
{
  set_radius(max);
}

void BallMover::do_move(Float scale)
{
  IMP_OBJECT_LOG;
  for (unsigned int i = 0;
       i < get_number_of_particles(); ++i) {
    IMP_LOG_TERSE( "Moving particle " << get_particle_name(i) << std::endl);
    Floats center(get_number_of_keys());
    // Note that this loop would normally run from 0 to nkeys; this slightly
    // unusual formulation works around a g++ bug:
    // https://bugzilla.redhat.com/show_bug.cgi?id=758908
    for (int j = get_number_of_keys() - 1; j >= 0; --j) {
      center[j] = get_value(i, j);
    }
    algebra::VectorKD vcenter(center.begin(), center.end());
    algebra::VectorKD npos
      = IMP::algebra::get_random_vector_in(algebra::SphereKD(vcenter,
                                                             scale * radius_));
    IMP_LOG_VERBOSE( "Old pos is " << vcenter << " new is "
            << npos << std::endl);
    for (unsigned int j = 0; j < get_number_of_keys(); ++j) {
      propose_value(i, j, npos[j]);
    }
  }
}

void BallMover::do_show(std::ostream &out) const {
  out << "radius " << radius_ << std::endl;
}

IMPCORE_END_NAMESPACE
