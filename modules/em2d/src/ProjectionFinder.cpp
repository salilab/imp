/**
 *  \file ProjectionFinder.cpp
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/Fine2DRegistrationRestraint.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em/image_transformations.h"
#include "IMP/em/ImageReaderWriter.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/atom/Mass.h"
#include "IMP/gsl/Simplex.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"
#include "IMP/exception.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE



 void ProjectionFinder::set_subjects(const em2d::Images &subjects) {
  if(subjects.size()==0) {
    IMP_THROW("Passing empty set of subjects",ValueException);
  }
  if(polar_params_.get_is_initialized() == false) {
    polar_params_.initialize(subjects[0]->get_data().rows,
                             subjects[0]->get_data().cols);
    polar_params_.set_estimated_number_of_angles(
                    subjects[0]->get_header().get_number_of_columns());
    polar_params_.build_maps_for_resampling();
  }
  boost::timer preprocessing_timer;
  subjects_.resize(subjects.size());
  unsigned int n_subjects = subjects_.size();
  registration_results_.resize(n_subjects);
  SUBJECTS_.resize(n_subjects);
  SUBJECTS_POLAR_AUTOC_.resize(n_subjects);
  subjects_cog_.resize(n_subjects);
  for (unsigned int i=0;i<n_subjects;++i) {
    subjects_[i]=subjects[i]; // doest not copy
    preprocess_subject(i);
  }
  preprocessing_time_ = preprocessing_timer.elapsed();
}

void ProjectionFinder::set_projections(const em2d::Images &projections) {
  if(projections.size()==0) {
    IMP_THROW("Passing empty set of projections",ValueException);
  }

  if(polar_params_.get_is_initialized() == false) {
    polar_params_.initialize(projections[0]->get_data().rows,
                             projections[0]->get_data().cols);
    polar_params_.set_estimated_number_of_angles(
                    projections[0]->get_header().get_number_of_columns());
    polar_params_.build_maps_for_resampling();
  }

  projections_.resize(projections.size());
  unsigned int n_projections = projections_.size();
  PROJECTIONS_POLAR_AUTOC_.resize(n_projections);
  projections_cog_.resize(n_projections);
  boost::timer preprocessing_timer;
  for (unsigned int i=0;i<n_projections;++i) {
    projections_[i]=projections[i]; // does not copy
    preprocess_projection(i);
  }
  preprocessing_time_ = preprocessing_timer.elapsed();
}


void ProjectionFinder::set_model_particles(const ParticlesTemp &ps) {
  if(parameters_initialized_==false) {
    IMP_THROW("The ProjectionFinder is not initialized",ValueException);
  }
  model_particles_= ps;
  // Check the particles for coordinates, radius and mass
  for (unsigned int i=0;i<model_particles_.size();++i) {
    IMP_USAGE_CHECK((core::XYZR::particle_is_instance(model_particles_[i]) &&
              atom::Mass::particle_is_instance(model_particles_[i])),
       "Particle " << i
       << " does not have the required attributes" << std::endl);
  }
  masks_manager_.generate_masks(model_particles_);
  particles_set_=true;
}


void ProjectionFinder::set_fast_mode(unsigned int n) {
  if(n>projections_.size() || n==0) {
    IMP_THROW("ProjectionFinder fast mode: requested zero projections or "
              "more than available",ValueException);
  }
  number_of_optimized_projections_ = n;
  fast_optimization_mode_ = true;
}


void ProjectionFinder::preprocess_projection(unsigned int j) {
  // FFT PREPROCESSING
  if(coarse_registration_method_ == ALIGN2D_PREPROCESSING) {
    cv::Mat autoc,polar_autoc;
    em2d::autocorrelation2D(projections_[j]->get_data(),autoc);
    em2d::resample_polar(autoc,polar_autoc,polar_params_);
    get_fft_using_optimal_size(polar_autoc,PROJECTIONS_POLAR_AUTOC_[j]);
  }
  // CENTERS OF GRAVITY AND ROTATIONAL FFT PREPROCESSING
  if(coarse_registration_method_== ALIGN2D_WITH_CENTERS) {
    preprocess_for_fast_coarse_registration(projections_[j]->get_data(),
                                           projections_cog_[j],
                                           SUBJECTS_POLAR_AUTOC_[j]);
  }
}

void ProjectionFinder::preprocess_subject(unsigned int i) {
  if(coarse_registration_method_ == ALIGN2D_PREPROCESSING) {
    cv::Mat autoc,polar_autoc;
    get_fft_using_optimal_size(subjects_[i]->get_data(),SUBJECTS_[i]);
    autocorrelation2D(subjects_[i]->get_data(),autoc);
    resample_polar(autoc,polar_autoc,polar_params_);
    get_fft_using_optimal_size(polar_autoc,SUBJECTS_POLAR_AUTOC_[i]);
  }
  if(coarse_registration_method_ == ALIGN2D_WITH_CENTERS) {
    preprocess_for_fast_coarse_registration(subjects_[i]->get_data(),
                                            subjects_cog_[i],
                                            SUBJECTS_POLAR_AUTOC_[i]);
  }
}



void ProjectionFinder::get_coarse_registrations_for_subject(
             unsigned int i,RegistrationResults &coarse_RRs) {
  algebra::Transformation2D best_2d_transformation;
  double max_ccc=0.0;
  unsigned int projection_index = 0;

  coarse_RRs.resize(projections_.size());
  for(unsigned long j=0;j<projections_.size();++j) {
    ResultAlign2D RA;
    // Method without preprocessing
    if(coarse_registration_method_== ALIGN2D_NO_PREPROCESSING) {
      RA=align2D_complete(subjects_[i]->get_data(),
                          projections_[j]->get_data(),false);
    }
    // Methods with preprocessing and FFT alignment
    if(coarse_registration_method_== ALIGN2D_PREPROCESSING) {
      RA=align2D_complete_no_preprocessing(subjects_[i]->get_data(),
                                           SUBJECTS_[i],
                                           SUBJECTS_POLAR_AUTOC_[i],
                                           projections_[j]->get_data(),
                                           PROJECTIONS_POLAR_AUTOC_[j]);
    }

    // Method with centers of gravity alignment
    if(coarse_registration_method_== ALIGN2D_WITH_CENTERS) {
      RA=align2D_complete_with_centers_no_preprocessing(
                                                subjects_cog_[i],
                                                projections_cog_[j],
                                                SUBJECTS_POLAR_AUTOC_[i],
                                                PROJECTIONS_POLAR_AUTOC_[j]);
      // align2D_complete_with_centers_no_preprocessing returns a value of
      // Cross correlation from the rotational alignment but not the ccc.
      // compute the ccc here:
      cv::Mat aux;
      get_transformed(projections_[j]->get_data(),aux,RA.first);
      RA.second=cross_correlation_coefficient(subjects_[i]->get_data(),aux);
    }

    // Set result
    algebra::Vector2D shift(0.,0.);
    // Get values from the image
     algebra::Rotation3D R=algebra::get_rotation_from_fixed_zyz(
                    projections_[j]->get_header().get_Phi(),
                    projections_[j]->get_header().get_Theta(),
                    projections_[j]->get_header().get_Psi());
    RegistrationResult projection_result(R,shift);
    projection_result.set_projection_index(j);
    projection_result.set_image_index(i);
    projection_result.set_ccc(RA.second);
    // add the 2D alignment transformation to the registration result
    // for the projection
    projection_result.add_in_plane_transformation(RA.first);
    // and store
    coarse_RRs[j]=projection_result;
    IMP_LOG(IMP::VERBOSE,
            "Coarse registration: " << coarse_RRs[j] << std::endl)
    if(RA.second>max_ccc) {
      max_ccc = RA.second;
      best_2d_transformation =  RA.first;
      projection_index = j;
    }
  }

  if(save_match_images_) {
    IMP_NEW(em2d::Image,match,());
    get_transformed(projections_[projection_index]->get_data(),
                    match->get_data(),
                    best_2d_transformation);
    normalize(match,true);
    coarse_RRs[projection_index].set_in_image(match->get_header());
    std::ostringstream strm;
    strm << "coarse_match-" << i << ".spi";
    em2d::SpiderImageReaderWriter<double> srw;
    match->write_to_floats(strm.str(),srw);
  }

}



void ProjectionFinder::get_coarse_registration() {
  IMP_LOG(IMP::TERSE,"Coarse registration of subjects. " << std::endl);
  if(subjects_.size()==0) {
    IMP_THROW("get_coarse_registration:There are not subject images",
              ValueException);
  }
  if(projections_.size()==0) {
    IMP_THROW("get_coarse_registration:There are not projection images",
              ValueException);
  }
  /***** Computation ********/
  coarse_registration_time_ = 0;
//  boost::progress_display show_progress(subjects_.size());
  for(unsigned long i=0;i<subjects_.size();++i) {
    RegistrationResults coarse_RRs(projections_.size());
    boost::timer timer_coarse_subject;
    get_coarse_registrations_for_subject(i,coarse_RRs);
    coarse_registration_time_ += timer_coarse_subject.elapsed();

    std::sort(coarse_RRs.begin(),coarse_RRs.end(),has_higher_ccc);
    // Best result
    registration_results_[i]=coarse_RRs[0];
    registration_results_[i].set_in_image(subjects_[i]->get_header());
    IMP_LOG(IMP::TERSE,"Best coarse registration: "
                      << registration_results_[i] << std::endl);
//    ++show_progress;
  }
  registration_done_=true;
}



void ProjectionFinder::get_complete_registration() {
  IMP_LOG(IMP::TERSE,"Complete registration of subjects" << std::endl);
  if(subjects_.size()==0) {
    IMP_THROW("get_complete_registration:There are not subject images",
              ValueException);
  }
  if(projections_.size()==0) {
    IMP_THROW("get_complete_registration:There are not projection images",
              ValueException);
  }
  if(particles_set_==false) {
    IMP_THROW("get_complete_registration: "
              "Model particles have not been set",ValueException);
  }

  em2d::SpiderImageReaderWriter<double> srw;
  unsigned int rows= subjects_[0]->get_header().get_number_of_rows();
  unsigned int cols= subjects_[0]->get_header().get_number_of_columns();
  IMP_NEW(em2d::Image,match,());
  match->resize(rows,cols);

  // Set optimizer
  IMP_NEW(Model,scoring_model,());
  IMP_NEW(Fine2DRegistrationRestraint,fine2d,());
  IMP_NEW(IMP::gsl::Simplex,simplex_optimizer,());
  fine2d->initialize(model_particles_,
                     resolution_,
                     apix_,
                     scoring_model,
                     &masks_manager_);
  simplex_optimizer->set_model(scoring_model);
  simplex_optimizer->set_initial_length(simplex_initial_length_);
  simplex_optimizer->set_minimum_size(simplex_minimum_size_);
  IMP::SetLogState log_state(fine2d,IMP::TERSE);


  // Computation
//   boost::progress_display show_progress(
//                    subjects_.size()*projections_.size());
  coarse_registration_time_ = 0;
  fine_registration_time_ = 0;
  for(unsigned long i=0;i<subjects_.size();++i) {
    RegistrationResults coarse_RRs(projections_.size());

    boost::timer timer_coarse_subject;
    get_coarse_registrations_for_subject(i,coarse_RRs);
    coarse_registration_time_ += timer_coarse_subject.elapsed();

    std::sort(coarse_RRs.begin(),coarse_RRs.end(),has_higher_ccc);

    unsigned int n_optimized=projections_.size();
    if(fast_optimization_mode_) {
      n_optimized = number_of_optimized_projections_;
    }

    RegistrationResult best_fine_registration;
    best_fine_registration.set_ccc(0.0);

    boost::timer timer_fine_subject;
    for (unsigned int k=0;k<n_optimized;++k) {
      // Fine registration of the subject using simplex
      coarse_RRs[k].set_in_image(subjects_[i]->get_header());
      fine2d->set_subject_image(subjects_[i]);
      simplex_optimizer->optimize((double)optimization_steps_);
      // Update the registration parameters
      RegistrationResult fine_registration = fine2d->get_final_registration();
      if(has_higher_ccc(fine_registration,best_fine_registration)) {
        best_fine_registration=fine_registration;
      }
    }
    fine_registration_time_ += timer_fine_subject.elapsed();
    registration_results_[i]=best_fine_registration;
    IMP_LOG(IMP::TERSE,"Fine registration: "
                              << registration_results_[i] << std::endl);
    // save if requested
    if(save_match_images_) {
      generate_projection(match,model_particles_,registration_results_[i],
                    resolution_,apix_,srw,false,&masks_manager_);
      normalize(match,true);
      std::ostringstream strm;
      strm << "fine_match-" << i << ".spi";
      registration_results_[i].set_in_image(match->get_header());
      match->write_to_floats(strm.str(),srw);
    }
    // ++show_progress;
  }
  registration_done_=true;
}


RegistrationResults ProjectionFinder::get_registration_results() const {
  if(registration_done_ == false) {
    IMP_THROW("ProjectionFinder: trying to recover results "
     "before registration",ValueException);
  }
  RegistrationResults Regs(subjects_.size());
  for (unsigned int i=0;i<subjects_.size();++i) {
    Regs[i]=registration_results_[i];
  }
  return Regs;
}

double ProjectionFinder::get_em2d_score() const {
  if(!registration_done_) {
    IMP_THROW("get_em2d_score: registration not done ",ValueException);
  }
  return em2d::get_em2d_score(registration_results_);
}

  //! Time employed for preprocessing
double ProjectionFinder::get_preprocessing_time() const {
  if(subjects_.size()==0 && projections_.size()==0) {
    IMP_THROW("get_preprocessing_time: Preprocessing not done ",
                                                          ValueException);
  }
  return preprocessing_time_;
}

  //! Time employed for the coarse registration part
double ProjectionFinder::get_coarse_registration_time() const {
  if(!registration_done_) {
    IMP_THROW("get_coarse_registration_time: Coarse registration "
              "not done ",ValueException);
  }
  return coarse_registration_time_;
}

double ProjectionFinder::get_fine_registration_time() const {
  if(!registration_done_) {
    IMP_THROW("get_fine_registration_time: fine registration "
              "not done ",ValueException);
  }
  return fine_registration_time_;
}



void ProjectionFinder::preprocess_for_fast_coarse_registration(
        const cv::Mat &m,algebra::Vector2D &center,
        cv::Mat &POLAR_AUTOC) {
  // Make the matrix positive to compute the weighted centroid
  double min_value=0;
  cv::minMaxLoc(m,&min_value);
  cv::Mat result;
  result -= min_value;
  center=get_weighted_centroid(m);
  // Center the image in the weighted centroid
  algebra::Transformation2D T((-1)*center);
  get_transformed(m,result,T);
  // Get the autocorrelation in polar coordinates and its FFT
  cv::Mat autoc,polar_autoc;
  autocorrelation2D(result,autoc);
  resample_polar(autoc,polar_autoc,polar_params_);
  get_fft_using_optimal_size(polar_autoc,POLAR_AUTOC);
}


void ProjectionFinder::show(std::ostream &out) const {
  out << "ProjectionFinder:" << std::endl
  << "Number of projections = " << projections_.size()  << std::endl
  << "Number of subject images = " << subjects_.size() << std::endl
  << "Working parameters: " << std::endl
  << "Resolution: " <<  resolution_  << std::endl
  << "A/pixel: " << apix_ << std::endl
  << "Coarse egistration method: " << coarse_registration_method_ << std::endl
  << "Simplex initial size: " <<  simplex_initial_length_ << std::endl
  << "Simplex minimun size: " << simplex_minimum_size_ << std::endl
  << "Simplex maximum optimization steps: " <<optimization_steps_ << std::endl
  << "Save matching images: " << save_match_images_ << std::endl;
}




IMPEM2D_END_NAMESPACE
