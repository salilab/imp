/**
 * \file sinc_function \brief caching of sinc values
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSAXS_INTERNAL_ERF_FUNCTION_H
#define IMPSAXS_INTERNAL_ERF_FUNCTION_H

#include "../saxs_config.h"
#include <cmath>
#include <boost/math/special_functions/erf.hpp>
#include <complex>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

class ErfFunction : public std::vector<std::complex<double> > {
public:
  // Constructor
  ErfFunction() {
      /*
    bin_size_ = bin_size;
    one_over_bin_size_ = 1.0/bin_size_;
    max_value_ = max_value;
    unsigned int size = value2index(max_value_) + 1;
    reserve(size);
    for(unsigned int i=0; i<=size; i++) {
      float x = i*bin_size_;
      push_back(boost::math::sinc_pi(x));
    }
    */
  }

  /*
  unsigned int value2index(float value) const {
    return IMP::algebra::get_rounded(value * one_over_bin_size_ );
  }

  // get sinc value for x, compute values if they weren't computed yet
  float sinc(float x) {
    unsigned int index = value2index(x);
    if(index >= size()) {
      reserve(index);
      for(unsigned int i=size(); i<=index; i++) {
        float x = i*bin_size_;
        push_back(boost::math::sinc_pi(x));
      }
    }
    return (*this)[index];
  }
  */
  inline static std::complex<double> erf(std::complex<double> z,
          unsigned trunc_lo = 32, unsigned trunc_hi = 193){

      const double pi=3.1415926535897931;
      const double sqrtpi=1.772453850905516027298;
      const std::complex<double> i(0,1);

      if (std::abs(z) <= 8)
      {
          double x = std::real(z);
          double y = std::imag(z);

          double s1 = boost::math::erf(x);

          double k1 = 2./pi*std::exp(-x*x);
          std::complex<double> k2 = std::exp(-2.*i*x*y);

          std::complex<double> s2;
          if (x==0){
              s2 = i/pi * y;
          } else {
              s2 = k1/(4*x) * (1.-k2);
          }

          std::complex<double> retval = s1 + s2;

          if (y!=0){
              std::complex<double> s5=0;
              for (unsigned n=1; n<=trunc_lo; n++){
                  double enn=(double)n;
                  double s3 = std::exp(-enn*enn/4.) / (enn*enn + 4*x*x);
                  std::complex<double> s4 = 2*x-k2*(2*x*std::cosh(enn*y)
                                             - i*enn*std::sinh(enn*y));
                  s5 += s3*s4;
              }
              retval += k1*s5;
          }
          return retval;

      } else {
          bool isneg = (std::real(z)<0);
          if (isneg) z = -z;

          std::complex<double> s=1.;
          std::complex<double> y = 2.*z*z;
          for (int n= (int)trunc_hi; n>=1; n -= 2) s=1.-double(n)*(s/y);

          std::complex<double> retval = 1.-s*std::exp(-z*z)/(sqrtpi*z);

          if (isneg){
              z= -z;
              retval= -retval;
          }

          if (std::real(z)==0) retval -= 1.;

          return retval;

      }
  }

private:
  //float bin_size_, one_over_bin_size_; // resolution of discretization
  //float max_value_;
};

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_ERF_FUNCTION_H */
