/**
 * \file IMP/score_functor/Soap.h
 * \brief  A Score on the distance between a pair of particles.
 *
 * \authors Dina Schneidman
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SOAP_H
#define IMPSCORE_FUNCTOR_SOAP_H

#include <IMP/score_functor/score_functor_config.h>
#include "Dope.h"
#include "Statistical.h"

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Score pairs of atoms based on SOAP.
/** This is a simple pairwise atomistic statistical potential. For more
    details, see
    Optimized atomic statistical potentials: Assessment of protein interfaces
    and loops. Dong GQ, Fan H, Schneidman-Duhovny D, Webb B, Sali A.
    Bioinformatics. 2013

    \note This potential is independent of the orientation of the pairs of
          particles. Use OrientedSoap instead for a score that takes
          orientation into account.

    You need to use IMP::atom::add_dope_score_data() to add the requisite
    atom type data to the particles being scored.
*/
class Soap : public Statistical<DopeType, false, false> {
  typedef Statistical<DopeType, false, false> P;

 public:

  //! Set up using the default SOAP-PP potential file.
  /** This uses soap_score.lib, which is the pairwise part of the SOAP-PP
      score (see https://salilab.org/SOAP/). This is designed for ranking
      docking solutions. The score should be applied to pairs of atoms
      from the two docked proteins.
      \note The full SOAP-PP score also includes a surface accessibility
            term, which can be applied using SingletonStatistical (but this
            is unnecessary for simple ranking of solutions).
   */
  Soap(double threshold = std::numeric_limits<double>::max())
      : P(get_soap_type_key(), threshold, get_data_path("soap_score.lib")) {}

  //! Set up SOAP using an arbitrary potential file
  Soap(double threshold, TextInput data_file)
      : P(get_soap_type_key(), threshold, data_file) {}

  static IntKey get_soap_type_key() {
    static const IntKey ik("dope atom type");
    return ik;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SOAP_H */
