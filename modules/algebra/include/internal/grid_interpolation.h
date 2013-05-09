/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H
#define IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H

#include <IMP/algebra/algebra_config.h>
#include "../grid_indexes.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

// combine with get_values?
template <int D, class Storage, class Value, class Embedding>
base::Vector<VectorD<D> > get_interpolation_corners(
    const GridD<D, Storage, Value, Embedding> &g, const VectorD<D> &pt) {
  base::Vector<VectorD<D> > ret;
  VectorD<D> offset = .5 * g.get_unit_cell();
  // special case first
  ret.push_back(pt);
  ret.push_back(pt);
  ret[0][0] -= offset[0];
  ret[1][0] += offset[0];
  for (int i = 1; i < D; ++i) {
    ret.insert(ret.end(), ret.begin(), ret.end());
    for (unsigned int j = 0; j < ret.size() / 2; ++j) {
      ret[j][i] -= offset[i];
    }
    for (unsigned int j = ret.size() / 2; j < ret.size(); ++j) {
      ret[j][i] += offset[i];
    }
  }
}
template <int D, class Storage, class Value, class Embedding>
Floats get_interpolation_values(const GridD<D, Storage, Value, Embedding> &g,
                                const base::Vector<VectorD<D> > &pt) {
  Floats ret(pt.size());
  for (unsigned int i = 0; i < pt.size(); ++i) {
    GridIndexD<D> index = g.get_nearest_index(pt[i]);
    ret[i] = g[index];
  }
  return ret;
}
template <int D, class Storage, class Value, class Embedding>
VectorD<D> get_interpolation_fraction(
    const GridD<D, Storage, Value, Embedding> &g, VectorD<D> pt) {
  pt -= .5 * g.get_unit_cell();
  ExtendedGridIndexD<D> index = g.get_extended_index(pt);
  VectorD<D> center = g.get_center(index);
  pt -= center;
  for (unsigned int i = 0; i < D; ++i) {
    pt[i] /= g.get_unit_cell()[i];
  }
  return pt;
}
template <int D, class Storage, class Value, class Embedding>
double get_interpolation_value(const GridD<D, Storage, Value, Embedding> &,
                               Floats f, const VectorD<D> &fraction) {
  for (unsigned int i = 0; i < D; ++i) {
    const unsigned int hsize = f.size() / 2;
    for (unsigned int j = 0; j < hsize; ++j) {
      f[j] = (1.0 - fraction[i]) * f[j] + fraction[i] * f[j + hsize];
    }
    f.resize(hsize);
  }
  IMP_USAGE_CHECK(f.size() == 1, "Huh?");
  return f[0];
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H */
