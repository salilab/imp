/**
 *  \file ListTripletContainer.cpp   \brief A list of ParticleTripletsTemp.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.cc
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/ListLikeTripletContainer.h>
#include <IMP/TripletModifier.h>
#include <IMP/TripletScore.h>

IMP_BEGIN_INTERNAL_NAMESPACE

void ListLikeTripletContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMP_END_INTERNAL_NAMESPACE
