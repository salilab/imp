/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include <CGAL/Cartesian_d.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Optimisation_d_traits_d.h>
#include <CGAL/Min_sphere_d.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Search_traits_2.h>


IMPCGAL_BEGIN_INTERNAL_NAMESPACE
template <int D>
void
refine_unit_sphere_cover_d(base::Vector<algebra::VectorD<D> > &ret,
                            bool ALL) {
  unsigned int n= ret.size();
  typedef  ::CGAL::Cartesian_d< ::CGAL::Lazy_exact_nt<
  ::CGAL::Gmpq> > K;
  typedef typename K::Point_d P;
  typedef ::CGAL::Convex_hull_d<K> CH;
  typedef ::CGAL::Optimisation_d_traits_d<K>       Traits;
  typedef ::CGAL::Min_sphere_d<Traits>             Min_sphere;
  std::map<typename CH::Vertex_handle, int> indexes;
  for (unsigned int rep=0; rep< 10*D; ++rep) {
    CH ch(D);
    for (unsigned int i=0; i< ret.size(); ++i) {
      P p(D, ret[i].coordinates_begin(),
          ret[i].coordinates_end());
      typename CH::Vertex_handle vh=ch.insert(p);
      indexes[vh]= i+1;
      if (!ALL) {
        algebra::VectorD<D> nr=-ret[i];
        P p(D, nr.coordinates_begin(),
            nr.coordinates_end());
        typename CH::Vertex_handle vh=ch.insert(p);
        indexes[vh]= -static_cast<int>(i)-1;
      }
    }
    base::Vector<algebra::VectorD<D> > sums(n, algebra::get_zero_vector_d<D>());
    Floats counts(n, 0);
    for (CH::Facet_iterator it= ch.facets_begin();
         it != ch.facets_end(); ++it) {
      for (unsigned int i=0; i< D; ++i ) {
        int vi=indexes[ch.vertex_of_facet(it, i)];
        algebra::VectorD<D> pi;
        if (vi > 0) {
          pi= ret[vi-1];
        } else {
          continue;
        }
        /*vector<VectorD<D> > simplex;
        for (unsigned int j=0; i< D; ++i ) {
          int vj=indexes[ch.vertex_of_facet(it, j)];
          VectorD<D> pj;
          if (vj > 0) {
            pj= ret[vj-1];
          } else {
            pj= ret[-vj+1];
          }
          simplex.push_back((pj-pi).get_unit_vector());
        }
        double w= simplex_volume(simplex);*/
        for (unsigned int j=0; i< D; ++i ) {
          if (i==j) continue;
          int vj=indexes[ch.vertex_of_facet(it, j)];
          algebra::VectorD<D> pj;
          if (vj > 0) {
            pj= ret[vj-1];
          } else {
            pj= ret[-vj-1];
          }
          double d=(pj-pi).get_magnitude();
          if (counts[vi-1] < d) {
            counts[vi-1]=d;
            sums[vi-1]=pj;
          }
        }
      }
    }
    for (unsigned int i=0; i< (ALL?2*D:D); ++i) {
      sums[i]=ret[i];
    }
    for (unsigned int i=(ALL?2*D:D); i<ret.size(); ++i) {
      if (counts[i] != 0) {
        sums[i]= (.1*sums[i]+.9*ret[i]);
        sums[i]= sums[i].get_unit_vector();
      } else {
        // coincident points
        /*IMP_WARN("Coincident points at " << ret[i] << " in iteration " << rep
          << std::endl);*/
        sums[i]
          = algebra::get_random_vector_on<D>(algebra::get_unit_sphere_d<D>());
      }
    }
    std::swap(sums, ret);
  }
}

void
refine_unit_sphere_cover_4d(base::Vector<algebra::VectorD<4> > &ret,
                            bool ALL) {
  refine_unit_sphere_cover_d(ret, ALL);
}

IMPCGAL_END_INTERNAL_NAMESPACE
