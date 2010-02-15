/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */


#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/utility.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>
#include <IMP/core/internal/singleton_helpers.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

namespace {
  internal::CoreListSingletonContainer* get_list(SingletonContainer *g) {
    return dynamic_cast<internal::CoreListSingletonContainer*>(g);
  }
}

MovedSingletonContainer::MovedSingletonContainer(Model *m,
                                                 SingletonContainer *pc,
                                                 double threshold):
  internal::ListLikeSingletonContainer("MovedSingletonContainer"),
  threshold_(threshold),
  pc_(pc)
{
  IMP_USAGE_CHECK(pc->get_number_of_particles() != 0,
                  "Cannot initialize from empty"
                  << " container.", ValueException);
  set_model(pc->get_particle(0)->get_model());
  first_call_=true;
}


void MovedSingletonContainer::do_show(std::ostream &out) const
{
}



ContainersTemp MovedSingletonContainer::get_input_containers() const {
  return ContainersTemp(1,pc_);
}

void MovedSingletonContainer::do_after_evaluate() {
  internal::ListLikeSingletonContainer::do_after_evaluate();
}

void MovedSingletonContainer::do_before_evaluate()
{
  IMP_CHECK_OBJECT(pc_);
  if (first_call_) {
    first_call_=false;
    reset();
    ParticlesTemp t=pc_->get_particles();
    update_list(t);
  }
  if (pc_->get_added_singletons_container()->get_number_of_particles() != 0
      || pc_->get_removed_singletons_container()->get_number_of_particles()
      != 0) {
    reset();
    ParticlesTemp t=pc_->get_particles();
    update_list(t);
  } else {
    update_list();
  }
}


ParticlesTemp MovedSingletonContainer::get_state_input_particles() const {
  return pc_->get_particles();
}

ContainersTemp MovedSingletonContainer::get_state_input_containers() const {
  return ContainersTemp(1, pc_);
}

void MovedSingletonContainer::reset()
{
  save();
  ParticlesTemp t;
  update_list(t);
  first_call_=false;
}


void MovedSingletonContainer::reset_moved()
{
  save_moved();
  ParticlesTemp t;
  update_list(t);
}

void MovedSingletonContainer::set_threshold(double d) {
  threshold_= d;
  // could be more efficient, but why bother
  reset();
}

IMP_ACTIVE_CONTAINER_DEF(MovedSingletonContainer);



IMPCORE_END_INTERNAL_NAMESPACE
