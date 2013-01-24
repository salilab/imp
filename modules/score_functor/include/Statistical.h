/**
 *  \file IMP/score_functor/Statistical.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_STATISTICAL_H
#define IMPSCORE_FUNCTOR_STATISTICAL_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/Model.h>
#include "internal/PMFTable.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

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
class Statistical: public Score {
  internal::PMFTable<BIPARTITE, INTERPOLATE, SPARSE> table_;
  double threshold_;
  IntKey key_;
public:
   /** \param[in] k The attribute to use for determining the particle types
      \param[in] threshold The maximum distance to score
      \param[in] data_file Where to load the file from.
  */
  Statistical(IntKey k,
              double threshold,
              base::TextInput data_file):
    table_(0),
    threshold_(threshold), key_(k){
    IMP_USAGE_CHECK(!BIPARTITE,
                    "Constructor can only be used for non-bipartite scores.");
    table_.template initialize<Key>(data_file);
  }
  /** \param[in] k The attribute to use for determining the particle types
      \param[in] threshold The maximum distance to score
      \param[in] data_file Where to load the file from.
      \param[in] shift The offset for the types of the second set of types.
      eg, if the score is on protein and ligand atoms, the ligand atom types
      start with the value shift.
  */
  Statistical(IntKey k,
              double threshold,
              base::TextInput data_file,
              unsigned int shift):
    table_(shift),
    threshold_(threshold), key_(k){
    IMP_USAGE_CHECK(BIPARTITE,
                    "Constructor can only be used for bipartite scores.");
    table_.template initialize<Key>(data_file);
  }

  // depend on get_is_trivially_zero
  double get_score(Model *m, const base::Array<2, ParticleIndex>& pp,
                   double distance) const {
    if (distance >= threshold_ || distance < 0.001) {
      return 0;
    }
    int pt= m->get_attribute(key_, pp[0]);
    int lt= m->get_attribute(key_, pp[1]);
    if (pt==-1 || lt==-1) return 0;
    return table_.get_score(pt, lt, distance);
  }
  DerivativePair get_score_and_derivative(Model *m,
                                          const base::Array<2,
                                                            ParticleIndex>&pp,
                                          double distance) const {
    if (distance >= threshold_ || distance < 0.001) {
      return DerivativePair(0,0);
    }
    int pt= m->get_attribute(key_, pp[0]);
    int lt= m->get_attribute(key_, pp[1]);
    if (pt==-1 || lt==-1) return DerivativePair(0,0);
    return  table_.get_score_with_derivative(pt,
                                             lt, distance);
   }
  double get_maximum_range(Model *,
                           const base::Array<2, ParticleIndex>& ) const {
    return std::min(threshold_, table_.get_max());
  }
  bool get_is_trivially_zero(Model *m,
                             const base::Array<2, ParticleIndex>& p,
                             double squared_distance) const {
    return squared_distance > algebra::get_squared(get_maximum_range(m,p));
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_STATISTICAL_H */
