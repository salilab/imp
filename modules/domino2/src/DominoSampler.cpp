/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino2/utility.h>
#include <IMP/domino2/internal/JunctionTree.h>
#include <IMP/domino2/internal/RestraintEvaluator.h>
#include <IMP/domino2/internal/DominoOptimizer.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE

DominoSampler::DominoSampler(Model *m):
  Sampler(m, "Domino Sampler %1"),
  enumerators_(new ParticleStatesTable()),
  node_enumerators_(new DefaultSubsetStatesTable()),
  evaluators_(new ModelSubsetEvaluatorTable(m)){
}

struct IntsLess {
  bool operator()(const Ints &a, const Ints &b) const {
    IMP_USAGE_CHECK(a.size() == b.size(), "Sizes don't match "
                    << a.size() << " " << b.size());
    for (unsigned int i=0; i< a.size(); ++i ){
      if (a[i] < b[i]) return true;
      else if (a[i] > b[i]) return false;
    }
    return false;
  }
};

ConfigurationSet *DominoSampler::do_sample() const {
  IMP_OBJECT_LOG;
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  set_was_used(true);
  ParticlesTemp known_particles= enumerators_->get_particles();
  StringKey k=internal::node_name_key();
  for (unsigned int i=0; i< known_particles.size(); ++i) {
    std::ostringstream oss;
    oss << i;
    if (known_particles[i]->has_attribute(k)) {
      known_particles[i]->set_value(k, oss.str());
    } else {
      known_particles[i]->add_attribute(k, oss.str());
    }
  }

  InteractionGraph ig= get_interaction_graph(get_model());
  TextOutput dgraph= IMP::create_temporary_file();
  // dgraph.get_name()
  std::string jtreename;
  {
    TextOutput jtree= IMP::create_temporary_file();
    jtreename= jtree.get_name();
  }
  // call script to build jtree
  internal::JunctionTree jt;
  internal::read_junction_tree(jtreename,&jt);
  internal::RestraintEvaluator re;
  IMP_NEW(internal::DominoOptimizer, opt, (jt, get_model(), &re));
  internal::DiscreteSampler ds(get_particle_states_table(),
                               get_subset_states_table());
  opt->set_sampling_space(&ds);
  unsigned int numsol=5;
  do {
    // search for right number of solutions
    opt->set_number_of_solutions(numsol);
    opt->optimize(numsol);
    // check that last energy is greater than cutoff
    double score=-std::numeric_limits<double>::max();
    for (unsigned int i=0; i< numsol; ++i) {
      const internal::CombState *cs= opt->get_graph()->get_opt_combination(i);
      score= std::max(score,
                      static_cast<double>(opt->get_graph()
                                          ->move_to_configuration(*cs)));
    }
    if (score > get_maximum_score()) {
      break;
    } else {
      numsol*=2;
    }
  } while (true);
  /*container::ListSingletonContainers subsets;
  typedef std::map<Ints, double, IntsLess> Table;
  std::map<Subset*, Table> tables;
  for (unsigned int i=0; i< subsets.size(); ++i) {
    Pointer<SubsetStates> e= node_enumerators_->get_subset_states(subsets[i]);
    Pointer<SubsetEvaluator> eval=evaluators_->get_subset_evaluator(subsets[i]);
    unsigned int nstates=e->get_number_of_states();
    for (unsigned int j=0; j< nstates; ++j) {
      Ints state= e->get_state(j);
      double score= eval->get_score(state);
      tables[subsets[i]][state]=score;
    }
    }*/
  std::vector<Ints> final_solutions;

  for (unsigned int i=0; i< numsol; ++i) {
    const internal::CombState *cs= opt->get_graph()->get_opt_combination(i);
    internal::CombData cd= *cs->get_data();
    Ints sol(known_particles.size());
    for (unsigned int i=0; i< sol.size(); ++i) {
      sol[i]= cd[known_particles[i]];
    }
    final_solutions.push_back(sol);
  }

  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    IMP_INTERNAL_CHECK(final_solutions[i].size() == known_particles.size(),
                       "Number of particles doesn't match");
    ret->load_configuration(-1);
    for (unsigned int j=0; j< known_particles.size(); ++j) {
      Particle *p=known_particles[j];
      Pointer<ParticleStates> ps=enumerators_->get_particle_states(p);
      ps->load_state(final_solutions[i][j], p);
    }
    if (get_is_good_configuration()) {
      ret->save_configuration();
    }
  }
  return ret.release();
}

void DominoSampler::set_particle_states(Particle *p, ParticleStates *se) {
  enumerators_->set_enumerator(p, se);
}
void DominoSampler::set_subset_evaluator_table(SubsetEvaluatorTable *eval) {
  evaluators_= eval;
  evaluators_->set_particle_states_table(enumerators_);
}
void DominoSampler::set_subset_states_table(SubsetStatesTable *cse) {
  node_enumerators_= cse;
  node_enumerators_->set_particle_states_table(enumerators_);
}

void DominoSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
