/**
 * \file IMP/integrative_docking/Normalization.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_NORMALIZATION_H
#define IMPINTEGRATIVE_DOCKING_NORMALIZATION_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/atom/Residue.h>

#include <string>
#include <map>

IMPINTEGRATIVEDOCKING_BEGIN_NAMESPACE

class IMPINTEGRATIVEDOCKINGEXPORT Normalization {
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

IMPINTEGRATIVEDOCKING_END_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_NORMALIZATION_H */
