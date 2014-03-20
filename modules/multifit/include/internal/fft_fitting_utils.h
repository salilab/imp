/**
 *  \file fft_fitting_utils.h   \brief masking function for fft fitting
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FFT_FITTING_UTILS_H
#define IMPMULTIFIT_FFT_FITTING_UTILS_H

#include <IMP/base/log.h>
#include <IMP/base/exception.h>
#include <IMP/multifit/multifit_config.h>
#include <IMP/em/DensityMap.h>
#include <IMP/atom/Hierarchy.h>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

typedef struct {
  unsigned long ifft;
  unsigned long ireal;
  unsigned int ix;
  unsigned int iy;
  unsigned int iz;
} FFTScore;
typedef std::vector<FFTScore> FFTScores;

struct RotScore {
  RotScore(unsigned int rot_ind, double score)
      : rot_ind_(rot_ind), score_(score) {}
  unsigned int rot_ind_;
  double score_;
};
typedef std::vector<RotScore> RotScores;
typedef std::vector<RotScores> RotScoresVec;

class IMPMULTIFITEXPORT EulerAngles {
 public:
  EulerAngles(double in_psi, double in_theta, double in_phi) {
    psi = in_psi;
    theta = in_theta;
    phi = in_phi;
  }
  double psi;
  double theta;
  double phi;
};

typedef std::vector<EulerAngles> EulerAnglesList;

//! relaxes shell of width radius, just outside thresholded area,
//!by the Poisson equation
IMPMULTIFITEXPORT
void relax_laplacian(em::DensityMap *dmap, unsigned ignored[3], double radius);

//! convolves a map with a kernel
//! The function filters out convolutions where kernel hits zero density
//!in the input map
//! to avoid cutoff edge effects when using finite difference kernel
IMPMULTIFITEXPORT
void convolve_kernel_inside_erode(em::DensityMap *dmap, double *kernel,
                                  unsigned kernel_size);
IMPMULTIFITEXPORT
void get_rotation_matrix(double m[3][3], double psi, double theta, double phi);
IMPMULTIFITEXPORT
void rotate_mol(atom::Hierarchy mh, double psi, double theta, double phi);
IMPMULTIFITEXPORT
void translate_mol(atom::Hierarchy mh, algebra::Vector3D t);

IMPMULTIFITEXPORT
double *convolve_array(double *in_arr, unsigned int nx, unsigned int ny,
                       unsigned int nz, double *kernel, unsigned int nk);

IMPMULTIFITEXPORT
EulerAnglesList get_uniformly_sampled_rotations(
    double angle_sampling_internal_rad);
IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPMULTIFIT_FFT_FITTING_UTILS_H */
