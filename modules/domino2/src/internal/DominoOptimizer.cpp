/**
 *  \file DominoOptimizer.cpp  \brief An exact inference optimizer
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/domino2/internal/DominoOptimizer.h>
#include <numeric>

IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

void DominoOptimizer::do_show(std::ostream &out) const {
}

DominoOptimizer
::DominoOptimizer(container::ListSingletonContainer* all_particles,
                  const JunctionTree &jt, Model *m,
                  RestraintEvaluator *r_eval):
  all_particles_(all_particles)
{
  ds_ = NULL;
  g_ = new RestraintGraph(jt, m,r_eval);
  set_model(m);
  num_of_solutions_=1;
  rstr_eval_=r_eval;
}

void DominoOptimizer::set_sampling_space(DiscreteSampler *ds)
{
  ds_ = ds;
  g_->set_sampling_space(*ds_);
  g_->initialize_potentials();
  IMP_LOG(VERBOSE,"DominoOptimizer::set_sampling_space after potential"
          << " initialization"<<std::endl);
  IMP_LOG_WRITE(VERBOSE,g_->show());
}

void DominoOptimizer::initialize_jt_graph(int number_of_nodes)
{
  g_->initialize_graph(number_of_nodes);
}


Float DominoOptimizer::optimize(unsigned int max_steps)
{
  IMP_INTERNAL_CHECK(ds_ != NULL,
                     "DominoOptimizer::optimize the sampling space was not set"
                     <<std::endl);
  //init all the potentials
  g_->clear();
  IMP_LOG(VERBOSE,"DominoOptimizer::optimize going to set sampling space"
          <<std::endl);
  set_sampling_space(ds_);
  // now infer the minimum
  IMP_LOG(VERBOSE,"DominoOptimizer::optimize going to infer solutions"
          <<std::endl);
  g_->infer(num_of_solutions_);
  //move the model to the states that reach the global minimum
  g_->move_to_global_minimum_configuration();
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE,"DominoOptimizer::optimize going to move the "
            <<"model to the global minimum: "<<std::endl);
    IMP_LOG_WRITE(TERSE,g_->get_opt_combination(0)->show());
    IMP_LOG(TERSE,std::endl);

  }
  return g_->get_minimum_score();
}

void DominoOptimizer::add_jt_node(int node_index,
                                  std::vector<Int> &particles_ind, Model &m)
{
  Particles particles = Particles();
  for  (std::vector<Int>::const_iterator it =particles_ind.begin();
        it != particles_ind.end(); it++) {
    // may be missing a -1 after *it
    particles.push_back(all_particles_->get_particle(*it));
  }
  g_->add_node(node_index,particles,rstr_eval_);
}

void DominoOptimizer::add_jt_edge(int node1_ind, int node2_ind)
{
  g_->add_edge(node1_ind, node2_ind);
}
IMPDOMINO2_END_INTERNAL_NAMESPACE
