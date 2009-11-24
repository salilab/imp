/**
 *  \file DominoOptimizer.cpp  \brief An exact inference optimizer
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/domino/DominoOptimizer.h>

IMPDOMINO_BEGIN_NAMESPACE

namespace {
  Particle* get_particle(Model *m, unsigned int i) {
    IMP_INTERNAL_CHECK(i>=1, "Out of range index in hacked function");
    --i;
    Model::ParticleIterator pit= m->particles_begin();
    while (i != 0) {
      ++pit;
      --i;
    }
    return *pit;
  }
}

void DominoOptimizer::show(std::ostream &out) const {
  out << "DOMINO optimizer" << std::endl;
}

DominoOptimizer::DominoOptimizer(const JunctionTree &jt, Model *m,
                                 internal::RestraintEvaluator *r_eval)
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
  Restraint *r; Particles ps; Float w;
  for(std::vector<OptTuple>::iterator it = rs_.begin(); it != rs_.end();it++) {
    r = boost::get<0>(*it);
    ps = boost::get<1>(*it);
    w = boost::get<2>(*it);
    g_->initialize_potentials(r,&ps,w);
  }
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
  IMP_LOG(VERBOSE," DominoOptimizer::optimize going to set sampling space"
                  <<std::endl);
  set_sampling_space(ds_);
  // now infer the minimum
  IMP_LOG(VERBOSE," DominoOptimizer::optimize going to infer solutions"
                  <<std::endl);
  g_->infer(num_of_solutions_);
  //move the model to the states that reach the global minimum
  g_->move_to_global_minimum_configuration();
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE," DominoOptimizer::optimize going to move the "
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
    particles.push_back(get_particle(&m, *it));
  }
  g_->add_node(node_index,particles,rstr_eval_);
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
     IMP_LOG(VERBOSE,"adding a restraint set"<<std::endl);
     for (Model::RestraintIterator it = rs_set->restraints_begin();
          it != rs_set->restraints_end(); it++) {
       add_restraint_recursive(*it, weight*rs_set->get_weight());
     }
   }
   else {
     IMP_LOG(VERBOSE,"adding a single restraint " << std::endl);
     IMP_LOG(VERBOSE,"number of interacting particles is :"
             <<rs->get_interacting_particles().size()<<std::endl);
     IMP_USAGE_CHECK(rs->get_interacting_particles().size()==1,
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
void DominoOptimizer::add_restraint(Restraint *r,Particles ps,float weight) {
  rs_.push_back(OptTuple(r,ps,weight));
}
IMPDOMINO_END_NAMESPACE
