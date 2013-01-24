/**
 *  \file IMP/core/StatisticalPairScore.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_STATISTICAL_PAIR_SCORE_H
#define IMPCORE_STATISTICAL_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/Model.h>
#include <IMP/score_functor/DistancePairScore.h>
#include <IMP/score_functor/Statistical.h>
#include <IMP/Particle.h>
#include <IMP/PairScore.h>
#include <IMP/algebra/Vector3D.h>
#include "XYZ.h"
#include <IMP/file.h>
#include <limits>

IMPCORE_BEGIN_NAMESPACE
/** Create a pairwise statistical potential from a file. The Keys passed
    as a template argument are used to determine how to map the names of
    the types as described in the loaded file to indexes. That is, if
    atom::ResidueKey is passed as the Keys, the potential will expect a file
    which has one line for each pair of residue names.

    The expected file format is:
\verbatim
    bin_width number_a number_b [offset]
    key_0a key_0b bin0 bin1 bin2...
    key_1a key_1b bin0 bin1 bin2...
\endverbatim

    The order of the lines (after the first one) does not matter.
    The bin_width is how much distance is allocated per bin (the distance used
    is that between the points). number_a and number_b are the numbers of
    particle types (number_b should only be specified if BIPARTITE is set).

    \note The values read in the file are for bins. That is, the first bin
    is from offset to offset+width. The second is offset+width to offset+
    2width.
    As a result, when interpolation is used, the function achieves the
    bin value at the center of the bin.

    \param[in] Key is an IMP::Key which maps between names and indices
    \param[in] BIPARTITE If true, the two sets of particles being stored are
    different (e.g. a protein atom and a ligand atom), otherwise they are
    assumed to both be the same. Appropriate values must be provided
    in the file.
    \param[in] INTERPOLATE If true, even the scores without derivatives are
    spline interpolated. If false, only the evaluation of derivatives is
    interpolated with a spline.
*/
template <class Key, bool BIPARTITE, bool INTERPOLATE, bool SPARSE=false>
class StatisticalPairScore:
  public score_functor::DistancePairScore<score_functor::Statistical<Key,
                                                                     BIPARTITE,
                                                                    INTERPOLATE,
                                                                     SPARSE> > {
  typedef score_functor::Statistical<Key,
                                     BIPARTITE,
                                     INTERPOLATE,
                                     SPARSE> S;
  typedef score_functor::DistancePairScore<S > P;
public:
  /** \param[in] k The attribute to use for determining the particle types
      \param[in] threshold The maximum distance to score
      \param[in] data_file Where to load the file from.
  */
  StatisticalPairScore(IntKey k,
                       double threshold,
                       base::TextInput data_file):
    P(S(k, threshold, data_file))
      {  }
  /** \param[in] k The attribute to use for determining the particle types
      \param[in] threshold The maximum distance to score
      \param[in] data_file Where to load the file from.
      \param[in] shift The offset for the types of the second set of types.
      eg, if the score is on protein and ligand atoms, the ligand atom types
      start with the value shift.
  */
  StatisticalPairScore(IntKey k,
                       double threshold,
                       base::TextInput data_file,
                       unsigned int shift): P(S(k, threshold, data_file, shift))
      {}
};
IMPCORE_END_NAMESPACE

#endif /* IMPCORE_STATISTICAL_PAIR_SCORE_H */
