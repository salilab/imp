/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/utility.h"
#include "IMP/log.h"

IMP_BEGIN_NAMESPACE


Ownable::~Ownable() {
  if (!was_owned_) {
    IMP_WARN("Object " << this << " destroyed without being owned");
  }
}

IMP_END_NAMESPACE
