/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/ConnectivityRestraint.h"
#include "IMP/PairScore.h"

#include <cmath>
#include <set>

namespace IMP
{


ConnectivityRestraint::ConnectivityRestraint(PairScore *ps): ps_(ps)
{
  clear_sets();
}


ConnectivityRestraint::~ConnectivityRestraint()
{
}

struct Pair {
  typedef Pair This;
  Pair(){p_[0]=-1; p_[1]=-1;}
  Pair(int a, int b){
    p_[0]= std::min(a,b);
    p_[1]= std::max(a,b);
  }
  int operator[](unsigned int i) const {
    return p_[i];
  }
  bool is_default() const {
    return p_[0] < 0 && p_[1] < 0;
  }
  IMP_COMPARISONS_2(p_[0], p_[1]);
  int p_[2];
};

void ConnectivityRestraint::add_set(const Particles &ps)
{
  IMP_check(!ps.empty(), "Cannot add empty set to ConnectivityRestraint",
            InvalidStateException("Empty set"));
  unsigned int sz= ps.size();
  Restraint::add_particles(ps);
  set_offsets_.push_back(set_offsets_.back()+sz);
}

void ConnectivityRestraint::clear_sets()
{
  set_offsets_.clear();
  Restraint::clear_particles();
  set_offsets_.push_back(0);
}

Float ConnectivityRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(ps_.get());
  unsigned int num_sets= set_offsets_.size()-1;
  typedef std::vector<std::pair<Pair, float> > Edges;
  typedef std::map<Pair, Edges> EMap;
  EMap edges;
  for (unsigned int i=0; i< num_sets; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      Pair cp(i,j);
      for (unsigned int ii= set_offsets_[i]; ii != set_offsets_[i+1]; ++ii) {
        for (unsigned int ij= set_offsets_[j]; ij != set_offsets_[j+1]; ++ij) {
          Pair pp(ii, ij);
          float d= ps_->evaluate(get_particle(ii), get_particle(ij), NULL);
          edges[cp].push_back(std::make_pair(pp,d));
        }
      }
    }
  }

  std::set<int> in;
  in.insert(0);
  Edges picked_edges;
  while (in.size() != num_sets) {
    float best=std::numeric_limits<float>::max();
    Pair best_edge, best_sets;
    IMP_assert(!edges.empty(), "No more edges :-(");
    for (EMap::iterator it = edges.begin(); it != edges.end(); ++it) {
      Pair pit= it->first;
      /*IMP_LOG(VERBOSE, "Looking at edges between " << pit[0] << " " 
        << pit[1] << std::endl);*/
      IMP_assert(in.find(pit[0]) == in.end() 
                 || in.find(pit[1]) == in.end(), 
                 "Edge only involves already visited nodes");
      if (in.find(pit[0]) == in.end() && in.find(pit[1]) == in.end()) {
        // want to maintain a connected tree
        continue;
      }
      for (unsigned int i=0; i< it->second.size(); ++i) {
        if (it->second[i].second < best) {
          best= it->second[i].second;
          best_edge= it->second[i].first;
          IMP_assert(best_edge != Pair(), "Bad pair");
          best_sets= it->first;
        }
      }
    }
    int newi=best_sets[0];
    if (in.find(newi) != in.end()) newi= best_sets[1];

    IMP_LOG(VERBOSE, "Adding set " << newi << " with score " 
            << best << std::endl);
    picked_edges.push_back(std::make_pair(best_edge, best));
    in.insert(newi);
    IMP_assert(edges.find(best_sets) != edges.end(),
               "Where did they go?");
    /*IMP_LOG(VERBOSE, "Removing edges " << best_sets[0] << " " 
      << best_sets[1] << std::endl);*/
    edges.erase(best_sets);
  }

  float sum=0;
  // could be more clever if accum is NULL
  for (unsigned int i=0; i< picked_edges.size(); ++i) {
    sum+= ps_->evaluate(get_particle(picked_edges[i].first[0]),
                        get_particle(picked_edges[i].first[1]),
                        accum);
  }
  return sum;
}


void ConnectivityRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "connectivity restraint (active):" << std::endl;
  } else {
    out << "connectivity restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  ";
  out << "last_modified_by: " << last_modified_by() << std::endl;
}

}  // namespace IMP
