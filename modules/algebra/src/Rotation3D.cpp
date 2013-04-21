/**
 *  \file Rotation3D.cpp   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/vector_generators.h"
#include "IMP/algebra/utility.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>

IMPALGEBRA_BEGIN_NAMESPACE

Rotation3D::~Rotation3D() {
}


Rotation3D get_rotation_from_matrix(double m11,double m12,double m13,
                                    double m21,double m22,double m23,
                                    double m31,double m32,double m33) {
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    Vector3D v0(m11, m12, m13);
    Vector3D v1(m21, m22, m23);
    Vector3D v2(m31, m32, m33);
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
    Vector3D c0(m11, m21, m31);
    Vector3D c1(m12, m22, m32);
    Vector3D c2(m13, m23, m33);
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
#if IMP_HAS_CHECKS >= IMP_USAGE
    double det = m11*(m22*m33- m23*m32) + m12*(m23*m31-m21*m33)
                   + m13*(m21*m32-m22*m31);
#endif
    IMP_USAGE_CHECK(std::abs(det-1) < .1, "The determinant of the rotation "
                    "matrix is not 1. Got " << det);
  }
  Rotation3D ret;
  {
    double a,b,c,d;
    a = std::abs(1+m11+m22+m33)/4;
    b = std::abs(1+m11-m22-m33)/4;
    c = std::abs(1-m11+m22-m33)/4;
    d = std::abs(1-m11-m22+m33)/4;

    // make sure quat is normalized.
    double sum = a+b+c+d;
    a = std::sqrt(a/sum);
    b = std::sqrt(b/sum);
    c = std::sqrt(c/sum);
    d = std::sqrt(d/sum);

    if (m32-m23 < 0.0) b=-b;
    if (m13-m31 < 0.0) c=-c;
    if (m21-m12 < 0.0) d=-d;
    ret=Rotation3D(a,b,c,d);
#if 0
    IMP_IF_LOG(VERBOSE) {
      Vector3D xr= ret.get_rotated(get_basis_vector_d<3>(0));
      Vector3D yr= ret.get_rotated(get_basis_vector_d<3>(1));
      Vector3D zr= ret.get_rotated(get_basis_vector_d<3>(2));
      IMP_LOG_TERSE( "Got:\n");
      IMP_LOG_TERSE( xr[0] << " " <<  yr[0] << " " <<  zr[0] << std::endl);
      IMP_LOG_TERSE( xr[1] << " " <<  yr[1] << " " <<  zr[1] << std::endl);
      IMP_LOG_TERSE( xr[2] << " " <<  yr[2] << " " <<  zr[2] << std::endl);
    }
#endif
  }
  return ret;
}


const Vector3D Rotation3D::get_derivative(const Vector3D &o,
                                            unsigned int i) const {
    /* The computation was derived in maple. Source code is probably in
       modules/algebra/tools
    */
    double t4 = v_[0]*o[0] - v_[3]*o[1] + v_[2]*o[2];
    double t5 = get_squared(v_[0]);
    double t6 = get_squared(v_[1]);
    double t7 = get_squared(v_[2]);
    double t8 = get_squared(v_[3]);
    double t9 = t5 + t6 + t7 + t8;
    double t10 = 1.0/t9;
    double t11 = 2*t4*t10;
    double t14 = v_[1]*v_[2];
    double t15 = v_[0]*v_[3];

    double t19 = v_[1]*v_[3];
    double t20 = v_[0]*v_[2];
    double t25 = get_squared(t9);
    double t26 = 1.0/t25;

    double t27 = ((t5 + t6 - t7 - t8)*o[0] + 2*(t14 - t15)*o[1]
                  + 2*(t19 + t20)*o[2])*t26;

    double t34 = v_[3]*o[0] + v_[0]*o[1] - v_[1]*o[2];
    double t35 = 2*t34*t10;
    double t41 = v_[2]*v_[3];
    double t42 = v_[0]*v_[1];

    double t47 = (2*(t14 + t15)*o[0] + (t5 - t6 + t7 - t8)*o[1]
                  + 2*(t41 - t42)*o[2])*t26;

    double t54 = -v_[2]*o[0] + v_[1]*o[1] + v_[0]*o[2];
    double t55 = 2*t54*t10;

    double t65 = (2*(t19 - t20)*o[0] + 2*(t41 + t42)*o[1]
                  + (t5 - t6 - t7 + t8)*o[2])*t26;

    double t73 = 2*(v_[1]*o[0] + v_[2]*o[1] + v_[3]*o[2])*t10;

    /*all[1, 1] = t11 - 2*t27*v_[0];
      all[1, 2] = t35 - 2*t47*v_[0];
      all[1, 3] = t55 - 2*t65*v_[0];

      all[2, 1] = t73 - 2*t27*v_[1];
      all[2, 2] = -2*t54 t10 - 2*t47*v_[1];
      all[2, 3] = t35 - 2*t65*v_[1];

      all[3, 1] = t55 - 2*t27*v_[2];
      all[3, 2] = t73 - 2*t47*v_[2];
      all[3, 3] = -2*t4 t10 - 2*t65*v_[2];

      all[4, 1] = -2*t34 t10 - 2*t27*v_[3];
      all[4, 2] = t11 - 2*t47*v_[3];
      all[4, 3] = t73 - 2*t65*v_[3];
    */

    switch (i) {
    case 0:
      return Vector3D(t11 - 2*t27*v_[0],
                        t35 - 2*t47*v_[0],
                        t55 - 2*t65*v_[0]);
    case 1:
      return Vector3D(t73 - 2*t27*v_[1],
                        -2*t54*t10 - 2*t47*v_[1],
                        t35 - 2*t65*v_[1]);
    case 2:
      return Vector3D(t55 - 2*t27*v_[2],
                        t73 - 2*t47*v_[2],
                        -2*t4*t10 - 2*t65*v_[2]);
    case 3:
      return Vector3D(-2*t34*t10 - 2*t27*v_[3],
                        t11 - 2*t47*v_[3],
                        t73 - 2*t65*v_[3]);
    default:
      IMP_THROW("Invalid derivative component",
                base::IndexException);
    };
    return Vector3D(0,0,0);
  }

Rotation3D get_random_rotation_3d() {
  VectorD<4> r= get_random_vector_on<4>(get_unit_sphere_d<4>());
  return Rotation3D(r[0], r[1], r[2], r[3]);
}


Rotation3Ds get_uniform_cover_rotations_3d(unsigned int n) {
  // "surface area" is 2 pi^2 r^3= 2pi^2.
  // each rotation has an area of approximately 4/3 pi distance^3
  base::Vector<VectorD<4> > vs
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
  double d2= get_squared(distance);
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

  /*IMP_LOG_VERBOSE( "Intermedites front: "
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

FixedZXZ get_fixed_zxz_from_rotation(const Rotation3D &r) {

  Vector4D q = r.get_quaternion();
  double q0=q[0];
  double q1=q[1];
  double q2=q[2];
  double q3=q[3];
  double phi=std::atan2((2*(q0*q3+q1*q2)),(1-2*(q2*q2+q3*q3)));
  double theta=std::asin(2*((q0*q2)-(q3*q1)));
  double psi=std::atan2((2*(q0*q1+q2*q3)),(1-2*(q1*q1+q2*q2)));
  return FixedZXZ(phi,theta,psi);
}

FixedZYZ get_fixed_zyz_from_rotation(const Rotation3D &r) {
  // double d22 = c2
  double cos_tilt= r.get_rotated(Vector3D(0,0,1))[2];
  // double d12 = s2 * s3;
  double sin_tilt_sin_psi= r.get_rotated(Vector3D(0,0,1))[1];
  // double d21 = s1 * s2;
  double sin_rot_sin_tilt= r.get_rotated(Vector3D(0,1,0))[2];
  // double d02 = c3 * s2;
  double cos_psi_sin_tilt = r.get_rotated(Vector3D(0,0,1))[0];
  //double d20 = (-1.0) * c1 * s2;
  double cos_rot_sin_tilt = -r.get_rotated(Vector3D(1,0,0))[2];
  double psi= std::atan2(sin_tilt_sin_psi, cos_psi_sin_tilt);
  if (std::abs(sin(psi)) < .01) {
    IMP_THROW("Attempting to divide by 0 in get_fixed_zyz_from_rotation"
              << " bug Daniel about getting a more stable implementation"
              << " or restructure your code to stay with quaternions.",
              base::ValueException);
  }
  double sin_tilt= sin_tilt_sin_psi/std::sin(psi);
  double tilt= std::atan2(sin_tilt, cos_tilt);
  if (std::abs(sin_tilt) < .01) {
    IMP_THROW("Attempting to divide by 0 in get_fixed_zyz_from_rotation"
              << " bug Daniel about getting a more stable implementation"
              << " or restructure your code to stay with quaternions.",
              base::ValueException);
  }
  double cos_rot= cos_rot_sin_tilt/sin_tilt;
  double sin_rot= sin_rot_sin_tilt/sin_tilt;
  double rot= std::atan2(sin_rot, cos_rot);
  /*IMP_LOG_VERBOSE( "Intermedites back: "
          << cos_rot << " " << cos_tilt << " "
          << cos_psi_sin_tilt/sin_tilt << "\n"
          << sin_rot << " " << sin_tilt << " "
          << sin_tilt_sin_psi/sin_tilt << std::endl);*/
  IMP_IF_CHECK(base::USAGE) {
    Rotation3D rrot= get_rotation_from_fixed_zyz(rot, tilt, psi);
    IMP_LOG(base::VERBOSE,
            "Input is " << r << " output results in " << rrot << std::endl);
    IMP_INTERNAL_CHECK(get_distance(r, rrot) < .1,
               "The input and output rotations are far apart " << r
               << " and " << rrot << std::endl);
  }
  return FixedZYZ(rot, tilt, psi);
}

FixedXYZ get_fixed_xyz_from_rotation(const Rotation3D &r) {
  VectorD<4> quat = r.get_quaternion();
  double q00 = get_squared(quat[0]);
  double q11 = get_squared(quat[1]);
  double q22 = get_squared(quat[2]);
  double q33 = get_squared(quat[3]);
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

Rotation3D get_rotation_about_normalized_axis(const Vector3D& axis_norm,
                                              double angle)
{
  IMP_USAGE_CHECK(axis_norm.get_magnitude() - 1.0 < 1e-6,
                  "expected normalized vector as axis of rotation");
  double s = std::sin(angle/2);
  double a,b,c,d;
  a = std::cos(angle/2);
  b = axis_norm[0]*s;
  c = axis_norm[1]*s;
  d = axis_norm[2]*s;
  return Rotation3D(a,b,c,d);
}

Rotation3D get_rotation_about_axis(const Vector3D& axis,
                                   double angle)
{
  //normalize the vector
  Vector3D axis_norm = axis.get_unit_vector();
  return get_rotation_about_normalized_axis(axis_norm, angle);
}

Rotation3D get_rotation_taking_first_to_second(const Vector3D &v1,
                                               const Vector3D &v2) {
  Vector3D v1_norm = v1.get_unit_vector();
  Vector3D v2_norm = v2.get_unit_vector();
  //get a vector that is perpendicular to the plane containing v1 and v2
  Vector3D vv = get_vector_product(v1_norm,v2_norm);
  //get the angle between v1 and v2
  double dot = v1_norm*v2_norm;
  dot = ( dot < -1.0 ? -1.0 : ( dot > 1.0 ? 1.0 : dot ) );
  double angle = std::acos(dot);
  //check a special case: the input vectors are parallel / antiparallel
  if (std::abs(dot) >.999999) {
    IMP_LOG_VERBOSE(" the input vectors are (anti)parallel "<<std::endl);
    return get_rotation_about_axis(get_orthogonal_vector(v1),
                                              angle);
  }
  return get_rotation_about_axis(vv,angle);
}


Rotation3D get_rotation_from_x_y_axes(const Vector3D &x,
                                             const Vector3D &y) {
  IMP_USAGE_CHECK(std::abs(x.get_squared_magnitude()-1.0) < .1,
                  "The x vector is not a unit vector.");
  IMP_USAGE_CHECK(std::abs(y.get_squared_magnitude()-1.0) < .1,
                  "The y vector is not a unit vector.");
  IMP_USAGE_CHECK(std::abs(x*y) < .1,
                  "The x and y vectors are not perpendicular.");
  Vector3D z = get_vector_product(x,y);
  Rotation3D rot = get_rotation_from_matrix(x[0],y[0], z[0],
                                            x[1], y[1], z[1],
                                            x[2],y[2],z[2]);
  return rot;
}

std::pair<Vector3D,double> get_axis_and_angle(
                                      const Rotation3D &rot) {
  VectorD<4> q = rot.get_quaternion();
  double a,b,c,d;
  a=q[0];b=q[1];c=q[2];d=q[3];
  if (std::abs(a) > .9999) return std::make_pair(Vector3D(1,0,0),0.0);
  double angle = std::acos(a)*2;
  double s = std::sin(angle/2);
  Vector3D axis(b/s,c/s,d/s);
  return std::make_pair(axis.get_unit_vector(),angle);
}




//! Generates a nondegenerate set of Euler angles with a delta resolution
algebra::Rotation3Ds get_uniformly_sampled_rotations(double delta_rad) {
  double delta= delta_rad/PI*180.0;
  Vector3D eu_start(0.,0.,0.);//psi,theta,phi
  Vector3D eu_end(360.,180.,360.);
  Vector3D eu_range=eu_end-eu_start;
  double phi_steps       = get_rounded((eu_range[2]/delta) + 0.499);
  double phi_real_dist   = eu_range[2] / phi_steps;
  double theta_steps     = get_rounded((eu_range[1]/delta) + 0.499);
  double theta_real_dist = eu_range[1] / theta_steps;
  double angle2rad=PI/180.;
  double psi_steps,psi_ang_dist,psi_real_dist;

  algebra::Rotation3Ds ret;
  for (double phi=eu_start[2]; phi < eu_end[2];  phi+=phi_real_dist) {
    for (double theta=eu_start[1]; theta <= eu_end[1]; theta+=theta_real_dist) {
      if (theta == 0.0 || theta == 180.0) {
        psi_steps = 1;
      }
      else {
        psi_steps = get_rounded(360.0*std::cos((90.0-theta)*angle2rad)/delta);
      }
      psi_ang_dist  = 360.0/psi_steps;
      psi_real_dist = eu_range[0] / (ceil(eu_range[0]/psi_ang_dist));
      for (double psi=eu_start[0]; psi < eu_end[0];  psi+=psi_real_dist)  {
        ret.push_back(get_rotation_from_fixed_zxz(
                                                  phi*angle2rad,
                                                  theta*angle2rad,
                                                  psi*angle2rad));
      }}}
  return ret;
}








IMPALGEBRA_END_NAMESPACE
