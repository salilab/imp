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
SubsetEvaluator::SubsetEvaluator(std::string name): Object(name){}
SubsetEvaluator::~SubsetEvaluator(){}
SubsetEvaluatorTable::~SubsetEvaluatorTable(){}


namespace {
  class ModelSubsetEvaluator: public SubsetEvaluator {
    mutable Pointer<Model> model_;
    Pointer<Configuration> cs_;
    Pointer<Subset> s_;
    Restraints restraints_;
    Pointer<ParticleStatesTable> pst_;
  public:
    ModelSubsetEvaluator(Subset *s,
                         const ParticlesTemp &sorted_dependents,
                         ParticleStatesTable*t,
                         Model *m, Configuration *cs):
      SubsetEvaluator("ModelSubsetEvaluator on "+s->get_name()),
      model_(m), cs_(cs),
      s_(s),
      pst_(t) {
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
  double ModelSubsetEvaluator::get_score(const SubsetState &state) const{
    cs_->load_configuration();
    for (unsigned int i=0; i< state.size(); ++i) {
      Particle *p= s_->get_particle(i);
      Pointer<ParticleStates> ps
        =pst_->get_particle_states(p);
      ps->load_state(state[i], p);
    }
    return model_->evaluate(restraints_, false);
  }
  void ModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

ModelSubsetEvaluatorTable::ModelSubsetEvaluatorTable(Model *m,
                                                     ParticleStatesTable *pst):
  model_(m),
  cs_(new Configuration(m, "evaluator base configuation")),
  pst_(pst){
  }

SubsetEvaluator *
ModelSubsetEvaluatorTable::get_subset_evaluator(Subset *s) const {
  if (dependents_.empty()) {
    Model::DependencyGraph dg= s->get_model()->get_dependency_graph();
    ParticlesTemp kp= pst_->get_particles();
    IMP_USAGE_CHECK(!kp.empty(),
                    "No particles in particles table");
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
  return new ModelSubsetEvaluator(s, sorted, pst_,
                                  model_, cs_);
}

void ModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}








#if 0



namespace {
  class CachingModelSubsetEvaluator: public SubsetEvaluator {
    Pointer<internal::CachingEvaluatorData> data_;
    Pointer<Subset> s_;
    Restraints restraints_;
    Pointer<ParticleStatesTable> pst_;
  public:
    CachingModelSubsetEvaluator(Subset *s,
                         const ParticlesTemp &sorted_dependents,
                                internal::CachingEvaluatorData* data):
      SubsetEvaluator("CachingModelSubsetEvaluator on "+s->get_name()),
      data_(data),
      s_(s) {
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
    IMP_SUBSET_EVALUATOR(CachingModelSubsetEvaluator);
  };
  double CachingModelSubsetEvaluator::get_score(const SubsetState &state) const{
    cs_->load_configuration();
    for (unsigned int i=0; i< state.size(); ++i) {
      Particle *p= s_->get_particle(i);
      Pointer<ParticleStates> ps
        =pst_->get_particle_states(p);
      ps->load_state(state[i], p);
    }
    return model_->evaluate(restraints_, false);
  }
  void CachingModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

CachingModelSubsetEvaluatorTable::CachingModelSubsetEvaluatorTable(Model *m,
                                                     ParticleStatesTable *pst):
  data_(new internal::CachingEvaluatorData(m, pst)) {
  }

SubsetEvaluator *
CachingModelSubsetEvaluatorTable::get_subset_evaluator(Subset *s) const {
  if (dependents_.empty()) {
    Model::DependencyGraph dg= s->get_model()->get_dependency_graph();
    ParticlesTemp kp= pst_->get_particles();
    IMP_USAGE_CHECK(!kp.empty(),
                    "No particles in particles table");
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
  return new CachingModelSubsetEvaluator(s, sorted, data_);
}

void CachingModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}
#endif
IMPDOMINO2_END_NAMESPACE
