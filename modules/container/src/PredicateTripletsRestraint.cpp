/**
 *  \file PredicateTripletsRestraint.cpp   \brief Container for triplet.
 *
 *  WARNING This file was generated from PredicateNAMEsRestraint.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/PredicateTripletsRestraint.h"
#include <IMP/triplet_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

PredicateTripletsRestraint
::PredicateTripletsRestraint(TripletPredicate *pred,
                               TripletContainerAdaptor input,
                               std::string name):
  Restraint(input->get_model(), name),
  predicate_(pred), input_(input), updated_(false),
  error_on_unknown_(true){}

double
PredicateTripletsRestraint
::unprotected_evaluate(DerivativeAccumulator *da) const {
  update_lists_if_necessary();
  double ret=0;
  for (unsigned int i=0; i< restraints_.size(); ++i) {
    ret+=restraints_[i]->unprotected_evaluate(da);
  }
  return ret;
}

ParticlesTemp PredicateTripletsRestraint
::get_input_particles() const {
  // not correct, but correct is complicated
  return input_->get_all_possible_particles();
}
ContainersTemp PredicateTripletsRestraint
::get_input_containers() const {
  // List containers don't do anything interesting
  return ContainersTemp(1, input_);
}

Restraints PredicateTripletsRestraint
::do_create_current_decomposition() const {
  Restraints ret;
  for (unsigned int i=0; i< restraints_.size(); ++i) {
    base::Pointer<Restraint> r=restraints_[i]->create_current_decomposition();
    if (r) {
      RestraintSet *rs= dynamic_cast<RestraintSet*>(r.get());
      if (rs) {
        ret+=rs->get_restraints();
        // suppress warning
        rs->set_was_used(true);
      } else {
        ret.push_back(r);
      }
    }
  }
  return ret;
}

bool PredicateTripletsRestraint
::assign_pair(const ParticleIndexTriplet& index) const {
  int bin=predicate_->get_value_index(get_model(), index);
  Map::const_iterator it= containers_.find(bin);
  if (it == containers_.end()) {
    if (unknown_container_) {
      unknown_container_->add_particle_triplet(index);
      return true;
    } else if (error_on_unknown_) {
      IMP_THROW("Invalid predicate value of " << bin
                << " encounted for " << index,
                ValueException);
      return true;
    } else {
      return false;
    }
  } else {
    it->second->add_particle_triplet(index);
    return true;
  }
}
void PredicateTripletsRestraint
::update_lists_if_necessary() const {
  if (updated_ && !input_->get_is_changed()) return;
  updated_=true;
  if (unknown_container_) {
    unknown_container_->clear_particle_triplets();
  }
  for (Map::const_iterator it= containers_.begin();
       it != containers_.end(); ++it) {
    it->second->clear_particle_triplets();
  }
  int dropped=0;
  IMP_FOREACH_TRIPLET_INDEX(input_, {
      bool added=assign_pair(_1);
      if (!added) ++dropped;
    });
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    unsigned int total=dropped;
    for (Map::const_iterator it= containers_.begin();
         it != containers_.end(); ++it) {
      total+=it->second->get_number();
    }
    total+= unknown_container_->get_number();
    IMP_INTERNAL_CHECK(input_->get_number()==total,
                       "Wrong number of particles "
                       << total << "!=" << input_->get_number());
  }
}

void PredicateTripletsRestraint::do_show(std::ostream &) const {
}
IMPCONTAINER_END_NAMESPACE
