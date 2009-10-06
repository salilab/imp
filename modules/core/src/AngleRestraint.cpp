/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/AngleRestraint.h>
#include <IMP/core/AngleTripletScore.h>

IMPCORE_BEGIN_NAMESPACE

AngleRestraint::AngleRestraint(UnaryFunction* score_func,
                               Particle* p1, Particle* p2, Particle* p3)
{
  p_[0]=XYZ(p1);
  p_[1]=XYZ(p2);
  p_[2]=XYZ(p3);

  sf_= new AngleTripletScore(score_func);
}

AngleRestraint::AngleRestraint(UnaryFunction* score_func,
                               XYZ p0, XYZ p1, XYZ p2)
{
  p_[0]=p0;
  p_[1]=p1;
  p_[2]=p2;

  sf_= new AngleTripletScore(score_func);
}

double AngleRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  return sf_->evaluate(p_[0], p_[1], p_[2],
                       accum);
}

ParticlesList AngleRestraint::get_interacting_particles() const
{
  return ParticlesList(1, get_used_particles());
}

ParticlesTemp AngleRestraint::get_used_particles() const
{
  return ParticlesTemp(p_, p_+3);
}

void AngleRestraint::show(std::ostream& out) const
{
  out << "angle restraint:" << std::endl;

  get_version_info().show(out);
  out << "  particles: " << p_[0].get_particle()->get_name();
  out << ", " << p_[1].get_particle()->get_name();
  out << " and " << p_[2].get_particle()->get_name();
  out << "  ";
  sf_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
