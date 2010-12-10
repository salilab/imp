/**
 *  \file isd/JeffreysRestraint.cpp
 *  \brief Restrain a nuisance particle with log(nuisance)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Nuisance.h>
#include <IMP/isd/JeffreysRestraint.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

JeffreysRestraint::JeffreysRestraint(Particle *p): p_(p) {
}


/* Apply the score if it's a nuisance decorator.
 */
double
JeffreysRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    Nuisance::decorate_particle(p_);
  }
  Nuisance sig(p_);
  double score;
  score=log(Float(sig.get_nuisance()));
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp JeffreysRestraint::get_input_particles() const
{
  return ParticlesTemp(1,p_);
}

/* The only container used is pc_. */
ContainersTemp JeffreysRestraint::get_input_containers() const
{
  return ContainersTemp();
}

void JeffreysRestraint::do_show(std::ostream& out) const
{
  out << "particle= " << p_->get_name() << std::endl;
}

IMPISD_END_NAMESPACE
