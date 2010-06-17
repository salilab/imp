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
ModelData::ModelData(Model *m, const Model::DependencyGraph &dg,
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
  for (Model::RestraintIterator rit= m_->restraints_begin();
       rit != m_->restraints_end(); ++rit) {
    ParticlesTemp ip= (*rit)->get_input_particles();
    ParticlesTemp oip(ip.size());
    for (unsigned int i=0; i< ip.size(); ++i) {
      oip[i]= idm[ip[i]];
    }
    std::sort(oip.begin(), oip.end());
    oip.erase(std::unique(oip.begin(), oip.end()), oip.end());
    dependencies_.push_back(oip);
    rdata_.push_back(RestraintData(*rit, m_->get_weight(*rit)));
  }
}
const SubsetData &ModelData::get_subset_data(const Subset &s) const {
  if (sdata_.find(s) == sdata_.end()) {
    unsigned int i=0;
    ParticleIndex pi= get_index(s);
    Ints ris;
    std::vector<Ints> inds;
    for (Model::RestraintIterator rit= m_->restraints_begin();
         rit != m_->restraints_end(); ++rit) {
      if (std::includes(s.begin(), s.end(),
                        dependencies_[i].begin(), dependencies_[i].end())) {
        ris.push_back(i);
        inds.push_back(Ints());
        for (unsigned int j=0; j< dependencies_[i].size(); ++j) {
          inds.back().push_back(pi.find(dependencies_[i][j])->second);
        }
      }
      ++i;
    }
    sdata_[s]= SubsetData(this, ris, inds, s);
  }
  return sdata_.find(s)->second;
}


double SubsetData::get_score(const SubsetState &state,
                             double max) const {
  double score=0;
  for (unsigned int i=0; i< ris_.size(); ++i) {
    SubsetState ss(indices_[i]);
    ParticlesTemp ps(ss.size());
    for (unsigned int j=0; j< ss.size(); ++j) {
      ps[j]= s_[indices_[i][j]];
    }
    double ms=md_->rdata_[ris_[i]].get_score(md_->pst_,
                                             ps, ss);
    score+= ms;
    if (score > max) {
      return std::numeric_limits<double>::max();
    }
  }
  return score;
}

IMPDOMINO2_END_INTERNAL_NAMESPACE
