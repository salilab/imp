/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H
#define IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H

#include <IMP/algebra/algebra_config.h>
#include "../grid_indexes.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D, class Storage, class Value, class Embedding>
double get_interpolation_value(const GridD<D, Storage, Value, Embedding> &g,
                               const VectorD<D> &pt) {
  // get the values surrounding this one
  VectorD<D> bblb = pt - .5 * g.get_unit_cell(),
             bbub = pt + .5 * g.get_unit_cell();
  BoundingBoxD<D> bb(bblb, bbub);
  base::Vector<VectorD<D> > vertices = get_vertices(bb);
  Floats values(vertices.size());
  // suppress incorrect warning
  VectorD<D> lb = get_zero_vector_d<D>();
  for (unsigned int i = 0; i < vertices.size(); ++i) {
    GridIndexD<D> index = g.get_nearest_index(vertices[i]);
    values[i] = g[index];
    if (i == 0) lb = g.get_center(index);
  }
  VectorD<D> fraction(pt - lb);
  for (unsigned int i = 0; i < D; ++i) {
    fraction[i] /= g.get_unit_cell()[i];
  }
  for (unsigned int i = 0; i < D; ++i) {
    const unsigned int hsize = values.size() / 2;
    for (unsigned int j = 0; j < hsize; ++j) {
      values[j] =
          (1.0 - fraction[i]) * values[j] + fraction[i] * values[j + hsize];
    }
    values.resize(hsize);
  }
  IMP_USAGE_CHECK(values.size() == 1, "Huh?");
  return values[0];
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_GRID_INTERPOLATION_H */
