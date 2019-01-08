/**
 *  \file spb/ EM2DRestraint.cpp
 *  \brief
 *  Put description here
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/Vector3D.h>
#include <IMP/constants.h>
#include <IMP/core/XYZ.h>
#include <IMP/em2d/Image.h>
#include <IMP/em2d/RegistrationResult.h>
#include <IMP/em2d/TIFFImageReaderWriter.h>
#include <IMP/em2d/align2D.h>
#include <IMP/em2d/project.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/EM2DRestraint.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <boost/math/special_functions/erf.hpp>
#include <iostream>

IMPSPB_BEGIN_NAMESPACE

EM2DRestraint::EM2DRestraint(Particles ps, Particle* sigma, std::string ifile,
                             Float px_size, Float res)
    : spb::ISDRestraint(ps[0]->get_model(), "EM2DRestraint%1%") {
  ps_ = ps;
  sigma_ = sigma;
  constr_type_ = 0;
  // read experimental image
  read_image(ifile, px_size, res);
}

EM2DRestraint::EM2DRestraint(Particles ps, Floats sigma_grid, Floats fmod_grid,
                             std::string ifile, Float px_size, Float res)
    : spb::ISDRestraint(ps[0]->get_model(), "EM2DRestraint%1%") {
  ps_ = ps;
  constr_type_ = 1;

  // read experimental image
  read_image(ifile, px_size, res);

  // store fmod grid
  fmod_grid_ = fmod_grid;

  // prepare marginal likelihood grid
  init_grid(sigma_grid);
}

void EM2DRestraint::init_grid(const Floats& sigma_grid) {
  for (unsigned i = 0; i < fmod_grid_.size(); ++i) {
    double fmod = fmod_grid_[i];

    double cumul = 0;

    for (unsigned j = 1; j < sigma_grid.size(); ++j) {
      double sj = sigma_grid[j];
      double sjm1 = sigma_grid[j - 1];
      double ds = sj - sjm1;

      double pj = get_element(fmod, sj) / sj;
      double pjm1 = get_element(fmod, sjm1) / sjm1;

      cumul += (pj + pjm1) / 2.0 * ds;
    }
    // add marginal element
    grid_.push_back(cumul);
  }
}

void EM2DRestraint::read_image(std::string ifile, Float px_size, Float res) {
  // TIFF reader and writer
  IMP_NEW(em2d::TIFFImageReaderWriter, trw, ());

  // initialize image
  image_ = new em2d::Image(ifile, trw);
  // get rows and columns
  rows_ = image_->get_header().get_number_of_rows();
  cols_ = image_->get_header().get_number_of_columns();

  // options
  options_ = em2d::ProjectingOptions(px_size, res, trw);
  options_.save_images = false;

  // registration results: only one projection
  algebra::Vector2D shift = algebra::Vector2D(0., 0.);
  em2d::RegistrationResult rr = em2d::RegistrationResult(0., 0., 0., shift);
  reg_res_.push_back(rr);
};

void EM2DRestraint::set_particles(Particles ps) { ps_ = ps; }

double EM2DRestraint::get_cross_correlation() const {
  // get the projection
  em2d::Images prjs =
      em2d::get_projections(ps_, reg_res_, rows_, cols_, options_);

  // with alignment
  em2d::ResultAlign2D ra = em2d::get_complete_alignment(
      image_->get_data(), prjs[0]->get_data(), true);

  // and the correlation coefficient is the second member of the pair
  Float cc = ra.second;

  return cc;
}

double EM2DRestraint::get_normal_pdf(double mean, double sigma,
                                     double x) const {
  double inv_sigma = 1.0 / (sigma * sqrt(2.0));

  double prob = inv_sigma / sqrt(IMP::PI) *
                exp(-(mean - x) * (mean - x) * inv_sigma * inv_sigma);

  return prob;
}

double EM2DRestraint::get_truncated_normalization(double mean,
                                                  double sigma) const {
  double inv_sigma = 1.0 / (sigma * sqrt(2.0));

  double norm = 0.5 * (boost::math::erf(mean * inv_sigma) -
                       boost::math::erf((mean - 1.0) * inv_sigma));
  return norm;
}

int EM2DRestraint::get_closest(std::vector<double> const& vec,
                               double value) const {
  std::vector<double>::const_iterator ub =
      std::lower_bound(vec.begin(), vec.end(), value);
  if (ub == vec.end()) {
    return vec.size() - 1;
  } else if (ub == vec.begin()) {
    return 0;
  } else {
    std::vector<double>::const_iterator lb = ub - 1;
    int index = int(lb - vec.begin());
    if (fabs(*ub - value) < fabs(*lb - value)) {
      index = int(ub - vec.begin());
    }
    return index;
  }
}

double EM2DRestraint::get_element(double fmod, double sigma) const {
  double prob = get_normal_pdf(fmod, sigma, 1.0) /
                get_truncated_normalization(fmod, sigma);
  return prob;
}

double EM2DRestraint::get_marginal_element(double fmod) const {
  unsigned id = get_closest(fmod_grid_, fmod);
  return grid_[id];
}

/* call for probability */
// Probability is a normal distribution truncated to 0-1
double EM2DRestraint::get_probability() const {
  double prob = 0.;

  double fmod = get_cross_correlation();

  if (constr_type_ == 0) {
    double sigma = isd::Scale(sigma_).get_scale();

    prob = get_element(fmod, sigma);

  } else if (constr_type_ == 1) {
    prob = get_marginal_element(fmod);
  }

  return prob;
}

double EM2DRestraint::unprotected_evaluate(DerivativeAccumulator* accum) const {
  double score;

  double prob = get_probability();

  // check if probability is too low (e.g. equal to zero)
  // and assign its value to the smallest double
  if (prob <= std::numeric_limits<double>::epsilon()) {
    prob = std::numeric_limits<double>::epsilon();
  }

  score = -log(prob);

  if (accum) {
  }

  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
IMP::ModelObjectsTemp EM2DRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  ret.push_back(sigma_);
  return ret;
}

IMPSPB_END_NAMESPACE
