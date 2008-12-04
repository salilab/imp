/**
 *  \file NameDecorator.h     \brief Simple name decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_NAME_DECORATOR_H
#define IMPCORE_NAME_DECORATOR_H

#include "core_exports.h"
#include "internal/utility.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/DecoratorBase.h>

IMPCORE_BEGIN_NAMESPACE

//! A simple decorator which controls the Particle description.
/** \ingroup helper
    \ingroup decorators
 */
class IMPCOREEXPORT NameDecorator: public DecoratorBase
{
  IMP_DECORATOR(NameDecorator, DecoratorBase,
                return p->has_attribute(name_key_),
                {p->add_attribute(name_key_, "No Name");});
protected:
  static StringKey name_key_;

public:

  IMP_DECORATOR_GET_SET(name, name_key_, String, String);

};

IMP_OUTPUT_OPERATOR(NameDecorator);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_NAME_DECORATOR_H */
