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
  num_of_solutions_=1;
}
void DominoOptimizer::set_sampling_space(DiscreteSampler *ds)
{
  ds_ = ds;
 g_->set_sampling_space(*ds_);
 Restraint *r; Particles ps; Float w;
 for(std::vector<OptTuple>::iterator it = rs_.begin(); it != rs_.end();it++) {
   r = boost::get<0>(*it);
   ps = boost::get<1>(*it);
   w = boost::get<2>(*it);
   g_->initialize_potentials(r,&ps,w);
 }
}

void DominoOptimizer::initialize_jt_graph(int number_of_nodes)
{
  g_->initialize_graph(number_of_nodes);
}

Float DominoOptimizer::optimize(unsigned int max_steps)
{
  std::stringstream error_message;
  IMP_assert(ds_ != NULL,
             "DominoOptimizer::optimize the sampling space was not set");
  //init all the potentials
  g_->clear();
  set_sampling_space(ds_);
  // now infer the minimum
  g_->infer(num_of_solutions_);
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

void DominoOptimizer::move_to_opt_comb(unsigned int i)  const {
  const CombState *opt_s = g_->get_opt_combination(i);
  ds_->move2state(opt_s);
}

void DominoOptimizer::add_restraint_recursive(Restraint *rs, Float weight)
{
  core::RestraintSet *rs_set = dynamic_cast<core::RestraintSet*>(rs);
   if (rs_set) {
     for (Model::RestraintIterator it = rs_set->restraints_begin();
          it != rs_set->restraints_end(); it++) {
       add_restraint_recursive(*it, weight*rs_set->get_weight());
     }
   }
   else {
     IMP_check(rs->get_interacting_particles().size()==1,
               "DominoOptimizer::add_restraint_recursive dose not support"
               <<" lists with more than one set of particles for a single "
               <<"restraint: "
               << rs->get_interacting_particles().size(),ErrorException);
     rs_.push_back(OptTuple(rs,rs->get_interacting_particles()[0],weight));
   }
}

void DominoOptimizer::add_restraint(Restraint *r) {
  add_restraint_recursive(r,1.0);
}
void DominoOptimizer::add_restraint(Restraint *r,Particles ps) {
  rs_.push_back(OptTuple(r,ps,1.0));
}
IMPDOMINO_END_NAMESPACE
