/**
 *  \file SphericalRestraint.cpp \brief Absolute position restraint.
 *
 *  Optimize based on distance from an absolute position.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/SphericalRestraint.h"
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

static const float MIN_DISTANCE_SQUARED=.001;



SphericalRestraint::SphericalRestraint(Particle* p,
                                       Float x, Float y, Float z,
                                       UnaryFunctor* score_func)
{
  add_particle(p);
  center_[0]=x;
  center_[1]=y;
  center_[2]=z;
  score_func_ =score_func;
}

SphericalRestraint::~SphericalRestraint()
{
  delete score_func_;
}



Float SphericalRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(score_func_);

  Float d2=0;
  Float diff[3];
  XYZDecorator xyzd= XYZDecorator::cast(get_particle(0));
  for (unsigned int i=0; i< 3; ++i) {
    diff[i] =xyzd.get_coordinate(i) - center_[i];
    d2+= diff[i]*diff[i];
  }
  Float d= std::sqrt(d2);
  if (d2 < MIN_DISTANCE_SQUARED) {
    return 0;
  } 

  Float ret=0;
  if (accum) {
    Float deriv;
    ret= (*score_func_)(d, deriv);
    for (unsigned int i=0; i< 3; ++i) {
      Float dd= deriv*diff[i]/d;
      xyzd.add_to_coordinate_derivative(i, dd, *accum);
    }
  } else {
    ret= (*score_func_)(d);
  }
  return ret;
}


void SphericalRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "Spherical restraint (active):" << std::endl;
  } else {
    out << "Spherical restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  particle: " << get_particle(0)->get_index();
  out << "  ";
  score_func_->show(out);
  out << std::endl;
  out << std::endl;
}

}  // namespace IMP
