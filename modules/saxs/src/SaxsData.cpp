/**
 *  \file SaxsData.cpp
 *  \brief saxs data initilize cpp file.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/saxs/SaxsData.h>

#include <boost/multi_array.hpp>

IMPSAXS_BEGIN_NAMESPACE

//! Constructor
SaxsData::SaxsData(Model *model, IMP::core::MolecularHierarchyDecorator mp) {
  //! Initialization of parameters
  model_ = model;
  mp_ = mp;

  mp_.show();
  mp_.validate();

  ps_ = IMP::core::get_by_type(mp_,
                        IMP::core::MolecularHierarchyDecorator::ATOM);
  residues_ = IMP::core::get_by_type(mp_,
                        IMP::core::MolecularHierarchyDecorator::RESIDUE);

  //! number of particles and residues in pdb files
  num_particles_ = ps_.size();
  num_residues_ = residues_.size();

  printf("num of particles : %d\n", num_particles_);
  printf("num of residues : %d\n", num_residues_);

  for (int i=0; i<num_particles_; i++) {
    IMP::core::AtomDecorator ad = IMP::core::AtomDecorator::cast(ps_[i]);
    IMP::core::AtomType at = ad.get_type();

    IMP::core::MolecularHierarchyDecorator mpp =
        (IMP::core::MolecularHierarchyDecorator::cast(ps_[i])).get_parent();
    IMP::core::ResidueDecorator rd =
        IMP::core::ResidueDecorator::cast(mpp.get_particle());
    IMP::core::ResidueType rt = rd.get_type();

    //ad.show();
    /*printf("P%d: (%d, %s) r# = %d, (%d, %s), x = %g, y = %g, z = %g\n", i,
           rt.get_index(), (rt.get_string()).c_str(), rd.get_index(),
           at.get_index(), (at.get_string()).c_str(),
           ad.get_x(), ad.get_y(), ad.get_z());
     */
  }
}


//! Destructor
SaxsData::~SaxsData() {
  if (model_)
    free(model_);
}


/** ini_saxs : Initialize saxsdata
 ! ----------------------------------------------------------------------
 !>   Initialization of SAXS data.
 !!   in this routine the saxs parameters are set, the sampling in reciprocal
 !!   space is determined and the resulting scattering factors are computed
 !!   last change 04/21/06 FF - added lookup table for sinc
 !!               04/24/06 FF - increased size of formfactor array
 !!               05/25/06 FF - added nr and dr as input parameters,
 !!                             removed maxdiam
 !!               10/10/06 FF - ini_saxs now works on a selection
 !!               10/10/06 FF - initialization for formfactor indices now here
 !!               10/16/06 FF - s_low = s_min if s_low specified < s_min
 !!               10/30/06 FF - iform = 0 for atoms out of saxssel
 !!               12/07/06 FF - set rho_solv before computation of formfacs
 !!               02/26/07 FF - pr_smooth added
 !!               03/01/07 FF - 'CA' mode works for all atoms
 !!               03/14/07 FF - rolloff added as option
 ! ----------------------------------------------------------------------
 s_min :      minimum frequency in reciprocal space in A^-1
 s_max :      maximum frequency in reciprocal space in A^-1
 maxs :       maximum number of frequencies
 nmesh :      actual number of frequencies (<= maxs)
 natomtyp :   number of 'atoms', i.e. scattering centers
 represtyp :  representation: 'heav', 'allh', or 'CA'
 filename :   filename of the library for formfactors
 wswitch :    character for filter of scoring function
 options: 'unity', 'sq', or 'hybrid'
 s_hybrid :   frequency above which s^2 weighting is applied
 if wswitch='hybrid' (if 'hybrid' weighting is specified)
 s_low :      bandpass filter in A^-1 - lower cutoff
 s_hi :       bandpass filter in A^-1 - higher cutoff
 spaceflag :  how should I(s) be computed? 'real' space via P(r)
 or 'reciprocal'? 'real' is more than a magnitude
 faster but less accurate for high resolution
 rho_solv :   electron density of solvent;  default=0.334 e/A^-3 (H_2O)
 use_lookup : use lookup tables for SINC and COS function -
 significant increase in speed for 'reciprocal' mode
 nr :         number of points for P(r) sampling
 dr :         spacing (sampling) of P(r) in A
 nr_exp :     number of points for P_exp(r) sampling
 dr_exp :     spacing (sampling) of P(r) in A
 use_offset : allow for additive constant in expt. spectrum int_exp
 use_rolloff :allow for Gaussian rolloff of model spectrum
 use_conv :   convolute with nitrogen formfactor to mimic hydr layer
 mixflag :    different conformations present? implemented for
 HtpG project
 pr_smooth :  smoothing of P(r)

 ! ----------------------------------------------------------------------
 // TODO: Don't know how to handle these...
 (inds, mdl) = atmsel.get_atom_indices()

 !>      selection indices of model and dimension
 INTEGER, INTENT(IN)      :: n_indatm
 INTEGER, INTENT(IN)      :: indatm(n_indatm)
 atmsel :     selection of atoms
*/
int SaxsData::initialize(double s_min, double s_max, int maxs, int nmesh,
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
  nr_ = 100;
  //nr_ = 5000;
  dr_ = 0.5;
  //dr_ = 0.1;
  nr_exp_ = 300;
  dr_exp_ = 1.0;
  use_offset_ = false;
  use_rolloff_ = false;
  use_conv_ = false;
  mixflag_ = false;
  pr_smooth_ = false;
  ierr_ = 0;

  double mesh_density;    //! density of mesh in reciprocal space [1/\AA]
  std::vector<bool> picatm;  //! picatm(mdl%cd%natm)
  int i;           //! Local Scalars
  std::string routine = "ini_saxs";     //! Local Arrays

  //! copy specified selection to saxs structure
  //saxsd%n_isaxsatm = n_indatm
  //n_isaxsatm_ = n_sel1;

  n_isaxsatm_ = num_particles_;
  isaxsatm_.resize(n_isaxsatm_);
  std::cout << "size of isaxsatm_ = " << isaxsatm_.size() << std::endl;


  // TODO: Don't know how to handle these...
  // -> Probably ps_ ?
  //saxsd%isaxsatm(:) =  indatm(:)
  //CALL selection_set_picatm(mdl, indatm, n_indatm, picatm, ierr)


  // Select all just temporarily
  picatm.resize(num_particles_);
  for (i=0; i<(int)picatm.size(); i++)
    picatm[i] = 1;

  //! --------- sampling of reciprocal space ------------
  mesh_sinc_ = 500;         //! how many points per 1 unit in sinc
  sinc_dens_ = 1.0 / mesh_sinc_;

  if (nmesh_ > maxs_)
    maxs_ = nmesh_;

  if ( s_max_ < s_hi_ ) {
    printf("ini_saxs: specified low-pass > maxs! - set s_hi = maxs = %g\n",
           s_max_);
    s_hi_ = s_max_;
  } else {
    printf("ini_saxs: specified low-pass:  s_hi = %g\n", s_hi_);
  }

  if ( (s_low_ < s_min_) || (s_low_ < 0.0) ) {
    printf("ini_saxs: impossible choice of s_low - set s_low = s_min\n");
    printf("      specified s_low = %g, s_min = %g\n", s_low_, s_min_);
    printf("      set s_low = %g\n", s_min_);
    s_low_ = s_min_;
  } else {
    printf("ini_saxs: specified hi-pass: s_low = %g\n", s_low_);
  }

  printf("############ sampling parameters #################\n");
  printf("s_min_ = %g, s_max_ = %g\n", s_min_, s_max_);
  printf("s_low_ = %g, s_hi_ = %g\n", s_low_, s_hi_);
  printf("##################################################\n");

  ns_ = nmesh_;
  mesh_density = (s_max_ - s_min_) / (nmesh_ - 1);
  s_mesh_ = mesh_density;
  formfac_file_ = filename_;

  intensity_.resize(maxs_);
  s_.resize(maxs_);
  w_s_.resize(maxs_);
  std::cout << "size of intensity_ = " << intensity_.size() << std::endl;
  std::cout << "size of s_ = " << s_.size() << std::endl;
  std::cout << "size of w_s = " << w_s_.size() << std::endl;

  //!------ setup lookup tables for sinc and cosine, if use_lookup = True
  if (use_lookup_) {
    int nsinc = (int)(s_max_ * nr_ * dr_ * mesh_sinc_) + 1;
    printf("nsinc = %d\n", nsinc);

    sinc_lookup_.resize(nsinc);
    cos_lookup_.resize(nsinc);
    printf("ini_saxs: use_lookup = True\n");
    printf("          using lookup tables for sinc and cos\n");
    sinc_lookup_[0] = 1.0;  // to avoid the singularity of sinc function at zero
    cos_lookup_[0] = 1.0;
    for (int is=1; is<nsinc; i++) {
      double x = (double)(is) * sinc_dens_;

      //! TODO: Question) Is it sin(pi*x)/(pi*x)? or just sin(x)/x?
      sinc_lookup_[is] = sin(x) / x;
      cos_lookup_[is] = cos(x);
    }
  } else {
    printf("ini_saxs: use_lookup = False\n");
    printf("          no lookup tables used\n");
  }
  if (use_offset_) {
    printf("ini_saxs: use_offset = True\n");
    printf("          allow for additive const in experimental data\n");
  }
  if (use_rolloff_) {
    printf("ini_saxs: use_rolloff= True\n");
    printf("          allow for Gaussian rolloff - make sure you have\n");
    printf("          libsaxs, which is NOT part of modeller\n");
  }
  if (use_conv_) {
    printf("ini_saxs: use_conv= True - convolute with nitrogen formfactor\n");
  }
  if (mixflag_) {
    printf("ini_saxs: mixflag = True\n");
    printf("          more than one conformation simultaneously\n");
    printf("          make sure that set in FIRST saxsstructure\n");
    if (use_offset_) {
      printf("ini_saxs: use_offset AND mixflag does not work\n");
      ierr_ = -1;
      return ierr_;
    }
  }

  //!------ compute frequency mesh
  s_[0] = s_min_;
  for (i=1; i<ns_; i++)
    s_[i] = mesh_density + s_[i-1];

  //!------ compute the weighting function
  if ( wswitch_ == "unity" ) {
    s_hybrid_ = 0.0;
    printf("ini_saxs: wswitch = <unity>\n");
    printf("          using w(s) = 1 for all s\n");
    for (i=0; i<ns_; i++)
      w_s_[i] = 1.0;

  } else if ( wswitch_ == "sq" ) {
    s_hybrid_ = 0.0;
    printf("ini_saxs: wswitch = <sq>\n");
    printf("          using w(s) = s^2\n");
    for (i=0; i<ns_; i++)
      w_s_[i] = s_[i];

  } else if ( wswitch_ == "hybrid" ) {
    printf("ini_saxs: wswitch = <hybrid>\n");
    printf("          using w(s) = %g", s_hybrid_*s_hybrid_);
    printf(" for s < %g, otherwise s^2\n", s_hybrid_);
    for (i=0; i<ns_; i++) {
      if (s_[i] > s_hybrid_)
        w_s_[i] = s_[i];
      else
        w_s_[i] = s_hybrid_;
    }
  } else {
    //! use unity if nothing is specified
    wswitch_ = "unity";
    s_hybrid_ = 0.0;
    printf("ini_saxs: unknown wswitch\n");
    printf("          using w(s) = 1\n");
    for (i=0; i<ns_; i++)
      w_s_[i] = 1.0;
  }

  //!------ density of solvent
  printf("ini_saxs: rho_solv_ = %g\n", rho_solv_);
  printf("          electron density of solvent in e/A^3\n\n");

  //!------ compute formfactors for given sampling
  //! for Backbone Carbons
  if ( represtyp_ == "CA" ) {
    printf("ini_saxs: represtyp = <CA>\n");
    printf("          reduced representation - make sure\n");
    printf("          that only CA or CB atoms are used\n");

    //! Boost.MultiArray(?)
    formfactor_.resize(extents_[natomtyp_][maxs_]);
    zero_form_factors_.resize(natomtyp_);
    //! CALL saxs_formca(saxsd)
  //! for all atoms
  } else if ( represtyp_ == "allh" ) {
    printf("ini_saxs: represtyp = <allh>\n");
    printf("          All hydrogen mode\n");

    //! Boost.MultiArray(?)
    formfactor_.resize(extents_[natomtyp_][maxs_]);
    zero_form_factors_.resize(natomtyp_);
    //! CALL saxs_formallatm(saxsd)
  //! for heavy atoms
  } else if ( represtyp_ == "heav" ) {
    printf("ini_saxs: represtyp = <heav>\n");
    printf("          Heavy atom mode\n");

    //! Boost.MultiArray(?)
    formfactor_.resize(extents_[natomtyp_][maxs_]);
    zero_form_factors_.resize(natomtyp_);
    saxs_formheavatm();
  //! for an error
  } else {
    printf("ini_saxs: unrecognized restyp\n");
    printf("          choose <CA>, <allh>, or <heav>\n");
    printf("---- STOPPED in ini_saxs -------\n");
    ierr_ = -2;
    return ierr_;
  }

  //!------ allocate p_r if spaceflag=real
  if (spaceflag_ == "reciprocal") {
    printf("ini_saxs: spaceflag=<reciprocal>\n");
    printf("          compute I(s) in reciprocal space\n");
  } else if (spaceflag_ == "real") {
    printf("ini_saxs: spaceflag=<real>\n");
    printf("          compute I(s) in real space via P(r)\n");
    printf("          dr= %g, nr= %d\n", dr_, nr_);
    printf("       => max Diameter = %g\n", dr_*(double)(nr_-1));
    p_r_.resize(nr_);
  } else {
    printf("ini_saxs: unrecognized spaceflag\n");
    printf("          choose <reciprocal> or <real>\n");
    printf("          set to <reciprocal>\n");
    spaceflag_ = "reciprocal";
  }

  //!------------------------------------------------------------------------
  //!------------------ create array for index corresponding to atoms
  //!------------------------------------------------------------------------
  atmindx_.resize(num_particles_);
  // TODO: natm?
//  CALL alloc_array(saxsd%atmindx, 1, mdl%cd%natm, .TRUE., routine, &
//                   'atmindx')

  if (represtyp_ == "CA") {
    for (i=0; i<num_particles_; i++) {
      //!IF ( picatm(i) .AND. (mdl%cd%atmnam(i)=='CA') ) THEN
      if (picatm[i]) {
        // TODO: formfactor for CA?
        //iform = saxs_assignformfactorca(&
        //                          mdl%seq%irestyp(mdl%cd%iresatm(i)))
        //saxsd%atmindx(i) = iform
      }
    }
  } else if ( represtyp_ == "allh" ) {
    for (i=0; i<num_particles_; i++) {
      if (picatm[i]) {
        // TODO: formfactor for all atom?
        //iform = saxs_assformfac_allat(mdl%cd%atmnam(i)(1:1))
        //saxsd%atmindx(i) = iform
      }
    }
  } else if ( represtyp_ == "heav") {
    for (i=0; i<num_particles_; i++) {
      if (picatm[i]) {
        // TODO: ?
        //iform = saxs_assformfac_heavat(mdl%cd%atmnam(i), &
        //                         mdl%seq%irestyp(mdl%cd%iresatm(i)) )
        IMP::core::AtomDecorator ad = IMP::core::AtomDecorator::cast(ps_[i]);
        IMP::core::AtomType at = ad.get_type();

        IMP::core::MolecularHierarchyDecorator mpp =
          (IMP::core::MolecularHierarchyDecorator::cast(ps_[i])).get_parent();
        IMP::core::ResidueDecorator rd =
          IMP::core::ResidueDecorator::cast(mpp.get_particle());
        IMP::core::ResidueType rt = rd.get_type();

        atmindx_[i] = saxs_assformfac_heavat(at.get_string(), rt.get_string());

        /*
        printf("P%d: (%d, %s) r# = %d, (%d, %s), x = %g, y = %g, z = %g\n", i,
               rt.get_index(), (rt.get_string()).c_str(), rd.get_index(),
               at.get_index(), (at.get_string()).c_str(),
               ad.get_x(), ad.get_y(), ad.get_z());
        */

        //printf("%s -> %d\n", (at.get_string()).c_str(), atmindx_[i]);
        //!WRITE(card,*) ' ', mdl%cd%atmnam(i), ' -> ', iform
      }
    }
  } else {
    printf("ini_saxs: unrecognized represtyp\n");
    printf("          choose CA, allh, or heav\n");
    printf("          bye bye....\n");
    ierr_ = -3;
    return ierr_;
  }

  pr_smooth_ = false;
  if (pr_smooth_)
    printf("ini_saxs: smoothing of p_r according to s_max= %g\n", s_max_);

  printf("sinc_dens_ = %g\n", sinc_dens_);
  printf("maxs_ = %d\n", maxs_);
  printf("nmesh_= %d\n", nmesh_);
  printf("natomtyp_ = %d\n", natomtyp_);
  std::cout << "represtyp_ = " << represtyp_ << std::endl;
  std::cout << "filename_ = " << filename_ << std::endl;
  std::cout << "formfac_file_ = " << formfac_file_ << std::endl;
  std::cout << "wswitch_ = " << wswitch_ << std::endl;

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
  printf("ierr_ = %d\n", ierr_);

  return ierr_;
}


/*
! ----------------------------------------------------------------------
!>   Calculates the formfactor of an atom on given sampling in
!!   reciprocal space for heavy atom topology.
!!   same as saxs_formallatm.F90 but here hydrogens are added
!!   to the respective atoms
!!
!!   FF 04/26/06
!!   05/22/06 FF: added gold (AU)
!!   06/13/06 FF: check for natomtyp < 15
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_formheavatm(void) {
  //! .. Local Arrays
  int iat=0, is, isum, nh=0, iform, ierr=0;
  double s_waasmei;
  char temp[256];
  boost::multi_array<double,2> a(boost::extents[natomtyp_][5]);
  boost::multi_array<double,2> b(boost::extents[natomtyp_][5]);
  std::vector<double> c(natomtyp_), excl_vol(natomtyp_), volr(natomtyp_);

  std::ifstream fh;
//  const double PI = 4.0*atan(1.0);
  std::string line;

  fh.open(formfac_file_.c_str());
  if(!fh.is_open()) {
    std::cerr << "Can't open file " << formfac_file_ << std::endl;
    return -1;
  }

  while ( !fh.eof() ) {
    getline(fh, line);
    if (line[0] == '#' || line[0] == '\0')
      continue;

    if (iat >= natomtyp_) {
      printf("\nsaxs_formheavatm: More formfactors in FILE than specified ");
      printf("in natomtyp\n ignoring these lines");
      break;
    }

    sscanf(line.c_str(), "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
           temp, &a[iat][0], &a[iat][1], &a[iat][2], &a[iat][3], &a[iat][4],
           &c[iat], &b[iat][0], &b[iat][1], &b[iat][2], &b[iat][3], &b[iat][4],
           &excl_vol[iat]);

    volr[iat] = std::pow(excl_vol[iat], (2.0/3.0));
    //volr[iat] = pow(excl_vol[iat], (2.0/3.0)) / (4.0*IMP::internal::PI);

// for Debugging
printf("%d \t %g \t %g \t %g\n", iat, excl_vol[iat], IMP::internal::PI,
       volr[iat]);

    iat++;
  }
  fh.close();

  for (is=0; is<ns_; is++) {
    //s_waasmei = s_[is] / (4.0*IMP::internal::PI);
    //s_waasmei *= s_waasmei;
    s_waasmei = s_[is] * s_[is];

    for (iform=0; iform<14; iform++) {
      switch (iform) {
        case 0 : iat = 2; nh = 0; break;  //! C
        case 1 : iat = 2; nh = 1; break;  //! CH
        case 2 : iat = 2; nh = 2; break;  //! CH_2
        case 3 : iat = 2; nh = 3; break;  //! CH_3
        case 4 : iat = 3; nh = 0; break;  //! N
        case 5 : iat = 3; nh = 1; break;  //! NH
        case 6 : iat = 3; nh = 2; break;  //! NH_2
        case 7 : iat = 3; nh = 3; break;  //! NH_3
        case 8 : iat = 4; nh = 0; break;  //! O
        case 9 : iat = 4; nh = 1; break;  //! OH
        case 10: iat = 9; nh = 0; break;  //! S
        case 11: iat = 9; nh = 1; break;  //! SH
        case 12: iat = 8; nh = 0; break;  //! P
        case 13: iat = 14; nh = 0; break;  //! AU
      }
      if (iat <= natomtyp_) {
        formfactor_[iform][is] = c[iat] + nh*c[0];
        for (isum=0; isum<5; isum++) {
          formfactor_[iform][is] += a[iat][isum]*exp(-b[iat][isum]*s_waasmei)
                                  + nh * a[0][isum]*exp(-b[0][isum]*s_waasmei);
        }
        //! subtract solvation component
        formfactor_[iform][is] -= rho_solv_ *
        (excl_vol[iat]*exp(-(4.0*IMP::internal::PI)*volr[iat]*s_waasmei)
         + nh * excl_vol[0]*exp(-(4.0*IMP::internal::PI)*volr[0]*s_waasmei) );
        // TODO: ?
        //! - rho_solv_ * (excl_vol(iat) + nh * excl_vol(1))
        //! use for comparison to real space results - replace
        //! preceding 3 lines
      } else {
        formfactor_[iform][is] = 0.0;
      }
    }
  }

  // zero form factors at q=0 for the faster calculation of using P(r)
  for (int i=0; i<14; i++) {
    switch (i) {
      case 0 : iat = 2; nh = 0; break;  //! C
      case 1 : iat = 2; nh = 1; break;  //! CH
      case 2 : iat = 2; nh = 2; break;  //! CH_2
      case 3 : iat = 2; nh = 3; break;  //! CH_3
      case 4 : iat = 3; nh = 0; break;  //! N
      case 5 : iat = 3; nh = 1; break;  //! NH
      case 6 : iat = 3; nh = 2; break;  //! NH_2
      case 7 : iat = 3; nh = 3; break;  //! NH_3
      case 8 : iat = 4; nh = 0; break;  //! O
      case 9 : iat = 4; nh = 1; break;  //! OH
      case 10: iat = 9; nh = 0; break;  //! S
      case 11: iat = 9; nh = 1; break;  //! SH
      case 12: iat = 8; nh = 0; break;  //! P
      case 13: iat = 14; nh = 0; break;  //! AU
    }
    if (iat <= natomtyp_) {
      zero_form_factors_[i] = c[iat] + nh*c[0];
      for (int j=0; j<5; j++) {
        zero_form_factors_[i] += a[iat][j] + nh*a[0][j];
      }
      //! subtract solvation component
      zero_form_factors_[i] -= rho_solv_ * (excl_vol[iat] + nh*excl_vol[0]);
    } else {
      zero_form_factors_[i] = 0.0;
    }
    printf("%d \t %.15f\n", i, zero_form_factors_[i]);
  }

  return ierr;
}


/*
 ! ----------------------------------------------------------------------
 !>   Assigns the index of the formfactor to an atom based on atomic
 !!   symbol for heavyatom case. Hydrogens get accounted for by bound
 !!   heavy atom -> effective 'atoms':
 !!   NH, NH_2, NH_3
 !!   CH, CH_2, CH_3
 !!   OH
 !!   SH
 !!   FF 04/25/06
 !!   04/28/06 FF: adopted Ben's residue indices
 !!   05/22/06 FF: added gold (AU)
 ! ----------------------------------------------------------------------
!       Definitions:
!       group  iform
!       C      1
!       CH     2
!       CH_2   3
!       CH_3   4
!       N      5
!       NH     6
!       NH_2   7
!       NH_3   8
!       O      9
!       OH     10
!       S      11
!       SH     12
!       P      13
!       AU     14
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_assformfac_heavat(std::string atmnam, std::string restyp) {
  // TO DO: look at saxs_routines@routines.F90
  int iform = -1;
  //printf("%s %s\n", atmnam.c_str(), restyp.c_str());

  //!------- Carbons -------------------------------------
  if (atmnam == "CA") {
    if ( (restyp == "GLY") || (restyp == "UNK") ) {
      iform = 3;  //! CH_2
    } else {
      iform = 2;  //! CH
    }
  } else if (atmnam == "CT") {
    iform = 4; //! CH_3 at C-term
  } else if (atmnam == "C") {
    iform = 1;
  } else if (atmnam == "CB") {
    if (restyp == "GLY") {
      iform = 1; // ! C - GLY does not have a CB anyway ...
    } else if ( (restyp == "ILE") || (restyp == "THR") || (restyp == "VAL") ) {
      iform = 2; // ! CH
    } else if (restyp == "ALA") {
      iform = 4; //! CH3
    } else {
      iform = 3; //! CH2
    }
  } else if (atmnam == "CG") {
    if ((restyp == "ASN") || (restyp == "ASP") ||
        (restyp == "HIS") || (restyp == "PHE") ||
        (restyp == "TRP") || (restyp == "TYR") ||
        (restyp == "ASX") || (restyp == "HSP")) {
      iform = 1; //! C
    } else if (restyp == "LEU") {
      iform = 2; //! CH
    } else {
      iform = 3; //! CH_2
    }
  } else if (atmnam == "CG1") {
    if (restyp == "ILE") {
      iform = 3;  //! CH_2
    } else {  //! VAL
      iform = 4;
    }
  } else if (atmnam == "CG2") { //! only VAL, ILE, and THR
    iform = 4;
  } else if (atmnam == "CD") {
    if ((restyp == "GLU") || (restyp == "GLN") || (restyp == "GLX")) {
      iform = 1; //! C
    } else {
      iform = 3; //! CH_2
    }
  } else if (atmnam == "CD1") {
    if ((restyp == "LEU") || (restyp == "ILE")) {
      iform = 4;
    } else if ((restyp == "PHE") || (restyp == "TRP") || (restyp == "TYR")) {
      iform = 2;
    } else {
      iform = 1;
    }
  } else if (atmnam == "CD2") {
    if ( restyp == "LEU" ) {
      iform = 4; //! CH_3
    } else if ((restyp == "PHE") || (restyp == "TYR") ||
               (restyp == "HIS") || (restyp == "HSE")) {
      iform = 2; //! CH
    } else {
      iform = 1; //! C
    }
  } else if (atmnam == "CE") {
    if ( restyp == "LYS" ) {
      iform = 3; //! CH_2
    } else if ( restyp == "MET" ) {
      iform = 4; //! CH_3
    } else {
      iform = 1; //! C
    }
  } else if (atmnam == "CE1") {
    if ((restyp == "PHE") || (restyp == "TYR") || (restyp == "HIS") ||
        (restyp == "HSE") || (restyp == "HSP")) {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CE2") {
    if ((restyp == "PHE") || (restyp == "TYR")) {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CE3") {
    if ( restyp == "TRP" ) {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CZ") {
    if ( restyp == "PHE" ) {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CZ1") {
    iform = 1;
  } else if (atmnam == "CZ2") {
    if ( restyp == "TRP") {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CZ3") {
    if ( restyp == "TRP") {
      iform = 2; //! CH
    } else {
      iform = 1;
    }
  } else if (atmnam == "CH2" || atmnam == "CHA" ||
             atmnam == "CHC" || atmnam == "CHD") {
    iform = 2; //! CH in TRP and HEME
  } else if (atmnam == "CAA" || atmnam == "CBA" || atmnam == "CBB" ||
             atmnam == "CBC" || atmnam == "CAD" || atmnam == "CBD") {
    iform = 3; //! CH2 in HEME

  //!------- N --------------------------
  } else if (atmnam == "N") {
    if ( (restyp == "PRO") || (restyp == "BRK") ) {
      iform = 5; //! N
    } else {
      iform = 6; //! NH
    }
  } else if (atmnam == "ND") {
    iform = 5; //! N
  } else if (atmnam == "ND1") {
    if ( restyp == "HIS") {
      iform = 6; //! NH
    } else {
      iform = 5;
    }
  } else if (atmnam == "ND2") {
    if ( restyp == "ASN") {
      iform = 7; //! NH_2
    } else {
      iform = 5;
    }
  } else if (atmnam == "NH1" || atmnam == "NH2") { //! only in ARG
    if ( restyp == "ARG" ) {
      iform = 7; //! NH_2
    } else {
      iform = 5;
    }
  } else if (atmnam == "NE") {
    if ( restyp == "ARG" ) { //! only in ARG
      iform = 6; //! NH
    } else {
      iform = 5;
    }
  } else if (atmnam == "NE1") {
    if ( restyp == "TRP" ) {
      iform = 6; //! NH
    } else {
      iform = 5;
    }
  } else if (atmnam == "NE2") {
    if ( (restyp == "GLN") || (restyp == "GLX") ) {
      iform = 7; //! NH_2
    } else {
      iform = 5; //! e.g. 'HIS'
    }
  } else if (atmnam == "NZ") {
    if ( restyp == "LYS" ) {
      iform = 8; //! NH_3
    } else {
      iform = 5;
    }
  } else if (atmnam == "NA" || atmnam == "NB" || atmnam == "NC") {
    iform = 5;

  //!------- O --------------------------------------
  } else if (atmnam == "O" || atmnam == "OE1" || atmnam == "OE2" ||
             atmnam == "OD1" || atmnam == "OD2" || atmnam == "O1A" ||
             atmnam == "O2A" || atmnam == "OXT" || atmnam == "OT1" ||
             atmnam == "OT2") {
    iform = 9;
  } else if (atmnam == "OG") {
    if ( restyp == "SER" ) {
      iform = 10; //! OH
    } else {
      iform = 9;
    }
  } else if (atmnam == "OG1") {
    if ( restyp == "THR" ) {
      iform = 10;
    } else {
      iform = 9;
    }
  } else if (atmnam == "OH") {
    if ( restyp == "TYR" ) {
      iform = 10; //! OH
    } else {
      iform = 9;
    }

  //!------- S --------------------------------------
  } else if (atmnam == "SD") {
    iform = 11;
  } else if (atmnam == "SG") {
    if ( restyp == "CYS" ) {
      iform = 12;
    } else {
      iform = 11;
    }
  } else if (atmnam == "P") { //! phosphor
    iform = 13;
  } else if (atmnam.find("AU") == 0) { //! gold label
    iform = 14;
  } else {  //! complain if ATOM unknown
    iform = 1; //! set iform to carbon for undefined
    printf("\nsaxs_assformfac_heavat: ATOM < %s > unknown \n", atmnam.c_str());
  }

  iform--;  // To match the Fortran index and the C/C++ index
  return iform;
}


/*
! ----------------------------------------------------------------------
!>   Calculate SAXS and write spectrum
!!   03/15/07 FF - scaling included in intensity
!!   03/19/07 FF - scaling always computed if fitflag=True
!!   03/27/07 FF - rescaling in saxs_scale
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_intensity(std::string filename, bool fitflag) {
  int is;
  std::vector<double> wei;
  std::FILE *fp;

  wei.resize(ns_);
  printf("Intensity_filename = %s : %d\n", filename.c_str(), fitflag);

  /* // TODO: what is this?
  int i_low, i_hi;
  double weisum;

  !IF (saxsd%use_rolloff) THEN
  !  scf = -11.529+12.2126+3.13220+2.01250+1.16630-saxsd%rho_solv*2.49
  !  scf = 1/scf
  !  volr = 2.49**(2./3.) / (4.*pi)
  !END IF
  !saxsd%spaceflag = spaceflag
   */

  if (spaceflag_ == "reciprocal") {
    // TODO: saxs_computeis()
  /*CALL saxs_computeis(saxsd, &
                      REAL(mdl%cd%x(1:mdl%cd%natm), double), &
                      REAL(mdl%cd%y(1:mdl%cd%natm), double), &
                      REAL(mdl%cd%z(1:mdl%cd%natm), double), &
                      mdl%cd%natm, saxsd%n_isaxsatm, &
                      saxsd%isaxsatm, ierr)*/
    //saxs_computeis();
  } else if (spaceflag_ == "real") {
    saxs_computepr();
    saxs_pr2is();
  } else {
    printf("\nsaxs_intens: unrecognized spaceflag: %s\n Bye, Bye...\n",
           spaceflag_.c_str());
    return -1;
  }

  // TODO: what is this?
  /*!IF (saxsd%use_rolloff) THEN
  !  DO is=1, saxsd%ns
  !    s2 = saxsd%s(is)**2
  !    q2 = s2/(4*pi*pi)
  !    ! mult nitrogen
  !    q2 = scf*(-11.529+12.2126*EXP(-0.0057*q2)+&
                 !       3.13220*EXP(-9.89330*q2) + 2.01250*EXP(-28.9975*q2)+&
                 !       1.16630*EXP(-0.582600*q2)) - &
  !       saxsd%rho_solv*2.49*EXP(-volr*s2)
  !    saxsd%intensity(is) = saxsd%intensity(is)*q2*(1.-saxsd%rolloff*&
                                                     !EXP(-saxsd%bfac*s2))
  !  END DO
  !END IF*/

  fp = fopen(filename.c_str(), "w");
  fprintf(fp, "#  nmesh=%7d meshdens=%14.7lf\n", ns_, s_mesh_);

  if (fitflag) {
    // TODO: what is this?
/*    CALL saxs_bandpass2i(saxsd, i_low, i_hi);
    weisum = 0.0;
    for (is=i_low; is<=i_hi; is++) {
      wei(is) = saxsd%w_s(is) * saxsd%w_s(is);
      weisum = weisum + wei(is);
    }
    saxsd%c = saxs_scale(saxsd, wei, weisum, i_low, i_hi);
  DO is = 1, saxsd%ns
  IF (saxsd%use_offset) THEN
  WRITE(card, '(f7.4,2x,f22.5,2x,f22.5)') saxsd%s(is), &
  saxsd%intensity(is), saxsd%int_exp(is)+saxsd%offset
  CALL writelinef(fh, TRIM(card))
  ELSE
  WRITE(card, '(f7.4,2x,f22.5,2x,f22.5)') saxsd%s(is), &
  saxsd%intensity(is), saxsd%int_exp(is)
  CALL writelinef(fh, TRIM(card))
  END IF
  END DO*/
  } else {
    for (is=0; is<ns_; is++) {
      fprintf(fp, "%22.5lf\t%22.5lf\n", s_[is], intensity_[is]);
    }
  }
  fclose(fp);

  return 0;
}


/*
 ! ----------------------------------------------------------------------
 !>   computes P(r) for given coordinates and s sampling
 !!   FF 04/28/06
 !!   05/01/06 FF - added ierr
 !!   10/09/06 FF - added selection
 !!   12/08/06 FF - removed autocorr part for p_r(0)
 !!   12/18/06 FF - put it back in - agreement with Debye markedly better
 !!   03/27/07 FF - removed model - not needed anymore...
 ! ----------------------------------------------------------------------
*/
int SaxsData::saxs_computepr(void) {
  int i, j;
  unsigned int ir;
  double dist;
  std::vector<IMP::algebra::Vector3D> coordinates;

  // copy coordinates in advance, to avoid n^2 copy operations
  for(i=0; i<(int)ps_.size(); i++) {
    IMP::core::XYZDecorator d = IMP::core::XYZDecorator::cast(ps_[i]);
    coordinates.push_back(d.get_coordinates());
  }
  for (i=0; i<nr_; i++)
    p_r_[i] = 0.0;

  std::FILE *fp;
  fp = fopen("ir.txt", "w");
//!------ loop
  for (i=0; i<num_particles_; i++) {
    for (j=i+1; j<num_particles_; j++) {
      dist = distance(coordinates[i], coordinates[j]);
      ir = (unsigned int)(dist/dr_ + 0.5);
      if (ir > (unsigned int)nr_) {
        std::cout << "error (ir > nr_)" << std::endl;
        return -1;
      }
      fprintf(fp, "(%d, %d) = %d\n", i, j, ir);
      p_r_[ir] += 2.0 * zero_form_factors_[atmindx_[i]]
                      * zero_form_factors_[atmindx_[j]];
    }
//!--------   autocorrelation term
    p_r_[0] += zero_form_factors_[atmindx_[i]]*zero_form_factors_[atmindx_[i]];
  }
  fclose(fp);

  // for Debugging
  //std::FILE *fp;
  fp = fopen("p_r.txt", "w");
  for (i=0; i<nr_; i++) {
    fprintf(fp, "%d\t%20.10f\n", i, p_r_[i]);
    //printf("%d\t%20.10f\n", i, p_r_[i]);
  }
  fclose(fp);

  return 0;
}


/*
! ----------------------------------------------------------------------
!>   transformation P(r) to I(s)
!!   FF 04/28/06
!!   05/05/06 - FF: added Gaussian decay for effective formfactor
!!   10/30/06 - FF: added envelope for CA case
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_pr2is(void) {
  int ir, is, ilookup;
  double x, sincval;
  std::vector<double> r;
  r.resize(nr_);

  for (ir=0; ir<nr_; ir++)
    r[ir] = dr_ * (double)ir;

  for (is=0; is<ns_; is++) {
    intensity_[is] = 0.0;

    for (ir=0; ir<nr_; ir++) {
      x = s_[is] * r[ir];
      //printf("x=%g s=%g r=%g\n", x, s_[is], r[ir]);
      if (use_lookup_) {
        ilookup = (int)( mesh_sinc_ * x + 0.5);
        sincval = sinc_lookup_[ilookup];
      } else {
        if (x == 0.0)
          sincval = 1.0;
        else
          sincval = sin(x) / x;
      }
      intensity_[is] += p_r_[ir] * sincval;

      //printf("r=%d p(r)=%g x=%g sinc=%g I=%g\n",
      //        ir, p_r_[ir], x, sincval, intensity_[is]);
    }
    //exit(-1);

    // TODO: what is this?
    //!IF (saxsd%represtyp /='CA') THEN
    intensity_[is] *= exp( -0.23 * s_[is] * s_[is] );
      //!ELSE
    //!  saxsd%intensity(is) = saxsd%intensity(is) *&
    //!    EXP( -4.2718 * saxsd%s(is)*saxsd%s(is) )
    //!END IF
  }
  return 0;
}



/*
 ! ----------------------------------------------------------------------
 !>   Read SAXS data.
 !!   The routine reads experimental saxsdata and errors to saxsd%int_exp.
 !!   Furthermore, the weighting function is updated if an experimental
 !!   error is specified
 !!   last changes
 !!   05/03/06 FF - log updated
 !!   09/28/06 FF - use subroutine to get i_low and i_max, log updated
 !!   10/10/06 FF - experimental error set to 0.3 * I(s_max) if
 !!                 none specified
 !!   12/08/06 FF - fixed bug in is increment - when did it get it?!
 !!   01/26/07 FF - fixed bug for no error - now LAST I_max is taken ...
 ! ----------------------------------------------------------------------
*/
int SaxsData::saxs_read(std::string filename) {
  int ncols=0, is=0, i_low, i_hi;
  std::ifstream fh;
  std::string line;
  double sig_exp=0.0, temp1, temp2, temp3;

  std::cout << "reading a file : " << filename << std::endl;
  fh.open(filename.c_str());
  if(!fh.is_open()) {
    std::cerr << "Can't open the file : " << filename << std::endl;
    return -1;
  }

  //! ----- first check number of frequencies
  while ( !fh.eof() ) {
    getline(fh, line);
    if (line[0] == '#' || line[0] == '\0')
      continue;

    ncols = sscanf(line.c_str(), "%lf %lf %lf", &temp1, &temp2, &temp3);
    is++;
  }
  fh.close();

  if (is != ns_) {
    printf("saxs_read: ns other than specified in ini_saxs\n");
    printf("           ns_old = %d changed to ns_file = %d\n", ns_, is);
    ns_ = is;
    if (is > maxs_) {
      maxs_ = is;
      printf("more frequencies than specified in ini_saxs(maxs)\n");
      printf("exit...\n");
      return -2;
    }
  }

  int_exp_.resize(maxs_);
  sigma_exp_.resize(maxs_);
  if (ncols < 3) {
    printf("saxs_read: No experimental error specified\n");
    sig_exp = 0.3 * temp2;
    printf("           error set to 0.3 I(s_max) = %g\n", sig_exp);
  }

  //! ----- Rewind file
  fh.open(filename.c_str());

  //! ----- now read in the numbers
  is=0;
  while ( !fh.eof() ) {
    getline(fh, line);
    if (line[0] == '#' || line[0] == '\0')
      continue;

    ncols = sscanf(line.c_str(), "%lf %lf %lf",
                   &s_[is], &int_exp_[is], &sigma_exp_[is]);

    if (ncols < 3)
      sigma_exp_[is] = sig_exp;

    w_s_[is] /= sigma_exp_[is];

    //printf("%d (cols:%d) \t %.10lf \t %.10lf \t %.10lf \t %.10lf\n",
    //       is, ncols, s_[is], int_exp_[is], sigma_exp_[is], w_s_[is]);
    is++;
  }
  fh.close();

  //! ------ set parameters for sampling in reciprocal space
  if (s_max_ != s_[is-1]) {
    printf("saxs_read: s_max_ in file differs from initial value\n");
    printf("       old s_max_ = %g -> s_max_ = %g\n", s_max_, s_[is-1]);
    s_max_ = s_[is-1];
  }
  if (s_min_ != s_[0]) {
    printf("saxs_read: s_min_ in file differs from initial value\n");
    printf("       old s_min_ = %g -> s_min_ = %g\n", s_min_, s_[0]);
    s_min_ = s_[0];
  }
  s_mesh_ = (s_[is-1] - s_[0]) / (ns_ - 1);

  //! ------ calculate norm of experimental profile to from hipass to lowpass
  //!        frequency
  saxs_bandpass2i(&i_low, &i_hi);
  normsq_exp_ = saxs_sqnormfunc(int_exp_, w_s_, i_low, i_hi);
  printf("saxs_read: Norm (=Integral) of SAXS data: %.5lf\n", normsq_exp_);
  printf(" - integral ranged from s_low=%g to s_hi=%g\n", s_low_, s_hi_);
  std::cout << "i_low = " << i_low << ", i_hi = " << i_hi << std::endl;

  return 0;
}


/*
! ----------------------------------------------------------------------
!>   computes the norm^2 of a function I(s) as
!!   sum ( (func * wei)^2 )
!!   12/08/05 FF
!!   08/07/06 FF - sum ranges from i_low to i_hi
! ----------------------------------------------------------------------
 */
double SaxsData::saxs_sqnormfunc(std::vector<double> func,
                                std::vector<double> wei, int i_low, int i_hi) {
  double norm=0.0, temp;

  for (int i=i_low; i<i_hi; i++) {
    temp = func[i] * wei[i];
    norm += temp * temp;
  }
  return norm;
}


/*
! ----------------------------------------------------------------------
!>   Calculate SAXS score chi from model.
! ----------------------------------------------------------------------
*/
double SaxsData::saxs_chifun(bool transfer_is) {
  int i_low, i_hi; //, isaxsspecs, nsaxsspecs;
  //double saxsscore=0.0;
  //TYPE(saxslist), POINTER  :: saxspt
  //REAL(double), POINTER    :: saxsintens(:,:)
  //REAL(double), POINTER    :: scalefacs(:)
  std::vector<double> saxsintens;
  std::vector<double> scalefacs;

  //std::cout << "saxs_chifun : " << transfer_is << std::endl;
//!------ determine number of saxs spectra ---------------------------------
  // TODO: what is this?
  /*
  isaxsspecs = 0
  saxspt => enedata%saxslst
  DO WHILE (ASSOCIATED(saxspt))
    isaxsspecs = isaxsspecs + 1
    saxspt => saxspt%next
  END DO
  nsaxsspecs = isaxsspecs
  NULLIFY(saxsintens, scalefacs)
  saxspt => enedata%saxslst
  CALL alloc_array(saxsintens, 1, nsaxsspecs, 1, &
                   saxspt%d%maxs, .TRUE., routine, 'saxsintens')
  CALL alloc_array(scalefacs, 1, nsaxsspecs, .TRUE., routine, &
                   'scalefacs')
   */

//!-------calculate I(s) if desired
  if (!transfer_is) {
    //DO isaxsspecs=1, nsaxsspecs
    saxs_intensity("i_s.txt", false);
    //END DO
  } else {
    std::cout <<"transfer_is = True - do not calculate I(s) again"<<std::endl;
  }

//!------ copy intensities for mixed conformation case
  if (mixflag_) {
    /* // TODO: what is this?
    DO isaxsspecs=1, nsaxsspecs
      saxsintens(isaxsspecs,:) = saxspt%d%intensity(:)
      saxspt => saxspt%next
    END DO
    saxspt => enedata%saxslst
    */
  }

//!-------------------------------------------------------------------------
//!-------calculate scores -------------------------------------------------
//!-------------------------------------------------------------------------
  if (mixflag_) {
    /*
    // TODO: ?
    CALL saxs_bandpass2i(saxspt%d, i_low, i_hi)
    CALL saxs_chi_mix(saxsscore, saxspt%d, i_low, i_hi, nsaxsspecs, &
                      saxsintens, scalefacs, ierr)
    IF (ierr /= 0) RETURN
    //! copy scalefactors to saxsstructures
    DO isaxsspecs=1, nsaxsspecs
      saxspt%d%c = scalefacs(isaxsspecs)
      saxspt => saxspt%next
    END DO
    saxspt => enedata%saxslst
    saxspt%d%chi_sq = saxsscore
    */
  } else {
    // TODO: ?
    //DO isaxsspecs=1, nsaxsspecs
    saxs_bandpass2i(&i_low, &i_hi);
    //std::cout << "i_low = " << i_low << " i_hi = " << i_hi << std::endl;
    saxs_chi(&chi_sq_, i_low, i_hi);
    //saxspt => saxspt%next
    //END DO
  }

  return chi_sq_;
}



/*
! ----------------------------------------------------------------------
!>   calculate indices corresponding to bandpass
!!   FF 09/29/06
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_bandpass2i(int *i_low, int *i_hi) {
  int is;

  *i_hi = 0;
  *i_low = 0;
  for (is=0; is<maxs_; is++) {
    //!-------- determine freq that corresponds to lowpass
    if (s_[is] > s_hi_) {
      *i_hi = is;
      break;
    }
  }

  if ( (*i_hi >= ns_) || (*i_hi == 0) )
    *i_hi = ns_;

  //!-------- ... and hipass
  for (is=0; is<maxs_; is++) {
    //!-------- determine freq that corresponds to lowpass
    if (s_[is] >= s_low_) {
      *i_low = is;
      break;
    }
  }

  return 0;
}



/*
!------------ scoring function with weighting by error      ------------
! ----------------------------------------------------------------------
!>   calculate SAXS Chi square of experimental data and model
!!   in this function the following scoring function is used:
!!
!!         \chi = (\sum_{is} wei(is) *( int_exp(is)-intens(is) )**2) /
!!                    (\sum_{is} fil(is))
!!
!!   FF 06/08/06
!!   last change
!!   FF 09/28/06 - docu updated
!!   03/15/07 FF - scale c in intensity
! ----------------------------------------------------------------------
*/
int SaxsData::saxs_chi(double *saxsscore, int i_low, int i_hi) {
  int is;
  double weisum=0.0, filsum=0.0, fil, temp;
  std::vector<double> wei;

  wei.resize(ns_);
  *saxsscore = 0.0;
  std::cout << "i_low = " << i_low << " i_hi = " << i_hi << std::endl;

  //! precalculate square of weight
  for (is=i_low; is<i_hi; is++) {
    wei[is] = w_s_[is] * w_s_[is];
    weisum += wei[is];

    //! w_s already contains sigma_exp -> multiply to get rid
    fil = w_s_[is] * sigma_exp_[is];
    fil *= fil;
    filsum += fil;
  }

  //! calculate scaling and offset (passed to saxs structure as saxsd%offset)
  c_ = saxs_scale(wei, weisum, i_low, i_hi);
  std::cout << "c_ = " << c_ << std::endl;


  // TODO: do we need this?
  /*!IF (saxsd%use_rolloff) THEN
  !  CALL saxs_fancyscore(saxsscore, saxsd%ns, i_low, i_hi, &
  !      saxsd%intensity, &
  !      saxsd%int_exp, saxsd%w_s, saxsd%sigma_exp, saxsd%s, saxsd%c, &
  !      saxsd%bfac, saxsd%rolloff, saxsd%offset, saxsd%use_offset)
  !ELSE
   */

  for (is=i_low; is<i_hi; is++) {
    if ( use_offset_ ) {
      // TODO: Why ignoring this scaling parameter, c?
      //!saxsscore = saxsscore + wei(is) * ( saxsd%int_exp(is) +&
      //!   saxsd%offset - saxsd%c * saxsd%intensity(is) )**2
      temp = int_exp_[is] + offset_ - intensity_[is];
    } else {
      //!  saxsd%c * saxsd%intensity(is))**2
      temp = int_exp_[is] - intensity_[is];
    }
    *saxsscore += wei[is] * temp * temp;
  }
  // TODO:  Why?
  *saxsscore /= filsum;

  return 0;
}



/*
! ----------------------------------------------------------------------
!>   calculate scaling of intensity to int_exp
!!   saxsd%c, saxsd%offset
!!   FF 09/28/06
!!   FF 03/15/07 intensity multiplied by c
! ----------------------------------------------------------------------
*/
double SaxsData::saxs_scale(std::vector<double> wei,
                            double weisum, int i_low, int i_hi) {
  int is;
  double sumi_ei=0.0, sumi_i=0.0, sumi=0.0, sume=0.0, c;

  for (is=i_low; is<i_hi; is++) {
    sumi_ei += wei[is] * int_exp_[is] * intensity_[is];
    sumi_i += wei[is] * intensity_[is] * intensity_[is];
    sumi += wei[is] * intensity_[is];
    sume += wei[is] * int_exp_[is];
  }
  if ( use_offset_ ) {
    // TODO: is it true? I don't understand this.
    offset_ = sumi_ei / sumi_i * sumi - sume;
    // TODO: is it true? must be "weisum - sumi / sumi_i"
    offset_ /= (weisum - sumi*sumi/sumi_i);
    c = sumi_ei + offset_ * sumi;
    c /= sumi_i;
  } else {
    c = sumi_ei / sumi_i;
  }
  if (use_rolloff_) {
    // TODO: saxs_fit_test()
    /*CALL saxs_fit_test(saxsd%intensity, saxsd%int_exp, saxsd%w_s,&
          saxsd%sigma_exp, saxsd%s, saxsd%ns, i_low, i_hi, c, saxsd%bfac,&
          saxsd%rolloff, saxsd%offset, saxsd%use_offset, saxsd%use_conv)
     */
  } else {
    for (is=0; is<ns_; is++)
      intensity_[is] *= c;
  }
  return c;
}



/*  for (i=0; i<nr*nr; i++) {
 ir3.push_back( (unsigned int)(sqrt(i) * dr_reciprocal + 0.5) );
 }

 double dx = coordinates[iatom][0] - coordinates[i][0];
 double dy = coordinates[iatom][1] - coordinates[i][1];
 double dz = coordinates[iatom][2] - coordinates[i][2];
 double dist2 = dx*dx + dy*dy + dz*dz;
 unsigned int index = ir3[(unsigned int)(dist2 + 0.5)];
 double temp =formfactor[iatom]*formfactor[i]*r_square_reciprocal[index];

 Delta_x[index] += temp * dx;
 Delta_y[index] += temp * dy;
 Delta_z[index] += temp * dz;
 */



/*
! ----------------------------------------------------------------------
!>    Routines for SAXS restraints in MODELLER.
!!
!! saxs_computeis(saxsd, x, y, z, natm,           - compute I(s)
                  !!                  indatm, n_indatm, ierr)
!! saxs_computepr(saxsd, x, y, z, natm,           - compute P(r)
                  !!                  indatm, n_indatm, ierr)
!! saxs_pr2is(saxsd)                              - compute I(s) from P(r)
!! saxs_sqnormfunc(func, wei, i_low, i_hi)        - calculate norm^2 of func
!!
!!--------- FORMFACTOR stuff -----------------------------------------------
!! saxs_formallatm(saxsd)                         - initialize formfac
!!                                                  (all atom repres)
!! saxs_formheavatm(saxsd)                        - formfactor heavy atom
!!                                                  representation
!! saxs_formca(saxsd)                             - formfactor CA
!! saxs_assignformfactorca(resnam, scattf, saxsd) - assign formfac
!!                                                  (CA representation)
!! iform=saxs_assformfac_allat(atmtyp)            - index of formfactor for
!!                                                  atomtyp - all atom mode
!! iform=saxs_assformfac_heavat(atmnam, irestyp)  - index of formfactor for
!!                                                  atomtyp in irestyp -
!!                                                  heavy atom mode
!! iform=saxs_assignformfactorca(irestyp)         - index of formfactor for
!!                                                  irestyp - CA mode
!!
!!---------------------SCORING FUNCTION -----------------------------------
!! saxs_freqweight(wei, s, wswitch, s_hybrid, ns) - determine w_s for
!!                                                  scoring function
!! saxs_forces(saxsd, deriv, mdl, x, y, z, &      - calculate score func and
!!     dvx, dvy, dvz, &                             forces (called from
!!     scaln, isch, natm, natms, indats,ierr)       obj_function
!! saxs_chisquare(saxsscore, saxsd, ns)           - calculate scoring
!!                                                  function chi Svergun
!! saxs_chi_deriv(mdl, saxsd, iatom, dsx, dsy, &  - derivative of chi
!!     dsz,x, y, z, natm, n_indatm, indatm, ierr)   Svergun
!! saxs_chi_deriv_rig(mdl, saxsd, iatom, dsx, &   - derivative as above
!!     dsy, dsz,x, y, z, natm, irigats, nrigats     for rigid bodies
!!     n_indatm, indatm, ierr)
!! saxs_chinorm(saxsscore, saxsd, ns)             - scoring function with
!!                                                  normalization of exp data
!! saxs_chinorm_deriv(saxsd, iatom, dsx, &        - derivative of above function
!!     dsy, dsz, x, y, z, natm, i_low, i_hi, ierr)
!! saxs_chinorm_deriv_rig(saxsd, iatom, dsx,&     - same as above for rigid
!!     dsy, dsz, x, y, z, natm, irigats, nrigats,&  bodies
!!     i_low, i_hi, ierr)
!! saxs_chinorm_real_deriv(saxsd, iatom, dsx, &   - derivative in real space
!!     dsy, dsz, x, y, z, natm, i_low, i_hi, ierr)        mode
!! saxs_chinorm_real_deriv_rig(saxsd, iatom, dsx,&- same as above for rigid
!!     dsy, dsz, x, y, z, natm, irigats, nrigats,&  bodies
!!     i_low, i_hi, ierr)
!!
!!---------------------MISC STUFF------------------------------------------
!! norm = saxs_sqnormfunc(func, wei, i_low, i_hi) - calculate norm^2 of func
!! dist = saxs_dist(x1,y1,z1,x2,y2,z2)            - Euclidean distance
!! distsq = saxs_distsq(x1,y1,z1,x2,y2,z2)        - squared Euclidean dist
!!
! ----------------------------------------------------------------------
*/

IMPSAXS_END_NAMESPACE
