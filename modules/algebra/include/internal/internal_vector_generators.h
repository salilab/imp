/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#define IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#include "../algebra_config.h"
#include <IMP/base_types.h>
#include "../VectorD.h"
#include "../SphereD.h"
#include "../SphericalVector3D.h"
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
#include <IMP/cgal/internal/sphere_cover.h>
#endif


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D>
inline VectorD<D> get_random_vector_on(const SphereD<D> &s) {
  BoundingBoxD<D> bb= get_bounding_box(s);
  do {
    VectorD<D> pt= get_random_vector_in(bb);
    double r2= (s.get_center()-pt).get_squared_magnitude();
    if (r2 < square(s.get_radius()) && r2 > square(.1*s.get_radius())) {
      VectorD<D> diff= pt-s.get_center();
      VectorD<D> udiff= diff.get_unit_vector();
      return s.get_center()+udiff*s.get_radius();
    }
  } while (true);
}

inline VectorD<2> get_random_vector_on(const SphereD<2> &s) {
  ::boost::uniform_real<> rand(0, 2*PI);
  double angle=rand(base::random_number_generator);
  VectorD<2> ret(s.get_radius()*sin(angle),
                 s.get_radius()*cos(angle));
  return ret+ s.get_center();
}

/*inline VectorD<3> get_random_vector_on(const SphereD<3> &s) {
  double cur_radius2=square(s.get_radius());
  Floats up(s.get_center().get_dimension());
  for (unsigned int i=s.get_dimension()-1; i>0; --i) {
    double r= std::sqrt(cur_radius2);
    ::boost::uniform_real<> rand(-r, r);
    up[i]= rand(random_number_generator);
    // radius of circle
    cur_radius2= cur_radius2-square(up[i]);
  }
  ::boost::uniform_int<> rand(0, 1);
  double x= std::sqrt(cur_radius2);
  if (rand(random_number_generator)) {
    x=-x;
  }
  up[0]=x;

  IMP_INTERNAL_CHECK(std::abs(VectorD<3>(up.begin(),
                                         up.end()).get_magnitude()
                              -s.get_radius()) < .1,
                     "Error generating vector on sphere: "
                     << VectorD<3>(up.begin(), up.end())
                     << " for " << s.get_radius());
  //IMP_LOG(VERBOSE, "Random vector on sphere is " << up << std::endl);

  return s.get_center()+ VectorD<3>(up.begin(), up.end());
  }*/

/*inline VectorKD get_random_vector_on(const SphereKD &s) {
  if (s.get_dimension()==2) {
    Vector2D pt
    = get_random_vector_on(Sphere2D(Vector2D(s.get_center().coordinates_begin(),
    s.get_center().coordinates_end()),
                                               s.get_radius()));
    return pt;
  }
  }*/

template <int D>
inline std::vector<VectorD<D> >
native_uniform_cover_unit_sphere(unsigned int d,
                                 unsigned int n,bool ALL) {
  BOOST_STATIC_ASSERT(D!=3);
  std::vector<VectorD<D> > ret(n);
  for (unsigned int i=0; i< std::min(d, n/(ALL?2:1)); ++i) {
    VectorD<D> v= get_basis_vector_kd(d, i);
    if(ALL) {
      ret[2*i]=v;
      ret[2*i+1]= -v;
    } else {
      ret[i]=v;
    }
  }
  for (unsigned int i=(ALL?2*d:d); i< n; ++i) {
    VectorD<D> v= internal::get_random_vector_on(get_unit_sphere_kd(d));
    if (!ALL && v[d-1]<= 0) v=-v;
    ret[i]=v;
  }
  return ret;
}

/*If all is true, cover the whole sphere.
 */
template <int D>
inline std::vector<VectorD<D> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<D> &center,
                     double radius, bool ALL) {
  std::vector<VectorD<D> > ret
    = native_uniform_cover_unit_sphere<D>(center.get_dimension(),
                                          n, ALL);
  for (unsigned int i=0; i< ret.size(); ++i) {
    if (!ALL && ret[i][center.get_dimension()-1] < 0) {
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
  std::vector<VectorD<4> > ret
    = native_uniform_cover_unit_sphere<4>(center.get_dimension(),
                                          n, ALL);
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
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


inline Vector3Ds
uniform_cover_sphere(unsigned int N,
                     const Vector3D &center,
                     double r, bool ALL) {
  Vector3Ds ret(N);
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


inline Vector2Ds
uniform_cover_sphere(unsigned int N,
                     const Vector2D &center,
                     double r, bool ALL) {
  Vector2Ds ret(N);
  for (unsigned int i=0; i< N; ++i) {
    double f;
    if (ALL) {
      f= static_cast<double>(i)/(N+1);
    } else {
      f= static_cast<double>(i)/(2*N+2);
    }
    ret[i]= center+r*Vector2D(sin(f), cos(f));
  }
  return ret;
}


template <int DO>
struct DMinus1 {
  static const int D=DO-1;
};
template <>
struct DMinus1<-1> {
  static const int D=-1;
};


template <int D>
struct RandomVectorOnBB {
  static VectorD<D> get(BoundingBoxD<D> bb) {
    IMP_USAGE_CHECK(D>0, "Does not work in runtime D yet");
    boost::scoped_array<double> areas(new double[bb.get_dimension()*2]);
    VectorD<D> lb= bb.get_corner(0);
    VectorD<D> ub= bb.get_corner(1);
    for (unsigned int i=0; i< bb.get_dimension(); ++i) {
      areas[i]=1;
      for (unsigned int j=1; j< bb.get_dimension(); ++j) {
        areas[i] *= ub[(i+j)%bb.get_dimension()]-lb[(i+j)%bb.get_dimension()];
      }
      if (i!= 0) {
        areas[i]+= areas[i-1];
      }
    }
    for (unsigned int i=0; i< bb.get_dimension(); ++i) {
      areas[bb.get_dimension()+i]= areas[bb.get_dimension()-1]+areas[i];
    }
    /*for (unsigned int i=0; i< D*2; ++i) {
      std::cout << areas[i] << " ";
      }*/
    ::boost::uniform_real<> rand(0, areas[2*bb.get_dimension()-1]);
    double a= rand(base::random_number_generator);
    //std::cout << ": " << a << std::endl;
    unsigned int side;
    for (side=0; side< 2*bb.get_dimension(); ++side) {
      if (areas[side] > a) break;
    }
    unsigned int coord= (side>=bb.get_dimension()
                         ? side-bb.get_dimension(): side);
    Floats fmin(bb.get_dimension()-1), fmax(bb.get_dimension()-1);
    for (unsigned int i=1; i< bb.get_dimension(); ++i) {
      fmin[i-1]= 0;
      fmax[i-1]= ub[(coord+i)%bb.get_dimension()]
        - lb[(coord+i)%bb.get_dimension()];
    }
    VectorD<internal::DMinus1<D>::D> vfmin(fmin.begin(), fmin.end()),
      vfmax(fmax.begin(), fmax.end());
    VectorD<internal::DMinus1<D>::D> sv=
      get_random_vector_in(BoundingBoxD<internal::DMinus1<D>::D>(vfmin, vfmax));

    Floats ret(bb.get_dimension());
    //std::cout << "Side is " << side << std::endl;
    if (side >=bb.get_dimension()) {
      std::copy(ub.coordinates_begin(), ub.coordinates_end(), ret.begin());
      for (unsigned int i=1; i< bb.get_dimension(); ++i) {
        ret[(coord+i)%bb.get_dimension()]-= sv[i-1];
      }
    } else {
      std::copy(lb.coordinates_begin(), lb.coordinates_end(), ret.begin());
      for (unsigned int i=1; i< bb.get_dimension(); ++i) {
        ret[(coord+i)%bb.get_dimension()]+= sv[i-1];
      }
    }

    return VectorD<D>(ret.begin(), ret.end());
  }
};

template <>
struct RandomVectorOnBB<1> {
  static VectorD<1> get(BoundingBoxD<1> bb) {
    ::boost::uniform_int<> rand(0, 1);
    return bb.get_corner(rand(base::random_number_generator));
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE


#endif  /* IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H */
