/*
 *  saxs.cpp
 *  imp
 *
 *  Created by sjkim on 12/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <IMP/saxs/saxs.h>
#include <IMP/log.h>

IMPSAXS_BEGIN_NAMESPACE


//! Constructor
SaxsData::SaxsData(Model &model, IMP::core::MolecularHierarchyDecorator &mp) {
  model_ = &model;
  mp_ = &mp;

  mp_->show();
  mp_->validate();

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

/*     def ini_saxs(self, atmsel,
 filename='$(LIB)/formfactors-int_tab_solvation.lib',
 s_min=0.0, s_max=2.0, maxs=100, nmesh=100, natomtyp=15,
 represtyp='heav', wswitch='uniform', s_hybrid=0.0,
 s_low=0.0, s_hi=2.0, spaceflag='real', rho_solv=0.334,
 use_lookup=True, nr=5000, dr=0.1, nr_exp=300, dr_exp=1.0,
 use_offset=False, use_rolloff=False, use_conv=False,
 mixflag=False, pr_smooth=False):
 */

/*(inds, mdl) = atmsel.get_atom_indices()
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

void SaxsData::ini_saxs(void) {
  printf("ini_saxs!!\n");
}

IMPSAXS_END_NAMESPACE
