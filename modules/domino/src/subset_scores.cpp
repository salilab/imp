/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/subset_scores.h>
#include <IMP/dependency_graph.h>
#include <IMP/domino/Order.h>
#ifdef IMP_DOMINO_USE_IMP_RMF
#include <RMF/HDF5/DataSetD.h>
#endif

IMPDOMINO_BEGIN_NAMESPACE
RestraintCache::RestraintCache(ParticleStatesTable *pst,
                               unsigned int size):
  Object("RestraintCache%1%"),
  cache_(Generator(pst),
         size,
         ApproximatelyEqual()) {
  next_index_=0;

}
void RestraintCache::add_restraint_set_internal(RestraintSet *rs,
                                                unsigned int index,
                                                const Subset &cur_subset,
                                                double cur_max,
                                                const DepMap &dependencies) {
  IMP_LOG_TERSE( "Parsing restraint set " << Showable(rs) << std::endl);
  if (cur_max < std::numeric_limits<double>::max()) {
    for (RestraintSet::RestraintIterator it= rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      add_restraint_internal(*it, index, rs, cur_max, cur_subset, dependencies);
    }
  } else {
    for (RestraintSet::RestraintIterator it= rs->restraints_begin();
         it != rs->restraints_end(); ++it) {
      add_restraint_internal(*it, index,
                             nullptr, std::numeric_limits<double>::max(),
                             Subset(), dependencies);
    }
  }
}

void RestraintCache::add_restraint_set_child_internal(Restraint *r,
                                                      const Subset &cur_subset,
                                                      RestraintSet *parent,
                                                      double parent_max,
                                                      Subset parent_subset) {
  if (!parent) return;
  IMP_LOG_TERSE( "Adding restraint " << Showable(r)
          << " to set " << Showable(parent) << std::endl);
  cache_.access_generator().add_to_set(parent,
                                       r,
                                       Slice(parent_subset,
                                             cur_subset),
                                       parent_max);
}
Subset RestraintCache::get_subset(Restraint *r,
                                  const DepMap &dependencies) const {
  ParticlesTemp ups= IMP::get_input_particles(r->get_inputs());
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
                                            unsigned int index,
                                            RestraintSet *parent,
                                            double parent_max,
                                            Subset parent_subset,
                                            const DepMap &dependencies) {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Processing " << Showable(r) << " with "
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
    IMP_LOG_TERSE( "Adding restraint " << Showable(r)
            << " with max " << cur_max << " and subset " << cur_subset
            << std::endl);
    known_restraints_[r]=cur_subset;
    restraint_index_[r]=index;
  }
  add_restraint_set_child_internal(r,
                                   cur_subset,
                                   parent,
                                   parent_max,
                                   parent_subset);
  RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
  if (rs) {
    add_restraint_set_internal(rs, index, cur_subset, cur_max, dependencies);
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
  DependencyGraph dg = get_dependency_graph(m);
  ParticleStatesTable *pst= cache_.get_generator().get_particle_states_table();
  DepMap dependencies;
  ParticlesTemp allps= pst->get_particles();
  DependencyGraphVertexIndex index= IMP::get_vertex_index(dg);
  for (unsigned int i=0; i< allps.size(); ++i) {
    ParticlesTemp depp= get_dependent_particles(allps[i], allps, dg, index);
    for (unsigned int j=0; j< depp.size(); ++j) {
      dependencies[depp[j]].push_back(allps[i]);
    }
    dependencies[allps[i]].push_back(allps[i]);
    IMP_LOG_TERSE( "Particle " << Showable(allps[i])
            << " controls " << dependencies[allps[i]] << std::endl);
  }

  for (unsigned int i=0; i< rs.size(); ++i) {
    Pointer<Restraint> r= rs[i]->create_decomposition();
    IMP_IF_LOG(TERSE) {
      IMP_LOG_TERSE( "Before:" << std::endl);
      IMP_LOG_WRITE(TERSE, show_restraint_hierarchy(rs[i]));
    }
    if (r) {
      IMP_LOG_TERSE( "after:" << std::endl);
      IMP_LOG_WRITE(TERSE, show_restraint_hierarchy(r));
      add_restraint_internal(r,
                             next_index_,
                             nullptr,
                             std::numeric_limits<double>::max(),
                             Subset(),
                             dependencies);
    }
    ++next_index_;
  }
  IMP_IF_LOG(TERSE) {
    IMP_LOG_WRITE(TERSE, show_restraint_information(IMP_STREAM));
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
RestraintsTemp RestraintCache::get_restraints() const {
  IMP_OBJECT_LOG;
  RestraintsTemp ret;
  for (KnownRestraints::const_iterator it= known_restraints_.begin();
       it != known_restraints_.end(); ++it) {
    ret.push_back(it->first);
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


double RestraintCache::get_score(Restraint *r, const Subset &s,
                                 const Assignment &a) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  Slice slice= get_slice(r, s);
  Assignment ra=slice.get_sliced(a);
  return get_score(r, ra);
}



void RestraintCache::load_last_score(Restraint *r, const Subset &s,
                                     const Assignment &a) {
  IMP_OBJECT_LOG;
  double ss= get_score(r, s, a);
  r->set_last_score(ss);
}


void RestraintCache::validate() const {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  IMP_OBJECT_LOG;
  IMP_LOG_VERBOSE( "Validating cache...." << std::endl);
  for (Cache::ContentIterator it= cache_.contents_begin();
       it != cache_.contents_end(); ++it) {
    double score= it->value;
    double new_score= cache_.get_generator()(it->key, cache_);
    IMP_LOG_VERBOSE( "Validating " << it->key << std::endl);
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(score, new_score,
                                   "Cached and computed scores don't match "
                                   << score << " vs " << new_score);

  }
#endif
}
/* Structure is one child group per restraints with two data sets,
   one for all the scores and one for the assignments.

 */
#if IMP_DOMINO_HAS_RMF
namespace {
  Ints get_ids(const base::map<Particle*, int> &map,
               const Subset &s) {
    Ints ret(s.size());
    for (unsigned int i=0; i<s.size(); ++i) {
      ret[i]= map.find(s[i])->second;
    }
    std::sort(ret.begin(), ret.end());
    return ret;
  }
  IMP_NAMED_TUPLE_2(RestraintID,RestraintIDs,
                    int, restraint_index,
                    base::ConstVector<unsigned int>, particle_indexes,);

  typedef base::map<Particle*, int> ParticleIndex;
  RestraintID get_restraint_id(const ParticleIndex &map,
                               const Subset &s,
                               unsigned int restraint_index) {
    RestraintID ret;
    ret.set_restraint_index(restraint_index);
    Ints pis= get_ids(map, s);
    ret.set_particle_indexes(base::ConstVector<unsigned int>(pis));
    return ret;
  }
  ParticleIndex get_particle_index(const ParticlesTemp &particle_ordering) {
    ParticleIndex map;
    for (unsigned int i=0; i< particle_ordering.size(); ++i) {
      map[particle_ordering[i]]=i;
    }
    return map;
  }
  Orders get_orders(const base::map<Pointer<Restraint>, Subset>
                    &known_restraints,
                    const RestraintsTemp &restraints,
                    const ParticlesTemp &particle_ordering) {
    Orders ret(restraints.size());
    for (unsigned int i=0; i< restraints.size(); ++i) {
      ret[i]=Order(known_restraints.find(restraints[i])->second,
                      particle_ordering);
    }
    return ret;
  }
}

void RestraintCache::save_cache(const ParticlesTemp &particle_ordering,
                                const RestraintsTemp &restraints,
                                RMF::HDF5::Group group,
                                unsigned int max_entries) {
  RMF::HDF5::FloatDataSet1Ds scores;
  RMF::HDF5::IntDataSet2Ds assignments;
  base::map<Restraint*, int> restraint_index;
  ParticleIndex particle_index=get_particle_index(particle_ordering);
  Orders orders= get_orders(known_restraints_, restraints, particle_ordering);
  // create data sets for restraints
  for (unsigned int i=0; i< restraints.size(); ++i) {
    Restraint *r=restraints[i];
    RestraintID rid= get_restraint_id(particle_index,
                                      known_restraints_.find(r)->second,
                                      restraint_index_.find(r)->second);
    RMF::HDF5::Group g= group.add_child_group(r->get_name());
    g.set_attribute<RMF::HDF5::IndexTraits>("restraint",
                                   RMF::HDF5::Indexes(1,
                                                   rid.get_restraint_index()));
    g.set_attribute<RMF::HDF5::IndexTraits>("particles",
                         RMF::HDF5::Indexes(rid.get_particle_indexes().begin(),
                                             rid.get_particle_indexes().end()));
    scores.push_back(g.add_child_data_set<RMF::HDF5::FloatTraits, 1>("scores"));
    assignments.push_back(g.add_child_data_set
                          <RMF::HDF5::IntTraits, 2>("assignments"));
    restraint_index[r]=i;
  }
  // finally start saving them
  unsigned int count=0;
  for (Cache::ContentIterator it= cache_.contents_begin();
       it != cache_.contents_end(); ++it) {
    int ri= restraint_index.find(it->key.get_restraint())->second;
    Ints ord= orders[ri].get_list_ordered(it->key.get_assignment());
    double score= it->value;
    RMF::HDF5::DataSetIndexD<2> asz= assignments[ri].get_size();
    RMF::HDF5::DataSetIndexD<1> row(asz[0]);
    asz[1]=ord.size();
    ++asz[0];
    assignments[ri].set_size(asz);
    assignments[ri].set_row(row, RMF::HDF5::Ints(ord.begin(), ord.end()));
    RMF::HDF5::DataSetIndexD<1> ssz= scores[ri].get_size();
    RMF::HDF5::DataSetIndexD<1> nsz=ssz;
    ++nsz[0];
    scores[ri].set_size(nsz);
    scores[ri].set_value(ssz, score);
    ++count;
    if (count > max_entries) break;
  }
}

void RestraintCache::load_cache(const ParticlesTemp &particle_ordering,
                                RMF::HDF5::ConstGroup group) {
  ParticleIndex particle_index=get_particle_index(particle_ordering);
  base::map<RestraintID, Restraint*> index;
  for (KnownRestraints::const_iterator it=known_restraints_.begin();
       it != known_restraints_.end(); ++it) {
    index[get_restraint_id(particle_index, it->second,
                           restraint_index_.find(it->first)->second)]
      = it->first;
  }
  RestraintsTemp restraints;
  for (unsigned int i=0; i< group.get_number_of_children(); ++i) {
    RMF::HDF5::ConstGroup ch= group.get_child_group(i);
    int restraint_index
      = ch.get_attribute<RMF::HDF5::IndexTraits>("restraint")[0];
    RMF::HDF5::Indexes particle_indexes
      = ch.get_attribute<RMF::HDF5::IndexTraits>("particles");
    RestraintID rid(restraint_index,
                  base::ConstVector<unsigned int>(Ints(particle_indexes.begin(),
                                                      particle_indexes.end())));
    Restraint *r= index.find(rid)->second;
    restraints.push_back(r);
    IMP_LOG_TERSE( "Matching " << Showable(r) << " with "
            << ch.get_name() << std::endl);
  }
  Orders orders= get_orders(known_restraints_, restraints, particle_ordering);
  for (unsigned int i=0; i< group.get_number_of_children(); ++i) {
    RMF::HDF5::ConstGroup ch= group.get_child_group(i);
    RMF::HDF5::FloatConstDataSet1D scores
      = ch.get_child_float_data_set_1d("scores");
    RMF::HDF5::IntConstDataSet2D assignments
      = ch.get_child_int_data_set_2d("assignments");
    for (unsigned int j=0; j< scores.get_size()[0]; ++j) {
      double s= scores.get_value(RMF::HDF5::DataSetIndex1D(j));
      RMF::HDF5::Ints rw= assignments.get_row(RMF::HDF5::DataSetIndex1D(j));
      Ints psit(rw.begin(), rw.end());
      Assignment ass= orders[i].get_subset_ordered(psit);
      cache_.insert(Key(restraints[i], ass), s);
    }
  }
  validate();
}
#endif
IMPDOMINO_END_NAMESPACE
