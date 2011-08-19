/**
 *  \file functions.h    \brief Classes for general functions
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FUNCTIONS_H
#define IMPISD_FUNCTIONS_H

#include "isd_config.h"
#include <IMP/Particle.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/isd/Scale.h>
#include <IMP/Object.h>

#define MINIMUM 1e-7

IMPISD_BEGIN_NAMESPACE

//! Base class for functions of one variable
class IMPISDEXPORT UnivariateFunction : public Object
{
 public:

     UnivariateFunction(std::string str) : Object(str) {}

     //! evaluate the function at a certain point
     virtual std::vector<double> operator() (std::vector<double> x) const = 0; 

     //! return true if internal parameters have changed.
     virtual bool has_changed() const = 0; 

     //! update internal parameters
     virtual void update() = 0;

     //! update derivatives of particles
     virtual void add_to_derivatives(std::vector<double> x,
             DerivativeAccumulator &accum) const = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x() const = 0;

     //! returns the number of output dimensions
     virtual unsigned get_ndims_y() const = 0;

     //! particle manipulation
     virtual ParticlesTemp get_input_particles() const = 0;
     virtual ContainersTemp get_input_containers() const = 0;

     IMP_REF_COUNTED_DESTRUCTOR(UnivariateFunction);
};

//! Base class for functions of two variables
class IMPISDEXPORT BivariateFunction : public Object
{
 public:

     BivariateFunction(std::string str) : Object(str) {}

     //! evaluate the function at a certain point
     virtual std::vector<double> operator()
                (std::vector<double> x1, std::vector<double> x2) const = 0;

     //! return true if internal parameters have changed.
     virtual bool has_changed() const = 0;

     //! update internal parameters
     virtual void update() = 0;

     //! update derivatives of particles
     virtual void add_to_derivatives(std::vector<double> x1,
             std::vector<double> x2, DerivativeAccumulator &accum) const = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x1() const = 0;
     virtual unsigned get_ndims_x2() const = 0;

     //! returns the number of output dimensions
     virtual unsigned get_ndims_y() const = 0;

     //! particle manipulation
     virtual ParticlesTemp get_input_particles() const = 0;
     virtual ContainersTemp get_input_containers() const = 0;

     IMP_REF_COUNTED_DESTRUCTOR(BivariateFunction);
};

//! Linear one-dimensional function
/* f(x) = a*x + b, where a,b are ISD nuisances, and f(x) and x are doubles.
 */
class IMPISDEXPORT Linear1DFunction : public UnivariateFunction
{
    public:
        Linear1DFunction(Particle * a, Particle * b) 
            : UnivariateFunction("Linear1DFunction %1%"), a_(a), b_(b) 
        {
            IMP_LOG(TERSE, "Linear1DFunction: constructor" << std::endl);
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(a); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(b); }
            a_val_ = Nuisance(a).get_nuisance();
            b_val_ = Nuisance(b).get_nuisance();
        }

        bool has_changed() const {
            double tmpa = Nuisance(a_).get_nuisance();
            double tmpb = Nuisance(b_).get_nuisance();
            if ((std::abs(tmpa - a_val_) > MINIMUM)
                    || (std::abs(tmpb - b_val_) > MINIMUM))
            {
                IMP_LOG(TERSE, "Linear1DFunction: has_changed():");
                IMP_LOG(TERSE, "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            a_val_ = Nuisance(a_).get_nuisance();
            b_val_ = Nuisance(b_).get_nuisance();
            IMP_LOG(TERSE, "Linear1DFunction: update()  a:= "
                    << a_val_ << " b:=" << b_val_ << std::endl);
        }

        std::vector<double> operator()(std::vector<double> x) const {
            IMP_USAGE_CHECK(x.size() == 1, "expecting a 1-D vector");
            std::vector<double> ret(1,a_val_*x[0]+b_val_);
            return ret;
        }

        void add_to_derivatives(std::vector<double> x,
                DerivativeAccumulator &accum) const
        {
            //d[f(x)]/da = x
            Nuisance(a_).add_to_nuisance_derivative(x[0], accum);
            //d[f(x)]/db = 1
            Nuisance(b_).add_to_nuisance_derivative(1, accum);
        }


        unsigned get_ndims_x() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        ParticlesTemp get_input_particles() const
        {
            ParticlesTemp ret;
            ret.push_back(a_);
            ret.push_back(b_);
            return ret;
        }

        ContainersTemp get_input_containers() const
        {
            ContainersTemp ret;
            return ret;
        }

        IMP_OBJECT_INLINE(Linear1DFunction, out << "y = " << a_val_ 
                << " * x + " << b_val_ << std::endl, {});

    private:
        Pointer<Particle> a_,b_;
        double a_val_, b_val_;
    
};

//! Covariance function
/* \f[w(x,x') = \tau^2 \exp\left(-\frac{|x-x'|^\alpha}{2\lambda^\alpha}\f]
 * \f$\tau\f$ and \f$\lambda\f$ are ISD nuisance,s \f$\alpha\f$ is set up front and
 * should be positive, usually greater than 1. Default is 2.
 * Jitter is a parameter that helps to condition the covariance matrices (avoids
 * NANs). Try a jitter of 0.1 if you get NANs. Minimum value: 1e-3.
 */
class IMPISDEXPORT Covariance1DFunction : public BivariateFunction
{
    public:
        Covariance1DFunction(double alpha, Particle* tau, Particle* lambda, double jitter = 0.0) :
            BivariateFunction("Covariance1DFunction %1%"), alpha_(alpha),
            tau_(tau), lambda_(lambda), J_(jitter)
    {
        IMP_LOG(TERSE, "Covariance1DFunction: constructor" << std::endl);
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(tau); }
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(lambda);}
        lambda_val_= Scale(lambda).get_nuisance();
        tau_val_= Scale(tau).get_nuisance();
        do_jitter = (jitter > 1e-3);

    }

        bool has_changed() const {
            double tmpt = Scale(tau_).get_nuisance();
            double tmpl = Scale(lambda_).get_nuisance();
            if ((std::abs(tmpt - tau_val_) > MINIMUM)
                    || (std::abs(tmpl - lambda_val_) > MINIMUM))
            {
                IMP_LOG(TERSE, "Covariance1DFunction: has_changed():");
                IMP_LOG(TERSE, "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            lambda_val_= Scale(lambda_).get_nuisance();
            tau_val_= Scale(tau_).get_nuisance();
            IMP_LOG(TERSE, "Covariance1DFunction: update()  tau:= "
                    << tau_val_ << " lambda:=" << lambda_val_ << std::endl);
        }

        void add_to_derivatives(std::vector<double> x1, std::vector<double> x2,
                DerivativeAccumulator &accum) const
        {
            //d[w(x1,x2)]/dtau = 2/tau*w(x1,x2)
            std::vector<double> val = (*this)(x1,x2);
            Scale(tau_).add_to_nuisance_derivative(
                    2./tau_val_ * val[0], accum);
            //d[w(x,x')]/dlambda 
            //= w(x,x') ( alpha |x'-x|^alpha/(2 lambda^{alpha+1}))
            
            Scale(lambda_).add_to_nuisance_derivative(
                    val[0] * (alpha_ * 
                        std::pow((std::abs(x1[0]-x2[0])/lambda_val_),alpha_)
                        /(2.*lambda_val_)), accum);
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
            if (x1[0]==x2[0] && do_jitter) ret[0] += J_; 
            return ret;
        }

        unsigned get_ndims_x1() const {return 1;}
        unsigned get_ndims_x2() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        ParticlesTemp get_input_particles() const
        {
            ParticlesTemp ret;
            ret.push_back(tau_);
            ret.push_back(lambda_);
            return ret;
        }

        ContainersTemp get_input_containers() const
        {
            ContainersTemp ret;
            return ret;
        }


        IMP_OBJECT_INLINE(Covariance1DFunction, out << "covariance function with alpha = " << alpha_ << std::endl, {});


    private:
        double alpha_;
        Pointer<Particle> tau_,lambda_;
        double tau_val_,lambda_val_,J_;
        bool do_jitter;
    
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FUNCTIONS_H */
