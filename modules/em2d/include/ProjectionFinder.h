/**
 *  \file ProjectionFinder.h
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_FINDER_H
#define IMPEM2D_PROJECTION_FINDER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/Particle.h"
#include <string>

IMPEM2D_BEGIN_NAMESPACE

//! class to perform registration of images
class IMPEM2DEXPORT ProjectionFinder
{
public:

  ProjectionFinder() {
    parameters_initialized_=false;
    registration_done_=false;
  }

  //! Initializes the parameters to generate and match projections
  /**
    \param[in] resolution to employ to generate projections for matching with
              the EM images. Default is the maximum possible, 1.
    \param[in] coarse_registration_method Method for 1st step of projection
              finding, the 2D alignment:
                0 => FFT alignment no preprocessing.
                1 => FFT alignment with preprocessing (Default and recommended).
                2 => FFT alginment and centers of gravity
                        (fast, but only works for low noise)
    \param[in] interpolation_method Default is linear. Fast, enough accuracy
                during testing.
    \param[in] optimization steps
    \param[in] simplex step size
  **/
  void initialize(double apix,double resolution =1,
                 int coarse_registration_method = 1,
                 bool save_match_images =false,
                 int interpolation_method = 0,
                 int optimization_steps = 10,
                 double simplex_initial_length =0.1,
                 double simplex_minimum_size =0.01) {
    resolution_ = resolution;
    apix_ = apix;
    coarse_registration_method_ = coarse_registration_method;
    interpolation_method_ = interpolation_method;
    simplex_minimum_size_ = simplex_minimum_size;
    optimization_steps_ = optimization_steps;
    save_match_images_ = save_match_images;
    simplex_initial_length_ = simplex_initial_length;
    masks_manager_.init_kernel(resolution_,apix_);
    parameters_initialized_=true;
  }

  //! Set EM images
  void set_subjects(const em::Images subjects);

  //! Set model projections
  void set_projections(em::Images projections);

  //! Set the model particles
  void set_model_particles(const ParticlesTemp &ps);

  void set_save_match_images(bool x) {
    save_match_images_=x;
  }

  bool get_save_match_images() const {
     return save_match_images_;
  }

  //! Recover the registration results. Only works if a registration has been
  //! done previously
  RegistrationResults get_registration_results() const;

  //! Coarse registration of projections by enumeration.
  void get_coarse_registration();


  //! Performs complete registration of projections against subjects in 2D
  void get_complete_registration();

  double get_em2d_score() const;

  void show(std::ostream &out) const;

protected:

   //! Coarse registration for one subject
  algebra::Transformation2D get_coarse_registrations_for_subject(
                        unsigned int i,RegistrationResults &subject_RRs);

  void preprocess_projection(unsigned int j);
  void preprocess_subject(unsigned int i);

  //! Preprocess an matrix computing its center of gravity
  //! and the FFT of the polar-resampled autocorrelation. Calls
  //! preprocess_autocorrelation2D
  void preprocess_for_fast_coarse_registration(
          algebra::Matrix2D_d &m,algebra::Vector2D &center,
          algebra::Matrix2D_c &POLAR_AUTOC);

  //! Preprocess a matrix computing the autocorrelation,
  //! resampling to polar, and then computing the FFT.
  void fft_polar_autocorrelation2D(algebra::Matrix2D_d &m,
                                    algebra::Matrix2D_c &POLAR_AUTOC);

  // Main parameters
  em::Images subjects_;
  em::Images projections_;
  RegistrationResults registration_results_;
  ParticlesTemp model_particles_;
  bool save_match_images_,
       registration_results_set_,
       particles_set_,
       registration_done_,
       parameters_initialized_;
  //! resolution used to generate projections during the fine registration
  double resolution_;
  //! Sampling of the images in Amstrong/pixel
  double apix_;

  //! Coarse registration method
  unsigned int coarse_registration_method_;
  //! Interpolation method for projection generation
  unsigned int interpolation_method_;
  //! Simplex optimization parameters
  double simplex_minimum_size_,simplex_initial_length_;
  unsigned int optimization_steps_;

  // FFT of subjects (storing the FFT of projections is not neccessary
  std::vector< algebra::Matrix2D_c > SUBJECTS_;
  // FFT of the autocorrelation resampled images
  std::vector< algebra::Matrix2D_c > SUBJECTS_POLAR_AUTOC_;
  std::vector< algebra::Matrix2D_c > PROJECTIONS_POLAR_AUTOC_;
  algebra::Vector2Ds subjects_cog_;
  algebra::Vector2Ds projections_cog_;

  MasksManager masks_manager_;
};


IMP_VALUES(ProjectionFinder,ProjectionFinders);
IMP_OUTPUT_OPERATOR(ProjectionFinder);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_FINDER_H */
