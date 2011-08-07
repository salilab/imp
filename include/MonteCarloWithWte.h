/**
 *  \file MonteCarloWithWte.h
 *  \brief An exotic version of MonteCarlo
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H
#define IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H

#include "membrane_config.h"

#include <boost/scoped_array.hpp>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! MonteCarlo in the Well-Tempered Ensemble
class IMPMEMBRANEEXPORT MonteCarloWithWte: public core::MonteCarlo
{
  double  min_, max_, sigma_, gamma_, dx_, w0_;
  boost::scoped_array<double> bias_;
  int     nbin_;
  void    update_bias(double score);
  double  spline(double score, int index) const;

private:
  double do_evaluate(const ParticlesTemp &moved) const;

public:
  MonteCarloWithWte(Model *m, double emin,  double emax,
                              double sigma, double gamma,
                              double w0);

  double get_bias(double score) const;

#ifndef SWIG
  double* get_bias_buffer() const {
   return bias_.get();
  }
#endif

  int get_nbin() const {
   return nbin_;
  }

  void set_w0(double w0) {w0_=w0;}

  void set_bias(const Floats &bias) {
   IMP_USAGE_CHECK(bias.size() == 2*nbin_, "Don't match");
   std::copy(bias.begin(), bias.end(), bias_.get());
  }

  IMP_MONTE_CARLO(MonteCarloWithWte);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H */
