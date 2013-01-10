/**
 * \file ResidueContent \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "ResidueContent.h"

#include <IMP/exception.h>
#include <IMP/utility.h>
#include <fstream>

#include <boost/algorithm/string.hpp>

void ResidueContent::read_content_file(const std::string& file_name) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IMP::IOException);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0') continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if (split_results.size() != 2) continue;
    int counter = atoi(split_results[1].c_str());
    IMP::atom::ResidueType residue_type =
      IMP::atom::ResidueType(split_results[0]);
    residue_content_[residue_type] = counter;
  }
  in_file.close();
}

void ResidueContent::add_residue(IMP::atom::ResidueType rt) {
  if(residue_content_.find(rt) == residue_content_.end())
    residue_content_[rt] = 1;
  else
    residue_content_[rt]++;
}

void ResidueContent::add_residue(IMP::atom::ResidueType rt, int counter) {
  residue_content_[rt] = counter;
}

int ResidueContent::score(ResidueContent& other_rc) const {
  int score = 0;
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for(it = residue_content_.begin(); it != residue_content_.end(); it++) {
    score += std::abs(it->second - other_rc.get_counter(it->first));
  }
  return score;
}

int ResidueContent::score2(ResidueContent& other_rc) const {
  int score = 0;
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for(it = residue_content_.begin(); it != residue_content_.end(); it++) {
    score += IMP::square(it->second - other_rc.get_counter(it->first));
  }
  return score;
}

int ResidueContent::get_counter(IMP::atom::ResidueType rt) const {
  if(residue_content_.find(rt) == residue_content_.end())
    return 0;
  else
    return residue_content_.find(rt)->second;
}

int ResidueContent::get_total_counter() const {
  int counter = 0;
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for(it = residue_content_.begin(); it != residue_content_.end(); it++)
    counter += it-> second;
  return counter;
}

int ResidueContent::get_total_counter2() const {
  int counter = 0;
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for(it = residue_content_.begin(); it != residue_content_.end(); it++)
    counter += IMP::square(it->second);
  return counter;
}

std::ostream & operator<<(std::ostream & s, const ResidueContent& rc)
{
  //int counter = rc.get_total_counter();
  std::map<IMP::atom::ResidueType, int>::const_iterator it;
  for(it = rc.residue_content_.begin(); it != rc.residue_content_.end(); it++) {
    s << it->first.get_string() << " " << it->second << " " << std::endl;
  }
  return s;
}
