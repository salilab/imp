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

inline std::complex<double> erf(std::complex<double> z,
          unsigned trunc_lo = 32, unsigned trunc_hi = 193, double sep=80.){

      const double pi=3.1415926535897931;
      const double sqrtpi=1.772453850905516027298;
      const std::complex<double> i(0,1);

      if (std::abs(z) <= sep)
      {
          //infinite series approximation, abramowitz p. 313 7.1.29
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
          //asymptotic expansion, abramowitz p. 312 7.1.23
          bool isneg = (std::real(z)<0);
          if (isneg) z = -z;

          std::complex<double> s=1.;
          std::complex<double> y = 2.*z*z;
          for (int n= (int)trunc_hi; n>=1; n -= 2)
          {
              //std::cout << s << std::endl;
              s=1.-double(n)*(s/y);
          }

          std::complex<double> retval = 1.-s*std::exp(-z*z)/(sqrtpi*z);
          //std::cout << std::exp(-z*z)/(sqrtpi*z) << " " << retval <<
          //std::endl;

          if (isneg){
              z= -z;
              retval= -retval;
          }

          if (std::real(z)==0) retval -= 1.;

          return retval;

      }
  }

//experfc[z] = Exp[z^2]*(1-Erf[z])
inline std::complex<double> experfc(std::complex<double> z,
        unsigned trunc_lo = 32, unsigned trunc_hi = 193, double sep=8.){

      const double pi=3.1415926535897931;
      const double sqrtpi=1.772453850905516027298;
      const std::complex<double> i(0,1);

      if (sep < 0 || std::abs(z) <= sep)
      {
          //infinite series approximation, abramowitz p. 313 7.1.29
          double x = std::real(z);
          double y = std::imag(z);

          std::complex<double> ez2(std::exp(z*z));

          double s1 = boost::math::erf(x);

          double k1 = std::exp(-y*y);
          std::complex<double> k2 = std::exp(+2.*i*x*y);

          std::complex<double> s2;
          if (x==0){
              s2 = i/pi * y * k1;
          } else {
              s2 = k1/(2*pi*x) * (k2-1.);
          }

          std::complex<double> retval = ez2*s1 + s2;

          if (y!=0){
              std::complex<double> s5=0;
              for (unsigned n=1; n<=trunc_lo; n++){
                  double enn=(double)n;
                  double s3 = std::exp(-enn*enn/4.) / (enn*enn + 4*x*x);
                  std::complex<double> s4 = 2*x*k1*k2
                      - ( i*enn/2. + x)* std::exp(-y*(enn+y))
                      - (-i*enn/2. + x)* std::exp( y*(enn-y));
                  s5 += s3*s4;
                  //std::cout << s3 << " " << s4 << " " << s5 << std::endl;
              }
              retval += 2./pi*s5;
          }
          //std::cout << z << ez2 << " " << s1 << " " << s2 << " " << retval <<
          //std::endl;
          return ez2-retval;

      } else {
          //asymptotic expansion, abramowitz p. 312 7.1.23
          bool isneg = (std::real(z)<0);
          if (isneg) z = -z;

          std::complex<double> s=1.;
          std::complex<double> y = 2.*z*z;
          for (int n= (int)trunc_hi; n>=1; n -= 2)
          {
              //std::cout << s << std::endl;
              s=1.-double(n)*(s/y);
          }

          std::complex<double> retval = s/(sqrtpi*z);
          //std::cout << 1./(sqrtpi*z) << " " << retval << std::endl;

          if (isneg){
              z= -z;
              retval= 2.-retval;
          }

          //if (std::real(z)==0) retval += 1.;

          return retval;
      }
}


//computes s_jk = (-1)^j 2^(2j+1)/( k (1+2j-k)! (k-j-1)! )
//for 0 <= j < k integers
//s_jk = 0 when 2j+1 < k
class SjkHelper {
    public:

        SjkHelper() {
            std::vector<double> tmp;
            tmp.push_back(2.);
            sjk_.push_back(tmp); // s_01 = 2
        };

        //return s_jk
        //strategy: recurse to j+1,k as long as j < k-1
        //then recurse to k-2,k-1 until 0,1
        double get(unsigned j, unsigned k){
            //
            if (2*j+1 < k) return 0;
            //
            //check if element has already been computed
            if (sjk_.size() >=k && sjk_[k-1].size() >= k-j)
                return sjk_[k-1][k-j-1];
            //
            if (j<k-1) {
                //get s(j+1)k
                double val = get(j+1,k);
                //at that point, the vector has the correct length
                double dk = (double)k;
                double dj = (double)j;
                double factor = (2+2*dj-dk)*(3+2*dj-dk)/(4*(1+dj-dk));
                val *= factor;
                sjk_[k-1].push_back(val);
                return val;
            } else {
                //j == k-1, get s(k-2)(k-1)
                double val = get(k-2,k-1);
                double factor = -4./double(k);
                val *= factor;
                std::vector<double> tmp;
                tmp.push_back(val);
                sjk_.push_back(tmp);
                return val;
            }
        }

        //returns (s_0k, s_1k, ... s_(k-1)k)
        std::vector<double> get_all(unsigned k){
            //generate values if needed
            get( k/2, k);
            std::vector<double> ret(k/2, 0.);
            //ret.reserve(k);
            ret.insert(ret.end(), sjk_[k-1].rbegin(), sjk_[k-1].rend());
            return ret;
        }

    private:
        std::vector<std::vector<double> > sjk_;

};

//computes t_jk = (-1)^j 2^(2j)/((2j-k)! (k-j)! )
//for 1 <= j < k integers
//t_jk = 0 when 2j < k
class TjkHelper {
    public:

        TjkHelper() {
            std::vector<double> tmp;
            tmp.push_back(-4.);
            tjk_.push_back(tmp); // t_11 = -4
        };

        //return t_jk
        //strategy: recurse to j+1,k as long as j < k
        //then recurse to k-1,k-1 until 1,1
        double get(unsigned j, unsigned k){
            //
            if (2*j < k) return 0;
            //
            //check if element has already been computed
            if (tjk_.size() >=k && tjk_[k-1].size() >= k-j+1)
                return tjk_[k-1][k-j];
            //
            if (j<k) {
                //get t(j+1)k
                double val = get(j+1,k);
                //at that point, the vector has the correct length
                double dk = (double)k;
                double dj = (double)j;
                double factor = (1+2*dj-dk)*(2+2*dj-dk)/(4*(dj-dk));
                val *= factor;
                tjk_[k-1].push_back(val);
                return val;
            } else {
                //j == k, get t(k-1)(k-1)
                double val = get(k-1,k-1);
                double factor = -4./double(k);
                val *= factor;
                std::vector<double> tmp;
                tmp.push_back(val);
                tjk_.push_back(tmp);
                return val;
            }
        }

        //returns (t_1k, t_2k, ... t_kk)
        std::vector<double> get_all(unsigned k){
            //generate values if needed
            unsigned jmin = k/2+k%2;
            get(jmin, k);
            std::vector<double> ret(jmin-1, 0.);
            //ret.reserve(k);
            ret.insert(ret.end(), tjk_[k-1].rbegin(), tjk_[k-1].rend());
            return ret;
        }

    private:
        std::vector<std::vector<double> > tjk_;

};

//efficient computation of exp(z^2)*(erf(z+b) - erf(z-b))
//as 4/sqrt(pi) sum_{k=0}^{+\infty} c_{2k+1}(z) (b/2z)^{2k+1}
//truncated at kmax
//only needed for real b, but implementation compatible with complex
//when calling multiple times, try changing z as less often as possible
//
class ErfDiff {
    public:
        ErfDiff (unsigned kmax=100) : kmax_(kmax) {
            zval_=0.;
            cks(1.);
        };

        std::complex<double> get(std::complex<double> z, double b){
            const double tsqpi = 1.1283791670955125739; // 2/sqrt(pi)
            //compute Sum_k c_{2k+1}(z) (b/2z)^k
            std::complex<double> zsq = b/(2.*z);
            zsq *= zsq;
            std::vector<std::complex<double> > ckvals(cks(z));
            std::complex<double> tmp = ckvals.back();
            for (std::vector<std::complex<double> >::reverse_iterator
                    i = ckvals.rbegin()+1; i < ckvals.rend(); ++i){
                tmp = *i + tmp*zsq;
                //std::cout << z << " " << b << " " << *i << std::endl;
        }
            //multiply by missing terms
            std::complex<double> ret(tsqpi*b/z);
            return ret*tmp;
        }

    private:
        //c_k(z) = sum_{i=0}^{k-1} s_ik z^{2i+1}
        std::complex<double> ck(unsigned k, std::complex<double> z){
            std::vector<double> sjks = sjk_.get_all(k);
            std::complex<double> zsq = z*z;
            std::complex<double> tmp = sjks.back();
            for (std::vector<double>::reverse_iterator i = sjks.rbegin()+1;
                    i < sjks.rend(); ++i)
                tmp = *i + tmp*zsq;
            return z*tmp;
        }

        //return and memoize (c_1(z), ... , c_(2kmax+1)(z))
        std::vector<std::complex<double> >
        cks(std::complex<double> z){
            //std::cout << "memoize " << z << " " << zval_
            //    << " " << std::abs(zval_-z)/std::abs(z) << std::endl;
            if (std::abs(zval_-z)/std::abs(z) > 1e-6){
                //recalculate values
                cks_.clear();
                cks_.reserve(kmax_+1);
                for (unsigned i=1; i<= 2*kmax_+1; i+= 2){
                    cks_.push_back(ck(i,z));
                    //std::cout << cks_.back() << std::endl;
                }
                zval_ = z;
            }
            return cks_;
        }

        unsigned kmax_;
        std::complex<double> zval_;
        std::vector<std::complex<double> > cks_;
        SjkHelper sjk_;
};


//faddeeva function w(z) = exp(-z^2)*erfc(-i*z)
//implementation of TOMS algorithm 680 (accurate to 10^-14)
//adapted from fortran code provided as supplementary material
inline std::complex<double> w(std::complex<double> z){

    //define constants
    const double factor = 1.12837916709551257388;
    const double rmaxreal = std::sqrt(DBL_MAX);
    const double rmaxexp = std::log(DBL_MAX/2.);
    const double rmaxgoni = DBL_MAX;

    //define variables
    double xi = std::real(z);
    double yi = std::imag(z);
    double u,v; //return values
    bool a,b;
    double xabs = std::abs(xi);
    double yabs = std::abs(yi);
    double x=xabs/6.3;
    double y=yabs/4.4;
    double u2(0),v2(0);

    //protect qrho against overflow
    if (xabs > rmaxreal)
        throw std::overflow_error("overflow in w(z): xabs > rmaxreal");
    if (yabs > rmaxreal)
        throw std::overflow_error("overflow in w(z): yabs > rmaxreal");

    double qrho = x*x + y*y;

    double xabsq = xabs*xabs;
    double xquad = xabsq - yabs*yabs;
    double yquad = 2*xabs*yabs;

    a = (qrho < 0.085264);
    /*std::cout << "xabs " << xabs
              << " yabs " << yabs
              << " qrho " << qrho
              << " a " << a
              << std::endl;*/
    if (a)
    {
        //evaluate faddeeva-function using abramowitz 7.1.5, truncating at N
        qrho = (1-0.85*y)*std::sqrt(qrho);
        unsigned n = algebra::get_rounded(6+72*qrho);
        unsigned j = 2*n+1;
        double xsum = 1.0/double(j);
        double ysum=0;
        /*std::cout << "case A" << std::endl;
        std::cout
              << "qrho " << qrho
              << " n " << n
              << " j " << j
              << " xsum " << xsum
              << " ysum " << ysum
              << std::endl;*/

        for (unsigned i=n; i>= 1; i--) {
            j -= 2;
            double xaux = (xsum*xquad - ysum*yquad)/double(i);
            ysum = (xsum*yquad + ysum*xquad)/double(i);
            xsum = xaux + 1/double(j);
        }
        /*std::cout
              << " xsum " << xsum
              << " ysum " << ysum
              << std::endl;*/
        double u1 = -factor*(xsum*yabs+ysum*xabs) + 1.;
        double v1 =  factor*(xsum*xabs-ysum*yabs);
        double daux = std::exp(-xquad);
        u2 =  daux*std::cos(yquad);
        v2 = -daux*std::sin(yquad);
        /*std::cout
              << " u1 " << u1
              << " v1 " << v1
              << " daux " << daux
              << " u2 " << u2
              << " v2 " << v2
              << std::endl;*/

        u = u1*u2 - v1*v2;
        v = u1*v2 + v1*u2;

    } else {
        double h,h2(0);
        int kapn,nu;
        if (qrho > 1.) {
            //evaluate w(z) using laplace continued fraction, up to nu terms
            h = 0;
            kapn = 0;
            qrho = std::sqrt(qrho);
            nu = (int)algebra::get_rounded(3+1442./(26*qrho+77));
        } else {
            //truncated taylor expansion, using laplace continued fraction
            //to compute derivatives of w(z).
            //kapn : minimum number of terms in taylor expansion
            //nu : minimum number of terms in continued fraction
            qrho = (1-y)*std::sqrt(1-qrho);
            h = 1.88*qrho;
            h2 = 2*h;
            kapn = (int)algebra::get_rounded(7  + 34*qrho);
            nu   = (int)algebra::get_rounded(16 + 26*qrho);
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
        b = (h>0);
        if (b) qlambda = std::pow(h2,kapn);
        //std::cout << qlambda << std::endl;

        double rx(0), ry(0), sx(0), sy(0);

        for (int n=nu; n>=0; n--){
            double np1 = double(n)+1;
            double tx = yabs + h + np1*rx;
            double ty = xabs - np1*ry;
            double c = 0.5/(tx*tx+ty*ty);
            rx = c*tx;
            ry = c*ty;
            if ( (b) && (n <= kapn) ) {
                tx = qlambda + sx;
                sx = rx*tx - ry*sy;
                sy = ry*tx + rx*sy;
                qlambda = qlambda/h2;
            }
        }
            /*std::cout
                << "rx " << rx
                << " ry " << ry
                << " sx " << sx
                << " sy " << sy
                << std::endl;*/

        if (h==0) {
            u = factor*rx;
            v = factor*ry;
        } else {
            u = factor*sx;
            v = factor*sy;
        }

        if (yabs == 0) u = std::exp(-xabs*xabs);
    }

    //evaluation in the other quadrants
    if (yi < 0) {
        if (a) {
            u2 = 2*u2;
            v2 = 2*v2;
        } else {
            xquad = -xquad;

            //protect agains 2*exp(-z**2) overflow
            if (yquad > rmaxgoni)
                throw std::overflow_error("overflow in w(z): yquad > rmaxgoni");
            if (xquad > rmaxexp)
                throw std::overflow_error("overflow in w(z): xquad > rmaxexp");

            double w1 = 2*std::exp(xquad);
            u2 = w1*std::cos(yquad);
            v2 = -w1*std::sin(yquad);
        }

        u = u2 - u;
        v = v2 - v;

        if (xi>0) v = -v;
    } else {
        if (xi<0) v = -v;
    }

    return std::complex<double>(u,v);
}

//C(a,b,c) = exp(-(a^2+b^2)) * (
//              sqrt(pi)*Exp[-2ab]*Im[ Exp[-2i(a+b)c] * w(-c+i(a+b)) ]
//              + 1/c * cos(2(a+b)c)
//              )
inline double C(double a, double b, double c){
    const double sqrtpi = 1.7724538509055160273;
    //compute left part
    std::complex<double> i(0,1);
    std::complex<double> z(-c,a+b);
    std::complex<double> fad = w(z);
    fad *= std::exp(-2*(a+b)*c*i);
    double left = sqrtpi * std::imag(fad) * std::exp(-2*a*b);
    //compute right part
    double right = std::cos(2*(a+b)*c)/c;
    /*std::cout << " a= " << a
              << " b= " << b
              << " c= " << c
        << " left= " << left << " right= " << right << std::endl;*/
    //exponentiate and return
    double retval = std::exp(-a*a-b*b);
    return retval*(left+right);
}

//A(a,b,c) = 1/(8abc) * (C(a,b,c) - C(a,-b,c))
inline double A(double a, double b, double c){
 double c1 = C(a,b,c);
 double c2 = C(a,-b,c);
 std::cout << " a= " << a
              << " b= " << b
              << " c= " << c
              << " c1= " << c1
              << " c2= " << c2
              << " c1-c2= " << c1-c2
              << std::endl;
 return 1./(8*a*b*c) * ( c1 - c2);
}

//Y(a,b,c) = sinc(2*a*c)*sinc(2*b*c)*exp(-(a^2+b^2))
//takes a pointer to an instance of sinc function
inline double Y(double a, double b, double c ){
     return boost::math::sinc_pi(2*a*c)*boost::math::sinc_pi(2*b*c)
         *std::exp(-a*a-b*b);
}

//Z(a,b,c) = sqrt(pi)/(8*a*b*c) * exp(-c^2)
//              * Im[ erf[a-b+i*c] - erf[a+b+i*c] ]
inline double Z(double a, double b, double c, unsigned kmax=100){
    const double sqrtpi = 1.7724538509055160273;
    const std::complex<double> i(0,1);
    const std::complex<double> z(a,c);
    double factor = -sqrtpi/(8.*a*b*c);
    std::complex<double> expo(std::exp(-a*(a+2.*i*c)));
    ErfDiff dif(kmax);
    return factor * std::imag(expo*dif.get(z,b));
}

//efficient computation of Z-Y in the case where d12 == d13
class ZmYsim {
    public:
        //kmax1 : maximum k for small z approx
        //kmax2 : maximum k for large z approx
        //zcut : cut-off value for norm of z between the two methods
        ZmYsim(unsigned kmax1=10, unsigned kmax2=10, double zcut=10.0) :
            kmax1_(kmax1), kmax2_(kmax2), zcut_(zcut) {
            reset(std::complex<double>(0,0),0);
            zval_small_=0.;
            even_dks(1.);
        }

        double get(double a, double b, double c){
            if (square(a)+square(c) > square(zcut_)){
                return get_large_z(a,b,c);
            }else{
                return get_small_z(a,b,c);
            }
        }

    private:
        double get_large_z(double a, double b, double c){
            std::complex<double> z(a,c);
            std::complex<double> i(0,1);
            //sum alpha_k delta_k / sqrt(pi)
            //sum beta_k delta_k / sqrt(pi)
            std::complex<double> alpha(0,0), beta(0,0);
            for (unsigned k=0; k<=kmax2_; k++){
                std::complex<double> d(get_delta(k,z,b));
                alpha += get_alpha(k,z,b)*d;
                beta += get_beta(k,z,b)*d;
            }
            const double sqrtpi = 1.7724538509055160273;
            alpha = alpha/sqrtpi;
            beta = beta/sqrtpi;
            //
            // -i/c sinh(2ab)
            //  i/c cosh(2ab)
            std::complex<double> ralpha = -i/c*std::sinh(2*a*b);
            std::complex<double> rbeta = i/c*std::cosh(2*a*b);
            //
            //sum up terms
            std::complex<double> lterm = std::cosh(2*b*z)*(alpha+ralpha);
            std::complex<double> rterm = std::sinh(2*b*z)*(beta+rbeta);
            //add prefactors
            std::complex<double> retval =-std::exp(-a*a-b*b-2.*i*a*c)/(4*a*b*c);
            return std::imag(retval*(lterm+rterm));
        }

        std::complex<double>
        get_alpha(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (alpha_.size() <= k){
                //compute values < k
                if (k == 0){
                    alpha_.push_back(get_u(1,1,z,b));
                } else {
                    get_alpha(k-1,z,b);
                    //compute last value
                    std::complex<double> tmp=0;
                    std::vector<std::complex<double> > uk(get_all_u(2*k+1,z,b));
                    for (std::vector<std::complex<double> >::iterator
                            i=uk.begin()+1; i<uk.end(); i += 2) tmp += *i;
                    alpha_.push_back(tmp);
                }
            }
            return alpha_[k];
        }

        std::complex<double>
        get_beta(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (beta_.size() <= k){
                //compute values < k
                if (k == 0){
                    beta_.push_back(get_u(0,1,z,b));
                } else {
                    get_beta(k-1,z,b);
                    //compute last value
                    std::complex<double> tmp=0;
                    std::vector<std::complex<double> > uk(get_all_u(2*k+1,z,b));
                    for (std::vector<std::complex<double> >::const_iterator
                            i=uk.begin(); i<uk.end(); i += 2) tmp += *i;
                    beta_.push_back(tmp);
                }
            }
            return beta_[k];
        }

        std::complex<double>
        get_delta(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (delta_.size() <= k){
                //compute values < k
                if (k == 0){
                    const double sqrtpi=1.772453850905516027298;
                    delta_.push_back(factor_*sqrtpi);
                } else {
                    std::complex<double> previous = get_delta(k-1,z,b);
                    //compute last value
                    std::complex<double> val = previous*square(factor_)
                                                *(1.-2*double(k))/2.;
                    delta_.push_back(val);
                }
            }
            return delta_[k];
        }

        //go from i,k to i-1,k until 0,k then to 0,k-1 until 0,0
        std::complex<double> get_u(unsigned i, unsigned k,
                std::complex<double> z, double b){
            if (uvals_.size() >=k+1 && uvals_[k].size() >= i+1)
                return uvals_[k][i];
            if (i >0){
                //get (i-1),k
                std::complex<double> val = get_u(i-1,k,z,b);
                double di = double(i);
                double dk = double(k);
                std::complex<double> tmp = b*(1-di+dk)/(di*z);
                val *= tmp;
                uvals_[k].push_back(val);
                return val;
            } else {
                if (k>0) get_u(0,k-1,z,b);
                //u(0,k) == 1
                std::vector<std::complex<double> > tmp;
                tmp.push_back(1.);
                uvals_.push_back(tmp);
                return 1.;
            }
        }

        std::vector<std::complex<double> > get_all_u(unsigned k,
                std::complex<double> z, double b){
            if (uvals_.size() <k+1 || uvals_[k].size() < k+1)
                get_u(k,k,z,b);
            return uvals_[k];
        }

        void reset(std::complex<double> z, double b){
            alpha_.clear();
            beta_.clear();
            delta_.clear();
            uvals_.clear();
            zval_ = z;
            bval_ = b;
            factor_ = z/(z*z-b*b);
        }

        bool good_vals(std::complex<double> z, double b){
            return (std::abs(z-zval_) < 1e-6 && std::abs(b-bval_) < 1e-6);
        }

        double get_small_z(double a, double b, double c){
            std::complex<double> complexI(0,1);
            std::complex<double> z(a,c);
            std::complex<double> zsq = b/(2.*z);
            zsq *= zsq;
            //get even dk values
            std::vector<std::complex<double> > dkvals(even_dks(a+complexI*c));
            //compute \sum_{k=0}^{kmax} d_{2k}(z) (b/2z)^{2k}
            std::complex<double> tmp = dkvals.back();
            for (std::vector<std::complex<double> >::reverse_iterator
                    i = dkvals.rbegin()+1; i < dkvals.rend(); ++i){
                tmp = *i + tmp*zsq;
                //std::cout << z << " " << b << " " << *i << std::endl;
            }
            //multiply by missing terms
            double sine = std::sinh(2*a*b);
            std::complex<double> sinesum = tmp*sine*zsq;
            //
            //get odd epsilon_k values
            std::vector<std::complex<double> > epskvals(odd_epsks(a,b,c));
            //compute \sum_{k=0}^{kmax} \epsilon_{2k+1}(z) (b/2z)^{2k}
            tmp = epskvals.back();
            //std::cout << tmp << std::endl;
            for (std::vector<std::complex<double> >::reverse_iterator
                    i = epskvals.rbegin()+1; i < epskvals.rend(); ++i){
                tmp = *i + tmp*zsq;
                //std::cout << tmp << std::endl;
            }
            //multiply by missing terms
            std::complex<double> cosinesum = tmp*b/(2.*z);
            //std::cout << sine << " " << sinesum << " " <<cosinesum <<
            //std::endl;
            std::complex<double> total = sine + sinesum + cosinesum;
            //
            //compute prefactors
            std::complex<double> prefac =
                std::exp(-a*a-2.*complexI*a*c)/(4.*a*b*c*c);
            return std::real(prefac*total);
        }

        //d_k(z) = sum_{j=1}^k t_jk z^{2j}
        std::complex<double> dk(unsigned k, std::complex<double> z){
            std::vector<double> tjks = tjk_.get_all(k);
            std::complex<double> zsq = z*z;
            std::complex<double> tmp = tjks.back();
            //std::cout << "  tjk("<<j--<<","<<k<<")= " << tmp << std::endl;
            for (std::vector<double>::reverse_iterator i = tjks.rbegin()+1;
                    i < tjks.rend(); ++i){
                //std::cout << "  tjk("<<j--<<","<<k<<")= " << *i << std::endl;
                tmp = *i + tmp*zsq;
            }
            return tmp*zsq;
        }

        //epsilon_k(z) = sum_{j=1}^k t_jk z^{2j} ( ch(2ab) - ic(2j-k)/(zk) )
        std::complex<double> epsk(unsigned k, double a, double b, double c){
            //std::cout << " epsilon("<<k<<","<<a <<" "<<b<<"
            //"<<c<<")"<<std::endl;
            std::vector<double> tjks = tjk_.get_all(k);
            std::complex<double> z(a,c);
            std::complex<double> zsq = z*z;
            double dk = (double) k;
            std::complex<double> var = std::complex<double>(0,c)/z;
            std::complex<double> con( std::cosh(2*a*b), 0);
            con = var + con;
            var = -2.*var/dk;
            std::complex<double> tmp = tjks.back()*(con + var*dk);
            for (unsigned j=k-1; j >= 1; j--){
                tmp = tjks[j-1]*(var*double(j) + con) + tmp*zsq;
            }
            tmp *= zsq;
            //std::cout << " epsilon("<<k<<","<<z<<")= "<<tmp<<std::endl;
            return tmp;
        }

        //recompute all even dks given z (if needed) and return them
        std::vector<std::complex<double> > even_dks(std::complex<double> z){
            if (std::abs(zval_small_-z)/std::abs(z) > 1e-6){
                //std::cout << "even_dks: recalc " << zval_small_ << " " << z <<
                //std::endl;
                zval_small_ = z;
                //recalculate values
                dks_.clear();
                dks_.reserve(kmax1_+1);
                //std::cout << "memoize for z="<<z<<std::endl;
                for (unsigned i=2; i<= 2*kmax1_; i+= 2){
                    dks_.push_back(dk(i,z));
                    //std::cout << " dk("<<i<<")= " << dks_.back() << std::endl;
                }
            }
            return dks_;
        }

        std::vector<std::complex<double> >
            odd_epsks(double a, double b, double c){
            std::vector<std::complex<double> > epsks;
            epsks.reserve(kmax1_+1);
            for (unsigned i=1; i<=2*kmax1_+1; i+=2){
                epsks.push_back(epsk(i,a,b,c));
            }
            return epsks;
        }

        unsigned kmax1_, kmax2_;
        double zcut_, bval_;
        std::complex<double> zval_, zval_small_, factor_;
        std::vector<std::complex<double> > dks_, alpha_, beta_, delta_;
        std::vector<std::vector<std::complex<double> > > uvals_;
        TjkHelper tjk_;
};

//efficient computation of Z-Y in the nondegenerate case ( -1 < rho < 1 )
// gaussian approximation with asymtotic series expansion for erf to 1st order
// ZmY(a,b,c,d,rho) = exp(-a^2-b^2)( sinc(c)*sinc(d)*(cosh(2*rho*a*b)-1)
//                                  +cosc(c)*cosc(d)*sinh(2*rho*a*b) )
inline double cov_approx(double a, double b, double c, double d, double rho){
     double sc = boost::math::sinc_pi(c);
     double sd = boost::math::sinc_pi(d);
     double cc = std::cos(c)/c;
     double cd = std::cos(d)/d;
     return std::exp(-a*a-b*b)*(sc*sd*(std::cosh(2*rho*a*b)-1)
                               +cc*cd*std::sinh(2*rho*a*b));
}

//efficient computation of Z-Y in the general case
class ZmY {
    public:
        //kmax : maximum k asymptotic expansion
        ZmY(unsigned kmax=10) :
            kmax_(kmax) {
            reset(std::complex<double>(0,0),0);
        }

        double get(double a, double b, double c1, double c2, double rho){
            if (a*rho >= b){
                return get_arho(a,b,c1,c2,rho);
            } else {
                return get_b(a,b,c1,c2,rho);
            }
        }

    private:

        //Z(a,b,c1,c2,rho)-Y(a,b,c1,c2) when a*rho >= b
        double get_arho(double a, double b, double c1, double c2, double rho){
            std::complex<double> z(a*rho,c2);
            std::complex<double> i(0,1);
            //sum alpha_k delta_k / sqrt(pi)
            //sum beta_k delta_k / sqrt(pi)
            std::complex<double> alpha(0,0), beta(0,0);
            for (unsigned k=0; k<=kmax_; k++){
                std::complex<double> d(get_delta(k,z,b));
                alpha += get_alpha(k,z,b)*d;
                beta += get_beta(k,z,b)*d;
            }
            const double sqrtpi = 1.7724538509055160273;
            alpha = alpha/sqrtpi;
            beta = beta/sqrtpi;
            //
            // -i/c2 sinh(2 a b rho)
            //  i/c2 cosh(2 a b rho)
            std::complex<double> ralpha = -i/c2*std::sinh(2*a*b*rho);
            std::complex<double> rbeta = i/c2*std::cosh(2*a*b*rho);
            //
            //sum up terms
            std::complex<double> lterm = std::cosh(2*b*z)*(alpha+ralpha);
            std::complex<double> rterm = std::sinh(2*b*z)*(beta+rbeta);
            //add prefactors
            double retval =std::imag(std::exp(-2.*i*a*c1)*(lterm+rterm));
            retval = -std::exp(-a*a-b*b)*retval/(4*a*b*c1);
            return retval;
        }

        //Z(a,b,c1,c2,rho)-Y(a,b,c1,c2) when b > a*rho
        double get_b(double a, double b, double c1, double c2, double rho){
            std::complex<double> z(b,c2);
            std::complex<double> i(0,1);
            double aro=a*rho;
            //sum alpha_k delta_k / sqrt(pi)
            //sum beta_k delta_k / sqrt(pi)
            std::complex<double> alpha(0,0), beta(0,0);
            for (unsigned k=0; k<=kmax_; k++){
                std::complex<double> d(get_delta(k,z,aro));
                alpha += get_alpha(k,z,aro)*d;
                beta += get_beta(k,z,aro)*d;
            }
            const double sqrtpi = 1.7724538509055160273;
            alpha = alpha/sqrtpi;
            beta = beta/sqrtpi;
            //
            // -i/2c2 (e^(2abrho) - e^(-2abrho + 4ic2(b-arho))
            //  i/2c2 (e^(2abrho) + e^(-2abrho + 4ic2(b-arho))
            double tmp = std::exp(2*a*b*rho);
            std::complex<double> tmp2 = std::exp(4.*i*c2*(b-aro))/tmp;
            std::complex<double> ralpha = -i/(2*c2)*(tmp - tmp2);
            std::complex<double> rbeta = i/(2*c2)*(tmp + tmp2);
            //
            //sum up terms
            std::complex<double> lterm = std::cosh(2*aro*z)*(alpha+ralpha);
            std::complex<double> rterm = std::sinh(2*aro*z)*(beta+rbeta);
            //add prefactors
            double retval =std::imag(std::exp(-2.*i*(b*c2+a*(c1-rho*c2)))

                                    *(lterm+rterm));
            retval = -std::exp(-a*a-b*b)*retval/(4*a*b*c1);
            return retval;
        }

        std::complex<double>
        get_alpha(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (alpha_.size() <= k){
                //compute values < k
                if (k == 0){
                    alpha_.push_back(get_u(1,1,z,b));
                } else {
                    get_alpha(k-1,z,b);
                    //compute last value
                    std::complex<double> tmp=0;
                    std::vector<std::complex<double> > uk(get_all_u(2*k+1,z,b));
                    for (std::vector<std::complex<double> >::iterator
                            i=uk.begin()+1; i<uk.end(); i += 2) tmp += *i;
                    alpha_.push_back(tmp);
                }
            }
            return alpha_[k];
        }

        std::complex<double>
        get_beta(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (beta_.size() <= k){
                //compute values < k
                if (k == 0){
                    beta_.push_back(get_u(0,1,z,b));
                } else {
                    get_beta(k-1,z,b);
                    //compute last value
                    std::complex<double> tmp=0;
                    std::vector<std::complex<double> > uk(get_all_u(2*k+1,z,b));
                    for (std::vector<std::complex<double> >::const_iterator
                            i=uk.begin(); i<uk.end(); i += 2) tmp += *i;
                    beta_.push_back(tmp);
                }
            }
            return beta_[k];
        }

        std::complex<double>
        get_delta(unsigned k, std::complex<double> z, double b){
            //reset if z or b have changed
            if (!good_vals(z,b)) reset(z,b);
            //if value was not memoized, recompute it
            if (delta_.size() <= k){
                //compute values < k
                if (k == 0){
                    const double sqrtpi=1.772453850905516027298;
                    delta_.push_back(factor_*sqrtpi);
                } else {
                    std::complex<double> previous = get_delta(k-1,z,b);
                    //compute last value
                    std::complex<double> val = previous*square(factor_)
                                                *(1.-2*double(k))/2.;
                    delta_.push_back(val);
                }
            }
            return delta_[k];
        }

        //go from i,k to i-1,k until 0,k then to 0,k-1 until 0,0
        std::complex<double> get_u(unsigned i, unsigned k,
                std::complex<double> z, double b){
            if (uvals_.size() >=k+1 && uvals_[k].size() >= i+1)
                return uvals_[k][i];
            if (i >0){
                //get (i-1),k
                std::complex<double> val = get_u(i-1,k,z,b);
                double di = double(i);
                double dk = double(k);
                std::complex<double> tmp = b*(1-di+dk)/(di*z);
                val *= tmp;
                uvals_[k].push_back(val);
                return val;
            } else {
                if (k>0) get_u(0,k-1,z,b);
                //u(0,k) == 1
                std::vector<std::complex<double> > tmp;
                tmp.push_back(1.);
                uvals_.push_back(tmp);
                return 1.;
            }
        }

        std::vector<std::complex<double> > get_all_u(unsigned k,
                std::complex<double> z, double b){
            if (uvals_.size() <k+1 || uvals_[k].size() < k+1)
                get_u(k,k,z,b);
            return uvals_[k];
        }

        void reset(std::complex<double> z, double b){
            alpha_.clear();
            beta_.clear();
            delta_.clear();
            uvals_.clear();
            zval_ = z;
            bval_ = b;
            factor_ = z/(z*z-b*b);
        }

        bool good_vals(std::complex<double> z, double b){
            return (std::abs(z-zval_) < 1e-6 && std::abs(b-bval_) < 1e-6);
        }

        unsigned kmax_;
        double zcut_, bval_;
        std::complex<double> zval_, factor_;
        std::vector<std::complex<double> > alpha_, beta_, delta_;
        std::vector<std::vector<std::complex<double> > > uvals_;
};

//efficient computation of B(a,b,c1,c2,rho)
//=1/2(Z(a,b,c1,c2,rho)+Z(b,a,c1,c2,rho))-Y(a,b,c1,c2,rho)
class B {

    public:
        //kmax for asymptotic expansion
        B(unsigned kmax=10) {
            zmy_ = new ZmY(kmax);
            zmy2_ = new ZmY(kmax);
            zmysim_ = new ZmYsim(kmax,kmax,0);
        }

        ~B() {
            delete zmy_;
            delete zmy2_;
            delete zmysim_;
        }

        double get(double a, double b, double c){
            return zmysim_->get(a,b,c);
        }

        double get(double a, double b, double c1, double c2, double rho){
            if (rho==1 && c1==c2) return zmysim_->get(a,b,c1);
            double left=zmy_->get(a,b,c1,c2,rho);
            double right=zmy2_->get(b,a,c2,c1,rho);
            return 0.5*(left+right);
        }

    private:
        ZmY *zmy_, *zmy2_;
        ZmYsim *zmysim_;

};

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_VARIANCE_HELPERS_H */
