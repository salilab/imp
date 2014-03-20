/**
 *  \file IMP/score_functor/OrientedSoap.h
 *  \brief Score a particle pair using an orientation-dependent SOAP potential.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_ORIENTED_SOAP_H
#define IMPSCORE_FUNCTOR_ORIENTED_SOAP_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/constants.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include "internal/soap_hdf5.h"
#include "internal/soap_helpers.h"
#include "Score.h"

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Orientation-dependent SOAP score.
/** Orientation-dependent SOAP scores include those that score loops
    (SOAP-Loop), protein-peptide interfaces (SOAP-Peptide) and proteins
    (SOAP-Protein). The library files themselves, such as soap_loop.hdf5 or
    soap_protein_od.hdf5, are rather large (~1.5GB) and so are not included
    here. They can be downloaded separately from http://salilab.org/SOAP/.
 */
class OrientedSoap : public Score {
  double maxrange_;
  std::string library_;
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
  internal::SoapPotential potential_;
  internal::SoapDoublets doublets_;

  typedef std::vector<internal::SoapModelDoublet> DList;

  void read_library(std::string library) {
    internal::Hdf5File file_id(library);
    doublets_.read(file_id);
    potential_.read(file_id, doublets_);
    maxrange_ = potential_.get_max_range();
  }

  // Return the score for the interaction between a pair of doublets
  double score_doublets(atom::Atom a1, atom::Atom a2, int distbin,
                        const internal::SoapModelDoublet &d1,
                        const internal::SoapModelDoublet &d2) const {
    double ang1 =
        core::internal::angle(a1, a2, d2.atom, nullptr, nullptr, nullptr) *
        180.0 / PI;
    double ang2 =
        core::internal::angle(d1.atom, a1, a2, nullptr, nullptr, nullptr) *
        180.0 / PI;
    double dih = core::internal::dihedral(d1.atom, a1, a2, d2.atom, nullptr,
                                          nullptr, nullptr, nullptr) *
                 180.0 / PI;
    return potential_.get_value(distbin, ang1, ang2, dih, d1.doublet_class,
                                d2.doublet_class);
  }
#endif  // IMP_SCORE_FUNCTOR_USE_HDF5

 public:
  //! Constructor.
  /** \param[in] library The HDF5 file containing the SOAP library.
   */
  OrientedSoap(std::string library) : library_(library) {
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    read_library(library);
#else
    maxrange_ = 0.;
    IMP_THROW("Must configure IMP with HDF5 to use this class", ValueException);
#endif
  }
  template <unsigned int D>
  double get_score(kernel::Model *m,
                   const base::Array<D, kernel::ParticleIndex> &pis,
                   double distance) const {
    double score = 0;
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    int distbin =
        potential_.get_index(internal::SoapPotential::DISTANCE, distance);
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
#endif  // IMP_SCORE_FUNCTOR_USE_HDF5
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

  double get_distance_threshold() const { return maxrange_; }

  template <unsigned int D>
  bool get_is_trivially_zero(kernel::Model *,
                             const base::Array<D, kernel::ParticleIndex> &,
                             double squared_distance) const {
    return squared_distance > algebra::get_squared(maxrange_);
  }

  kernel::ModelObjectsTemp get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const {
    kernel::ModelObjectsTemp ret = IMP::kernel::get_particles(m, pis);
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    // We touch the Residue for each Atom, so make sure that's in the list
    // Note that we potentially touch other atoms in the Residue that aren't
    // in the input list; we should really expand the list of Atoms to include
    // *all* atoms in any Residue we touch.
    for (unsigned int i = 0; i < pis.size(); ++i) {
      if (atom::Atom::get_is_setup(m, pis[i])) {
        atom::Residue r = atom::get_residue(atom::Atom(m, pis[i]));
        ret.push_back(r);
      }
    }
#endif
    return ret;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_ORIENTED_SOAP_H */
