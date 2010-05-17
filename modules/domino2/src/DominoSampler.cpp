/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>


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
  // extract dependency graph from the model
  // write out the graph to a file
  // run the junction tree python app on the file
  // read in results to define the Subsets
  container::ListSingletonContainers subsets;
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
  }
  /*  propagate up tree making sure sets of states overlap
      build final solutions into configuration set using StateEnumeratorTable
   */
  // assume filled
  std::vector<Ints> final_solutions;
  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    IMP_INTERNAL_CHECK(final_solutions[i].size() == known_particles.size(),
                       "Number of particles doesn't match");
    ret->load_configuration(-1);
    for (unsigned int j=0; j< known_particles.size(); ++j) {
      Particle *p=known_particles[j];
      Pointer<ParticleStates> ps=enumerators_->get_particle_states(p);
      ps->load_state(final_solutions[i][j], p);
    }
    ret->save_configuration();
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
