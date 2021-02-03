/**
 * \file IMP/score_functor/SingletonStatistical.h
 * \brief statistical score for a single particle based on one parameter,
 * such as solvent accessibility
 *
 * \authors Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SINGLETON_STATISTICAL_H
#define IMPSCORE_FUNCTOR_SINGLETON_STATISTICAL_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/Pointer.h>
#include "internal/SASTable.h"

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Create a single key/single particle statistical potential from a file.
/** The Key passed as a template argument is used to determine how to map
    the name of the type as described in the loaded file to table index.
    That is, if atom::ResidueKey is passed as the Key, the potential will
    expect a file which has one line for each residue name.

    The expected file format is:
\verbatim
    bin_width number [offset]
    key_0 bin0 bin1 bin2...
    key_1 bin0 bin1 bin2...
\endverbatim

    The order of the lines (after the first one) does not matter.
    The bin_width is how much distance is allocated per bin (the distance used
    is that between the points). number is the number of particle types.

    \note The values read in the file are for bins. That is, the first bin
    is from offset to offset+width. The second is offset+width to offset+
    2width.
    As a result, when interpolation is used, the function achieves the
    bin value at the center of the bin.

    \param[in] Key is an IMP::Key which maps between names and indices
    \param[in] INTERPOLATE If true, even the scores without derivatives are
    spline interpolated. If false, only the evaluation of derivatives is
    interpolated with a spline.
*/
template <class Key, bool INTERPOLATE>
class SingletonStatistical {
  typedef internal::SASTable<INTERPOLATE> Table;
  PointerMember<Table> table_;
  IntKey key_;

 public:
  SingletonStatistical(IntKey k, TextInput data_file =
                                     get_data_path("soap_score_sas.lib"))
      : table_(new Table(data_file, Key())), key_(k) {}

  double get_score(Model *m, const ParticleIndex pi,
                   double area) const {
    int table_index = m->get_attribute(key_, pi);
    if (table_index == -1) return 0;
    return table_->get_score(table_index, area);
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SINGLETON_STATISTICAL_H */
