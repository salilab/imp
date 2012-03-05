/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/subset_scores.h>
#include <IMP/dependency_graph.h>

IMPDOMINO_BEGIN_NAMESPACE
RestraintCache::RestraintCache(ParticleStatesTable *pst,
                               unsigned int size):
  Object("RestraintCache%1%"),
  cache_(Generator(pst),
         size,
         ApproximatelyEqual()) {

}
void RestraintCache::add_restraint_set_internal(RestraintSet *rs,
                                                const Subset &cur_subset,
                                                double cur_max,
                                                const DepMap &dependencies) {
  IMP_LOG(TERSE, "Parsing restraint set " << Showable(rs) << std::endl);
  if (cur_max < std::numeric_limits<double>::max()) {
    for (RestraintSet::RestraintIterator it= rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      add_restraint_internal(*it, rs, cur_max, cur_subset, dependencies);
    }
  } else {
    for (RestraintSet::RestraintIterator it= rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      add_restraint_internal(*it,
                             nullptr, std::numeric_limits<double>::max(),
                             Subset(), dependencies);
    }
  }
}

void RestraintCache::add_restraint_set_child_internal(Restraint *r,
                                                      const Subset &cur_subset,
                                                      RestraintSet *parent,
                                                      Subset parent_subset) {
  if (!parent) return;
  IMP_LOG(TERSE, "Adding restraint " << Showable(r)
          << " to set " << Showable(parent) << std::endl);
  cache_.access_generator().add_to_set(parent,
                                       r,
                                       Slice(parent_subset,
                                             cur_subset));
}
Subset RestraintCache::get_subset(Restraint *r,
                                  const DepMap &dependencies) const {
  ParticlesTemp ups=r->get_input_particles();
  std::sort(ups.begin(), ups.end());
  ups.erase(std::unique(ups.begin(), ups.end()), ups.end());
  ParticlesTemp outps;
  for (unsigned int i=0; i< ups.size(); ++i) {
    DepMap::const_iterator it= dependencies.find(ups[i]);
    if (it != dependencies.end()) {
      outps= outps+ it->second;
    }
  }
  std::sort(outps.begin(), outps.end());
  outps.erase(std::unique(outps.begin(), outps.end()), outps.end());
  return Subset(outps);
}


void RestraintCache::add_restraint_internal(Restraint *r,
                                            RestraintSet *parent,
                                            double parent_max,
                                            Subset parent_subset,
                                            const DepMap &dependencies) {
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Processing " << Showable(r) << " with "
          << parent_max << std::endl);
  r->set_was_used(true);
  // fix using PST
  Subset cur_subset= get_subset(r, dependencies);
  double cur_max= r->get_maximum_score();
  if (parent) {
    cur_max=std::min(parent_max/r->get_weight(),
                     cur_max);
  }

  if (cur_max < std::numeric_limits<double>::max()) {
    IMP_LOG(TERSE, "Adding restraint " << Showable(r)
            << " with max " << cur_max << " and subset " << cur_subset
            << std::endl);
    known_restraints_[r]=cur_subset;
  }
  add_restraint_set_child_internal(r,
                                   cur_subset,
                                   parent,
                                   parent_subset);
  RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
  if (rs) {
    add_restraint_set_internal(rs, cur_subset, cur_max, dependencies);
  } else {
    if (cur_max < std::numeric_limits<double>::max()) {
      cache_.access_generator().add_restraint(r, cur_subset, cur_max);
    }
  }
}

void RestraintCache::add_restraints(const RestraintsTemp &rs) {
  IMP_OBJECT_LOG;
  if (rs.empty()) return;
  Model *m= rs[0]->get_model();
  DependencyGraph dg
      = get_dependency_graph(ScoreStatesTemp(m->score_states_begin(),
                                             m->score_states_end()),
                             // we just care about interactions between
                             // particles
                             RestraintsTemp());
  ParticleStatesTable *pst= cache_.get_generator().get_particle_states_table();
  DepMap dependencies;
  ParticlesTemp allps= pst->get_particles();
  for (unsigned int i=0; i< allps.size(); ++i) {
    dependencies[allps[i]]= get_dependent_particles(allps[i], allps, dg);
    IMP_LOG(TERSE, "Particle " << Showable(allps[i])
            << " controlls " << dependencies[allps[i]] << std::endl);
  }

  for (unsigned int i=0; i< rs.size(); ++i) {
    Pointer<Restraint> r= rs[i]->create_decomposition();
    if (r) {
      add_restraint_internal(r,
                             nullptr,
                             std::numeric_limits<double>::max(),
                             Subset(),
                             dependencies);
    }
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







// filtetring==========================================

namespace {
class RestraintCacheSubsetFilter: public SubsetFilter {
  OwnerPointer<RestraintCache> cache_;
  RestraintsTemp rs_;
  Slices slices_;
public:
  RestraintCacheSubsetFilter(RestraintCache *cache,
                             const RestraintsTemp rs,
                             const Subset&s,
                             const Subsets &):
      SubsetFilter("RestraintCacheSubsetFilter%1%"),
      rs_(rs) {
    for (unsigned int i=0; i < rs_.size(); ++i) {
      slices_.push_back(cache->get_slice(rs_[i], s));
    }
  }
  IMP_SUBSET_FILTER(RestraintCacheSubsetFilter);
};
bool RestraintCacheSubsetFilter
::get_is_ok(const Assignment& state) const {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< rs_.size(); ++i) {
    Assignment substate=slices_[i].get_sliced(state);
    double score=cache_->get_score(rs_[i], substate);
    IMP_LOG(VERBOSE, "Score for restraint " << Showable(rs_[i])
            << " with assignment " << substate << " is " << score
            << std::endl);
    if (score >= std::numeric_limits<double>::max()) return false;
  }
  return true;
}

void RestraintCacheSubsetFilter
::do_show(std::ostream &out) const {
  out << "restraints: " << Showable(rs_) << std::endl;
  out << "slices: " << Showable(slices_) << std::endl;
}

}

RestraintCacheSubsetFilterTable
::RestraintCacheSubsetFilterTable(RestraintCache *cache):
  SubsetFilterTable("RestraintCacheSubsetFilterTable%1%"),
  cache_(cache){}

SubsetFilter*
RestraintCacheSubsetFilterTable
::get_subset_filter(const Subset&s,
                    const Subsets &excluded) const {
  RestraintsTemp rs= cache_->get_restraints(s, excluded);
  if (rs.empty()) {
    IMP_LOG(TERSE, "No restraints on subset " << s
            << " with excluded " << excluded << std::endl);
    return nullptr;
  } else {
    return new RestraintCacheSubsetFilter(cache_, rs, s, excluded);
  }
}
double RestraintCacheSubsetFilterTable
::get_strength(const Subset&s,
               const Subsets &excluded) const {
  int n= cache_->get_restraints(s, excluded).size();
  return 1.0-1.0/(n+1.0);
}

void RestraintCacheSubsetFilterTable
::do_show(std::ostream &out) const {
  out << "cache: " << Showable(cache_) << std::endl;
}


IMPDOMINO_END_NAMESPACE
