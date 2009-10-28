/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/utility.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>
#include <IMP/core/internal/singleton_helpers.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

namespace {
  ListSingletonContainer* get_list(SingletonContainer *g) {
    return dynamic_cast<ListSingletonContainer*>(g);
  }
}

MovedSingletonContainer::MovedSingletonContainer(Model *m,
                                                 SingletonContainer *pc,
                                                 double threshold):
  threshold_(threshold),
  pc_(pc)
{
  set_added_and_removed_containers(new ListSingletonContainer(),
                                   new ListSingletonContainer());
  IMP_USAGE_CHECK(pc->get_number_of_particles() != 0,
                  "Cannot initialize from empty"
                  << " container.", ValueException);
  set_model(pc->get_particle(0)->get_model());
  first_call_=true;
}

IMP_LISTLIKE_SINGLETON_CONTAINER_DEF(MovedSingletonContainer);


void MovedSingletonContainer::show(std::ostream &out) const
{
  out << "MaximumChangeXYScoreState" << std::endl;
}



ObjectsTemp MovedSingletonContainer::get_input_objects() const {
  return ObjectsTemp(1,pc_);
}

void MovedSingletonContainer::do_after_evaluate() {
  get_list(get_added_singletons_container())->clear_particles();
  get_list(get_removed_singletons_container())->clear_particles();
}

void MovedSingletonContainer::do_before_evaluate()
{
  IMP_CHECK_OBJECT(pc_);
  if (first_call_) {
    first_call_=false;
    reset();
    data_=pc_->get_particles();
  }
  if (pc_->get_added_singletons_container()->get_number_of_particles() != 0
      || pc_->get_removed_singletons_container()->get_number_of_particles()
      != 0) {
    reset();
    data_ = pc_->get_particles();
  } else {
    update_list();
  }
  std::sort(data_.begin(), data_.end());
}


ParticlesTemp MovedSingletonContainer::get_state_input_particles() const {
  return pc_->get_particles();
}

void MovedSingletonContainer::reset()
{
  save();
  data_.clear();
  first_call_=false;
}


void MovedSingletonContainer::reset_moved()
{
  save_moved();
  data_.clear();
}

void MovedSingletonContainer::set_threshold(double d) {
  threshold_= d;
  // could be more efficient, but why bother
  reset();
}

IMP_ACTIVE_CONTAINER_DEF(MovedSingletonContainer);



IMPCORE_END_INTERNAL_NAMESPACE
