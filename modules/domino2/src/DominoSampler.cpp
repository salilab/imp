/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino2/utility.h>

#include <IMP/domino2/internal/inference.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m, ParticleStatesTable* pst):
  Sampler(m, "Domino Sampler %1"),
  enumerators_(pst){
}

DominoSampler::DominoSampler(Model *m):
  Sampler(m, "Domino Sampler %1"),
  enumerators_(new ParticleStatesTable()){
}



namespace {
  std::vector<SubsetState> get_solutions(const SubsetGraph &jt,
                                  const Subset &known_particles,
                                  Model *model,
                                  ParticleStatesTable *pst,
                                  SubsetStatesTable *sst,
                                  SubsetEvaluatorTable *eval,
                                  double max_score) {
    const internal::PropagatedData pd
      = internal::get_best_conformations(jt, 0,
                                         known_particles,
                                         eval, sst,
                                         max_score);
    std::vector<SubsetState> final_solutions;
    for (internal::PropagatedData::ScoresIterator it= pd.scores_begin();
         it != pd.scores_end(); ++it) {
      final_solutions.push_back(it->first);
    }
    return final_solutions;
  }
}

ConfigurationSet *DominoSampler::do_sample() const {
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  set_was_used(true);
  Subset known_particles(enumerators_->get_particles(), true);
  IMP_LOG(TERSE, "Sampling with " << known_particles.size()
          << " particles" << std::endl);
  ParticlesTemp pt(known_particles.begin(), known_particles.end());
  InteractionGraph ig= get_interaction_graph(get_model(),
                                             pt);
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Interaction graph is " << boost::num_vertices(ig));
    // for some reason output to the ostringstream doesn't work
    //std::ostringstream oss;
    IMP::internal::show_as_graphviz(ig, std::cout);
    //oss << std::endl;
    //IMP_LOG(TERSE, oss.str() << std::endl);
  }
  SubsetGraph jt=get_junction_tree(ig);
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Junction tree is ");
    //std::ostringstream oss;
    IMP::internal::show_as_graphviz(jt, std::cout);
    //oss << std::endl;
    //IMP_LOG(TERSE, oss.str() << std::endl);
  }

  IMP::internal::OwnerPointer<SubsetStatesTable> sst;
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> set;
  if (evaluators_) {
    set= evaluators_;
  } else {
    set= new ModelSubsetEvaluatorTable(get_model(),
                                       get_particle_states_table());
    set->set_sampler(this);
  }
  if (node_enumerators_) {
    sst= node_enumerators_;
  } else {
    IMP_NEW(DefaultSubsetStatesTable, dsst, (get_particle_states_table()));
    dsst->set_sampler(this);
    dsst->set_subset_evaluator_table(set);
    sst=dsst;
  }

  std::vector<SubsetState> final_solutions= get_solutions(jt, known_particles,
                                                   get_model(),
                                                   get_particle_states_table(),
                                                   sst,
                                                   set,
                                                   get_maximum_score());

  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    IMP_LOG(TERSE, "Solutions is " << final_solutions[i] << std::endl);
    IMP_INTERNAL_CHECK(final_solutions[i].size()
                       == known_particles.size(),
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
  enumerators_->set_particle_states(p, se);
}
void DominoSampler::set_subset_evaluator_table(SubsetEvaluatorTable *eval) {
  evaluators_= eval;
  evaluators_->set_sampler(this);
}
void DominoSampler::set_subset_states_table(SubsetStatesTable *cse) {
  node_enumerators_= cse;
  node_enumerators_->set_sampler(this);
}

void DominoSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
