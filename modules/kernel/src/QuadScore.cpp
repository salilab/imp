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
::create_current_decomposition(Model *m,
                               const ParticleIndexQuad& vt) const {
  return internal::create_score_current_decomposition(this,
                                                      m, vt);
}

IMP_INPUTS_DEF(QuadScore);

IMP_END_NAMESPACE
