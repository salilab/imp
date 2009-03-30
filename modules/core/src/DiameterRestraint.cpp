/**
 *  \file DiameterRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/DiameterRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZRDecorator.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/PairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE
namespace {

  class SPairContainer: public PairContainer {
    Pointer<SingletonContainer> sc_;
    Pointer<Particle> p_;
  public:
    SPairContainer(SingletonContainer* sc, Particle *p): sc_(sc), p_(p){}
    IMP_PAIR_CONTAINER(SPairContainer, internal::version_info)
  };

  ParticlePair SPairContainer::get_particle_pair(unsigned int i) const {
    return ParticlePair(sc_->get_particle(i), p_);
  }

  unsigned int SPairContainer::get_number_of_particle_pairs() const {
    return sc_->get_number_of_particles();
  }

  bool SPairContainer::get_contains_particle_pair(ParticlePair p) const {
    return p[1]==p_ && sc_->get_contains_particle(p[0]);
  }


  void SPairContainer::show(std::ostream &out) const {}


  class ShiftedDistancePairScore: public PairScore {
    Pointer<UnaryFunction> f_;
    Float offset_;
  public:
    ShiftedDistancePairScore(UnaryFunction *f, Float o): f_(f), offset_(o){}
    virtual ~ShiftedDistancePairScore(){}
    virtual Float evaluate(Particle *a, Particle *b,
                           DerivativeAccumulator *da) const{
      return internal::evaluate_distance_pair_score(XYZDecorator(a),
                                                    XYZDecorator(b),
                                                    da, f_.get(),
                                                    boost::lambda::_1-offset_);
    }

    virtual void show(std::ostream &out=std::cout) const {}
  };

}

DiameterRestraint::DiameterRestraint(UnaryFunction *f,
                                     SingletonContainer *sc,
                                     Float diameter):diameter_(diameter),
                                                     sc_(sc), f_(f){
  IMP_check(sc->get_number_of_particles()>2,
            "Need at least two particles to restrain diameter",
            ValueException);
  IMP_check(diameter>0, "The diameter must be positive",
            ValueException);
}

void DiameterRestraint::set_model(Model *m) {
  if (m) {
    IMP_LOG(TERSE, "Creating components of DiameterRestraint" << std::endl);
    Model *m= sc_->get_particle(0)->get_model();

    // make pairs from special generator
    Float radius= diameter_/2.0;

    p_= new Particle(m);
    SPairContainer *sp= new SPairContainer(sc_, p_);
    r_=new PairsRestraint(new ShiftedDistancePairScore(f_, radius), sp);
    ss_=create_cover(p_,
                     new FixedRefiner(Particles(sc_->particles_begin(),
                                                sc_->particles_end())),
                     FloatKey("diameter_radius"),
                     0);
    m->add_score_state(ss_);
    r_->set_was_owned(true);
  } else {
    IMP_LOG(TERSE, "Removing components of DiameterRestraint" << std::endl);
    m->remove_score_state(ss_);
    m->remove_particle(p_);
    ss_=NULL;
    p_=NULL;
    r_=NULL;
  }
  Restraint::set_model(m);
}

Float DiameterRestraint::evaluate(DerivativeAccumulator *da) {
  return r_->evaluate(da);
}

void DiameterRestraint::show(std::ostream &out) const {
  out << "DiameterRestraint" << std::endl;
}

ParticlesList DiameterRestraint::get_interacting_particles() const {
  // all the interactions are returned by the actual restraint
  return r_->get_interacting_particles();
}

IMPCORE_END_NAMESPACE
