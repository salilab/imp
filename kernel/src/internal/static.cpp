/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/static.h"
#include "IMP/Particle.h"
#include "IMP/compatibility/map.h"

IMP_BEGIN_INTERNAL_NAMESPACE
/*
  With all the static data in a single file, we needn't
  fuss about initialization and destruction order.
 */

// objects

compatibility::map<std::string, unsigned int> object_type_counts;


// exceptins

// The error message is already in the exception
bool print_exceptions=true;

VectorOfRefCounted<FailureHandler*> handlers;


 CheckLevel check_mode =
#if IMP_BUILD == IMP_FAST
   NONE;
#elif IMP_BUILD == IMP_RELEASE
  USAGE;
#else
  USAGE_AND_INTERNAL;
#endif



// logging
bool print_time;
boost::timer log_timer;

LogLevel log_level= TERSE;
unsigned int log_indent=0;
IMP_CHECK_CODE(double initialized=11111111);
std::ofstream fstream;
internal::LogStream stream;


// particle validation
typedef bool (*ParticleFunction)(Particle*);
std::vector<std::pair<ParticleFunction, ParticleFunction> > particle_validators;

IMP_END_INTERNAL_NAMESPACE


IMP_BEGIN_NAMESPACE
#if IMP_BUILD < IMP_FAST
unsigned int IMP::RefCounted::live_objects_=0;
compatibility::set<Object*> live_;

Strings Object::get_live_object_names() {
  Strings ret;
  for (compatibility::set<Object*>::const_iterator it = live_.begin();
       it != live_.end(); ++it) {
    ret.push_back((*it)->get_name());
  }
  return ret;
}
void Object::add_live_object(Object*o) {
  live_.insert(o);
}
void Object::remove_live_object(Object*o) {
  live_.erase(o);
}
#endif
IMP_END_NAMESPACE
