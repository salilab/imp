/**
 *  \file PMFTable.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPCORE_INTERNAL_PMF_TABLE_H
#define IMPCORE_INTERNAL_PMF_TABLE_H

#include "../core_config.h"
#include "evaluate_distance_pair_score.h"
#include <IMP/exception.h>
#include <IMP/file.h>
#include <cmath>
#include <vector>

IMPCORE_BEGIN_INTERNAL_NAMESPACE
template <bool BIPARTITE>
struct PMFTable {
private:
  unsigned int split_;
  double inverse_bin_width_;
  double bin_width_;
  double max_;
  std::vector<std::vector< IMP::core::internal::RawOpenCubicSpline > > data_;
  void order(unsigned int &i, unsigned int &j) const {
    if (i > j) {
      std::swap(i,j);
    }
    IMP_USAGE_CHECK(!BIPARTITE || j >=split_,
                    "One of the particles should be of each type: "
                    << i << " " << j);
    j-=split_;
  }
public:
  PMFTable(unsigned int split): split_(split){}
  template <class Key>
  void initialize(TextInput tin,
                  unsigned int ni, unsigned int nj)  {
    std::istream &in =tin;
    double bin;
    std::string line;
    std::getline(in, line);
    std::istringstream iss(line);
    iss >> bin;
    unsigned int np, nl;
    iss >> np >> nl;
    if (np != ni
        || nl != nj) {
      IMP_THROW("Expected number of protein and ligand types not found. "
                << "Expected " << ni
                << " " << nj
                << " but got " << np << " " << nl,
                IOException);
    }
    if (!iss) {
      IMP_THROW("Error reading bin size from line " << line,
                IOException);
    }
    {
      double test;
      iss >> test;
      if (iss) {
        IMP_THROW("Extra data found on bin size line " << line
                  << " got " << test,
                  IOException);
      }
    }
    bin_width_=bin;
    inverse_bin_width_=1.0/bin;

    data_.resize(ni);
    int bins_read=-1;
    for(unsigned int i=0;i<data_.size();i++){
      data_[i].resize(nj);
    }
    while (true) {
      std::string line;
      std::getline(in, line);
      if (!in) {
        break;
      }
      std::istringstream ins(line);
      std::string pname, lname;
      ins >> pname >> lname;
      unsigned int i = Key(pname).get_index();
      unsigned int j = Key(lname).get_index();
      IMP_INTERNAL_CHECK(Key(pname) == Key(i),
                         "Expected and found protein types don't match: "
                         << "expected \"" <<  Key(i).get_string()
                         << " got " << pname << " at " << i << " " << j);
      IMP_INTERNAL_CHECK(Key(lname) == Key(j),
                         "Expected and found ligand types don't match: "
                         << "expected \"" <<  Key(j).get_string()
                         << " got " << pname << " at " << i << " " << j);
      //      std::cout << line << std::endl;
      //      if(j>1) exit(EXIT_FAILURE);
      int cur_bins_read=0;
      Floats data;
      while(true) {
        double potentialvalue;
        ins >> potentialvalue;
        if (ins) {
          data.push_back(potentialvalue);
          ++cur_bins_read;
        } else {
          break;
        }
      }
      order(i, j);
      data_[i][j]= core::internal::RawOpenCubicSpline(data, bin_width_,
                                                      inverse_bin_width_);
      if (bins_read != -1 && cur_bins_read != bins_read) {
        IMP_THROW("Read wrong number of bins from line: "
                  << line << "\nExpected " << bins_read
                  << " got " << cur_bins_read,
                  IOException);
      }
      bins_read= cur_bins_read;
      ins.clear();
    }
    max_= bin_width_*bins_read;
    IMP_LOG(TERSE, "PMF table entries have "
            << bins_read << " bins with width " << bin_width_ << std::endl);
  }
  double get_score(unsigned int i, unsigned int j, double dist) const {
   if (dist >= max_) return 0;
    order(i,j);
    IMP_USAGE_CHECK(i < data_.size(), "Out of range protein index " << i);
    IMP_USAGE_CHECK(j < data_[i].size(),
                    "Out of range ligand index " << i << " " << j);
    return data_[i][j].get_bin(dist, bin_width_, inverse_bin_width_);
  }
  double get_max() const {
    return max_;
  }
  DerivativePair get_score_with_derivative(unsigned int i,
                                           unsigned int j, double dist) const {
   if (dist >= max_-.5*bin_width_) return DerivativePair(0,0);
    order(i,j);
    IMP_USAGE_CHECK(i < data_.size(), "Out of range protein index " << i);
    IMP_USAGE_CHECK(j < data_[i].size(),
                    "Out of range ligand index " << i << " " << j);
    if (dist <= .5*bin_width_) return DerivativePair(get_score(i,j,dist), 0);
    // shift by .5 for the splines so as to be between the centers of the cells
    return data_[i][j].evaluate_with_derivative(dist-.5*bin_width_, bin_width_,
                                                inverse_bin_width_);
  }
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_PMF_TABLE_H */
