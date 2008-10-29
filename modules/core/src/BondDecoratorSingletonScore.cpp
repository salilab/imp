/**
 *  \file BondDecoratorSingletonScore.cpp
 *  \brief A score based a bond decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/BondDecoratorSingletonScore.h"
#include "IMP/core/internal/evaluate_distance_pair_score.h"
#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZDecorator.h"
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

BondDecoratorSingletonScore::BondDecoratorSingletonScore(UnaryFunction *f):
                                                              f_(f){}

Float BondDecoratorSingletonScore::evaluate(Particle *b,
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
    internal::evaluate_distance_pair_score(XYZDecorator(pa),
                                           XYZDecorator(pb),
                                           da,
                                           f_.get(),
                                           s*(boost::lambda::_1-l));
}

void BondDecoratorSingletonScore::show(std::ostream &out) const
{
  out << "BondDecoratorSingletonScore using ";
  f_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
