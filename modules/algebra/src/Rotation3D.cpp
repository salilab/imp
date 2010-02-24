/**
 *  \file Rotation3D.cpp   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/vector_generators.h"

IMPALGEBRA_BEGIN_NAMESPACE

Rotation3D::~Rotation3D() {
}

Rotation3D Rotation3D::get_inverse() const {
  IMP_USAGE_CHECK(v_.get_squared_magnitude() != 0,
            "Attempting to invert uninitialized rotation");
  Rotation3D ret(v_[0], -v_[1], -v_[2], -v_[3]);
  return ret;
}



Rotation3D get_rotation_from_matrix(double m11,double m12,double m13,
                                    double m21,double m22,double m23,
                                    double m31,double m32,double m33) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    VectorD<3> v0(m11, m12, m13);
    VectorD<3> v1(m21, m22, m23);
    VectorD<3> v2(m31, m32, m33);
    IMP_USAGE_CHECK(std::abs(v0.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 0).");
    IMP_USAGE_CHECK(std::abs(v1.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 1).");
    IMP_USAGE_CHECK(std::abs(v2.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 2).");
    IMP_USAGE_CHECK(std::abs(v0 *v1) < .1,
              "The passed matrix is not a rotation matrix (row 0, row 1).");
    IMP_USAGE_CHECK(std::abs(v0 *v2) < .1,
              "The passed matrix is not a rotation matrix (row 0, row 2).");
    IMP_USAGE_CHECK(std::abs(v1 *v2) < .1,
              "The passed matrix is not a rotation matrix (row 1, row 2).");
    VectorD<3> c0(m11, m21, m31);
    VectorD<3> c1(m12, m22, m32);
    VectorD<3> c2(m13, m23, m33);
    IMP_USAGE_CHECK(std::abs(c0.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 0).");
    IMP_USAGE_CHECK(std::abs(c1.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 1).");
    IMP_USAGE_CHECK(std::abs(c2.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 2).");
    IMP_USAGE_CHECK(std::abs(c0 *c1) < .1,
              "The passed matrix is not a rotation matrix (col 0, col 1).");
    IMP_USAGE_CHECK(std::abs(c0 *c2) < .1,
              "The passed matrix is not a rotation matrix (col 0, col 2).");
    IMP_USAGE_CHECK(std::abs(c1 *c2) < .1,
              "The passed matrix is not a rotation matrix (col 1, col 2).");
  }
  double a,b,c,d;
  a = fabs(1+m11+m22+m33)/4;
  b = fabs(1+m11-m22-m33)/4;
  c = fabs(1-m11+m22-m33)/4;
  d = fabs(1-m11-m22+m33)/4;

  // make sure quat is normalized.
  double sum = a+b+c+d;
  a = sqrt(a/sum);
  b = sqrt(b/sum);
  c = sqrt(c/sum);
  d = sqrt(d/sum);

  if (m32-m23 < 0.0) b=-b;
  if (m13-m31 < 0.0) c=-c;
  if (m21-m12 < 0.0) d=-d;
  return Rotation3D(a,b,c,d);
}

Rotation3D get_random_rotation_3d() {
  VectorD<4> r= get_random_vector_on<4>(get_unit_sphere_d<4>());
  return Rotation3D(r[0], r[1], r[2], r[3]);
}


Rotation3Ds get_uniform_cover_rotations_3d(unsigned int n) {
  // "surface area" is 2 pi^2 r^3= 2pi^2.
  // each rotation has an area of approximately 4/3 pi distance^3
  std::vector<VectorD<4> > vs
    = internal::uniform_cover_sphere<4>(n,
                                        get_zero_vector_d<4>(),
                                        1, false);
  Rotation3Ds ret;
  for (unsigned int i=0; i< vs.size(); ++i) {
    if (vs[i][0] <=0) {
      vs[i]= -vs[i];
    }
    ret.push_back(Rotation3D(vs[i][0],
                             vs[i][1],
                             vs[i][2],
                             vs[i][3]));
  }
  return ret;
}


Rotation3D get_random_rotation_3d(const Rotation3D &center,
                                  double distance) {
  unsigned int count=0;
  double d2= square(distance);
  while (count < 10000) {
    Rotation3D rr= get_random_rotation_3d();
    if (get_distance(center, rr) < d2) {
      return rr;
    }
    ++count;
  }
  IMP_FAILURE("Unable to find a suitably close rotation");
}

Rotation3D get_rotation_from_fixed_xyz(double xr,double yr, double zr)
{
  double a,b,c,d;
  double cx = cos(xr);  double cy = cos(yr);  double cz = cos(zr);
  double sx = sin(xr);  double sy = sin(yr);  double sz = sin(zr);
  double m00 = cz*cy;
  double m11 = -sy*sx*sz + cx*cz;
  double m22 = cy*cx;
  double zero =0.0;
  a = std::sqrt(std::max(1+m00+m11+m22,zero))/2.0;
  b = std::sqrt(std::max(1+m00-m11-m22,zero))/2.0;
  c = std::sqrt(std::max(1-m00+m11-m22,zero))/2.0;
  d = std::sqrt(std::max(1-m00-m11+m22,zero))/2.0;
  if (cy*sx + sy*cx*sz + sx*cz < 0.0) b = -b;
  if (sz*sx - sy*cx*cz - sy < 0.0)    c = -c;
  if (sz*cy + sy*sx*cz + sz*cx < 0.0) d = -d;
  return Rotation3D(a,b,c,d);
}

Rotation3D get_rotation_from_fixed_zxz(double phi, double theta, double psi)
{
  double a,b,c,d;
  double c1,c2,c3,s1,s2,s3;
  c2=std::cos(theta/2);c1=cos(phi/2);c3=cos(psi/2);
  s2=std::sin(theta/2);s1=sin(phi/2);s3=sin(psi/2);
  a = c1*c2*c3+s1*s2*s3;
  b = s1*c2*c3-c1*s2*s3;
  c = c1*s2*c3+s1*c2*s3;
  d = c1*c2*s3-s1*s2*c3;
  return Rotation3D(a,b,c,d);
}


Rotation3D get_rotation_from_fixed_zyz(double Rot, double Tilt, double Psi) {
  double c1 = std::cos(Rot);
  double c2 = std::cos(Tilt);
  double c3 = std::cos(Psi);
  double s1 = std::sin(Rot);
  double s2 = std::sin(Tilt);
  double s3 = std::sin(Psi);

  /*IMP_LOG(VERBOSE, "Intermedites front: "
          << c1 << " " << c2 << " " << c3 << "\n"
          << s1 << " " << s2 << " " << s3 << std::endl);*/
  double d00 = c1 * c2 * c3 - s1 * s3;
  double d01 = (-1.0) * c2 * c3 * s1 - c1 * s3;
  double d02 = c3 * s2;
  double d10 = c3 * s1 + c1 * c2 * s3;
  double d11 = c1 * c3 - c2 * s1 * s3;
  double d12 = s2 * s3;
  double d20 = (-1.0) * c1 * s2;
  double d21 = s1 * s2;
  double d22 = c2;
  Rotation3D rot= get_rotation_from_matrix(d00, d01, d02,
                                           d10, d11, d12,
                                           d20, d21, d22);
  return rot;
}



FixedZYZ get_fixed_zyz_from_rotation(const Rotation3D &r) {
  // double d22 = c2
  double cos_tilt= r.get_rotated(VectorD<3>(0,0,1))[2];
  // double d12 = s2 * s3;
  double sin_tilt_sin_psi= r.get_rotated(VectorD<3>(0,0,1))[1];
  // double d21 = s1 * s2;
  double sin_rot_sin_tilt= r.get_rotated(VectorD<3>(0,1,0))[2];
  // double d02 = c3 * s2;
  double cos_psi_sin_tilt = r.get_rotated(VectorD<3>(0,0,1))[0];
  //double d20 = (-1.0) * c1 * s2;
  double cos_rot_sin_tilt = -r.get_rotated(VectorD<3>(1,0,0))[2];
  double psi= std::atan2(sin_tilt_sin_psi, cos_psi_sin_tilt);
  double sin_tilt= sin_tilt_sin_psi/std::sin(psi);
  double tilt= std::atan2(sin_tilt, cos_tilt);
  double cos_rot= cos_rot_sin_tilt/sin_tilt;
  double sin_rot= sin_rot_sin_tilt/sin_tilt;
  double rot= std::atan2(sin_rot, cos_rot);
  /*IMP_LOG(VERBOSE, "Intermedites back: "
          << cos_rot << " " << cos_tilt << " "
          << cos_psi_sin_tilt/sin_tilt << "\n"
          << sin_rot << " " << sin_tilt << " "
          << sin_tilt_sin_psi/sin_tilt << std::endl);*/
  IMP_IF_CHECK(USAGE) {
    Rotation3D rrot= get_rotation_from_fixed_zyz(rot, tilt, psi);
    IMP_LOG(VERBOSE,
            "Input is " << r << " output results in " << rrot << std::endl);
    IMP_INTERNAL_CHECK((rrot.get_quaternion()
                -r.get_quaternion()).get_squared_magnitude() < .1,
               "The input and output rotations are far apart " << r
               << " and " << rrot << std::endl);
  }
  //if (rot > PI/2) rot=rot-PI;
  //if (psi > PI/2) psi= psi-PI;
  return FixedZYZ(rot, tilt, psi);
}

FixedXYZ get_fixed_xyz_from_rotation(const Rotation3D &r) {
  VectorD<4> quat = r.get_quaternion();
  double q00 = square(quat[0]);
  double q11 = square(quat[1]);
  double q22 = square(quat[2]);
  double q33 = square(quat[3]);
  double mat11 = q00 +  q11 - q22 - q33;
  double mat21 = 2*(quat[1]*quat[2] + quat[0]*quat[3]);
  //double mat23 = 2*(quat[2]*quat[3] - quat[0]*quat[1]);
  double mat31 = 2*(quat[1]*quat[3] - quat[0]*quat[2]);
  double mat32 = 2*(quat[2]*quat[3] + quat[0]*quat[1]);
  double mat33 = q00 - q11 - q22 + q33;
  return FixedXYZ(std::atan2(mat32, mat33),
     std::atan2(mat31, std::sqrt(std::pow(mat21,2)+ std::pow(mat11,2))),
     std::atan2(mat21, mat11));
}
IMPALGEBRA_END_NAMESPACE
