/**
 *  \file project.cpp
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/project.h"
#include "IMP/em2d/CenteredMat.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/internal/rotation_helper.h"
#include "IMP/em/image_transformations.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/atom/Mass.h"

#include "IMP/Pointer.h"
#include "IMP/core/utility.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>

IMPEM2D_BEGIN_NAMESPACE


em::Images generate_projections(const ParticlesTemp &ps,
                    const algebra::SphericalVector3Ds vs,
                    int rows, int cols,
                    double resolution, double pixelsize,
                    em::ImageReaderWriter<double> &srw,
                    bool project_and_save,
                    Strings names) {
  unsigned long n_projs= vs.size();
  RegistrationResults registration_values(n_projs);
  for (unsigned long i=0;i<n_projs;++i) {
    algebra::Rotation3D R=
        em2d::internal::get_rotation_from_projection_direction(vs[i]);
    algebra::Vector2D shift(0.0,0.0);
    RegistrationResult rr(R,shift);
    registration_values[i]=rr;
  }
  return generate_projections(ps,registration_values,rows,cols,resolution,
                    pixelsize,srw,project_and_save,names);
}

em2d::Images generate_projections(const ParticlesTemp &ps,
                    const algebra::SphericalVector3Ds vs,
                    int rows, int cols,
                    double resolution, double pixelsize,
                    em2d::ImageReaderWriter<double> &srw,
                    bool project_and_save,
                    Strings names) {
  IMP_LOG(IMP::VERBOSE,
            "Generating projections from spherical vectors" << std::endl);
  unsigned long n_projs= vs.size();
  RegistrationResults registration_values(n_projs);
  for (unsigned long i=0;i<n_projs;++i) {
    algebra::Rotation3D R=
        em2d::internal::get_rotation_from_projection_direction(vs[i]);
    algebra::Vector2D shift(0.0,0.0);
    RegistrationResult rr(R,shift);
    registration_values[i]=rr;
  }
  return generate_projections(ps,registration_values,rows,cols,resolution,
                    pixelsize,srw,project_and_save,names);
}


em::Images generate_projections(const ParticlesTemp &ps,
                    RegistrationResults registration_values,
                    int rows, int cols,
                    double resolution, double pixelsize,
                    em::ImageReaderWriter<double> &srw,
                    bool project_and_save,
                    Strings names) {
  unsigned long n_projs= registration_values.size();
  em::Images projections(n_projs);
  // Precomputation of all the possible projection masks for the particles
  OldMasksManager masks(resolution,pixelsize);
  masks.generate_masks(ps);
  for (unsigned long i=0;i<n_projs;++i) {
    IMP_NEW(em::Image,img,());
    img->resize(rows,cols);
    String name="";
    if(project_and_save) { name = names[i]; } // deal with name only if saving
    generate_projection(*img,ps,registration_values[i],
                resolution,pixelsize,srw,project_and_save,&masks,name);
    projections.set(i,img);
  }
  return projections;
}

em2d::Images generate_projections(const ParticlesTemp &ps,
                    RegistrationResults registration_values,
                    int rows, int cols,
                    double resolution, double pixelsize,
                    em2d::ImageReaderWriter<double> &srw,
                    bool project_and_save,
                    Strings names) {
  IMP_LOG(IMP::VERBOSE,
          "Generating projections from registraion results" << std::endl);

  unsigned long n_projs= registration_values.size();
  em2d::Images projections(n_projs);
  // Precomputation of all the possible projection masks for the particles
  Masks_Manager masks(resolution,pixelsize);
  masks.generate_masks(ps);
  for (unsigned long i=0;i<n_projs;++i) {
    IMP_NEW(em2d::Image,img,());
    img->resize(rows,cols);
    String name="";
    if(project_and_save) { name = names[i]; } // deal with name only if saving
    generate_projection(*img,ps,registration_values[i],
                resolution,pixelsize,srw,project_and_save,&masks,name);
    projections.set(i,img);
  }
  return projections;
}



void generate_projection(em::Image &img,const ParticlesTemp &ps,
            RegistrationResult &reg,
            double resolution, double pixelsize,
           em::ImageReaderWriter<double> &srw,bool save_image,
           OldMasksManager *masks,String name) {
  if(masks==NULL) {
    masks = new OldMasksManager(resolution,pixelsize);
    masks->generate_masks(ps);
  }
  algebra::Vector3D translation = pixelsize*reg.get_shift3D();
  algebra::Rotation3D R = reg.get_rotation();
  project_particles(ps,img.get_data(),R,translation,resolution,pixelsize,masks);
  em::normalize(img,true);
  reg.set_in_image(img.get_header());
  img.get_header().set_object_pixel_size(pixelsize);
  if(save_image) {
    img.write_to_floats(name,srw);
  }
}


void generate_projection(em2d::Image &img,const ParticlesTemp &ps,
            RegistrationResult &reg,
            double resolution, double pixelsize,
           em2d::ImageReaderWriter<double> &srw,bool save_image,
           Masks_Manager *masks,String name) {
  IMP_LOG(IMP::VERBOSE,"Generating projection in a em2d::Image" << std::endl);

  if(masks==NULL) {
    masks = new Masks_Manager(resolution,pixelsize);
    masks->generate_masks(ps);
  }
  IMP_LOG(IMP::VERBOSE, "Masks generated from generate_projection()"
                                                                << std::endl);
  algebra::Vector3D translation = pixelsize*reg.get_shift3D();
  algebra::Rotation3D R = reg.get_rotation();
  project_particles(ps,img.get_data(),R,translation,resolution,pixelsize,masks);
  em2d::normalize(img,true);
  reg.set_in_image(img.get_header());
  img.get_header().set_object_pixel_size(pixelsize);
  if(save_image) {
    img.write_to_floats(name,srw);
  }
}

// rotates respect the centroid of the particles and applies translation.
void project_particles(const ParticlesTemp &ps,
             algebra::Matrix2D_d &m2,
             algebra::Rotation3D &R,
             algebra::Vector3D &translation,
             double resolution, double pixelsize,
             OldMasksManager *masks) {
  if(masks==NULL) {
    // Create the masks
    masks = new OldMasksManager(resolution,pixelsize);
    masks->generate_masks(ps);
  }
  // clear data before creating a new projection
  m2.fill_with_value(0.0);
  // save and center image
  int orig2D[2];
  orig2D[0]=m2.get_start(0);
  orig2D[1]=m2.get_start(1);
  m2.centered_start();
  // Centroid
  unsigned long n_particles = ps.size();
  algebra::Vector3D centroid(0.0,0.0,0.0);
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZ xyz(ps[i]);
    centroid += xyz.get_coordinates();
  }
  centroid /= n_particles;

  // Project
  for (unsigned long i=0; i<n_particles; i++) {
    // Coordinates respect to the centroid
    core::XYZR xyzr(ps[i]);
    atom::Mass mass(ps[i]);
    algebra::Vector3D p=xyzr.get_coordinates()-centroid;
    // Pixel after trasformation to project in Z axis
    // Not necessary to compute pz, is going to be ignored
    double pix_x = (R.get_rotated_one_coordinate(p,0)+translation[0])/pixelsize;
    double pix_y = (R.get_rotated_one_coordinate(p,1)+translation[1])/pixelsize;
    // Apply mask
    OldProjectionMask* mask = masks->find_mask(xyzr.get_radius());
    algebra::Vector2D pix(pix_x,pix_y);
    mask->apply(m2,pix,mass.get_mass());
  }
  // Restore origin
  m2.set_start(0,orig2D[0]);
  m2.set_start(1,orig2D[1]);
}


void project_particles(const ParticlesTemp &ps,
             cv::Mat &m2,
             algebra::Rotation3D &R,
             algebra::Vector3D &translation,
             double resolution, double pixelsize,
             Masks_Manager *masks) {
  IMP_LOG(IMP::VERBOSE,"Projecting particles in a openCV matrix" << std::endl);
  if(m2.empty()) {
    IMP_THROW("Cannot project on a empty matrix",ValueException);
  }
  if(masks==NULL) {
    // Create the masks
    masks = new Masks_Manager(resolution,pixelsize);
    masks->generate_masks(ps);
  }
  // Centroid
  unsigned long n_particles = ps.size();
  algebra::Vector3D centroid(0.0,0.0,0.0);
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZ xyz(ps[i]);
    centroid += xyz.get_coordinates();
  }
  centroid /= n_particles;
  // clear data before creating a new projection
  m2.setTo(0.0);
  // Project
  for (unsigned long i=0; i<n_particles; i++) {
    // Coordinates respect to the centroid
    core::XYZR xyzr(ps[i]);
    atom::Mass mass(ps[i]);
    algebra::Vector3D p=xyzr.get_coordinates()-centroid;
    // Pixel after trasformation to project in Z axis
    // Not necessary to compute pz, is going to be ignored
    double pix_x = (R.get_rotated_one_coordinate(p,0)+translation[0])/pixelsize;
    double pix_y = (R.get_rotated_one_coordinate(p,1)+translation[1])/pixelsize;
    // Apply mask
    Projection_Mask* mask = masks->find_mask(xyzr.get_radius());
    algebra::Vector2D pix(pix_x,pix_y);
    mask->apply(m2,pix,mass.get_mass());
  }
}




// Core function for projection of particles
algebra::Vector2Ds project_vectors(const algebra::Vector3Ds &ps,
             const algebra::Rotation3D &R,
             const algebra::Vector3D &translation) {
  algebra::Vector3D centroid = algebra::get_centroid(ps);
  return project_vectors(ps,R,translation,centroid);
}

// Core function for projection of particles. Rotates respect to the centroid
// and translates
algebra::Vector2Ds project_vectors(const algebra::Vector3Ds &ps,
            const algebra::Rotation3D &R,const algebra::Vector3D &translation,
            const algebra::Vector3D &center) {
  // Project
  unsigned long n_particles = ps.size();
  algebra::Vector3D p;
  algebra::Vector2Ds vs(n_particles);
  for (unsigned long i=0; i<n_particles; i++) {
    // Coordinates respect to the centroid
    p[0] = ps[i][0] - center[0];
    p[1] = ps[i][1] - center[1];
    p[2] = ps[i][2] - center[2];
    // Point after trasformation to project in Z axis
    // Not necessary to compute pz, is going to be ignored
    double px = R.get_rotated_one_coordinate(p,0)+translation[0];
    double py = R.get_rotated_one_coordinate(p,1)+translation[1];
    // Project is simply ignore Z-coordinate
    vs[i]=algebra::Vector2D(px,py);
  }
  return vs;
}



em::Images generate_projections(em::DensityMap &map,
                    const algebra::SphericalVector3Ds &vs,
                    const int rows,const int cols,
                    em::ImageReaderWriter<double> &srw,
                    const bool project_and_save,
                    Strings names) {
  unsigned long n_projs= vs.size();
  em::Images images(n_projs);
  // Generate the projections
  for (unsigned long i=0;i<n_projs;++i) {
    Pointer<em::Image> img(new em::Image());
    img->resize(rows,cols);
    // shift in pixels (same as the translation when zero)
    algebra::Vector2D shift(0.0,0.0);
    algebra::Rotation3D R=
      em2d::internal::get_rotation_from_projection_direction(vs[i]);
    RegistrationResult reg(R,shift);
    project_map(map,img->get_data(),rows,cols,reg,1e-6);
    // Write image
    reg.set_in_image(img->get_header());
    images.set(i,img);
    if(project_and_save) {
     img->write_to_floats(names[i],srw);
    }
  }
  return images;
}


void project_map(em::DensityMap &map,
    algebra::Matrix2D_d &m2,const int rows,const int cols,
    RegistrationResult &reg,const double equality_tolerance) {
  m2.resize(rows,cols);
  // Save the origin of the matrices and the voxelsize of the map
  std::vector<int> orig2D(2);
  orig2D[0]=m2.get_start(0);
  orig2D[1]=m2.get_start(1);
  algebra::Vector3D orig3D = map.get_origin();
  float voxelsize=map.get_header()->get_spacing();
  // Set the pixel size of the map to 1 (necessary for the projection algorithm)
  map.update_voxel_size(1.0);
  // Center the map and Matrix2D
  m2.centered_start();
  algebra::Vector3D init0((-1)*(int)(map.get_header()->get_nx()/2.),
                          (-1)*(int)(map.get_header()->get_ny()/2.),
                          (-1)*(int)(map.get_header()->get_nz()/2.) );
  map.set_origin(init0);
  // Logical ints for the end of the map. The values make sense
  // because the voxel size is set to 1.0
  algebra::Vector3D end0( init0[0]-1+map.get_header()->get_nx(),
                          init0[1]-1+map.get_header()->get_ny(),
                          init0[2]-1+map.get_header()->get_nz() );

  // Get the rotation and the direction from the Euler angles
  algebra::Vector3D shift = reg.get_shift3D();
  algebra::Rotation3D Rot = reg.get_rotation();
  algebra::Rotation3D InvRot = Rot.get_inverse();
  algebra::Vector3D direction;// = RotMat.direction();
  algebra::Vector3D signs, half_signs;
  for (unsigned int i=0; i< 3; ++i) {
    algebra::Vector3D v=algebra::get_basis_vector_d<3>(i);
    algebra::Vector3D r= Rot.get_rotated(v);
    direction[i]=r[2];
    // Adjust to avoid divisions by zero and allow orthogonal rays computation
    // (If any of the direction of projection's component is zero)
    if (std::abs(direction[i]) < equality_tolerance) {
      direction[i] = equality_tolerance;
    }
    // Get the signs of the direction
    signs[i] =algebra::get_sign(direction[i]);
    half_signs[i] = 0.5 * signs[i];
  }


  algebra::Vector3D r; // A point in the coordinate system for the map
  algebra::Vector3D p; // A point in the coord. system of the projection
  // For each pixel, 4 rays of projection are computed on each direction. The
  // step is going to be 1/3 from the center of the pixel in 4 directions
  double step = 1.0 / 3.0;

  // build projection
  for (int j = m2.get_start(1);j <= m2.get_finish(1);j++) { // columns
    for (int i = m2.get_start(0);i <= m2.get_finish(0);i++) { // rows

      double ray_sum = 0.0;  // Line integral value

      for (int rays_per_pixel = 0; rays_per_pixel < 4; rays_per_pixel++) {
        // point in projection coordinate system
        p[2]=0.0; // 3rd coordinate does not matter now
        switch (rays_per_pixel) {
        case 0:
          p[0]=j - step; p[1]= i - step;
          break;
        case 1:
          p[0]=j - step; p[1]= i + step;
          break;
        case 2:
          p[0]=j + step; p[1]= i - step;
          break;
        case 3:
          p[0]=j + step; p[1]= i + step;
          break;
        }
        // Get the coordinates r in the universal system corresponding to
        // the point p
        if (algebra::get_linf_norm(shift) >equality_tolerance) {
          p -= shift;
        }
        r = InvRot.get_rotated(p);

        // Compute the minimum and maximum alpha for the line of the ray
        // intersecting the given volume. line = r + alpha * direction
        algebra::Vector3D v_alpha_min, v_alpha_max, v_alpha, v_diff;
        double alpha_min=-1/equality_tolerance;
        double alpha_max=1/equality_tolerance;
        for (unsigned int ii = 0;ii < 3;ii++) {
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
        if (std::abs(alpha_max-alpha_min) <equality_tolerance) {
          continue;
        }
        // v is going to be the first voxel in the volume intersecting the ray
        algebra::Vector3D v;
        v = r + alpha_min * direction; // vector operation

        // idx is a vector of logical ints indicating a voxel of the map
        std::vector<int> idx(3);
       // Index of the first voxel
        for (unsigned int ii=0;ii < 3;ii++) {
          if (v[ii] >= 0.) {
            idx[ii] = static_cast<int>(algebra::get_constrained(
              (double)((int)(v[ii] + 0.5)) , init0[ii] , end0[ii] ) );
          } else {
            idx[ii] = static_cast<int>(algebra::get_constrained(
              (double)((int)(v[ii] - 0.5)) , init0[ii] , end0[ii] ) );
          }
        }
        // Follow the ray
        double alpha = alpha_min;
        do {
          for (int ii=0;ii < 3;ii++) {
            v_alpha[ii] = (idx[ii] + half_signs[ii] - r[ii]) / direction[ii];
            v_diff[ii] = std::abs(alpha - v_alpha[ii]);
          }
          // Determine the dimension the ray will move in the next step.
          // (Is the mininum value in v_diff)
          double diff_alpha=std::min(std::min(v_diff[0],v_diff[1]),v_diff[2]);
          ray_sum += diff_alpha *
              map.get_value(map.get_voxel_by_location(idx[0],idx[1],idx[2]));
          // update the indexes in the required dimensions
          for (int ii=0;ii < 3;ii++) {
            if (std::abs(diff_alpha-v_diff[ii]) < equality_tolerance) {
              alpha = v_alpha[ii];
              idx[ii] += static_cast<int>(signs[ii]);
            }
          }
        } while ((alpha_max - alpha) > equality_tolerance); // end of the ray
      } // for involving the 4 rays

      // Average the value of the 4 rays
      m2(i,j) = ray_sum * 0.25;
    } // i for
  } // j for

  // Restore matrix and map origins, and voxelsize
  m2.reindex(orig2D);
  map.update_voxel_size(voxelsize);
  map.set_origin(orig3D);
}



IMPEM2D_END_NAMESPACE
