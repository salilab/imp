/**
 *  \file functions.h    \brief Classes for general functions
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FUNCTIONS_H
#define IMPISD_FUNCTIONS_H

#include "isd_config.h"
#include <IMP/Particle.h>
#include <IMP/isd/Scale.h>
#include <IMP/Object.h>

#define MINIMUM 1e-7

IMPISD_BEGIN_NAMESPACE

//! Base class for functions of one variable
class IMPISDEXPORT UnivariateFunction : public Object
{
 public:

     UnivariateFunction() : Object("") {}

     //! evaluate the function at a certain point
     virtual std::vector<double> operator() (std::vector<double> x) const = 0; 

     //! update internal parameters and return true if they have changed.
     virtual bool update_if_changed() = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x() const = 0;

     //! returns the number of output dimensions
     virtual unsigned get_ndims_y() const = 0;

     IMP_REF_COUNTED_DESTRUCTOR(UnivariateFunction);
};

//! Base class for functions of two variables
class IMPISDEXPORT BivariateFunction : public Object
{
 public:

     BivariateFunction() : Object("") {}

     //! evaluate the function at a certain point
     virtual std::vector<double> operator()
                (std::vector<double> x1, std::vector<double> x2) const = 0;

     //! update internal parameters and return true if they have changed.
     virtual bool update_if_changed() = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x1() const = 0;
     virtual unsigned get_ndims_x2() const = 0;

     //! returns the number of output dimensions
     virtual unsigned get_ndims_y() const = 0;

     IMP_REF_COUNTED_DESTRUCTOR(BivariateFunction);
};

//! Linear one-dimensional function
/* f(x) = a*x + b, where a,b are ISD scales, and f(x) and x are doubles.
 */
class IMPISDEXPORT Linear1DFunction : public UnivariateFunction
{
    public:
        Linear1DFunction(Particle * a, Particle * b) 
            : a_(a), b_(b) 
        {
            a_val_ = Scale(a).get_scale();
            b_val_ = Scale(b).get_scale();
        }

        bool update_if_changed() {
            double tmpa = Scale(a_).get_scale();
            double tmpb = Scale(b_).get_scale();
            if (std::abs(tmpa != a_val_) < MINIMUM
                    || std::abs(tmpb != b_val_) < MINIMUM)
            {
                a_val_ = tmpa;
                b_val_ = tmpb;
                return true;
            } else {
                return false;
            }
        }

        std::vector<double> operator()(std::vector<double> x) const {
            IMP_USAGE_CHECK(x.size() == 1, "expecting a 1-D vector");
            std::vector<double> ret(1,a_val_*x[0]+b_val_);
            return ret;
        }

        unsigned get_ndims_x() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

    IMP_OBJECT_INLINE(Linear1DFunction, out << "y = " << a_val_ 
            << " * x + " << b_val_ << std::endl, {});

    private:
        Pointer<Particle> a_,b_;
        double a_val_, b_val_;
    
};

//! Covariance function
/* \f[w(x,x') = \tau^2 \exp\left(-\frac{(x-x')^\alpha}{2\lambda^\alpha}\f]
 * \f$\tau\f$ and \f$\lambda\f$ are ISD scale,s \f$\alpha\f$ is set up front and
 * should be positive, usually greater than 1. Default is 2.
 */
class IMPISDEXPORT Covariance1DFunction : public BivariateFunction
{
    public:
        Covariance1DFunction(double alpha, Particle* tau, Particle* lambda) :
            alpha_(alpha), tau_(tau), lambda_(lambda) 
    {
        lambda_val_= Scale(lambda).get_scale();
        tau_val_= Scale(tau).get_scale();
    }

        bool update_if_changed() {
            double tmpt = Scale(tau_).get_scale();
            double tmpl = Scale(lambda_).get_scale();
            if (std::abs(tmpt != tau_val_) < MINIMUM
                    || std::abs(tmpl != lambda_val_) < MINIMUM)
            {
                lambda_val_ = tmpl;
                tau_val_ = tmpt;
                return true;
            } else {
                return false;
            }
        }

        std::vector<double> operator()(std::vector<double> x1,
                std::vector<double> x2) const {
            IMP_USAGE_CHECK(x1.size() == 1, "expecting a 1-D vector");
            IMP_USAGE_CHECK(x2.size() == 1, "expecting a 1-D vector");
            std::vector<double> ret(1,
                    IMP::square(tau_val_)
                    *std::exp(
                        -0.5*std::pow(
                                std::abs( (x1[0]-x2[0])/lambda_val_ )
                                , alpha_)
                        )
                    );
            return ret;
        }

        unsigned get_ndims_x1() const {return 1;}
        unsigned get_ndims_x2() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

    IMP_OBJECT_INLINE(Covariance1DFunction, out << "covariance function with alpha = " << alpha_ << std::endl, {});


    private:
        double alpha_;
        Pointer<Particle> tau_,lambda_;
        double tau_val_,lambda_val_;
    
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FUNCTIONS_H */
