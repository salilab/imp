/**
 *  \file project.cpp
 *  \brief Projection operation for 3D volumes
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#include <IMP/em/project.h>

IMPEM_BEGIN_NAMESPACE

void project_given_direction(DensityMap& map,
             IMP::algebra::Matrix2D<double>& m2,
             const int Ydim,const int Xdim,
             IMP::algebra::Vector3D& direction,
             const IMP::algebra::Vector3D& shift,
             const double equality_tolerance) {

  IMP::algebra::SphericalVector3D sph(direction);
  algebra::Rotation3D angles
    = algebra::rotation_from_fixed_zyz(sph[2],sph[1],0.0);
  project_given_rotation(map,m2,Ydim,Xdim,angles,shift,equality_tolerance);
};


void project_given_rotation(DensityMap& map,
             IMP::algebra::Matrix2D<double>& m2,
             const int Ydim,const int Xdim,
             const IMP::algebra::Rotation3D& Rot,
             const IMP::algebra::Vector3D& shift,
             const double equality_tolerance) {

// #define DEBUG
  m2.resize(Ydim, Xdim);
  // Save the origin of the matrices
  std::vector<int> orig2D(2), orig3D(3);
  orig2D[0]=m2.get_start(0); // X
  orig2D[1]=m2.get_start(1); // Y
  orig3D[0]=map.get_header()->get_xorigin();
  orig3D[1]=map.get_header()->get_yorigin();
  orig3D[2]=map.get_header()->get_zorigin();
  float voxelsize=map.get_header()->get_spacing();
  // Center the 2D matrix (necessary for the projection algorithm)
  m2.centered_start();
  // Set the pixel size of the map to 1 (necessary for the projection algorithm)
  map.get_header_writable()->set_spacing(1.0);
  // Center the map (necessary for the projection algorithm)
  map.set_origin((-1)*(int)(map.get_header()->nx/2.),
                 (-1)*(int)(map.get_header()->ny/2.),
                 (-1)*(int)(map.get_header()->nz/2.));

  // Get the rotation and the direction from the Euler angles
  IMP::algebra::Rotation3D InvRot = Rot.get_inverse();
  IMP::algebra::Vector3D direction;// = RotMat.direction();
  for (unsigned int i=0; i< 3; ++i) {
    IMP::algebra::Vector3D v=algebra::basis_vector<3>(i);
    algebra::Vector3D r= Rot.rotate(v);
    direction[i]=r[2];
  }

  // For each pixel, 4 rays of projection are computed on each direction. The
  // step is going to be 1/3 from the center of the pixel in 4 directions
  double step = 1.0 / 3.0;
  // Avoids divisions by zero and allows orthogonal rays computation
  // (If any of the direction of projection's component is zero)
  for (int ii = 0;ii < 3;ii++) {
    if (std::abs(direction[ii]) < equality_tolerance) {
      direction[ii] = equality_tolerance;
    }
  }

#ifdef DEBUG
  std::cout << " direction " << direction << std::endl;
  std::cout << "Rotation: " << Rot << std::endl;
 #endif

  // Logical ints for the beginning of the map
  IMP::algebra::Vector3D init0, end0, signs, half_signs;
  init0[0] = map.get_header()->get_xorigin();
  init0[1] = map.get_header()->get_yorigin();
  init0[2] = map.get_header()->get_zorigin();
  // Logical ints for the end of the map. The next 3 lines make sense
  // because the voxel size is set to 1.0
  end0[0] = init0[0]-1+map.get_header()->nx;
  end0[1] = init0[1]-1+map.get_header()->ny;
  end0[2] = init0[2]-1+map.get_header()->nz;
  for (int i = 0;i < 3;i++) {
    signs[i] = IMP::algebra::sign(direction[i]);
    half_signs[i] = 0.5 * signs[i];
  }

  IMP::algebra::Vector3D r; // A point in the coordinate system for the map
  IMP::algebra::Vector3D p; // A point in the coord. system of the projection

#ifdef DEBUG
  std::string fn_p = "plane_p.txt";
  std::string fn_r = "plane_r.txt";
  std::ofstream f_p,f_r;
  f_p.open(fn_p.c_str(), std::ios::out);
  f_r.open(fn_r.c_str(), std::ios::out);
#endif

  // build projection
  for (int j = m2.get_start(1);j <= m2.get_finish(1);j++) { // X
    for (int i = m2.get_start(0);i <= m2.get_finish(0);i++) { // Y

      double ray_sum = 0.0;  // Line integral value
      for (int rays_per_pixel = 0; rays_per_pixel < 4; rays_per_pixel++) {
#ifdef DEBUG
         std::cout << "(" << j << "," << i << ") init ray " <<
                      rays_per_pixel << std::endl;
#endif
        // point in projection coordinate system
        switch (rays_per_pixel) {
        case 0:
          p = IMP::algebra::Vector3D(j - step, i - step, 0);
          break;
        case 1:
          p = IMP::algebra::Vector3D(j - step, i + step, 0);
          break;
        case 2:
          p = IMP::algebra::Vector3D(j + step, i - step, 0);
          break;
        case 3:
          p = IMP::algebra::Vector3D(j + step, i + step, 0);
          break;
        }

        // Get the coordinates r in the universal system corresponding to
        // the point p
        if (!shift.is_zero(equality_tolerance)) {
          p -= shift;
        }

        r = InvRot.rotate(p);

#ifdef DEBUG
       std::cout << "p: " << p << std::endl;
       std::cout << "r: " << r << std::endl;
        if(rays_per_pixel==0) {
          f_p << p << std::endl;
          f_r << r << std::endl;
        }
#endif
        // Compute the minimum and maximum alpha for the line of the ray
        // intersecting the given volume. line = r + alpha * direction
        IMP::algebra::Vector3D v_alpha_min, v_alpha_max, v_alpha, v_diff;
        double alpha_min=-1/equality_tolerance;
        double alpha_max=1/equality_tolerance;
        for (int ii = 0;ii < 3;ii++) {
          v_alpha_min[ii] = (init0[ii] - 0.5 - r[ii]) / direction[ii];
          v_alpha_max[ii] = ( end0[ii] + 0.5 - r[ii]) / direction[ii];
          // Consider this index if the component of the direction
          // is not almost zero
          if(std::abs(direction[ii]) > equality_tolerance ) {
            alpha_min=std::max(alpha_min,
                      std::min(v_alpha_min[ii],v_alpha_max[ii]));
            alpha_max=std::min(alpha_max,
                      std::max(v_alpha_min[ii],v_alpha_max[ii]));
          }
        }

#ifdef DEBUG
        std::cout << "v_alpha_min " << v_alpha_min;
        std::cout << " v_alpha_max " << v_alpha_max << std::endl;
#endif
        if (IMP::algebra::almost_equal(
           alpha_max, alpha_min, equality_tolerance)) {
#ifdef DEBUG
         std::cout << " ray skipped (" << j << "," << i << ") init ray " <<
                      rays_per_pixel << std::endl;
#endif
          continue;
        }
        // v is going to be the first voxel in the volume intersecting the ray
        IMP::algebra::Vector3D v;
        // idx is a vector of logical ints indicating a voxel of the map
        std::vector<int> idx(3);
        v = r + alpha_min * direction; // vector operation

#ifdef DEBUG
        std::cout << " v " << v << std::endl;

        std::ofstream f_txt;
        if(rays_per_pixel==0) {
          std::cout << "(" << j << "," << i << ") init ray " <<
                        rays_per_pixel << std::endl;
          std::string fn_txt = "m2_";
          fn_txt+=IMP::algebra::internal::float_to_string(i,0,0);
          fn_txt+="_";
          fn_txt+=IMP::algebra::internal::float_to_string(j,0,0);
          fn_txt+="_ray_";
          fn_txt+=IMP::algebra::internal::float_to_string(rays_per_pixel,0,0);
          fn_txt+=".txt";

          f_txt.open(fn_txt.c_str(), std::ios::out);
          f_txt << v << std::endl;
        }
#endif
        // Index of the first voxel
        for (int ii=0;ii < 3;ii++) {
          if (v[ii] >= 0.) {
            idx[ii] = IMP::algebra::constrain((double)((int)(v[ii] + 0.5)),
                                              init0[ii], end0[ii]);
          } else {
            idx[ii] = IMP::algebra::constrain((double)((int)(v[ii] - 0.5)),
                                              init0[ii], end0[ii]);
          }
#ifdef DEBUG
          std::cout << " idx[" << ii << "]= " << idx[ii];
#endif
        }
#ifdef DEBUG
        std::cout << std::endl;
#endif
        // Follow the ray
        double alpha = alpha_min;
#ifdef DEBUG
        std::cout << " alpha_min " << alpha_min;
        std::cout << " alpha_max " << alpha_max;
        std::cout << " initial alpha " << alpha << std::endl;
#endif
        do {
          for (int ii=0;ii < 3;ii++) {
            v_alpha[ii] = (idx[ii] + half_signs[ii] - r[ii]) / direction[ii];
            v_diff[ii] = std::abs(alpha - v_alpha[ii]);
          }
          // Determine the dimension the ray will move in the next step.
          // (Is the mininum value in v_diff)
#ifdef DEBUG
          std::cout << " v_alpha " << v_alpha << std::endl;
          std::cout << " v_diff " << v_diff << std::endl;
#endif
          double diff_alpha=std::min(std::min(v_diff[0],v_diff[1]),v_diff[2]);

#ifdef DEBUG
          std::cout << " diff_alpha " << diff_alpha << std::endl;
          std::cout << "voxel indexes " <<
              idx[0] << " " << idx[1] << " " << idx[2] << " | ";
          std::cout << "map(idx) = "
          << map.get_value(map.loc2voxel(idx[0],idx[1],idx[2]));
#endif
          ray_sum += diff_alpha *
                     map.get_value(map.loc2voxel(idx[0],idx[1],idx[2]));
          // update the indexes in the required dimensions
          for (int ii=0;ii < 3;ii++) {
            if (IMP::algebra::almost_equal(diff_alpha, v_diff[ii],
                equality_tolerance)) {
              alpha = v_alpha[ii];
              idx[ii] += signs[ii];
            }
          }
#ifdef DEBUG
          std::cout << " alpha =" << alpha << std::endl;
          if(rays_per_pixel==0) {
            v = r + alpha * direction; // vector operation
            f_txt << v << std::endl;
          }
#endif
        } while ((alpha_max - alpha) > equality_tolerance); // end of the ray


#ifdef DEBUG
        std::cout << " final alpha =" << alpha << std::endl;
        std::cout << " ray_sum =" << ray_sum << std::endl;
        if(rays_per_pixel==0) {
         f_txt.close();
        }
#endif
      } // for involving the 4 rays
      // Average the value of the 4 rays
      m2(i,j) = ray_sum * 0.25;
#ifdef DEBUG
       std::cout << "m2(" << i << "," << j << ")=  " << m2(i,j) << std::endl;
#endif
    } // i for
  } // j for

#ifdef DEBUG
  f_p.close();
  f_r.close();
#endif


  // Reindex the matrix again
  m2.reindex(orig2D);
  // Restore the map parameters
  map.get_header_writable()->set_spacing(voxelsize);
  map.set_origin(orig3D[0],orig3D[1],orig3D[2]);
};


IMPEM_END_NAMESPACE
