/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_UTILITY_H
#define IMP_INTERNAL_UTILITY_H

#include "../config.h"
#include "../Particle.h"
#include <boost/format.hpp>
#include <algorithm>
#include <sstream>

IMP_BEGIN_INTERNAL_NAMESPACE

//! \internal \return true if a passed particle is inactive
struct IsInactiveParticle
{
  bool operator()(Particle *p) const {
    return !p->get_is_active();
  }
};


inline std::string make_object_name(std::string templ, unsigned int index) {
  std::ostringstream oss;
  boost::format format(templ);
  format.exceptions(boost::io::no_error_bits );
  oss << format %index;
  return oss.str();
}

struct Counter {
  unsigned int count;
  Counter(): count(0){}
  operator unsigned int () const {return count;}
  Counter operator++() {
    ++count;
    return *this;
  }
  void operator=(unsigned int i) {
    count=i;
  }
};


inline
ParticlesTemp get_union(const ParticlesList &pl) {
  ParticlesTemp ret=pl[0];
  for (unsigned int i=1; i < pl.size(); ++i) {
    ret.insert(ret.end(), pl[i].begin(), pl[i].end());
  }
  return ret;
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_UTILITY_H */
