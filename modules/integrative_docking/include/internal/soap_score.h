/**
 * \file IMP/integrative_docking/soap_score.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_SOAP_SCORE_H
#define IMPINTEGRATIVE_DOCKING_SOAP_SCORE_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/OrientedSoap.h>

#include <IMP/Model.h>

#include <IMP/algebra/standard_grids.h>

#include <IMP/atom/StereochemistryPairFilter.h>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

IMPINTEGRATIVEDOCKINGEXPORT
double soap_score(const IMP::score_functor::Soap* soap_score,
                  IMP::Model* model, IMP::ParticleIndexes& pis1,
                  IMP::ParticleIndexes& pis2,
                  float distance_threshold);

IMPINTEGRATIVEDOCKINGEXPORT
double oriented_soap_score(const IMP::score_functor::OrientedSoap* soap_score,
                           IMP::Model* model,
                           IMP::ParticleIndexes& pis1,
                           IMP::ParticleIndexes& pis2);

IMPINTEGRATIVEDOCKINGEXPORT
double oriented_soap_score(const IMP::score_functor::OrientedSoap* soap_score,
                           IMP::Model* model,
                           IMP::ParticleIndexes& pis,
                           IMP::atom::StereochemistryPairFilter* filter);

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_SOAP_SCORE_H */
