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
                 int interpolation_method = 0,
                 int optimization_steps = 10,
                 double simplex_minimum_size =0.01) {
    resolution_ = resolution;
    apix_ = apix;
    coarse_registration_method_ = coarse_registration_method;
    interpolation_method_ = interpolation_method;
    simplex_minimum_size_ = simplex_minimum_size;
    optimization_steps_ = optimization_steps;
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
  /**
    \param[in] save_match_images saves the matches for the subjects obtained
              transforming the projections. Their names are match-0.spi,
              match-1.spi, and so on.
  **/
  double get_coarse_registration(bool save_match_images);


  //! Performs complete registration of projections against subjects in 2D
  /**
    \param[in] save_match_images If true, saves the matched projections.
    \param[in] apix. Amstrongs per pixel for the images
    \param[in] optimization_steps needed for the Simplex algorithm
    \param[in] simplex_minimum_size stop criteria for Simplex
    \param[out] output the function returns the total discrepancy score after
      refining all the registratation parameters.
  **/
  double get_complete_registration( bool save_match_images);

  void all_vs_all_projections_ccc(String &fn_out);

protected:

   //! Coarse registration for one subject
  algebra::Transformation2D get_coarse_registrations_for_subject(
                        unsigned int i,RegistrationResults &subject_RRs);
  //! Calls preprocessing routines
  void preprocess_subjects_and_projections();

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
  double simplex_minimum_size_;
  unsigned int optimization_steps_;

  // FFT related variables
  std::vector< algebra::Matrix2D_c > SUBJECTS_;
  std::vector< algebra::Matrix2D_c > PROJECTIONS_;
  std::vector< complex_rings > fft_rings_subjects_; // need to be resized
  std::vector< complex_rings > fft_rings_projections_; // need to be resized

  // PCA related variables
  algebra::Vector3Ds subjects_pcas_;
  algebra::Vector3Ds projections_pcas_;
  algebra::Vector2Ds subjects_cog_;
  algebra::Vector2Ds projections_cog_;
};


IMP_VALUES(ProjectionFinder,ProjectionFinders);


//! Computes the autocorrelation functions and their polar resampling of the
//! input set.
IMPEM2DEXPORT void preprocess_for_coarse_registration(
        em::Images input_set,
        std::vector< algebra::Matrix2D_c > &MATRICES,
        std::vector< complex_rings > &fft_rings_images,
        bool dealing_with_subjects=false,
        int interpolation_method=0 );


IMPEM2DEXPORT void preprocess_for_fast_coarse_registration(
        em::Images input_set,
        algebra::Vector2Ds centers,
        std::vector< complex_rings > &fft_rings_images,
        bool dealing_with_subjects=false,
        int interpolation_method=0);



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_FINDER_H */
