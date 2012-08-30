/**
 *  \file ListPairContainer.cpp   \brief A list of ParticlePairsTemp.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/ListLikePairContainer.h>
#include <IMP/PairModifier.h>
#include <IMP/PairScore.h>

IMP_BEGIN_INTERNAL_NAMESPACE

void ListLikePairContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMP_END_INTERNAL_NAMESPACE
