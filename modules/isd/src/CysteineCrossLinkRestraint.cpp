/**
 *  \file isd/ CysteineCrossLinkRestraint.h
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <stdio.h>
#include <IMP/constants.h>
#include <IMP/isd/CysteineCrossLinkRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/Weight.h>
#include <math.h>
#include <limits.h>
#include <IMP/base/random.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/erf.hpp>
#include <iostream>

IMPISD_BEGIN_NAMESPACE

// first constructor, beta is unmarginalized and need to be sampled
CysteineCrossLinkRestraint::CysteineCrossLinkRestraint(
    kernel::Particle *beta, kernel::Particle *sigma, kernel::Particle *epsilon,
    kernel::Particle *weight, CrossLinkData *data, double fexp)
    : Restraint(beta->get_model(), "CysteineCrossLinkRestraint%1%"),
      beta_(beta),
      sigma_(sigma),
      epsilon_(epsilon),
      weight_(weight),
      data_(data),
      fexp_(fexp) {
  constr_type_ = 0;
}

// second constructor, marginalize the omega on the beat variable
// using the support of the CysteineCrossLinkData
CysteineCrossLinkRestraint::CysteineCrossLinkRestraint(
    kernel::Particle *beta, kernel::Particle *sigma, kernel::Particle *epsilon,
    kernel::Particle *weight, CrossLinkData *data,
    CysteineCrossLinkData *ccldata)
    : Restraint(beta->get_model(), "CysteineCrossLinkRestraint%1%"),
      beta_(beta),
      sigma_(sigma),
      epsilon_(epsilon),
      weight_(weight),
      data_(data),
      ccldata_(ccldata) {
  constr_type_ = 1;
}

void CysteineCrossLinkRestraint::add_contribution(kernel::Particle *p1,
                                                  kernel::Particle *p2) {
  if (get_number_of_contributions() == 0)
    use_CA_ = true;
  else if (use_CA_ == false) {
    IMP_THROW("Use either CA or CB, not both!", ModelException);
  }
  ps1_.push_back(p1);
  ps2_.push_back(p2);
  if (Weight(weight_).get_number_of_states() <
      static_cast<int>(get_number_of_contributions())) {
    Weight(weight_).add_weight();
  }
}

void CysteineCrossLinkRestraint::add_contribution(kernel::Particles p1,
                                                  kernel::Particles p2) {
  if (get_number_of_contributions() == 0)
    use_CA_ = false;
  else if (use_CA_ == true) {
    IMP_THROW("Use either CA or CB, not both!", ModelException);
  }
  if (p1.size() != 3 || p2.size() != 3) {
    IMP_THROW("Exactly 3 CA are needed!", ModelException);
  }
  pslist1_.push_back(p1);
  pslist2_.push_back(p2);
  if (Weight(weight_).get_number_of_states() <
      static_cast<int>(get_number_of_contributions())) {
    Weight(weight_).add_weight();
  }
}

// truncated normal standard error (according to Wikipedia)
double CysteineCrossLinkRestraint::get_standard_error() const {

  double freq = get_model_frequency();
  double beta = Scale(beta_).get_scale();

  // using constructor type 1 beta corresponds to omega0
  if (constr_type_ == 1) {
    beta = ccldata_->get_omega(freq, beta);
  }

  double norm = get_truncated_normalization(freq, beta);

  double n0 = get_normal_pdf(freq, beta, 0.) / norm;
  double n1 = get_normal_pdf(freq, beta, 1.) / norm;

  double var = beta * beta * (1.0 + (-freq * n0 - (1.0 - freq) * n1) -
                              beta * beta * (n0 - n1) * (n0 - n1));
  return sqrt(var);
}

algebra::Vector3D CysteineCrossLinkRestraint::get_CB_coordinates(
    const kernel::Particles &ps) const {
  // get coordinates of central CA
  algebra::Vector3D x1 = core::XYZ(ps[1]).get_coordinates();

  // first versor
  algebra::Vector3D a =
      (core::XYZ(ps[0]).get_coordinates() - x1).get_unit_vector();

  // auxiliary vector
  algebra::Vector3D b =
      (core::XYZ(ps[2]).get_coordinates() - x1).get_unit_vector();

  // second versor
  algebra::Vector3D k = (algebra::get_vector_product(a, b)).get_unit_vector();

  // third versor
  algebra::Vector3D j = (algebra::get_vector_product(a, k)).get_unit_vector();

  // get cosine of alpha
  double cosalpha = a.get_scalar_product(b);
  double sinalpha = sqrt(1. - cosalpha * cosalpha);

  // get cosine of phi
  double cosphi = -sinalpha / sqrt(2. - 2. * cosalpha);
  double sinphi = sqrt(1. - cosphi * cosphi);

  algebra::Vector3D cbcoord = (cosphi * a + sinphi * j) * 1.53 + x1;

  return cbcoord;
}

Floats CysteineCrossLinkRestraint::get_distances() const {
  Floats dists;
  for (unsigned n = 0; n < get_number_of_contributions(); ++n) {
    double dist;
    if (use_CA_) {
      core::XYZ d1(ps1_[n]), d2(ps2_[n]);
      dist = core::get_distance(d1, d2);
    } else {
      algebra::Vector3D cb1 = get_CB_coordinates(pslist1_[n]);
      algebra::Vector3D cb2 = get_CB_coordinates(pslist2_[n]);
      dist = (cb1 - cb2).get_magnitude();
    }
    dists.push_back(dist);
  }
  return dists;
}

double CysteineCrossLinkRestraint::get_model_frequency() const {

  Floats freqs = get_frequencies();
  double freq = std::accumulate(freqs.begin(), freqs.end(), 0.0);

  return freq;
}

Floats CysteineCrossLinkRestraint::get_frequencies() const {
  // list of marginal probabilities
  Floats dists = get_distances();

  double sigma = Scale(sigma_).get_scale();

  Floats nus = data_->get_marginal_elements(sigma, dists);

  double epsilon = Scale(epsilon_).get_scale();

  double numax = data_->get_marginal_maximum(sigma);

  Floats frequencies;

  for (unsigned i = 0; i < get_number_of_contributions(); ++i) {

    double ww = Weight(weight_).get_weight(i);

    double fi = (1.0 - pow(epsilon, nus[i] / numax)) * ww;
    frequencies.push_back(fi);
  }

  return frequencies;
}

double CysteineCrossLinkRestraint::get_normal_pdf(double mean, double sigma,
                                                  double x) const {
  double inv_sigma = 1.0 / (sigma * sqrt(2.0));

  double prob = inv_sigma / sqrt(IMP::PI) *
                exp(-(mean - x) * (mean - x) * inv_sigma * inv_sigma);

  return prob;
}

double CysteineCrossLinkRestraint::get_truncated_normalization(
    double mean, double sigma) const {
  double inv_sigma = 1.0 / (sigma * sqrt(2.0));

  double norm = 0.5 * (boost::math::erf(mean * inv_sigma) -
                       boost::math::erf((mean - 1.0) * inv_sigma));
  return norm;
}

// this is to use with truncated normal distribution
double CysteineCrossLinkRestraint::get_probability() const {
  double freq = get_model_frequency();

  double beta = Scale(beta_).get_scale();

  double prob = 1.0;

  if (constr_type_ == 0) {
    prob = get_normal_pdf(freq, beta, fexp_) /
           get_truncated_normalization(freq, beta);
  }

  if (constr_type_ == 1) {
    prob = ccldata_->get_marginal_element(freq, beta);
  }

  return prob;
}

double CysteineCrossLinkRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
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

unsigned CysteineCrossLinkRestraint::get_number_of_contributions() const {
  // we return the max of the two list to get the actual size of the used list
  // since we don't know which list we are using
  return std::max(ps1_.size(), pslist1_.size());
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp CysteineCrossLinkRestraint::do_get_inputs() const {
  kernel::ParticlesTemp ret;
  ret.push_back(beta_);
  ret.push_back(sigma_);
  ret.push_back(epsilon_);
  ret.push_back(weight_);
  for (unsigned i = 0; i < ps1_.size(); ++i) {
    ret.push_back(ps1_[i]);
    ret.push_back(ps2_[i]);
  }
  for (unsigned i = 0; i < pslist1_.size(); ++i) {
    for (unsigned j = 0; j < pslist1_[i].size(); ++j) {
      ret.push_back(pslist1_[i][j]);
      ret.push_back(pslist2_[i][j]);
    }
  }
  return ret;
}

IMPISD_END_NAMESPACE
