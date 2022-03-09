/**
 *  \file BayesEM3D.cpp
 *  \brief Compute a Bayesian formalism score and its derivatives
 *  to assess degree of match between a tested model and a density object.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Mass.h>
#include <IMP/core/utility.h>
#include <IMP/em/BayesEM3D.h>
#include <math.h>
#include <boost/math/special_functions/sign.hpp>
#include <boost/math/special_functions/log1p.hpp>
#include <boost/unordered_set.hpp>

IMPEM_BEGIN_NAMESPACE

namespace {
  //! Quick function for extracting first element of pair to a vector of floats
  template <typename M, typename V>
  void bayesem3d_map_first_to_floats(const M &m, V &v) {
    for (typename M::const_iterator it = m.begin(); it != m.end(); ++it)
      v.push_back(it->first);
  }

  //! Quick function for extracting second element of pair to a vector of floats
  template <typename M, typename V>
  void bayesem3d_map_second_to_floats(const M &m, V &v) {
    for (typename M::const_iterator it = m.begin(); it != m.end(); ++it)
      v.push_back(it->second);
  }
}  // anonymous namespace


std::map<double, int> bayesem3d_get_distinct_and_counts(DensityMap *em) {
  const DensityHeader *em_header = em->get_header();
  const double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();

  std::map<double, int> values_and_counts;

  for (long f = 0; f < nvox; ++f)
    values_and_counts[(int)(em_data[f] * 1000.)]++;

  return values_and_counts;
}

Floats bayesem3d_get_cumulative_sum(DensityMap *em) {
  const DensityHeader *em_header = em->get_header();
  long nvox = em_header->get_number_of_voxels();

  std::map<double, int> values_and_counts =
      bayesem3d_get_distinct_and_counts(em);

  Floats cumulative_sum;
  std::map<double, int>::iterator it;
  double previous = 0.0;
  double current = 0.0;

  for (it = values_and_counts.begin(); it != values_and_counts.end(); ++it) {
    current = (previous + it->second);
    cumulative_sum.push_back(current / (1.0 * nvox));
    previous = current;
  }
  return cumulative_sum;
}

std::map<double, double> bayesem3d_get_cumulative_sum_with_reference(
    DensityMap *em) {
  std::map<double, int> values_and_counts =
      bayesem3d_get_distinct_and_counts(em);

  std::map<double, double> cumulative_sum;
  std::map<double, int>::iterator it;
  double previous = 0.0;
  double current = 0.0;

  for (it = values_and_counts.begin(); it != values_and_counts.end(); ++it) {
    current = (previous + it->second);
    cumulative_sum[it->first] = current;
    previous = current;
  }
  return cumulative_sum;
}

double bayesem3d_linear_interpolate(const Floats &x_data, const Floats &y_data,
                                    double x, bool extrapolate) {
  int size = x_data.size();

  if (size < 2) IMP_ERROR("The box size cannot be smaller than 2")

  int i = 0;                    // find left end of interval for interpolation
  if (x >= x_data[size - 2]) {  // special case: beyond right end
    i = size - 2;
  } else {
    while (x > x_data[i + 1] && i < size - 2) i++;
  }
  double xL = x_data[i], yL = y_data[i], xR = x_data[i + 1],
         yR = y_data[i + 1];  // points on either side (unless beyond ends)

  if (!extrapolate) {  // if beyond ends of array and not extrapolating
    if (x < xL) {
      yR = yL;
    }
    if (x > xR) {
      yL = yR;
    }
  }

  double dydx = (yR - yL) / (xR - xL);
  return yL + dydx * (x - xL);
}

FloatPair bayesem3d_get_logabssumexp(double x, double y, double sx, double sy) {
  if (IMP::isinf(x) || IMP::isinf(y)) {
    if (x >= y) {
      return FloatPair(x, sx);
    }

    else if (x < y) {
      return FloatPair(y, sy);
    }

    else if (x > 0 && sx != sy) {
      return FloatPair(std::numeric_limits<double>::quiet_NaN(),
                       std::numeric_limits<double>::quiet_NaN());
    }
  }

  if (IMP::is_nan(x) || IMP::is_nan(y)) {
    return FloatPair(std::numeric_limits<double>::quiet_NaN(),
                     std::numeric_limits<double>::quiet_NaN());
  }

  if (x > y) {
    return FloatPair(x + boost::math::log1p(sx * sy * exp(y - x)), sx);
  } else {
    return FloatPair(y + boost::math::log1p(sx * sy * exp(x - y)), sy);
  }
}

FloatPair bayesem3d_get_logabssumprodexp(double x, double y, double wx,
                                         double wy) {
  double sx = boost::math::copysign(1.0, wx);
  double sy = boost::math::copysign(1.0, wy);

  return bayesem3d_get_logabssumexp(x + log(wx * sx), y + log(wy * sy), sx, sy);
}

double bayesem3d_get_logsumexp(double x, double y) {
  if (IMP::isinf(x) || IMP::isinf(y)) {
    return std::max(x, y);
  }

  if (IMP::is_nan(x) || IMP::is_nan(y)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  if (x > y) {
    return x + boost::math::log1p(exp(y - x));
  } else {
    return y + boost::math::log1p(exp(x - y));
  }
}

double bayesem3d_get_logsumprodexp(double x, double y, double wx, double wy) {
  return bayesem3d_get_logsumexp(x + log(wx), y + log(wy));
}

std::vector<double> bayesem3d_get_value(Particle *p,
                                        const algebra::Vector3D &pt,
                                        double mass_ii,
                                        const IMP::em::KernelParameters &kps) {
  std::vector<double> results;
  algebra::Vector3D cs = core::XYZ(p).get_coordinates();

  double rsq = (cs - pt).get_squared_magnitude();
  double norm = (kps.get_rnormfac() * mass_ii);

  double tmp = -rsq * kps.get_inv_rsigsq();
  results.push_back(tmp);
  results.push_back(norm);

  results.push_back(-2. * (cs[0] - pt[0]) * kps.get_inv_rsigsq());
  results.push_back(-2. * (cs[1] - pt[1]) * kps.get_inv_rsigsq());
  results.push_back(-2. * (cs[2] - pt[2]) * kps.get_inv_rsigsq());

  return results;
}

std::vector<double> bayesem3d_get_value_no_deriv(
    Particle *p, const algebra::Vector3D &pt, double mass_ii,
    const IMP::em::KernelParameters &kps) {
  std::vector<double> results;
  algebra::Vector3D cs = core::XYZ(p).get_coordinates();

  double rsq = (cs - pt).get_squared_magnitude();
  double norm = (kps.get_rnormfac() * mass_ii);

  double tmp = -rsq * kps.get_inv_rsigsq();
  results.push_back(tmp);
  results.push_back(norm);

  return results;
}

FloatPair bayesem3d_get_em_density_squared(DensityMap *em,
                                           long number_of_voxels) {
  FloatPair results;

  const double *em_data = em->get_data();
  double em_square = 0;
  double scale = 0;

  for (long jj = 0; jj < number_of_voxels; ++jj) {
    em_square += em_data[jj] * em_data[jj];
    scale += em_data[jj];
  }
  results.first = scale;
  results.second = em_square;
  return results;
}

double bayesem3d_get_cross_correlation_coefficient(const DensityMap *em1,
                                                   const DensityMap *em2) {
  const DensityHeader *em1_header = em1->get_header();
  const double *em1_data = em1->get_data();

  const DensityHeader *em2_header = em2->get_header();
  const double *em2_data = em2->get_data();

  long nvox1 = em1_header->get_number_of_voxels();
  long nvox2 = em2_header->get_number_of_voxels();

  if (nvox1 != nvox2)
    IMP_THROW("Can't compute correlation for different size maps.",
              ValueException);

  double em1_mean = 0.;
  double em2_mean = 0.;

  double em1_sqr = 0.;
  double em2_sqr = 0.;

  double cross_term = 0.;

  for (long i = 0; i < nvox1; ++i) {
    if (em1_data[i] > 0 && em2_data[i] > 0) {
      em1_mean += em1_data[i];
      em2_mean += em2_data[i];

      em1_sqr += IMP::square(em1_data[i]);
      em2_sqr += IMP::square(em2_data[i]);

      cross_term += em1_data[i] * em2_data[i];
    }
  }

  em1_mean /= nvox1;
  em2_mean /= nvox2;

  em1_sqr /= nvox1;
  em2_sqr /= nvox2;

  double sig1 = em1_sqr - IMP::square(em1_mean);
  sig1 = (sig1 < 0. ? 0. : sqrt(sig1));

  double sig2 = em2_sqr - IMP::square(em2_mean);
  sig2 = (sig2 < 0. ? 0. : sqrt(sig2));

  double ccc = cross_term - nvox1 * em1_mean * em2_mean;
  ccc = std::max(0., ccc);

  ccc /= nvox1 * sig1 * sig2;
  return ccc;
}

DensityMap *bayesem3d_get_density_from_particle(DensityMap *em,
                                                const IMP::ParticlesTemp &ps,
                                                double resolution,
                                                const double window_size) {
  const DensityHeader *em_header = em->get_header();
  IMP::algebra::BoundingBox3D density_bb = get_bounding_box(em);
  IMP::em::KernelParameters kps(resolution);

  core::XYZRs xyzr(ps);

  IMP_NEW(em::DensityMap, ret, (*(em->get_header())));
  ret->reset_data(0.);
  double *vals = ret->get_data();

  int ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  int nxny = em_header->get_nx() * em_header->get_ny();
  int znxny;

  boost::unordered_set<int> visited_voxels;

  for (unsigned int ii = 0; ii < xyzr.size(); ii++) {
    double px = xyzr[ii].get_x();
    double py = xyzr[ii].get_y();
    double pz = xyzr[ii].get_z();
    double pr = xyzr[ii].get_radius();
    double mass_ii = IMP::atom::Mass(ps[ii]).get_mass();
    double local_box_size = pr + window_size * kps.get_rkdist();

    calc_local_bounding_box(em, px, py, pz, local_box_size, iminx, iminy, iminz,
                            imaxx, imaxy, imaxz);

    for (ivoxz = iminz; ivoxz <= imaxz; ivoxz++) {
      znxny = ivoxz * nxny;

      for (ivoxy = iminy; ivoxy <= imaxy; ivoxy++) {
        ivox = znxny + ivoxy * em_header->get_nx() + iminx;

        for (ivoxx = iminx; ivoxx <= imaxx; ivoxx++) {
          algebra::Vector3D cur(em->get_location_by_voxel(ivox));
          std::vector<double> value =
              bayesem3d_get_value_no_deriv(xyzr[ii], cur, mass_ii, kps);

          vals[ivox] =
              bayesem3d_get_logsumprodexp(vals[ivox], value[0], 1., value[1]);

          ivox++;
          visited_voxels.insert(ivox);
        }
      }
    }
  }

  for (boost::unordered_set<int>::iterator vv = visited_voxels.begin();
       vv != visited_voxels.end(); ++vv) {
    vals[*vv] = exp(vals[*vv]) - 1.;
  }

  return ret.release();
}

std::pair<double, algebra::Vector3Ds> bayesem3d_get_score_and_derivative(
    DensityMap *em, const IMP::ParticlesTemp &ps, double resolution,
    double sigma, const double window_size) {
  const DensityHeader *em_header = em->get_header();
  const double *em_data = em->get_data();

  IMP::algebra::BoundingBox3D density_bb = get_bounding_box(em);
  IMP::em::KernelParameters kps(resolution);

  core::XYZRs xyzr(ps);

  // Set up score and derivative variables
  FloatPair score_lnr(0., -1.);
  FloatPair score_lcc(0., -1.);
  FloatPair score_sqr(0., -1.);

  algebra::Vector3Ds dv_out;
  dv_out.insert(dv_out.end(), ps.size(), algebra::Vector3D(0., 0., 0.));

  // indices and temporaries
  FloatPair dsdx_lnr(0., -1.);
  FloatPair dsdy_lnr(0., -1.);
  FloatPair dsdz_lnr(0., -1.);

  FloatPair dsdx_lcc(0., -1.);
  FloatPair dsdy_lcc(0., -1.);
  FloatPair dsdz_lcc(0., -1.);

  int ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  int nxny = em_header->get_nx() * em_header->get_ny();
  int znxny;

  // IMP_NEW(DensityMap, retct, ());
  IMP::Pointer<DensityMap> retct =
      bayesem3d_get_density_from_particle(em, ps, resolution);
  double *modelct = retct->get_data();

  // Compute the sum and the sum^2 of the experimental EM denisty map
  // FloatPair em_sums = get_square_em_density(em, nvox);

  for (unsigned int ii = 0; ii < xyzr.size(); ii++) {
    double px = xyzr[ii].get_x();
    double py = xyzr[ii].get_y();
    double pz = xyzr[ii].get_z();
    double pr = xyzr[ii].get_radius();
    double mass_ii = IMP::atom::Mass(ps[ii]).get_mass();
    double local_box_size = pr + window_size * kps.get_rkdist();

    // Initialize derivative to 0 for each particles
    dsdx_lnr = FloatPair(0., -1.);
    dsdy_lnr = FloatPair(0., -1.);
    dsdz_lnr = FloatPair(0., -1.);

    dsdx_lcc = FloatPair(0., -1.);
    dsdy_lcc = FloatPair(0., -1.);
    dsdz_lcc = FloatPair(0., -1.);

    // Get Voxel box
    calc_local_bounding_box(em, px, py, pz, local_box_size, iminx, iminy, iminz,
                            imaxx, imaxy, imaxz);

    for (ivoxz = iminz; ivoxz <= imaxz; ivoxz++) {
      znxny = ivoxz * nxny;

      for (ivoxy = iminy; ivoxy <= imaxy; ivoxy++) {
        ivox = znxny + ivoxy * em_header->get_nx() + iminx;

        for (ivoxx = iminx; ivoxx <= imaxx; ivoxx++) {
          algebra::Vector3D cur(em->get_location_by_voxel(ivox));
          std::vector<double> value =
              bayesem3d_get_value(xyzr[ii], cur, mass_ii, kps);

          double intensity = value[1] * exp(value[0]);

          // score when exponent is linear
          score_lnr = bayesem3d_get_logabssumprodexp(
              score_lnr.first, value[0], score_lnr.second,
              (modelct[ivox] - intensity) * value[1]);

          // score when cross term with em
          score_lcc = bayesem3d_get_logabssumprodexp(
              score_lcc.first, value[0], score_lcc.second,
              -2. * em_data[ivox] * value[1]);

          // score when exponent is squared
          score_sqr = bayesem3d_get_logabssumprodexp(
              score_sqr.first, 2. * value[0], score_sqr.second,
              value[1] * value[1]);

          // derivative when exponent is linear
          dsdx_lnr = bayesem3d_get_logabssumprodexp(
              dsdx_lnr.first, value[0], dsdx_lnr.second,
              2. * modelct[ivox] * value[1] * value[2]);

          dsdy_lnr = bayesem3d_get_logabssumprodexp(
              dsdy_lnr.first, value[0], dsdy_lnr.second,
              2. * modelct[ivox] * value[1] * value[3]);

          dsdz_lnr = bayesem3d_get_logabssumprodexp(
              dsdz_lnr.first, value[0], dsdz_lnr.second,
              2. * modelct[ivox] * value[1] * value[4]);

          // Derivative of the cross term with em
          dsdx_lcc = bayesem3d_get_logabssumprodexp(
              dsdx_lcc.first, value[0], dsdx_lcc.second,
              -2. * em_data[ivox] * value[1] * value[2]);

          dsdy_lcc = bayesem3d_get_logabssumprodexp(
              dsdy_lcc.first, value[0], dsdy_lcc.second,
              -2. * em_data[ivox] * value[1] * value[3]);

          dsdz_lcc = bayesem3d_get_logabssumprodexp(
              dsdz_lcc.first, value[0], dsdz_lcc.second,
              -2. * em_data[ivox] * value[1] * value[4]);

          // If we want to write the generated density from particles.
          ivox++;
        }
      }
    }

    dv_out[ii][0] = dsdx_lnr.second * exp(dsdx_lnr.first) + 1 +
                    dsdx_lcc.second * exp(dsdx_lcc.first) + 1;

    dv_out[ii][1] = dsdy_lnr.second * exp(dsdy_lnr.first) + 1 +
                    dsdy_lcc.second * exp(dsdy_lcc.first) + 1;

    dv_out[ii][2] = dsdz_lnr.second * exp(dsdz_lnr.first) + 1 +
                    dsdz_lcc.second * exp(dsdz_lcc.first) + 1;

    dv_out[ii][0] /= (2 * sigma);
    dv_out[ii][1] /= (2 * sigma);
    dv_out[ii][2] /= (2 * sigma);
  }

  std::pair<double, algebra::Vector3Ds> results;
  results.first = score_lnr.second * exp(score_lnr.first) + 1 +
                  score_lcc.second * exp(score_lcc.first) + 1 +
                  score_sqr.second * exp(score_sqr.first) + 1;

  results.first /= (2 * sigma);
  // results.first += nvox * log(sigma);
  results.second = dv_out;
  // IMP::em::write_map(em, "cpp_normalized.mrc", new
  // IMP::em::MRCReaderWriter());

  return results;
}

void bayesem3d_get_normalized_intensities(DensityMap *em,
                                          const IMP::ParticlesTemp &ps,
                                          double resolution) {
  const DensityHeader *em_header = em->get_header();
  double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();

  IMP::Pointer<DensityMap> em_m =
      bayesem3d_get_density_from_particle(em, ps, resolution);

  // std::map<double, int> values_and_counts_model =
  // get_distinct_and_counts(em_m, nvox); std::map<double, int>
  // values_and_counts_em = get_distinct_and_counts(em, nvox);

  std::map<double, double> cumulative_sum_model_with_ref =
      bayesem3d_get_cumulative_sum_with_reference(em_m);
  std::map<double, double> cumulative_sum_em_with_ref =
      bayesem3d_get_cumulative_sum_with_reference(em);

  Floats model_sorted_unique_intensities;
  Floats cumulative_sum_em;
  Floats cumulative_sum_model;

  bayesem3d_map_first_to_floats(cumulative_sum_model_with_ref,
                                model_sorted_unique_intensities);
  bayesem3d_map_second_to_floats(cumulative_sum_em_with_ref, cumulative_sum_em);
  bayesem3d_map_second_to_floats(cumulative_sum_model_with_ref,
                                 cumulative_sum_model);

  double value = 0.0;
  double interp = 0.0;

  for (long ivox = 0; ivox < nvox; ++ivox) {
    value = cumulative_sum_em_with_ref[(int)(em_data[ivox] * 1000.)];
    interp = bayesem3d_linear_interpolate(
        cumulative_sum_model, model_sorted_unique_intensities, value, false);

    em_data[ivox] = interp / 1000.;
  }

  // IMP::em::write_map(em, "cpp_normalized.mrc", new
  // IMP::em::MRCReaderWriter());
}

IMPEM_END_NAMESPACE
