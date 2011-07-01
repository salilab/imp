/**
 *  \file MonteCarloWithWte.h
 *  \brief An exotic version of MonteCarlo
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H
#define IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H

#include "membrane_config.h"

#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! MonteCarlo in the Well-Tempered Ensemble
class IMPMEMBRANEEXPORT MonteCarloWithWte: public core::MonteCarlo
{
  double  min_, max_, sigma_, gamma_, dx_, w0_;
  double* bias_;
  int     nbin_;
  void    update_bias(double score);
  double  do_evaluate() {
    double score=evaluate(false);
    return score+get_bias(score);
  }
public:
  MonteCarloWithWte(Model *m, double emin,  double emax,
                              double sigma, double gamma,
                              double w0);

  double get_bias(double score);

  IMP_MONTE_CARLO(MonteCarloWithWte);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H */
