/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/subset_evaluators.h>
#include <IMP/domino2/utility.h>
#include <IMP/Restraint.h>

IMPDOMINO2_BEGIN_NAMESPACE

SubsetEvaluator::~SubsetEvaluator(){}
SubsetEvaluatorTable::~SubsetEvaluatorTable(){}
SubsetEvaluator::SubsetEvaluator(Subset *s,
                                 ParticleStatesTable *t):
  table_(t), subset_(s) {
}


/**
   Strategy for decomposable evaluation:
   - use disactivate all particles that are not used
   - have model compute which restraints are still usable
   - restraints have a method to see if they are decomposable,
   if so an evaluate method with a bool at the end is called
   to say if it is partial or not. Hidden methods will, when
   appropriate, have a partial version.

   removing a particle causes the model to invalidate the list of
   restraint dependencies
 */
namespace {
  class ModelSubsetEvaluator: public SubsetEvaluator {
    mutable Pointer<Model> model_;
    Pointer<Configuration> cs_;
    Pointer<Subset> s_;
    Restraints restraints_;
  public:
    ModelSubsetEvaluator(Subset *s,
                         const ParticlesTemp &sorted_dependents,
                         ParticleStatesTable*t,
                         Model *m, Configuration *cs): SubsetEvaluator(s, t),
                                             model_(m), cs_(cs) {
      for (Model::RestraintIterator rit= model_->restraints_begin();
           rit != model_->restraints_end(); ++rit) {
        ParticlesTemp in= (*rit)->get_input_particles();
        std::sort(in.begin(), in.end());
        in.erase(std::unique(in.begin(), in.end()), in.end());
        ParticlesTemp inter;
        std::set_intersection(in.begin(), in.end(), sorted_dependents.begin(),
                              sorted_dependents.end(),
                              std::back_inserter(inter));
        if (inter.size() == in.size()) {
          restraints_.push_back(*rit);
        }
      }
    }
    IMP_SUBSET_EVALUATOR(ModelSubsetEvaluator);
  };
  double ModelSubsetEvaluator::get_score(const Ints &state) const{
    cs_->load_configuration();
    for (unsigned int i=0; i< state.size(); ++i) {
      Particle *p= get_subset()->get_particle(i);
      Pointer<ParticleStates> ps
        =get_particle_states_table()->get_particle_states(p);
      ps->load_state(state[i], p);
    }
    return model_->evaluate(restraints_, false);
  }
  void ModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

ModelSubsetEvaluatorTable::ModelSubsetEvaluatorTable(Model *m): model_(m),
    cs_(new Configuration(m, "evaluator")){
  }

SubsetEvaluator *
ModelSubsetEvaluatorTable::get_subset_evaluator(Subset *s) const {
  if (dependents_.empty()) {
    Model::DependencyGraph dg= s->get_model()->get_dependency_graph();
    ParticlesTemp kp= get_particle_states_table()->get_particles();
    for (unsigned int i=0; i< kp.size(); ++i) {
      dependents_[kp[i]]= get_dependent_particles(kp[i], dg);
    }
  }

  ParticlesTemp sorted;
  for (unsigned int i=0; i< s->get_number_of_particles(); ++i) {
    sorted.insert(sorted.end(),
                  dependents_.find(s->get_particle(i))->second.begin(),
                  dependents_.find(s->get_particle(i))->second.end());
  }
  std::sort(sorted.begin(), sorted.end());
  sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
  return new ModelSubsetEvaluator(s, sorted, get_particle_states_table(),
                                  model_, cs_);
}

void ModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}

IMPDOMINO2_END_NAMESPACE
