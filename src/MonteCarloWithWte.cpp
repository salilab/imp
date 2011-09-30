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
  dx_    = sigma / 4.0;
  nbin_  = floor((emax-emin)/dx_)+1;
  bias_.reset(new double[nbin_]);
  for(unsigned int i=0;i<nbin_;++i) {bias_[i]=0.0;}
  }

double MonteCarloWithWte::get_bias(double score) const
{
  int index=floor((score-min_)/dx_);
  index=std::max(0,std::min(nbin_-1,index));
  return bias_[index];
}

void MonteCarloWithWte::update_bias(double score)
{
  if(score < min_ || score >= max_) return;
  double vbias=get_bias(score);
  double ww=w0_*exp(-vbias/(get_kt()*(gamma_-1.0)));
  int i0=floor((score-4.0*sigma_-min_)/dx_);
  int i1=floor((score+4.0*sigma_-min_)/dx_)+1;
  for (int i=std::max(0,i0);i<=std::min(i1,nbin_-1);++i){
   double xx=min_ + ((double) i)*dx_;
   double dp=(xx-score)/sigma_;
   bias_[i] += ww*exp(-0.5*dp*dp);
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
