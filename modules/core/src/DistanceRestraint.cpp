/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2) :
  dp_(new DistancePairScore(score_func))
{
  p_[0]=XYZ(p1);
  p_[1]=XYZ(p2);
}

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     XYZ a, XYZ b) :
  dp_(new DistancePairScore(score_func))
{
  p_[0]=a;
  p_[1]=b;
}

//! Calculate the score for this distance restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
double
DistanceRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  return dp_->evaluate(ParticlePair(p_[0], p_[1]), accum);
}


ParticlesList DistanceRestraint::get_interacting_particles() const {
  return dp_->get_interacting_particles(ParticlePair(p_[0], p_[1]));
}

ParticlesTemp DistanceRestraint::get_input_particles() const {
  return dp_->get_input_particles(ParticlePair(p_[0], p_[1]));
}


ObjectsTemp DistanceRestraint::get_input_objects() const {
  return ObjectsTemp();
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void DistanceRestraint::show(std::ostream& out) const
{
  out << "distance restraint:" << std::endl;

  get_version_info().show(out);
  out << "  particles: " << p_[0].get_particle()->get_name();
  out << " and " << p_[1].get_particle()->get_name();
  out << "  ";
  dp_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
