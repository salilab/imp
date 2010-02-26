/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#define IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#include "../config.h"
#include <IMP/base_types.h>
#include "../VectorD.h"
#include "../SphericalVector3D.h"

#ifdef IMP_USE_CGAL
#ifndef NDEBUG
//#define NDEBUG
#define IMP_NO_HAD_NDEBUG
#endif


#include <CGAL/Cartesian_d.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Optimisation_d_traits_d.h>
#include <CGAL/Min_sphere_d.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Search_traits_2.h>

#ifdef IMP_NO_HAD_NDEBUG
#undef NDEBUG
#undef IMP_NO_HAD_NDEBUG
#endif
#endif

IMPALGEBRA_BEGIN_NAMESPACE
template <unsigned int D>
VectorD<D> get_random_vector_on(const SphereD<D> &s);
IMPALGEBRA_END_NAMESPACE


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
/*If all is true, cover the whole sphere.
*/
template <unsigned int D>
std::vector<VectorD<D> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<D> &center,
                     double radius, bool ALL) {
  BOOST_STATIC_ASSERT(D!=3);
  std::vector<VectorD<D> > ret(n);
  for (unsigned int i=0; i< D; ++i) {
    VectorD<D> v= get_basis_vector_d<D>(i);
    if(ALL) {
      ret[2*i]=v;
      ret[2*i+1]= -v;
    } else {
      ret[i]=v;
    }
  }
  for (unsigned int i=(ALL?2*D:D); i< n; ++i) {
    VectorD<D> v= get_random_vector_on<D>(get_unit_sphere_d<D>());
    if (!ALL && v[D-1]<= center[D-1]) v=-v;
    ret[i]=v;
  }


#ifdef IMP_USE_CGAL
  typedef typename ::CGAL::Cartesian_d< ::CGAL::Lazy_exact_nt<
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
        VectorD<D> nr=-ret[i];
        P p(D, nr.coordinates_begin(),
            nr.coordinates_end());
        typename CH::Vertex_handle vh=ch.insert(p);
        indexes[vh]= -i-1;
      }
    }
    std::vector<VectorD<D> > sums(n, get_zero_vector_d<D>());
    std::vector<double> counts(n, 0);
    for (CH::Facet_iterator it= ch.facets_begin();
         it != ch.facets_end(); ++it) {
      for (unsigned int i=0; i< D; ++i ) {
        int vi=indexes[ch.vertex_of_facet(it, i)];
        VectorD<D> pi;
        if (vi > 0) {
          pi= ret[vi-1];
        } else {
          continue;
        }
        /*std::vector<VectorD<D> > simplex;
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
          VectorD<D> pj;
          if (vj > 0) {
            pj= ret[vj-1];
          } else {
            pj= ret[-vj+1];
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
        sums[i]= get_random_vector_on<D>(get_unit_sphere_d<D>());
      }
    }
    std::swap(sums, ret);
  }

#endif

  for (unsigned int i=0; i< ret.size(); ++i) {
    if (!ALL && ret[i][D-1] < 0) {
      ret[i]= -radius*ret[i]+center;
    } else {
      ret[i]= radius*ret[i]+center;
    }
  }

  return ret;
}


inline std::vector<VectorD<3> >
uniform_cover_sphere(unsigned int N,
                     const VectorD<3> &center,
                     double r, bool ALL) {
  std::vector<VectorD<3> > ret(N);
  double f=1;
  if (!ALL) {
    f=2.0;
  }
  double opsi;
  for (unsigned long k=1;k<=N;++k) {
    double h = 2.0*(k-1.0)/(f*N-1)-1.0;
    double theta = std::acos(h);
    double psi;
    if( k==1 || (ALL && k==N)) {
      psi=0;
    } else {
      psi=opsi + 3.6/std::sqrt(f*(1.0-h*h));
      int div = static_cast<int>( psi / (2.0*PI) );
      psi -= div*2.0*PI;
    }
    SphericalVector3D v(r, theta, psi);
    opsi=psi;
    if (!ALL) {
      ret[k-1]= -v.get_cartesian_coordinates()+ center;
    } else {
      ret[k-1]= v.get_cartesian_coordinates()+ center;
    }
  }
  return ret;
}

IMPALGEBRA_END_INTERNAL_NAMESPACE


#endif  /* IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H */
