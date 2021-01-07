/**
 * \file IMP/em2d/PCAFitRestraint.cpp
 * \brief fast scoring of fit between Particles in 3D and 2D class averages
 *
 * \authors Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/PCAFitRestraint.h>

#include <IMP/em2d/internal/Projection.h>
#include <IMP/em2d/internal/ImageTransform.h>
#include <IMP/file.h>

IMPEM2D_BEGIN_NAMESPACE

PCAFitRestraint::PCAFitRestraint(Particles particles,
                                 const std::vector<std::string>& image_files,
                                 double pixel_size, double resolution,
                                 unsigned int projection_number, bool reuse_direction,
                                 unsigned int n_components)
  : Restraint(particles[0]->get_model(), "PCAFitRestraint%1%"),
    ps_(particles),
    pixel_size_(pixel_size),
    resolution_(resolution),
    projection_number_(projection_number),
    projector_(ps_, projection_number, pixel_size, resolution),
    reuse_direction_(reuse_direction),
    n_components_(n_components),
    counter_(0)
{
  // read and process the images
  for (unsigned int i = 0; i < image_files.size(); i++) {
    image_files_.push_back(get_absolute_path(image_files[i]));
    internal::Image2D<> image(image_files[i]);
    image.get_largest_connected_component(n_components_);
    image.pad((int)(image.get_width() * 1.7), (int)(image.get_height() * 1.7));
    image.center();
    image.average();
    image.stddev();
    image.compute_PCA();
    images_.push_back(image);
  }

  internal::Image2D<>::write_PGM(images_, "images.pgm");
}

double PCAFitRestraint::unprotected_evaluate(
                                  IMP::DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);

  counter_++; // increase counter

  // generate projections
  boost::ptr_vector<internal::Projection> projections;
  if(reuse_direction_ && counter_ != 500 &&
     best_projections_axis_.size() == images_.size()) {
    projector_.compute_projections(best_projections_axis_,
                                   IMP_DEG_2_RAD(20),  // 20 degrees for now
                                   projections,
                                   images_[0].get_height());
  } else {
    projector_.compute_projections(projections, images_[0].get_height());
    counter_ = 0;
  }
  IMP_LOG_VERBOSE(projections.size() << " projections were created"
                  << std::endl);

  // process projections
  for (unsigned int i = 0; i < projections.size(); i++) {
    projections[i].get_largest_connected_component(n_components_);
    projections[i].center();
    projections[i].average();
    projections[i].stddev();
    projections[i].compute_PCA();
  }

  // score each image against projections
  double total_score = 0.0;
  double area_threshold = 0.4;
  PCAFitRestraint* non_const_this = const_cast<PCAFitRestraint *>(this);
  non_const_this->best_projections_.clear();
  non_const_this->best_projections_info_.clear();
  non_const_this->best_projections_axis_.clear();
  non_const_this->best_image_transform_.clear();
  for (unsigned int i = 0; i < images_.size(); i++) {
    internal::ImageTransform best_transform;
    best_transform.set_score(0.000000001);
    int best_projection_id = 0;
    for (unsigned int j = 0; j < projections.size(); j++) {
      // do not align images with more than X% area difference
      double area_score = std::abs(images_[i].segmented_pixels() -
                                   projections[j].segmented_pixels()) /
        (double)std::max(images_[i].segmented_pixels(),
                         projections[j].segmented_pixels());
      if (area_score > area_threshold) continue;

      internal::ImageTransform curr_transform =
        images_[i].pca_align(projections[j]);
      curr_transform.set_area_score(area_score);
      if (curr_transform.get_score() > best_transform.get_score()) {
        best_transform = curr_transform;
        best_projection_id = projections[j].get_id();
      }
    }
    IMP_LOG_VERBOSE("Image " << i << " Best projection " << best_projection_id
                    << " " << best_transform << std::endl);
    total_score += -log(best_transform.get_score());

    // save best projections, info and their projection axis
    internal::ProjectionInfo pinfo;
    pinfo.rotation = projections[best_projection_id].get_rotation();
    algebra::Vector2D c = projections[best_projection_id].get_centroid();
    // centroid of original (pre-center()) projection
    pinfo.centroid = projections[best_projection_id].get_point_for_index(
                         c[0] - projections[best_projection_id].get_center_x(),
                         c[1] - projections[best_projection_id].get_center_y());

    internal::Image2D<> transformed_image;
    projections[best_projection_id]
        .rotate_circular(transformed_image, best_transform.get_angle());
    transformed_image.translate(best_transform.get_x(), best_transform.get_y());
    non_const_this->best_projections_.push_back(transformed_image);
    non_const_this->best_projections_info_.push_back(pinfo);
    non_const_this->best_projections_axis_.push_back(projections[best_projection_id].get_axis());
    non_const_this->best_image_transform_.push_back(best_transform);
  }
  return total_score;
}

double PCAFitRestraint::get_cross_correlation_coefficient(
                                    unsigned int image_number) const {
  IMP_USAGE_CHECK(image_number < best_projections_.size(),
                  "image number is out of bounds");
  return best_image_transform_[image_number].get_score();
}

algebra::Transformation3D PCAFitRestraint::get_transformation(
                                    unsigned int image_number) const {
  IMP_USAGE_CHECK(image_number < best_projections_.size(),
                  "image number is out of bounds");

  // The rotation is the projection 3D rotation followed a 2D rotation
  // (in the xy plane) about the centroid to align the projection with the image
  internal::ProjectionInfo pinfo = best_projections_info_[image_number];
  internal::ImageTransform best_transform = best_image_transform_[image_number];

  algebra::Vector3D axis(0., 0., 1.);
  algebra::Rotation3D rot_orig = algebra::get_rotation_about_normalized_axis(
                                           axis, best_transform.get_angle());
  algebra::Transformation3D rot = algebra::get_rotation_about_point(
                                           pinfo.centroid, rot_orig);

  // Finally, translate centroid to that of the image
  algebra::Vector2D i_centroid = images_[image_number].get_centroid();
  // image was padded and centered, but we want to align with the original,
  // so reverse the padding and centering translation
  int center_x = images_[image_number].get_center_x();
  int center_y = images_[image_number].get_center_y();
  algebra::Vector3D i_center((i_centroid[0] - center_x) * pixel_size_,
                             (i_centroid[1] - center_y) * pixel_size_, 0.);

  return algebra::Transformation3D(i_center - pinfo.centroid)
         * rot * algebra::Transformation3D(pinfo.rotation);
}

RestraintInfo *PCAFitRestraint::get_static_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_string("type", "IMP.em2d.PCAFitRestraint");
  ri->add_filenames("image files", image_files_);
  ri->add_float("pixel size", pixel_size_);
  ri->add_float("resolution", resolution_);
  ri->add_int("projection number", projection_number_);
  return ri.release();
}

RestraintInfo *PCAFitRestraint::get_dynamic_info() const {
  IMP_NEW(RestraintInfo, ri, ());

  Floats ccs, rotations, translations;
  unsigned i;
  for (i = 0; i < best_projections_.size(); ++i) {
    ccs.push_back(get_cross_correlation_coefficient(i));
    algebra::Transformation3D t = get_transformation(i);
    algebra::Vector4D rotation = t.get_rotation().get_quaternion();
    algebra::Vector3D translation = t.get_translation();
    unsigned j;
    for (j = 0; j < 4; ++j) {
      rotations.push_back(rotation[j]);
    }
    for (j = 0; j < 3; ++j) {
      translations.push_back(translation[j]);
    }
  }
  ri->add_floats("cross correlation", ccs);
  ri->add_floats("rotation", rotations);
  ri->add_floats("translation", translations);

  return ri.release();
}

void PCAFitRestraint::write_best_projections(std::string file_name,
                                             bool evaluate) {
  if(best_projections_.size() == 0 || evaluate) {
    unprotected_evaluate(nullptr);
  }
  internal::Image2D<>::write_PGM(best_projections_, file_name);
}

IMPEM2D_END_NAMESPACE
