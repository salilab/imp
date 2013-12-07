/**
 * \file sinc_function \brief caching of sinc values
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_INTERNAL_VARIANCE_HELPERS_H
#define IMPSAXS_INTERNAL_VARIANCE_HELPERS_H

#include <IMP/saxs/saxs_config.h>
#include "sinc_function.h"
#include <cmath>
#include <cfloat>
#include <boost/math/special_functions/erf.hpp>
#include <complex>
#include <boost/math/special_functions/sinc.hpp>
#include <exception>
#include <IMP/macros.h>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

inline std::complex<double> erf(std::complex<double> z, unsigned trunc_lo = 32,
                                unsigned trunc_hi = 193, double sep = 80.) {

  const double pi = 3.1415926535897931;
  const double sqrtpi = 1.772453850905516027298;
  const std::complex<double> i(0, 1);

  if (std::abs(z) <= sep) {
    // infinite series approximation, abramowitz p. 313 7.1.29
    double x = std::real(z);
    double y = std::imag(z);

    double s1 = boost::math::erf(x);

    double k1 = 2. / pi * std::exp(-x * x);
    std::complex<double> k2 = std::exp(-2. * i * x * y);

    std::complex<double> s2;
    if (x == 0) {
      s2 = i / pi * y;
    } else {
      s2 = k1 / (4 * x) * (1. - k2);
    }

    std::complex<double> retval = s1 + s2;

    if (y != 0) {
      std::complex<double> s5 = 0;
      for (unsigned n = 1; n <= trunc_lo; n++) {
        double enn = (double)n;
        double s3 = std::exp(-enn * enn / 4.) / (enn * enn + 4 * x * x);
        std::complex<double> s4 = 2 * x - k2 * (2 * x * std::cosh(enn * y) -
                                                i * enn * std::sinh(enn * y));
        s5 += s3 * s4;
      }
      retval += k1 * s5;
    }
    return retval;

  } else {
    // asymptotic expansion, abramowitz p. 312 7.1.23
    bool isneg = (std::real(z) < 0);
    if (isneg) z = -z;

    std::complex<double> s = 1.;
    std::complex<double> y = 2. * z * z;
    for (int n = (int)trunc_hi; n >= 1; n -= 2) {
      // std::cout << s << std::endl;
      s = 1. - double(n) * (s / y);
    }

    std::complex<double> retval = 1. - s * std::exp(-z * z) / (sqrtpi * z);
    // std::cout << std::exp(-z*z)/(sqrtpi*z) << " " << retval <<
    // std::endl;

    if (isneg) {
      z = -z;
      retval = -retval;
    }

    if (std::real(z) == 0) retval -= 1.;

    return retval;
  }
}

// experfc[z] = Exp[z^2]*(1-Erf[z])
inline std::complex<double> experfc(std::complex<double> z,
                                    unsigned trunc_lo = 32,
                                    unsigned trunc_hi = 193, double sep = 8.) {

  const double pi = 3.1415926535897931;
  const double sqrtpi = 1.772453850905516027298;
  const std::complex<double> i(0, 1);

  if (sep < 0 || std::abs(z) <= sep) {
    // infinite series approximation, abramowitz p. 313 7.1.29
    double x = std::real(z);
    double y = std::imag(z);

    std::complex<double> ez2(std::exp(z * z));

    double s1 = boost::math::erf(x);

    double k1 = std::exp(-y * y);
    std::complex<double> k2 = std::exp(+2. * i * x * y);

    std::complex<double> s2;
    if (x == 0) {
      s2 = i / pi * y * k1;
    } else {
      s2 = k1 / (2 * pi * x) * (k2 - 1.);
    }

    std::complex<double> retval = ez2 * s1 + s2;

    if (y != 0) {
      std::complex<double> s5 = 0;
      for (unsigned n = 1; n <= trunc_lo; n++) {
        double enn = (double)n;
        double s3 = std::exp(-enn * enn / 4.) / (enn * enn + 4 * x * x);
        std::complex<double> s4 =
            2 * x * k1 * k2 - (i * enn / 2. + x) * std::exp(-y * (enn + y)) -
            (-i * enn / 2. + x) * std::exp(y * (enn - y));
        s5 += s3 * s4;
        // std::cout << s3 << " " << s4 << " " << s5 << std::endl;
      }
      retval += 2. / pi * s5;
    }
    // std::cout << z << ez2 << " " << s1 << " " << s2 << " " << retval <<
    // std::endl;
    return ez2 - retval;

  } else {
    // asymptotic expansion, abramowitz p. 312 7.1.23
    bool isneg = (std::real(z) < 0);
    if (isneg) z = -z;

    std::complex<double> s = 1.;
    std::complex<double> y = 2. * z * z;
    for (int n = (int)trunc_hi; n >= 1; n -= 2) {
      // std::cout << s << std::endl;
      s = 1. - double(n) * (s / y);
    }

    std::complex<double> retval = s / (sqrtpi * z);
    // std::cout << 1./(sqrtpi*z) << " " << retval << std::endl;

    if (isneg) {
      z = -z;
      retval = 2. - retval;
    }

    // if (std::real(z)==0) retval += 1.;

    return retval;
  }
}

// Faddeeva function w(z) = exp(-z^2)*erfc(-i*z)
// implementation of TOMS algorithm 680 (accurate to 10^-14)
// adapted from fortran code provided as supplementary material
inline std::complex<double> w(std::complex<double> z) {

  // define constants
  const double factor = 1.12837916709551257388;
  const double rmaxreal = std::sqrt(DBL_MAX);
  const double rmaxexp = std::log(DBL_MAX / 2.);
  const double rmaxgoni = DBL_MAX;

  // define variables
  double xi = std::real(z);
  double yi = std::imag(z);
  double u, v;  // return values
  bool a, b;
  double xabs = std::abs(xi);
  double yabs = std::abs(yi);
  double x = xabs / 6.3;
  double y = yabs / 4.4;
  double u2(0), v2(0);

  // protect qrho against overflow
  if (xabs > rmaxreal)
    throw std::overflow_error("overflow in w(z): xabs > rmaxreal");
  if (yabs > rmaxreal)
    throw std::overflow_error("overflow in w(z): yabs > rmaxreal");

  double qrho = x * x + y * y;

  double xabsq = xabs * xabs;
  double xquad = xabsq - yabs * yabs;
  double yquad = 2 * xabs * yabs;

  a = (qrho < 0.085264);
  /*std::cout << "xabs " << xabs
            << " yabs " << yabs
            << " qrho " << qrho
            << " a " << a
            << std::endl;*/
  if (a) {
    // evaluate faddeeva-function using abramowitz 7.1.5, truncating at N
    qrho = (1 - 0.85 * y) * std::sqrt(qrho);
    unsigned n = algebra::get_rounded(6 + 72 * qrho);
    unsigned j = 2 * n + 1;
    double xsum = 1.0 / double(j);
    double ysum = 0;
    /*std::cout << "case A" << std::endl;
    std::cout
          << "qrho " << qrho
          << " n " << n
          << " j " << j
          << " xsum " << xsum
          << " ysum " << ysum
          << std::endl;*/

    for (unsigned i = n; i >= 1; i--) {
      j -= 2;
      double xaux = (xsum * xquad - ysum * yquad) / double(i);
      ysum = (xsum * yquad + ysum * xquad) / double(i);
      xsum = xaux + 1 / double(j);
    }
    /*std::cout
          << " xsum " << xsum
          << " ysum " << ysum
          << std::endl;*/
    double u1 = -factor * (xsum * yabs + ysum * xabs) + 1.;
    double v1 = factor * (xsum * xabs - ysum * yabs);
    double daux = std::exp(-xquad);
    u2 = daux * std::cos(yquad);
    v2 = -daux * std::sin(yquad);
    /*std::cout
          << " u1 " << u1
          << " v1 " << v1
          << " daux " << daux
          << " u2 " << u2
          << " v2 " << v2
          << std::endl;*/

    u = u1 * u2 - v1 * v2;
    v = u1 * v2 + v1 * u2;

  } else {
    double h, h2(0);
    int kapn, nu;
    if (qrho > 1.) {
      // evaluate w(z) using laplace continued fraction, up to nu terms
      h = 0;
      kapn = 0;
      qrho = std::sqrt(qrho);
      nu = (int)algebra::get_rounded(3 + 1442. / (26 * qrho + 77));
    } else {
      // truncated taylor expansion, using laplace continued fraction
      // to compute derivatives of w(z).
      // kapn : minimum number of terms in taylor expansion
      // nu : minimum number of terms in continued fraction
      qrho = (1 - y) * std::sqrt(1 - qrho);
      h = 1.88 * qrho;
      h2 = 2 * h;
      kapn = (int)algebra::get_rounded(7 + 34 * qrho);
      nu = (int)algebra::get_rounded(16 + 26 * qrho);
      /*std::cout << "case B2" << std::endl;
          std::cout
              << "qrho " << qrho
              << " h " << h
              << " h2 " << h2
              << " kapn " << kapn
              << " nu " << nu
              << std::endl;*/
    }

    double qlambda(0);
    b = (h > 0);
    if (b) qlambda = std::pow(h2, kapn);
    // std::cout << qlambda << std::endl;

    double rx(0), ry(0), sx(0), sy(0);

    for (int n = nu; n >= 0; n--) {
      double np1 = double(n) + 1;
      double tx = yabs + h + np1 * rx;
      double ty = xabs - np1 * ry;
      double c = 0.5 / (tx * tx + ty * ty);
      rx = c * tx;
      ry = c * ty;
      if ((b) && (n <= kapn)) {
        tx = qlambda + sx;
        sx = rx * tx - ry * sy;
        sy = ry * tx + rx * sy;
        qlambda = qlambda / h2;
      }
    }
    /*std::cout
        << "rx " << rx
        << " ry " << ry
        << " sx " << sx
        << " sy " << sy
        << std::endl;*/

    if (h == 0) {
      u = factor * rx;
      v = factor * ry;
    } else {
      u = factor * sx;
      v = factor * sy;
    }

    if (yabs == 0) u = std::exp(-xabs * xabs);
  }

  // evaluation in the other quadrants
  if (yi < 0) {
    if (a) {
      u2 = 2 * u2;
      v2 = 2 * v2;
    } else {
      xquad = -xquad;

      // protect agains 2*exp(-z**2) overflow
      if (yquad > rmaxgoni)
        throw std::overflow_error("overflow in w(z): yquad > rmaxgoni");
      if (xquad > rmaxexp)
        throw std::overflow_error("overflow in w(z): xquad > rmaxexp");

      double w1 = 2 * std::exp(xquad);
      u2 = w1 * std::cos(yquad);
      v2 = -w1 * std::sin(yquad);
    }

    u = u2 - u;
    v = v2 - v;

    if (xi > 0) v = -v;
  } else {
    if (xi < 0) v = -v;
  }

  return std::complex<double>(u, v);
}

// C(a,b,c) = exp(-(a^2+b^2)) * (
//              sqrt(pi)*Exp[-2ab]*Im[ Exp[-2i(a+b)c] * w(-c+i(a+b)) ]
//              + 1/c * cos(2(a+b)c)
//              )
inline double C(double a, double b, double c) {
  const double sqrtpi = 1.7724538509055160273;
  // compute left part
  std::complex<double> i(0, 1);
  std::complex<double> z(-c, a + b);
  std::complex<double> fad = w(z);
  fad *= std::exp(-2 * (a + b) * c * i);
  double left = sqrtpi * std::imag(fad) * std::exp(-2 * a * b);
  // compute right part
  double right = std::cos(2 * (a + b) * c) / c;
  /*std::cout << " a= " << a
            << " b= " << b
            << " c= " << c
      << " left= " << left << " right= " << right << std::endl;*/
  // exponentiate and return
  double retval = std::exp(-a * a - b * b);
  return retval * (left + right);
}

// A(a,b,c) = 1/(8abc) * (C(a,b,c) - C(a,-b,c))
inline double A(double a, double b, double c) {
  double c1 = C(a, b, c);
  double c2 = C(a, -b, c);
  std::cout << " a= " << a << " b= " << b << " c= " << c << " c1= " << c1
            << " c2= " << c2 << " c1-c2= " << c1 - c2 << std::endl;
  return 1. / (8 * a * b * c) * (c1 - c2);
}

// univariate normal distribution PDF
inline double uninorm(double delta, double var) {
  const double pi = 3.141592653589793;
  return std::exp(-square(delta) / (2 * var)) / (std::sqrt(2 * pi * var));
}

// bivariate normal distribution with same variance
inline double binorm(double dx, double dy, double var, double rho) {
  const double pi = 3.141592653589793;
  double z = (square(dx) + square(dy) - 2 * rho * (dx) * (dy)) / var;
  return std::exp(-z / (2 * (1 - rho * rho))) /
         (2 * pi * var * std::sqrt(1 - rho * rho));
}

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_VARIANCE_HELPERS_H */
