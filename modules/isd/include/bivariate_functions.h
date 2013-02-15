/**
 *  \file IMP/isd/bivariate_functions.h
 *  \brief Classes for general functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_BIVARIATE_FUNCTIONS_H
#define IMPISD_BIVARIATE_FUNCTIONS_H

#include <IMP/isd/isd_config.h>
#include <IMP/Particle.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/Switching.h>
#include <IMP/base/Object.h>
#include <Eigen/Dense>

#define IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM 1e-7

IMPISD_BEGIN_NAMESPACE

//! Base class for functions of two variables
class IMPISDEXPORT BivariateFunction : public base::Object
{
 public:

 BivariateFunction(std::string str) : Object(str) {}

     //! evaluate the function at a certain point
     virtual Floats operator()
                (const Floats& x1,
                 const Floats& x2) const = 0;

     //! evaluate the function at a list of points
     /* returns a NxN matrix of entries where N=xlist.rows()
      * and entry ij of the matrix is f(xlist(i),xlist(j))
      */
     virtual Eigen::MatrixXd operator()
                (const IMP::FloatsList& xlist) const = 0;

     //! used for testing only
     virtual FloatsList operator() (const IMP::FloatsList& xlist,
             bool stupid) const = 0;

     //! return true if internal parameters have changed.
     virtual bool has_changed() const = 0;

     //! update internal parameters
     virtual void update() = 0;

     //! update derivatives of particles
     virtual void add_to_derivatives(
             const Floats& x1,
             const Floats& x2,
             DerivativeAccumulator &accum) const = 0;

     //! update derivatives of particles
     /* add to the given particle the specified derivative
      * guarantees that the particle_no (starting at 0) matches with
      * the columns of get_derivative_matrix.
      */
     virtual void add_to_particle_derivative(unsigned particle_no,
             double value, DerivativeAccumulator &accum) const = 0;

     //! return derivative matrix
     /* m_ij = d(func(xlist[i],xlist[j]))/dparticle_no
      * the matrix is NxN where N = xlist.size()
      */
     virtual Eigen::MatrixXd get_derivative_matrix(
             unsigned particle_no,
             const FloatsList& xlist) const = 0;

     //for testing purposes
     virtual FloatsList get_derivative_matrix(
             unsigned particle_no,
             const FloatsList& xlist,
             bool stupid) const = 0;

     //! return second derivative matrix
     virtual Eigen::MatrixXd get_second_derivative_matrix(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist) const = 0;

     //for testing purposes
     virtual FloatsList get_second_derivative_matrix(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist,
             bool stupid) const = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x1() const = 0;
     virtual unsigned get_ndims_x2() const = 0;

     //! returns the number of output dimensions
     virtual unsigned get_ndims_y() const = 0;

     //! returns the number of particles that this function uses
     virtual unsigned get_number_of_particles() const = 0;

     //! returns true if the particle whose index is provided is optimized
     virtual bool get_particle_is_optimized(unsigned particle_no) const = 0;

     //! returns the number of particles that are optimized
     virtual unsigned get_number_of_optimized_particles() const = 0;

     //! particle manipulation
     virtual ParticlesTemp get_input_particles() const = 0;
     virtual ContainersTemp get_input_containers() const = 0;

     IMP_REF_COUNTED_DESTRUCTOR(BivariateFunction);
};
//
//! Covariance function
/* \f[w(x,x') = \tau^2 \exp\left(-\frac{|x-x'|^\alpha}{2\lambda^\alpha} +
 * \delta_{ij} J\f]
 * \param[in] \f$\tau\f$ ISD Scale
 * \param[in] \f$\lambda\f$ ISD Scale
 * \param[in] \f$\alpha\f$ positive double, usually greater than 1.
 *             Default is 2.
 * \param[in] J is some jitter. Try J=0.01 if you get NANs.
 * \param[in] cutoff is a positive double indicating when to consider that
 * values are zero (to avoid computing exponentials). cutoff is relative to the
 * value when x=x', and affects only the call get_derivative_matrix.
 */
class IMPISDEXPORT Covariance1DFunction : public BivariateFunction
{
    public:
        Covariance1DFunction(Particle* tau, Particle* ilambda,
                double alpha=2.0, double jitter =0.0, double cutoff=1e-7) :
            BivariateFunction("Covariance1DFunction %1%"), alpha_(alpha),
            tau_(tau), lambda_(ilambda), J_(jitter),
            cutoff_(cutoff)
    {
        IMP_LOG_TERSE( "Covariance1DFunction: constructor" << std::endl);
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(tau); }
        IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(ilambda);}
        lambda_val_= Scale(ilambda).get_nuisance();
        tau_val_= Scale(tau).get_nuisance();
        do_jitter = (jitter>IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM);
        alpha_square_ = (std::abs(alpha-2) <
                IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM);
    }

        bool has_changed() const {
            double tmpt = Scale(tau_).get_nuisance();
            double tmpl = Scale(lambda_).get_nuisance();
            if ((std::abs(tmpt - tau_val_) >
                        IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM)
                || (std::abs(tmpl - lambda_val_) >
                    IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM))
            {
                IMP_LOG_TERSE( "Covariance1DFunction: has_changed():");
                IMP_LOG_TERSE( "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            lambda_val_= Scale(lambda_).get_nuisance();
            tau_val_= Scale(tau_).get_nuisance();
            IMP_LOG_TERSE( "Covariance1DFunction: update()  tau:= "
                    << tau_val_ << " lambda:=" << lambda_val_
                    << std::endl);
            IMP_INTERNAL_CHECK(!base::isnan(tau_val_),
                "tau is nan.");
            IMP_INTERNAL_CHECK(!base::isnan(lambda_val_),
                "lambda is nan.");
            IMP_INTERNAL_CHECK(tau_val_ >= 0, "tau is negative.");
            IMP_INTERNAL_CHECK(lambda_val_ >= 0, "lambda is negative.");
            IMP_INTERNAL_CHECK(lambda_val_ != 0, "lambda is zero.");
        }

        Floats operator()(const Floats& x1,
                const Floats& x2) const
        {
            IMP_USAGE_CHECK(x1.size() == 1, "expecting a 1-D vector");
            IMP_USAGE_CHECK(x2.size() == 1, "expecting a 1-D vector");
            Floats ret(1, get_value(x1[0],x2[0]));
            return ret;
        }

        Eigen::MatrixXd operator()(const IMP::FloatsList& xlist) const
        {
            const unsigned M=xlist.size();
            Eigen::MatrixXd Mret(M,M);
            for (unsigned i=0; i<M; i++)
            {
                for (unsigned j=i; j<M; j++)
                {
                    IMP_USAGE_CHECK(xlist[i].size() == 1,
                            "expecting a 1-D vector");
                    IMP_USAGE_CHECK(xlist[j].size() == 1,
                            "expecting a 1-D vector");
                    double x1 = xlist[i][0];
                    double x2 = xlist[j][0];
                    double ret = get_value(x1,x2);
                    Mret(i,j) = ret;
                    if (i != j) Mret(j,i) = ret;
                }
            }
            return Mret;
        }

        FloatsList operator()(const IMP::FloatsList& xlist, bool) const
        {
            Eigen::MatrixXd mat((*this)(xlist));
            FloatsList ret;
            for (unsigned i=0; i<xlist.size(); i++)
                for (unsigned j=0; j<xlist.size(); j++)
                    ret.push_back(Floats(1,mat(i,j)));
            return ret;
        }

        void add_to_derivatives(const Floats& x1,
                const Floats& x2,
                DerivativeAccumulator &accum) const
        {
            //d[w(x1,x2)]/dtau = 2/tau*(w(x1,x2))
            double val = get_value(x1[0],x2[0]);
            double tauderiv = 2./tau_val_ * val;
            IMP_INTERNAL_CHECK(!base::isnan(tauderiv),
                "tau derivative is nan.");
            Scale(tau_).add_to_nuisance_derivative(tauderiv, accum);
            //d[w(x,x')]/dlambda
            // = w(x,x') ( alpha |x'-x|^alpha/(2 lambda^{alpha+1}))
            double lambdaderiv =
                    val * (alpha_ *
                        std::pow((std::abs(x1[0]-x2[0])/lambda_val_),alpha_)
                        /(2.*lambda_val_));
            IMP_INTERNAL_CHECK(!base::isnan(lambdaderiv),
                "lambda derivative is nan.");
            Scale(lambda_).add_to_nuisance_derivative(lambdaderiv, accum);
        }

        void add_to_particle_derivative(unsigned particle_no,
             double value, DerivativeAccumulator &accum) const
        {
            switch (particle_no)
            {
                case 0: //tau
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "tau derivative is nan.");
                    Scale(tau_).add_to_nuisance_derivative(value, accum);
                    break;
                case 1: //lambda
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "lambda derivative is nan.");
                    Scale(lambda_).add_to_nuisance_derivative(value, accum);
                    break;
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        Eigen::MatrixXd get_derivative_matrix(
             unsigned particle_no,
             const FloatsList& xlist) const
        {
            // Strategy: fill in the main diagonal, then fill with zeros
            // if the value of the function falls below cutoff.
            // assumes data points are ordered!
            unsigned N=xlist.size();
            Eigen::MatrixXd ret(N,N);
            double diag;
            switch (particle_no)
            {
                case 0: //tau
                    //d[w(x1,x1)]/dtau
                    //  = 2/tau*w(x1,x1)
                    diag = get_value(xlist[0][0],xlist[0][0]);
                    diag *= 2./tau_val_;
                    break;
                case 1: //lambda
                    //d[w(x,x)]/dlambda
                    //= w(x,x)
                    //  *( alpha /(2 lambda^{alpha+1}))
                    diag = 0;
                    break;
                default:
                    IMP_THROW("Invalid particle number",
                            ModelException);
            }
            IMP_INTERNAL_CHECK(!base::isnan(diag),
                "derivative matrix is nan on the diagonal.");
            for (unsigned i=0; i<N; i++) ret(i,i) = diag;
            //
            bool initial_loop=true;
            double abs_cutoff = cutoff_*diag;
            double dmax=-1;
            for (unsigned i=0; i<N; i++)
            {
                for (unsigned j=i+1; j<N; j++)
                {
                    double x1(xlist[i][0]), x2(xlist[j][0]);
                    double val;
                    double dist(std::abs(x1-x2));
                    //compute all entries as long as the cutoff distance was
                    //not recorded (initial_loop) or as long as the distance is
                    //smaller than the cutoff distance
                    if (initial_loop || dist <= dmax)
                    {
                        switch (particle_no)
                        {
                            case 0: //tau
                                //d[w(x1,x2)]/dtau
                                //  = 2/tau*w(x1,x2)
                                val = get_value(xlist[i][0],xlist[j][0]);
                                val *= 2./tau_val_;
                                break;
                            case 1: //lambda
                                //d[w(x,x')]/dlambda
                                //= w(x,x')
                                //  *( alpha |x'-x|^alpha/(2 lambda^{alpha+1}))
                                if (dist<IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM) {
                                    val = 0;
                                } else {
                                    val = get_value(xlist[i][0],xlist[j][0]);
                                    val *= alpha_ *
                                        std::pow(
                                            (dist/lambda_val_), alpha_)
                                        /(2.*lambda_val_);
                                }
                                break;
                            default:
                                IMP_THROW("Invalid particle number",
                                        ModelException);
                        }
                        // the value has been computed and is in val
                        //now check if it is smaller than the cutoff.
                        //If true change the flag and update the distance
                        if (std::abs(val) <= abs_cutoff)
                        {
                            if (initial_loop)
                            {
                                initial_loop = false;
                                dmax = dist;
                            } else if (dist < dmax)
                            {
                                dmax = dist;
                            }
                        }
                    } else { // e.g. initial_loop == false && dist > dmax
                        val = 0;
                    }
                    IMP_INTERNAL_CHECK(!base::isnan(val),
                        "derivative matrix is nan at position("
                        << i << "," << j << ").");
                    ret(i,j) = val;
                    ret(j,i) = val;
                }
            }
            return ret;
        }

        FloatsList get_derivative_matrix(
             unsigned particle_no,
             const FloatsList& xlist, bool) const
        {
            Eigen::MatrixXd mat(get_derivative_matrix(particle_no, xlist));
            FloatsList ret;
            for (int i=0; i<mat.rows(); i++)
            {
                Floats line;
                for (int j=0; j<mat.cols(); j++)
                    line.push_back(mat(i,j));
                ret.push_back(line);
            }
            return ret;
        }

        Eigen::MatrixXd get_second_derivative_matrix(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist) const
        {
            unsigned N(xlist.size());
            Eigen::MatrixXd ret(N,N);
            if (particle_a > 1)
                    IMP_THROW("Invalid particle 1 number", ModelException);
            if (particle_b > 1)
                    IMP_THROW("Invalid particle 2 number", ModelException);
            //d2f/dtau2
            if (particle_a == 0 && particle_b == 0)
            {
                for (unsigned i=0; i<N; i++)
                {
                    for (unsigned j=i; j<N; j++)
                    {
                        double dist = std::abs(xlist[i][0]-xlist[j][0]);
                        double exponent = std::pow( dist/lambda_val_ , alpha_);
                        double expterm = std::exp(-0.5*exponent);
                        ret(i,j) = 2*expterm;
                        if (i!=j) ret(j,i) = ret(i,j);
                    }
                }
            } else if (particle_a == 1 && particle_b == 1) { //d2f/dlambda2
                for (unsigned i=0; i<N; i++)
                {
                    for (unsigned j=i; j<N; j++)
                    {
                        double dist = std::abs(xlist[i][0]-xlist[j][0]);
                        double exponent = std::pow( dist/lambda_val_ , alpha_);
                        double expterm = std::exp(-0.5*exponent);
                        ret(i,j) = tau_val_*tau_val_*expterm
                                   *exponent/(lambda_val_*lambda_val_)*alpha_/2
                            * (alpha_/2 * exponent - (alpha_+1));
                        if (i!=j) ret(j,i) = ret(i,j);
                    }
                }
            } else { // d2f/d(tau)d(lambda)
                for (unsigned i=0; i<N; i++)
                {
                    for (unsigned j=i; j<N; j++)
                    {
                        double dist = std::abs(xlist[i][0]-xlist[j][0]);
                        double exponent = std::pow( dist/lambda_val_ , alpha_);
                        double expterm = std::exp(-0.5*exponent);
                        ret(i,j) = tau_val_ * alpha_ * expterm / lambda_val_
                                    * exponent;
                        if (i!=j) ret(j,i) = ret(i,j);
                    }
                }
            }
            return ret;
        }

        FloatsList get_second_derivative_matrix(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist, bool) const
        {
            Eigen::MatrixXd mat( get_second_derivative_matrix(
                        particle_a, particle_b, xlist));
            FloatsList ret;
            for (int i=0; i<mat.rows(); i++)
            {
                Floats line;
                for (int j=0; j<mat.cols(); j++)
                    line.push_back(mat(i,j));
                ret.push_back(line);
            }
            return ret;
        }

        unsigned get_ndims_x1() const {return 1;}
        unsigned get_ndims_x2() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        unsigned get_number_of_particles() const { return 2; }

        bool get_particle_is_optimized(unsigned particle_no) const
        {
            switch(particle_no)
            {
                case 0: //tau
                    return Scale(tau_).get_nuisance_is_optimized();
                case 1: //lambda
                    return Scale(lambda_).get_nuisance_is_optimized();
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        unsigned get_number_of_optimized_particles() const
        {
            unsigned count=0;
            if (Scale(tau_).get_nuisance_is_optimized()) count++;
            if (Scale(lambda_).get_nuisance_is_optimized()) count++;
            return count;
        }

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


        IMP_OBJECT_INLINE(Covariance1DFunction,
                out << "covariance function with alpha = "
                << alpha_ << std::endl, {});


    private:

        inline double get_value(double x1, double x2) const
        {
            double dist = std::abs(x1-x2);
            double ret = dist /lambda_val_ ;
            if (alpha_square_)
            {
                ret  *= ret;
            } else {
                ret = std::pow(ret, alpha_);
            }
            ret = IMP::square(tau_val_) *std::exp(-0.5*ret);
            if (do_jitter && dist<IMP_ISD_BIVARIATE_FUNCTIONS_MINIMUM)
            {
                ret += J_;
            }
            IMP_INTERNAL_CHECK(!base::isnan(ret),
                "function value is nan. tau = "
                << tau_val_ << " lambda = " << lambda_val_
                << " q1 = " << x1 << " q2 = " << x2);
            return ret;
        }

    private:
        double alpha_;
        Pointer<Particle> tau_,lambda_;
        double tau_val_,lambda_val_,J_,cutoff_,alpha_square_;
        bool do_jitter;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_BIVARIATE_FUNCTIONS_H */
