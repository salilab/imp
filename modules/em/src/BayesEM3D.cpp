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

IMPEM_BEGIN_NAMESPACE

const double window_size = 3.0;

BayesEM3D::BayesEM3D() : Object("BayesEM3D%1%") {}

std::map<double, int> BayesEM3D::get_distinct_and_counts(DensityMap *em) {

  const DensityHeader *em_header = em->get_header();
  const double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();

  std::map<double, int> values_and_counts;

  for (long f = 0; f < nvox; ++f)
    values_and_counts[(int)(em_data[f] * 1000.)]++;

  return values_and_counts;
}

Floats BayesEM3D::get_cumulative_sum(DensityMap *em) {

  const DensityHeader *em_header = em->get_header();
  long nvox = em_header->get_number_of_voxels();

  std::map<double, int> values_and_counts = get_distinct_and_counts(em);

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

std::map<double, double>
BayesEM3D::get_cumulative_sum_with_reference(DensityMap *em) {

  const DensityHeader *em_header = em->get_header();
  // long nvox = em_header->get_number_of_voxels();
  std::map<double, int> values_and_counts = get_distinct_and_counts(em);

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

double BayesEM3D::linear_interpolate(const Floats &xData, const Floats &yData,
 double x, bool extrapolate) {

  int size = xData.size();

  int i = 0;                  // find left end of interval for interpolation
  if (x >= xData[size - 2]) { // special case: beyond right end
    i = size - 2;
  } else {
    while (x > xData[i + 1])
      i++;
  }
  double xL = xData[i], yL = yData[i], xR = xData[i + 1],
         yR = yData[i + 1]; // points on either side (unless beyond ends)

  if (!extrapolate) { // if beyond ends of array and not extrapolating
    if (x < xL)
      yR = yL;
    if (x > xR)
      yL = yR;
  }

  double dydx = (yR - yL) / (xR - xL);
  return yL + dydx * (x - xL);
}

FloatPair BayesEM3D::logabssumexp(double x, double y, double sx, double sy) {
  if (IMP::isinf(abs(x)) or IMP::isinf(abs(y))) {

    if (x >= y)
      return FloatPair(x, sx);

    else if (x < y)
      return FloatPair(y, sy);

    else if (x > 0 and sx != sy)
      return FloatPair(std::numeric_limits<double>::quiet_NaN(),
       std::numeric_limits<double>::quiet_NaN());
  }

  if (IMP::is_nan(x) || IMP::is_nan(y))
    return FloatPair(std::numeric_limits<double>::quiet_NaN(),
     std::numeric_limits<double>::quiet_NaN());

  if (x > y)
    return FloatPair(x + log1p(sx * sy * exp(y - x)), sx);
  else
    return FloatPair(y + log1p(sx * sy * exp(x - y)), sy);
}

FloatPair BayesEM3D::logabssumprodexp(double x, double y, double wx,
  double wy) {
  double sx = std::copysign(1, wx);
  double sy = std::copysign(1, wy);

  return logabssumexp(x + log(wx * sx), y + log(wy * sy), sx, sy);
}

double BayesEM3D::logsumexp(double x, double y) {

  if (IMP::isinf(abs(x)) or IMP::isinf(abs(y))) {

    if (x >= y)
      return x;

    else if (x < y)
      return y;
  }

  if (IMP::is_nan(x) || IMP::is_nan(y))
    return std::numeric_limits<double>::quiet_NaN();

  if (x > y)
    return x + log1p(exp(y - x));
  else
    return y + log1p(exp(x - y));
}

double BayesEM3D::logsumprodexp(double x, double y, 
  double wx,double wy) 
{
  return logsumexp(x + log(wx), y + log(wy));
}

std::vector<double> BayesEM3D::get_value(Particle *p,
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

  results.push_back( ( -2. * (cs[0] - pt[0]) * kps.get_inv_rsigsq() ) );
  results.push_back( ( -2. * (cs[1] - pt[1]) * kps.get_inv_rsigsq() ) );
  results.push_back( ( -2. * (cs[2] - pt[2]) * kps.get_inv_rsigsq() ) );

  return results;
}

std::vector<double>
BayesEM3D::get_value_no_deriv(Particle *p, 
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

  return results;
}

FloatPair BayesEM3D::get_square_em_density(DensityMap *em,
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

DensityMap *BayesEM3D::get_density_from_particle(DensityMap *em,
 const IMP::ParticlesTemp &ps,
 double resolution) {
  const DensityHeader *em_header = em->get_header();
  const double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();
  IMP::algebra::BoundingBox3D density_bb = get_bounding_box(em);
  IMP::em::KernelParameters kps(resolution);

  core::XYZRs xyzr(ps);
  
  IMP_NEW(em::DensityMap, ret, (*(em->get_header())));
  ret->reset_data(0.);  
  double *vals = ret->get_data();

  int ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  int nxny = em_header->get_nx() * em_header->get_ny();
  int znxny;

  std::unordered_set<int> visited_voxels;

  for (unsigned int ii = 0; ii < xyzr.size(); ii++) {

    double px = xyzr[ii].get_x();
    double py = xyzr[ii].get_y();
    double pz = xyzr[ii].get_z();
    double pr = xyzr[ii].get_radius();
    double mass_ii = IMP::atom::Mass(ps[ii]).get_mass();
    double local_box_size = pr + window_size * kps.get_rkdist();

    calc_local_bounding_box(em, px, py, pz, local_box_size, iminx, iminy,
      iminz, imaxx, imaxy, imaxz);

    for (ivoxz = iminz; ivoxz <= imaxz; ivoxz++) {
      znxny = ivoxz * nxny;

      for (ivoxy = iminy; ivoxy <= imaxy; ivoxy++) {
        ivox = znxny + ivoxy * em_header->get_nx() + iminx;

        for (ivoxx = iminx; ivoxx <= imaxx; ivoxx++) {

          algebra::Vector3D cur(em->get_location_by_voxel(ivox));
          std::vector<double> value =
          get_value_no_deriv(xyzr[ii], cur, mass_ii, kps);

          vals[ivox] = logsumprodexp(vals[ivox], value[0],
            1., value[1]);

          ivox++;
          visited_voxels.insert(ivox);
        }
      }
    }
  }

  for (int v : visited_voxels){
    vals[v] = exp(vals[v]) - 1.;
  }

  return ret.release();
}

std::pair<double, algebra::Vector3Ds> BayesEM3D::calc_score_and_derivative(
  DensityMap *em, const IMP::ParticlesTemp &ps, double resolution,
  double sigma) {

  const DensityHeader *em_header = em->get_header();
  const double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();
  IMP::algebra::BoundingBox3D density_bb = get_bounding_box(em);

  IMP::em::KernelParameters kps(resolution);

  core::XYZRs xyzr(ps);
  IMP_NEW(em::DensityMap, ret, (*(em->get_header())));
  ret->reset_data(0.);
  double *model = ret->get_data();

  // Set up score and derivative variables
  FloatPair score_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair score_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair score_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

  algebra::Vector3Ds dv_out;
  dv_out.insert(dv_out.end(), ps.size(), algebra::Vector3D(0., 0., 0.));

  // indices and temporaries
  FloatPair dsdx_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdy_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdz_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

  FloatPair dsdx_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdy_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdz_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

  FloatPair dsdx_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdy_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
  FloatPair dsdz_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

  int ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  int nxny = em_header->get_nx() * em_header->get_ny();
  int znxny;

  //IMP_NEW(DensityMap, retct, ());
  IMP::Pointer<DensityMap> retct = get_density_from_particle(em, ps, resolution);
  double *modelct = retct->get_data();

  // Compute the sum and the sum^2 of the experimental EM denisty map
  //FloatPair em_sums = get_square_em_density(em, nvox);

  for (unsigned int ii = 0; ii < xyzr.size(); ii++) {

    double px = xyzr[ii].get_x();
    double py = xyzr[ii].get_y();
    double pz = xyzr[ii].get_z();
    double pr = xyzr[ii].get_radius();
    double mass_ii = IMP::atom::Mass(ps[ii]).get_mass();
    double local_box_size = pr + window_size * kps.get_rkdist();

    // Initialize derivative to 0 for each particles
    dsdx_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdy_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdz_lnr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

    dsdx_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdy_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdz_lcc = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

    dsdx_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdy_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);
    dsdz_sqr = FloatPair(-std::numeric_limits<double>::infinity(), -1.);

    // Get Voxel box
    calc_local_bounding_box(em, px, py, pz, local_box_size, iminx,
      iminy, iminz, imaxx, imaxy, imaxz);

    for (ivoxz = iminz; ivoxz <= imaxz; ivoxz++) {
      znxny = ivoxz * nxny;

      for (ivoxy = iminy; ivoxy <= imaxy; ivoxy++) {
        ivox = znxny + ivoxy * em_header->get_nx() + iminx;

        for (ivoxx = iminx; ivoxx <= imaxx; ivoxx++) {

          algebra::Vector3D cur(em->get_location_by_voxel(ivox));
          std::vector<double> value = get_value(xyzr[ii], cur, mass_ii, kps);
          
          double intensity = value[1] * exp(value[0]);
          double constant_term = (modelct[ivox] - intensity);

          // score when exponent is linear
          score_lnr = logabssumprodexp(
            score_lnr.first, value[0], 
            score_lnr.second, 
            (modelct[ivox] - intensity) * value[1]);
          
          // score when cross term with em
          score_lcc = logabssumprodexp(
            score_lcc.first, value[0], 
            score_lcc.second, 
            -2. * em_data[ivox] * value[1]);

          // score when exponent is squared
          score_sqr = logabssumprodexp(
            score_sqr.first, 2. * value[0], 
            score_sqr.second, 
            value[1] * value[1]);

          // derivative when exponent is linear
          dsdx_lnr = logabssumprodexp(
            dsdx_lnr.first, value[0], 
            dsdx_lnr.second,
            2. * (modelct[ivox] - intensity) * value[1] * value[2]);

          dsdy_lnr = logabssumprodexp(
            dsdy_lnr.first, value[0], 
            dsdy_lnr.second,
            2. * (modelct[ivox] - intensity) * value[1] * value[3]);

          dsdz_lnr = logabssumprodexp(
            dsdz_lnr.first, value[0], 
            dsdz_lnr.second,
            2. * (modelct[ivox] - intensity) * value[1] * value[4]);

          // Derivative of the cross term with em
          dsdx_lcc = logabssumprodexp(
            dsdx_lcc.first, value[0], 
            dsdx_lcc.second,
            -2. * em_data[ivox] * value[1] * value[2]);

          dsdy_lcc = logabssumprodexp(
            dsdy_lcc.first, value[0], 
            dsdy_lcc.second,
            -2. * em_data[ivox] * value[1] * value[3]);

          dsdz_lcc = logabssumprodexp(
            dsdz_lcc.first, value[0], 
            dsdz_lcc.second,
            -2. * em_data[ivox] * value[1] * value[4]);

          // Derivative of the (Sum)^2 term
          dsdx_sqr = logabssumprodexp(
            dsdx_sqr.first, 2. * value[0], 
            dsdx_sqr.second,
            value[1] * value[1] * 2. * value[2]);

          dsdy_sqr = logabssumprodexp(
            dsdy_sqr.first, 2. * value[0], 
            dsdy_sqr.second,
            value[1] * value[1] * 2. * value[3]);

          dsdz_sqr = logabssumprodexp(
            dsdz_sqr.first, 2. * value[0], 
            dsdz_sqr.second,
            value[1] * value[1] * 2. * value[4]);

          // If we want to write the generated density from particles.
          model[ivox] += intensity;
          ivox++;
        }
      }
    }

    dv_out[ii][0] = dsdx_lnr.second * exp(dsdx_lnr.first) +
      dsdx_lcc.second * exp(dsdx_lcc.first) +
      dsdx_sqr.second * exp(dsdx_sqr.first);
    
    dv_out[ii][1] = dsdy_lnr.second * exp(dsdy_lnr.first) +
      dsdy_lcc.second * exp(dsdy_lcc.first) +
      dsdy_sqr.second * exp(dsdy_sqr.first);
    
    dv_out[ii][2] = dsdz_lnr.second * exp(dsdz_lnr.first) +
      dsdz_lcc.second * exp(dsdz_lcc.first) +
      dsdz_sqr.second * exp(dsdz_sqr.first);

    dv_out[ii][0] /= (2 * sigma);
    dv_out[ii][1] /= (2 * sigma);
    dv_out[ii][2] /= (2 * sigma);
    
  }
  
  std::pair<double, algebra::Vector3Ds> results;
  results.first = score_lnr.second * exp(score_lnr.first) +
      score_lcc.second * exp(score_lcc.first) +
      score_sqr.second * exp(score_sqr.first);

  results.first /= (2 * sigma);
  // results.first += nvox * log(sigma);
  results.second = dv_out;
  return results;
}

template <typename M, typename V>
void BayesEM3D::MapFirst2Floats(const M &m, V &v) {
  for (typename M::const_iterator it = m.begin(); it != m.end(); ++it)
    v.push_back(it->first);
}

template <typename M, typename V>
void BayesEM3D::MapSecond2Floats(const M &m, V &v) {
  for (typename M::const_iterator it = m.begin(); it != m.end(); ++it)
    v.push_back(it->second);
}

void BayesEM3D::get_normalized_intensities(DensityMap *em,
 const IMP::ParticlesTemp &ps,
 double resolution) {

  const DensityHeader *em_header = em->get_header();
  double *em_data = em->get_data();
  long nvox = em_header->get_number_of_voxels();

  IMP::Pointer<DensityMap> em_m = get_density_from_particle(em, ps, resolution);
  const double *em_model = em_m->get_data();

  // std::map<double, int> values_and_counts_model =
  // get_distinct_and_counts(em_m, nvox); std::map<double, int>
  // values_and_counts_em = get_distinct_and_counts(em, nvox);

  std::map<double, double> cumulative_sum_model_with_ref =
  get_cumulative_sum_with_reference(em_m);
  std::map<double, double> cumulative_sum_em_with_ref =
  get_cumulative_sum_with_reference(em);

  Floats model_sorted_unique_intensities;
  Floats cumulative_sum_em;
  Floats cumulative_sum_model;

  MapFirst2Floats(cumulative_sum_model_with_ref,
    model_sorted_unique_intensities);
  MapSecond2Floats(cumulative_sum_em_with_ref, cumulative_sum_em);
  MapSecond2Floats(cumulative_sum_model_with_ref, cumulative_sum_model);

  double value = 0.0;
  double interp = 0.0;

  for (long ivox = 0; ivox < nvox; ++ivox) {
    value = cumulative_sum_em_with_ref[(int)(em_data[ivox] * 1000.)];
    interp = linear_interpolate(cumulative_sum_model,
      model_sorted_unique_intensities, value, false);

    em_data[ivox] = interp / 1000.;
  }

  // IMP::em::write_map(em, "cpp_normalized.mrc", new
  // IMP::em::MRCReaderWriter());
}

IMPEM_END_NAMESPACE
