/** \file SaxsData.h
    \brief saxs data initilize header file.
    Copyright 2007-8 Sali Lab. All rights reserved.
  */

#ifndef IMPSAXS_SAXS_DATA_H
#define IMPSAXS_SAXS_DATA_H

#include "config.h"
#include "boost/multi_array.hpp"

#include <IMP/Model.h>
#include <IMP/core.h>

IMPSAXS_BEGIN_NAMESPACE

/** SAXSDATA data type. This objects contains all relevant information
 for incorporation of SAXS constraints to modeling. In particular
 experimental SAXS data, formfactors, q, ...
 12/08/05 FF - added w_s, wswitch, s_hybrid, normsq_exp
 last change 06/02/06 FF - added use_offset
 02/26/07 FF - added scorespace, pr_smooth, autocorr
 */

class IMPSAXSEXPORT SaxsData
{
public:
  //! Constructor
  SaxsData(Model *model, IMP::core::MolecularHierarchyDecorator mp);
  virtual ~SaxsData();

  virtual int initialize(double s_min, double s_max, int maxs, int nmesh,
                  int natomtyp, std::string represtyp, std::string filename,
                  std::string wswitch, double s_low, double s_hi,
                  double s_hybrid, std::string spaceflag, bool use_lookup);

  //! formfactor heavy atom representation
  int saxs_formheavatm(void);

  //! index of formfactor for atomtyp in restyp - heavy atom mode
  int saxs_assformfac_heavat(std::string atmnam, std::string restyp);

  //! Calculate SAXS profile and write spectrum
  int saxs_intensity(std::string filename, bool fitflag);

  //! Faster computation in real space, compute P(r)
  int saxs_computepr(void);

  //! compute I(s) from P(r)
  int saxs_pr2is(void);
  //void saxs_computeis(); // Computation in reciprocal space

  //! Read SAXS data
  int saxs_read(std::string filename);

  //! computes the norm^2 of a function I(s) as sum ( (func * wei)^2 )
  double saxs_sqnormfunc(std::vector<double> func,
                         std::vector<double> wei, int i_low, int i_hi);

  //! Calculate SAXS score from model
  double saxs_chifun(bool transfer_is);

  //! calculate indices corresponding to bandpass
  int saxs_bandpass2i(int *i_low, int *i_hi);

  //! calculate SAXS Chi square of experimental data and model
  int saxs_chi(double *saxsscore, int i_low, int i_hi);

  //! calculate scaling of intensity to int_exp
  double saxs_scale(std::vector<double> wei,double weisum, int i_low, int i_hi);

  //void saxs_forces();  // Derivative of SAXS score ('forces')
  //void saxs_chi_deriv();   // Formulation in reciprocal space
  //void saxs_chi_real_deriv();  // Formulation in real space

private:
  Model *model_;
  IMP::core::MolecularHierarchyDecorator mp_;

  //! number of particles and residues in pdb files
  int num_particles_;
  int num_residues_;

  Particles ps_;
  Particles residues_;

  //! ----- sampling of reciprocal space -----------------------------------
  //! number of sampling points
  int ns_;

  //! maximum number of sampling point
  int maxs_;

  //! nmesh: actual number of frequencies (<= maxs)
  int nmesh_;

  //! number of different atoms (correlates to file!)
  int natomtyp_;

  //! number of formfactors per scattering center to be used.
  //! 2 or larger if isomorphic replacement or Cysteine labeling works...
  int nscatts_;

  //! maximum frequency in space (4\pi\sin(theta)/\lambda) in \AA
  double s_max_;

  //! minimum frequency in space
  double s_min_;

  //! sampling density in reciprocal space (s_max/(ns-1))
  double s_mesh_;

  //! bandpass to exclude frequency below / above
  double s_low_, s_hi_;

  /** frequency above which s^2 weighting is applied if 'hybrid'
     weighting is specified
   */
  double s_hybrid_;

  /** norm^2 of I(s) given the weighting scheme wswitch and the error
     sigma_exp
   */
  double normsq_exp_;

  //! scaling factor of model spectrum to match experimental one
  double c_;

  //! offset of experimental data (optional)
  double offset_;

  //! b-factor for Gaussian rolloff (optional)
  double bfac_;

  //! magnitude of Gaussian rolloff (optional)
  double rolloff_;

  //! chi_square of experimental and model saxs data
  double chi_sq_;

  //! electron density of solvent - default=0.334 e/A^3 (H2O)
  double rho_solv_;

  //! density in r
  double dr_;

  //! density in r of experimental data
  double dr_exp_;

  //! no of r-samples of experimental data p_r_exp
  int nr_exp_;

  //! no of sampling points for sinc function per 1 unit
  int mesh_sinc_;

  //! density of sampling for sinc function
  double sinc_dens_;

  //! array containing atom index for each atom (~formfactor file)
  //! original: struct mod_int1_array atmindx;
  std::vector<int> atmindx_;

  //! mesh in reciprocal space s(maxs)
  //! original: struct mod_double1_array s;
  std::vector<double> s_;

  //! weighting factors in frequency space
  //! original: struct mod_double1_array w_s;
  std::vector<double> w_s_;

  /** ----- intensity and radial distribution function ---------------------
      from model - FF: later extend to multiple scattering factors -
      i.e. isomorphous replacement, gold labeling
      calculated SAXS profile of model intensity(maxs)
    */
  //! struct mod_double1_array intensity;
  std::vector<double> intensity_;

  //! calculated radial distribution function p(maxs) of model
  //! original: struct mod_double1_array p_r;
  std::vector<double> p_r_;

  //! formfactors of scattering centers formfactor(natomtyp, maxs)
  //! original: struct mod_double2_array formfactor;
  typedef boost::multi_array<double, 2> VECTOR2D_;
  VECTOR2D_::extent_gen extents_;

  //typedef array_type::index index;
  //std::vector< std::vector<double> > formfactor_;
  VECTOR2D_ formfactor_;

  std::vector<double> zero_form_factors_;

  /** ----- intensity and radial distribution function - EXPERIMENT ---------
      FF: later: extend to multiple f's
      atom indices saxs restraint is working on
    */
  //! original: struct mod_int1_array isaxsatm;
  std::vector<int> isaxsatm_;

  //! length of isaxsatm array
  int n_isaxsatm_;

  //! no of r-samples
  int nr_;

  //! measured saxsdata int_exp(maxs)
  //! original: struct mod_double1_array int_exp;
  std::vector<double> int_exp_;

  //! corresponding error
  //! original: struct mod_double1_array sigma_exp;
  std::vector<double> sigma_exp_;

  //! measured p(r) - derived from int_exp
  //! original: struct mod_double1_array p_r_exp;
  std::vector<double> p_r_exp_;

  //! radius mesh corresponding to p_r_exp
  //! original: struct mod_double1_array r_exp;
  std::vector<double> r_exp_;

  //! approximated error of p_r_exp - derived from int_exp
  //! original: struct mod_double1_array p_r_sig;
  std::vector<double> p_r_sig_;

  //! model p(r) on mesh of p_r_exp
  //! original: struct mod_double1_array p_r_resamp;
  std::vector<double> p_r_resamp_;

  //! switch for using lookup tables for sinc and cos functions
  //! original: gboolean use_lookup;
  bool use_lookup_;

  //! switch for using additive constant in exp. data
  //! origianl: gboolean use_offset;
  bool use_offset_;

  //! switch for using Gaussian rolloff on model
  //! original: gboolean use_rolloff;
  bool use_rolloff_;

  //! switch for using nitrogen formfactor for convolution
  //! original: gboolean use_conv;
  bool use_conv_;

  //! switch for mixed conformations
  //! original: gboolean mixflag;
  bool mixflag_;

  //! switch smoothing of p_r
  //! original: gboolean pr_smooth;
  bool pr_smooth_;

  //! include autocorrelation term in P(r)?
  //! original: gboolean autocorr;
  bool autocorr_;

  //! lookup table for sinc function
  //! original: struct mod_double1_array sinc_lookup;
  std::vector<double> sinc_lookup_;

  //! lookup table for cos function
  //! original: struct mod_double1_array cos_lookup;
  std::vector<double> cos_lookup_;

  //! Pointers for maintaining a doubly-linked list
  //! original: struct mod_derv_pt prev, next;
  //! original: TYPE(saxsdata), POINTER :: prev, nex
  //! ?: saxsdata *prev, *nex;

  //! ----- scoring in reciprocal or real space 'reciprocal' 'real'
  std::string scorespace_;

  //! how should I(s) be computed? real space via P(r) or reciprocal?
  std::string spaceflag_;

  //! type of representation - 'allatm' or 'CA'
  std::string represtyp_;

  //! filename of the library for formfactors
  std::string filename_;

  //! weighting scheme in frequency space
  std::string wswitch_;

  //! filename of used formfactors
  std::string formfac_file_;

  std::vector<std::string> atmnam_;

  //! error parameter
  int ierr_;
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SAXS_DATA_H */
