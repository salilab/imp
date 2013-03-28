/**
 *  \file AlignSymmetric.h
 *  \brief Fast alignment of a cyclic model to its density.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_ALIGN_SYMMETRIC_H
#define IMPCNMULTIFIT_ALIGN_SYMMETRIC_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <IMP/em/DensityMap.h>
#include "CnSymmAxisDetector.h"
#include <boost/scoped_ptr.hpp>

IMPCNMULTIFIT_BEGIN_NAMESPACE

//! A class for fast alignment of a cyclic model to its density
/**
The class supports Cn and Dn symmetry
 */
class IMPCNMULTIFITEXPORT AlignSymmetric {
public:
  //! Constructor
  /**
\param[in] dmap the density map
\param[in] threshold density threshold used for determining the map pca
\param[in] cn_symm_deg the ring symmetry degree
   */
  AlignSymmetric(em::DensityMap *dmap,float threshold,
                 int cn_symm_deg);//,int dn_symm_deg=1);
  Floats get_sorted_principal_values() const {return map_v_;}
  //! Return the number of matching eigen vectors by their eigen values
  /**
   \param[in] mhs the model to align with the density
   \param[in] max_allowed_diff the maximum allowed difference between matching
                           eigen values
  */
  int score_alignment(atom::Hierarchies mhs,
                       float max_allowed_diff);
  //! Return the two alignments that match the model symmetry axis on
  //! the density symmetry axis
  algebra::Transformation3Ds get_symm_axis_alignments_from_model_to_density(
               atom::Hierarchies mhs,bool sample_translation,
               bool fine_rotation_sampling=true) const;

  void show(std::ostream& out=std::cout) const {
    out << "AlignSymmetric with cn_symm_deg " << cn_symm_deg_ << std::endl;
  }

protected:
  algebra::Transformation3Ds generate_cn_density_rotations(
                  bool fine=false) const;

  algebra::Transformation3Ds generate_cn_density_translations()const;

  float spacing_;
  boost::scoped_ptr<CnSymmAxisDetector> symm_map_;
  int cn_symm_deg_,dn_symm_deg_;
  Floats map_v_; //the principal values of non-symm axis first
};
IMP_VALUES(AlignSymmetric, AlignSymmetrics);

IMPCNMULTIFIT_END_NAMESPACE

#endif  /* IMPCNMULTIFIT_ALIGN_SYMMETRIC_H */
