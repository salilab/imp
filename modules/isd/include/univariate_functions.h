/**
 *  \file IMP/isd/univariate_functions.h
 *  \brief Classes for general functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_UNIVARIATE_FUNCTIONS_H
#define IMPISD_UNIVARIATE_FUNCTIONS_H

#include <IMP/isd/isd_config.h>
#include <IMP/Particle.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/Switching.h>
#include <IMP/base/Object.h>
#include <Eigen/Dense>

#define IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM 1e-7

IMPISD_BEGIN_NAMESPACE


//! Base class for functions of one variable
class IMPISDEXPORT UnivariateFunction : public base::Object
{
 public:

 UnivariateFunction(std::string str) : Object(str) {}

     //! evaluate the function at a certain point
     virtual Floats operator() (const Floats& x) const = 0;

     //! evaluate the function at a list of points
     virtual Eigen::VectorXd operator() (
             const IMP::FloatsList& xlist) const = 0;

     //! used for testing only
     virtual FloatsList operator() (const IMP::FloatsList& xlist,
             bool stupid) const = 0;

     //! return true if internal parameters have changed.
     virtual bool has_changed() const = 0;

     //! update internal parameters
     virtual void update() = 0;

     //! update derivatives of particles
     /* add to each particle the derivative of the function
      * times the weight of the DA.
      */
     virtual void add_to_derivatives(const Floats& x,
             DerivativeAccumulator &accum) const = 0;

     //! update derivatives of particles
     /* add to the given particle the specified derivative
      * guarantees that the particle_no (starting at 0) matches with
      * the columns of get_derivative_matrix.
      */
     virtual void add_to_particle_derivative(unsigned particle_no,
             double value, DerivativeAccumulator &accum) const = 0;

     //! return derivative vector
     /* m_ij = d(func(xlist[i]))/dparticle_j
      * the matrix has N rows and M columns
      * where N = xlist.size() and M is the number of particles
      * associated to this function.
      */
     virtual Eigen::VectorXd get_derivative_vector(
             unsigned particle_no, const FloatsList& xlist) const = 0;

     //! for testing purposes
     virtual FloatsList get_derivative_matrix(
             const FloatsList& xlist,
             bool stupid) const = 0;

     //! return second derivative vector
     virtual Eigen::VectorXd get_second_derivative_vector(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist) const = 0;

     //! for testing purposes
     virtual FloatsList get_second_derivative_vector(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist, bool stupid) const = 0;

     //! returns the number of input dimensions
     virtual unsigned get_ndims_x() const = 0;

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

     IMP_REF_COUNTED_DESTRUCTOR(UnivariateFunction);
};
//
//! Linear one-dimensional function
/* f(x) = a*x + b, where a,b are ISD nuisances, and f(x) and x are doubles.
 */
class IMPISDEXPORT Linear1DFunction : public UnivariateFunction
{
    public:
        Linear1DFunction(Particle * a, Particle * b)
            : UnivariateFunction("Linear1DFunction %1%"), a_(a), b_(b)
        {
            IMP_LOG_TERSE( "Linear1DFunction: constructor" << std::endl);
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(a); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(b); }
            a_val_ = Nuisance(a).get_nuisance();
            b_val_ = Nuisance(b).get_nuisance();
        }

        bool has_changed() const {
            double tmpa = Nuisance(a_).get_nuisance();
            double tmpb = Nuisance(b_).get_nuisance();
            if ((std::abs(tmpa - a_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM)
                    || (std::abs(tmpb - b_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM))
            {
                IMP_LOG_TERSE( "Linear1DFunction: has_changed():");
                IMP_LOG_TERSE( "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            a_val_ = Nuisance(a_).get_nuisance();
            b_val_ = Nuisance(b_).get_nuisance();
            IMP_LOG_TERSE( "Linear1DFunction: update()  a:= "
                    << a_val_ << " b:=" << b_val_ << std::endl);
        }

        Floats operator()(const Floats& x) const {
            IMP_USAGE_CHECK(x.size() == 1, "expecting a 1-D vector");
            Floats ret(1,a_val_*x[0]+b_val_);
            return ret;
        }

        Eigen::VectorXd operator()(const FloatsList& xlist) const
        {
            unsigned M = xlist.size();
            Eigen::VectorXd retlist(M);
            for (unsigned i = 0; i < M; i++)
            {
                IMP_USAGE_CHECK(xlist[i].size() == 1,
                        "expecting a 1-D vector");
                retlist(i) = a_val_*xlist[i][0]+b_val_;
            }
            return retlist;
        }

        FloatsList operator()(const FloatsList& xlist, bool) const
        {
            Eigen::VectorXd vec((*this)(xlist));
            FloatsList ret;
            for (unsigned i=0; i<xlist.size(); i++)
                ret.push_back(Floats(1,vec(i)));
            return ret;
        }

        void add_to_derivatives(const Floats& x,
                DerivativeAccumulator &accum) const
        {
            //d[f(x)]/da = x
            Nuisance(a_).add_to_nuisance_derivative(x[0], accum);
            //d[f(x)]/db = 1
            Nuisance(b_).add_to_nuisance_derivative(1, accum);
        }

        void add_to_particle_derivative(unsigned particle_no,
                double value, DerivativeAccumulator &accum) const
        {
            switch (particle_no)
            {
                case 0:
                    Nuisance(a_).add_to_nuisance_derivative(value, accum);
                    break;
                case 1:
                    Nuisance(b_).add_to_nuisance_derivative(value, accum);
                    break;
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        Eigen::VectorXd get_derivative_vector(
                unsigned particle_no, const FloatsList& xlist) const
        {
            unsigned N=xlist.size();
            Eigen::VectorXd ret(N);
            switch(particle_no)
            {
                case 0: // a
                    for (unsigned i=0; i<N; i++)
                        ret(i) = xlist[i][0];
                    break;
                case 1: // b
                    ret.setOnes();
                    break;
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
            return ret;
        }

        FloatsList get_derivative_matrix(
             const FloatsList& xlist, bool) const
        {
            Eigen::MatrixXd mat(xlist.size(),2);
            mat.col(0) = get_derivative_vector(0, xlist);
            mat.col(1) = get_derivative_vector(1, xlist);
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

        Eigen::VectorXd get_second_derivative_vector(
                unsigned, unsigned, const FloatsList& xlist) const
        {
            // The Hessian is zero for all particles.
            unsigned N=xlist.size();
            Eigen::VectorXd H(Eigen::VectorXd::Zero(N));
            return H;
        }

        FloatsList get_second_derivative_vector(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist, bool) const
        {
            Eigen::VectorXd mat( get_second_derivative_vector(
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

        unsigned get_ndims_x() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        unsigned get_number_of_particles() const { return 2; }

        bool get_particle_is_optimized(unsigned particle_no) const
        {
            switch(particle_no)
            {
                case 0: //a
                    return Nuisance(a_).get_nuisance_is_optimized();
                case 1: //b
                    return Nuisance(b_).get_nuisance_is_optimized();
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        unsigned get_number_of_optimized_particles() const
        {
            unsigned count = 0;
            if (Nuisance(a_).get_nuisance_is_optimized()) count++;
            if (Nuisance(b_).get_nuisance_is_optimized()) count++;
            return count;
        }

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

//! 1D mean function for SAS data
/*  Generalized Guinier-Porod model (Hammouda, J. Appl. Cryst., 2010, eq 3 & 4
 *  to which a constant offset is added)
 *  I(q) = A + G/q^s exp(-(q.Rg)^2/(3-s)) for q <= q1
 *  I(q) = A + D/q^d for q > q1
 *  q1 = 1/Rg * ((d-s)(3-s)/2)^(1/2)
 *  D = G exp(-(q1.Rg)^2/(3-s)) q1^(d-s)
 *  only valid for q>0 Rg>0  0<s<d s<3 G>0
 *  s=0 in the globular case.
 *  G, Rg, d, s and A are particles (ISD Scales).
 */
class IMPISDEXPORT GeneralizedGuinierPorodFunction : public UnivariateFunction
{
    public:
        GeneralizedGuinierPorodFunction(Particle * G, Particle * Rg,
                Particle * d, Particle * s, Particle * A)
            : UnivariateFunction("GeneralizedGuinierPorodFunction %1%"),
            G_(G), Rg_(Rg), d_(d), s_(s), A_(A)
        {
            IMP_LOG_TERSE( "GeneralizedGuinierPorodFunction: constructor"
                            << std::endl);
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(G); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(Rg); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(d); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(s); }
            IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(A); }
            update();
        }

        bool has_changed() const {
            double tmpG = Scale(G_).get_scale();
            double tmpRg = Scale(Rg_).get_scale();
            double tmpd = Scale(d_).get_scale();
            double tmps = Scale(s_).get_scale();
            double tmpA = Nuisance(A_).get_nuisance();
            if ((std::abs(tmpG - G_val_) > IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM)
                    || (std::abs(tmpRg - Rg_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM)
                    || (std::abs(tmpd - d_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM)
                    || (std::abs(tmps - s_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM)
                    || (std::abs(tmpA - A_val_) >
                        IMP_ISD_UNIVARIATE_FUNCTIONS_MINIMUM))
            {
                IMP_LOG_TERSE(
                        "GeneralizedGuinierPorodFunction: has_changed():");
                IMP_LOG_TERSE( "true" << std::endl);
                return true;
            } else {
                return false;
            }
        }

        void update() {
            G_val_ = Scale(G_).get_scale();
            Rg_val_ = Scale(Rg_).get_scale();
            d_val_ = Scale(d_).get_scale();
            s_val_ = Scale(s_).get_scale();
            A_val_ = Nuisance(A_).get_nuisance();
            q1_param_ = std::sqrt((d_val_-s_val_)*(3-s_val_)/2.);
            D_param_ = G_val_ *std::exp(-IMP::square(q1_param_)/(3-s_val_));
            q1_param_ = q1_param_ / Rg_val_;
            D_param_ *= std::pow(q1_param_,d_val_-s_val_);
            IMP_LOG_TERSE( "GeneralizedGuinierPorodFunction: update()  G:= "
                            << G_val_
                    << " Rg:=" << Rg_val_
                    << " d:=" << d_val_
                    << " s:=" << s_val_
                    << " A:=" << A_val_
                    << " Q1.Rg =" << q1_param_*Rg_val_
                    << " D =" << D_param_
                    << std::endl);
            /*std::cout << "cpp"
                << " 3:Q1 " << q1_param_
                << " 5:D " << D_param_
                << " 7:G " << G_val_
                << " 9:Rg " << Rg_val_
                << " 11:d " << d_val_
                << " 13:s " << s_val_
                << " 15:val " << get_value(0.1)
                <<std::endl;*/
        }

        Floats operator()(const Floats& x) const {
            IMP_USAGE_CHECK(x.size() == 1, "expecting a 1-D vector");
            Floats ret(1,get_value(x[0]));
            return ret;
        }

        Eigen::VectorXd operator()(const FloatsList& xlist) const
        {
            unsigned M=xlist.size();
            Eigen::VectorXd retlist(M);
            for (unsigned i = 0; i < M; i++)
            {
                IMP_USAGE_CHECK(xlist[i].size() == 1,
                        "expecting a 1-D vector");
                retlist(i) = get_value(xlist[i][0]);
            }
            return retlist;
        }

        FloatsList operator()(const FloatsList& xlist, bool) const
        {
            Eigen::VectorXd vec((*this)(xlist));
            FloatsList ret;
            for (unsigned i=0; i<xlist.size(); i++)
                ret.push_back(Floats(1,vec(i)));
            return ret;
        }

        void add_to_derivatives(const Floats& x,
                DerivativeAccumulator &accum) const
        {
            double qval = x[0];
            double value = get_value(qval) - A_val_;
            double deriv;
            //d[f(x)+A]/dG = f(x)/G
            deriv = value/G_val_;
            IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for G is nan.");
            Scale(G_).add_to_nuisance_derivative(deriv, accum);
            if (qval <= q1_param_)
            {
                //d[f(x)]/dRg = - f(x) * 2 q^2 Rg / (3-s)
                deriv = - value * 2*IMP::square(qval)*Rg_val_/(3-s_val_);
                IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for Rg is nan.");
                Scale(Rg_).add_to_nuisance_derivative(deriv, accum);
                //d[f(x)]/dd = 0
                //
                //d[f(x)]/ds = - f(x) * ( (q Rg / (3-s))^2 + log(q) )
                deriv = - value
                    * (IMP::square((qval*Rg_val_)/(3-s_val_)) + std::log(qval));
                IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for s is nan.");
                Scale(s_).add_to_nuisance_derivative(deriv, accum);
            } else {
                //d[f(x)]/dRg = f(x) * (s-d)/Rg
                deriv = value * (s_val_-d_val_)/Rg_val_;
                IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for Rg is nan.");
                Scale(Rg_).add_to_nuisance_derivative(deriv, accum);
                //d[f(x)]/dd = f(x) * log(q1/q)
                deriv = value * std::log(q1_param_/qval);
                IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for d is nan.");
                Scale(d_).add_to_nuisance_derivative(deriv, accum);
                //d[f(x)]/ds = - f(x) * ( (d-s)/(2(3-s)) + log(q1) )
                deriv = - value * ( (d_val_-s_val_)/(2*(3-s_val_))
                                        + std::log(q1_param_) );
                IMP_INTERNAL_CHECK(!base::isnan(deriv),
                        "derivative for d is nan.");
                Scale(s_).add_to_nuisance_derivative(deriv, accum);
            }
            //d[f(x)+A]/dA = 1
            deriv = 1;
            Nuisance(A_).add_to_nuisance_derivative(deriv, accum);
        }

        void add_to_particle_derivative(unsigned particle_no,
                double value, DerivativeAccumulator &accum) const
        {
            switch (particle_no)
            {
                case 0:
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "derivative for G is nan.");
                    Scale(G_).add_to_scale_derivative(value, accum);
                    break;
                case 1:
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "derivative for Rg is nan.");
                    Scale(Rg_).add_to_scale_derivative(value, accum);
                    break;
                case 2:
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "derivative for d is nan.");
                    Scale(d_).add_to_scale_derivative(value, accum);
                    break;
                case 3:
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "derivative for s is nan.");
                    Scale(s_).add_to_scale_derivative(value, accum);
                    break;
                case 4:
                    IMP_INTERNAL_CHECK(!base::isnan(value),
                        "derivative for A is nan.");
                    Nuisance(A_).add_to_nuisance_derivative(value, accum);
                    break;
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        Eigen::VectorXd get_derivative_vector(
                unsigned particle_no, const FloatsList& xlist) const
        {
            unsigned N=xlist.size();
            Eigen::VectorXd ret(N);
            switch(particle_no)
            {
                case 0: // G
                    //d[f(x)]/dG = f(x)/G
                    ret = ((*this)(xlist)
                            -Eigen::VectorXd::Constant(N,A_val_))/G_val_;
                    break;
                case 1: // Rg
                    for (unsigned i=0; i<N; i++)
                    {
                        double qval = xlist[i][0];
                        if (qval <= q1_param_)
                        {
                            //d[f(x)]/dRg = - f(x) * 2 q^2 Rg / (3-s)
                            ret(i) = - (get_value(qval) - A_val_)
                                * 2*IMP::square(qval)*Rg_val_/(3-s_val_);
                        } else {
                            //d[f(x)]/dRg = f(x) * (s-d)/Rg
                            ret(i) = (get_value(qval)-A_val_)
                                        * (s_val_-d_val_)/Rg_val_;
                        }
                    }
                    break;
                case 2: // d
                    for (unsigned i=0; i<N; i++)
                    {
                        double qval = xlist[i][0];
                        if (qval <= q1_param_)
                        {
                            //d[f(x)]/dd = 0
                            ret(i) = 0;
                        } else {
                            //d[f(x)]/dd = f(x) * log(q1/q)
                            ret(i) = (get_value(qval)-A_val_)
                                    * std::log(q1_param_/qval);
                        }
                    }
                    break;
                case 3: // s
                    for (unsigned i=0; i<N; i++)
                    {
                        double qval = xlist[i][0];
                        if (qval <= q1_param_)
                        {
                            //d[f(x)]/ds = - f(x)
                            //    * (q^2 Rg^2 + (3-s)^2 log(q)) / (3-s)^2
                            ret(i) = - (get_value(qval) - A_val_)
                                        * (IMP::square((qval*Rg_val_)
                                                        /(3-s_val_))
                                            + std::log(qval));
                        } else {
                            //d[f(x)]/ds = - f(x) * ( (d-s)/(2(3-s)) + log(q1) )
                            ret(i) = - (get_value(qval) - A_val_)
                                        * ( (d_val_-s_val_)/(2*(3-s_val_))
                                            + std::log(q1_param_) );
                        }
                    }
                    break;
                case 4: // A
                    ret = Eigen::VectorXd::Constant(N,1);
                    break;
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
            return ret;
        }

        FloatsList get_derivative_matrix(
             const FloatsList& xlist, bool) const
        {
            Eigen::MatrixXd mat(xlist.size(),5);
            mat.col(0) = get_derivative_vector(0, xlist);
            mat.col(1) = get_derivative_vector(1, xlist);
            mat.col(2) = get_derivative_vector(2, xlist);
            mat.col(3) = get_derivative_vector(3, xlist);
            mat.col(4) = get_derivative_vector(4, xlist);
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

        Eigen::VectorXd get_second_derivative_vector(
                unsigned particle_a, unsigned particle_b,
                const FloatsList& xlist) const
        {
            if (particle_a >=5)
                    IMP_THROW("Invalid particle 1 number", ModelException);
            if (particle_b >= 5)
                    IMP_THROW("Invalid particle 2 number", ModelException);
            unsigned N=xlist.size();
            //hessian involving A is always 0
            if (particle_a == 4 || particle_b == 4)
                return Eigen::VectorXd::Zero(N);
            unsigned pa = std::min(particle_a,particle_b);
            unsigned pb = std::max(particle_a,particle_b);
            Eigen::VectorXd ret(N);
            switch(pa)
            {
                case 0: //G
                    switch(pb)
                    {
                        case 0: //G
                            //d2f/dG2 = 0
                            ret.noalias() = Eigen::VectorXd::Zero(N);
                            break;

                        case 1: //Rg
                            //d2f/dGdRg = df/dRg * 1/G
                            ret.noalias() =
                                get_derivative_vector(1, xlist)/G_val_;
                            break;

                        case 2: //d
                            //d2f/dGdd = df/dd * 1/G
                            ret.noalias() =
                                get_derivative_vector(2, xlist)/G_val_;
                            break;

                        case 3: //s
                            //d2f/dGds = df/ds * 1/G
                            ret.noalias() =
                                get_derivative_vector(3, xlist)/G_val_;
                            break;

                        default:
                            IMP_THROW("Invalid particle 2 number",
                                    ModelException);
                            break;
                    }
                    break;

                case 1: // Rg
                    switch(pb)
                    {
                        case 1: //Rg
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/dRg2 =
                                    //f(x) * (2 q^2 / (3-s))
                                    //  * (2 q^2 Rg^2 / (3-s) - 1)
                                    ret(i) = (get_value(qval) - A_val_)
                                * 2*IMP::square(qval)/(3-s_val_)
                                * (2*IMP::square(qval*Rg_val_)/(3-s_val_) -1);
                                } else {
                                    //d2[f(x)]/dRg2=f(x) * (d-s)/Rg^2 * (d-s+1)
                                    ret(i) = (get_value(qval) - A_val_)
                                * (d_val_-s_val_)/IMP::square(Rg_val_)
                                * (d_val_-s_val_+1);
                                }
                            }
                            break;

                        case 2: //d
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/dddRg = 0
                                    ret(i) = 0;
                                } else {
                                    //d2[f(x)]/dddRg = -f(x)/Rg
                                    //           - (d-s)/Rg *df(x)/dd
                                    double val=(get_value(qval)-A_val_);
                                    ret(i) = -val/Rg_val_
                                        - (val*std::log(q1_param_/qval)
                                * (d_val_-s_val_)/Rg_val_);
                                }
                            }
                            break;

                        case 3: //s
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                double val=(get_value(qval)-A_val_);
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/dsdRg = -2q^2Rg/(3-s)
                                    //     * (df(x)/ds + f(x)/(3-s))
                                    double deriv = - val
                                        * (IMP::square((qval*Rg_val_)
                                                        /(3-s_val_))
                                            + std::log(qval));
                                    ret(i) =
                                        -2*IMP::square(qval)*Rg_val_/(3-s_val_)
                                        *(deriv + val/(3-s_val_));
                                } else {
                                    //d2[f(x)]/dsdRg =
                                    //          1/Rg * (f(x)- (d-s)*df(x)/ds)
                                    double deriv = - val
                                        * ( (d_val_-s_val_)/(2*(3-s_val_))
                                            + std::log(q1_param_) );
                                    ret(i) = (val - (d_val_-s_val_)*deriv)
                                                /Rg_val_;
                                }
                            }
                            break;

                        default:
                            IMP_THROW("Invalid particle 2 number",
                                    ModelException);
                            break;
                    }
                    break;

                case 2: //d
                    switch(pb)
                    {
                        case 2: //d
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/dddd = 0
                                    ret(i) = 0;
                                } else {
                                    //d2[f(x)]/dddd =
                                    //         f(x)*(log(q1/q)^2 + 1/(2*(d-s)))
                                    double val=(get_value(qval)-A_val_);
                                    ret(i) = val * (
                                        IMP::square(std::log(q1_param_/qval))
                                        +1/(2*(d_val_-s_val_)));
                                }
                            }
                            break;

                        case 3: //s
                            {
                            double lterm=(d_val_-s_val_)/(2*(3-s_val_))
                                            + std::log(q1_param_);
                            double rterm=0.5*(1/(3-s_val_) + 1/(d_val_-s_val_));
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/ddds = 0
                                    ret(i) = 0;
                                } else {
                                    //d2[f(x)]/ddds = log(q1/q)*df(x)/ds
                                    //      - (1/(3-s) + 1/(d-s))*f(x)/2
                                    //
                                    double val=(get_value(qval)-A_val_);
                                    ret(i) = - val * (
                                        std::log(q1_param_/qval)*lterm + rterm);
                                }
                            }
                            }
                            break;

                        default:
                            IMP_THROW("Invalid particle 2 number",
                                    ModelException);
                            break;
                    }
                    break;

                case 3: //s
                    switch(pb)
                    {
                        case 3: //s
                            {
                            double cterm =
                                    IMP::square( 0.5*(d_val_-s_val_)/(3-s_val_)
                                                 + std::log(q1_param_) )
                            + 0.5*((6 - s_val_ - d_val_)/IMP::square(3-s_val_)
                                            + 1./(d_val_-s_val_));
                            for (unsigned i=0; i<N; i++)
                            {
                                double qval = xlist[i][0];
                                double val=(get_value(qval)-A_val_);
                                if (qval <= q1_param_)
                                {
                                    //d2[f(x)]/dsds = f(x) *
                                    //  ( [(qRg)^2/(3-s)^2 + log q ]^2
                                    //    - 2(qRg)^2/(3-s)^3 )
                                    double factor =
                                        IMP::square((qval*Rg_val_)/(3-s_val_));
                                    ret(i) = val *
                                      (IMP::square(factor + std::log(qval))
                                       - 2*factor/(3-s_val_));
                                } else {
                                    //d2[f(x)]/dsds = f(x)
                                    // * ( [ (d-s)/(2*(3-s)) + log(q1) ]^2
                                    //     + 1/2 * [ (6-s-d)/(3-s)^2
                                    //               + 1/(d-s) ] )
                                    ret(i) = val * cterm;
                                }
                            }
                            }
                            break;

                        default:
                            IMP_THROW("Invalid particle 2 number",
                                    ModelException);
                            break;
                    }
                    break;

                default:
                    IMP_THROW("Invalid particle 1 number",
                            ModelException);
                    break;
            }
            return ret;
        }

        FloatsList get_second_derivative_vector(
             unsigned particle_a, unsigned particle_b,
             const FloatsList& xlist, bool) const
        {
            Eigen::VectorXd mat( get_second_derivative_vector(
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

        unsigned get_ndims_x() const {return 1;}
        unsigned get_ndims_y() const {return 1;}

        unsigned get_number_of_particles() const { return 5; }

        bool get_particle_is_optimized(unsigned particle_no) const
        {
            switch(particle_no)
            {
                case 0: //G
                    return Scale(G_).get_scale_is_optimized();
                case 1: //Rg
                    return Scale(Rg_).get_scale_is_optimized();
                case 2: //d
                    return Scale(d_).get_scale_is_optimized();
                case 3: //s
                    return Scale(s_).get_scale_is_optimized();
                case 4: //A
                    return Nuisance(A_).get_nuisance_is_optimized();
                default:
                    IMP_THROW("Invalid particle number", ModelException);
            }
        }

        unsigned get_number_of_optimized_particles() const
        {
            unsigned count = 0;
            if (Scale(G_).get_scale_is_optimized()) count++;
            if (Scale(Rg_).get_scale_is_optimized()) count++;
            if (Scale(d_).get_scale_is_optimized()) count++;
            if (Scale(s_).get_scale_is_optimized()) count++;
            if (Nuisance(A_).get_nuisance_is_optimized()) count++;
            return count;
        }

        ParticlesTemp get_input_particles() const
        {
            ParticlesTemp ret;
            ret.push_back(G_);
            ret.push_back(Rg_);
            ret.push_back(d_);
            ret.push_back(s_);
            ret.push_back(A_);
            return ret;
        }

        ContainersTemp get_input_containers() const
        {
            ContainersTemp ret;
            return ret;
        }

        IMP_OBJECT_INLINE(GeneralizedGuinierPorodFunction, out
                << " G = " << G_val_
                << " Rg = " << Rg_val_
                << " d = " << d_val_
                << " s = " << s_val_
                << " A = " << A_val_
                << " Q1.Rg = " << q1_param_*Rg_val_
                << std::endl, {});

    private:

        inline double get_value(double q) const
        {
            double value;
            if (q <= q1_param_)
            {
                value = A_val_ + G_val_/std::pow(q,s_val_)
                    * std::exp(- IMP::square(q*Rg_val_)/(3-s_val_));
            } else {
                value = A_val_ + D_param_/std::pow(q,d_val_);
            }
            return value;
        }

        Pointer<Particle> G_,Rg_,d_,s_,A_;
        double G_val_,Rg_val_,d_val_,s_val_,A_val_,q1_param_,D_param_;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_UNIVARIATE_FUNCTIONS_H */
