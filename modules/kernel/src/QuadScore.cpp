/**
 *  \file QuadScore.cpp  \brief Define QuadScore
 *
 *  WARNING This file was generated from NAMEScore.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/QuadScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/quad_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

QuadScore::QuadScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}


Restraints
QuadScore
::create_current_decomposition(const ParticleQuad& vt) const {
  return Restraints(1,
     internal::create_tuple_restraint(const_cast<QuadScore*>(this),
                                     vt,
                                     get_name()));
}

IMP_END_NAMESPACE
