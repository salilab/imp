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

#ifndef IMP_NO_DEPRECATED

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPCORE_BEGIN_NAMESPACE
//! A simple decorator which controls the Particle description.
/** \ingroup helper
    \ingroup decorators
    \deprecated "Particles now have built in names"
 */
class IMPCOREEXPORT NameDecorator: public Decorator
{
  IMP_DECORATOR(NameDecorator, Decorator);
public:

  void set_name(std::string name) {
    get_particle()->set_name(name);
  }

  const std::string &get_name() const {
    return get_particle()->get_name();
  }

  //! Create a decorator with the name
  static NameDecorator create(Particle *p, std::string name="") {
    if (!name.empty()) p->set_name(name);
    return NameDecorator(p);
  }

  //! return true if it has a name
  static bool is_instance_of(Particle *p) {
    return true;
  }
};

IMP_OUTPUT_OPERATOR(NameDecorator);

IMPCORE_END_NAMESPACE

#endif // IMP_NO_DEPRECATED

#endif  /* IMPCORE_NAME_DECORATOR_H */
