/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/internal/restraint_evaluator.h>
#include <IMP/domino/utility.h>
#include <IMP/domino/internal/inference_utility.h>
#include <IMP/internal/map.h>
#include <algorithm>


IMPDOMINO_BEGIN_INTERNAL_NAMESPACE
ModelData::ModelData(RestraintSet *rs,
                          ParticleStatesTable* pst) {
  rs_=rs;
  pst_=pst;
  initialized_=false;
  cache_=false;
}

void ModelData::set_use_caching(bool tf) {
  cache_= tf;
  for (unsigned int i=0; i < rdata_.size(); ++i) {
    rdata_[i].set_use_caching(tf);
  }
}

namespace {
  void handle_restraint(Restraint *r,
                                 double weight,
                        const IMP::internal::Map<Particle*, ParticlesTemp> &idm,
      const IMP::internal::Map<Restraint*, ModelData::PreloadData> &preload,
                                 std::vector<Subset> &dependencies,
                                 ParticlesTemp ip,
                                 std::vector<RestraintData> &rdata,
                        IMP::internal::Map<Restraint*, Ints> &index,
                        bool cache) {
    std::vector<ParticlesTemp> oip;
    oip.push_back(ParticlesTemp());
    for (unsigned int i=0; i< ip.size(); ++i) {
      if (idm.find(ip[i]) != idm.end()) {
        if (idm.find(ip[i])->second.size() > 1) {
          std::vector<ParticlesTemp> noip;
          int last=0;
          for (unsigned int j=0; j< idm.find(ip[i])->second.size(); ++j) {
            noip.insert(noip.end(), oip.begin(), oip.end());
            for (unsigned int k=last; k< noip.size(); ++k) {
              noip[k].push_back(idm.find(ip[i])->second[j]);
            }
            last= noip.size();
          }
          std::swap(noip, oip);
        } else {
          for (unsigned int j=0; j< oip.size(); ++j) {
            oip[j].push_back(idm.find(ip[i])->second[0]);
          }
        }
      }
    }
    for (unsigned int i=0; i< oip.size(); ++i) {
      std::sort(oip[i].begin(), oip[i].end());
      oip[i].erase(std::unique(oip[i].begin(), oip[i].end()), oip[i].end());
      dependencies.push_back(Subset(oip[i], true));
      RestraintData ret(r, weight);
      if (preload.find(r) != preload.end()){
        const ModelData::PreloadData &data= preload.find(r)->second;
        IMP_USAGE_CHECK(dependencies.back() == data.s,
                        "Was passed subset " << data.s
                        << " but expected subset " << dependencies.back()
                        << " for restraint " << r->get_name());
        for (unsigned int j=0; j< data.scores.size(); ++j) {
          ret.set_score(data.sss[j], data.scores[j]);
        }
      }
      ret.set_use_caching(cache);
      rdata.push_back(ret);
      index[r].push_back(rdata.size()-1);
    }
  }
}

void ModelData::initialize() {
  IMP_FUNCTION_LOG;
  //IMP_LOG(SILENT, "Initializing model score data" << std::endl);
  DependencyGraph dg= get_dependency_graph(RestraintsTemp(1, rs_));
  const ParticlesTemp all= pst_->get_particles();
  IMP::internal::Map<Particle*, ParticlesTemp> idm;
  for (unsigned int i=0; i < all.size(); ++i) {
    Particle *p= all[i];
    ParticlesTemp ps= get_dependent_particles(p, all, dg);
    for (unsigned int j=0; j< ps.size(); ++j) {
      idm[ps[j]].push_back(p);
    }
  }
  IMP::internal::Map<Restraint*, Ints> index;
  Restraints restraints= get_restraints(rs_->restraints_begin(),
                                        rs_->restraints_end());
  for (Restraints::const_iterator rit= restraints.begin();
       rit != restraints.end(); ++rit) {
    ParticlesTemp ip= (*rit)->get_input_particles();
    double weight=rs_->get_model()->get_weight(*rit);
    handle_restraint(*rit, weight, idm, preload_,
                     dependencies_, ip,
                     rdata_, index, cache_);
  }
  RestraintSets restraint_sets= get_restraint_sets(rs_->restraints_begin(),
                                                   rs_->restraints_end());
  restraint_sets.push_back(rs_);
  for (unsigned int i=0; i< restraint_sets.size(); ++i) {
    double max=restraint_sets[i]
      ->get_maximum_score();
    if (max >= std::numeric_limits<double>::max() && restraint_sets[i]==rs_) {
      max= restraint_sets[i]->get_model()->get_maximum_score();
    }
    if (max >= std::numeric_limits<double>::max()) {
      continue;
    }
    std::pair<Restraints, Floats> cur=
      get_restraints_and_weights(restraint_sets[i]->restraints_begin(),
                                 restraint_sets[i]->restraints_end(),
                                 restraint_sets[i]->get_weight());
    Ints curi;
    Floats curw;
    for (unsigned int j=0; j< cur.first.size(); ++j) {
      curi.insert(curi.end(), index[cur.first[j]].begin(),
                  index[cur.first[j]].end());
      curw.push_back(cur.second[j]);
    }
    sets_.push_back(std::make_pair(max, curi));
    set_weights_.push_back(curw);
    IMP_LOG(TERSE, "Restraint set " << restraint_sets[i]->get_name()
            << " has maximum score " << max
            << " over " << cur.first.size() << " restraints."
            << std::endl);
  }

  for (unsigned int i=0; i< rdata_.size(); ++i) {
    double max= rdata_[i].get_restraint()->get_maximum_score();
    /*std::cout << "Restraint " << rdata_[i].get_restraint()->get_name()
      << " has max of " << max << std::endl;*/
    IMP_LOG(TERSE, "Restraint " << rdata_[i].get_restraint()->get_name()
            << " has max of " << max << std::endl);
    rdata_[i].set_max(max);
  }
  initialized_=true;
}

void ModelData::validate() const {
  IMP_USAGE_CHECK(get_restraints(rs_->restraints_begin(),
                                 rs_->restraints_end()).size()
                  == dependencies_.size(),
                     "The restraints changed after Domino was set up. "
                  << "This is a bad thing: "
                  << get_restraints(rs_->restraints_begin(),
                                    rs_->restraints_end()).size()
                  << " vs " << dependencies_.size());
  IMP_INTERNAL_CHECK(dependencies_.size()== rdata_.size(),
                     "Inconsistent data in Restraint evaluator or Filter");
}

const SubsetData &ModelData::get_subset_data(const Subset &s,
                                             const Subsets &exclusions) const {
  IMP_FUNCTION_LOG;
  if (!initialized_) {
    const_cast<ModelData*>(this)->initialize();
  }
  validate();
  SubsetID id(s, exclusions);
  if (sdata_.find(id) == sdata_.end()) {
    ParticleIndex pi= get_index(s);
    Ints ris;
    std::vector<Ints> inds;
    Ints total_ris;
    std::vector<Ints> total_inds;
    IMP::internal::Set<Restraint*> found;
    //std::cout << "Find data for subset " << s << std::endl;
    for (unsigned int i=0; i< dependencies_.size(); ++i) {
      if (std::includes(s.begin(), s.end(),
                        dependencies_[i].begin(), dependencies_[i].end())) {
        if (found.find(rdata_[i].get_restraint()) != found.end()) {
          // restraints with multiple support sets
          continue;
        }
        found.insert(rdata_[i].get_restraint());
        bool exclude=false;
        {for (unsigned int j=0; j< exclusions.size(); ++j) {
            if (std::includes(exclusions[j].begin(),
                              exclusions[j].end(),
                              dependencies_[i].begin(),
                              dependencies_[i].end())) {
              exclude=true;
              break;
            }
          }}
        if(!exclude) {
          //std::cout << "Found restraint " << (*rit)->get_name() << std::endl;
          ris.push_back(i);
          inds.push_back(Ints());
          for (unsigned int j=0; j< dependencies_[i].size(); ++j) {
            inds.back().push_back(pi.find(dependencies_[i][j])->second);
          }
        } else {
          total_ris.push_back(i);
          total_inds.push_back(Ints());
          for (unsigned int j=0; j< dependencies_[i].size(); ++j) {
            total_inds.back().push_back(pi.find(dependencies_[i][j])->second);
          }
        }
      }
    }
    std::vector<std::pair<double, Ints> > set_ris;
    std::vector<std::vector<Ints> > set_inds;
    std::vector<Floats> set_weights;
    for (unsigned int i=0; i< sets_.size(); ++i) {
      Ints cris;
      std::vector<Ints> cinds;
      Floats weights;
      for (unsigned int j=0; j< ris.size(); ++j) {
        for (unsigned int k=0; k < sets_[i].second.size(); ++k) {
          if (sets_[i].second[k]== ris[j]) {
            cris.push_back(ris[j]);
            cinds.push_back(inds[j]);
            weights.push_back(set_weights_[i][k]);
            break;
          }
        }
      }
      for (unsigned int j=0; j< total_ris.size(); ++j) {
        for (unsigned int k=0; k < sets_[i].second.size(); ++k) {
          if (sets_[i].second[k]== total_ris[j]) {
            cris.push_back(total_ris[j]);
            cinds.push_back(total_inds[j]);
            weights.push_back(set_weights_[i][k]);
            break;
          }
        }
      }
      if (cris.size() >1) {
        IMP_LOG(VERBOSE, "Adding restraint set with size "
                << cris.size() << std::endl);
        set_ris.push_back(std::make_pair(sets_[i].first, cris));
        set_inds.push_back(cinds);
        set_weights.push_back(weights);
      }
    }
    sdata_[id]= SubsetData(this, ris, set_ris, inds,set_inds, set_weights, s);
  }
  return sdata_.find(id)->second;
}



double SubsetData::get_score(const Assignment &state) const {
  double score=0;
  /*std::cout << "Scoring " << state << " on " << ris_.size()
    << " restraints" << std::endl;*/
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    Assignment ss(ssi);
    ParticlesTemp ps(ss.size());
    for (unsigned int j=0; j< ss.size(); ++j) {
      ps[j]= s_[indices_[i][j]];
    }
    double ms=md_->rdata_[ris_[i]].get_score<false>(md_->pst_,
                                             ps, ss);
    score+= ms;
  }
  return score;
}

bool SubsetData::get_is_ok(const Assignment &state,
                           double total_max) const {
  /*std::cout << "For subset " << s_ << " got state "
    << state << std::endl;*/
  double total=0;
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    Assignment ss(ssi);
    ParticlesTemp ps(ss.size());
    for (unsigned int j=0; j< ss.size(); ++j) {
      ps[j]= s_[indices_[i][j]];
    }
    double ms=md_->rdata_[ris_[i]].get_score<true>(md_->pst_,
                                                   ps, ss);
    if (ms >= std::numeric_limits<double>::max()) {
      return false;
    }
    total+=ms;
    if (total > total_max) {
      //std::cout << "Rejected by restraint " << i << std::endl;
      return false;
    }
  }
  for (unsigned int h=0; h < set_ris_.size(); ++h) {
    double set_total=0;
    for (unsigned int i=0; i< set_ris_[h].second.size(); ++i) {
      Ints ssi(set_indices_[h][i].size());
      for (unsigned int j=0; j< ssi.size();++j) {
        ssi[j]= state[set_indices_[h][i][j]];
      }
      Assignment ss(ssi);
      ParticlesTemp ps(ss.size());
      for (unsigned int j=0; j< ss.size(); ++j) {
        ps[j]= s_[set_indices_[h][i][j]];
      }
      double ms=md_->rdata_[set_ris_[h].second[i]].get_score<true>(md_->pst_,
                                                                   ps, ss);
      set_total+=ms *set_weights_[h][i];
      if (set_total > set_ris_[h].first) {
        return false;
      }
    }
  }
  /*std::cout << "Total score is " << total << " max is " << total_max
    << " over " << ris_.size() << " and " << total_ris_.size() << std::endl;*/
  return true;
}
void ModelData
::add_score(Restraint *r, const Subset &subset,
               const Assignment &state, double score) {
  if (preload_.find(r) == preload_.end()) {
    preload_[r]= PreloadData();
    preload_[r].s= subset;
  }
  preload_[r].sss.push_back(state);
  preload_[r].scores.push_back(score);
}


IMPDOMINO_END_INTERNAL_NAMESPACE
