/**
 * \file IMP/em2d/PCAFitRestraint.h
 * \brief fast scoring of fit between Particles in 3D and 2D class averages
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_PCA_FIT_RESTRAINT_H
#define IMPEM2D_PCA_FIT_RESTRAINT_H

#include <IMP/em2d/em2d_config.h>
#include <IMP/em2d/internal/Image2D.h>

#include <IMP/kernel/Restraint.h>

IMPEM2D_BEGIN_NAMESPACE

//! A restraint for fast scoring of Particles vs. em2d class averages
/**
   The restraint fits the particles into the 2D images by alignment of
   principal components of each of the particles projections with
   principal components of the 2D class average. Therefore, the
   particle set should comprise all subunits that are in the
   image. The aligned projections are scored against 2D class averages
   using cross correlation score. This is a fast alignment and scoring
   that works well for negative stain EM class averages.
 */
class IMPEM2DEXPORT PCAFitRestraint : public IMP::Restraint {
public:
  //! Constructor
  /**
    \param[in] particles The particles participating in the score,
    need to have XYZ, radius and mass
    \param[in] image_file_names 2D class averages filnames in PGM text format
    \param[in] pixel_size Pixel size in Angstrom
    \param[in] resolution Estimated resolution of the images
    \param[in] projection_number Number of projections to generate and fit to
    images. The lower the number, the faster the evaluation, but also less
    accurate.
  */
  PCAFitRestraint(kernel::Particles particles,
                  const std::vector<std::string>& image_files,
                  double pixel_size, double resolution = 10.0,
                  unsigned int projection_number = 100);

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;

  unsigned int get_projection_number() const { return projection_number_; }

  void set_projection_number(unsigned int n) { projection_number_ = n; }

  // write best projections fromm last calculation, if evaluate is true,
  // best projections will be recalculated
  void write_best_projections(std::string file_name, bool evaluate=false);

  IMP::kernel::ModelObjectsTemp do_get_inputs() const { return ps_; }
  IMP_OBJECT_METHODS(PCAFitRestraint);

 private:
  // particles to fit to the images
  kernel::Particles ps_;

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
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PCA_FIT_RESTRAINT_H */
