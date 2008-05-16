/**
 *  \file MaxChangeScoreState.cpp
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include "IMP/score_states/MaxChangeScoreState.h"
#include "IMP/internal/utility.h"

#include <algorithm>
#include <sstream>

namespace IMP
{

MaxChangeScoreState::MaxChangeScoreState(const FloatKeys &keys,
                                         const Particles &ps): keys_(keys)
{
  add_particles(ps);
  origkeys_.resize(keys_.size());
  std::ostringstream oss;
  oss << " MCSS base " << this;
  for (unsigned int i=0; i< keys_.size(); ++i) {
    origkeys_[i]= FloatKey((keys_[i].get_string()+oss.str()).c_str());
  }
}


IMP_LIST_IMPL(MaxChangeScoreState, Particle, particle, Particle*,
              {for (unsigned int i=0; i< keys_.size(); ++i) {
                  IMP_check(obj->has_attribute(keys_[i]),
                            "Particle missing needed attribute " << keys_[i]
                            << obj,
                            ValueException("Particle missing attribute"));
                };
                for (unsigned int i=0; i< origkeys_.size(); ++i) {
                  if (!obj->has_attribute(origkeys_[i])) {
                    obj->add_attribute(origkeys_[i],
                                       obj->get_value(keys_[i]), false);
                  }
                }
              }, {reset();});

void MaxChangeScoreState::do_before_evaluate()
{
  max_change_=0;
  // get rid of inactive particles and their stored values
  internal::remove_inactive_particles(particle_vector_);
  for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
    (*it)->assert_is_valid();
    for (unsigned int j=0; j < keys_.size(); ++j) {
      Float v= (*it)->get_value(keys_[j]);
      Float ov= (*it)->get_value(origkeys_[j]);
      IMP_LOG(VERBOSE, "Particle " << (*it)->get_index() 
              << " and attribute " << keys_[j]
              << " moved " << std::abs(v - ov) << std::endl);
      max_change_= std::max(max_change_,
                            std::abs(v-ov));
    }
  }
  IMP_LOG(TERSE, "MaxChange update got " << max_change_ << std::endl); 
}


void MaxChangeScoreState::reset()
{
  for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
    for (unsigned int j=0; j < keys_.size(); ++j) {
      (*it)->set_value(origkeys_[j], (*it)->get_value(keys_[j]));
    }
  }
  max_change_=0;
}

void MaxChangeScoreState::show(std::ostream &out) const
{
  out << "MaxChangeScoreState" << std::endl;
}

} // namespace IMP
