/**
 *  \file MonteCarloWithWte.h
 *  \brief An exotic version of MonteCarlo
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H
#define IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H

#include "membrane_config.h"

#include <IMP/Restraint.h>
#include <boost/scoped_array.hpp>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! MonteCarlo in the Well-Tempered Ensemble
class IMPMEMBRANEEXPORT MonteCarloWithWte: public core::MonteCarlo
{
  double  min_;
  double  max_;
  double  sigma_;
  double  gamma_;
  double  dx_;
  double  w0_;
  boost::scoped_array<double> bias_;
  int     nbin_;
  bool    full_;
  Pointer<RestraintSet> rset_;

  void do_initialize(double emin,  double emax, double sigma,
                     double gamma, double w0);

  void    update_bias(double score);
  double  do_evaluate(const ParticleIndexes &moved) const;
  double  get_spline(double score) const;

public:
  MonteCarloWithWte(Model *m, double emin,  double emax,
                    double sigma, double gamma, double w0);

  MonteCarloWithWte(Model *m, double emin,  double emax,
                    double sigma, double gamma, double w0,
                    RestraintSet *rset);

  double get_bias(double score) const;

#ifndef SWIG
  double* get_bias_buffer() const {
   return bias_.get();
  }
#endif

  Floats get_bias_asfloats() const {
    Floats  buffer(bias_.get(), bias_.get()+nbin_);
   return buffer;
  }

  int get_nbin() const {
   return nbin_;
  }

  void set_w0(double w0) {w0_=w0;}

  void set_bias(const Floats &bias) {
    IMP_USAGE_CHECK(bias.size() == static_cast<unsigned int>(nbin_),
                    "Don't match");
   std::copy(bias.begin(), bias.end(), bias_.get());
  }

  IMP_MONTE_CARLO(MonteCarloWithWte);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H */
