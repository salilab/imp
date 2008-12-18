/**
 *  \file SaxsData.cpp
 *  \brief saxs data initilize cpp file.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/saxs/SaxsData.h>
#include <IMP/log.h>

IMPSAXS_BEGIN_NAMESPACE


//! Constructor
SaxsData::SaxsData(Model *model, IMP::core::MolecularHierarchyDecorator mp) {
  //! Initialization of parameters
  model_ = model;
  mp_ = mp;

  mp_.show();
  mp_.validate();

  Particles ps = IMP::core::molecular_hierarchy_get_by_type(mp,
                        IMP::core::MolecularHierarchyDecorator::ATOM);
  Particles residues = IMP::core::molecular_hierarchy_get_by_type(mp,
                        IMP::core::MolecularHierarchyDecorator::RESIDUE);

  /*  number of particles in pdb files  */
  int num_particles = ps.size();
  int num_residues = residues.size();

  printf("num of particles : %d\n", num_particles);
  printf("num of residues : %d\n", num_residues);

  for (int i=0; i<num_particles; i++) {
    IMP::core::XYZDecorator d = IMP::core::XYZDecorator::cast(ps[i]);
    d.show();
    printf("P%d: x = %g, y = %g, z = %g\n", i, d.get_x(), d.get_y(), d.get_z());
  }
}


//! Destructor
SaxsData::~SaxsData() {
  /*if (model_)
    free(model_);
  if (mp_)
    free(mp_);*/
}


/** ini_saxs : Initialize saxsdata
 ! ----------------------------------------------------------------------
 !>   Initialization of SAXS data.
 !!   In this routine the SAXS parameters are set, the sampling in reciprocal
 !!   space is determined and the resulting scattering factors are computed.
 !!   last change 04/10/06
 !!   10/10/06 FF - note: sel1 and n_sel1 have to be called like that to be
 !!                 passed correctly ...
 !!   01/28/07 FF - set nr_exp and dr_exp in ini_saxs
 !!   03/14/07 FF - included rolloff
 ! ----------------------------------------------------------------------

 * atmsel: selection of atoms
 * s_min: minimum frequency in reciprocal space in A^-1
 * s_max: maximum frequency in reciprocal space in A^-1
 * maxs: maximum number of frequencies
 * nmesh: actual number of frequencies (<= maxs)
 * natomtyp: number of 'atoms', i.e. scattering centers
 * represtyp: representation: 'heav', 'allh', or 'CA'
 * filename: filename of the library for formfactors
 * wswitch: character for filter of scoring function options:
  'unity', 'sq', or 'hybrid'
 * s_hybrid: frequency above which $ s^2$ weighting is applied
  if wswitch='hybrid'
 * s_low: bandpass filter in A^-1 - lower cutoff
 * s_hi: bandpass filter in A^-1 - higher cutoff
 * spaceflag: how should I(s) be computed? 'real' space via P(r)
  or 'reciprocal'? 'real' is more than a magnitude
  faster but less accurate for high resolution
 * rho_solv: electron density of solvent;
  default=0.334 e/A^-3 (H_2O)
 * use_lookup: use lookup tables for SINC and COS function -
  significant increase in speed for 'reciprocal' mode
 * nr: number of points for P(r) sampling
 * dr: spacing (sampling) of P(r) in A
 * nr_exp: number of points for P_exp(r) sampling
 * dr_exp: spacing (sampling) of P(r) in A
 * use_offset: allow for additive constant in expt. spectrum
 * use_rolloff: allow for Gaussian rolloff of model spectrum
 * use_conv: convolute with nitrogen formfactor to mimic hydr
  layer
 * mixflag: different conformations present? implemented for
  HtpG project
 * pr_smooth: smoothing of P(r)
 */

/*
 def ini_saxs(self, atmsel,
 filename='$(LIB)/formfactors-int_tab_solvation.lib',
 s_min=0.0, s_max=2.0, maxs=100, nmesh=100, natomtyp=15,
 represtyp='heav', wswitch='uniform', s_hybrid=0.0,
 s_low=0.0, s_hi=2.0, spaceflag='real', rho_solv=0.334,
 use_lookup=True, nr=5000, dr=0.1, nr_exp=300, dr_exp=1.0,
 use_offset=False, use_rolloff=False, use_conv=False,
 mixflag=False, pr_smooth=False):


 (inds, mdl) = atmsel.get_atom_indices()
 return _modeller.mod_saxs_ini(self.modpt, mdl.modpt, inds, s_min,
                              s_max, maxs, nmesh, natomtyp, represtyp,
                              filename, wswitch, s_hybrid, s_low, s_hi,
                              spaceflag, rho_solv, use_lookup, nr, dr,
                              nr_exp, dr_exp, use_offset, use_rolloff,
                              use_conv, mixflag, pr_smooth)
*/

/*
! ----------------------------------------------------------------------
!>   Initialization of SAXS data.
!!   In this routine the SAXS parameters are set, the sampling in reciprocal
!!   space is determined and the resulting scattering factors are computed.
!!   last change 04/10/06
!!   10/10/06 FF - note: sel1 and n_sel1 have to be called like that to be
!!                 passed correctly ...
!!   01/28/07 FF - set nr_exp and dr_exp in ini_saxs
!!   03/14/07 FF - included rolloff
! ----------------------------------------------------------------------
SUBROUTINE mod_saxs_ini_f(saxsd, mdl, sel1, n_sel1, s_min, s_max, maxs, &
                          nmesh, natomtyp, represtyp, filename, wswitch, &
                          s_hybrid, s_low, s_hi, spaceflag, rho_solv, &
                          use_lookup, nr, dr, nr_exp, dr_exp, use_offset, &
                          use_rolloff, use_conv, mixflag, pr_smooth, ierr)
USE types
USE ini_saxs_module
USE alloc_routines
USE io_routines
USE io_constants
USE most_constants

IMPLICIT NONE

TYPE(saxsdata), INTENT(INOUT)    :: saxsd
TYPE(model), INTENT(IN)          :: mdl
INTEGER, INTENT(IN)              :: n_sel1
INTEGER, INTENT(IN)              :: sel1(n_sel1)
REAL(double), INTENT(IN)         :: s_min
REAL(double), INTENT(IN)         :: s_max
INTEGER, INTENT(IN)      :: maxs
INTEGER, INTENT(IN)      :: nmesh
INTEGER, INTENT(IN)      :: natomtyp
CHARACTER(*), INTENT(IN) :: represtyp
CHARACTER(*), INTENT(IN) :: filename
CHARACTER(*), INTENT(IN) :: wswitch
REAL(double), INTENT(IN) :: s_hybrid
REAL(double), INTENT(IN) :: s_low, s_hi
CHARACTER(*), INTENT(IN) :: spaceflag
REAL(double), INTENT(IN) :: rho_solv
LOGICAL, INTENT(IN)      :: use_lookup
INTEGER, INTENT(IN)      :: nr
REAL(double), INTENT(IN) :: dr
INTEGER, INTENT(IN)      :: nr_exp
REAL(double), INTENT(IN) :: dr_exp
LOGICAL, INTENT(IN)      :: use_offset
LOGICAL, INTENT(IN)      :: use_rolloff
LOGICAL, INTENT(IN)      :: use_conv
LOGICAL, INTENT(IN)      :: mixflag
LOGICAL, INTENT(IN)      :: pr_smooth
INTEGER, INTENT(OUT)     :: ierr

ierr = 0
CALL ini_saxs(saxsd, mdl, sel1, n_sel1, s_min, s_max, maxs,&
              nmesh, natomtyp, represtyp, filename, wswitch, &
              s_hybrid, s_low, s_hi, spaceflag, rho_solv, &
              use_lookup, nr, dr, nr_exp, dr_exp, use_offset, &
              use_rolloff, use_conv, mixflag, pr_smooth, ierr)

END SUBROUTINE mod_saxs_ini_f
*/

void SaxsData::initialize(double s_min, double s_max, int maxs, int nmesh,
                          int natomtyp, std::string represtyp,
                          std::string filename, std::string wswitch,
                          double s_low, double s_hi, double s_hybrid,
                          std::string spaceflag, bool use_lookup) {
  s_min_ = s_min;
  s_max_ = s_max;
  maxs_ = maxs;
  nmesh_= nmesh;
  natomtyp_ = natomtyp;
  represtyp_ = represtyp;
  filename_ = filename;
  wswitch_ = wswitch;
  s_hybrid_ = s_hybrid;
  s_low_ = s_low;
  s_hi_ = s_hi;
  spaceflag_ = spaceflag;
  rho_solv_ = 0.334;
  use_lookup_ = use_lookup;
  nr_ = 5000;
  dr_ = 0.1;
  nr_exp_ = 300;
  dr_exp_ = 1.0;
  use_offset_ = false;
  use_rolloff_ = false;
  use_conv_ = false;
  mixflag_ = false;
  pr_smooth_ = false;

  printf("s_min_ = %g\n", s_min_);
  printf("s_max_ = %g\n", s_max_);
  printf("maxs_ = %d\n", maxs_);
  printf("nmesh_= %d\n", nmesh_);
  printf("natomtyp_ = %d\n", natomtyp_);
  std::cout << "represtyp_ = " << represtyp_ << std::endl;
  std::cout << "filename_ = " << filename_ << std::endl;
  std::cout << "wswitch_ = " << wswitch_ << std::endl;
  printf("s_low_ = %g\n", s_low_);
  printf("s_hi_ = %g\n", s_hi_);
  printf("s_hybrid_ = %g\n", s_hybrid_);
  std::cout << "spaceflag_ = " << spaceflag_ << std::endl;
  printf("use_lookup_ = %d\n", use_lookup_);
  printf("rho_solv_ = %g\n", rho_solv_);
  printf("dr_ = %g\n", dr_);
  printf("dr_exp_ = %g\n", dr_exp_);
  printf("nr_exp_ = %d\n", nr_exp_);
  printf("nr_ = %d\n", nr_);
  printf("use_offset_ = %d\n", use_offset_);
  printf("use_rolloff_ = %d\n", use_rolloff_);
  printf("use_conv_ = %d\n", use_conv_);
  printf("mixflag_ = %d\n", mixflag_);
  printf("pr_smooth_ = %d\n", pr_smooth_);
}

IMPSAXS_END_NAMESPACE
