/**
 *  \file BondDecoratorSingletonScore.cpp
 *  \brief A score based a bond decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/atom/BondSingletonScore.h"
#include "IMP/core/internal/evaluate_distance_pair_score.h"
#include "IMP/atom/bond_decorators.h"
#include "IMP/core/XYZDecorator.h"
#include <boost/lambda/lambda.hpp>

IMPATOM_BEGIN_NAMESPACE

BondSingletonScore::BondSingletonScore(UnaryFunction *f): f_(f){}

Float BondSingletonScore::evaluate(Particle *b,
                                   DerivativeAccumulator *da) const
{
  IMP_IF_CHECK(EXPENSIVE) {
    BondDecorator::cast(b);
  }
  BondDecorator bd(b);
  Float l= bd.get_length();
  Float s= bd.get_stiffness();
  if (l < 0) {
    IMP_WARN("Bond does not have a length: " << bd << std::endl);
    return 0;
  }
  if (s <0) s=1;
  Particle *pa=NULL, *pb=NULL;
  try {
    pa = bd.get_bonded(0).get_particle();
    pb = bd.get_bonded(1).get_particle();
  } catch (const IndexException &e) {
    IMP_WARN("Problem processing bond: " << bd << std::endl);
    IMP_WARN(e.what() << std::endl);
    return 0;
  }
  return
    IMP::core::internal::
    evaluate_distance_pair_score(IMP::core::XYZDecorator(pa),
                                 IMP::core::XYZDecorator(pb),
                                 da,
                                 f_.get(),
                                 s*(boost::lambda::_1-l));
}

void BondSingletonScore::show(std::ostream &out) const
{
  out << "BondSingletonScore using ";
  f_->show(out);
  out << std::endl;
}

IMPATOM_END_NAMESPACE
