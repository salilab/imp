/**
 *  \file MoverBase.cpp  \brief A class to help implement movers.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MoverBase.h>

IMPCORE_BEGIN_NAMESPACE

MoverBase::~MoverBase()
{
}



MoverBase::MoverBase(const ParticlesTemp &ps,
                     const FloatKeys &keys,
                     std::string name):
  Mover(IMP::internal::get_model(ps), name),
  keys_(keys),
  particles_(IMP::internal::get_index(ps)) {}


IMPCORE_END_NAMESPACE
