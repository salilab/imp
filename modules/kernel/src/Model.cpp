/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Model.h"
#include "IMP/kernel/Particle.h"
#include "IMP/kernel/internal/scoring_functions.h"
#include "IMP/base//set.h"

IMPKERNEL_BEGIN_NAMESPACE


//! Constructor
Model::Model(std::string name):
    RestraintSet(Restraint::ModelInitTag(), name)
{
  cur_stage_=internal::NOT_EVALUATING;
  gather_statistics_=false;
  eval_count_=0;
  set_was_used(true);
  first_call_=true;
  next_particle_=0;
  dependencies_dirty_=false;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  internal::FloatAttributeTable::set_masks(&this->Masks::read_mask_,
                                           &this->Masks::write_mask_,
                                           &this->Masks::add_remove_mask_,
                                           &this->Masks::read_derivatives_mask_,
                                           &this->Masks::write_derivatives_mask_
                                           );
  internal::StringAttributeTable::set_masks(&this->Masks::read_mask_,
                                            &this->Masks::write_mask_,
                                            &this->Masks::add_remove_mask_);
  internal::IntAttributeTable::set_masks(&this->Masks::read_mask_,
                                         &this->Masks::write_mask_,
                                         &this->Masks::add_remove_mask_);
  internal::ObjectAttributeTable::set_masks(&this->Masks::read_mask_,
                                  &this->internal::Masks::write_mask_,
                                  &this->Masks::add_remove_mask_);
  internal::WeakObjectAttributeTable::set_masks(&this->Masks::read_mask_,
                                  &this->internal::Masks::write_mask_,
                                  &this->Masks::add_remove_mask_);
  internal::IntsAttributeTable::set_masks(&this->Masks::read_mask_,
                                          &this->Masks::write_mask_,
                                          &this->Masks::add_remove_mask_);
  internal::ObjectsAttributeTable::set_masks(&this->Masks::read_mask_,
                                             &this->Masks::write_mask_,
                                             &this->Masks::add_remove_mask_);
  internal::ParticleAttributeTable::set_masks(&this->Masks::read_mask_,
                                              &this->Masks::write_mask_,
                                              &this->Masks::add_remove_mask_);
  internal::ParticlesAttributeTable::set_masks(&this->Masks::read_mask_,
                                               &this->Masks::write_mask_,
                                               &this->Masks::add_remove_mask_);
#endif
  // be careful as this calls back to model
  RestraintSet::set_model(this);
}


void Model::cleanup()
{
  IMP_CHECK_OBJECT(this);
  {
    ScoreStates rs(score_states_begin(), score_states_end());
    for (unsigned int i=0; i < rs.size(); ++i) {
      rs[i]->set_model(nullptr);
    }
  }
}

IMP_LIST_ACTION_IMPL(Model, ScoreState, ScoreStates, score_state,
                     score_states, ScoreState*,
                     ScoreStates);

RestraintSet *Model::get_root_restraint_set() {
  return this;
}

 double Model::get_maximum_score(Restraint *r) const {
  return r->get_maximum_score();
}
 void Model::set_maximum_score(Restraint *r, double s) {
  r->set_maximum_score(s);
}
 void Model::set_maximum_score(double s) {
  return RestraintSet::set_maximum_score(s);
}
 double Model::get_maximum_score() const {
  return RestraintSet::get_maximum_score();
}


ParticlesTemp Model::get_particles() const {
  return ParticlesTemp(particles_begin(),
                       particles_end());
}


void Model::add_particle_internal(Particle *p, bool set_name) {
    IMP_CHECK_OBJECT(this);
    IMP_CHECK_OBJECT(p);
    p->set_was_used(true);
    ParticleIndex id;
    if (free_particles_.empty()){
      id= ParticleIndex(next_particle_);
      ++next_particle_;
    } else {
      id= free_particles_.back();
      free_particles_.pop_back();
    }
    p->id_=id;
    int maxp= std::max<unsigned int>(particle_index_.size(),
                       get_as_unsigned_int(p->id_)+1);
    particle_index_.resize(maxp);
    particle_index_[p->id_]=p;
    if (set_name) {
      std::ostringstream oss;
      oss << boost::format("P%1%")
        % id;
      p->set_name(oss.str());
    }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    //xstd::cout << "Resizing to " << particle_index_.size() << std::endl;
    Masks::read_mask_.resize(particle_index_.size(), true);
    Masks::write_mask_.resize(particle_index_.size(), true);
    Masks::add_remove_mask_.resize(particle_index_.size(), true);
    Masks::read_derivatives_mask_.resize(particle_index_.size(), true);
    Masks::write_derivatives_mask_.resize(particle_index_.size(), true);
#endif
  }



void Model::update() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  before_evaluate(ordered_score_states_);
}

void Model::show_it(std::ostream& out) const
{
  out << get_particles().size() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}

void Model::remove_particle(Particle *p) {
  remove_particle(p->get_index());
}

void Model::remove_particle(ParticleIndex pi) {
  if (undecorators_index_.size() > static_cast<size_t>(pi.get_index())) {
    for (unsigned int i = 0; i < undecorators_index_[pi].size(); ++i) {
      undecorators_index_[pi][i]->teardown(pi);
    }
    undecorators_index_[pi].clear();
  }
  particle_index_[pi]->set_tracker(particle_index_[pi], nullptr);
  free_particles_.push_back(pi);
  particle_index_[pi]=nullptr;
  internal::FloatAttributeTable::clear_attributes(pi);
  internal::StringAttributeTable::clear_attributes(pi);
  internal::IntAttributeTable::clear_attributes(pi);
  internal::ObjectAttributeTable::clear_attributes(pi);
  internal::WeakObjectAttributeTable::clear_attributes(pi);
  internal::IntsAttributeTable::clear_attributes(pi);
  internal::ObjectsAttributeTable::clear_attributes(pi);
  internal::ParticleAttributeTable::clear_attributes(pi);
  internal::ParticlesAttributeTable::clear_attributes(pi);
#if 0
  IMP_IF_CHECK(base::USAGE) {
    for (unsigned int i=0; i< internal::ParticleAttributeTable::size();
         ++i) {
      for (unsigned int j=0; j < internal::ParticleAttributeTable::size(i);
           ++j) {
        if (internal::ParticleAttributeTable
            ::get_has_attribute(ParticleIndexKey(i),
                                ParticleIndex(j))) {
          ParticleIndex pc= internal::ParticleAttributeTable
            ::get_attribute(ParticleIndexKey(i),
                            ParticleIndex(j),
                            false);
          IMP_USAGE_CHECK(pc != pi,
                          "There is still a reference to removed particle "
                          << Showable(p) << " in particle "
                          << Showable(get_particle(ParticleIndex(j)))
                          << " attribute " << ParticleIndexKey(i));
        }
      }
    }
    for (unsigned int i=0; i< internal::ParticlesAttributeTable::size();
         ++i) {
      for (unsigned int j=0; j < internal::ParticlesAttributeTable::size(i);
           ++j) {
        if (internal::ParticlesAttributeTable
            ::get_has_attribute(ParticleIndexesKey(i),
                                ParticleIndex(j))) {
          ParticleIndexes pcs= internal::ParticlesAttributeTable
            ::get_attribute(ParticleIndexesKey(i),
                            ParticleIndex(j),
                            false);
          for (unsigned int k=0; k < pcs.size(); ++k) {
            ParticleIndex pc= pcs[k];
            IMP_USAGE_CHECK(pc != pi,
                            "There is still a reference to removed particle "
                            << Showable(p) << " in particle "
                            << Showable(get_particle(ParticleIndex(j)))
                            << " attribute " << ParticleIndexesKey(i));
          }
        }
      }
    }
  }
#endif
}


void Model::add_data(ModelKey mk, Object *o) {
  model_data_.resize(std::max<int>(mk.get_index()+1,
                                   model_data_.size()));
  model_data_[mk.get_index()]=o;
}
base::Object *Model::get_data(ModelKey mk) const {
  return model_data_[mk.get_index()].get();
}
void Model::remove_data(ModelKey mk) {
  model_data_[mk.get_index()]= nullptr;
}
bool Model::get_has_data(ModelKey mk) const {
  if (model_data_.size() > mk.get_index()) {
    return model_data_[mk.get_index()];
  } else {
    return false;
  }
}

Model::ParticleIterator Model::particles_begin() const {
  return ParticleIterator(NotNull(), particle_index_.begin(),
                          particle_index_.end());
}
Model::ParticleIterator Model::particles_end() const {
  return ParticleIterator(NotNull(), particle_index_.end(),
                          particle_index_.end());
}


ModelObjectsTemp Model::get_model_objects() const {
  return ModelObjectsTemp(ModelObjectTracker::tracked_begin(),
                          ModelObjectTracker::tracked_end());
}

ParticleIndex Model::add_particle(std::string name) {
  IMP_NEW(Particle, p, (this, name));
  return p->get_index();
}

void Model::add_undecorator(ParticleIndex pi, Undecorator *d) {
  undecorators_index_.resize(std::max<size_t>(pi.get_index() + 1,
                                              undecorators_index_.size()));
  undecorators_index_[pi].push_back(d);
}

IMPKERNEL_END_NAMESPACE
