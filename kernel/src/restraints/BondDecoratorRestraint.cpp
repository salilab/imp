/**
 *  \file BondDecoratorRestraint.cpp 
 *  \brief A bond restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/restraints/BondDecoratorRestraint.h"
#include "IMP/UnaryFunction.h"
#include "IMP/score_states/BondDecoratorListScoreState.h"
#include "IMP/decorators/bond_decorators.h"
#include "IMP/pair_scores/DistancePairScore.h"
#include "IMP/internal/evaluate_distance_pair_score.h"
#include "IMP/decorators/XYZDecorator.h"
#include <boost/lambda/lambda.hpp>

namespace IMP
{

BondDecoratorRestraint
::BondDecoratorRestraint(UnaryFunction *f,
                         BondDecoratorListScoreState *s): bl_(s),
                                                          f_(f){}

Float BondDecoratorRestraint::evaluate(DerivativeAccumulator *accum)
{
  Float sum=0;
  for (BondDecoratorListScoreState::BondIterator bi= bl_->bonds_begin();
       bi != bl_->bonds_end(); ++bi) {
    BondDecorator bd= *bi;
    Float l= bd.get_length();
    Float s= bd.get_stiffness();
    if (l < 0) {
      IMP_WARN("Bond does not have a length: " << bd << std::endl);
      continue;
    }
    if (s <0) s=1;
    Particle *pa=NULL, *pb=NULL;
    try {
      /*IMP_LOG(VERBOSE, "Bonded pair " 
              << bd.get_bonded(0).get_particle()->get_index()
              << " " << bd.get_bonded(1).get_particle()->get_index()
              << " with length " << l << " and stiffness " << s << std::endl);*/
      pa = bd.get_bonded(0).get_particle();
      pb = bd.get_bonded(1).get_particle();
    } catch (const IndexException &e) {
      IMP_WARN("Problem processing bond: " << bd << std::endl);
      IMP_WARN(e.what() << std::endl);
    }
    if (pa && pb) {
      sum+= 
        internal::evaluate_distance_pair_score(XYZDecorator(pa),
                                               XYZDecorator(pb),
                                               accum,
                                               f_.get(),
                                               s*(boost::lambda::_1-l));
    }
  }
  return sum;
}

void BondDecoratorRestraint::show(std::ostream& out) const
{
  out << "Bond decorator restraint with unary function ";
  f_->show(out);
  out << " on " << bl_->number_of_bonds() << " bonds";
  out << std::endl;
}

} // namespace IMP
