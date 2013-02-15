/**
 *  \file example/ExampleRestraint.cpp
 *  \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/example/ExampleRestraint.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleRestraint::ExampleRestraint(Particle *p,
                                   double k) :
  Restraint(p->get_model(), "ExampleRestraint%1%"), p_(p),
  k_(k) {
}


/* Apply the pair score to each particle pair listed in the container.
 */
void
ExampleRestraint::do_add_score_and_derivatives(ScoreAccumulator sa)
  const
{
  IMP_OBJECT_LOG;
  core::XYZ d(p_);
  IMP_LOG_VERBOSE( "The z coordinate of " << d->get_name()
          << " is " << d.get_z() << std::endl);
  double score= .5*k_*square(d.get_z());
  if (sa.get_derivative_accumulator()) {
    double deriv= k_*d.get_z();
    d.add_to_derivative(2, deriv, *sa.get_derivative_accumulator());
  }
  sa.add_score(score);
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp ExampleRestraint::do_get_inputs() const
{
  return ModelObjectsTemp(1,p_);
}

IMPEXAMPLE_END_NAMESPACE
