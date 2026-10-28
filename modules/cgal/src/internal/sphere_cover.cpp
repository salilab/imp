/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#include <IMP/cgal/internal/sphere_cover.h>

#ifdef IMP_HAS_BOOST_TR1_BUG
#define BOOST_HAS_GCC_TR1
#include <boost/tr1/detail/config.hpp>
#if defined(BOOST_HAS_GCC_TR1) && defined(BOOST_HAS_INCLUDE_NEXT)
#undef BOOST_HAS_INCLUDE_NEXT
#endif
#endif

IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
#include <IMP/algebra/vector_generators.h>
#include <CGAL/Epick_d.h>
#include <CGAL/Triangulation.h>
#include <vector>

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
template <int D>
void refine_unit_sphere_cover_d(IMP::Vector<algebra::VectorD<D>> &ret,
                                bool ALL) {
  unsigned int n = ret.size();
  // The convex hull is computed from a triangulation of the points; only
  // predicates are needed, which Epick_d evaluates exactly
  typedef ::CGAL::Epick_d< ::CGAL::Dimension_tag<D>> K;
  typedef ::CGAL::Triangulation<K> T;
  typedef typename T::Point P;
  typedef typename T::Vertex_handle VertexHandle;
  typedef typename T::Full_cell_iterator FullCellIterator;

  for (unsigned int rep = 0; rep < 10 * D; ++rep) {
    T t(D);
    std::map<VertexHandle, int> indexes;
    for (unsigned int i = 0; i < ret.size(); ++i) {
      P p(ret[i].begin(), ret[i].end());
      VertexHandle vh = t.insert(p);
      indexes[vh] = i + 1;
      if (!ALL) {
        algebra::VectorD<D> nr = -ret[i];
        P opp_p(nr.begin(), nr.end());
        VertexHandle opp_vh = t.insert(opp_p);
        indexes[opp_vh] = -static_cast<int>(i) - 1;
      }
    }
    IMP::Vector<algebra::VectorD<D>> sums(n, algebra::get_zero_vector_d<D>());
    Floats counts(n, 0);
    // Each facet of the convex hull is the finite part of an infinite full
    // cell of the triangulation (if the points span all D dimensions)
    std::vector<VertexHandle> facet;
    for (FullCellIterator it = t.full_cells_begin();
         t.current_dimension() == D && it != t.full_cells_end(); ++it) {
      if (!t.is_infinite(it)) continue;
      facet.clear();
      for (int k = 0; k <= D; ++k) {
        if (!t.is_infinite(it->vertex(k))) facet.push_back(it->vertex(k));
      }
      for (unsigned int i = 0; i < D; ++i) {
        int vi = indexes[facet[i]];
        algebra::VectorD<D> pi;
        if (vi > 0) {
          pi = ret[vi - 1];
        } else {
          continue;
        }
        for (unsigned int j = 0; j < D; ++j) {
          if (i == j) continue;
          int vj = indexes[facet[j]];
          algebra::VectorD<D> pj;
          if (vj > 0) {
            pj = ret[vj - 1];
          } else {
            // the opposite of a point
            pj = -ret[-vj - 1];
          }
          double d = (pj - pi).get_magnitude();
          if (counts[vi - 1] < d) {
            counts[vi - 1] = d;
            sums[vi - 1] = pj;
          }
        }
      }
    }
    for (unsigned int i = 0; i < (ALL ? 2 * D : D); ++i) {
      sums[i] = ret[i];
    }
    for (unsigned int i = (ALL ? 2 * D : D); i < ret.size(); ++i) {
      if (counts[i] != 0) {
        sums[i] = (.1 * sums[i] + .9 * ret[i]);
        sums[i] = sums[i].get_unit_vector();
      } else {
        // coincident points
        sums[i] =
            algebra::get_random_vector_on<D>(algebra::get_unit_sphere_d<D>());
      }
    }
    std::swap(sums, ret);
  }
}

void refine_unit_sphere_cover_4d(IMP::Vector<algebra::VectorD<4>> &ret,
                                 bool ALL) {
  refine_unit_sphere_cover_d(ret, ALL);
}

IMPCGAL_END_INTERNAL_NAMESPACE
