/**
 *  \file PolarResamplingParameters.h
 *  \brief Funtions related with rotations in em2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H
#define IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H


#include "IMP/em2D/em2D_config.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/constants.h"
#include "IMP/exception.h"
#include "IMP/log.h"

IMPEM2D_BEGIN_NAMESPACE

//! Class to manage the parameters required for the polar resampling in the
//! rotational_alignment function

class IMPEM2DEXPORT PolarResamplingParameters {

public:

  PolarResamplingParameters() {
    parameters_set_=false;
  };

  //! The initial and final radius for the polar resampling are computed
  //! from the matrix dimensions. Also the number of rings to employ.
  /**
      \params[in] matrix Matrix that is going to be resampled
  **/
  PolarResamplingParameters(algebra::Matrix2D_d &matrix) {
    initialize(matrix);
  }

  void initialize(algebra::Matrix2D_d &m) {
    starting_radius_=5.0;
    ending_radius_=std::min(m.get_number_of_rows(),m.get_number_of_columns());
    n_rings_ = (unsigned int)floor(ending_radius_/3); // quick
    radius_step_ = (ending_radius_-starting_radius_)/((double)(n_rings_));
    parameters_set_ = true;
    IMP_LOG(IMP::VERBOSE,
       "PolarResamplingParameters initialized. Input matrix: "
      << m.get_number_of_rows() << " x " << m.get_number_of_columns()
      << " Starting radius= "  <<  starting_radius_  << " Ending radius= "
      << ending_radius_ << " Rings= " << n_rings_ << std::endl);
   }

  double get_starting_radius() const {
    check_initalized();
    return starting_radius_;
  }

  double get_ending_radius() const  {
    check_initalized();
    return ending_radius_;
  }

  double get_radius(unsigned int n_ring) const {
    check_initalized();
    IMP_USAGE_CHECK(n_ring<=n_rings_,
     "PolarResamplingParameters: Requested ring is above the maximum number");
    return starting_radius_+n_ring*radius_step_;
  }

  unsigned int get_number_of_rings() const {
    check_initalized();
    return n_rings_;
  }

  bool get_parameters_are_set() const {
    return parameters_set_;
  }

  unsigned int get_sampling_points (unsigned int n_ring) const {
    return algebra::get_closer_power_of_2(2*PI*get_radius(n_ring));
  }

  double get_angle_step(unsigned int n_ring) const {
    return (2*PI)/(double)get_sampling_points(n_ring);
  }


  // radius step does not depend on the ring number
  double get_radius_step() const {
    check_initalized();
    return radius_step_;
  }


  void show(std::ostream &out) const {
    out << "starting_radius = " << starting_radius_ << std::endl;
    out << "ending_radius = " << ending_radius_ << std::endl;
    out << "n_rings = " << n_rings_ << std::endl;
  }

~PolarResamplingParameters() {};

protected:

  void check_initalized() const  {
   IMP_USAGE_CHECK(parameters_set_,
   "Attempting to use PolarResamplingParameters not initialized ");
  }

  double starting_radius_; // starting radius for the polar resamplig
  double ending_radius_; // ending radius for the polar resamplig
  unsigned int n_rings_; // Number of rings to get
  bool parameters_set_;
  double radius_step_;
};

IMP_OUTPUT_OPERATOR(PolarResamplingParameters);
IMP_VALUES(PolarResamplingParameters, PolarResamplingParametersList);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_POLAR_RESAMPLING_PARAMETERS_H */
