/**
 * \file IMP/integrative_docking/Normalization.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/integrative_docking/internal/Normalization.h>

#include <IMP/base/exception.h>
#include <boost/algorithm/string.hpp>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

void Normalization::read_table_file(const std::string& file_name) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IMP::base::IOException);
  }
  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line);  // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0') continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() != 2) continue;
    double val = atof(split_results[1].c_str());
    IMP::atom::ResidueType residue_type =
        IMP::atom::ResidueType(split_results[0]);
    normalization_table_[residue_type] = val;
  }
  in_file.close();
}

double Normalization::get_normalization_score(const std::map<IMP::atom::ResidueType,
                                              int>& residue_content) const {
  double score = 0;
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for (it = residue_content.begin(); it != residue_content.end(); it++) {
    score += it->second * normalization_table_.find(it->first)->second;
  }
  return score;
}

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE
