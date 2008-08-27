/**
 *  \file DominoOptimizer.cpp  \brief An exact inference optimizer
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/domino/DominoOptimizer.h"

namespace IMP
{

namespace domino
{

DominoOptimizer::DominoOptimizer(Model *m)
{
  ds_ = NULL;
  g_ = new RestraintGraph();
  set_model(m);
}

void DominoOptimizer::realize_rec(RestraintSet *rs, Float weight)
{
  std::cout << "DominoOptimizer::realize_rec number of restraints "
            << rs->get_number_of_restraints() << " with weight : " << weight
            << std::endl;
  for (Model::RestraintIterator it = rs->restraints_begin();
       it != rs->restraints_end(); it++) {
    Restraint *r = *it;
    // if it is a restraint set
    RestraintSet *rs_child = dynamic_cast< RestraintSet*>(r);
    if (rs_child) {
      realize_rec(rs_child, weight*rs->get_weight());
    } else {
      g_->initialize_potentials(*r, weight*rs->get_weight());
    }
  }
}

void DominoOptimizer::set_sampling_space(DiscreteSampler *ds)
{
  ds_ = ds;
  g_->set_sampling_space(*ds_);
  Model *opt_mdl = get_model();
  //go over all of the restraints and assign the values to one of the
  //appropriate nodes
  Restraint *r;
  for (Model::RestraintIterator it = opt_mdl->restraints_begin();
       it != opt_mdl->restraints_end(); it++) {
    r = *it;
    if (r != NULL) {
      r->show();
      RestraintSet *rs_child = dynamic_cast< RestraintSet*>(r);
      if (rs_child) {
        realize_rec(rs_child, 1.0);
      } else {
        g_->initialize_potentials(*r, 1.0);
      }
    }
  }
}

Float DominoOptimizer::optimize(unsigned int max_steps)
{
  Model *opt_mdl = get_model();
  std::stringstream error_message;
  error_message << "DominoOptimizer::optimize the sampling space was not set";
  IMP_assert(ds_ != NULL, error_message.str());
  error_message.clear();
  error_message << "DominoOptimizer::optimize the model was not set";
  IMP_assert(opt_mdl != NULL, error_message.str());
  //init all the potentials

  // now infer the minimum
  g_->infer();
  //move the model to the states that reach the global minimum
  g_->move_model2global_minimum();
  return g_->get_minimum_score();
}

} // namespace domino

} // namespace IMP
