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

//! This variant of Monte Carlo that relaxes after each move
class IMPMEMBRANEEXPORT MonteCarloWithWte: public core::MonteCarlo
{
  double  min_, max_, sigma_, gamma_, dx_;
  double* bias_;
  unsigned int nbin_;
  double  get_bias(double score);
  void    add_Gaussian(double score);
public:
  MonteCarloWithWte(Model *m, double gmin,  double gmax,
                              double sigma, double gamma);

  double do_optimize(unsigned int max_steps);

  double get_min_energy() const {
    return min_;
  }

  double get_max_energy() const {
    return max_;
  }

  double get_sigma() const {
    return sigma_;
  }

  double get_gamma() const {
    return gamma_;
  }

  IMP_MONTE_CARLO(MonteCarloWithWte);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MONTE_CARLO_WITH_WTE_H */
