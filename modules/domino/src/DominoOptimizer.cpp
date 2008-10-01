/**
 *  \file DominoOptimizer.cpp  \brief An exact inference optimizer
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/domino/DominoOptimizer.h>

IMPDOMINO_BEGIN_NAMESPACE

DominoOptimizer::DominoOptimizer(std::string jt_filename, Model *m)
{
  ds_ = NULL;
  g_ = new RestraintGraph(jt_filename, m);
  set_model(m);
}

void DominoOptimizer::realize_rec(RestraintSet *rs, Float weight)
{
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
      RestraintSet *rs_child = dynamic_cast< RestraintSet*>(r);
      if (rs_child) {
        realize_rec(rs_child, 1.0);
      } else {
        g_->initialize_potentials(*r, 1.0);
      }
    }
  }
}

void DominoOptimizer::initialize_jt_graph(int number_of_nodes)
{
  g_->initialize_graph(number_of_nodes);
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
  g_->clear();
  set_sampling_space(ds_);
  // now infer the minimum
  g_->infer();
  //move the model to the states that reach the global minimum
  g_->move_model2global_minimum();
  return g_->get_minimum_score();
}

void DominoOptimizer::add_jt_node(int node_index,
                                  std::vector<Int> &particles_ind, Model &m)
{
  Particles particles = Particles();
  for  (std::vector<Int>::const_iterator it =particles_ind.begin();
   it != particles_ind.end(); it++) {
    particles.push_back(m.get_particle(*it));
  }
  g_->add_node(node_index,particles);
}

void DominoOptimizer::add_jt_edge(int node1_ind, int node2_ind)
{
  g_->add_edge(node1_ind, node2_ind);
}

IMPDOMINO_END_NAMESPACE
