/**
 *  \file NameDecorator.h     \brief Simple name decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_NAME_DECORATOR_H
#define __IMP_NAME_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include <vector>
#include <deque>

namespace IMP
{

//! A single name decorator.
class IMPDLLEXPORT NameDecorator
{
  IMP_DECORATOR(NameDecorator);
protected:
  static bool keys_initialized_;
  static StringKey name_key_;

  static bool has_required_attributes(Particle *p) {
    return p->has_attribute(name_key_);
  }
  static void add_required_attributes(Particle *p) {
    p->add_attribute(name_key_, "No Name");
  }
  static void initialize_static_data();
public:



  void show(std::ostream &out, int level=0) const ;

  void set_name(String nm) {
    get_particle()->set_value(name_key_, nm);
  }

  const String get_name() const {
    return get_particle()->get_value(name_key_);
  }
};

IMP_OUTPUT_OPERATOR(NameDecorator);

}

#endif  /* __IMP_NAME_DECORATOR_H */
