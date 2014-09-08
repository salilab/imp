/**
 *  \file DiameterRgyrRestraint.cpp \brief Diameter restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/DiameterRgyrRestraint.h>
#include <numeric>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>
#include <string>
#include <iostream>
#include <math.h>
#include <limits>

IMPMEMBRANE_BEGIN_NAMESPACE

DiameterRgyrRestraint::DiameterRgyrRestraint
  (Particles ps, Float diameter, Float rgyr, Float kappa):
   Restraint(ps[0]->get_model(),"Diameter and Radius of Gyration Restraint"),
   ps_(ps), diameter_(diameter), rgyr_(rgyr), kappa_(kappa) {}

double
DiameterRgyrRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
 // check if derivatives are requested
 IMP_USAGE_CHECK(!da, "Derivatives not available");

 Float max_dist = 0.;
 Float rgyr = 0.;

 for (unsigned i=0; i<ps_.size()-1; ++i){
  Float Ri = core::XYZR(ps_[i]).get_radius();
  for (unsigned j=i+1; j<ps_.size(); ++j){
   Float Rj = core::XYZR(ps_[j]).get_radius();
   // distance between bead centers
   Float dist = core::get_distance(core::XYZ(ps_[i]), core::XYZ(ps_[j]));
   if (dist + Ri + Rj > max_dist) {max_dist = dist + Ri + Rj;}
   // and radius of gyration
   rgyr += dist * dist;
  }
 }
 // calculate radius of gyration
 rgyr = sqrt( rgyr / static_cast<double> ( ps_.size() * ps_.size() ) );

 // score
 Float score = ( diameter_ - max_dist ) * ( diameter_ - max_dist ) +
               ( rgyr_ - rgyr ) * ( rgyr_ - rgyr );
 return 0.5 * kappa_ * score;
}

kernel::ModelObjectsTemp DiameterRgyrRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  return ret;
}
/*
ContainersTemp DiameterRgyrRestraint::get_input_containers() const {
  return ContainersTemp();
}
*/

IMPMEMBRANE_END_NAMESPACE
