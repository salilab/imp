/**
 *  \file project.cpp
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/project.h"
#include "IMP/em2d/ImageReaderWriter.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/internal/rotation_helper.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/core/utility.h"
#include "IMP/core/XYZ.h"
#include "IMP/core/CoverRefined.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>

IMPEM2D_BEGIN_NAMESPACE


em2d::Images get_projections(const ParticlesTemp &ps,
        const algebra::SphericalVector3Ds &vs,
        int rows, int cols, const ProjectingOptions &options,
        Strings names) {
  IMP_LOG_VERBOSE(
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
  return get_projections(ps,registration_values,rows,cols,options,names);
}



em2d::Images get_projections(const ParticlesTemp &ps,
        const RegistrationResults &registration_values,
        int rows, int cols, const ProjectingOptions &options,
        Strings names) {
  IMP_LOG_VERBOSE(
          "Generating projections from registraion results" << std::endl);

  if(options.save_images && (names.size() < registration_values.size() ) ) {
    IMP_THROW("get_projections: Insufficient number of image names provided",
              IOException);
  }

  unsigned long n_projs= registration_values.size();
  em2d::Images projections(n_projs);
  // Precomputation of all the possible projection masks for the particles
  MasksManagerPtr masks(new MasksManager(options.resolution,
                                         options.pixel_size));
  masks->create_masks(ps);
  for (unsigned long i=0;i<n_projs;++i) {
    IMP_NEW(em2d::Image,img,());
    img->set_size(rows,cols);
    img->set_was_used(true);
    String name="";
    if(options.save_images) name = names[i];
    get_projection(img,ps,registration_values[i], options, masks, name);
    projections[i]=img;
  }
  return projections;
}



void get_projection(em2d::Image *img,const ParticlesTemp &ps,
        const RegistrationResult &reg, const ProjectingOptions &options,
        MasksManagerPtr masks, String name) {
  IMP_LOG_VERBOSE("Generating projection in a em2d::Image" << std::endl);

  if(masks==MasksManagerPtr()) {
    masks =MasksManagerPtr(new MasksManager(options.resolution,
                                            options.pixel_size));
    masks->create_masks(ps);
    IMP_LOG_VERBOSE(
          "Masks generated from get_projection()"  << std::endl);
  }
  algebra::Vector3D translation = options.pixel_size*reg.get_shift_3d();
  algebra::Rotation3D R = reg.get_rotation();

  do_project_particles(ps, img->get_data(), R, translation, options, masks);
  if(options.normalize) em2d::do_normalize(img,true);
  reg.set_in_image(img->get_header());
  img->get_header().set_object_pixel_size(options.pixel_size);
  if(options.save_images) {
    if(name.empty()) {
      IMP_THROW("get_projection: File name string is empty ", IOException);
    }
    if(options.srw == Pointer<ImageReaderWriter>()) {
      IMP_THROW("The options class does not have an "
                "ImageReaderWriter assigned. Create an ImageReaderWriter "
                "and assigned to the srw member of ProjectingOptions.",
                IOException);
    }
    img->write(name,options.srw);
  }
}




void do_project_particles(const ParticlesTemp &ps,
             cv::Mat &m2,
             const algebra::Rotation3D &R,
             const algebra::Vector3D &translation,
             const ProjectingOptions &options,
             MasksManagerPtr masks) {
  IMP_LOG_VERBOSE("Projecting particles" << std::endl);
  if(m2.empty()) {
    IMP_THROW("Cannot project on a empty matrix",ValueException);
  }
  if(masks==MasksManagerPtr()) {
    // Create the masks
    masks=MasksManagerPtr(new MasksManager(options.resolution,
                                           options.pixel_size));
    masks->create_masks(ps);
  }
  // Centroid
  unsigned long n_particles = ps.size();
  //core::XYZRsTemp xyzrs(ps);
  core::XYZRs xyzrs(ps);
  algebra::Vector3D centroid = core::get_centroid(xyzrs);

  // clear data before creating a new projection
  if(options.clear_matrix_before_projecting) m2.setTo(0.0);
  // Project
  double invp = 1.0/options.pixel_size;

  for (unsigned long i=0; i<n_particles; i++) {
    // Coordinates respect to the centroid
    algebra::Vector3D p = xyzrs[i].get_coordinates()-centroid;
    // Pixel after trasformation to project in Z axis
    // Not necessary to compute pz, is going to be ignored
    double pix_x = invp * (R.get_rotated_one_coordinate(p,0)+translation[0]);
    double pix_y = invp * (R.get_rotated_one_coordinate(p,1)+translation[1]);

    IMP_USAGE_CHECK( !base::isnan(pix_x)
                  || !base::isnan(pix_y),
                    "do_project_particles: " << n_particles
              << " resolution "  << options.resolution << " pixel size "
              << options.pixel_size << std::endl);

    // Apply mask
    ProjectionMaskPtr mask= masks->find_mask(xyzrs[i].get_radius());
    algebra::Vector2D pix(pix_x, pix_y);
    mask->apply(m2,pix);
  }
  IMP_LOG_VERBOSE("END of do_project_particles" << std::endl);
}



algebra::Vector2Ds do_project_vectors(const algebra::Vector3Ds &ps,
             const algebra::Rotation3D &R,
             const algebra::Vector3D &translation) {
  algebra::Vector3D centroid = algebra::get_centroid(ps);
  return do_project_vectors(ps,R,translation,centroid);
}


algebra::Vector2Ds do_project_vectors(const algebra::Vector3Ds &ps,
            const algebra::Rotation3D &R,const algebra::Vector3D &translation,
            const algebra::Vector3D &center) {
  // Project
  unsigned long n_particles = ps.size();
  algebra::Vector3D p;
  algebra::Vector2Ds vs(n_particles);
  for (unsigned long i=0; i<n_particles; i++) {
    // Coordinates respect to the center
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


Images create_evenly_distributed_projections(const ParticlesTemp &ps,
                                             unsigned int n,
                                             const ProjectingOptions &options) {
  IMP_LOG_TERSE( "creating evenly distributed projections"<< std::endl);

  // Sphere that encloses_the_particles
  IMP_NEW(Particle, p, (ps[0]->get_model(), "cover Particle") );
  core::XYZs xyzs(ps);
  unsigned int size = get_enclosing_image_size(ps, options.pixel_size, 4);
  RegistrationResults regs = get_evenly_distributed_registration_results(n);
  Images projections = get_projections(ps, regs, size, size, options);
  return projections;
}


unsigned int get_enclosing_image_size(const ParticlesTemp &ps,
                                      double pixel_size,
                                      unsigned int slack) {
  IMP_NEW(Particle, p, (ps[0]->get_model(), "cover Particle") );
  // core::XYZsTemp xyzs(ps);
  core::XYZs xyzs(ps);
  double diameter = 2 * core::get_enclosing_sphere(xyzs).get_radius();
  unsigned int size = static_cast<unsigned int>(diameter/pixel_size) + 2*slack;
  return size;
}


IMPEM2D_END_NAMESPACE
