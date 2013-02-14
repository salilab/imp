/**
 * \file CrossLink \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_CROSS_LINK_H
#define IMP_CROSS_LINK_H

#include <fstream>
#include <iostream>
#include <vector>

class CrossLink {
public:
  CrossLink():
    residue_number1_(0), chain_id1_('-'),
    residue_number2_(0), chain_id2_('-'),
    min_distance_(0.0), max_distance_(0.0)
  {}

  CrossLink(int residue_number1, char chain_id1,
            int residue_number2, char chain_id2,
            float min_dist, float max_dist,
            float actual_distance = 0.0, float actual_cb_distance = 0.0) :
    residue_number1_(residue_number1), chain_id1_(chain_id1),
    residue_number2_(residue_number2), chain_id2_(chain_id2),
    min_distance_(min_dist), max_distance_(max_dist),
    actual_distance_(actual_distance), actual_cb_distance_(actual_cb_distance)
  {}

  int get_residue1() const { return residue_number1_; }
  int get_residue2() const { return residue_number2_; }

  char get_chain1() const { return chain_id1_; }
  char get_chain2() const { return chain_id2_; }

  float get_min_distance() const { return min_distance_; }
  float get_max_distance() const { return max_distance_; }
  float get_actual_distance() const { return actual_distance_; }
  float get_actual_cb_distance() const { return actual_cb_distance_; }

  friend std::ostream& operator<<(std::ostream& q, const CrossLink& cl);
  friend std::istream& operator>>(std::istream& s, CrossLink &cl);

protected:
  int residue_number1_;
  char chain_id1_;
  int residue_number2_;
  char chain_id2_;
  float min_distance_;
  float max_distance_;
  float actual_distance_; // from PDB - for testing purposes
  float actual_cb_distance_; // from PDB - for testing purposes
};

int read_cross_link_file(const std::string& file_name,
                         std::vector<CrossLink>& cross_links);

void write_cross_link_file(const std::string& file_name,
                           const std::vector<CrossLink>& cross_links,
                           bool include_actual_distance=false);

#endif /* IMP_CROSS_LINK_H */
