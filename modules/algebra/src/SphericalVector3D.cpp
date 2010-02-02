/**
 *  \file  SphericalVector3D.cpp
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/
#include "IMP/algebra/SphericalVector3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Vector3D SphericalVector3D::get_cartesian_coordinates() {
  return Vector3D(v_[0]*cos(v_[2])*sin(v_[1]),
                  v_[0]*sin(v_[2])*sin(v_[1]),
                  v_[0]*cos(v_[1]));
}

void SphericalVector3D::set_cartesian_coordinates(Vector3D& v) {
  v_[0] = v.get_magnitude();
  v_[1] = atan2(sqrt(v[0]*v[0]+v[1]*v[1]),v[2]);
  v_[2] = atan2(v[1],v[0]);
}

bool SphericalVector3D::check(double r, double theta, double psi) {
  if(r<0) {return false;}
  if(theta< -PI || theta>PI) {return false;}
  if(psi< -2*PI || psi>2*PI) {return false;}
  return true;
}


void quasi_evenly_spherical_distribution(unsigned long N,
                              SphericalVector3Ds &vs,double r) {
  vs.resize(N);
  double theta,psi;
  for (unsigned long k=1;k<=N;++k) {
    double h = -1 + 2*((double)k-1)/((double)N-1);
    theta = acos(h);
    if( k==1 || k==N) {
      psi=0;
    } else {
      psi=(vs[k-2][2] + 3.6/sqrt((double)N*(1-h*h)));
      int div = static_cast<int>( psi / (2*PI) );
      psi -= static_cast<double>(div)*(2*PI);
    }
    // Set the values the spherical vector
    vs[k-1][0]=r;
    vs[k-1][1]=theta;
    vs[k-1][2]=psi;
  }
}

void quasi_evenly_semi_spherical_distribution(unsigned long N,
                              SphericalVector3Ds &vs,double r) {
  vs.resize(N);
  double theta,psi;
  for (unsigned long k=1;k<=N;++k) {
    double h = 2*((double)k-1)/(2*N-1) - 1.0;
    theta = acos(h);
    if(k==1) {
      psi=0;
    } else {
      psi=(vs[k-2][2] + 3.6/sqrt((double)2*N*(1.0-h*h)));
      int div = psi/(2*PI);
      psi -= div*2*PI;
    }
    // Set the values the spherical vector
    vs[k-1][0]=r;
    vs[k-1][1]=theta;
    vs[k-1][2]=psi;
  }
}

IMPALGEBRA_END_NAMESPACE
