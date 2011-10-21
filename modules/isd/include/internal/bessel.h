/**
 * \file bessel.h
 * \brief Modified Bessel functions of the first kind of orders zero and one.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_INTERNAL_BESSEL_H
#define IMPISD_INTERNAL_BESSEL_H

#include "../isd_config.h"
#include <cmath>

IMPISD_BEGIN_INTERNAL_NAMESPACE

inline double poly(const double p[], size_t count, double y) {
  double sum = p[count - 1];
  for (int i = count - 2; i >= 0; --i) {
    sum *= y;
    sum += p[i];
  }
  return sum;
}

//! Approximation to modified Bessel function of the first kind of order zero
/** See Blair and Edwards, Chalk River Report AECL-4928, 1974.
    To be replaced by boost::math::cyl_bessel_i(0, kappa) when IMP requires
    a newer version of Boost.
 */
inline double bessel_i0(double kappa) {
  static const double P1[] = {
    -2.2335582639474375249e+15L, -5.5050369673018427753e+14L,
    -3.2940087627407749166e+13L, -8.4925101247114157499e+11L,
    -1.1912746104985237192e+10L, -1.0313066708737980747e+08L,
    -5.9545626019847898221e+05L, -2.4125195876041896775e+03L,
    -7.0935347449210549190e+00L, -1.5453977791786851041e-02L,
    -2.5172644670688975051e-05L, -3.0517226450451067446e-08L,
    -2.6843448573468483278e-11L, -1.5982226675653184646e-14L,
    -5.2487866627945699800e-18L
  };
  static const double Q1[] = {
    -2.2335582639474375245e+15L, 7.8858692566751002988e+12L,
    -1.2207067397808979846e+10L, 1.0377081058062166144e+07L,
    -4.8527560179962773045e+03L, 1.0L
  };
  static const double P2[] = {
    -2.2210262233306573296e-04L, 1.3067392038106924055e-02L,
    -4.4700805721174453923e-01L, 5.5674518371240761397e+00L,
    -2.3517945679239481621e+01L, 3.1611322818701131207e+01L,
    -9.6090021968656180000e+00L
  };
  static const double Q2[] = {
    -5.5194330231005480228e-04L, 3.2547697594819615062e-02L,
    -1.1151759188741312645e+00L, 1.3982595353892851542e+01L,
    -6.0228002066743340583e+01L, 8.5539563258012929600e+01L,
    -3.1446690275135491500e+01L, 1.0L
  };

  if (kappa < 0.) {
    kappa = -kappa;
  }

  if (kappa == 0.) {
    return 1.;
  } else if (kappa <= 15.) {
    double y = kappa * kappa;
    return poly(P1, 15, y) / poly(Q1, 6, y);
  } else {
    double y = 1.0 / kappa - 1.0 / 15.0;
    return std::exp(kappa) / std::sqrt(kappa) * poly(P2, 7, y) / poly(Q2, 8, y);
  }
}

//! Approximation to modified Bessel function of the first kind of order one
/** See Blair and Edwards, Chalk River Report AECL-4928, 1974.
    To be replaced by boost::math::cyl_bessel_i(1, kappa) when IMP requires
    a newer version of Boost.
 */
inline double bessel_i1(double kappa) {
  static const double P1[] = {
    -1.4577180278143463643e+15L, -1.7732037840791591320e+14L,
    -6.9876779648010090070e+12L, -1.3357437682275493024e+11L,
    -1.4828267606612366099e+09L, -1.0588550724769347106e+07L,
    -5.1894091982308017540e+04L, -1.8225946631657315931e+02L,
    -4.7207090827310162436e-01L, -9.1746443287817501309e-04L,
    -1.3466829827635152875e-06L, -1.4831904935994647675e-09L,
    -1.1928788903603238754e-12L, -6.5245515583151902910e-16L,
    -1.9705291802535139930e-19L
  };
  static const double Q1[] = {
    -2.9154360556286927285e+15L, 9.7887501377547640438e+12L,
    -1.4386907088588283434e+10L, 1.1594225856856884006e+07L,
    -5.1326864679904189920e+03L, 1.0L
  };
  static const double P2[] = {
    1.4582087408985668208e-05L, -8.9359825138577646443e-04L,
    2.9204895411257790122e-02L, -3.4198728018058047439e-01L,
    1.3960118277609544334e+00L, -1.9746376087200685843e+00L,
    8.5591872901933459000e-01L, -6.0437159056137599999e-02L
  };
  static const double Q2[] = {
    3.7510433111922824643e-05L, -2.2835624489492512649e-03L,
    7.4212010813186530069e-02L, -8.5017476463217924408e-01L,
    3.2593714889036996297e+00L, -3.8806586721556593450e+00L,
    1.0L
  };

  if (kappa == 0.) {
    return 0.;
  }

  double val;
  double abskappa = std::abs(kappa);
  if (abskappa <= 15.) {
    double y = kappa * kappa;
    val = abskappa * poly(P1, 15, y) / poly(Q1, 6, y);
  } else {
    double y = 1.0 / abskappa - 1.0 / 15.0;
    val = std::exp(abskappa) / std::sqrt(abskappa)
          * poly(P2, 8, y) / poly(Q2, 7, y);
  }
  if (kappa < 0.) {
    val = -val;
  }
  return val;
}

IMPISD_END_INTERNAL_NAMESPACE

#endif /* IMPISD_INTERNAL_BESSEL_H */
