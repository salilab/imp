/**
 *  \file random_generator.h   \brief random number generator
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_HISTOGRAM_H
#define IMPSTATISTICS_INTERNAL_HISTOGRAM_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/base/utility.h>
#include <IMP/algebra/VectorD.h>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
template <int D>
struct MinMax {
  FloatPair minmax;
  MinMax()
      : minmax(std::numeric_limits<double>::max(),
               -std::numeric_limits<double>::max()) {}
  template <class G>
  void operator()(const G &g, const typename G::Index &i,
                  const typename G::Vector &) {
    minmax.first = std::min(minmax.first, g[i]);
    minmax.second = std::max(minmax.second, g[i]);
  }
};

template <int D>
struct Mean {
  algebra::VectorD<D> mn;
  Mean(algebra::VectorD<D> start) : mn(start) {}
  template <class G>
  void operator()(const G &g, const typename G::Index &i,
                  const typename G::Vector &v) {
    mn += g[i] * v;
  }
};

template <int D>
struct Sigma2 {
  algebra::VectorD<D> mn, sigma2;
  Sigma2(algebra::VectorD<D> mn, algebra::VectorD<D> start)
      : mn(mn), sigma2(start) {}
  template <class G>
  void operator()(const G &g, const typename G::Index &index,
                  const typename G::Vector &v) {
    if (g[index] != 0) {
      for (unsigned int i = 0; i < mn.get_dimension(); ++i) {
        sigma2[i] += g[index] * base::square(mn[i] - v[i]);
      }
    }
  }
};

template <int D, class Grid>
struct Frequency {
  Grid &g_;
  double norm_;
  Frequency(Grid &g, double norm) : g_(g), norm_(norm) {}
  template <class G>
  void operator()(const G &g, const typename G::Index &index,
                  const typename G::Vector &) {
    g_[index] = g[index] * norm_;
  }
};

IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_INTERNAL_HISTOGRAM_H */
