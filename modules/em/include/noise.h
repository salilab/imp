/**
 *  \file noise.h
 *  \brief Management of IMP images for Electron Microscopy
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_NOISE_H
#define IMPEM_NOISE_H

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
  * \note Tested with MultiArray, Matrix2D and Matrix3D
  */
//template<typename T>
//void add_noise(T& data, double op1, double op2,
//               const String& mode = "uniform", double df = 3)
//{
//  if (mode == "uniform") {
//    // Distribution
//    boost::uniform_real<> unif_dist(op1, op2);
//    // Put generator and distribution together
//    typedef boost::variate_generator < RandomNumberGenerator,
//                      boost::uniform_real<> > rnd_unif_generator;
//    rnd_unif_generator rnd_unif(random_number_generator, unif_dist);
//
//    Int D = data.dimensionality;
//    // add noise
//    std::vector<Int> idx(D); // access index
//    while (IMP::algebra::internal::roll_inds(idx,
//                                             data.shape(),
//                                             data.index_bases())) {
//      data(idx) += rnd_unif();
//    }
//  } else if (mode == "gaussian") {
//    // Distribution
//    boost::normal_distribution<> gauss_dist(op1, op2);
//    // Put generator and distribution together
//    typedef boost::variate_generator < RandomNumberGenerator,
//    boost::normal_distribution<> > rnd_gauss_generator;
//    rnd_gauss_generator rnd_gauss(random_number_generator, gauss_dist);
//
//    Int D = data.dimensionality;
//    // add noise
//    std::vector<Int> idx(D); // access index
//    while (IMP::algebra::internal::roll_inds(idx,
//                                             data.shape(),
//                                             data.index_bases())) {
//      data(idx) += rnd_gauss();
//    }
//  } else if (mode == "student") {
//  } else {
//    String msg = "MultiArray::add_noise: Mode " + mode + " not supported.";
//    throw ErrorException(msg.c_str());
//  }
//}

template<typename T>
void add_noise(T& v, double op1, double op2,
               const String& mode = "uniform", double df = 3)
{
  // Generator
//  typedef boost::rand48 base_generator_type;
  typedef boost::mt19937 base_generator_type;
  base_generator_type generator;
  generator.seed(static_cast<unsigned long>(std::time(NULL)));
  // Distribution types
  typedef boost::uniform_real<> unif_distribution;
  typedef boost::normal_distribution<> norm_distribution;
  // Variate generators (put generator and distributions together)
  typedef boost::variate_generator < base_generator_type&,
                         unif_distribution  > unif_var_generator;
  typedef boost::variate_generator < base_generator_type&,
                         norm_distribution  > norm_var_generator;
  if (mode == "uniform") {
    unif_distribution dist(op1, op2);
    unif_var_generator random_var(generator,dist);
    // Add the random numbers
    for (unsigned long i=0;i<v.num_elements();++i) {v.data()[i]+=random_var();}
  } else if (mode == "gaussian") {
    norm_distribution dist(op1, op2);
    norm_var_generator random_var(generator, dist);
    // Add the random numbers
    for (unsigned long i=0;i<v.num_elements();++i) {v.data()[i]+=random_var();}
  } else if (mode == "student") {
  } else {
    IMP_THROW("MultiArray::add_noise: Mode " + mode + " not supported.",
              UsageException);
  }


}

IMPEM_END_NAMESPACE
#endif  /* IMPEM_NOISE_H */
