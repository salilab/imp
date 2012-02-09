/**
 *  \file FretrRestraint.cpp \brief FRET_R restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/FretrRestraint.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>
#include <iostream>
#include <map>

IMPMEMBRANE_BEGIN_NAMESPACE

FretrRestraint::FretrRestraint(Particles pd, Particles pa,
                               double R0, double Sd, double Sa,
                               double gamma, double Ida,
                               double fretr, double kappa,
                               std::string name):
  Restraint("FRET_R Restraint " + name) {

 pd_ = pd;
 pa_ = pa;
 R0_ = R0;
 Sd_ = Sd;
 Sa_ = Sa;
 gamma_ = gamma;
 Ida_   = Ida;
 fretr_ = fretr;
 kappa_ = kappa;
 IMP_NEW(container::ListSingletonContainer,lsc_p,(pd));
 IMP_NEW(container::ListSingletonContainer,lsc_a,(pa));
 cbpc_= new container::CloseBipartitePairContainer(lsc_p,lsc_a,2.5*R0);
}

double
FretrRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
// check if derivatives are requested
 IMP_USAGE_CHECK(!da, "Derivatives not available");

// clear the Fi_ map
 Fi_.clear();

// cycle the neighbor list
 IMP_FOREACH_PAIR_INDEX(cbpc_, {
   ParticleIndexPair  pp=_1;
   double power = R0_/core::get_distance(core::XYZ(get_model(), pp[0]),
                                         core::XYZ(get_model(), pp[1]));
   Fi_[pp[0]]+=power*power*power*power*power*power;
 });

 double sumFi=0.0;
 for(unsigned j=0;j<pd_.size();++j){sumFi+=1.0/(1.0+Fi_[pd_[j]->get_index()]);}

 double fretr = 1.0 + gamma_ * ( pd_.size() - sumFi ) /
                ( Sd_ * Ida_ * sumFi + Sa_ * pa_.size() );

 return 0.5 * kappa_ * ( fretr - fretr_ ) * ( fretr - fretr_ );
}

ParticlesTemp FretrRestraint::get_input_particles() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), pd_.begin(), pd_.end());
  ret.insert(ret.end(), pa_.begin(), pa_.end());
  return ret;
}

ContainersTemp FretrRestraint::get_input_containers() const {
  return ContainersTemp();
}

void FretrRestraint::do_show(std::ostream &) const
{
}

IMPMEMBRANE_END_NAMESPACE
