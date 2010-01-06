/**
 *  \file distance.h
 *  \brief distance metrics
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_DISTANCE_H
#define IMPATOM_DISTANCE_H

#include "config.h"
#include <IMP/core/XYZ.h>
#include "Hierarchy.h"

IMPATOM_BEGIN_NAMESPACE
//! Calculate the root mean square deviation between two sets of 3D points.
/**
\note the function assumes correspondence between two sets of points and does
not perform rigid alignment.
 */
IMPATOMEXPORT double rmsd(const core::XYZs& m1 ,const core::XYZs& m2);

//! Measure the difference between two placements of the same set of points
/**
\param[in] from The reference placement represented by XYZ coordinates
\param[in] to The modeled placement represented by XYZ coordinates
\note The measure quantifies the difference between placements
      of the same structure. A rigid transformation that brings mdl1 to
      ref1 is reported.
\return (d,a), A transformation from mdl to ref represented by
  a a distance (d) and an angle (a).
  d is the distance bewteen the centroids of the two
  placements and a is the axis angle of the rotation matrix between
  the two placements
\note see Lasker,Topf et al JMB, 2009 for details
 */
IMPATOMEXPORT std::pair<double,double> placement_score(
  const core::XYZs& from ,const core::XYZs& to);

//! Measure the difference between two placements of the same set of points
/**
\param[in] ref1 The reference placement of the first component represented
               by XYZ coordinates
\param[in] ref2 The reference placement of the second component represented
               by XYZ coordinates
\param[in] mdl1 The modeled placement of the first component
           represented by XYZ coordinates
\param[in] mdl2 The modeled placement of the second component
           represented by XYZ coordinates
\return the function returns (distance score,angle score)
\note The measure quantifies the difference between the relative placements
      of two components compared to a reference relative placement.
      First, the two compared structures are brought into
      the same frame of reference by superposing the first pair of
      equivalent domains (ref1 and mdl1). Next, the distance and angle
      scores are calculated for the second component using placement_score.
\note see Topf, Lasker et al Structure, 2008 for details
 */
IMPATOMEXPORT std::pair<double,double> component_placement_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2);

//! Measure the RMSD between two placements of the same set of points
/**
\param[in] ref1 The reference placement of the first component represented
               by XYZ coordinates
\param[in] ref2 The reference placement of the second component represented
               by XYZ coordinates
\param[in] mdl1 The modeled placement of the first component
           represented by XYZ coordinates
\param[in] mdl2 The modeled placement of the second component
           represented by XYZ coordinates
\note The measure quantifies the RMSD between the relative placements
      of two components compared to a reference relative placement.
      First, the two compared structures are brought into
      the same frame of reference by superposing the first pair of
      equivalent domains (ref1 and mdl1). Next, the RMSD is calculated
      for the second component
\note see Lasker et al JMB, 2009 for details
 */
IMPATOMEXPORT Float pairwise_rmsd_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2);


IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DISTANCE_H */
