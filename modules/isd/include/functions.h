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
#include <IMP/isd/Switching.h>
#include <IMP/Object.h>

#define MINIMUM 1e-7

IMPISD_BEGIN_NAMESPACE

//! Base class for functions of one variable
class IMPISDEXPORT UnivariateFunction : public base::Object
{
 public:

 UnivariateFunction(std::string str) : base::Object(str) {}

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
class IMPISDEXPORT BivariateFunction : public base::Object
{
 public:

 BivariateFunction(std::string str) : base::Object(str) {}

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
/* \f[w(x,x') = \tau^2 \exp\left(-\frac{|x-x'|^\alpha}{2\lambda^\alpha} +
 * \delta_{ij} (\sigma^2 + J)\f]
 * \f$\sigma\f$, \f$\tau\f$ and \f$\lambda\f$ are ISD nuisance,s \f$\alpha\f$ is set up front and
 * should be positive, usually greater than 1. Default is 2.
 * J is some jitter, needed when sigma gets really small. try J=0.01 if you get
 * NANs.
 */
class IMPISDEXPORT Covariance1DFunction : public BivariateFunction
{
    public:
        Covariance1DFunction(Particle* tau, Particle* lambda, Particle* sigma,
                double alpha=2.0, double jitter =0.0) :
            BivariateFunction("Covariance1DFunction %1%"), alpha_(alpha),
            tau_(tau), lambda_(lambda), sigma_(sigma), J_(jitter)
    {
        IMP_LOG(TERSE, "Covariance1DFunction: constructor" << std::endl);
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(tau); }
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(lambda);}
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(sigma);}
        lambda_val_= Scale(lambda).get_nuisance();
        tau_val_= Scale(tau).get_nuisance();
        sigma_val_= Scale(sigma).get_nuisance();
        do_jitter = (jitter>MINIMUM);

    }

        bool has_changed() const {
            double tmpt = Scale(tau_).get_nuisance();
            double tmpl = Scale(lambda_).get_nuisance();
            double tmps = Scale(sigma_).get_nuisance();
            if ((std::abs(tmpt - tau_val_) > MINIMUM)
                    || (std::abs(tmpl - lambda_val_) > MINIMUM)
                    || (std::abs(tmps - sigma_val_) > MINIMUM))
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
            sigma_val_= Scale(sigma_).get_nuisance();
            IMP_LOG(TERSE, "Covariance1DFunction: update()  tau:= "
                    << tau_val_ << " lambda:=" << lambda_val_ 
                    <<" sigma:=" << sigma_val_ << std::endl);
        }

        void add_to_derivatives(std::vector<double> x1, std::vector<double> x2,
                DerivativeAccumulator &accum) const
        {
            //d[w(x1,x2)]/dtau = 2/tau*(w(x1,x2)-delta_ij sigma^2)
            double val;
            if (std::abs(x1[0] - x2[0])<MINIMUM) {
                val = (*this)(x1,x2)[0]-IMP::square(sigma_val_);
            } else { 
                val = (*this)(x1,x2)[0]; 
            }
            Scale(tau_).add_to_nuisance_derivative(
                    2./tau_val_ * val, accum);
            //d[w(x,x')]/dlambda 
            //= (w(x,x')-delta_ij sigma^2) ( alpha |x'-x|^alpha/(2 lambda^{alpha+1}))
            Scale(lambda_).add_to_nuisance_derivative(
                    val * (alpha_ * 
                        std::pow((std::abs(x1[0]-x2[0])/lambda_val_),alpha_)
                        /(2.*lambda_val_)), accum);
            //d[w(x,x')]/dsigma = 2 delta_ij sigma
            if (std::abs(x1[0] - x2[0])<MINIMUM)
                Scale(sigma_).add_to_nuisance_derivative(2*sigma_val_, accum);

        }


        std::vector<double> operator()(std::vector<double> x1,
                std::vector<double> x2) const {
            IMP_USAGE_CHECK(x1.size() == 1, "expecting a 1-D vector");
            IMP_USAGE_CHECK(x2.size() == 1, "expecting a 1-D vector");
            //std::cout<<"eval ";
            //std::cout<<"tau = " << tau_val_ << " sig = " << sigma_val_
            //         <<"lambda = " << lambda_val_ << " alpha = " << alpha_ << std::endl;
            std::vector<double> ret(1,
                    IMP::square(tau_val_)
                    *std::exp(
                        -0.5*std::pow(
                                std::abs( (x1[0]-x2[0])/lambda_val_ )
                                , alpha_)
                        )
                    );
            //std::cout<<" exp " << ret[0];
            //std::cout<<" x1= " << x1[0] << " x2= " << x2[0];
            //std::cout<<" abs(x1-x2)= " << std::abs(x1[0]-x2[0]);
            if (std::abs(x1[0]-x2[0])<MINIMUM)
            {
                //std::cout<<" x1==x2 ";
                ret[0] += IMP::square(sigma_val_);
                if (do_jitter) ret[0] += J_;
            }
            //std::cout <<" retval " << ret[0] << std::endl;
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
            ret.push_back(sigma_);
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
        Pointer<Particle> tau_,lambda_,sigma_;
        double tau_val_,lambda_val_,sigma_val_,J_;
        bool do_jitter;
    
};

//! Reparametrized covariance function
/* This is the previous covariance function, with
 * \f[\theta = \frac{\tau^2}{\tilde{\sigma}^2+\tau^2}\f] and
 * \f[\sigma^2 = \frac{\tilde{\sigma}^2+\tau^2}\f] ( with \f$\tilde{\sigma}\f$
 * being the parameter of the original covariance function). With this
 * parametrization, we have
 * \f[w(x,x) = \sigma^2 + J\f]
 * \f[w(x,x') = \sigma^2\theta
   \exp\left(-\frac{|x-x'|^\alpha}{2\lambda^\alpha}\right)\f]
 * \f$\sigma\f$ and \f$\lambda\f$ are Scales, \f$\theta\f$ is a Switching,
 * \f$\alpha\f$ is set up front and should be positive, usually greater than 1.
 * (default is 2).  J is some jitter, needed when sigma gets really small. try
 * J=0.01 if you get NANs.
 */
class IMPISDEXPORT ReparametrizedCovariance1DFunction : public BivariateFunction
{
    public:
        ReparametrizedCovariance1DFunction(Particle* sigma, Particle* lambda,
                Particle* theta, double alpha=2.0, double jitter =0.0) :
            BivariateFunction("ReparametrizedCovariance1DFunction %1%"),
            alpha_(alpha), sigma_(sigma), lambda_(lambda), theta_(theta),
            J_(jitter) 
    { 
        IMP_LOG(TERSE, "ReparametrizedCovariance1DFunction: constructor" 
                << std::endl); 
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Switching::decorate_particle(theta); }
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(lambda);}
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(sigma);} 
        lambda_val_= Scale(lambda).get_scale(); 
        theta_val_= Switching(theta).get_switching(); 
        sigma_val_= Scale(sigma).get_scale(); 
        do_jitter = (jitter>MINIMUM);

    }

        bool has_changed() const {
            double tmpt = Switching(theta_).get_switching();
            double tmpl = Scale(lambda_).get_scale();
            double tmps = Scale(sigma_).get_scale();
            if ((std::abs(tmpt - theta_val_) > MINIMUM)
                    || (std::abs(tmpl - lambda_val_) > MINIMUM)
                    || (std::abs(tmps - sigma_val_) > MINIMUM))
            {
                IMP_LOG(TERSE, "ReparametrizedCovariance1DFunction: has_changed():");
                IMP_LOG(TERSE, "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            lambda_val_= Scale(lambda_).get_scale();
            theta_val_= Switching(theta_).get_switching();
            sigma_val_= Scale(sigma_).get_scale();
            IMP_LOG(TERSE, 
                    "ReparametrizedCovariance1DFunction: update()  tau:= " 
                    << theta_val_ << " lambda:=" << lambda_val_ 
                    <<" sigma:=" << sigma_val_ << std::endl);
        }

        void add_to_derivatives(std::vector<double> x1, std::vector<double> x2,
                DerivativeAccumulator &accum) const
        {
            if (std::abs(x1[0] - x2[0])<MINIMUM) {
                Scale(sigma_).add_to_scale_derivative(2*sigma_val_, accum);
            } else { 
                double exponent = std::pow( std::abs( 
                            (x1[0]-x2[0])/lambda_val_
                            ) , alpha_);
                double expterm = std::exp( -0.5*exponent);
                Scale(sigma_).add_to_scale_derivative(
                        2*theta_val_*sigma_val_*expterm, accum);
                Scale(lambda_).add_to_scale_derivative(
                        IMP::square(sigma_val_)*theta_val_
                        *exponent*alpha_/(2*lambda_val_)*expterm, accum);
                Switching(theta_).add_to_switching_derivative(
                        IMP::square(sigma_val_)*expterm, accum);
            }
        }


        std::vector<double> operator()(std::vector<double> x1,
                std::vector<double> x2) const {
            IMP_USAGE_CHECK(x1.size() == 1, "expecting a 1-D vector");
            IMP_USAGE_CHECK(x2.size() == 1, "expecting a 1-D vector");
            double ret=IMP::square(sigma_val_);
            if (std::abs(x1[0]-x2[0])>MINIMUM)
            {
                    ret *=theta_val_ * std::exp(
                        -0.5*std::pow(
                                std::abs( (x1[0]-x2[0])/lambda_val_ )
                                , alpha_)
                        );
            } else {
                if (do_jitter) ret += J_;
            }
            return std::vector<double> (1,ret);
        }

        unsigned get_ndims_x1() const {return 1;}
        unsigned get_ndims_x2() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        ParticlesTemp get_input_particles() const
        {
            ParticlesTemp ret;
            ret.push_back(theta_);
            ret.push_back(lambda_);
            ret.push_back(sigma_);
            return ret;
        }

        ContainersTemp get_input_containers() const
        {
            ContainersTemp ret;
            return ret;
        }


        IMP_OBJECT_INLINE(ReparametrizedCovariance1DFunction, out << 
                "Reparametrized covariance function with alpha = " 
                << alpha_ << std::endl, {});


    private:
        double alpha_;
        Pointer<Particle> sigma_,lambda_,theta_;
        double theta_val_,lambda_val_,sigma_val_,J_;
        bool do_jitter;
    
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FUNCTIONS_H */
