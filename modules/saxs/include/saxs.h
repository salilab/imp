/**
 *  \file saxs.h
 *  \brief saxs initial header file.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPSAXS_SAXSDATA_H
#define IMPSAXS_SAXSDATA_H

#include "config.h"
#include "internal/saxs_version_info.h"

#include <IMP/Model.h>
#include <IMP/core.h>

#define LENF 600

/**  */
/** SAXSDATA data type. This objects contains all relevant information */
/** for incorporation of SAXS constraints to modeling. In particular */
/** experimental SAXS data, formfactors, q, ... */
/** 12/08/05 FF - added w_s, wswitch, s_hybrid, normsq_exp */
/** last change 06/02/06 FF - added use_offset */
/** 02/26/07 FF - added scorespace, pr_smooth, autocorr */
/**  */

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT SaxsData
{
public:
  //! Constructor
  SaxsData(Model &model, IMP::core::MolecularHierarchyDecorator &mp);
  virtual ~SaxsData();

  void ini_saxs(void);
  //void saxs_computeis(); // Computation in reciprocal space
  //void saxs_computepr(); // Faster computation in real space
  //void saxs_chi(); // SAXS score
  //void saxs_scale(); // Scalining parameter and offset
  //void saxs_forces();  // Derivative of SAXS score ('forces')
  //void saxs_chi_deriv();   // Formulation in reciprocal space
  //void saxs_chi_real_deriv();  // Formulation in real space

  Model *model_;
  IMP::core::MolecularHierarchyDecorator *mp_;

private:
  /** ----- sampling of reciprocal space ----------------------------------- */
  /** number of sampling points */
  int ns_;

  /** maximum number of sampling point */
  int maxs_;

  /** number of different atoms (correlates to file!) */
  int natomtyp_;

  /** number of formfactors per scattering center to be used. */
  /** 2 or larger if isomorphic replacement or Cysteine labeling */
  /** works... */
  int nscatts_;

  /** maximum frequency in space (4\pi\sin(theta)/\lambda) in \AA */
  double s_max_;

  /** minimum frequency in space */
  double s_min_;

  /** sampling density in reciprocal space (s_max/(ns-1)) */
  double s_mesh_;

  /** bandpass to exclude frequency below / above */
  double s_low_, s_hi_;

  /** frequency above which s^2 weighting is applied if 'hybrid' */
  /** weighting is specified */
  double s_hybrid_;

  /** norm^2 of I(s) given the weighting scheme wswitch and the error */
  /** sigma_exp */
  double normsq_exp_;

  /** scaling factor of model spectrum to match experimental one */
  double c_;

  /** offset of experimental data (optional) */
  double offset_;

  /** b-factor for Gaussian rolloff (optional) */
  double bfac_;

  /** magnitude of Gaussian rolloff (optional) */
  double rolloff_;

  /** chi_square of experimental and model saxs data */
  double chi_sq_;

  /** electron density of solvent - default=0.334 e/A^3 (H2O) */
  double rho_solv_;

  /** density in r */
  double dr_;

  /** density in r of experimental data */
  double dr_exp_;

  /** no of r-samples of experimental data p_r_exp */
  int nr_exp_;

  /** no of sampling points for sinc function per 1 unit */
  int mesh_sinc_;

  /** density of sampling for sinc function */
  double sinc_dens_;

  /** array containing atom index for each atom (~formfactor file) */
//  struct mod_int1_array atmindx;
  int *atmindx_;

  /** mesh in reciprocal space s(maxs) */
//  struct mod_double1_array s;
  double *s_;

  /** weighting factors in frequency space */
//  struct mod_double1_array w_s;
  double *w_s_;

  /** ----- intensity and radial distribution function --------------------- */
  /** from model - FF: later extend to multiple scattering factors - */
  /** i.e. isomorphous replacement, gold labeling */
  /** calculated SAXS profile of model intensity(maxs) */
//  struct mod_double1_array intensity;
  double *intensity_;

  /** calculated radial distribution function p(maxs) of model */
//  struct mod_double1_array p_r;
  double *p_r_;

  /** formfactors of scattering centers formfactor(natomtyp, maxs) */
//  struct mod_double2_array formfactor;
  double **formfactor_;

  /** ----- intensity and radial distribution function - EXPERIMENT -------- */
  /** FF: later: extend to multiple f's */
  /** atom indices saxs restraint is working on */
//  struct mod_int1_array isaxsatm;
  int *isaxsatm_;

  /** length of isaxsatm array */
  int n_isaxsatm_;

  /** no of r-samples */
  int nr_;

  /** measured saxsdata int_exp(maxs) */
//  struct mod_double1_array int_exp;
  double *int_exp_;

  /** corresponding error */
//  struct mod_double1_array sigma_exp;
  double *sigma_exp_;

  /** measured p(r) - derived from int_exp */
//  struct mod_double1_array p_r_exp;
  double *p_r_exp_;

  /** radius mesh corresponding to p_r_exp */
//  struct mod_double1_array r_exp;
  double *r_exp_;

  /** approximated error of p_r_exp - derived from int_exp */
//  struct mod_double1_array p_r_sig;
  double *p_r_sig_;

  /** model p(r) on mesh of p_r_exp */
//  struct mod_double1_array p_r_resamp;
  double *p_r_resamp_;

  /** switch for using lookup tables for sinc and cos functions */
//  gboolean use_lookup;
  bool use_lookup_;

  /** switch for using additive constant in exp. data */
//  gboolean use_offset;
  bool use_offset_;

  /** switch for using Gaussian rolloff on model */
//  gboolean use_rolloff;
  bool use_rolloff_;

  /** switch for using nitrogen formfactor for convolution */
//  gboolean use_conv;
  bool use_conv_;

  /** switch for mixed conformations */
//  gboolean mixflag;
  bool mixflag_;

  /** switch smoothing of p_r */
//  gboolean pr_smooth;
  bool pr_smooth_;

  /** include autocorrelation term in P(r)? */
//  gboolean autocorr;
  bool autocorr_;

  /** lookup table for sinc function */
//  struct mod_double1_array sinc_lookup;
  double *sinc_lookup_;

  /** lookup table for cos function */
//  struct mod_double1_array cos_lookup;
  double *cos_lookup_;

  /** Pointers for maintaining a doubly-linked list */
//  struct mod_derv_pt prev, next;
// TYPE(saxsdata), POINTER :: prev, nex
// saxsdata *prev, *nex;

  /** ----- scoring in reciprocal or real space 'reciprocal' 'real' */
  char scorespace_[LENF];

  /** how should I(s) be computed? real space via P(r) or reciprocal? */
  char spaceflag_[LENF];

  /** type of representation - 'allatm' or 'CA' */
  char represtyp_[LENF];

  /** weighting scheme in frequency space */
  char wswitch_[LENF];

  /** filename of used formfactors */
  char formfac_file_[LENF];
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SAXSDATA_H */
