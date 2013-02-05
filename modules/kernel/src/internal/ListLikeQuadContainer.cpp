/**
 *  \file ListQuadContainer.cpp   \brief A list of ParticleQuadsTemp.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kernel/internal/ListLikeQuadContainer.h>
#include <IMP/kernel/QuadModifier.h>
#include <IMP/kernel/QuadScore.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

void ListLikeQuadContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMPKERNEL_END_INTERNAL_NAMESPACE
