/**
 *  \file MoverBase.cpp  \brief A class to help implement movers.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MoverBase.h>
#include <IMP/macros.h>
#include <IMP/core/internal/singleton_helpers.h>

IMPCORE_BEGIN_NAMESPACE

IMP_LIST_IMPL(MoverBase, FloatKey, float_key, FloatKey,FloatKeys,{},{},{});
IMP_LIST_IMPL(MoverBase, IntKey, int_key, IntKey,IntKeys,{},{},{});

void MoverBase::propose_move(Float f)
{
  if (get_number_of_float_keys() != 0) {
    floats_.resize(pc_->get_number_of_particles(),
                   Floats(get_number_of_float_keys(), 0));
  }
  if (get_number_of_int_keys() != 0) {
    ints_.resize(pc_->get_number_of_particles(),
                 Ints(get_number_of_int_keys(), 0));
  }
  for (unsigned int i=0; i< pc_->get_number_of_particles(); ++i) {
    Particle *p = pc_->get_particle(i);
    for (unsigned int j=0; j< get_number_of_float_keys(); ++j) {
      floats_[i][j]= p->get_value(get_float_key(j));
    }
    for (unsigned int j=0; j< get_number_of_int_keys(); ++j) {
      ints_[i][j]= p->get_value(get_int_key(j));
    }
  }
  do_move(f);
}

void MoverBase::reset_move()
{
  IMP_FOREACH_SINGLETON(pc_, {
      for (unsigned int j=0; j< get_number_of_float_keys(); ++j) {
        _1->set_value(get_float_key(j), floats_[_2][j]);
      }
      for (unsigned int j=0; j< get_number_of_int_keys(); ++j) {
        _1->set_value(get_int_key(j), ints_[_2][j]);
      }
    });
}

IMPCORE_END_NAMESPACE
