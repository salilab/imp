/**
 *  \file merge_tree_utils.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/multifit/merge_tree_utils.h>

IMPMULTIFIT_BEGIN_NAMESPACE

double DummyRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{return 0.;
}
ParticlesTemp DummyRestraint::get_input_particles() const
{
  ParticlesTemp pt;
  pt.push_back(p1_);
  pt.push_back(p2_);
  return pt;
}

ContainersTemp DummyRestraint::get_input_containers() const {
  return ContainersTemp();
}

void DummyRestraint::do_show(std::ostream& out) const
{
  out<<"Dummy restraint bewteen:"<<p1_->get_name()<<" and "
     << p2_->get_name()<<std::endl;
}

IMPMULTIFIT_END_NAMESPACE
