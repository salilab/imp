/**
 *  \file CrossLinkData.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/CrossLinkData.h>
#include <IMP/macros.h>
#include <IMP/base/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

IMPISD_BEGIN_NAMESPACE

CrossLinkData::CrossLinkData(Floats dist_grid, Floats omega_grid,
                             Floats sigma_grid, double lexp, double don,
                             double doff, int prior_type)
    : base::Object("Data Structure for CrossLinkMSRestraint %1%") {
  prior_type_ = prior_type;
  lexp_ = lexp;
  bias_ = false;
  // this constructor calculates the marginal likelihood using a theta function
  // to approximate the propensity of the linker

  // Store dist grid
  dist_grid_ = dist_grid;

  // Store omega grid
  omega_grid_ = omega_grid;

  // Store sigma grid
  sigma_grid_ = sigma_grid;

  for (unsigned k = 0; k < sigma_grid_.size(); ++k) {

    double sigma = sigma_grid_[k];

    Floats grid;

    for (unsigned i = 0; i < dist_grid_.size(); ++i) {

      double dist = dist_grid_[i];
      double cumul = 0;

      for (unsigned j = 1; j < omega_grid_.size(); ++j) {

        double omj = omega_grid_[j];
        double omjm1 = omega_grid_[j - 1];

        double dom = omj - omjm1;

        double pj;
        double pjm1;

        if (prior_type_ == 0) {
          pj = get_biased_element(dist, omj * sigma) / omj;
          pjm1 = get_biased_element(dist, omjm1 * sigma) / omjm1;
        } else {
          pj = get_biased_element(dist, omj) * get_omega_prior(omj, sigma);
          pjm1 =
              get_biased_element(dist, omjm1) * get_omega_prior(omjm1, sigma);
        }
        cumul += (pj + pjm1) / 2.0 * dom;
      }

      // switching cumul to zero between don and doff
      if (dist > doff) {
        cumul = 0.;
      }
      if (dist > don && dist <= doff) {
        cumul *= pow(doff * doff - dist * dist, 2) *
                 (doff * doff + 2. * dist * dist - 3. * don * don) /
                 pow(doff * doff - don * don, 3);
      }

      grid.push_back(cumul);
    }
    grid_.push_back(grid);
  }
}

CrossLinkData::CrossLinkData(Floats dist_grid, Floats omega_grid,
                             Floats sigma_grid, Floats pot_x_grid,
                             Floats pot_value_grid, double don, double doff,
                             int prior_type)
    : base::Object("Data Structure for CrossLinkMSRestraint %1%") {
  prior_type_ = prior_type;
  bias_ = true;
  // this constructor calculates the marginal likelihood using a biasing
  // potential
  // obtained from Free energy calculations.

  // Store dist grid
  dist_grid_ = dist_grid;

  // Store omega grid
  omega_grid_ = omega_grid;

  // Store sigma grid
  sigma_grid_ = sigma_grid;

  // Store the potential x coordinates
  pot_x_grid_ = pot_x_grid;

  // Store the normalized potential
  double cumul = 0;
  for (unsigned j = 1; j < pot_value_grid.size(); ++j) {
    double xj = pot_x_grid_[j];
    double xjm1 = pot_x_grid_[j - 1];
    double dx = xj - xjm1;
    double pj = pot_value_grid[j];
    double pjm1 = pot_value_grid[j - 1];
    cumul += (pj + pjm1) / 2.0 * dx;
  }

  for (unsigned i = 0; i < pot_value_grid.size(); ++i) {
    pot_value_grid_.push_back(pot_value_grid[i] / cumul);
  }

  for (unsigned k = 0; k < sigma_grid_.size(); ++k) {

    double sigma = sigma_grid_[k];

    Floats grid;

    for (unsigned i = 0; i < dist_grid_.size(); ++i) {

      double dist = dist_grid_[i];
      double cumul = 0;

      for (unsigned j = 1; j < omega_grid_.size(); ++j) {

        double omj = omega_grid_[j];
        double omjm1 = omega_grid_[j - 1];

        double dom = omj - omjm1;

        double pj;
        double pjm1;

        if (prior_type_ == 0) {
          pj = get_biased_element(dist, omj * sigma) / omj;
          pjm1 = get_biased_element(dist, omjm1 * sigma) / omjm1;
        } else {
          pj = get_biased_element(dist, omj) * get_omega_prior(omj, sigma);
          pjm1 =
              get_biased_element(dist, omjm1) * get_omega_prior(omjm1, sigma);
        }
        cumul += (pj + pjm1) / 2.0 * dom;
      }

      // switching cumul to zero between don and doff
      if (dist > doff) {
        cumul = 0.;
      }
      if (dist > don && dist <= doff) {
        cumul *= pow(doff * doff - dist * dist, 2) *
                 (doff * doff + 2. * dist * dist - 3. * don * don) /
                 pow(doff * doff - don * don, 3);
      }
      grid.push_back(cumul);
    }
    grid_.push_back(grid);
  }
}

double CrossLinkData::get_omega_prior(double omega, double omega0) const {
  double prior = 0.0;
  if (prior_type_ == 0) {
    // jeffrey's prior
    prior = 1.0 / omega;
  }

  if (prior_type_ == 1) {
    // cauchy-style outlier, as described in Sivia's book
    prior = 2.0 * omega0 / sqrt(IMP::PI) / omega / omega *
            exp(-omega0 * omega0 / omega / omega);
  }
  return prior;
}

Floats CrossLinkData::get_omegas(double sigma, Floats dists) const {
  Floats omegas;
  for (unsigned n = 0; n < dists.size(); ++n) {

    double cumul = 0;
    double cumul2 = 0;

    if (!bias_) {
      for (unsigned j = 1; j < omega_grid_.size(); ++j) {
        double omj = omega_grid_[j];
        double omjm1 = omega_grid_[j - 1];
        double dom = omj - omjm1;

        double pj = get_unbiased_element(dists[n], omj * sigma) / omj;
        double pjm1 = get_unbiased_element(dists[n], omjm1 * sigma) / omjm1;

        double pj2 = get_unbiased_element(dists[n], omj * sigma);
        double pjm12 = get_unbiased_element(dists[n], omjm1 * sigma);

        cumul += (pj + pjm1) / 2.0 * dom;
        cumul2 += (pj2 + pjm12) / 2.0 * dom;
      }
    }

    if (bias_) {
      for (unsigned j = 1; j < omega_grid_.size(); ++j) {
        double omj = omega_grid_[j];
        double omjm1 = omega_grid_[j - 1];
        double dom = omj - omjm1;

        double pj = get_biased_element(dists[n], omj * sigma) / omj;
        double pjm1 = get_biased_element(dists[n], omjm1 * sigma) / omjm1;

        double pj2 = get_biased_element(dists[n], omj * sigma);
        double pjm12 = get_biased_element(dists[n], omjm1 * sigma);

        cumul += (pj + pjm1) / 2.0 * dom;
        cumul2 += (pj2 + pjm12) / 2.0 * dom;
      }
    }

    omegas.push_back(cumul2 / cumul);
  }
  return omegas;
}

Floats CrossLinkData::get_nonmarginal_elements(double sigmai,
                                               Floats dists) const {
  Floats probs;
  if (bias_) {
    for (unsigned n = 0; n < dists.size(); n++) {
      probs.push_back(get_biased_element(dists[n], sigmai));
    }
  }
  if (!bias_) {
    for (unsigned n = 0; n < dists.size(); n++) {
      probs.push_back(get_unbiased_element(dists[n], sigmai));
    }
  }
  return probs;
}

Floats CrossLinkData::get_marginal_elements(double sigma, Floats dists) const {
  Floats probs;
  unsigned is = get_closest(sigma_grid_, sigma);
  for (unsigned n = 0; n < dists.size(); n++) {

    unsigned id = get_closest(dist_grid_, dists[n]);
    probs.push_back(grid_[is][id]);
  }
  return probs;
}

double CrossLinkData::get_marginal_maximum(double sigma) const {

  unsigned is = get_closest(sigma_grid_, sigma);

  double maximum = *(std::max_element(grid_[is].begin(), grid_[is].end()));

  return maximum;
}

double CrossLinkData::get_marginal_maximum() const {
  // this overloaded version is releasing
  // the maximum valua of the function at the smallest
  // value of sigma defined in the grid

  double sigma_min =
      *(std::min_element(sigma_grid_.begin(), sigma_grid_.end()));

  unsigned is = get_closest(sigma_grid_, sigma_min);

  double maximum = *(std::max_element(grid_[is].begin(), grid_[is].end()));

  return maximum;
}

int CrossLinkData::get_closest(std::vector<double> const& vec,
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

double CrossLinkData::get_unbiased_element(double dist, double sigmai) const {
  // Mathematica formula of the probability
  // MargLike[x_, l0_,  s_] := ((E^(-((l0 + x)^2/(2*s^2))) -
  // E^(-((l0 - x)^2/(2*s^2))))*
  //     s)/(Sqrt[2*Pi]*x*l0) + (1/(2*l0))*(Erf[(l0 - x)/(Sqrt[2]*s)] +
  //     Erf[(l0 + x)/(Sqrt[2]*s)])

  double a = (lexp_ + dist) / sqrt(2.0) / sigmai;
  double b = (lexp_ - dist) / sqrt(2.0) / sigmai;
  double a1 = exp(-a * a);
  double b1 = exp(-b * b);
  double d = (a1 - b1) * sigmai / sqrt(2.0 * IMP::PI) / dist;
  double e = (boost::math::erf(b) + boost::math::erf(a)) / 2;

  double f = 1.0 / lexp_ * (d + e);
  // check if f is normal, i.e. f!=nan,inf or negative. Else sets it to zero
  if (!boost::math::isnormal(f)) {
    f = 0.0;
  }
  if (f < 0.0) {
    f = 0.0;
  }

  return f;
}

double CrossLinkData::get_biased_element(double dist, double sigmai) const {

  // double a = exp(-dist*dist/sigmai/sigmai/2.0);
  // double b = sqrt(2.0/IMP::PI);
  // double c = dist/sigmai/sigmai;
  // double d = -1.0/sigmai/sigmai/2.0;
  // double e = 1.0/sigmai/dist;

  double a = 1.0 / sqrt(2.0 * IMP::PI) / sigmai / dist;
  double b = -1.0 / 2.0 / sigmai / sigmai;
  // double c = -2.0*dist/sigmai/sigmai;

  double cumul = 0;
  for (unsigned i = 1; i < pot_x_grid_.size(); ++i) {
    double rti = pot_x_grid_[i];
    double rtim1 = pot_x_grid_[i - 1];
    double drt = rti - rtim1;

    // double kerneli=e*(a*exp(rti*rti*d))*b*rti*std::sinh(rti*c);
    // double kernelim1=e*(a*exp(rtim1*rtim1*d))*b*rtim1*std::sinh(rtim1*c);

    double kerneli = a * rti * (exp(b * (dist - rti) * (dist - rti)) -
                                exp(b * (dist + rti) * (dist + rti)));
    double kernelim1 = a * rtim1 * (exp(b * (dist - rtim1) * (dist - rtim1)) -
                                    exp(b * (dist + rtim1) * (dist + rtim1)));

    double pi = pot_value_grid_[i] * kerneli;
    double pim1 = pot_value_grid_[i - 1] * kernelim1;
    cumul += (pi + pim1) / 2.0 * drt;
  }

  // check if cumul is normal, i.e. cumul!=nan,inf or negative.
  // Else sets it to zero
  if (!boost::math::isnormal(cumul)) {
    cumul = 0.0;
  }
  if (cumul < 0.0) {
    cumul = 0.0;
  }

  return cumul;
}

IMPISD_END_NAMESPACE
