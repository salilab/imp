/**
 * \file IMP/em2d/PCAFitRestraint.h
 * \brief fast scoring of fit between Particles in 3D and 2D class averages
 *
 * \authors Dina Schneidman
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_PCA_FIT_RESTRAINT_H
#define IMPEM2D_PCA_FIT_RESTRAINT_H

#include <IMP/em2d/em2d_config.h>
#include <IMP/em2d/internal/Image2D.h>
#include <IMP/em2d/internal/Projector.h>

#include <IMP/Restraint.h>

IMPEM2D_BEGIN_NAMESPACE

//! Fast scoring of Particles against electron microscopy class averages
/**
   The restraint fits the particles into the 2D images by alignment of
   principal components of each of the particles' projections with
   principal components of the 2D class average. Therefore, the
   particle set should comprise all subunits that are in the
   image. The aligned projections are scored against 2D class averages
   using the cross correlation score. This is a fast alignment and score
   that works well for negative stain EM class averages.
 */
class IMPEM2DEXPORT PCAFitRestraint : public IMP::Restraint {
public:
  //! Constructor
  /**
    \param[in] particles The particles participating in the score, which
               need to have XYZ, radius and mass
    \param[in] image_files 2D class average filenames in PGM text format
    \param[in] pixel_size Pixel size in angstroms
    \param[in] resolution Estimated resolution of the images in angstroms
    \param[in] projection_number Number of projections of the model
               to generate and fit to images. The lower the number, the
               faster the evaluation, but the lower the accuracy.
    \param[in] reuse_direction Speed up evaluation by only periodically
               recalculating projections
    \param[in] n_components Number of the largest components to be
               considered for the EM image
  */
  PCAFitRestraint(Particles particles,
                  const std::vector<std::string>& image_files,
                  double pixel_size, double resolution = 10.0,
                  unsigned int projection_number = 100,
                  bool reuse_direction = false,
                  unsigned int n_components = 1);

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;

  unsigned int get_projection_number() const { return projection_number_; }

  void set_projection_number(unsigned int n) { projection_number_ = n; }

  //! Get transformation that best places the model on the image
  /** The transformation places the model such that if the image is placed
      on the xy plane with its lower left corner at the origin the model's
      projection along the z axis will coincide with the image.
      This transformation is that calculated from the last scoring function
      evaluation; calling this function before the score is calculated
      results in undefined behavior.
    */
  algebra::Transformation3D get_transformation(unsigned int image_number) const;

  //! Get cross correlation between the image and the best model projection
  /** This CCC is that calculated from the last scoring function
      evaluation; calling this function before the score is calculated
      results in undefined behavior.
    */
  double get_cross_correlation_coefficient(unsigned int image_number) const;

  // write best projections from last calculation, if evaluate is true,
  // best projections will be recalculated
  void write_best_projections(std::string file_name, bool evaluate=false);

  //! \return Information for writing to RMF files
  RestraintInfo *get_dynamic_info() const IMP_OVERRIDE;

  IMP::ModelObjectsTemp do_get_inputs() const { return ps_; }
  IMP_OBJECT_METHODS(PCAFitRestraint);

 private:
  // particles to fit to the images
  Particles ps_;

  // EM2D images
  std::vector<internal::Image2D<> > images_;

  // image pixel size
  double pixel_size_;

  // estimated image resolution
  double resolution_;

  // number of particle projections to generate for image fitting
  unsigned int projection_number_;

  // from last score calculation
  std::vector<internal::Image2D<> > best_projections_;

  // from last calculation
  std::vector<internal::ProjectionInfo> best_projections_info_;
  IMP::algebra::Vector3Ds best_projections_axis_;
  std::vector<internal::ImageTransform> best_image_transform_;

  // Projector class instance
  internal::Projector projector_;

  // Speed up evaluation by only periodically recalculating projections
  bool reuse_direction_;

  // Number of the largest components to be considered for the EM image
  unsigned int n_components_;

  mutable unsigned long counter_;
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PCA_FIT_RESTRAINT_H */
