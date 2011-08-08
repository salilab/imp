/**
 *  \file MonteCarloWithWte.cpp
 *  \brief An exotic version of MonteCarlo
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/MonteCarloWithWte.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

MonteCarloWithWte::MonteCarloWithWte(Model *m, double emin,  double emax,
                                               double sigma, double gamma,
                                               double w0):
  core::MonteCarlo(m) {
  min_   = emin;
  max_   = emax;
  sigma_ = sigma;
  gamma_ = gamma;
  w0_    = w0;
  dx_    = sigma / 3.0;
  nbin_  = floor((emax-emin)/dx_)+1;
  bias_.reset(new double[2*nbin_]);
  for(unsigned int i=0;i<2*nbin_;++i) {bias_[i]=0.0;}
  }

double MonteCarloWithWte::get_bias(double score) const
{
  int index=floor((score-min_)/dx_);
  index=std::max(0,std::min(nbin_-1,index));
  return spline(score,index);
}

double MonteCarloWithWte::spline(double score, int index) const
{
 double where=score-min_-(double)index*dx_;
 const int npoints=2;
 int shift;
 double spline=0.0;
 for(int i=0;i<npoints;++i){
  shift=i%2;
  double X=fabs(where/dx_-(double)shift);
  double X2=X*X;
  double X3=X2*X;
  double yy;
  int indexx=std::min(index+shift,nbin_-1);
  if(fabs(bias_[indexx])<0.0000001){
   yy=0.0;
  }else{
   yy=bias_[indexx+nbin_]/bias_[indexx];
  }
  double ff=(1.0-3.0*X2+2.0*X3)-(double)(shift?-1:1)*yy*(X-2.0*X2+X3)*dx_;
  spline+=bias_[indexx]*ff;
 }
 return spline;
}

void MonteCarloWithWte::update_bias(double score)
{
  if(score < min_ || score > max_) return;
  double vbias=get_bias(score);
  double ww=w0_*exp(-vbias/(get_kt()*(gamma_-1.0)));
  int i0=floor((score-4.0*sigma_-min_)/dx_);
  int i1=floor((score+4.0*sigma_-min_)/dx_)+1;
  for (int i=std::max(0,i0);i<=std::min(i1,nbin_-1);++i){
   double xx=min_ + ((double) i)*dx_;
   double dp=(xx-score)/sigma_;
   double newbias=ww*exp(-0.5*dp*dp);
   bias_[i] += newbias;
   bias_[i+nbin_] += newbias*dp/sigma_;
  }
}

void MonteCarloWithWte::do_step() {
  ParticlesTemp moved=do_move(get_move_probability());
  double energy= evaluate(false);
  bool do_accept=do_accept_or_reject_move(energy+get_bias(energy));
  if(do_accept) update_bias(energy);
}

double MonteCarloWithWte::do_evaluate(const ParticlesTemp &moved) const {
   double score=evaluate(false);
   return score+get_bias(score);
}

void MonteCarloWithWte::do_show(std::ostream &) const {
}

IMPMEMBRANE_END_NAMESPACE
