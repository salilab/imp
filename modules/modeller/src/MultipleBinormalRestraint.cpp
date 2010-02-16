/**
 *  \file MultipleBinormalRestraint.cpp
 *  \brief Modeller-style multiple binormal (phi/psi) restraint.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/modeller/MultipleBinormalRestraint.h>

IMPMODELLER_BEGIN_NAMESPACE

MultipleBinormalRestraint::MultipleBinormalRestraint(
    const ParticleQuad &q1, const ParticleQuad &q2) : terms_(), q1_(q1),
                                                      q2_(q2)
{
}

double
MultipleBinormalRestraint::unprotected_evaluate(
                                DerivativeAccumulator *accum) const
{
  return 0.;
}


ParticlesList MultipleBinormalRestraint::get_interacting_particles() const {
  ParticlesTemp r(8);
  r[0] = q1_[0]; r[1] = q1_[1]; r[2] = q1_[2]; r[3] = q1_[3];
  r[4] = q2_[0]; r[5] = q2_[1]; r[6] = q2_[2]; r[7] = q2_[3];
  return ParticlesList(1, r);
}

ParticlesTemp MultipleBinormalRestraint::get_input_particles() const {
  return ParticlesTemp();
  ParticlesTemp r(8);
  r[0] = q1_[0]; r[1] = q1_[1]; r[2] = q1_[2]; r[3] = q1_[3];
  r[4] = q2_[0]; r[5] = q2_[1]; r[6] = q2_[2]; r[7] = q2_[3];
  return r;
}


ContainersTemp MultipleBinormalRestraint::get_input_containers() const {
  return ContainersTemp();
}

void MultipleBinormalRestraint::do_show(std::ostream& out) const
{
  out << "particle quads: ";
  q1_.show();
  out << " and ";
  q2_.show();
  out << std::endl;
}

IMPMODELLER_END_NAMESPACE
