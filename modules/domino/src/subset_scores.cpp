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
RestraintCache::RestraintCache(ParticleStatesTable *pst,
                               unsigned int size):
  Object("RestraintCache%1%"),
  cache_(Generator(pst),
         size,
         ApproximatelyEqual()) {

}
void RestraintCache::add_restraint_internal(Restraint *rin,
                                            RestraintSet *parent,
                                            double parent_max,
                                            Subset parent_subset) {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Processing " << Showable(rin) << " with "
          << parent_max << std::endl);
  Pointer<Restraint> r= rin->create_decomposition();
  r->set_was_used(true);
  RestraintSet *rs= dynamic_cast<RestraintSet*>(r.get());
  Subset cur_subset(r->get_input_particles());
  double cur_max= r->get_maximum_score();
  if (parent) {
    cur_max=std::min(parent_max/r->get_weight(),
                     cur_max);
  }
  if (rs) {
    IMP_LOG(TERSE, "Parsing restraint set " << Showable(rs) << std::endl);
    if (cur_max < std::numeric_limits<double>::max()) {
      known_restraints_[r]=cur_subset;
      for (RestraintSet::RestraintIterator it= rs->restraints_begin();
           it != rs->restraints_end(); ++it) {
        add_restraint_internal(*it, rs, cur_max, cur_subset);
      }
    } else {
      for (RestraintSet::RestraintIterator it= rs->restraints_begin();
           it != rs->restraints_end(); ++it) {
        add_restraint_internal(*it,
                               nullptr, std::numeric_limits<double>::max(),
                               Subset());
      }
    }
  } else {
    if (cur_max < std::numeric_limits<double>::max()) {
      IMP_LOG(TERSE, "Adding restraint " << Showable(r)
              << " with max " << cur_max << " and subset " << cur_subset
              << std::endl);
      cache_.access_generator().add_restraint(r, cur_subset, cur_max);
      known_restraints_[r]=cur_subset;
      if (parent) {
        IMP_LOG(TERSE, "Adding restraint " << Showable(r)
                << " to set " << Showable(parent) << std::endl);
        cache_.access_generator().add_to_set(parent,
                                             r,
                                             Slice(parent_subset,
                                                   cur_subset));
      }
    }
  }
}

void RestraintCache::add_restraints(const RestraintsTemp &rs) {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< rs.size(); ++i) {
    add_restraint_internal(rs[i],
                           nullptr,
                           std::numeric_limits<double>::max(),
                           Subset());
  }
}

RestraintsTemp RestraintCache::get_restraints(const Subset&s,
                                              const Subsets&exclusions) const {
  IMP_OBJECT_LOG;
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

void RestraintCache::Generator
::show_restraint_information(std::ostream &out) const {
  for (RMap::const_iterator it= rmap_.begin(); it != rmap_.end(); ++it) {
    out << it->first->get_name() << ": " << it->second << std::endl;
  }
  for (SMap::const_iterator it= sets_.begin(); it != sets_.end(); ++it) {
    out << it->first->get_name() << ": " << it->second << std::endl;
  }
}

void RestraintCache::show_restraint_information(std::ostream &out) const {
  cache_.get_generator().show_restraint_information(out);
}

IMPDOMINO_END_NAMESPACE
