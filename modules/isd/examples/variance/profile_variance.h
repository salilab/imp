/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_PROFILE_VARIANCE_H
#define IMPISD_PROFILE_VARIANCE_H


#include <string>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <Eigen/Dense>
#include <boost/random/normal_distribution.hpp>

#include <IMP.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/base.h>
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/isd.h>
#include <IMP/algebra.h>
#include <IMP/macros.h>
#include <IMP/base/flags.h>
#include <IMP/base/random.h>

#include <IMP/saxs/internal/sinc_function.h>
#include <IMP/saxs/Profile.h>


namespace {
using namespace IMP;
using namespace IMP::base;
using namespace IMP::isd;
using Eigen::Array3d;
using Eigen::ArrayXd;
using Eigen::ArrayXXd;
using Eigen::MatrixXd;
using saxs::internal::SincFunction;
using saxs::internal::SincCosFunction;
typedef Eigen::Array<double,Eigen::Dynamic, 3> ArrayCoord;
}

Particles get_particles(Model *m) {
    IMP_NEW(atom::CAlphaPDBSelector, sel, () );
    atom::Hierarchy mp = atom::read_pdb(
            saxs::get_example_path("6lyz.pdb"), m, sel);
    Particles p = atom::get_by_type(mp, atom::ATOM_TYPE);
    return p;
}

ArrayCoord get_coords(Model *m, Particles p) {
    ArrayCoord coords(p.size(), 3);
    for (unsigned i=0; i<p.size(); ++i){
        algebra::Vector3D tmp(core::XYZ(p[i]).get_coordinates());
        for (unsigned j=0; j<3; ++j)
            coords(i,j) = tmp[j];
    }
    return coords;
}

ArrayXd get_form_factors(Particles p){
    saxs::FormFactorTable *fft = saxs::default_form_factor_table();
    ArrayXd ffs(p.size());
    for (unsigned i=0; i< p.size(); ++i)
        ffs(i) = fft->get_form_factor(p[i], saxs::CA_ATOMS);
    return ffs;
}

ArrayCoord perturb_particles(const ArrayCoord coords, double tau=0.1){
  static boost::normal_distribution<Float> mrng(0., 1.);
  static boost::variate_generator<base::RandomNumberGenerator&,
                                  boost::normal_distribution<Float> >
                  sampler(base::random_number_generator, mrng);
  ArrayCoord newcoords(coords.rows(),3);
  for (unsigned i=0; i<coords.rows(); i++)
      for (unsigned j=0; j<3; j++)
          newcoords(i,j) = coords(i,j) + sampler()*tau;
  return newcoords;
}

ArrayXd get_distances(Eigen::Array3d atom, ArrayCoord coords){
    return (coords.rowwise()-atom.transpose()).square().rowwise().sum().sqrt();
}

ArrayXd compute_profile(ArrayCoord coords, ArrayXd ffs, SincFunction& sf,
        double qmin=0., double qmax=0.5, unsigned num=101){
    //I(q) = E^2(q) sum_{i<=j} fi(0) fj(0) sinc(q dij)
    //E^2(q) = exp(- q^2 * 0.23 )
    ArrayXd profile(num);
    //add autocorrelation, dij = 0
    profile.setConstant(ffs.square().sum());
    //
    //add other terms
    for (unsigned i=0; i<ffs.rows()-1; ++i){
        ArrayXd dists = get_distances(coords.row(i),
                coords.bottomRows(ffs.rows()-i-1));
        for (unsigned j=0; j<dists.rows(); ++j){
            double dij = dists(j);
            double fij = 2*ffs(i)*ffs(i+1+j);
            for (unsigned nq=0; nq<num; ++nq){
                double q = qmin + (qmax-qmin)*nq/(num-1);
                double sinc = sf.sinc(q*dij);
                profile(nq) += fij*sinc;
            }
        }
    }
    //add q dependence for form factors
    for (unsigned nq=0; nq<num; ++nq){
        double q = qmin + (qmax-qmin)*nq/(num-1);
        double factor = std::exp(-q*q*0.23);
        profile(nq) *= factor;
    }
    return profile;
}

ArrayXd compute_blurred_profile(ArrayCoord coords, ArrayXd ffs, double tau,
        SincFunction& sf, double qmin=0., double qmax=0.5, unsigned num=101){
    ArrayXd profile = compute_profile(coords, ffs, sf, qmin, qmax, num);
    for (unsigned nq=0; nq<num; ++nq){
        double q = qmin + (qmax-qmin)*nq/(num-1);
        double factor = std::exp(-q*q*tau*tau);
        profile(nq) *= factor;
    }
    return profile;
}

ArrayXXd compute_autocovariance(ArrayCoord coords, ArrayXd ffs, double tau,
        SincCosFunction& sf, double qmin=0., double qmax=0.5, unsigned num=101){
    //V(qi,qj) = E^2(qi) E^2(qj) sum_k fk(0)^2 sum_l fl(0)^2 Vij0(dkl)
    //Vij0(dkl) = 2*tau^2*qi*qj*sigma(qi*dkl)*sigma(qj*dkl)
    //sigma(x) = (sinc(x)-cos(x))/x
    //
    ArrayXXd autocov = ArrayXXd::Zero(num,num);
    for (unsigned k=0; k<ffs.size()-1; ++k){
        ArrayXd dists = get_distances(coords.row(k),
                coords.bottomRows(ffs.rows()-k-1));
        for (unsigned l=0; l<dists.rows(); ++l){
            double dkl = dists(l);
            double fkl = square(ffs(k)*ffs(k+1+l));
            for (unsigned nqi=0; nqi<num; ++nqi){
                double qi = qmin + (qmax-qmin)*nqi/(num-1);
                double si = sf.sico(qi*dkl);
                for (unsigned nqj=nqi; nqj<num; ++nqj){
                    double qj = qmin + (qmax-qmin)*nqj/(num-1);
                    double sj = sf.sico(qj*dkl);
                    autocov(nqi,nqj) += fkl*qi*qj*si*sj;
                } //qj
            } //qi
        } //l
    } //k
    //
    ArrayXd factor = (-ArrayXd::LinSpaced(num,qmin,qmax).square()*0.23).exp();
    autocov.colwise() *= factor;
    autocov.rowwise() *= factor.transpose();
    //factor 2 for the half-sum
    return MatrixXd((2*(2*tau*tau)*autocov).matrix(
                                ).selfadjointView<Eigen::Upper>()).array();
}

double get_rho(Array3d ck, Array3d cl, Array3d cn, double tau){
    //rho(dkl,dkn) = (2*dkl.dkn + 3*tau^2)/(4*dkl*dkn)
    double scal=((cl-ck)*(cn-ck)).sum();
    double den=4*std::sqrt((cl-ck).square().sum()*(cn-ck).square().sum());
    return (2*scal+3*tau*tau)/den;
}

ArrayXXd compute_crosscovariance(ArrayCoord coords, ArrayXd ffs, double tau,
        SincCosFunction& sf, double qmin=0., double qmax=0.5, unsigned num=101){
    //V(qi,qj) = E^2(qi) E^2(qj) sum_k fk(0)^2
    //             x sum_l fl(0) sum_{n!=l} fn(0) Vij(dkl,dkn)
    //Vij(dkl,dkn) = tau^2/2*qi*qj*sigma(qi*dkl)*sigma(qj*dkn)*rho(dkl,dkn)
    //sigma(x) = (sinc(x)-cos(x))/x
    //rho(dkl,dkn) = (2*dkl.dkn + 3*tau^2)/(4*dkl*dkn)
    ArrayXXd ccov = ArrayXXd::Zero(num,num);
    for (unsigned k=0; k<ffs.size()-1; ++k){
        ArrayXd dists = get_distances(coords.row(k), coords);
        for (unsigned l=0; l<dists.rows()-1; ++l){
            if (l==k) continue;
            double dkl = dists(l);
            double fkl = square(ffs(k))*ffs(l);
            for (unsigned n=l+1; n<dists.rows(); ++n){
                if (n==k) continue;
                double dkn = dists(n);
                double fkln = fkl*ffs(n);
                double rho = get_rho(coords.row(k),coords.row(l),coords.row(n),
                                    tau);
                for (unsigned nqi=0; nqi<num; ++nqi){
                    double qi = qmin + (qmax-qmin)*nqi/(num-1);
                    double sil = sf.sico(qi*dkl);
                    double sin = sf.sico(qi*dkn);
                    for (unsigned nqj=nqi; nqj<num; ++nqj){
                        double qj = qmin + (qmax-qmin)*nqj/(num-1);
                        double sjl = sf.sico(qj*dkl);
                        double sjn = sf.sico(qj*dkn);
                        ccov(nqi,nqj) += fkln*rho*qi*qj*(sil*sjn+sjl*sin);
                    } //qj
                } //qi
            } //n
        } //l
    } //k
    //
    ArrayXd factor = (-ArrayXd::LinSpaced(num,qmin,qmax).square()*0.23).exp();
    ccov.colwise() *= factor;
    ccov.rowwise() *= factor.transpose();
    //factor 2 for the half-sum, compensated by sigma mix
    return MatrixXd((0.5*(tau*tau)*ccov).matrix(
            ).selfadjointView<Eigen::Upper>()).array();
}

ArrayXXd compute_crosscovariance_approx(ArrayCoord coords, ArrayXd ffs,
        double tau, SincCosFunction& sf,
        double qmin=0., double qmax=0.5, unsigned num=101){
    //V(qi,qj) = E^2(qi) E^2(qj) sum_k fk(0)^2
    //             x sum_l fl(0) sum_{n!=l} fn(0) Vij(dkl,dkn)
    //Vij(dkl,dkn) = tau^2/2*qi*qj*sigma(qi*dkl)*sigma(qj*dkn)*rho(dkl,dkn)
    //sigma(x) = (sinc(x)-cos(x))/x
    //rho(dkl,dkn) = 3*tau^2/(4*dkl*dkn)
    ArrayXXd ccov = ArrayXXd::Zero(num,num);
    for (unsigned k=0; k<ffs.size()-1; ++k){
        ArrayXd dists = get_distances(coords.row(k), coords);
        for (unsigned l=0; l<dists.rows()-1; ++l){
            if (l==k) continue;
            double dkl = dists(l);
            double fkl = square(ffs(k))*ffs(l);
            for (unsigned n=l+1; n<dists.rows(); ++n){
                if (n==k) continue;
                double dkn = dists(n);
                double fkln = fkl*ffs(n);
                double rho = 3*tau*tau/(4*dists(l)*dists(n));
                for (unsigned nqi=0; nqi<num; ++nqi){
                    double qi = qmin + (qmax-qmin)*nqi/(num-1);
                    double sil = sf.sico(qi*dkl);
                    double sin = sf.sico(qi*dkn);
                    for (unsigned nqj=nqi; nqj<num; ++nqj){
                        double qj = qmin + (qmax-qmin)*nqj/(num-1);
                        double sjl = sf.sico(qj*dkl);
                        double sjn = sf.sico(qj*dkn);
                        ccov(nqi,nqj) += fkln*rho*qi*qj*(sil*sjn+sjl*sin);
                    } //qj
                } //qi
            } //n
        } //l
    } //k
    //
    ArrayXd factor = (-ArrayXd::LinSpaced(num,qmin,qmax).square()*0.23).exp();
    ccov.colwise() *= factor;
    ccov.rowwise() *= factor.transpose();
    //factor 2 for the half-sum, compensated by sigma mix
    return MatrixXd((0.5*(tau*tau)*ccov).matrix(
            ).selfadjointView<Eigen::Upper>()).array();
}

ArrayXXd compute_covariance_numerical(ArrayCoord coords, ArrayXd ffs, double
        tau, SincFunction& sinc, double qmin=0., double qmax=0.5,
        unsigned num=101, unsigned numloops=100){
    //compute perturbed profiles
    ArrayXXd profiles(num,numloops);
    for (unsigned n=0; n<numloops; ++n){
        ArrayCoord newcoords = perturb_particles(coords,tau);
        profiles.col(n) = compute_profile(newcoords, ffs, sinc,
            qmin, qmax, num);
    }
    //compute covariance matrix
    ArrayXd meanprof(compute_profile(coords, ffs, sinc, qmin, qmax, num));
    MatrixXd residuals(profiles.colwise()-meanprof);
    ArrayXXd covmat((residuals*residuals.transpose())/double(numloops));
    return covmat;
}

ArrayXXd make_relative(ArrayXXd mat, ArrayXd vec){
    return (mat.rowwise()/vec.transpose()).colwise()/vec;
}

//used in get_intensity_contrib_at_q
bool compfunction (std::vector<double> i, std::vector<double> j){
        return i[3] > j[3];
    }


#endif  /* IMPISD_PROFILE_VARIANCE_H */
