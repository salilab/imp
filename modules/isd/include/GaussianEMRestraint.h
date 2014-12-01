/**
 *  \file isd/GaussianEMRestraint.h
 *  \brief Restraint two sets of gaussians (model and gmm derived from EM map)
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_GAUSSIAN_EM_RESTRAINT_H
#define IMPISD_GAUSSIAN_EM_RESTRAINT_H

#include "isd_config.h"
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
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <boost/unordered_map.hpp>

IMPISD_BEGIN_NAMESPACE

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
struct KahanAccumulation{
double sum;
double correction;
KahanAccumulation():
  sum(0.0),correction(0.0)
  {}
};
struct KahanVectorAccumulation{
  IMP_Eigen::Vector3d sum;
  IMP_Eigen::Vector3d correction;
KahanVectorAccumulation():
  sum(IMP_Eigen::Vector3d(0,0,0)),
    correction(IMP_Eigen::Vector3d(0,0,0))
  {}
};
KahanAccumulation KahanSum(KahanAccumulation accumulation, double value){
  KahanAccumulation result;
  double y = value - accumulation.correction;
  double t = accumulation.sum + y;
  result.correction = (t - accumulation.sum) - y;
  result.sum = t;
  return result;
}
KahanVectorAccumulation KahanVectorSum(KahanVectorAccumulation accumulation, IMP_Eigen::Vector3d value){
  KahanVectorAccumulation result;
  IMP_Eigen::Vector3d y = value - accumulation.correction;
  IMP_Eigen::Vector3d t = accumulation.sum + y;
  result.correction = (t - accumulation.sum) - y;
  result.sum = t;
  return result;
}
#endif


//! Creates a restraint between two sets of anchor points "model" and "density"
//
/**
 */
class IMPISDEXPORT GaussianEMRestraint : public Restraint
{

 public:
  //! Constructor
  /**
     \param[in] model_ps particles for the model GMM
     \param[in] density_ps particles for the density GMM
     \param[in] global_sigma Particle to modulate the uncertainty
     \param[in] rigid Set to true if the model is rigid (faster)
  */
  GaussianEMRestraint(Model *mdl,
                      ParticleIndexes model_ps, ParticleIndexes density_ps,
                      ParticleIndex global_sigma,
                      Float model_cutoff_dist,Float density_cutoff_dist,Float slope,
                      bool update_model=true, bool backbone_slope=false,
                      std::string name="GaussianEMRestraint%1%");

  double get_probability() const {
    return exp(-unprotected_evaluate(NULL));
  }
  void compute_initial_scores();
  void set_slope(Float s){slope_=s;}
  Float get_slope(){return slope_;}
  virtual double
    unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
    const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  void show(std::ostream &out) const { out << "GEM restraint"; }
  IMP_OBJECT_METHODS(GaussianEMRestraint);

 private:
  ParticleIndexes model_ps_;
  ParticleIndexes density_ps_;
  ParticleIndex global_sigma_;
  Float slope_;
  bool update_model_;
  int msize_,dsize_;
  Float normalization_;
  Float dd_score_;
  Float self_mm_score_;
  base::PointerMember<container::CloseBipartitePairContainer> md_container_;
  base::Pointer<container::ClosePairContainer> mm_container_;
  ParticleIndexes slope_ps_; //experiment

  //variables needed to tabulate the exponential
  Floats exp_grid_;
  double invdx_;
  double argmax_;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_EM_RESTRAINT_H */
