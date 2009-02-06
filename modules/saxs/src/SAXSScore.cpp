/**
 *  \file SAXSScore.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/saxs/SAXSScore.h>
#include <IMP/algebra/math_macros.h>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

SAXSScore::SAXSScore(FormFactorTable* ff_table,
                     SAXSProfile* exp_saxs_profile,
                     const SAXSProfile& model_saxs_profile,
                     const std::vector<Particle*>& particles) :
  ff_table_(ff_table), exp_saxs_profile_(exp_saxs_profile)
{
  init(model_saxs_profile, particles);
}


/*
 !----------------------------------------------------------------------
 !------------ Precalculate common array data for faster calculation
 !----------------------------------------------------------------------
*/
int SAXSScore::init(const SAXSProfile& model_saxs_profile,
                    const std::vector<Particle*>& particles)
{
  //!------ setup lookup tables for sinc and cosine function
  mesh_sinc_ = 1000;
  offset_ = 0.0;
  Float dr = model_saxs_profile.get_pr_resolution();
  nr_ = round( model_saxs_profile.get_max_pr_distance() / dr ) + 1;
  int nsinc = round( exp_saxs_profile_->get_q( exp_saxs_profile_->size()-1 )
                    * nr_ * dr * mesh_sinc_ ) + 1;
  const Float sinc_dense = 1.0 / (Float)mesh_sinc_;
  sinc_lookup_.clear();   sinc_lookup_.resize(nsinc, 0.0);
  cos_lookup_.clear();    cos_lookup_.resize(nsinc, 0.0);

  // to avoid the singularity of sinc function at zero
  sinc_lookup_.push_back(1.0);  cos_lookup_.push_back(1.0);
  for (int i=1; i<nsinc; i++) {
    Float x = i * sinc_dense;
    sinc_lookup_[i] = sin(x) / x;
    cos_lookup_[i] = cos(x);
  }

  r_.clear();   r_.resize(nr_, 0.0);
  r_square_reciprocal_.clear();     r_square_reciprocal_.resize(nr_, 0.0);
  for (unsigned int i=0; i<nr_; i++) {
    r_[i] = dr * i;
    r_square_reciprocal_[i] = 1.0 / square(r_[i]);
  }

  sincval_array_.clear(); sincval_array_.resize(exp_saxs_profile_->size());
  std::vector<Float> sincval_temp;
  sincval_temp.resize(nr_, 0.0);
  for (unsigned int iq=0; iq<exp_saxs_profile_->size(); iq++) {
    for (unsigned int ir=0; ir<nr_; ir++) {
      Float qr = exp_saxs_profile_->get_q(iq) * r_[ir];
      int ilookup = (int)( mesh_sinc_ * qr + 0.5 );
      sincval_temp[ir] = sinc_lookup_[ilookup] - cos_lookup_[ilookup];
    }
    sincval_array_[iq] = sincval_temp;
  }

  // Pre-store zero_formfactor for all particles, for the faster calculation
  zero_formfactor_.clear();   zero_formfactor_.resize(particles.size(), 0.0);
  for (unsigned int iatom=0; iatom<particles.size(); iatom++)
    zero_formfactor_[iatom] = ff_table_->get_form_factor(particles[iatom]);

  chi_square_ = 0.0;
  return 0;
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
// TODO: Combine with "RadialDistribution Class"? -> poor performance
std::vector<IMP::algebra::Vector3D> SAXSScore::calculate_chi_real_derivative (
                                       const SAXSProfile& model_saxs_profile,
                                       const std::vector<Particle*>& particles)
{
  std::vector<algebra::Vector3D> chi_derivatives;
  unsigned int i, iq, iatom, ir;
  const Float dr_reciprocal = 1.0 / exp_saxs_profile_->get_pr_resolution();

  if (exp_saxs_profile_->size() != model_saxs_profile.size()) {
    std::cerr << "Number of profile entries mismatch! "
              << "(exp:" << exp_saxs_profile_->size()
              << ", model:" << model_saxs_profile.size() << ")" << std::endl;
   return chi_derivatives;
  }

  //!------ precalculate difference of intensities and squares of weight
  std::vector<Float> delta_i, e_q, delta_i_and_e_q;
  delta_i.resize(model_saxs_profile.size(), 0.0);
  e_q.resize(model_saxs_profile.size(), 0.0);
  delta_i_and_e_q.resize(model_saxs_profile.size(), 0.0);
  for (iq=0; iq<model_saxs_profile.size(); iq++) {
    Float delta = exp_saxs_profile_->get_intensity(iq)
                  - c_ * model_saxs_profile.get_intensity(iq);
    Float square_error = square(exp_saxs_profile_->get_error(iq));
    Float weight_tilda = model_saxs_profile.get_weight(iq) / square_error;

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_saxs_profile_->get_intensity(iq)) < IMP_SAXS_DELTA_LIMIT)
      delta = 0.0;
    delta_i[iq] = weight_tilda * delta;
    e_q[iq] = std::exp( - exp_saxs_profile_-> modulation_function_parameter_
                       * square( exp_saxs_profile_->get_q(iq) ) );
    delta_i_and_e_q[iq] = delta_i[iq] * e_q[iq];
  }
  //!------ copy coordinates in advance, to avoid n^2 copy operations
  std::vector<algebra::Vector3D> coordinates;
  coordinates.resize(particles.size());
  for (iq=0; iq<particles.size(); iq++)
    coordinates[iq] = core::XYZDecorator::cast(particles[iq]).get_coordinates();

  std::vector<algebra::Vector3D> Delta;
  algebra::Vector3D Delta_q(0.0, 0.0, 0.0), chi_derivative(0.0, 0.0, 0.0);
  // TODO: Very weird. (in double precision)
  // TODO: Why this definition makes huge difference in performance?
  std::vector<Float> Delta_x;//, Delta_y, Delta_z;

  //!------ The real loop starts from here
  chi_derivatives.resize(particles.size());
  for (iatom=0; iatom<particles.size(); iatom++) {
    Delta.clear();
    Delta.resize(nr_, algebra::Vector3D(0.0, 0.0, 0.0));

    //!------ precalculate difference delta
    for (i=0; i<particles.size(); i++) {
      if (i != iatom) {
        Float dist = distance(coordinates[iatom], coordinates[i]);
        unsigned int ir = (unsigned int)( dist * dr_reciprocal + 0.5 );
        Float temp = zero_formfactor_[iatom] * zero_formfactor_[i]
                      * r_square_reciprocal_[ir];
        Delta[ir] += temp * (coordinates[iatom] - coordinates[i]);
      }
    }

    //!------ core of chi derivatives
    chi_derivative[0] = chi_derivative[1] = chi_derivative[2] = 0.0;
    for (iq=0; iq<model_saxs_profile.size(); iq++) {
      Delta_q[0] = Delta_q[1] = Delta_q[2] = 0.0;

      for (ir=0; ir<nr_; ir++) {
        //!------ sincval_array_[][] = sinc_lookup_[] - cos_lookup_[]
        Delta_q += Delta[ir] * sincval_array_[iq][ir];
      }
      //!----- delta_i_and_e_q[] = delta_i[] * e_q[]
      chi_derivative += Delta_q * delta_i_and_e_q[iq];
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
