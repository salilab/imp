/**
 *  \file IMP/atom/distance.h
 *  \brief distance metrics
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DISTANCE_H
#define IMPATOM_DISTANCE_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/XYZ.h>
#include "Hierarchy.h"
#include "Selection.h"
#include "IMP/base_types.h"

IMPATOM_BEGIN_NAMESPACE

//! Calculate the root mean square deviation between two sets of 3D points.
/**
   \note the function assumes correspondence between the two sets of
   points and does not perform rigid alignment.

   \genericgeometry
 */
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_rmsd(const Vector3DsOrXYZs0& m1, const Vector3DsOrXYZs1& m2,
                const IMP::algebra::Transformation3D &tr_for_second
                = IMP::algebra::get_identity_transformation_3d()) {
  IMP_USAGE_CHECK(std::distance(m1.begin(), m1.end())
                  ==std::distance(m2.begin(), m2.end()),
            "The input sets of XYZ points "
            <<"should be of the same size");
  double rmsd=0.0;
  typename Vector3DsOrXYZs0::const_iterator it0= m1.begin();
  typename Vector3DsOrXYZs1::const_iterator it1= m2.begin();
  for(; it0!= m1.end(); ++it0, ++it1) {
    algebra::Vector3D tred
      =tr_for_second.get_transformed(get_vector_d_geometry(*it1));
    rmsd += algebra::get_squared_distance(get_vector_d_geometry(*it0),
                                      tred);
  }
  return std::sqrt(rmsd / m1.size());
}

/** RMSD on a pair of Selections.*/
inline double get_rmsd(const Selection &s0,
                       const Selection &s1,
                       const IMP::algebra::Transformation3D &tr_for_second
                       = IMP::algebra::get_identity_transformation_3d()) {
  return get_rmsd(s0.get_selected_particles(),
                  s1.get_selected_particles(), tr_for_second);
}

//! Calculate the root mean square deviation between two sets of 3D points.
/**
   \note the function assumes correspondence between the two sets of
   points and does not need rigid alignment. Note that it is different from
   get_drms().

   \genericgeometry
 */
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_drmsd(const Vector3DsOrXYZs0& m0, const Vector3DsOrXYZs1& m1)
  {
  IMP_USAGE_CHECK(m0.size()==m1.size(),
            "The input sets of XYZ points "
            <<"should be of the same size");
  double drmsd=0.0;

  int npairs = 0;
  for(unsigned i=0;i<m0.size()-1;++i){
       algebra::Vector3D v0i=get_vector_d_geometry(m0[i]);
       algebra::Vector3D v1i=get_vector_d_geometry(m1[i]);

       for(unsigned j=i+1;j<m0.size();++j){
             algebra::Vector3D v0j=get_vector_d_geometry(m0[j]);
             algebra::Vector3D v1j=get_vector_d_geometry(m1[j]);

             double dist0=algebra::get_distance(v0i,v0j);
             double dist1=algebra::get_distance(v1i,v1j);
             drmsd+=(dist0-dist1)*(dist0-dist1);
             npairs++;
       }
    }
  return std::sqrt(drmsd/npairs);
}


//! Computes the native overlap between two sets of 3D points
/**
  \param[in] m1 first set
  \param[in] m2 second set
  \param[in] threshold threshold distance (angstroms) for the calculation
  \note the function assumes correspondence between two sets of points and does
  not perform rigid alignment.
  \return the native overlap, as a percentage (from 0 to 100)
  \genericgeometry
**/
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_native_overlap(const Vector3DsOrXYZs0& m1,
                      const Vector3DsOrXYZs1& m2,double threshold) {
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


/*! Distance-RMS between two sets of points, defined as:
    sqrt( sum[ (d1ij**2 - d2ij**2)**2]/(4 * sum[d1ij**2]) )
    (Levitt, 1992)
    d1ij - distance between points i and j in set m1 (the "reference" set)
    d2ij - distance between points i and j in set m2
  \param[in] m1 set of points
  \param[in] m2 set of points
*/
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_drms(const Vector3DsOrXYZs0& m1,
                       const Vector3DsOrXYZs1& m2) {
  IMP_USAGE_CHECK(m1.size()==m2.size(),
            "native_verlap: The input sets of XYZ points "
            <<"should be of the same size");

  unsigned int n = m1.size();
  double drms = 0.0;
  double sum_d1ij = 0.0;
  for (unsigned int i=0; i < n; ++i) {
    for (unsigned int j= i+1; j< n; ++j) {
      double sqd1 = algebra::get_squared_distance(get_vector_d_geometry(m1[i]),
                                          get_vector_d_geometry(m1[j]));
      double sqd2 = algebra::get_squared_distance(get_vector_d_geometry(m2[i]),
                                          get_vector_d_geometry(m2[j]));
      drms += (sqd1 - sqd2)*(sqd1 - sqd2);
      sum_d1ij += sqd1;
    }
  }
  drms /= (4 * sum_d1ij);
  return sqrt(drms);
}


/*! DRMS between to sets of rigid bodies. Points ij belonging to the same
rigid body are not evaluated, as they are the same in both sets
  \param[in] m1 set of points
  \param[in] m2 set of points
  \param[in] ranges of points considered to be the same rigid body. Eg,
        (0-29,30-49) means two rigid bodies, first with the 30 first atoms,
        second with the last 20
*/
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_rigid_bodies_drms(const Vector3DsOrXYZs0 &m1,
                                    const Vector3DsOrXYZs1 &m2,
                                    const IMP::IntRanges &ranges) {
  IMP_USAGE_CHECK(m1.size() == m2.size(),
                  "get_rigid_bodies_drms: The input sets of XYZ points "
                  <<"should be of the same size");
  int n = m1.size();
  int rn = ranges.size();
  double drms = 0.0;
  double sum_d1ij = 0.0;
  for ( int i=0; i < n; ++i) {
    int range1 = -1;
    for ( int k=0; k < rn; ++k) {
      if(i >= ranges[k].first && i <= ranges[k].second) {
        range1 = k;
        break;
      }
    }
    IMP_USAGE_CHECK(range1 >=0, "Point " << i << " of m1 does not belong to "
                   "any range");
    for ( int j= i+1; j < n; ++j) {
      int range2 = -1;
      for ( int k=0; k < rn; ++k) {
        if(j >= ranges[k].first && j <= ranges[k].second) {
          range2 = k;
          break;
        }
      }
      IMP_USAGE_CHECK(range2 >=0, "Point " << j << " of m2 does not belong to "
               "any range");

      double sqd1 = algebra::get_squared_distance(get_vector_d_geometry(m1[i]),
                                                  get_vector_d_geometry(m1[j]));
      sum_d1ij += sqd1;
      if(range1 != range2) {
        // points i and j in different ranges compare distances
        double sqd2 = algebra::get_squared_distance(
                                                get_vector_d_geometry(m2[i]),
                                                get_vector_d_geometry(m2[j]));
        drms += (sqd1 - sqd2) * (sqd1 - sqd2);
      }
    }
  }
  drms /= (4 * sum_d1ij);
  return sqrt(drms);
}




//! Measure the difference between two placements of the same set of points
/**
   \param[in] source The reference placement represented by XYZ coordinates
   \param[in] target The modeled placement represented by XYZ coordinates
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
get_placement_score(const core::XYZs& source,
                    const core::XYZs& target);

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

/** \brief Used to calculate rmsd
    between multiple transformation that operate on the same particles

    The constructor takes a list of particles defining the thing which
    is having its rmsd calculated.
*/
class IMPATOMEXPORT RMSDCalculator {
public:
  RMSDCalculator() {}
  //! Constructor
  /**
   \param[in] ps the particles on which the transformation operate.
               RMSD will be calculate on these particles.
   */
  RMSDCalculator(const ParticlesTemp &ps);

  //! Get rmsd between two transformations
  /** The number returned is equilavent to the pseudo code
\code
    get_rmsd(t1.get_transformed(ps),
             t2.get_transformed(ps));
\endcode
      where ps is the list of particles passed to the constructor.
 */
  double get_rmsd(const algebra::Transformation3D& t1,
                 const algebra::Transformation3D& t2) {
    return sqrt(get_squared_rmsd(t1, t2));}

  //! Get the squared rmsd between two transformations
  double get_squared_rmsd(const algebra::Transformation3D& t1,
              const algebra::Transformation3D& t2);
  IMP_SHOWABLE_INLINE(RMSDCalculator, out << centroid_);
private:
  algebra::Vector3D centroid_;
  double d_[3][3];//partial calculation
  //for example dist[0][1] is the dot product of a two vectors of lenght N
  //one of all X coordiantes and the second of all Y coordiantes.
  //N is the number of particles
};

IMP_VALUES(RMSDCalculator, RMSDCalculators);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DISTANCE_H */
