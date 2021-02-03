/**
 *  \file isd/ CysteineCrossLinkRestraint.h
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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
#include <IMP/random.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/erf.hpp>
#include <iostream>

IMPISD_BEGIN_NAMESPACE

// first constructor, beta is unmarginalized and need to be sampled
CysteineCrossLinkRestraint::CysteineCrossLinkRestraint(Model *m,
    ParticleIndexAdaptor beta, ParticleIndexAdaptor sigma,
    ParticleIndexAdaptor epsilon, ParticleIndexAdaptor weight,
    CrossLinkData *data, double fexp)
    : Restraint(m, "CysteineCrossLinkRestraint%1%"),
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
CysteineCrossLinkRestraint::CysteineCrossLinkRestraint(Model *m,
    ParticleIndexAdaptor beta, ParticleIndexAdaptor sigma,
    ParticleIndexAdaptor epsilon, ParticleIndexAdaptor weight,
    CrossLinkData *data, CysteineCrossLinkData *ccldata)
    : Restraint(m, "CysteineCrossLinkRestraint%1%"),
      beta_(beta),
      sigma_(sigma),
      epsilon_(epsilon),
      weight_(weight),
      data_(data),
      ccldata_(ccldata) {
  constr_type_ = 1;
}

void CysteineCrossLinkRestraint::add_contribution(ParticleIndexAdaptor p1,
                                                  ParticleIndexAdaptor p2) {
  if (get_number_of_contributions() == 0)
    use_CA_ = true;
  else if (use_CA_ == false) {
    IMP_THROW("Use either CA or CB, not both!", ModelException);
  }
  Model *m = get_model();
  ps1_.push_back(p1);
  ps2_.push_back(p2);
  Weight w(m, weight_);
  if (w.get_number_of_weights() <
      static_cast<int>(get_number_of_contributions())) {
    w.add_weight();
    w.set_weights_lazy(w.get_unit_simplex().get_barycenter());
  }
}

void CysteineCrossLinkRestraint::add_contribution(ParticleIndexes p1,
                                                  ParticleIndexes p2) {
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
  Model *m = get_model();
  Weight w(m, weight_);
  if (w.get_number_of_weights() <
      static_cast<int>(get_number_of_contributions())) {
    w.add_weight();
    w.set_weights_lazy(w.get_unit_simplex().get_barycenter());
  }
}

// truncated normal standard error (according to Wikipedia)
double CysteineCrossLinkRestraint::get_standard_error() const {

  Model *m = get_model();
  double freq = get_model_frequency();
  double beta = Scale(m, beta_).get_scale();

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
    const ParticleIndexes &ps) const {
  Model *m = get_model();
  // get coordinates of central CA
  algebra::Vector3D x1 = core::XYZ(m, ps[1]).get_coordinates();

  // first versor
  algebra::Vector3D a =
      (core::XYZ(m, ps[0]).get_coordinates() - x1).get_unit_vector();

  // auxiliary vector
  algebra::Vector3D b =
      (core::XYZ(m, ps[2]).get_coordinates() - x1).get_unit_vector();

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
  Model *m = get_model();
  for (unsigned n = 0; n < get_number_of_contributions(); ++n) {
    double dist;
    if (use_CA_) {
      core::XYZ d1(m, ps1_[n]), d2(m, ps2_[n]);
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
  Model *m = get_model();
  Floats dists = get_distances();

  double sigma = Scale(m, sigma_).get_scale();

  Floats nus = data_->get_marginal_elements(sigma, dists);

  double epsilon = Scale(m, epsilon_).get_scale();

  double numax = data_->get_marginal_maximum(sigma);

  Floats frequencies;

  Weight w(m, weight_);
  IMP_INTERNAL_CHECK(
    get_number_of_contributions() == w.get_number_of_weights(),
    "Number of contributions does not equal weights dimension."
  );
  for (unsigned i = 0; i < get_number_of_contributions(); ++i) {

    double ww = w.get_weight(i);

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
  Model *m = get_model();
  double freq = get_model_frequency();

  double beta = Scale(m, beta_).get_scale();

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
  ParticlesTemp ret;
  Model *m = get_model();
  ret.push_back(m->get_particle(beta_));
  ret.push_back(m->get_particle(sigma_));
  ret.push_back(m->get_particle(epsilon_));
  ret.push_back(m->get_particle(weight_));
  for (unsigned i = 0; i < ps1_.size(); ++i) {
    ret.push_back(m->get_particle(ps1_[i]));
    ret.push_back(m->get_particle(ps2_[i]));
  }
  for (unsigned i = 0; i < pslist1_.size(); ++i) {
    for (unsigned j = 0; j < pslist1_[i].size(); ++j) {
      ret.push_back(m->get_particle(pslist1_[i][j]));
      ret.push_back(m->get_particle(pslist2_[i][j]));
    }
  }
  return ret;
}

IMPISD_END_NAMESPACE
