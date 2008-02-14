/**
 *  \file MoverBase.cpp  \brief A class to help implement movers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/optimizers/MoverBase.h"

namespace IMP
{

void MoverBase::propose_move(float f)
{
  if (!fkeys_.empty()) {
    floats_.resize(number_of_particles(), Floats(fkeys_.size(), 0));
  }
  if (!ikeys_.empty()) {
    ints_.resize(number_of_particles(), Ints(ikeys_.size(), 0));
  }
  for (unsigned int i=0; i< number_of_particles(); ++i) {
    Particle *p = get_particle(i);
    for (unsigned int j=0; j< fkeys_.size(); ++j) {
      floats_[i][j]= p->get_value(fkeys_[j]);
    }
    for (unsigned int j=0; j< ikeys_.size(); ++j) {
      ints_[i][j]= p->get_value(ikeys_[j]);
    }
  }
  generate_move(f);
}

void MoverBase::reject_move()
{
  for (unsigned int i=0; i< number_of_particles(); ++i) {
    Particle *p = get_particle(i);
    for (unsigned int j=0; j< fkeys_.size(); ++j) {
      p->set_value(fkeys_[j], floats_[i][j]);
    }
    for (unsigned int j=0; j< ikeys_.size(); ++j) {
      p->set_value(fkeys_[j], floats_[i][j]);
    }
  }
}

} // namespace IMP
