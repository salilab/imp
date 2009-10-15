/**
 *  \file CentroidOfRefined.cpp
 *  \brief CentroidOf a the refined particles with a sphere.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/core/CentroidOfRefined.h"

#include "IMP/core/XYZR.h"
#include "IMP/core/FixedRefiner.h"
#include "IMP/core/SingletonScoreState.h"
#include "IMP/core/SingletonsScoreState.h"
#include "IMP/core/DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

CentroidOfRefined
::CentroidOfRefined(Refiner *r,
                                     FloatKey weight,
                                     FloatKeys ks): r_(r),
ks_(ks), w_(weight)
{
}


void CentroidOfRefined::apply(Particle *p) const
{
  ParticlesTemp ps = r_->get_refined(p);
  unsigned int n= ps.size();
  double tw=0;
  if (w_ != FloatKey()) {
    for (unsigned int i=0; i< n; ++i) {
      tw+= ps[i]->get_value(w_);
    }
  } else {
    tw=1;
  }
  for (unsigned int j=0; j< ks_.size(); ++j) {
    double v=0;
    for (unsigned int i=0; i< n; ++i) {
      double w;
      if (w_ != FloatKey()) {
        w= ps[i]->get_value(w_)/tw;
      } else {
        w= Float(1.0)/n;
      }
      v += ps[i]->get_value(ks_[j])*w;
    }
    p->set_value(ks_[j], v);
  }
}

ParticlesList CentroidOfRefined::get_interacting_particles(Particle*p) const {
  ParticlesTemp pt=get_read_particles(p);
  pt.push_back(get_write_particles(p)[0]);
  return ParticlesList(1, pt);
}

ParticlesTemp CentroidOfRefined::get_read_particles(Particle*p) const {
  ParticlesTemp ps = r_->get_refined(p);
  ps.push_back(p);
  return ps;
}

ParticlesTemp CentroidOfRefined::get_write_particles(Particle*p) const {
  return ParticlesTemp(1,p);
}

void CentroidOfRefined::show(std::ostream &out) const
{
  out << "CentroidOfRefined" << std::endl;
}


IMPCORE_END_NAMESPACE
