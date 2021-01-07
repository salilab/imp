/**
 *  \file IMP/isd/GaussianEMRestraint.h
 *  \brief Restrain two sets of Gaussians (model and GMM derived from EM map)
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_GAUSSIAN_EM_RESTRAINT_H
#define IMPISD_GAUSSIAN_EM_RESTRAINT_H

#include "isd_config.h"
#include <IMP/file.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container_macros.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/Gaussian.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/atom/Mass.h>
#include <math.h>
#include <Eigen/Dense>
#include <boost/unordered_map.hpp>

IMPISD_BEGIN_NAMESPACE

//! Restraint between two Gaussian Mixture Models, "model" and "density"
/** This restrains two sets of GMMs with a density overlap function.
    The function is correlation of the two GMMs \f$f_M\f$ and \f$f_D\f$:
    \f[
       \frac{2\int{f_M(x)f_D(x)dx}}{\int{f_M^2(x)+f_D^2(x)dx}}
    \f]
    Where the integral is the "overlap function" given by:
    \f[
    ov(f_M,f_D) = \sum_{i=1}^{N_M} \sum_{j=1}^{N_D} \frac{1}{(2 \pi)^{3/2}|\Sigma_{Mi}+\Sigma_{Dj}|^{1/2}}\exp\left [-\frac{1}{2}(\boldsymbol\mu_{Mi} - \boldsymbol\mu_{Dj})^\top (\Sigma_{Mi}+\Sigma_{Dj})^{-1} (\boldsymbol\mu_{Mi} - \boldsymbol \mu_{Dj})\right ]
    \f]
    \note Source: Greenberg, Pellarin, Sali. In preparation.
 */
class IMPISDEXPORT GaussianEMRestraint : public Restraint
{

 public:
  //! Setup the GaussianEMRestraint
  /**
     \param[in] mdl the Model object to operate on
     \param[in] model_ps particles for the model GMM
     \param[in] density_ps particles for the density GMM
     \param[in] global_sigma Particle to modulate the uncertainty
                (currently unused)
     \param[in] model_cutoff_dist Cutoff for the model-model interactions
     \param[in] density_cutoff_dist Cutoff for model-density interactions
     \param[in] slope Gentle term to move all particles to the density
     \param[in] update_model (DEPRECATED) update model each cycle
     \param[in] backbone_slope Limit the slope only to backbone particles
     \param[in] local Only consider density particles that are within the
                specified cutoff of the model particles (experimental)
     \param[in] name Name of this restraint
     \note the model and density particles must be set up as Gaussian
  */
  GaussianEMRestraint(Model *mdl,
                      ParticleIndexes model_ps, ParticleIndexes density_ps,
                      ParticleIndex global_sigma,
                      Float model_cutoff_dist, Float density_cutoff_dist,
                      Float slope,
                      bool update_model=true, bool backbone_slope=false,
                      bool local=false,
                      std::string name="GaussianEMRestraint%1%");

  //! Returns exp(score)
  double get_probability() const {
    return exp(-unprotected_evaluate(NULL));
  }

  //! Get cross correlation between the model and the map
  /** This CCC is that calculated from the last scoring function
      evaluation; calling this function before the score is calculated
      results in undefined behavior.
    */
  double get_cross_correlation_coefficient() const {
    return cross_correlation_;
  }

  //! Set the filename corresponding to the density GMM particles
  /** If the density GMM particles were read from a file, this method
      can be used to tell the restraint so that it can track this
      information back to the original EM density file, which is useful
      for deposition. */
  void set_density_filename(std::string density_fn) {
    density_fn_ = get_absolute_path(density_fn);
  }

  //! Pre-calculate the density-density and model-model scores
  /** This is automatically called by the constructor.
      You only need to call it manually if you change Gaussian variances.
  */
  void compute_initial_scores();

  //! Set restraint slope
  void set_slope(Float s){slope_=s;}

  //! Get restraint slope
  Float get_slope(){return slope_;}
  virtual double
    unprotected_evaluate(IMP::DerivativeAccumulator *accum) const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  void show(std::ostream &out) const { out << "GEM restraint"; }

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const IMP_OVERRIDE;

  //! \return Information for writing to RMF files
  RestraintInfo *get_dynamic_info() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(GaussianEMRestraint);

 private:
  double model_cutoff_dist_, density_cutoff_dist_;
  ParticleIndexes model_ps_;
  ParticleIndexes density_ps_;
  Float slope_;
  bool update_model_,local_;
  int msize_,dsize_;
  Float normalization_;
  Float dd_score_;
  Float self_mm_score_;
  PointerMember<container::CloseBipartitePairContainer> md_container_;
  PointerMember<container::ClosePairContainer> mm_container_;
  ParticleIndexes slope_ps_; //experiment
  std::string density_fn_;

  //variables needed to tabulate the exponential
  Floats exp_grid_;
  double invdx_;
  double argmax_;

  mutable double cross_correlation_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_EM_RESTRAINT_H */
