/**
 *  \file IMP/atom/OrientedSoapPairScore.h
 *  \brief Orientation-dependent SOAP scoring
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H
#define IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/OrientedSoap.h>

IMPATOM_BEGIN_NAMESPACE

//! Score a pair of atoms using an orientation-dependent SOAP score.
/** The library files themselves, such as soap_loop.hdf5 or
    soap_protein_od.hdf5, are rather large (~1.5GB) and so are not included
    here. They can be downloaded separately from http://salilab.org/SOAP/.
*/
class OrientedSoapPairScore
    : public score_functor::DistancePairScore<score_functor::OrientedSoap> {
  typedef score_functor::DistancePairScore<score_functor::OrientedSoap> P;

public:
  OrientedSoapPairScore(std::string library)
      : P(score_functor::OrientedSoap(library)) {}
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H */
