/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#define IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#include "../algebra_config.h"
#include <IMP/base_types.h>
#include "../VectorD.h"
#include "../SphericalVector3D.h"
#ifdef IMP_USE_CGAL
#include <IMP/cgal/internal/sphere_cover.h>
#endif



IMPALGEBRA_BEGIN_NAMESPACE
template <unsigned int D>
VectorD<D> get_random_vector_on(const SphereD<D> &s);
IMPALGEBRA_END_NAMESPACE


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <unsigned int D>
std::vector<VectorD<D> >
native_uniform_cover_unit_sphere(unsigned int n,bool ALL) {
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
    if (!ALL && v[D-1]<= 0) v=-v;
    ret[i]=v;
  }
  return ret;
}

/*If all is true, cover the whole sphere.
*/
template <unsigned int D>
std::vector<VectorD<D> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<D> &center,
                     double radius, bool ALL) {
  std::vector<VectorD<D> > ret= native_uniform_cover_unit_sphere<D>(n, ALL);
  for (unsigned int i=0; i< ret.size(); ++i) {
    if (!ALL && ret[i][D-1] < 0) {
      ret[i]= -radius*ret[i]+center;
    } else {
      ret[i]= radius*ret[i]+center;
    }
  }
  return ret;
}

/*If all is true, cover the whole sphere.
*/
inline std::vector<VectorD<4> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<4> &center,
                     double radius, bool ALL) {
  std::vector<VectorD<4> > ret= native_uniform_cover_unit_sphere<4>(n, ALL);
#ifdef IMP_USE_CGAL
  IMP::cgal::internal::refine_unit_sphere_cover_4d(ret, ALL);
#endif
  for (unsigned int i=0; i< ret.size(); ++i) {
    if (!ALL && ret[i][4-1] < 0) {
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


inline std::vector<VectorD<2> >
uniform_cover_sphere(unsigned int N,
                     const VectorD<2> &center,
                     double r, bool ALL) {
  std::vector<VectorD<2> > ret(N);
  for (unsigned int i=0; i< N; ++i) {
    double f;
    if (ALL) {
      f= static_cast<double>(i)/(N+1);
    } else {
      f= static_cast<double>(i)/(2*N+2);
    }
    ret[i]= center+r*VectorD<2>(sin(f), cos(f));
  }
  return ret;
}

IMPALGEBRA_END_INTERNAL_NAMESPACE


#endif  /* IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H */
