/**
 *  \file IMP/isd/CysteineCrossLinkData.h \brief Normal distribution of Function
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_CYSTEINE_CROSS_LINK_DATA_H
#define IMPISD_CYSTEINE_CROSS_LINK_DATA_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <math.h>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

IMPISD_BEGIN_NAMESPACE

//! CysteineCrossLinkData
/**
 */

class IMPISDEXPORT CysteineCrossLinkData : public Object {
  Floats omega0_grid_;  // the typical uncertainty, it goes in the prior
  Floats fmod_grid_;
  double fexp_;
  int prior_type_;
  Floats omega_grid_;
  std::vector<Floats> grid_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Object>(this), omega0_grid_, fmod_grid_, fexp_,
       prior_type_, omega_grid_, grid_);
  }

  double get_element(double fexp, double fmod, double omega) const;
  double get_omega_prior(double omega, double omega0) const;

 public:
  CysteineCrossLinkData(double fexp, Floats fmod_grid, Floats omega_grid,
                        Floats omega0_grid, int prior_type = 3);
  CysteineCrossLinkData() : Object("") {}

  int get_closest(std::vector<double> const& vec, double value) const;
  Floats get_omegas(Floats fmods, double omega0) const;
  Floats get_nonmarginal_elements(double fexp, Floats fmods,
                                  double omega) const;
  Floats get_marginal_elements(Floats fmods, double omega0) const;

  double get_omega(double fmod, double omega0) const;
  double get_standard_deviation(double fmod, double omega0) const;
  double get_nonmarginal_element(double fexp, double fmod, double omega) const;
  double get_marginal_element(double fmod, double omega0) const;

  IMP_OBJECT_METHODS(CysteineCrossLinkData);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CYSTEINE_CROSS_LINK_DATA_H */
