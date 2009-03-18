/**
 *  \file NameDecorator.h     \brief Simple name decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_NAME_DECORATOR_H
#define IMPCORE_NAME_DECORATOR_H

#include "config.h"
#include "internal/utility.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPCORE_BEGIN_NAMESPACE

//! A simple decorator which controls the Particle description.
/** \ingroup helper
    \ingroup decorators
 */
class IMPCOREEXPORT NameDecorator: public Decorator
{
  IMP_DECORATOR(NameDecorator, Decorator);
public:

  IMP_DECORATOR_GET_SET(name, get_name_key(), String, String);

  //! Create a decorator with the name
  static NameDecorator create(Particle *p, std::string name="new_name") {
    p->add_attribute(get_name_key(), name);
    return NameDecorator(p);
  }

  //! return true if it has a name
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_name_key());
  }

  //! Return the key used to store the name
  static StringKey get_name_key();
};

IMP_OUTPUT_OPERATOR(NameDecorator);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_NAME_DECORATOR_H */
