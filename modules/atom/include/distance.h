/**
 *  \file distance.h
 *  \brief distance metrics
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPATOM_DISTANCE_H
#define IMPATOM_DISTANCE_H

#include "atom_config.h"
#include <IMP/core/XYZ.h>
#include "Hierarchy.h"

IMPATOM_BEGIN_NAMESPACE

//! Calculate the root mean square deviation between two sets of 3D points.
/**
   \note the function assumes correspondence between the two sets of
   points and does not perform rigid alignment.

   \genericgeometry
 */
template <class Vecto3DsOrXYZs0, class Vecto3DsOrXYZs1>
double get_rmsd(const Vecto3DsOrXYZs0& m1 ,const Vecto3DsOrXYZs1& m2,
                const IMP::algebra::Transformation3D &tr_for_second
                = IMP::algebra::get_identity_transformation_3d()) {
  IMP_USAGE_CHECK(std::distance(m1.begin(), m1.end())
                  ==std::distance(m2.begin(), m2.end()),
            "The input sets of XYZ points "
            <<"should be of the same size");
  float rmsd=0.0;
  typename Vecto3DsOrXYZs0::const_iterator it0= m1.begin();
  typename Vecto3DsOrXYZs1::const_iterator it1= m2.begin();
  for(; it0!= m1.end(); ++it0, ++it1) {
    algebra::VectorD<3> tred
      =tr_for_second.get_transformed(get_vector_d_geometry(*it1));
    rmsd += algebra::get_squared_distance(get_vector_d_geometry(*it0),
                                      tred);
  }
  return std::sqrt(rmsd / m1.size());
}


//! Computes the native overlap between two sets of 3D points
/**
  \param[in] m1 first set
  \param[in] m2 second set
  \param[in] threshold threshold distance (amstrongs) for the calculation
  \note The result is returned as a percentage (from 0 to 100)
  \note the function assumes correspondence between two sets of points and does
  not perform rigid alignment.
  \genericgeometry
**/
template <class Vecto3DsOrXYZs0, class Vecto3DsOrXYZs1>
double get_native_overlap(const Vecto3DsOrXYZs0& m1,
                      const Vecto3DsOrXYZs1& m2,double threshold) {
  IMP_USAGE_CHECK(m1.size()==m2.size(),
            "native_verlap: The input sets of XYZ points "
            <<"should be of the same size");
  unsigned int distances=0;
  for(unsigned int i=0;i<m1.size();i++) {
    double d = algebra::get_distance(get_vector_d_geometry(m1[i]),
                                     get_vector_d_geometry(m2[i]));
    if(d<=threshold) distances++;
  }
  return 100.0*distances/m1.size();
}


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
IMPATOMEXPORT FloatPair
get_placement_score(const core::XYZs& from,
                    const core::XYZs& to);

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
IMPATOMEXPORT FloatPair get_component_placement_score(
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
IMPATOMEXPORT double get_pairwise_rmsd_score(
      const core::XYZs& ref1 ,const core::XYZs& ref2,
      const core::XYZs& mdl1 ,const core::XYZs& mdl2);


/** Compute the radius of gyration of a set of particles with (optional)
    radii and mass and (non-optional) coordinates. Either all particles
    must have mass or none of them.
*/
IMPATOMEXPORT double get_radius_of_gyration(const ParticlesTemp &ps);

//! Fast rmsd calculation. Used to calculate rmsd
//! between multiple transformation that operate on the same particles
class IMPATOMEXPORT RMSDCalculator {
public:
  RMSDCalculator() {}
  //! Constructor
  /**
   \param[in] the particles on which the transformation operate.
               RMSD will be calculate on these particles.
   */
  RMSDCalculator(const ParticlesTemp &ps);

  //! Get rmsd between two transformations
  float get_rmsd(const algebra::Transformation3D& t1,
                 const algebra::Transformation3D& t2) {
    return sqrt(get_squared_rmsd(t1, t2));}

  //! Get the squared rmsd between two transformations
  float get_squared_rmsd(const algebra::Transformation3D& t1,
              const algebra::Transformation3D& t2);

protected:
  algebra::Vector3D centroid_;
  double d_[3][3];//partial calculation
  //for example dist[0][1] is the dot product of a two vectors of lenght N
  //one of all X coordiantes and the second of all Y coordiantes.
  //N is the number of particles
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DISTANCE_H */
