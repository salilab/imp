/**
 *  \file CADihedralRestraint.cpp \brief Dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/CADihedralRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/constants.h>
#include <math.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

#include <map>

IMPATOM_BEGIN_NAMESPACE

CADihedralRestraint::CADihedralRestraint(Model *m,
                      ParticleIndexAdaptor p1, ParticleIndexAdaptor p2,
                      ParticleIndexAdaptor p3, ParticleIndexAdaptor p4,
                      ParticleIndexAdaptor p5, Floats phi0, Floats phi1,
                      Floats score):
  Restraint(m, "CADihedralRestraint%1%")
{
  p_[0] = p1;
  p_[1] = p2;
  p_[2] = p3;
  p_[3] = p4;
  p_[4] = p5;
  phi0_ = phi0;
  phi1_ = phi1;
  score_ = score;
}

double CADihedralRestraint::get_distance
 (double v0, double v1) const {

 double dist = v0 - v1;
 if( dist >  IMP::PI ){dist -= 2.*IMP::PI;}
 if( dist < -IMP::PI ){dist += 2.*IMP::PI;}
 return dist;
}

int CADihedralRestraint::get_closest
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
CADihedralRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{
  Model *m = get_model();
  core::XYZ d0(m, p_[0]);
  core::XYZ d1(m, p_[1]);
  core::XYZ d2(m, p_[2]);
  core::XYZ d3(m, p_[3]);
  core::XYZ d4(m, p_[4]);

  double phi0 = core::internal::dihedral(d0, d1, d2, d3, NULL,NULL,NULL,NULL);

  double phi1 = core::internal::dihedral(d1, d2, d3, d4, NULL,NULL,NULL,NULL);

  int i0 = get_closest(phi0_,phi0);

  int i1 = get_closest(phi1_,phi1);

  return score_[i0*phi0_.size()+i1];
}

ModelObjectsTemp CADihedralRestraint::do_get_inputs() const {
  ParticlesTemp ret(5);
  for (unsigned i = 0; i < 5; ++i) {
    ret[i] = get_model()->get_particle(p_[i]);
  }
  return ret;
}

IMPATOM_END_NAMESPACE
