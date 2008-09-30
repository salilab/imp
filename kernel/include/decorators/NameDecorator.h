/**
 *  \file NameDecorator.h     \brief Simple name decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_NAME_DECORATOR_H
#define __IMP_NAME_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "../DecoratorBase.h"
#include "utility.h"

namespace IMP
{

//! A simple decorator which controls the Particle description.
/** \ingroup helper
    \ingroup decorators
 */
class IMPDLLEXPORT NameDecorator: public DecoratorBase
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

}

#endif  /* __IMP_NAME_DECORATOR_H */
