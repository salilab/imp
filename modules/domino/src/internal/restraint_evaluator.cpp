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
}
void ModelData::initialize() {
  IMP_LOG(SILENT, "Initializing model score data" << std::endl);
  DependencyGraph dg= get_dependency_graph(RestraintsTemp(1, rs_));
  const ParticlesTemp all= pst_->get_particles();
  IMP::internal::Map<Particle*, Particle*> idm;
  for (unsigned int i=0; i < all.size(); ++i) {
    Particle *p= all[i];
    ParticlesTemp ps= get_dependent_particles(p, dg);
    for (unsigned int j=0; j< ps.size(); ++j) {
      idm[ps[j]]=p;
    }
  }
  Restraints restraints= get_restraints(rs_->restraints_begin(),
                                        rs_->restraints_end());
  for (Restraints::const_iterator rit= restraints.begin();
       rit != restraints.end(); ++rit) {
    ParticlesTemp ip= (*rit)->get_input_particles();
    ParticlesTemp oip;
    for (unsigned int i=0; i< ip.size(); ++i) {
      if (idm.find(ip[i]) != idm.end()) {
        oip.push_back(idm.find(ip[i])->second);
      }
    }
    std::sort(oip.begin(), oip.end());
    oip.erase(std::unique(oip.begin(), oip.end()), oip.end());
    dependencies_.push_back(Subset(oip, true));
    rdata_.push_back(RestraintData(*rit, rs_->get_model()->get_weight(*rit)));
    if (preload_.find(*rit) != preload_.end()){
      const PreloadData &data= preload_.find(*rit)->second;
      IMP_USAGE_CHECK(dependencies_.back() == data.s,
                      "Was passed subset " << data.s
                      << " but expected subset " << dependencies_.back()
                      << " for restraint " << (*rit)->get_name());
      for (unsigned int i=0; i< data.scores.size(); ++i) {
        rdata_.back().set_score(data.sss[i], data.scores[i]);
      }
    }
  }
  for (unsigned int i=0; i< rdata_.size(); ++i) {
    double max= rs_->get_model()->get_maximum_score(rdata_[i].get_restraint());
    /*std::cout << "Restraint " << rdata_[i].get_restraint()->get_name()
      << " has max of " << max << std::endl;*/
    IMP_LOG(VERBOSE, "Restraint " << rdata_[i].get_restraint()->get_name()
            << " has max of " << max << std::endl);
    rdata_[i].set_max(max);
  }
  initialized_=true;
}

void ModelData::validate() const {
  IMP_USAGE_CHECK(get_restraints(rs_->restraints_begin(),
                                 rs_->restraints_end()).size()
                  == dependencies_.size(),
                     "The restraints changed after Domino2 was set up. "
                  << "This is a bad thing: "
                  << get_restraints(rs_->restraints_begin(),
                                    rs_->restraints_end()).size()
                  << " vs " << dependencies_.size());
  IMP_INTERNAL_CHECK(dependencies_.size()== rdata_.size(),
                     "Inconsistent data in Restraint evaluator or Filter");
}

const SubsetData &ModelData::get_subset_data(const Subset &s,
                                             const Subsets &exclusions) const {
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
    //std::cout << "Find data for subset " << s << std::endl;
    for (unsigned int i=0; i< dependencies_.size(); ++i) {
      if (std::includes(s.begin(), s.end(),
                        dependencies_[i].begin(), dependencies_[i].end())) {
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
    sdata_[id]= SubsetData(this, ris, total_ris, inds,total_inds, s);
  }
  return sdata_.find(id)->second;
}



double SubsetData::get_score(const SubsetState &state) const {
  double score=0;
  /*std::cout << "Scoring " << state << " on " << ris_.size()
    << " restraints" << std::endl;*/
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    SubsetState ss(ssi);
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

bool SubsetData::get_is_ok(const SubsetState &state,
                           double total_max) const {
  /*std::cout << "For subset " << s_ << " got state "
    << state << std::endl;*/
  double total=0;
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    SubsetState ss(ssi);
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
  if (total_max < std::numeric_limits<double>::max()) {
    for (unsigned int i=0; i< total_ris_.size(); ++i) {
      Ints ssi(total_indices_[i].size());
      for (unsigned int j=0; j< ssi.size();++j) {
        ssi[j]= state[total_indices_[i][j]];
      }
      SubsetState ss(ssi);
      ParticlesTemp ps(ss.size());
      for (unsigned int j=0; j< ss.size(); ++j) {
        ps[j]= s_[total_indices_[i][j]];
      }
      double ms=md_->rdata_[total_ris_[i]].get_score<true>(md_->pst_,
                                                           ps, ss);
      total+=ms;
      if (total > total_max) {
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
               const SubsetState &state, double score) {
  if (preload_.find(r) == preload_.end()) {
    preload_[r]= PreloadData();
    preload_[r].s= subset;
  }
  preload_[r].sss.push_back(state);
  preload_[r].scores.push_back(score);
}


IMPDOMINO_END_INTERNAL_NAMESPACE
