/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"
#include "IMP/dependency_graph.h"
#include "IMP/compatibility/set.h"
#include <boost/format.hpp>

IMP_BEGIN_NAMESPACE


//! Constructor
Model::Model(std::string name):
  Object(name),
  //#if IMP_BUILD < IMP_FAST
  rs_(new RestraintSet())
{
  cur_stage_=internal::NOT_EVALUATING;
  gather_statistics_=false;
  eval_count_=0;
  set_was_used(true);
  rs_->set_model(this);
  rs_->set_name("root restraint set");
  first_call_=true;
  max_score_ =std::numeric_limits<double>::max();
  has_good_score_=false;
  next_particle_=0;
#if IMP_BUILD < IMP_FAST
  FloatAttributeTable::set_masks(&this->Masks::read_mask_,
                                 &this->Masks::write_mask_,
                                 &this->Masks::add_remove_mask_,
                                 &this->Masks::read_derivatives_mask_,
                                 &this->Masks::write_derivatives_mask_);
  StringAttributeTable::set_masks(&this->Masks::read_mask_,
                                  &this->Masks::write_mask_,
                                  &this->Masks::add_remove_mask_);
  IntAttributeTable::set_masks(&this->Masks::read_mask_,
                               &this->Masks::write_mask_,
                               &this->Masks::add_remove_mask_);
  ObjectAttributeTable::set_masks(&this->Masks::read_mask_,
                                  &this->Masks::write_mask_,
                                  &this->Masks::add_remove_mask_);
  IntsAttributeTable::set_masks(&this->Masks::read_mask_,
                                &this->Masks::write_mask_,
                                &this->Masks::add_remove_mask_);
  ObjectsAttributeTable::set_masks(&this->Masks::read_mask_,
                                   &this->Masks::write_mask_,
                                   &this->Masks::add_remove_mask_);
  ParticleAttributeTable::set_masks(&this->Masks::read_mask_,
                                    &this->Masks::write_mask_,
                                    &this->Masks::add_remove_mask_);
  ParticlesAttributeTable::set_masks(&this->Masks::read_mask_,
                                     &this->Masks::write_mask_,
                                     &this->Masks::add_remove_mask_);
#endif
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  rs_->set_model(nullptr);
  for (unsigned int i=0; i< particle_index_.size(); ++i) {
    if (particle_index_[i]) {
      IMP_CHECK_OBJECT(particle_index_[i]);
      Particle* op=particle_index_[i];
      op->m_=nullptr;
    }
  }
}

void Model::set_maximum_score(double d) {
  max_score_=d;
  reset_dependencies();
}

void Model::add_restraint(Restraint *r) {
  IMP_USAGE_CHECK(r, "Cannot add null restraint.");
  rs_->add_restraint(r);
}
void Model::remove_restraint(Restraint *r) {
  IMP_USAGE_CHECK(r, "Cannot remove null restraint.");
  rs_->remove_restraint(r);
}
Model::RestraintIterator Model::restraints_begin() {
  return rs_->restraints_begin();
}
Model::RestraintIterator Model::restraints_end() {
  return rs_->restraints_end();
}
Model::RestraintConstIterator Model::restraints_begin() const {
  return rs_->restraints_begin();
}
Model::RestraintConstIterator Model::restraints_end() const {
  return rs_->restraints_end();
}

void Model::add_tracked_restraint(Restraint *r) {
  IMP_OBJECT_LOG;
  if (dynamic_cast<RestraintSet*>(r)) {
    // ignore it
    return;
  }
  IMP_LOG(VERBOSE, "Adding tracked restraint " << r->get_name()
          << std::endl);
  reset_dependencies();
  IMP_USAGE_CHECK(tracked_restraints_.find(r)
                  == tracked_restraints_.end(),
                  "Tracked restraint found on add");
  tracked_restraints_.insert(r);

}
void Model::remove_tracked_restraint(Restraint *r) {
  if (dynamic_cast<RestraintSet*>(r)) {
    // ignore it
    return;
  }
  reset_dependencies();
  IMP_USAGE_CHECK(tracked_restraints_.find(r)
                  != tracked_restraints_.end(),
                  "Tracked restraint not found on removal");
  tracked_restraints_.erase(r);
}

IMP_LIST_ACTION_IMPL(Model, ScoreState, ScoreStates, score_state,
                     score_states, ScoreState*,
                     ScoreStates);

void Model::set_score_state_model(ScoreState *ss, Model *model) {
  IMP_CHECK_OBJECT(ss);
  if (model) {
    IMP_CHECK_OBJECT(model);
  }
  ss->set_model(model);
}


double Model::get_weight(Restraint *r) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  return r->model_weight_;
}


ParticlesTemp Model::get_particles() const {
  return ParticlesTemp(particles_begin(),
                       particles_end());
}


void Model::add_particle_internal(Particle *p, bool set_name) {
    IMP_CHECK_OBJECT(this);
    IMP_CHECK_OBJECT(p);
    p->set_was_used(true);
    int id;
    if (free_particles_.empty()){
      id= next_particle_;
      ++next_particle_;
    } else {
      id= free_particles_.back();
      free_particles_.pop_back();
    }
    p->id_=id;
    int maxp= std::max(particle_index_.size(),
                       static_cast<size_t>(p->id_+1));
    particle_index_.resize(maxp);
    particle_index_[p->id_]=p;
    if (set_name) {
      std::ostringstream oss;
      oss << boost::format("P%1%")
        % id;
      p->set_name(oss.str());
    }
#if IMP_BUILD < IMP_FAST
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
  Floats ret= do_evaluate(RestraintsTemp(),
                          ordered_score_states_,
                          false, true, false);
}

void Model::do_show(std::ostream& out) const
{
  out << get_particles().size() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}

void Model::remove_particle(Particle *p) {
  int pi= p->get_index();
  free_particles_.push_back(pi);
  p->m_=nullptr;
  particle_index_[pi]=nullptr;
  FloatAttributeTable::clear_attributes(pi);
  StringAttributeTable::clear_attributes(pi);
  IntAttributeTable::clear_attributes(pi);
  ObjectAttributeTable::clear_attributes(pi);
  IntsAttributeTable::clear_attributes(pi);
  ObjectsAttributeTable::clear_attributes(pi);
  ParticleAttributeTable::clear_attributes(pi);
  ParticlesAttributeTable::clear_attributes(pi);
  IMP_IF_CHECK(USAGE) {
    ParticlesTemp cp= get_particles();
    for (unsigned int i=0; i< particle_index_.size(); ++i) {
      if (particle_index_[i]) {
        ParticleIndex cur=i;
        {
          ParticleKeys keys
            = ParticleAttributeTable::get_attribute_keys(cur);
          for (unsigned int j=0; j< keys.size(); ++j) {
            if (get_has_attribute(keys[j], cur)) {
              IMP_USAGE_CHECK(get_attribute(keys[j], cur) != pi,
                              "There is still a reference to"
                              << " removed particle in"
                              " particle "
                              << particle_index_[i]->get_name()
                              << " attribute "
                              << keys[j]);
            }
          }
        }
        {
          ParticlesKeys keys
            = ParticlesAttributeTable::get_attribute_keys(cur);
          for (unsigned int j=0; j< keys.size(); ++j) {
            if (get_has_attribute(keys[j], cur)) {
              ParticleIndexes pis
                = get_attribute(keys[j], cur);
              for (unsigned int k=0; k< pis.size(); ++k) {
                IMP_USAGE_CHECK(pis[k] != pi,
                                "There is still a reference to "
                                << "removed particle in"
                                << " particle "
                                << particle_index_[i]->get_name()
                                << " attribute "
                                << keys[j]);
              }
            }
          }
        }
      }
    }
  }
}


bool Model::get_has_good_score() const {
  return has_good_score_;
}

IMP_END_NAMESPACE
