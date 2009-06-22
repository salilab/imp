/**
 *  \file SimulationInfo.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/SimulationInfo.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Chain.h>
#include <IMP/core/XYZ.h>

#include <IMP/log.h>

#include <sstream>
#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE


SimulationInfo SimulationInfo::create(Particle *p, double dt, double T) {
  p->add_attribute(get_current_time_key(), 0, false);
  p->add_attribute(get_last_time_key(), 0, false);
  p->add_attribute(get_temperature_key(), T, false);
  p->add_attribute(get_maximum_time_step_key(), dt, false);
  IMP_check(dt > 0, "Time step must be positive", ValueException);
  IMP_check(T > 0, "Temperature mustbe positive", ValueException);
  return SimulationInfo(p);
}

void SimulationInfo::show(std::ostream &out, std::string prefix) const
{
  out << "Current time is " << get_current_time() << "\n";
  out << "Last time was " << get_last_time() << "\n";
  out << "Temperature is " << get_temperature() << "\n";
  out << std::endl;
}


FloatKey SimulationInfo::get_temperature_key() {
  static FloatKey k("temperature");
  return k;
}

FloatKey SimulationInfo::get_current_time_key() {
  static FloatKey k("current_time");
  return k;
}

FloatKey SimulationInfo::get_last_time_key() {
  static FloatKey k("last_time");
  return k;
}

FloatKey SimulationInfo::get_maximum_time_step_key() {
  static FloatKey k("maximum_time_step");
  return k;
}


IMPATOM_END_NAMESPACE
