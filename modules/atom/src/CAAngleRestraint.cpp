/**
 *  \file CAAngleRestraint.cpp \brief Dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/CAAngleRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

#include <map>

IMPATOM_BEGIN_NAMESPACE

CAAngleRestraint::CAAngleRestraint(Model *m, ParticleIndexAdaptor p1,
                      ParticleIndexAdaptor p2, ParticleIndexAdaptor p3,
                      Floats phi0, Floats score):
  Restraint(m, "CAAngleRestraint%1%")
{
  p_[0] = p1;
  p_[1] = p2;
  p_[2] = p3;
  phi0_ = phi0;
  score_ = score;
}

double CAAngleRestraint::get_distance
 (double v0, double v1) const {

 double dist = v0 - v1;
 if( dist >  IMP::PI ){dist -= 2.*IMP::PI;}
 if( dist < -IMP::PI ){dist += 2.*IMP::PI;}
 return dist;
}

int CAAngleRestraint::get_closest
 (std::vector<double> const& vec, double value) const {

 int index=0;
 double dist_min=1.e+10;
 for(unsigned i=0; i<vec.size(); ++i){
  double dist = fabs(get_distance(value, vec[i]));
  if( dist < dist_min ){
   dist_min = dist;
   index = i;
  }
 }
 return index;
}

double
CAAngleRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{
  Model *m = get_model();
  core::XYZ d0(m, p_[0]);
  core::XYZ d1(m, p_[1]);
  core::XYZ d2(m, p_[2]);

  double phi0 = core::internal::angle(d0, d1, d2, NULL,NULL,NULL);

  int index = get_closest(phi0_,phi0);

  return score_[index];
}

ModelObjectsTemp CAAngleRestraint::do_get_inputs() const {
  ParticlesTemp ret(3);
  for (unsigned i = 0; i < 3; ++i) {
    ret[i] = get_model()->get_particle(p_[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE
