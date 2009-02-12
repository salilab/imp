/**
 *  \file SAXSScore.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSScore.h>
#include <IMP/algebra/utility.h>
#include <iomanip>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

SAXSScore::SAXSScore(FormFactorTable* ff_table,
                     SAXSProfile* exp_saxs_profile) :
ff_table_(ff_table), exp_saxs_profile_(exp_saxs_profile)
{
  offset_ = 0.0;
  mesh_sinc_ = 1000;
}


/*
 ! ----------------------------------------------------------------------
 !------------ scoring function with weighting by error      ------------
 ! calculate SAXS Chi square of experimental data and model
 ! added weight_tilda function w_tilda(q) = w(q) / sigma_exp^2
 ! ----------------------------------------------------------------------
*/
// TODO: define a weight function (w(q) = 1, q^2, or hybrid)
Float SAXSScore::compute_chi_score(const SAXSProfile& model_saxs_profile)
{
  if (exp_saxs_profile_->size() != model_saxs_profile.size()) {
    std::cerr << "Number of profile entries mismatch! "
              << "(exp:" << exp_saxs_profile_->size()
              << ", model:" << model_saxs_profile.size() << ")" << std::endl;
    return -1.0;
  }
  Float sum1=0.0, sum2=0.0;
  chi_square_ = 0.0;

  //! determine the scaling parameter c_
  for (unsigned int k=0; k<model_saxs_profile.size(); k++) {
    //! in the theoretical profile the error equals to 1 (?)
    Float square_error = square(exp_saxs_profile_->get_error(k));
    Float weight_tilda = model_saxs_profile.get_weight(k) / square_error;

    sum1 += weight_tilda * model_saxs_profile.get_intensity(k)
                         * exp_saxs_profile_->get_intensity(k);
    sum2 += weight_tilda * square(model_saxs_profile.get_intensity(k));
  }
  c_ = sum1 / sum2;

  //! compute Chi square
  for (unsigned int k=0; k<model_saxs_profile.size(); k++) {
    //! in the theoretical profile the error equals to 1 (?)
    Float square_error = square(exp_saxs_profile_->get_error(k));
    Float weight_tilda = model_saxs_profile.get_weight(k) / square_error;
    Float delta = exp_saxs_profile_->get_intensity(k) + offset_
                    - c_ * model_saxs_profile.get_intensity(k);

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_saxs_profile_->get_intensity(k)) >= IMP_SAXS_DELTA_LIMIT)
      chi_square_ += weight_tilda * square(delta);
  }
  chi_square_ /= model_saxs_profile.size();

  //! TODO: make this optional
  write_SAXS_fit_file("fitfile.dat", model_saxs_profile);

  return chi_square_;
}


/*
 !----------------------------------------------------------------------
 !------------ Precalculate common array data for faster calculation
 !----------------------------------------------------------------------
*/
int SAXSScore::init(const SAXSProfile& model_saxs_profile)
{
  Float dr = model_saxs_profile.get_pr_resolution();
  //!----- this is the only way to get maximum p(r) distance dynamically.
  unsigned int nr=algebra::round(model_saxs_profile.get_max_pr_distance()/dr)+1;
  int nsinc_qr = algebra::round(
                         exp_saxs_profile_->get_q( exp_saxs_profile_->size()-1 )
                         * nr * dr * mesh_sinc_ ) + 1;
  Float sinc_dense = 1.0 / (Float)mesh_sinc_;

  //!----- setup lookup tables for sinc(qr) and cos(qr)
  std::vector<Float> sinc_qr(nsinc_qr, 0.0), cos_qr(nsinc_qr, 0.0);
  sinc_qr.push_back(1.0);   cos_qr.push_back(1.0);
  for (int i=1; i<nsinc_qr; i++) {
    Float qr = i * sinc_dense;
    sinc_qr[i] = sin(qr) / qr;
    cos_qr[i] = cos(qr);
  }

  std::vector<Float> r(nr, 0.0), one_over_rr(nr, 0.0);
  for (unsigned int i=0; i<nr; i++) {
    r[i] = dr * i;
    one_over_rr[i] = 1.0 / square(r[i]);
  }

  //!----- delta_val_array_ = (sinc(qr) - cos_(qr)) / (r*r)
  delta_val_array_.clear();  delta_val_array_.resize(exp_saxs_profile_->size());
  for (unsigned int iq=0; iq<exp_saxs_profile_->size(); iq++) {
    std::vector<Float> sincval_temp(nr, 0.0);
    for (unsigned int ir=1; ir<nr; ir++) {
      Float qr = exp_saxs_profile_->get_q(iq) * r[ir];
      int ilookup = algebra::round( mesh_sinc_ * qr );
      sincval_temp[ir] = (sinc_qr[ilookup] - cos_qr[ilookup]) * one_over_rr[ir];
    }
    delta_val_array_[iq] = sincval_temp;
  }

  //!----- precalculate difference of intensities and squares of weight
  //!----- delta_i = weight_tilda * (I_exp - c*I_mod)
  //!----- e_q = exp( -0.23 * q*q )
  //!----- delta_i_and_e_q_ = delta_i * e_q
  delta_i_and_e_q_.clear();
  delta_i_and_e_q_.resize(model_saxs_profile.size(), 0.0);
  for (unsigned int iq=0; iq<model_saxs_profile.size(); iq++) {
    Float delta = exp_saxs_profile_->get_intensity(iq)
                  - c_ * model_saxs_profile.get_intensity(iq);
    Float square_error = square(exp_saxs_profile_->get_error(iq));
    Float weight_tilda = model_saxs_profile.get_weight(iq) / square_error;

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_saxs_profile_->get_intensity(iq)) < IMP_SAXS_DELTA_LIMIT)
      delta = 0.0;
    Float delta_i = weight_tilda * delta;
    Float E_q = std::exp( - exp_saxs_profile_-> modulation_function_parameter_
                       * square( exp_saxs_profile_->get_q(iq) ) );
    delta_i_and_e_q_[iq] = delta_i * E_q;
  }
  return 0;
  /* //! TODO: general sine & cosine look-up table - should be somewhere
  sin_.clear();   sin_.resize(mesh_sinc_);
  cos_.clear();   cos_.resize(mesh_sinc_);
  Float two_pi = 2.0*IMP::internal::PI;
  Float one_over_two_pi = 1.0 / two_pi;
  Float coeff = two_pi / mesh_sinc_;
  for (unsigned int i=0; i<mesh_sinc_; i++) {
    Float theta = coeff * i;
    sin_[i] = sin(theta);
    cos_[i] = cos(theta);
  }
 //int pi_lookup = algebra::round(qr * one_over_two_pi * mesh_sinc_)
 //                % mesh_sinc_;
*/
}


/*
 ! ----------------------------------------------------------------------
 !>   compute  derivatives on atom iatom - iatom is NOT part of rigid body
 !!   SCORING function : chi
 !!   This routine does the computations in REAL SPACE!
 !!          => FAST computations!
 !!
 !!   For calculation in real space the quantity Delta(r) is needed to get
 !!   derivatives on atom iatom
 !!
 !!   Delta(r) = f_iatom * sum_i f_i delta(r-r_{i,iatom}) (x_iatom-x_i)
 ! ----------------------------------------------------------------------
*/
std::vector<IMP::algebra::Vector3D> SAXSScore::calculate_chi_real_derivative (
                                       const SAXSProfile& model_saxs_profile,
                                       const std::vector<Particle*>& particles)
{
  std::vector<algebra::Vector3D> chi_derivatives;
  algebra::Vector3D Delta_q, chi_derivative;

  if (exp_saxs_profile_->size() != model_saxs_profile.size()) {
    std::cerr << "Number of profile entries mismatch! "
              << "(exp:" << exp_saxs_profile_->size()
              << ", model:" << model_saxs_profile.size() << ")" << std::endl;
    return chi_derivatives;
  }
  //!----- Pre-calculate common parameters for faster calculation
  init(model_saxs_profile);
  DeltaDistributionFunction delta_dist(model_saxs_profile.get_pr_resolution(),
                                       ff_table_, particles);

  chi_derivatives.resize(particles.size());
  for (unsigned int iatom=0; iatom<particles.size(); iatom++) {
    //!----- Calculate a delta distribution per an atom
    delta_dist.calculate_derivative_distribution(iatom);

    chi_derivative[0] = chi_derivative[1] = chi_derivative[2] = 0.0;
    for (unsigned int iq=0; iq<model_saxs_profile.size(); iq++) {
      Delta_q[0] = Delta_q[1] = Delta_q[2] = 0.0;

      for (unsigned int ir=0; ir<delta_dist.size(); ir++) {
        //!----- delta_dist.distribution = sum_i [f_k(0) * f_i(0) * (x_k - x_i)]
        //!----- delta_val_array_ = (sinc(qr) - cos_(qr)) / (r*r)
        Delta_q += delta_dist.distribution_[ir] * delta_val_array_[iq][ir];
      }
      //!----- delta_i = weight_tilda * (I_exp - c*I_model)
      //!----- e_q = exp( -0.23 * q*q )
      chi_derivative += Delta_q * delta_i_and_e_q_[iq];
    }
    chi_derivatives[iatom] = 4.0 * c_ * chi_derivative;
  }
  return chi_derivatives;
}


//!----------------------------------------------------------------------
//! Write SAXS Fit data file
//!----------------------------------------------------------------------
void SAXSScore::write_SAXS_fit_file(const std::string& file_name,
                                    const SAXSProfile& model_saxs_profile)
const {
  std::ofstream out_file(file_name.c_str());

  if (!out_file) {
    std::cerr << "Can't open file " << file_name << std::endl;
    exit(1);
  }

  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << exp_saxs_profile_->size()
           << ", q_min = " << exp_saxs_profile_->get_min_q()
           << ", q_max = " << exp_saxs_profile_->get_max_q();
  out_file << ", delta_q = " << exp_saxs_profile_->get_delta_q() << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file << "# offset = " << offset_ << ", scaling c = " << c_
           << ", Chi-square = " << chi_square_ << std::endl;
  out_file << "#       q             exp_intensity    model_intensity"
           << std::endl;

  // Main data
  for (unsigned int i = 0; i < exp_saxs_profile_->size(); i++) {
    out_file.setf(std::ios::left);
    out_file.width(20);
    out_file.fill('0');
    out_file << std::setprecision(15) << exp_saxs_profile_->get_q(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << std::setprecision(15)
             << exp_saxs_profile_->get_intensity(i) + offset_ << " ";

    out_file.setf(std::ios::left);
    out_file.width(16);
    out_file.fill('0');
    out_file << std::setprecision(15) << model_saxs_profile.get_intensity(i)*c_
             << std::endl;
  }
  out_file.close();
}

IMPSAXS_END_NAMESPACE
