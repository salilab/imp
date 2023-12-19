/**
 *  \file IMP/isd/CrossLinkData.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_CROSS_LINK_DATA_H
#define IMPISD_CROSS_LINK_DATA_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <cmath>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

IMPISD_BEGIN_NAMESPACE

//! CrossLinkData
/**
 */

class IMPISDEXPORT CrossLinkData : public Object {
  double lexp_;  // length of the linker
  Floats dist_grid_;
  Floats sigma_grid_;
  Floats omega_grid_;
  Floats pot_x_grid_;
  Floats pot_value_grid_;
  int prior_type_;
  bool bias_;
  std::vector<Floats> grid_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Object>(this), lexp_, dist_grid_, sigma_grid_,
       omega_grid_, pot_x_grid_, pot_value_grid_, prior_type_, bias_, grid_);
  }

  double get_unbiased_element(double dist, double sigmai) const;
  double get_biased_element(double dist, double sigmai) const;

 public:
  CrossLinkData(Floats dist_grid, Floats omega_grid, Floats sigma_grid,
                double lexp_, double don = std::numeric_limits<double>::max(),
                double doff = std::numeric_limits<double>::max(),
                int prior_type = 0);
  CrossLinkData(Floats dist_grid, Floats omega_grid, Floats sigma_grid,
                Floats pot_x_grid, Floats pot_value_grid,
                double don = std::numeric_limits<double>::max(),
                double doff = std::numeric_limits<double>::max(),
                int prior_type = 0);
  CrossLinkData() : Object("") {}
  int get_closest(std::vector<double> const& vec, double value) const;
  Floats get_omegas(double sigma, Floats dists) const;
  double get_omega_prior(double omega, double omega0) const;
  Floats get_nonmarginal_elements(double sigmai, Floats dists) const;
  Floats get_marginal_elements(double sigma, Floats dists) const;
  double get_marginal_maximum(double sigma) const;
  double get_marginal_maximum() const;

  IMP_OBJECT_METHODS(CrossLinkData);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CROSS_LINK_DATA_H */
