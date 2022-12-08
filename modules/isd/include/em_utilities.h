/**
 *  \file IMP/isd/em_utilities.h
 *  \brief Common scoring functions
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_EM_UTILITIES_H
#define IMPISD_EM_UTILITIES_H

#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/core/Gaussian.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/DensityHeader.h>
#include <IMP/random.h>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

IMPISD_BEGIN_NAMESPACE

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
inline Float score_gaussian_overlap(Model *m,
                             ParticleIndexPair pp,
                             Eigen::Vector3d * deriv){
  double determinant;
  bool invertible;
  Eigen::Matrix3d inverse = Eigen::Matrix3d::Zero();
  Float mass12 = atom::Mass(m,pp[0]).get_mass() *
                  atom::Mass(m,pp[1]).get_mass();
  core::Gaussian g1(m,pp[0]);
  core::Gaussian g2(m,pp[1]);
  Eigen::Matrix3d covar = g1.get_global_covariance() +
                               g2.get_global_covariance();
  Eigen::Vector3d v = Eigen::Vector3d(g2.get_coordinates().get_data())
    - Eigen::Vector3d(g1.get_coordinates().get_data());
  covar.computeInverseAndDetWithCheck(inverse,determinant,invertible);
  Eigen::Vector3d tmp = inverse*v;
  // 0.06349... = 1. / sqrt(2.0 * pi) ** 3
  Float score = mass12 * 0.06349363593424097 / (std::sqrt(determinant)) *
    std::exp(-0.5*v.transpose()*tmp);
  *deriv = -score*tmp;
  return score;
}
#endif

inline FloatsList sample_points_from_density(const em::DensityMap * dmap_orig,
                                      int npoints,
                                      Float threshold=0.0){
    // get sample region
    em::DensityMap * dmap = em::get_threshold_map(dmap_orig,threshold);
    dmap->calcRMS();
    const em::DensityHeader * dhead = dmap->get_header();
    Float dmax=dhead->dmax;
    algebra::BoundingBox3D bbox=em::get_bounding_box(dmap,0.00001);

    // setup random number generator
    FloatsList ret;
    boost::uniform_real<> uni_dist(0,1);
    boost::variate_generator<
         IMP::RandomNumberGenerator&, boost::uniform_real<> >
                uni(IMP::random_number_generator, uni_dist);
    for (int i=0;i<npoints;i++){
      algebra::Vector3D vs = algebra::get_random_vector_in(bbox);
      Float den=(dmap->get_value(vs))/dmax;
      Float t=uni();
      if (t<den) {
        Floats r;
        r.push_back(vs[0]);
        r.push_back(vs[1]);
        r.push_back(vs[2]);
        ret.push_back(r);
      }
    }
    return ret;
}

IMPISD_END_NAMESPACE

#endif  /* IMPISD_EM_UTILITIES_H */
