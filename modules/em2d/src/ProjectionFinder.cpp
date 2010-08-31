/**
 *  \file ProjectionFinder.cpp
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2D/ProjectionFinder.h"
#include "IMP/em2D/pca_features_extraction.h"
#include "IMP/em/image_transformations.h"
#include "IMP/em2D/scores2D.h"
#include "IMP/em2D/filenames_manipulation.h"
#include "IMP/em2D/FFToperations.h"
#include "IMP/em2D/project.h"
#include "IMP/em2D/Fine2DRegistrationRestraint.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/gsl/Simplex.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE



void ProjectionFinder::preprocess_subjects_and_projections() {
  // FFT PREPROCESSING
  if(coarse_registration_method_ ==1 ||coarse_registration_method_==2) {
    IMP_LOG(IMP::TERSE,"ProjectionFinder: preprocessing subjects" << std::endl);
    fft_rings_subjects_.resize(n_subjects_);
    fft_rings_projections_.resize(n_projections_);
    preprocess_for_coarse_registration(subjects_,SUBJECTS_,
            fft_rings_subjects_,true,interpolation_method_);
    // Storing the FFT of projections is not needed
  IMP_LOG(IMP::TERSE,
            "ProjectionFinder: preprocessing projections" << std::endl);
    preprocess_for_coarse_registration(projections_,PROJECTIONS_,
            fft_rings_projections_,false,interpolation_method_);
  }
  // CENTERS OF GRAVITY AND ROTATIONAL FFT PREPROCESSING
  if(coarse_registration_method_==3) {

    IMP_LOG(IMP::TERSE,"ProjectionFinder: preprocessing subjects" << std::endl);
    fft_rings_subjects_.resize(n_subjects_);
    fft_rings_projections_.resize(n_projections_);
    preprocess_for_fast_coarse_registration(subjects_,
                                      subjects_cog_,
                                      fft_rings_subjects_,
                                      true,
                                      interpolation_method_);
    IMP_LOG(IMP::TERSE,"ProjectionFinder: preprocessing projections"
                                << std::endl);
    preprocess_for_fast_coarse_registration(projections_,
                                      projections_cog_,
                                      fft_rings_projections_,
                                      false,
                                      interpolation_method_);
  }
  // PCA PREPROCESSING
  if(coarse_registration_method_ ==2 || coarse_registration_method_==3) {
    double pca_threshold = 0.0;
    IMP_LOG(IMP::TERSE,"PCA Preprocessing .... " << std::endl);
    subjects_pcas_=pca_features(subjects_,pca_threshold);
    projections_pcas_=pca_features(projections_,pca_threshold);
  }
}

double ProjectionFinder::get_coarse_registration(
                            bool save_match_images) {
  IMP_LOG(IMP::TERSE,"ProjectionFinder: get coarse registration" << std::endl);
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

/*********************************
    std::cout << "COARSE REGISTRATIONS FOR SUBJECT " << i << std::endl;
    std::ofstream f;
    std::ostringstream fn;
    fn << "all-subject-" << i << "-coarse_matches.sel";
    f.open(fn.str().c_str(),std::ios::out);
    for (unsigned int k=0;k<subject_RRs.size();++k) {
      f << "proj-" << subject_RRs[k].get_index() << ".spi 1" << std::endl;
      subject_RRs[k].write();
    }
    f.close();
*********************************/
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
    if(save_match_images) {
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
  /***** Computation ********/
  // Align with all projections
  double ccc=0,max_ccc=0;
  // Save all the registration results for the subject
  subject_RRs.resize(n_projections_);
  for(unsigned long j=0;j<n_projections_;++j) {
    // If method includes PCA check, do it
    if((coarse_registration_method_==2 || coarse_registration_method_==3) &&
      !pca_features_match(subjects_pcas_[i],projections_pcas_[j],0.1)) {
      continue;
    }
    // Method without preprocessing
    if(coarse_registration_method_==0) {
      ccc=align2D_complete(subjects_[i]->get_data(),
                            projections_[j]->get_data(),t);
    }
    // Methods with preprocessing and FFT alignment
    if(coarse_registration_method_==1 || coarse_registration_method_==2) {
      ccc=align2D_complete_no_preprocessing(
                             subjects_[i]->get_data(),
                             SUBJECTS_[i],
                             fft_rings_subjects_[i],
                             projections_[j]->get_data(),
                             fft_rings_projections_[j],t,false);
    }
    // Method with centers of gravity alignment
    if(coarse_registration_method_==3) {
      PolarResamplingParameters polar_params(subjects_[i]->get_data());
      align2D_complete_with_centers_no_preprocessing(
                      subjects_cog_[i],projections_cog_[j],
                      fft_rings_subjects_[i],
                      fft_rings_projections_[j],
                      t,polar_params);
      algebra::Matrix2D_d aux;
      em::apply_Transformation2D(projections_[j]->get_data(),t,aux,true);
      ccc=subjects_[i]->get_data().cross_correlation_coefficient(aux);
    }

    if(ccc> max_ccc) {
        max_ccc=ccc;
        best_transformation=t;
      }

    // Save projection parameters
    subject_RRs[j]= RegistrationResult(
                          projections_[j]->get_header().get_Phi(),
                          projections_[j]->get_header().get_Theta(),
                          projections_[j]->get_header().get_Psi(),
                          0.0,0.0,j,ccc);
    // add the 2D alignment transformation found
    subject_RRs[j].add_in_plane_transformation(t);
  }
  return best_transformation;
}


double ProjectionFinder::get_complete_registration(
    bool save_match_images,double apix,
   unsigned int optimization_steps_, double simplex_minimum_size) {
  IMP_LOG(IMP::TERSE,
          "ProjectionFinder: get complete registration" << std::endl);

  IMP_USAGE_CHECK(subjects_set_,
        "get_complete_registration: subject images have not been set");
  IMP_USAGE_CHECK(projections_set_,
        "get_complete_registration: projection images have not been set");
 IMP_USAGE_CHECK(particles_set_,
        "get_complete_registration: model particles have not been set\n");
  /********* Variables **********/
  em::SpiderImageReaderWriter<double> srw;
  algebra::Transformation2D t;
  bool masks_computed=false;
  MasksManager masks;
  IMP_NEW(em::Image,match,());
  unsigned int rows= subjects_[0]->get_data().get_number_of_rows();
  unsigned int cols= subjects_[0]->get_data().get_number_of_columns();
  match->resize(rows,cols);
  /***** Set optimizer ********/
  IMP_NEW(Model,scoring_model,());
  IMP_NEW(Fine2DRegistrationRestraint,fine2d,());
  IMP_NEW(IMP::gsl::Simplex,simplex_optimizer,());
  fine2d->initialize(model_particles_,resolution_,apix,scoring_model);
  simplex_optimizer->set_model(scoring_model);
  simplex_optimizer->set_initial_length(0.1);
  simplex_optimizer->set_minimum_size(simplex_minimum_size);
  IMP::SetLogState log_state(fine2d,IMP::TERSE);
  /***** Computation ********/
  preprocess_subjects_and_projections();
  IMP_LOG(IMP::DEFAULT,"Registration .... " << std::endl);
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
/*********************************
    // Show all the coarse scores for a subject:
    std::sort(subject_RRs.begin(),subject_RRs.end(),
              compare_registration_results);
    std::cout << "COARSE REGISTRATIONS FOR SUBJECT " << i << std::endl;

    std::ofstream f;
    std::ostringstream sel_file_coarse;
    sel_file_coarse << "all-subject-" << i << "-coarse_matches.sel";
    f.open(sel_file_coarse.str().c_str(),std::ios::out);
    for (unsigned int k=0;k<subject_RRs.size();++k) {
      f << "proj-" << subject_RRs[k].get_index() << ".spi 1" << std::endl;
      subject_RRs[k].write();
    }
    f.close();
*********************************/

    for (unsigned int k=0;k<subject_RRs.size();++k) {
      // Fine registration of the subject using simplex
      subject_RRs[k].set_in_image(*subjects_[i]);
      fine2d->set_subject_image(*subjects_[i]);
      simplex_optimizer->optimize((double)optimization_steps_);
      // Update the registration parameters
      double score = fine2d->get_final_values(subject_RRs[k]);
      subject_RRs[k].set_ccc(em2D_score_to_ccc(score));
      IMP_LOG(IMP::VERBOSE, "fine registration for subject " << k
          << ": " << subject_RRs[k] << std::endl);
      ++show_progress;
    }
    std::sort(subject_RRs.begin(),subject_RRs.end(),
                                            compare_registration_results);
    // Best fine registration
    registration_results_[i]=subject_RRs[0];
    Score += ccc_to_em2D_score(registration_results_[i].get_ccc());
    // save if requested
    if(save_match_images) {
      if(!masks_computed) {
        masks.init_kernel(resolution_,apix);
        masks.generate_masks(model_particles_);
        masks_computed=true;
      }
      std::ostringstream strm;
      strm << "fine_match-" << i << ".spi";
      generate_projection(*match,model_particles_,registration_results_[i],
                    resolution_,apix,srw,&masks,false,strm.str());
      em::normalize(*match,true);
      registration_results_[i].set_in_image(*match);
      match->write_to_floats(strm.str(),srw);
    }
  }
        IMP_LOG(IMP::TERSE, "get_complete_registration: END" << std::endl);

  registration_done_=true;
  return Score/n_subjects_;
}



void ProjectionFinder::all_vs_all_projections_ccc(String &fn_out) {
  IMP_USAGE_CHECK(projections_set_,
          "all_vs_all_projections_ccc: projection images have not been set");
  IMP_USAGE_CHECK(subjects_set_,
          "all_vs_all_projections_ccc: subject images have not been set");
  IMP_LOG(IMP::DEFAULT,"Preprocessing projections .... " << std::endl);
  fft_rings_projections_.resize(n_projections_);
  // First preprocess the projections as being subjects
  preprocess_for_coarse_registration(projections_,PROJECTIONS_,
         fft_rings_subjects_,true,interpolation_method_);
  // Then as projections
  preprocess_for_coarse_registration(projections_,PROJECTIONS_,
         fft_rings_projections_,false,interpolation_method_);

  IMP_LOG(IMP::TERSE,
           "Starting all against all projection matching" << std::endl);
  double correlations[n_projections_][n_projections_];
  boost::progress_display show_progress(n_projections_*n_projections_/2.);
  algebra::Transformation2D t;
  PolarResamplingParameters polar_params(projections_[0]->get_data());
  for(unsigned int i=0;i<n_projections_;++i) {
    for(unsigned int j=i+1;j<n_projections_;++j) {
      double ccc=align2D_complete_no_preprocessing(
                             projections_[i]->get_data(),
                             PROJECTIONS_[i],
                             fft_rings_subjects_[i],
                             projections_[j]->get_data(),
                             fft_rings_projections_[j],t,false);

       correlations[i][j]=ccc;
/*********************************
      // Saves time if the image is saved, the transformation is not done twice.
      em::SpiderImageReaderWriter<double> srw;
      IMP_NEW(em::Image,match,());
      em2D::apply_Transformation2D(projections_[j]->get_data(),t,
                                                  match->get_data(),true);
      RegistrationResult rr;
      rr.add_in_plane_transformation(t);
      rr.set_in_image(*match);
      em2D::normalize(*match,true);
      std::ostringstream strm;
      strm << "match-"  << i << "-" << j << ".spi";
      match->write_to_floats(strm.str(),srw);
*********************************/
      ++show_progress;
    }
  }
  // Write to a file
  std::ofstream f;
  std::ostringstream fn;
  fn << fn_out;
  f.open(fn.str().c_str(),std::ios::out);
  for(unsigned int i=0;i<n_projections_;++i) {
    for(unsigned int j=i+1;j<n_projections_;++j) {
      f << i <<" "<< j <<" "<<correlations[i][j] << std::endl;
    }
  }
  f.close();
}





void preprocess_for_coarse_registration(em::Images input_set,
        std::vector< algebra::Matrix2D_c > &MATRICES,
        std::vector< complex_rings > &fft_rings_images,
        bool dealing_with_subjects,int interpolation_method) {
  unsigned long size = input_set.size();
  boost::progress_display show_progress(size);
  for(unsigned long i=0;i<input_set.size();++i) {
    // Compute the autocorrelation
    algebra::Matrix2D_d autoc;
    autoc.resize(input_set[i]->get_data());
    algebra::Matrix2D_c INPUT;
    FFT2D fft(input_set[i]->get_data(),INPUT); fft.execute();
    autocorrelation2D_no_preprocessing(INPUT,autoc);
    // Preprocess for rotational alignment
    complex_rings RINGS =preprocess_for_align2D_rotational(
                                           autoc,dealing_with_subjects,
                                           interpolation_method);
    if(dealing_with_subjects) {
      MATRICES.push_back(INPUT);
    }
    fft_rings_images[i]=RINGS;
    ++show_progress;
  }
}


void preprocess_for_fast_coarse_registration(em::Images input_set,
        algebra::Vector2Ds centers,
        std::vector< complex_rings > &fft_rings_images,
        bool dealing_with_subjects,
        int interpolation_method) {
  unsigned long size = input_set.size();
  centers.resize(size);
  boost::progress_display show_progress(size);
  for(unsigned long i=0;i<size;++i) {
    // Make the matrix positive to compute the center of gravity
    double min_value=input_set[i]->get_data().compute_min();
    algebra::Matrix2D_d result(input_set[i]->get_data());
    result -= min_value; // Now the matrix "result" is positive
    // To compute the center of gravity and FFT rings
    //  the matrix has to be centered
    result.centered_start();
    centers[i]=compute_center_of_gravity(result);
    // Center the image in the center of gravity
    algebra::Vector2D minus_cent = (-1)*centers[i];
    em::shift(input_set[i]->get_data(),minus_cent,result,true);
    // Preprocess for rotational alignment
    complex_rings RINGS=preprocess_for_align2D_rotational(
                  result,dealing_with_subjects,interpolation_method);
    fft_rings_images[i]=RINGS;
    ++show_progress;
  }
}


IMPEM2D_END_NAMESPACE
