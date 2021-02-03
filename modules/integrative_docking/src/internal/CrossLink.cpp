/**
 * \file IMP/integrative_docking/CrossLink.cpp \brief
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/integrative_docking/internal/CrossLink.h>

#include <IMP/exception.h>
#include <IMP/check_macros.h>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

int read_cross_link_file(const std::string& file_name,
                         std::vector<CrossLink>& cross_links) {
  std::ifstream s(file_name.c_str());
  if (!s) {
    IMP_THROW("Can't find cross links file " << file_name,
              IMP::IOException);
  }
  CrossLink cl;
  while(s) {
    if( s >> cl) cross_links.push_back(cl);
  }
  std::cout << cross_links.size() << " cross links were read from file "
            << file_name << std::endl;
  return cross_links.size();
}

void write_cross_link_file(const std::string& file_name,
                           const std::vector<CrossLink>& cross_links,
                           bool include_actual_distance) {
  std::ofstream ofile(file_name.c_str());
  for (unsigned int i = 0; i < cross_links.size(); i++) {
    ofile << cross_links[i];
    if (include_actual_distance)
      ofile << " " << cross_links[i].get_actual_distance() << " "
            << cross_links[i].get_actual_cb_distance();
    ofile << std::endl;
  }
  ofile.close();
}

std::ostream& operator<<(std::ostream& s, const CrossLink& cl) {
  s << cl.residue_number1_ << " ";
  if (cl.chain_id1_ == " ")
    s << "-";
  else
    s << cl.chain_id1_;
  s << " " << cl.residue_number2_ << " ";
  if (cl.chain_id2_ == " ")
    s << "-";
  else
    s << cl.chain_id2_;
  s << " " << cl.min_distance_ << " " << cl.max_distance_ << " " << cl.weight_;
  return s;
}

std::istream& operator>>(std::istream& s, CrossLink& cl) {
  s >> cl.residue_number1_ >> cl.chain_id1_ >> cl.residue_number2_ >>
      cl.chain_id2_ >> cl.min_distance_ >> cl.max_distance_;
  if (cl.chain_id1_ == "-") cl.chain_id1_ = " ";
  if (cl.chain_id2_ == "-") cl.chain_id2_ = " ";

  int c = s.peek();
  if(c == ' ') { // read weight
    s >> cl.weight_;
  } else {
    cl.weight_ = 1.0;
  }

  return s;
}

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE
