/**
 *  \file MoverBase.cpp  \brief A class to help implement movers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/optimizers/MoverBase.h"

IMP_BEGIN_NAMESPACE

IMP_LIST_IMPL(MoverBase, Particle, particle, Particle*,,);
IMP_LIST_IMPL(MoverBase, FloatKey, float_key, FloatKey,,);
IMP_LIST_IMPL(MoverBase, IntKey, int_key, IntKey,,);

void MoverBase::propose_move(float f)
{
  if (get_number_of_float_keys() != 0) {
    floats_.resize(get_number_of_particles(),
                   Floats(get_number_of_float_keys(), 0));
  }
  if (get_number_of_int_keys() != 0) {
    ints_.resize(get_number_of_particles(), Ints(get_number_of_int_keys(), 0));
  }
  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    Particle *p = get_particle(i);
    for (unsigned int j=0; j< get_number_of_float_keys(); ++j) {
      floats_[i][j]= p->get_value(get_float_key(j));
    }
    for (unsigned int j=0; j< get_number_of_int_keys(); ++j) {
      ints_[i][j]= p->get_value(get_int_key(j));
    }
  }
  generate_move(f);
}

void MoverBase::reject_move()
{
  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    Particle *p = get_particle(i);
    for (unsigned int j=0; j< get_number_of_float_keys(); ++j) {
      p->set_value(get_float_key(j), floats_[i][j]);
    }
    for (unsigned int j=0; j< get_number_of_int_keys(); ++j) {
      p->set_value(get_int_key(j), ints_[i][j]);
    }
  }
}

IMP_END_NAMESPACE
