/**
 *  \file IMP/score_functor/SoapLoop.h
 *  \brief Score a particle pair using the SOAP-LOOP potential.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SOAP_LOOP_H
#define IMPSCORE_FUNCTOR_SOAP_LOOP_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/constants.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include "internal/soap_hdf5.h"
#include "internal/soap_loop_helpers.h"
#include "Score.h"

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! SOAP-LOOP score.
/** The library file itself, soap_loop.hdf5, is rather large (~500MB)
    and so is not included here. It can be downloaded separately from
    http://salilab.org/SOAP/.
 */
class SoapLoop : public Score {
  double maxrange_;
  internal::SoapLoopPotential potential_;
  internal::SoapLoopDoublets doublets_;

  typedef std::vector<internal::SoapModelDoublet> DList;

  void read_library(std::string library) {
    internal::Hdf5File file_id(library);
    doublets_.read(file_id);
    potential_.read(file_id, doublets_);
    maxrange_ = potential_.get_max_range();
  }

  // Return the score for the interaction between a pair of doublets
  double score_doublets(atom::Atom a1, atom::Atom a2,
                        int distbin, const internal::SoapModelDoublet &d1,
                        const internal::SoapModelDoublet &d2) const {
    double ang1 = core::internal::angle(a1, a2, d2.atom,
                                        nullptr, nullptr,
                                        nullptr) * 180.0 / PI;
    double ang2 = core::internal::angle(d1.atom, a1, a2,
                                        nullptr, nullptr,
                                        nullptr) * 180.0 / PI;
    double dih = core::internal::dihedral(d1.atom, a1, a2, d2.atom,
                                          nullptr, nullptr, nullptr,
                                          nullptr) * 180.0 / PI;
    return potential_.get_value(distbin, ang1, ang2, dih, d1.doublet_class,
                                d2.doublet_class);
  }

 public:
  //! Constructor.
  /** \param[in] library The HDF5 file containing the SOAP-LOOP library.
   */
  SoapLoop(std::string library) {
    read_library(library);
  }
  template <unsigned int D>
  double get_score(kernel::Model *m,
                   const base::Array<D, kernel::ParticleIndex> &pis,
                   double distance) const {
    double score = 0;
    int distbin = potential_.get_index(internal::SoapLoopPotential::DISTANCE,
                                       distance);
    if (distbin >= 0) {
      atom::Atom a1(m, pis[0]);
      atom::Atom a2(m, pis[1]);
      // Find the other atoms (if any) in the two doublet(s) that (a1,a2)
      // are members of
      DList doublets1 = doublets_.get_for_atom(a1);
      if (doublets1.size() > 0) {
        DList doublets2 = doublets_.get_for_atom(a2);
        for (DList::const_iterator dit1 = doublets1.begin();
             dit1 != doublets1.end(); ++dit1) {
          for (DList::const_iterator dit2 = doublets2.begin();
               dit2 != doublets2.end(); ++dit2) {
            score += score_doublets(a1, a2, distbin, *dit1, *dit2);
          }
        }
      }
    }
    return score;
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      kernel::Model *m, const base::Array<D, kernel::ParticleIndex> &pis,
      double distance) const {
    // No derivatives are available
    return DerivativePair(get_score(m, pis, distance), 0.);
  }
  template <unsigned int D>
  double get_maximum_range(
      kernel::Model *, const base::Array<D, kernel::ParticleIndex> &) const {
    return maxrange_;
  }
  template <unsigned int D>
  bool get_is_trivially_zero(kernel::Model *,
                             const base::Array<D, kernel::ParticleIndex> &,
                             double squared_distance) const {
    return squared_distance > algebra::get_squared(maxrange_);
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SOAP_LOOP_H */
