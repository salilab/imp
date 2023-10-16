/**
 *  \file IMP/isd/FretData.h \brief Auxiliary class useful for FRET_R restraint
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FRET_DATA_H
#define IMPISD_FRET_DATA_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <cmath>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

IMPISD_BEGIN_NAMESPACE

//! Auxiliary class for FRET_R restraint
/**
 */

class IMPISDEXPORT FretData : public Object {
  Floats d_term_;
  Floats d_center_;
  Floats s_grid_;
  std::vector<unsigned> nbin_;
  unsigned dimension_;
  Floats grid_;
  Floats norm_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Object>(this), d_term_, d_center_, s_grid_, nbin_,
       dimension_, grid_, norm_);
  }

  unsigned get_index(unsigned indices[3]) const;
  void init_grids(const Floats& d_grid_int, Float R0, Float Rmin, Float Rmax,
                  bool do_limit);

 public:
  FretData(Floats d_term, Floats d_center, Floats d_int, Floats s_grid,
           Float R0, Float Rmin, Float Rmax, bool do_limit = true);
  FretData() : Object("") {}

  int get_closest(std::vector<double> const& vec, double value) const;

  Float get_probability(Float distn, Float dist, Float sigma) const;

  Float get_kernel(Float dist, Float R0) const;

  FloatPair get_marginal_element(Float d_term, Float d_center,
                                 Float sigma) const;

  IMP_OBJECT_METHODS(FretData);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_FRET_DATA_H */
