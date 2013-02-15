/**
 *  \file PMFTable.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_INTERNAL_PMF_TABLE_H
#define IMPSCORE_FUNCTOR_INTERNAL_PMF_TABLE_H

#include <IMP/score_functor/score_functor_config.h>
#include "RawOpenCubicSpline.h"
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/grid_storages.h>
#include <IMP/exception.h>
#include <IMP/file.h>
#include <cmath>
#include <vector>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE



// slightly evil using the grid storage, but...
template <bool SPARSE>
struct StorageSelector {
  typedef algebra::DenseGridStorageD<2, RawOpenCubicSpline> Type;
};

template <>
struct StorageSelector<true> {
  typedef algebra::SparseGridStorageD<2, RawOpenCubicSpline,
                     algebra::UnboundedGridRangeD<2> > Type;
};

template <bool BIPARTITE, bool INTERPOLATE, bool SPARSE=false>
struct PMFTable {
private:
  unsigned int split_;
  double inverse_bin_width_;
  double bin_width_;
  double max_;
  double offset_;
  typedef typename StorageSelector<SPARSE>::Type Storage;
  Storage data_;
  void order(unsigned int &i, unsigned int &j) const {
    if (i > j) {
      std::swap(i,j);
    }
    IMP_USAGE_CHECK(!BIPARTITE || j >=split_,
                    "One of the particles should be of each type: "
                    << i << " " << j);
    j-=split_;
  }
  const RawOpenCubicSpline& get(int i, int j) const {
    Ints is(2);
    is[0]=i; is[1]=j;
    typename Storage::ExtendedIndex ei(is);
    return data_[data_.get_index(ei)];
  }
public:
  PMFTable(unsigned int split): split_(split){}
  template <class Key>
  void initialize(base::TextInput tin)  {
    std::istream &in =tin;
    double bin;
    std::string line;
    std::getline(in, line);
    std::istringstream iss(line);
    iss >> bin;
    if (!iss) {
      IMP_THROW("Error reading bin size from line " << line,
                IOException);
    }
    unsigned int np, nl;
    iss >> np;
    if (BIPARTITE) {
      iss >> nl;
    } else {
      nl=np;
    }
    if (!iss) {
      IMP_THROW("Error number of types from line " << line,
                IOException);
    }
    iss >> offset_;
    if (!iss) {
      offset_=0;
    }
    IMP_LOG_TERSE( "Reading " << np << " by " << nl
            << " from file " << tin.get_name() << std::endl);
    /*if (np != ni
        || nl != nj) {
      IMP_THROW("Expected number of protein and ligand types not found. "
                << "Expected " << ni
                << " " << nj
                << " but got " << np << " " << nl,
                IOException);
                }*/

    bin_width_=bin;
    inverse_bin_width_=1.0/bin;
    Ints dims(2);
    dims[0]=np;
    dims[1]=nl;
    data_= Storage(dims);
    int bins_read=-1;
    unsigned int read_entries=0;
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
      Ints is(2);
      is[0]=i; is[1]=j;
      typename Storage::ExtendedIndex ei(is);
      if (!data_.get_has_index(ei)) {
        data_.add_voxel(ei, score_functor::internal::RawOpenCubicSpline(data,
                                                                   bin_width_,
                                                          inverse_bin_width_));
      } else {
        data_[data_.get_index(ei)]=
          score_functor::internal::RawOpenCubicSpline(data, bin_width_,
                                             inverse_bin_width_);
      }
      if (bins_read != -1 && cur_bins_read != bins_read) {
        IMP_THROW("Read wrong number of bins from line: "
                  << line << "\nExpected " << bins_read
                  << " got " << cur_bins_read,
                  IOException);
      }
      bins_read= cur_bins_read;
      ins.clear();
      ++read_entries;
    }
    max_= bin_width_*bins_read;
    if (BIPARTITE) {
      if (read_entries != np *nl) {
        IMP_THROW("Read " << read_entries << " from table, but expected all of"
                  << np << "x"<< nl << "=" << np*nl, IOException);
      }
    } else {
      if (read_entries != np *(np+1)/2) {
        IMP_THROW("Read " << read_entries << " from table, but expected all of"
                  << np << "x"<< np+1 << "/2=" << np*(np+1)/2, IOException);
      }
    }
    IMP_LOG_TERSE( "PMF table entries have "
            << bins_read << " bins with width "
            << bin_width_ << std::endl);
  }
  double get_score(unsigned int i, unsigned int j, double dist) const {
   if (dist >= max_ || dist <= offset_) return 0;
    order(i,j);
    if (INTERPOLATE) {
      return get(i,j).evaluate(dist-.5*bin_width_-offset_, bin_width_,
                                  inverse_bin_width_);
    } else {
      return get(i,j).get_bin(dist-offset_, bin_width_, inverse_bin_width_);
    }
  }
  double get_max() const {
    return max_;
  }
  DerivativePair get_score_with_derivative(unsigned int i,
                                           unsigned int j, double dist) const {
    if (dist >= max_-.5*bin_width_ || dist <= offset_) {
      return DerivativePair(0,0);
    }
    order(i,j);
    if (dist <= .5*bin_width_) return DerivativePair(get_score(i,j,dist), 0);
    // shift by .5 for the splines so as to be between the centers of the cells
    return get(i,j).evaluate_with_derivative(dist-.5*bin_width_-offset_,
                                             bin_width_,
                                             inverse_bin_width_);
  }
};

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_INTERNAL_PMF_TABLE_H */
