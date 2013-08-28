/**
 *  \file isd/CrossLinkData.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_CROSS_LINK_DATA_H
#define IMPISD_CROSS_LINK_DATA_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

//! CrossLinkData
/**
 */

class IMPISDEXPORT CrossLinkData : public base::Object
{
    double lexp_;//length of the linker
    Floats dist_grid_;
    Floats sigma_grid_;
    Floats omega_grid_;
    Floats pot_x_grid_;
    Floats pot_value_grid_;
    int prior_type_;
    bool bias_;
    std::vector<Floats> grid_;
  double get_unbiased_element(double dist, double sigmai) const;
  double get_biased_element(double dist, double sigmai) const;


 public:

  CrossLinkData(Floats dist_grid, Floats omega_grid,
                Floats sigma_grid, double lexp_,
                double don=std::numeric_limits<double>::max(),
                double doff=std::numeric_limits<double>::max(),
                int prior_type=0);
  CrossLinkData(Floats dist_grid, Floats omega_grid, Floats sigma_grid,
                Floats pot_x_grid, Floats pot_value_grid,
                double don=std::numeric_limits<double>::max(),
                double doff=std::numeric_limits<double>::max(),
                int prior_type=0);
  int get_closest(std::vector<double> const& vec, double value) const;
  Floats get_omegas(double sigma, Floats dists) const;
  double get_omega_prior(double omega, double omega0) const;
  Floats get_nonmarginal_elements(double sigmai, Floats dists) const;
  Floats get_marginal_elements(double sigma, Floats dists) const ;
  double get_marginal_maximum(double sigma) const;
  double get_marginal_maximum() const;

  IMP_OBJECT_METHODS(CrossLinkData);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_CROSS_LINK_DATA_H */
