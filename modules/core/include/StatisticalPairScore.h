/**
 *  \file StatisticalPairScore.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPCORE_STATISTICAL_PAIR_SCORE_H
#define IMPCORE_STATISTICAL_PAIR_SCORE_H

#include "core_config.h"
#include "internal/PMFTable.h"
#include <IMP/Model.h>
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
    which has lines one per each pair of residue names.

    The expected file format is:
\verbatim
    bin_width number_a number_b [offset]
    key_0a key_0b bin0 bin1 bin2...
    key_1a key_1b bin0 bin1 bin2...
\endverbatim

    The order of the lines (after the first one) does not matter.
    The bin_width is how much distance is allocated per bin (the distance used
    is that between the points).

    \param[in] Key is an IMP::Key which maps between names and indices
    \param[in] BIPARTITE If true, the two sets of particles being stored are
    difference (eg a protein atom and a ligand atom), otherwise they are assumed
    to both be the same. Appropriate value must be provided in the file.
    \param[in] INTERPOLATE If true, even the scores without derivatives are
    spline interpolated. If false, only the evaluates with derivatives are
    interpolated with a spline.
*/
template <class Key, bool BIPARTITE, bool INTERPOLATE>
class StatisticalPairScore: public PairScore {
  core::internal::PMFTable<BIPARTITE, INTERPOLATE> table_;
  double threshold_;
  IntKey key_;
  inline double evaluate(const algebra::VectorD<3> &protein_v,
                         int iptype,
                         const algebra::VectorD<3> &ligand_v,
                         int iltype,
                         core::XYZ pxyz, core::XYZ lxyz,
                         DerivativeAccumulator *da) const{
    double distance = algebra::get_distance(protein_v, ligand_v);
    if (distance >= threshold_ || distance < 0.001) {
      return 0;
    }
    if (!da) {
      double v= table_.get_score(iptype, iltype, distance);
      return v;
    } else {
      DerivativePair dp= table_.get_score_with_derivative(iptype,
                                                          iltype, distance);
      algebra::VectorD<3> diff= protein_v-ligand_v;
      algebra::VectorD<3> norm= diff.get_unit_vector();
      pxyz.add_to_derivatives(dp.second*norm, *da);
      lxyz.add_to_derivatives(-dp.second*norm, *da);
      return dp.first;
    }
  }
public:
  /** \param[in] k The attribute to use for determining the particle types
      \param[in] threshold The maximum distance to score
      \param[in] data_file Where to load the file from.
  */
  StatisticalPairScore(IntKey k,
                       double threshold,
                       TextInput data_file):
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
  StatisticalPairScore(IntKey k,
                       double threshold,
                       TextInput data_file,
                       unsigned int shift):
    table_(shift),
    threshold_(threshold), key_(k){
    IMP_USAGE_CHECK(BIPARTITE,
                    "Constructor can only be used for bipartite scores.");
    table_.template initialize<Key>(data_file);
  }
  double get_maximum_distance() const {
    return std::min(threshold_, table_.get_max());
  }
  IMP_SIMPLE_PAIR_SCORE(StatisticalPairScore);
};
#ifndef IMP_DOXYGEN

template <class Key, bool BIPARTITE, bool INTERPOLATE>
void StatisticalPairScore<Key, BIPARTITE, INTERPOLATE>
::do_show(std::ostream &out) const {
}

template <class Key, bool BIPARTITE, bool INTERPOLATE>
double StatisticalPairScore<Key, BIPARTITE, INTERPOLATE>
::evaluate(const ParticlePair &pp,
           DerivativeAccumulator *da) const {
  int pt= pp[0]->get_value(key_);
  int lt= pp[1]->get_value(key_);
  core::XYZ pxyz(pp[0]);
  core::XYZ lxyz(pp[1]);
  algebra::VectorD<3> pv(pxyz.get_coordinates()),
    lv(lxyz.get_coordinates());
  if (pt==-1 || lt==-1) return 0;
  return evaluate(pv, pt, lv,lt, pxyz, lxyz, da);
}
#endif
IMPCORE_END_NAMESPACE

#endif /* IMPCORE_STATISTICAL_PAIR_SCORE_H */
