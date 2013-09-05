/**
 * \file SASTable.h
 * \brief Storage of Solvent Accessability Statistical potential
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SAS_TABLE_H
#define IMPSCORE_FUNCTOR_SAS_TABLE_H

#include <IMP/score_functor/score_functor_config.h>
#include "RawOpenCubicSpline.h"

#include <IMP/base/exception.h>
#include <IMP/base/file.h>

#include <vector>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

template <bool INTERPOLATE>
class SASTable {
public:
  SASTable() {}

  template <class Key>
  void initialize(base::TextInput tin) {
    std::istream &in = tin;

    // read header line
    double bin;
    std::string line;
    std::getline(in, line);
    std::istringstream iss(line);
    iss >> bin;
    if(!iss) IMP_THROW("Error reading bin size from line " << line,IOException);
    unsigned int np;
    iss >> np;
    if(!iss) IMP_THROW("Error number of types from line " << line, IOException);
    iss >> offset_;
    if(!iss) offset_ = 0;
    IMP_LOG_TERSE("Reading " << np << " from " << tin.get_name() << std::endl);

    // init table parameters
    bin_width_ = bin;
    inverse_bin_width_ = 1.0 / bin;
    data_.resize(np);

    // read the table
    int bins_read = -1;
    unsigned int read_entries = 0;
    while (true) {
      std::string line;
      std::getline(in, line);
      if (!in) break;
      std::istringstream ins(line);
      std::string pname;
      ins >> pname;
      unsigned int i = Key(pname).get_index();
      IMP_INTERNAL_CHECK(Key(pname) == Key(i),
                         "Expected and found protein types don't match: "
                         << "expected \"" << Key(i).get_string() << " got "
                         << pname << " at " << i << std::endl);
      int cur_bins_read = 0;
      Floats data;
      while(true) {
        double potentialvalue;
        ins >> potentialvalue;
        if(ins) {
          data.push_back(potentialvalue);
          ++cur_bins_read;
        } else {
          break;
        }
      }

      data_[read_entries] =
        RawOpenCubicSpline(data, bin_width_, inverse_bin_width_);

      if (bins_read != -1 && cur_bins_read != bins_read) {
        IMP_THROW("Read wrong number of bins from line: "
                  << line << "\nExpected " << bins_read << " got "
                  << cur_bins_read, IOException);
      }
      bins_read = cur_bins_read;
      ins.clear();
      ++read_entries;
    }
    max_ = bin_width_ * bins_read;

    IMP_LOG_TERSE("PMF table entries have " << bins_read << " bins with width "
                                            << bin_width_ << std::endl);
  }

  double get_score(unsigned int i, double area) const {
    if(area >= max_ || area < offset_) return 0;
    if(INTERPOLATE) {
      return data_[i].evaluate(area - .5 * bin_width_ - offset_, bin_width_,
                               inverse_bin_width_);
    } else {
      return data_[i].get_bin(area - offset_, bin_width_, inverse_bin_width_);
    }
  }

  double get_max() const { return max_; }


 private:
  double inverse_bin_width_;
  double bin_width_;
  double max_;
  double offset_;
  std::vector<RawOpenCubicSpline> data_;
};

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SAS_TABLE_H */
