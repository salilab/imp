/**
 *  \file IMP/em2d/PolarResamplingParameters.h
 *  \brief Functions related with rotations in em2d
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H
#define IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/em2d/Matrix.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/algebra/constants.h"
#include "IMP/exception.h"
#include "IMP/log.h"
#include "IMP/log_macros.h"
#include <cereal/access.hpp>
#include <IMP/constants.h>

IMPEM2D_BEGIN_NAMESPACE

//! Class to manage the parameters required for the polar resampling in the
//! rotational_alignment function
class IMPEM2DEXPORT PolarResamplingParameters {

 public:
  PolarResamplingParameters() {
    parameters_set_ = false;
  };

  //! Compute the parameters for a polar resampling from the dimensions of
  //! a matrix
  PolarResamplingParameters(unsigned int rows, unsigned int cols) {
    setup(rows, cols);
  }

  //! Compute the parameters for a polar resampling getting the dimensions from
  //! the matrix
  /*!
      \param[in] m Matrix that is going to be resampled
  */
  PolarResamplingParameters(const DoubleMatrix &m) {
    setup(m.rows(), m.cols());
  }

  ~PolarResamplingParameters() {};

  //! Initialize the internal parameters to generate all the values
  //! The class uses a number of radius values for resampling that is
  //! optimal to perform  FFT during the rotational alignment. The first guess
  //! is  half the rows and columns
  void setup(unsigned int rows, unsigned int cols) {
    starting_radius_ = 5.0;
    n_angles_ = 0;
    matrix_rows_ = rows;
    matrix_cols_ = cols;
    ending_radius_ =
        std::min(rows / 2., cols / 2.);  // maximum radius, half the size
    n_rings_ = get_optimal_dft_size((int)ending_radius_);
    radius_step_ =
        (ending_radius_ - starting_radius_) / (static_cast<double>(n_rings_));
    parameters_set_ = true;
    IMP_LOG_VERBOSE("PolarResamplingParameters setup. Input matrix: "
                    << rows << " x " << cols << " Starting radius= "
                    << starting_radius_ << " Ending radius= " << ending_radius_
                    << " Rings= " << n_rings_ << std::endl);
  }

  //! Gets the initial radius of the resampling
  double get_starting_radius() const {
    get_is_setup();
    return starting_radius_;
  }

  //! Gets the largest radius
  double get_ending_radius() const {
    get_is_setup();
    return ending_radius_;
  }

  //! Gets the current radius employed for the ring in consideration)
  double get_radius(unsigned int n_ring) const {
    get_is_setup();
    IMP_USAGE_CHECK(n_ring <= n_rings_,
                    "PolarResamplingParameters: Requested ring is above the "
                    "maximum number");
    return starting_radius_ + n_ring * radius_step_;
  }

  //! Get the number of rings (that is, the number of radius values considered)
  unsigned int get_number_of_rings() const {
    get_is_setup();
    return n_rings_;
  }

  //! You give an approximated number of values that you want to
  //! use for the resampling, and the function computes the optimal number
  //! for an FFT based on this approximated number
  void set_estimated_number_of_angles(unsigned int aprox_value) {
    n_angles_ = get_optimal_dft_size(aprox_value);
    angle_step_ = (2 * PI) / static_cast<double>(n_angles_);
  }

  //! Gets the number of points that are sampled for the angles
  //! remember that this number is usually different to the approximated value
  //! that you provide to the function set_estimated_number_of_angles()
  unsigned int get_number_of_angles() const { return n_angles_; }

  //! get the angular step used
  double get_angle_step() const { return angle_step_; }

  //! Get the step for the radius coordinate
  double get_radius_step() const {
    if (get_is_setup() == false) {
      IMP_THROW("trying to get radius_step before initializing",
                IMP::ValueException);
    }
    return radius_step_;
  }

  //! After the number of radius and angles values are set, this function
  //! Builds a map of resampling coordinates. This map is very useful for
  //! speeding up the resampling (see do_resample_polar())
  void create_maps_for_resampling();

#ifndef SWIG
  //! Get the maps used for fast resampling with OpenCV's remap()
  /** The first map has the (fixed-point) coordinates of each sample, as pairs
      of shorts; the second map has the interpolation coefficients.
      \note For internal use.
   */
  void get_resampling_maps(internal::RowMajorArray<short> &m1,
                           internal::RowMajorArray<unsigned short> &m2) const {
    m1 = map_16SC2_;
    m2 = map_16UC1_;
  }

  //! Get the map of the (row, column) coordinates of each sample
  /** The coordinates are stored as pairs of floats.
      \note For internal use.
   */
  void get_resampling_map(internal::RowMajorArray<float> &m1) const {
    m1 = polar_map_;
  }
#endif

  void show(std::ostream &out) const {
    out << "starting_radius = " << starting_radius_ << std::endl;
    out << "ending_radius = " << ending_radius_ << std::endl;
    out << "n_rings = " << n_rings_ << std::endl;
  }

  bool get_is_setup() const {
    if (parameters_set_) return true;
    return false;
  }

 protected:
  // Map of the (row, column) coordinates of each sample (pairs of floats)
  internal::RowMajorArray<float> polar_map_;
  // Maps for speeding up resampling (see OpenCV's convertMaps())
  internal::RowMajorArray<short> map_16SC2_;
  internal::RowMajorArray<unsigned short> map_16UC1_;
  double starting_radius_;  // starting radius for the polar resampling
  double ending_radius_;    // ending radius for the polar resampling
  // Number of sampling points for radius and angles
  unsigned int n_rings_, n_angles_, matrix_rows_, matrix_cols_;
  bool parameters_set_;
  double radius_step_, angle_step_;

private:
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(polar_map_, map_16SC2_, map_16UC1_, starting_radius_,
       ending_radius_, n_rings_, n_angles_, matrix_rows_, matrix_cols_,
       parameters_set_, radius_step_, angle_step_);
  }
};

IMP_VALUES(PolarResamplingParameters, PolarResamplingParametersList);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H */
