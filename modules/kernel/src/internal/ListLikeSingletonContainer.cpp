/**
 *  \file ListSingletonContainer.cpp   \brief A list of ParticlesTemp.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/ListLikeSingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/SingletonScore.h>

IMP_BEGIN_INTERNAL_NAMESPACE

void ListLikeSingletonContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMP_END_INTERNAL_NAMESPACE
