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
#include "IMP/em/Image.h"
#include "IMP/em/ImageReaderWriter.h"
#include "IMP/em/SpiderReaderWriter.h"
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
    subjects_set_ = false;
    projections_set_ = false;
    particles_set_=false;
    parameters_initialized_=false;
    registration_done_=false;
  }

  //! Inits the class with the particles of the model to score, the subject
  //! EM images, and the projections of the model to use for the registration
  /**
    \param[in] particles particles to score
    \param[in] subjects The EM images to match with optimal projections
    \param[in] projections Projections employed to start the search for the
                optimal ones
  **/
  ProjectionFinder(const ParticlesTemp &particles,
                  const em::Images subjects,em::Images projections) {
    set_model_particles(particles);
    set_projections(projections);
    set_subjects(subjects);
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
                2 => FFT and PCA alignment (fast, but only works for low noise)
                3 => PCA alginment and centers of gravity (very fast, but
                     very low noise tolerated)
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
    parameters_initialized_=true;
  }

  //! Set EM images
   void set_subjects(const em::Images subjects) {
    subjects_=subjects;
    n_subjects_=subjects_.size();
    registration_results_.resize(n_subjects_);
    subjects_set_ = true;
  }

  //! Set model projections
  void set_projections(em::Images projections) {
    projections_=projections;
    n_projections_=projections_.size();
    projections_set_ = true;
  }

  //! Set the model particles
  void set_model_particles(const ParticlesTemp &ps) {
    model_particles_= ps;
    particles_set_=true;
  }

  void set_save_match_images(bool x) {
    save_match_images_=x;
  }

  bool get_save_match_images() {
     return save_match_images_;
  }

  //! Recover the registration results. Only works if a registration has been
  //! done previously
  RegistrationResults get_registration_results() {
    if(!registration_done_) {
      IMP_THROW(
       "ProjectionFinder: trying to recover results before registration",
        ValueException);
    }
    RegistrationResults Regs(n_subjects_);
    for (unsigned int i=0;i<n_subjects_;++i) {
      Regs[i]=registration_results_[i];
    }
    return Regs;
  }

  //! Coarse registration of projections by enumeration.
  double get_coarse_registration();


  //! Performs complete registration of projections against subjects in 2D
  double get_complete_registration();


  //! Add images to those already stored in the restraint
  /**
    \param[in] em_images images to be added
  **/
  void add_images(const em::Images &em_images);

  //! Remove images from those used by the restraint
  /**
    \param[in] indices indices of the images to be removed
  **/
  void remove_images(const Ints &indices);

  //! Sets images to not be used by the finder, but does NOT delete them
  /**
    \param[in] indices indices of the images to be removed
  **/
  void set_not_used_images(const Ints &indices);


protected:

   //! Coarse registration for one subject
  algebra::Transformation2D get_coarse_registrations_for_subject(
                        unsigned int i,RegistrationResults &subject_RRs);

  void preprocess_subjects_and_projections();

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
  unsigned int n_subjects_,n_projections_;
  bool save_match_images_,
       subjects_set_,
       projections_set_,
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
  // PCA related variables
  algebra::Vector3Ds subjects_pcas_;
  algebra::Vector3Ds projections_pcas_;
  algebra::Vector2Ds subjects_cog_;
  algebra::Vector2Ds projections_cog_;
};


IMP_VALUES(ProjectionFinder,ProjectionFinders);



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_FINDER_H */
