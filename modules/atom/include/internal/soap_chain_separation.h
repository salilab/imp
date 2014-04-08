/**
 * \file soap_chain_separation.h
 * \brief SOAP chain separation filter
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SOAP_CHAIN_SEPARATION_H
#define IMPATOM_SOAP_CHAIN_SEPARATION_H

#include <IMP/atom/atom_config.h>
#include <IMP/score_functor/score_functor_config.h>

#ifdef IMP_SCORE_FUNCTOR_USE_HDF5

#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/score_functor/internal/soap_hdf5.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

// Calculate chain separation in the same manner that was used to generate
// the SOAP potential
class SoapChainSeparation {
  bool require_same_chain_, require_different_chains_;

 public:
  SoapChainSeparation() {}

  bool enabled() const {
    return require_same_chain_ || require_different_chains_;
  }

  bool get_separation_ok(kernel::Model *m,
                         const kernel::ParticleIndexPair &p) const {
    if (!enabled()) {
      return true;
    }
    Atom a1(m, p[0]);
    Atom a2(m, p[1]);
    Residue r1 = get_residue(a1);
    Residue r2 = get_residue(a2);
    Hierarchy p1 = r1.get_parent();
    Hierarchy p2 = r2.get_parent();
    bool same_chain = (p1 == p2);
    return (require_same_chain_ && same_chain) ||
           (require_different_chains_ && !same_chain);
  }

  void read(score_functor::internal::Hdf5File &file_id) {
    score_functor::internal::Hdf5Group group(file_id.get(), "scan");
    std::vector<int> c = group.read_int_attribute_vector("chain_span_range", 4);
    require_same_chain_ = (c[3] == 0);
    require_different_chains_ = (c[2] != 0);
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif  // IMP_SCORE_FUNCTOR_USE_HDF5

#endif /* IMPATOM_SOAP_CHAIN_SEPARATION_H */
