/**
 *  \file IMP/multifit/anchor_utilities.h
 *  \brief Common functions in anchor calculations
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_ANCHOR_UTILITIES_H
#define IMPMULTIFIT_ANCHOR_UTILITIES_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/statistics/internal/VQClustering.h>
#include <IMP/statistics/internal/DataPoints.h>
#include "DataPointsAssignment.h"
#include "anchors_reader.h"

IMPMULTIFIT_BEGIN_NAMESPACE

IMPMULTIFITEXPORT
inline AnchorsData molecule2anchors(atom::Hierarchy mh,int k) {
  IMP_NEW(IMP::statistics::internal::ParticlesDataPoints,ddp,
          (core::get_leaves(mh)));
  IMP::statistics::internal::VQClustering vq(ddp,k);
  vq.run();
  multifit::DataPointsAssignment assignment(ddp,&vq);
  multifit::AnchorsData ad(
                          assignment.get_centers(),
                          *(assignment.get_edges()));
  return ad;
}

IMPMULTIFITEXPORT
//! Generate anchors in several formats for a given density map.
void get_anchors_for_density(em::DensityMap *dmap, int number_of_means,
                             float density_threshold,
                             std::string pdb_filename,
                             std::string cmm_filename,
                             std::string seg_filename,
                             std::string txt_filename);

//! Get lists of anchors that match a sequence of secondary structures
/**
   \param[in] ad The AnchorsData
   \param[in] sse_ps The SecondaryStructureResidue particles to match
   \param[in] min_correlation SecondaryStructureResidue match must be
              below this value (0.816 is rmsd of known SSE to random)
*/
IMPMULTIFITEXPORT
IntsList get_anchor_indices_matching_secondary_structure(
                                  const AnchorsData &ad,
                                  const atom::SecondaryStructureResidues &ssrs,
                                  Float max_rmsd=0.7);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_ANCHOR_UTILITIES_H */
