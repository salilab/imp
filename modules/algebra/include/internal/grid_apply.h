/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_APPLY_H
#define IMPALGEBRA_INTERNAL_GRID_APPLY_H

#include <IMP/algebra/algebra_config.h>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <class Functor, class Grid, int D>
struct GridApplier {
  static void apply(const Grid &g, typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index, typename Grid::Vector &center,
                    Functor &f) {
    int *data = index.access_data().get_data();
    for (data[D] = lb[D]; data[D] < ub[D]; ++data[D]) {
      center[D] = corner[D] + cell[D] * (data[D] + .5);
      GridApplier<Functor, Grid, D - 1>::apply(g, lb, ub, corner, cell, index,
                                               center, f);
    }
  }
};

template <class Functor, class Grid>
struct GridApplier<Functor, Grid, 0> {
  static void apply(const Grid &g, typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index, typename Grid::Vector &center,
                    Functor &f) {
    const int D = 0;
    for (index.access_data().get_data()[D] = lb[0]; index[0] < ub[D];
         ++index.access_data().get_data()[D]) {
      center[D] = corner[D] + cell[D] * (index[D] + .5);
      f(g, index, center);
    }
  }
};

// silly and do it on D-1
template <class Functor, class Grid>
struct GridApplier<Functor, Grid, -2> {
  static void apply_recursive(const Grid &g, typename Grid::ExtendedIndex &lb,
                              typename Grid::ExtendedIndex &ub,
                              const typename Grid::Vector &corner,
                              unsigned int D, const typename Grid::Vector &cell,
                              typename Grid::Index &index,
                              typename Grid::Vector &center, Functor &f) {
    int *data = index.access_data().get_data();
    for (data[D] = lb[D]; data[D] < ub[D]; ++data[D]) {
      center[D] = corner[D] + cell[D] * data[D];
      if (D == 0) {
        f(g, index, center);
      } else {
        apply_recursive(g, lb, ub, corner, D - 1, cell, index, center, f);
      }
    }
  }
  static void apply(const Grid &g, typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index, typename Grid::Vector &center,
                    Functor &f) {
    apply_recursive(g, lb, ub, corner, g.get_dimension() - 1, cell, index,
                    center, f);
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_GRID_APPLY_H */
