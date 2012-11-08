/**
 *  \file TripletScore.cpp  \brief Define TripletScore
 *
 *  WARNING This file was generated from NAMEScore.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/TripletScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/triplet_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

TripletScore::TripletScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}


Restraints
TripletScore
::create_current_decomposition(const ParticleTriplet& vt) const {
  return internal::create_score_current_decomposition(this, vt);
}

IMP_INPUTS_DEF(TripletScore);

IMP_END_NAMESPACE
