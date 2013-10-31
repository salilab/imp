/**
 *  \file IMP/atom/SoapLoopPairScore.h
 *  \brief SOAP-LOOP scoring
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SOAP_LOOP_PAIR_SCORE_H
#define IMPATOM_SOAP_LOOP_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/SoapLoop.h>

IMPATOM_BEGIN_NAMESPACE

//! Score a pair of atoms using the SOAP-LOOP potential.
class SoapLoopPairScore
    : public score_functor::DistancePairScore<score_functor::SoapLoop> {
  typedef score_functor::DistancePairScore<score_functor::SoapLoop> P;

public:
  SoapLoopPairScore(std::string library)
      : P(score_functor::SoapLoop(library)) {}
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SOAP_LOOP_PAIR_SCORE_H */
