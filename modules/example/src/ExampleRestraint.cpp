/**
 *  \file example/ExampleRestraint.cpp
 *  \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/example/ExampleRestraint.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleRestraint::ExampleRestraint(Particle *p,
                                   double k) : p_(p),
                                          k_(k) {
}


/* Apply the pair score to each particle pair listed in the container.
 */
double
ExampleRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  core::XYZ d(p_);
  IMP_LOG(VERBOSE, "The z coordinate of " << d->get_name()
          << " is " << d.get_z() << std::endl);
  double score= .5*k_*square(d.get_z());
  if (accum) {
    double deriv= k_*d.get_z();
    d.add_to_derivative(2, deriv, *accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp ExampleRestraint::get_input_particles() const
{
  return ParticlesTemp(1,p_);
}

/* The only container used is pc_. */
ContainersTemp ExampleRestraint::get_input_containers() const
{
  return ContainersTemp();
}

void ExampleRestraint::do_show(std::ostream& out) const
{
  out << "particle= " << p_->get_name() << std::endl;
  out << "k= " << k_ << std::endl;
}

IMPEXAMPLE_END_NAMESPACE
