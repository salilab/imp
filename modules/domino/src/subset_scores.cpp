/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/subset_scores.h>

IMPDOMINO_BEGIN_NAMESPACE
RestraintCache::RestraintCache(const ParticlesTemp &order,
                               ParticleStatesTable *pst,
                               unsigned int size):
  Object("RestraintCache%1%"),
  cache_(Generator(pst),
         size,
         ApproximatelyEqual()),
  order_(order) {

}
void RestraintCache::add_restraint_internal(Restraint *r,
                                            RestraintSets sets,
                                            Floats set_weights,
                                            Subsets set_subsets) {
  Pointer<Restraint> decomp= r->create_decomposition();
  RestraintSet *rs= dynamic_cast<RestraintSet*>(decomp.get());
  Subset cur_subset(r->get_input_particles());
  if (rs) {
    if (rs->get_maximum_score() < std::numeric_limits<double>::max()) {
      known_restraints_[r]=cur_subset;
      sets.push_back(rs);
      set_weights.push_back(set_weights.back()*rs->get_weight());
      set_subsets.push_back(cur_subset);
    }
    for (RestraintSet::RestraintIterator it= rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      add_restraint_internal(*it,
                             sets, set_weights, set_subsets);
    }
  } else {
    if (r->get_maximum_score() < std::numeric_limits<double>::max()
        || !sets.empty()) {
      cache_.access_generator().add_restraint(r, cur_subset);
      known_restraints_[r]=cur_subset;
      for (unsigned int i=0; i< sets.size(); ++i) {
        cache_.access_generator().add_to_set(sets[i],
                                         r,
                                         set_weights[i]*r->get_weight(),
                                         Slice(set_subsets[i],
                                               cur_subset));
      }
    }
  }
}

void RestraintCache::add_restraints(const RestraintsTemp &rs) {
  for (unsigned int i=0; i< rs.size(); ++i) {
    add_restraint_internal(rs[i],
                           RestraintSets(),
                           Floats(),
                           Subsets());
  }
}

RestraintsTemp RestraintCache::get_restraints(const Subset&s,
                                              const Subsets&exclusions) const {
  RestraintsTemp ret;
  for (KnownRestraints::const_iterator it= known_restraints_.begin();
       it != known_restraints_.end(); ++it) {
    if (s.get_contains(it->second)) {
      bool excluded=false;
      for (unsigned int i=0; i< exclusions.size(); ++i) {
        if (exclusions[i].get_contains(it->second)) {
          excluded=true;
          break;
        }
      }
      if (!excluded) {
        ret.push_back(it->first);
      }
    }
  }
  return ret;
}

Slice RestraintCache::get_slice(Restraint *r, const Subset& s) const {
  Subset rs= known_restraints_.find(r)->second;
  return Slice(s, rs);
}

IMPDOMINO_END_NAMESPACE
