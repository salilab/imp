/**
 *  \file CoarseCCatIntervals.cpp
 *  \brief Cross correlation coefficient calculator.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/CoarseCCatIntervals.h>
#include <IMP/em/masking.h>

IMPEM_BEGIN_NAMESPACE

CoarseCCatIntervals::CoarseCCatIntervals(const int &ncd) {

  // Number of times the evaluation has been called. The evaluation is only
  // performed the first time and when calls_counter_ reaches eval_interval.
  // Otherwise the stored_cc_ value is returned
  calls_counter_ = 0;

  stored_cc_ = 0.0;
  dv_memory_allocated_ = false;
  allocate_derivatives_array(ncd);
}

CoarseCCatIntervals::CoarseCCatIntervals() {
  calls_counter_ = 0;
  stored_cc_ = 0.0;
  dv_memory_allocated_ = false;
}

void CoarseCCatIntervals::allocate_derivatives_array(int ncd) {
  if (dv_memory_allocated_) return;
  // Allocate memmory for the derivative terms if not done yet
  stored_dvx_ = new double[ncd];
  stored_dvy_ = new double[ncd];
  stored_dvz_ = new double[ncd];

  for (int i = 0; i < ncd; i++) {
    stored_dvx_[i] = 0.0;
    stored_dvy_[i] = 0.0;
    stored_dvz_[i] = 0.0;
  }
  dv_memory_allocated_ = true;
}

std::pair<double, algebra::Vector3Ds> CoarseCCatIntervals::evaluate(
    DensityMap *em_map, SampledDensityMap *model_map,
    const algebra::Vector3Ds &deriv, float scalefac, bool lderiv,
    unsigned long eval_interval) {
  // eval_interval is the interval size before recalculating the CC score
  algebra::Vector3Ds out_dv;
  unsigned int number_of_particles = model_map->get_xyzr_particles().size();
  // If the function requires to be evaluated
  if (calls_counter_ % eval_interval == 0) {
    // The base evaluate function calculates the derivates of the EM term.
    stored_cc_ = CoarseCC::calc_score(em_map, model_map, scalefac);
    if (lderiv) {
      out_dv = CoarseCC::calc_derivatives(
          em_map, model_map, model_map->get_sampled_particles(),
          model_map->get_weight_key(), model_map->get_kernel_params(), scalefac,
          deriv);
    }

    calls_counter_ = 1;
    if (lderiv) {
      // sync the derivatives.
      allocate_derivatives_array(number_of_particles);
      for (unsigned int i = 0; i < number_of_particles; i++) {
        stored_dvx_[i] = out_dv[i][0];
        stored_dvy_[i] = out_dv[i][1];
        stored_dvz_[i] = out_dv[i][2];
      }
    }
  }
      // If the evaluation was not required, return the previously stored values
      else {
    algebra::Vector3D v0(0., 0., 0.);
    out_dv.insert(out_dv.end(), number_of_particles, v0);
    for (unsigned int i = 0; i < number_of_particles; i++) {
      if (lderiv) {
        out_dv[i][0] = stored_dvx_[i];
        out_dv[i][1] = stored_dvy_[i];
        out_dv[i][2] = stored_dvz_[i];
      }
    }
    ++calls_counter_;
  }

  return std::pair<double, algebra::Vector3Ds>(stored_cc_, out_dv);
}

IMPEM_END_NAMESPACE
