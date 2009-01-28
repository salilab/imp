/**
 *  \file EMnoise.h
 *  \brief Management of IMP images for Electron Microscopy
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EMNOISE_H
#define IMPEM_EMNOISE_H

#include "config.h"
#include "ImageHeader.h"
#include "IMP/algebra/MultiArray.h"
#include "IMP/algebra/Matrix2D.h"
#include "boost/random.hpp"
#include "boost/version.hpp"
#if BOOST_VERSION > 103700
  #include "boost/math/distributions/students_t.hpp"
#endif

IMPEM_BEGIN_NAMESPACE

//!Add noise to actual values of the image.
/**
  * Adds random noise to the image array. Supported distributions:
  * - uniform distribution, giving the range (lower, upper). DEFAULT
  * - gaussian distribution, giving the mean and the standard deviation
  * \todo - t-Student distribuion giving the mean, standard deviation and
  * the degrees of freedom
  *
  * \code
  * add_noise(v1,0, 1);
  * // uniform distribution between 0 and 1
  *
  * v1.add_noise(0, 1, "uniform");
  * // the same
  *
  * v1.add_noise(0, 1, "gaussian");
  * // gaussian distribution with 0 mean and stddev=1
  *
  * \endcode
  * \note Tested with MuiltiArray, Matrix2D and Matrix3D
  */
template<typename T>
void add_noise(T& data, Float op1, Float op2,
               const String& mode = "uniform", Float df = 3)
{
  if (mode == "uniform") {
    // Distribution
    boost::uniform_real<> unif_dist(op1, op2);
    // Put generator and distribution together
    typedef boost::variate_generator < RandomNumberGenerator,
    boost::uniform_real<> > rnd_unif_generator;
    rnd_unif_generator rnd_unif(random_number_generator, unif_dist);

    Int D = data.dimensionality;
    // add noise
    std::vector<Int> idx(D); // access index
    while (roll_inds(idx, data.shape(), data.index_bases())) {
      data(idx) += rnd_unif();
    }
  } else if (mode == "gaussian") {
    // Distribution
    boost::normal_distribution<> gauss_dist(op1, op2);
    // Put generator and distribution together
    typedef boost::variate_generator < RandomNumberGenerator,
    boost::normal_distribution<> > rnd_gauss_generator;
    rnd_gauss_generator rnd_gauss(random_number_generator, gauss_dist);

    Int D = data.dimensionality;
    // add noise
    std::vector<Int> idx(D); // access index
    while (roll_inds(idx, data.shape(), data.index_bases())) {
      data(idx) += rnd_gauss();
    }
  } else if (mode == "student") {
  } else {
    String msg = "MultiArray::add_noise: Mode " + mode + " not supported.";
    throw ErrorException(msg.c_str());
  }
}


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EMNOISE_H */
