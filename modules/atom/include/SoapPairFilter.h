/**
 *  \file IMP/atom/SoapPairFilter.h
 *  \brief PairFilter for SOAP
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SOAP_PAIR_FILTER_H
#define IMPATOM_SOAP_PAIR_FILTER_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/score_functor_config.h>
#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
#include "internal/soap_chain_separation.h"
#include "internal/soap_bond_separation.h"

IMPATOM_BEGIN_NAMESPACE

//! Filter atom pairs for SOAP.
/** Most SOAP potentials are trained using only a subset of particle pairs
    (e.g. only pairs where the atoms are in different chains). This filter
    can be used to exclude atom pairs not in that subset.
 */
class SoapPairFilter : public PairPredicate {
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
  internal::SoapBondSeparation bondsep_;
  internal::SoapChainSeparation chainsep_;
#endif
 public:
  SoapPairFilter(std::string library) {
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    score_functor::internal::Hdf5File file_id(library);
    bondsep_.read(file_id);
    chainsep_.read(file_id);
#else
    IMP_UNUSED(library);
    IMP_THROW("Must configure IMP with HDF5 to use this class", ValueException);
#endif
  }

  virtual int get_value_index(kernel::Model *m,
                              const kernel::ParticleIndexPair &p) const
      IMP_OVERRIDE {
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    return !chainsep_.get_separation_ok(m, p) ||
           !bondsep_.get_separation_ok(m, p);
#else
    return true;
#endif
  }

  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE {
    kernel::ModelObjectsTemp ret = IMP::kernel::get_particles(m, pis);
#ifdef IMP_SCORE_FUNCTOR_USE_HDF5
    if (chainsep_.enabled() || bondsep_.enabled()) {
      // We touch the two parents (residue and chain) of each atom to
      // determine chain separation; for bond separation we potentially touch
      // other residues and atoms in the same chain as the inputs
      // (to detect dissulfides) but this isn't currently handled here.
      for (unsigned int i = 0; i < pis.size(); ++i) {
        if (Atom::get_is_setup(m, pis[i])) {
          Residue r = get_residue(Atom(m, pis[i]));
          Hierarchy c = r.get_parent();
          ret.push_back(r);
          ret.push_back(c);
        }
      }
    }
#endif
    return ret;
  }

  IMP_PAIR_PREDICATE_METHODS(SoapPairFilter);
  IMP_OBJECT_METHODS(SoapPairFilter);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SOAP_PAIR_FILTER_H */
