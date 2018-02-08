/**
 *  \file Typed.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Typed.h>

IMPCORE_BEGIN_NAMESPACE

void Typed::show(std::ostream &out) const { out << get_type(); }
IMPCORE_END_NAMESPACE
