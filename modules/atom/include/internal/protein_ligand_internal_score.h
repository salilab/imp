/**
 *  \file protein_ligand_internal_score.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_INTERNAL_PROTEIN_LIGAND_INTERNAL_SCORE_H
#define IMPATOM_INTERNAL_PROTEIN_LIGAND_INTERNAL_SCORE_H

#include "../config.h"
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/exception.h>
#include <IMP/file.h>
#include <cmath>
#include <vector>

IMPATOM_BEGIN_INTERNAL_NAMESPACE
struct PMFTable {
  double inverse_bin_width_;
  double bin_width_;
  double max_;
  std::vector<std::vector< IMP::core::internal::RawOpenCubicSpline > > data_;
  PMFTable(TextInput in);
  double get_score(unsigned int i, unsigned int j, double dist) const {
    IMP_USAGE_CHECK(i < data_.size(), "Out of range protein index " << i);
    IMP_USAGE_CHECK(j < data_[i].size(),
                    "Out of range ligand index " << i << " " << j);
    if (dist >= max_) return 0;
    return data_[i][j].get_bin(dist, bin_width_, inverse_bin_width_);
  }
  DerivativePair get_score_with_derivative(unsigned int i,
                                           unsigned int j, double dist) const {
    IMP_USAGE_CHECK(i < data_.size(), "Out of range protein index " << i);
    IMP_USAGE_CHECK(j < data_[i].size(),
                    "Out of range ligand index " << i << " " << j);
    if (dist >= max_-.5*bin_width_) return DerivativePair(0,0);
    if (dist <= .5*bin_width_) return DerivativePair(get_score(i,j,dist), 0);
    // shift by .5 for the splines so as to be between the centers of the cells
    return data_[i][j].evaluate_with_derivative(dist-.5*bin_width_, bin_width_,
                                                inverse_bin_width_);
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_PROTEIN_LIGAND_INTERNAL_SCORE_H */
