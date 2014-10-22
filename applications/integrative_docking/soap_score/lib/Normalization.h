/**
 * \file Normalization.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_NORMALIZATION_H
#define IMP_NORMALIZATION_H

#include <IMP/atom/Residue.h>

#include <string>
#include <map>

class Normalization {
public:
  Normalization(const std::string& file_name) { read_table_file(file_name); }

  double get_normalization_score(const std::map<IMP::atom::ResidueType,
                                                int>& residue_content) const;

private:
  /* read residue content file in the following format:
  ALA 5.33290560e+03
  CYS 5.62388191e+04
  ASP 4.14313671e+04
  */
  void read_table_file(const std::string& file_name);

private:
  std::map<IMP::atom::ResidueType, double> normalization_table_;
};

#endif /* IMP_NORMALIZATION_H */
