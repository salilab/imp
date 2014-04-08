/**
 *  \file IMP/atom/OrientedSoapPairScore.h
 *  \brief Orientation-dependent SOAP scoring
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H
#define IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/OrientedSoap.h>
#include <IMP/atom/SoapPairFilter.h>

IMPATOM_BEGIN_NAMESPACE

//! Score a pair of atoms using an orientation-dependent SOAP score.
/** Orientation-dependent SOAP scores include those that score loops
    (SOAP-Loop), protein-peptide interfaces (SOAP-Peptide) and proteins
    (SOAP-Protein). The library files themselves, such as soap_loop.hdf5 or
    soap_protein_od.hdf5, are rather large (~1.5GB) and so are not included
    here. They can be downloaded separately from http://salilab.org/SOAP/.
*/
class OrientedSoapPairScore
    : public score_functor::DistancePairScore<score_functor::OrientedSoap> {
  typedef score_functor::DistancePairScore<score_functor::OrientedSoap> P;
  std::string library_;

 public:
  OrientedSoapPairScore(std::string library)
      : P(score_functor::OrientedSoap(library)), library_(library) {}

  //! Get a filter that returns pairs that this score should use
  SoapPairFilter *get_pair_filter() { return new SoapPairFilter(library_); }
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ORIENTED_SOAP_PAIR_SCORE_H */
