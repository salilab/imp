/**
 * \file CrossLink \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "CrossLink.h"

#include <IMP/exception.h>

int read_cross_link_file(const std::string& file_name,
                         std::vector<CrossLink>& cross_links) {
  std::ifstream s(file_name.c_str());
  if(!s) {
    IMP_THROW("Can't find cross links file " << file_name, IOException);
  }
  CrossLink cl;
  while(s >> cl) cross_links.push_back(cl);
  return cross_links.size();
}

void write_cross_link_file(const std::string& file_name,
                           const std::vector<CrossLink>& cross_links,
                           bool include_actual_distance) {
  std::ofstream ofile(file_name.c_str());
  for(unsigned int i=0; i<cross_links.size(); i++) {
    ofile << cross_links[i];
    if(include_actual_distance)
      ofile << " " << cross_links[i].get_actual_distance()
            << " " << cross_links[i].get_actual_cb_distance();
    ofile << std::endl;
  }
  ofile.close();
}

std::ostream& operator<<(std::ostream& s, const CrossLink& cl) {
  s << cl.residue_number1_ << " ";
  if(cl.chain_id1_ == ' ') s << "-"; else s << cl.chain_id1_;
  s << " " << cl.residue_number2_ << " ";
  if(cl.chain_id2_ == ' ') s << "-"; else s << cl.chain_id2_;
  s << " " << cl.min_distance_ << " " << cl.max_distance_;
  return s;
}

std::istream& operator>>(std::istream& s, CrossLink& cl) {
  s >> cl.residue_number1_ >> cl.chain_id1_ >> cl.residue_number2_
    >> cl.chain_id2_ >> cl.min_distance_ >> cl.max_distance_;
  if(cl.chain_id1_ == '-') cl.chain_id1_ = ' ';
  if(cl.chain_id2_ == '-') cl.chain_id2_ = ' ';
  return s;
}
