/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/internal/restraint_evaluator.h>
#include <IMP/domino2/internal/inference.h>
#include <IMP/domino2/utility.h>
#include <algorithm>


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE
ModelData::ModelData(Model *m, RestraintSet *rs,
                     const DependencyGraph &dg,
                     ParticleStatesTable* pst): m_(m), pst_(pst) {
  const ParticlesTemp all= pst->get_particles();
  std::map<Particle*, Particle*> idm;
  for (unsigned int i=0; i < all.size(); ++i) {
    Particle *p= all[i];
    ParticlesTemp ps= get_dependent_particles(p, dg);
    for (unsigned int j=0; j< ps.size(); ++j) {
      idm[ps[j]]=p;
    }
  }
  Restraints rss= get_restraints(rs->restraints_begin(),
                                 rs->restraints_end());
  for (Restraints::const_iterator rit= rss.begin();
       rit != rss.end(); ++rit) {
    ParticlesTemp ip= (*rit)->get_input_particles();
    ParticlesTemp oip;
    for (unsigned int i=0; i< ip.size(); ++i) {
      if (idm.find(ip[i]) != idm.end()) {
        oip.push_back(idm.find(ip[i])->second);
      }
    }
    std::sort(oip.begin(), oip.end());
    oip.erase(std::unique(oip.begin(), oip.end()), oip.end());
    dependencies_.push_back(oip);
    rdata_.push_back(RestraintData(*rit, m_->get_weight(*rit)));
  }
}

void ModelData::set_sampler(const Sampler *s) {
  for (unsigned int i=0; i< rdata_.size(); ++i) {
    double max= s->get_maximum_score(rdata_[i].get_restraint());
    rdata_[i].set_max(max);
  }
}

const SubsetData &ModelData::get_subset_data(const Subset &s,
                                             const Subsets &exclusions) const {
  SubsetID id(s, exclusions);
  if (sdata_.find(id) == sdata_.end()) {
    unsigned int i=0;
    ParticleIndex pi= get_index(s);
    Ints ris;
    std::vector<Ints> inds;
    //std::cout << "Find data for subset " << s << std::endl;
    for (Model::RestraintIterator rit= m_->restraints_begin();
         rit != m_->restraints_end(); ++rit) {
      if (std::includes(s.begin(), s.end(),
                        dependencies_[i].begin(), dependencies_[i].end())) {
        bool exclude=false;
        {for (unsigned int i=0; i< exclusions.size(); ++i) {
            if (std::includes(exclusions[i].begin(),
                              exclusions[i].end(),
                              dependencies_[i].begin(),
                              dependencies_[i].end())) {
              exclude=true;
            }
          }}
        if(!exclude) {
          //std::cout << "Found restraint " << (*rit)->get_name() << std::endl;
          ris.push_back(i);
          inds.push_back(Ints());
          for (unsigned int j=0; j< dependencies_[i].size(); ++j) {
            inds.back().push_back(pi.find(dependencies_[i][j])->second);
          }
        }
      }
      ++i;
    }
    sdata_[id]= SubsetData(this, ris, inds, s);
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

bool SubsetData::get_is_ok(const SubsetState &state) const {
  /*std::cout << "For subset " << s_ << " got state "
    << state << std::endl;*/
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
  }
  return true;
}

IMPDOMINO2_END_INTERNAL_NAMESPACE
