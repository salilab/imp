/**
 *  \file ProjectionFinder.cpp
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/pca_features_extraction.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/Fine2DRegistrationRestraint.h"
#include "IMP/em/image_transformations.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/gsl/Simplex.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"
#include "IMP/exception.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE



void ProjectionFinder::preprocess_subjects_and_projections() {

  IMP_LOG(IMP::TERSE,
       "ProjectionFinder: preprocessing subjects and projections" << std::endl);
  IMP_USAGE_CHECK(subjects_set_,
        "preprocess_subjects_and_projections: "
        "subject images have not been set");
  IMP_USAGE_CHECK(projections_set_,
        "preprocess_subjects_and_projections: "
        "projection images have not been set");

  SUBJECTS_POLAR_AUTOC_.resize(n_subjects_);
  SUBJECTS_.resize(n_subjects_);
  PROJECTIONS_POLAR_AUTOC_.resize(n_projections_);
  boost::progress_display show_progress(n_subjects_+n_projections_);
  // FFT PREPROCESSING
  if(coarse_registration_method_ ==1 ||coarse_registration_method_==2) {
    for (unsigned int i=0;i<n_subjects_;++i) {
      // FFT
      FFT2D fft(subjects_[i]->get_data(),SUBJECTS_[i]); fft.execute();
      // autocorrelation, polar, FFT
      fft_polar_autocorrelation2D(subjects_[i]->get_data(),
                                    SUBJECTS_POLAR_AUTOC_[i]);
      IMP_LOG(IMP::TERSE,"Subject " << i << " preprocessed." << std::endl);
      ++show_progress;
    }

    for (unsigned int j=0;j<n_projections_;++j) {
      fft_polar_autocorrelation2D(projections_[j]->get_data(),
                                    PROJECTIONS_POLAR_AUTOC_[j]);
      IMP_LOG(IMP::TERSE,"Projection " << j << " preprocessed." << std::endl);
      ++show_progress;
    }
  }

  // CENTERS OF GRAVITY AND ROTATIONAL FFT PREPROCESSING
  if(coarse_registration_method_==3) {
    subjects_cog_.resize(n_subjects_);
    projections_cog_.resize(n_projections_);
    for (unsigned int i=0;i<n_subjects_;++i) {
      preprocess_for_fast_coarse_registration(
        subjects_[i]->get_data(),subjects_cog_[i],SUBJECTS_POLAR_AUTOC_[i]);
      ++show_progress;
    }
    for (unsigned int j=0;j<n_projections_;++j) {
      preprocess_for_fast_coarse_registration(projections_[j]->get_data(),
                               projections_cog_[j],SUBJECTS_POLAR_AUTOC_[j]);
      ++show_progress;
    }
  }
  // PCA PREPROCESSING
  if(coarse_registration_method_ ==2 || coarse_registration_method_==3) {
    double pca_threshold = 0.0;
    IMP_LOG(IMP::TERSE,"PCA Preprocessing .... " << std::endl);
    subjects_pcas_=pca_features(subjects_,pca_threshold);
    projections_pcas_=pca_features(projections_,pca_threshold);
  }
}


double ProjectionFinder::get_coarse_registration() {
  IMP_LOG(IMP::TERSE,"Coarse registration of subjects .... " << std::endl);
  IMP_USAGE_CHECK(subjects_set_,
        "get_coarse_registration: subject images have not been set");
  IMP_USAGE_CHECK(projections_set_,
        "get_coarse_registration: projection images have not been set");

  em::SpiderImageReaderWriter<double> srw;
  algebra::Transformation2D t,best_transformation;
  double Score=0;
  /***** Preprocessing ********/
  preprocess_subjects_and_projections();
  /***** Computation ********/
  boost::progress_display show_progress(n_subjects_);
  for(unsigned long i=0;i<n_subjects_;++i) {
    RegistrationResults subject_RRs(n_projections_);
    algebra::Transformation2D best_transformation=
         get_coarse_registrations_for_subject(i,subject_RRs);
    // Sort projections scores
    std::sort(subject_RRs.begin(),subject_RRs.end(),
              compare_registration_results);
    // Best result
    registration_results_[i]=subject_RRs[0];
    registration_results_[i].set_in_image(*subjects_[i]);

    IMP_LOG(IMP::VERBOSE,"Coarse registration results for subject : "
                                                            << std::endl);
    for (unsigned int k=0;k<subject_RRs.size();++k) {
      IMP_LOG(IMP::VERBOSE,"Projection " << k << " Registration result: "
            << subject_RRs[k].get_index() << std::endl);
    }

    /**** Compute score and save image if requested *****/
    // Apply the transformation here and not in the discrepancy score.
    // Saves time if the image is saved, the transformation is not done twice.
    IMP_NEW(em::Image,match,());
    int index=registration_results_[i].get_index();
    em::apply_Transformation2D(projections_[index]->get_data(),
                        best_transformation,match->get_data(),true);
    em::normalize(*match,true);
    // Compute score
    bool apply_transformations=false;
    Score += discrepancy_score(*subjects_[i],*match,apply_transformations);
    if(save_match_images_) {
      registration_results_[i].set_in_image(*match);
      std::ostringstream strm;
      strm << "coarse_match-" << i << ".spi";
      match->write_to_floats(strm.str(),srw);
    }
    ++show_progress;
  }
  registration_done_=true;
  return Score/n_subjects_;
}


algebra::Transformation2D
            ProjectionFinder::get_coarse_registrations_for_subject(
             unsigned int i,RegistrationResults &subject_RRs) {

  algebra::Transformation2D t,best_transformation;
  double max_ccc=0,ccc=0;


  subject_RRs.resize(n_projections_);
  IMP_LOG(IMP::VERBOSE," Size vector projections = " << projections_.size()
          << std::endl);

  for(unsigned long j=0;j<n_projections_;++j) {
    IMP_LOG(IMP::VERBOSE,"Registering subject " << i << " with projection "
            << j << std::endl);

    // If method includes PCA check, do it
    if((coarse_registration_method_==2 || coarse_registration_method_==3) &&
      !pca_features_match(subjects_pcas_[i],projections_pcas_[j],0.1)) {
      continue;
    }
    ResultAlign2D RA;
    // Method without preprocessing
    if(coarse_registration_method_==0) {
      RA=align2D_complete(subjects_[i]->get_data(),
                          projections_[j]->get_data(),
                          false,
                          interpolation_method_);
    }
    // Methods with preprocessing and FFT alignment
    if(coarse_registration_method_==1 || coarse_registration_method_==2) {
      RA=align2D_complete_no_preprocessing(
                      subjects_[i]->get_data(),
                      SUBJECTS_[i],
                      SUBJECTS_POLAR_AUTOC_[i],
                      projections_[j]->get_data(),
                      PROJECTIONS_POLAR_AUTOC_[j]);
    }
    // Method with centers of gravity alignment
    if(coarse_registration_method_==3) {
      PolarResamplingParameters polar_params(subjects_[i]->get_data());
      unsigned int n_rings =  polar_params.get_number_of_rings();
      unsigned int sampling_points = polar_params.get_sampling_points(n_rings);
      RA=align2D_complete_with_centers_no_preprocessing(
                      subjects_cog_[i],
                      projections_cog_[j],
                      SUBJECTS_POLAR_AUTOC_[i],
                      PROJECTIONS_POLAR_AUTOC_[j],
                      n_rings,sampling_points);
      // align2D_complete_with_centers_no_preprocessing returns a value of cc
      // from the rotational alignment, but not the ccc. compute the ccc here:
      algebra::Matrix2D_d aux;
      em::apply_Transformation2D(projections_[j]->get_data(),
                                 RA.first,aux,true,0.0,interpolation_method_);
      ccc=subjects_[i]->get_data().cross_correlation_coefficient(aux);
      RA.second = ccc;
    }

    if(RA.second>max_ccc) {
        max_ccc=RA.second;
        best_transformation=RA.first;
      }

    // Get the rotation values from the projection
    IMP_LOG(IMP::VERBOSE," Setting registration subject " << i << " projection "
            << j << std::endl);

    RegistrationResult rr(projections_[j]->get_header().get_Phi(),
                          projections_[j]->get_header().get_Theta(),
                          projections_[j]->get_header().get_Psi(),
                          0.0,0.0,j,RA.second);
    // add the 2D alignment transformation found to those values
    rr.add_in_plane_transformation(RA.first);
    IMP_LOG(IMP::VERBOSE," rr = " << rr << std::endl);
    subject_RRs[j]=rr;
    IMP_LOG(IMP::VERBOSE," Registration subject " << i << " projection "
            << j << " " << subject_RRs[j] << std::endl);
  }
  return best_transformation;
}




double ProjectionFinder::get_complete_registration() {
  IMP_LOG(IMP::TERSE,"Complete registration of subjects .... " << std::endl);
  IMP_USAGE_CHECK(subjects_set_,
        "get_complete_registration: subject images have not been set");
  IMP_USAGE_CHECK(projections_set_,
        "get_complete_registration: projection images have not been set");
  IMP_USAGE_CHECK(particles_set_,
        "get_complete_registration: model particles have not been set");
  IMP_USAGE_CHECK(parameters_initialized_,
        "get_complete_registration: the ProjectionFinder is not initialized");

  /********* Variables **********/
  em::SpiderImageReaderWriter<double> srw;
  algebra::Transformation2D t;
  bool masks_computed=false;
  unsigned int rows= subjects_[0]->get_data().get_number_of_rows();
  unsigned int cols= subjects_[0]->get_data().get_number_of_columns();
  IMP_NEW(em::Image,match,());
  match->resize(rows,cols);
  MasksManager masks;

  /***** Set optimizer ********/
  IMP_NEW(Model,scoring_model,());
  IMP_NEW(Fine2DRegistrationRestraint,fine2d,());
  IMP_NEW(IMP::gsl::Simplex,simplex_optimizer,());
  fine2d->initialize(model_particles_,resolution_,apix_,scoring_model);
  simplex_optimizer->set_model(scoring_model);
  simplex_optimizer->set_initial_length(simplex_initial_length_);
  simplex_optimizer->set_minimum_size(simplex_minimum_size_);
  IMP::SetLogState log_state(fine2d,IMP::TERSE);
  /***** Computation ********/
  preprocess_subjects_and_projections();
  boost::progress_display show_progress(n_subjects_*n_projections_);
  double Score=0;
  for(unsigned long i=0;i<n_subjects_;++i) {
    RegistrationResults subject_RRs(n_projections_);
    get_coarse_registrations_for_subject(i,subject_RRs);

    IMP_LOG(IMP::VERBOSE,
          "coarse registration of projections for subject " << i << std::endl);
    for (unsigned int k=0;k<subject_RRs.size();++k) {
     IMP_LOG(IMP::VERBOSE,
          "coarse registration " << k << ": " << subject_RRs[k] << std::endl);
    }

    for (unsigned int k=0;k<subject_RRs.size();++k) {
      // Fine registration of the subject using simplex
      subject_RRs[k].set_in_image(*subjects_[i]);
      fine2d->set_subject_image(*subjects_[i]);
      simplex_optimizer->optimize((double)optimization_steps_);
      // Update the registration parameters
      double score = fine2d->get_final_values(subject_RRs[k]);
      subject_RRs[k].set_ccc(em2d_score_to_ccc(score));
      IMP_LOG(IMP::VERBOSE, "fine registration for subject " << k
          << ": " << subject_RRs[k] << std::endl);
      ++show_progress;
    }
    std::sort(subject_RRs.begin(),subject_RRs.end(),
                                            compare_registration_results);
    // Best fine registration
    registration_results_[i]=subject_RRs[0];
    Score += ccc_to_em2d_score(registration_results_[i].get_ccc());
    // save if requested
    if(save_match_images_) {
      if(!masks_computed) {
        masks.init_kernel(resolution_,apix_);
        masks.generate_masks(model_particles_);
        masks_computed=true;
      }
      std::ostringstream strm;
      strm << "fine_match-" << i << ".spi";
      generate_projection(*match,model_particles_,registration_results_[i],
                    resolution_,apix_,srw,&masks,false,strm.str());
      em::normalize(*match,true);
      registration_results_[i].set_in_image(*match);
      match->write_to_floats(strm.str(),srw);
    }
  }
  registration_done_=true;
  return Score/n_subjects_;
}




void ProjectionFinder::preprocess_for_fast_coarse_registration(
        algebra::Matrix2D_d &m,algebra::Vector2D &center,
        algebra::Matrix2D_c &POLAR_AUTOC) {
  // Make the matrix positive to compute the center of gravity
  double min_value=m.compute_min();
  algebra::Matrix2D_d result(m);
  result -= min_value; // Now the matrix "result" is positive
  // To compute the center of gravity the matrix has to be centered
  result.centered_start();
  center=compute_center_of_gravity(result);
  // Center the image in the center of gravity
  algebra::Vector2D minus_cent = (-1)*center;
  em::shift(m,minus_cent,result,true);
  fft_polar_autocorrelation2D(result,POLAR_AUTOC);
}

void ProjectionFinder::fft_polar_autocorrelation2D(algebra::Matrix2D_d &m,
                                      algebra::Matrix2D_c &POLAR_AUTOC) {
  algebra::Matrix2D_d padded,autoc,polar_autoc;
  m.pad(padded,0);
  autocorrelation2D(padded,autoc);
  resample2D_polar(autoc,polar_autoc,interpolation_method_);
  FFT2D fft(polar_autoc,POLAR_AUTOC); fft.execute();
}



void ProjectionFinder::add_images(const em::Images &em_images) {
}

void ProjectionFinder::remove_images(const Ints &indices){
}

void ProjectionFinder::set_not_used_images(const Ints &indices){}



IMPEM2D_END_NAMESPACE
