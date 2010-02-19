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
#include <IMP/exception.h>
#include <IMP/file.h>
#include <cmath>
#include <vector>

IMPATOM_BEGIN_INTERNAL_NAMESPACE
struct PMFTable {
  double inverse_bin_width_;
  std::vector<std::vector<std::vector<float> > > data_;
  PMFTable(TextInput in);
  float get_score(unsigned int i, unsigned int j, double dist) const {
    IMP_USAGE_CHECK(i < data_.size(), "Out of range protein index " << i);
    IMP_USAGE_CHECK(j < data_[i].size(),
                    "Out of range ligand index " << i << " " << j);
    unsigned int index= static_cast<int>(std::floor(dist * inverse_bin_width_));
    if (index >= data_[i][j].size()) return 0;
    return -data_[i][j][index];
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_PROTEIN_LIGAND_INTERNAL_SCORE_H */
