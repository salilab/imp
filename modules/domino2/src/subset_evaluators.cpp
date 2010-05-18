/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/subset_evaluators.h>

IMPDOMINO2_BEGIN_NAMESPACE

SubsetEvaluator::~SubsetEvaluator(){}
SubsetEvaluatorTable::~SubsetEvaluatorTable(){}
SubsetEvaluator::SubsetEvaluator(Subset *s,
                                 ParticleStatesTable *t):
  table_(t), subset_(s) {
}

namespace {
  class ModelSubsetEvaluator: public SubsetEvaluator {
    mutable Pointer<Model> model_;
    Pointer<Configuration> cs_;
    Pointer<Subset> s_;
    double base_score_;
  public:
    ModelSubsetEvaluator(Subset *s, ParticleStatesTable*t,
                         Model *m, Configuration *cs,
                         double base_score): SubsetEvaluator(s, t),
                                             model_(m), cs_(cs),
                                             base_score_(base_score) {
    }
    IMP_SUBSET_EVALUATOR(ModelSubsetEvaluator);
  };
  double ModelSubsetEvaluator::get_score(const Ints &state) const{
    cs_->load_configuration();
    model_->set_is_incremental(true);
    for (unsigned int i=0; i< state.size(); ++i) {
      Particle *p= get_subset()->get_particle(i);
      Pointer<ParticleStates> ps
        =get_particle_states_table()->get_particle_states(p);
      ps->load_state(state[i], p);
    }
    return model_->evaluate(false)- base_score_;
  }
  void ModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

SubsetEvaluator *
ModelSubsetEvaluatorTable::get_subset_evaluator(Subset *s) const {
  return new ModelSubsetEvaluator(s, get_particle_states_table(),
                                  model_, cs_, cs_score_);
}

void ModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}

IMPDOMINO2_END_NAMESPACE
