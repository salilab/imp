/**
 *  \file AlignSymmetric.h
 *  \brief Adapted from cnmultifit
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_ALIGN_SYMMETRIC_H
#define IMPMULTIFIT_ALIGN_SYMMETRIC_H
#include "multifit_config.h"
#include <IMP/em/DensityMap.h>
#include <IMP/multifit/CnSymmAxisDetector.h>
#include <IMP/multifit/MolCnSymmAxisDetector.h>

IMPMULTIFIT_BEGIN_NAMESPACE
//! A class for fast alignment of a cyclic model to its density
/**
The class supports Cn and Dn symmetry
 */
class IMPMULTIFITEXPORT AlignSymmetric {
public:
  //! Constructor
  /**
\param[in] dmap the density map
\param[in] threshold density threshold used for determining the map pca
\param[in] cn_symm_deg the ring symmetry degree
   */
  AlignSymmetric(em::DensityMap *dmap,float threshold,
                 int cn_symm_deg);//,int dn_symm_deg=1);
  CnSymmAxisDetector *get_cn_symm_axis_detector() const {return symm_map_;}
  Floats get_sorted_principal_values() const {return map_v_;}
  //! Return the number of matching eigen vectors by their eigen values
  /**
   \input mhs the model to align with the density
   \input max_allowed_diff the maximum allowed difference between matching
                           eigen values
  */
  int score_alignment(atom::Hierarchies mhs,
                       float max_allowed_diff);
  //! Return the two alignments that match the model symmetry axis on
  //! the density symmetry axis
  algebra::Transformation3Ds get_symm_axis_alignments_from_model_to_density(
               atom::Hierarchies mhs,bool sample_translation,
               bool fine_rotation_sampling=true) const;
protected:
  algebra::Transformation3Ds generate_cn_density_rotations(
                  bool fine=false) const;

  algebra::Transformation3Ds generate_cn_density_translations()const;

  float spacing_;
  CnSymmAxisDetector *symm_map_;
  int cn_symm_deg_,dn_symm_deg_;
  Floats map_v_; //the principal values of non-symm axis first
};
IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_ALIGN_SYMMETRIC_H */
