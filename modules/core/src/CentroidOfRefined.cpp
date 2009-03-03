/**
 *  \file CentroidOfRefined.cpp
 *  \brief CentroidOf a the refined particles with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/CentroidOfRefined.h"

#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZRDecorator.h"

IMPCORE_BEGIN_NAMESPACE

CentroidOfRefined
::CentroidOfRefined(ParticleRefiner *r,
                                     FloatKey weight,
                                     FloatKeys ks): r_(r),
ks_(ks), w_(weight)
{
}

CentroidOfRefined::~CentroidOfRefined()
{
}

void CentroidOfRefined::apply(Particle *p) const
{
  Particles ps = r_->get_refined(p);
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
  r_->cleanup_refined(p, ps);
}

void CentroidOfRefined::show(std::ostream &out) const
{
  out << "CentroidOfRefined" << std::endl;
}

IMPCORE_END_NAMESPACE
