/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"
#include "IMP/internal/graph_utility.h"
#include "IMP/file.h"
#include <boost/timer.hpp>
#include <set>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


IMP_BEGIN_NAMESPACE
typedef boost::graph_traits<DependencyGraph> DGTraits;
typedef std::map<Object*, DGTraits::vertex_descriptor> DGIndex;
typedef boost::property_map<DependencyGraph, boost::vertex_name_t>::const_type
DGConstVertexMap;



namespace {
  template <class C>
  C filter(C c) {
    std::sort(c.begin(), c.end());
    c.erase(std::unique(c.begin(), c.end()), c.end());
    IMP_INTERNAL_CHECK(c.empty() || c[0] != NULL,
                       "NULL returned for dependencies.");
    return c;
  }


  bool get_has_edge(const DependencyGraph &graph,
                    DGTraits::vertex_descriptor va,
                    DGTraits::vertex_descriptor vb) {
    std::pair<DGTraits::out_edge_iterator,
      DGTraits::out_edge_iterator> edges= boost::out_edges(va, graph);
    for (; edges.first != edges.second;++edges.first) {
      if (boost::target(*edges.first, graph) == vb) return true;
    }
    return false;
  }

  void add_edge(DependencyGraph &graph,
                DGTraits::vertex_descriptor va,
                DGTraits::vertex_descriptor vb) {
    if (get_has_edge(graph, va, vb)) return;
    IMP_INTERNAL_CHECK(va != vb, "Can't dependend on itself " << va);
    IMP_INTERNAL_CHECK(!get_has_edge(graph, va, vb),
                       "Already has edge between " << va << " and " << vb);
    boost::add_edge(va, vb, graph);
    IMP_INTERNAL_CHECK(get_has_edge(graph, va, vb),
                       "No has edge between " << va << " and " << vb);

  }

  DGTraits::vertex_descriptor get_vertex(DependencyGraph &dg,
                                         DGIndex &dgi,
                                         Object *o) {
    DGIndex::const_iterator it=dgi.find(o);
    if (it==dgi.end()) {
      boost::property_map<DependencyGraph, boost::vertex_name_t>::type vm
        = boost::get(boost::vertex_name, dg);
      DGTraits::vertex_descriptor v= boost::add_vertex(dg);
      vm[v]=o;
      dgi[o]=v;
      return v;
    } else {
      return it->second;
    }
  }

  template <class It>
  void build_inputs_graph(It b, It e,
                          DependencyGraph &dg,
                          DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      DGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing inputs for \""
              << (*c)->get_name() << "\"" << std::endl);
      {
        ContainersTemp ct= filter((*c)->get_input_containers());
        IMP_LOG(VERBOSE, "Found input containers "
                << Containers(ct) <<std::endl);
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_input_particles());
        IMP_LOG(VERBOSE, "Found input particles " << Particles(pt) <<std::endl);
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
    }
  }

  template <class It>
  void build_outputs_graph(It b, It e,
                           DependencyGraph &dg,
                           DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      DGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing outputs for \""
              << (*c)->get_name()  << "\"" << std::endl);
      {
        ContainersTemp ct= filter((*c)->get_output_containers());
        IMP_LOG(VERBOSE, "Found output containers "
                << Containers(ct) <<std::endl);
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          add_edge(dg, rv, cv);
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_output_particles());
        IMP_LOG(VERBOSE, "Found output particles "
                << Particles(pt) <<std::endl);
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
           add_edge(dg, rv, cv);
        }
      }
    }
  }
}

DependencyGraph
get_dependency_graph(const ScoreStatesTemp &ss,
                     const RestraintsTemp &rs) {
  IMP_LOG(VERBOSE, "Making dependency graph on " << rs.size()
          << " restraints " << ss.size() << " score states."
          << std::endl);
  DGIndex index;
  DependencyGraph ret(ss.size()+rs.size());
  boost::property_map<DependencyGraph, boost::vertex_name_t>::type
    vm = boost::get(boost::vertex_name, ret);
  for (unsigned int i=0; i< ss.size(); ++i) {
    vm[i]= ss[i];
    index[ss[i]]=i;
  }
  for (unsigned int i=0; i< rs.size(); ++i) {
    vm[i+ss.size()]= rs[i];
    index[rs[i]]=i+ss.size();
  }
  build_outputs_graph(ss.begin(), ss.end(), ret, index);
  build_inputs_graph(ss.begin(), ss.end(), ret, index);
  build_inputs_graph(rs.begin(), rs.end(), ret, index);
  return ret;
}


class ScoreDependencies: public boost::default_dfs_visitor {
  boost::dynamic_bitset<> &bs_;
  const std::map<Object*, int> &ssindex_;
  DGConstVertexMap vm_;
public:
  ScoreDependencies(boost::dynamic_bitset<> &bs,
                    const std::map<Object*, int> &ssindex,
                    DGConstVertexMap vm): bs_(bs), ssindex_(ssindex),
                                          vm_(vm) {}
  template <class G>
  void discover_vertex(DGTraits::vertex_descriptor u,
                       const G& g) {
    Object *o= vm_[u];
    std::map<Object*, int>::const_iterator it= ssindex_.find(o);
    if (it != ssindex_.end()) {
      bs_.set(it->second);
    }
  }
};

namespace {
  void order_score_states(const DependencyGraph &dg,
                          ScoreStatesTemp &out) {
    std::vector<DGTraits::vertex_descriptor> sorted;
    DGConstVertexMap om= boost::get(boost::vertex_name, dg);
    ScoreStatesTemp ret;
    try {
      boost::topological_sort(dg, std::back_inserter(sorted));
    } catch (...) {
      TextOutput out=create_temporary_file();
      internal::show_as_graphviz(dg, out);
      IMP_THROW("Topological sort failed, probably due to loops in "
                << " dependency graph. See \"" << out.get_name() << "\"",
                ValueException);
    }
    for (int i=sorted.size()-1; i > -1; --i) {
      Object *o= om[sorted[i]];
      ScoreState *s=dynamic_cast<ScoreState*>(o);
      if (s) {
        out.push_back(s);
      }
    }
  }


  void
  compute_restraint_dependencies(const DependencyGraph &dg,
                                 const RestraintsTemp &ordered_restraints,
                                 const ScoreStatesTemp &ordered_score_states,
                                 std::vector<boost::dynamic_bitset<> >&bs) {
    std::map<Object *, int> ssindex;
    for (unsigned int i=0; i < ordered_score_states.size(); ++i) {
      ssindex[ordered_score_states[i]]=i;
    }
    bs.resize(ordered_restraints.size(),
              boost::dynamic_bitset<>(ordered_score_states.size(), false));
    DGConstVertexMap om= boost::get(boost::vertex_name, dg);
    boost::vector_property_map<int> color(boost::num_vertices(dg));
    for (std::pair<DGTraits::vertex_iterator,
           DGTraits::vertex_iterator> be= boost::vertices(dg);
         be.first != be.second; ++be.first) {
      Object *o= om[*be.first];
      for (unsigned int i=0; i< ordered_restraints.size(); ++i) {
        if (o== ordered_restraints[i]) {
          boost::depth_first_visit(boost::make_reverse_graph(dg), *be.first,
                                   ScoreDependencies(bs[i], ssindex, om),
                                   color);
        }
      }
    }
  }
}


void Model::reset_dependencies() {
  restraint_index_.clear();
  restraint_dependencies_.clear();
  restraint_weights_.clear();
  ordered_restraints_.clear();
  ordered_score_states_.clear();
}

void Model::compute_dependencies() const {
  IMP_LOG(VERBOSE, "Ordering score states. Input list is: ");
  boost::tie(ordered_restraints_,
             restraint_weights_) = get_restraints(restraints_begin(),
                                                  restraints_end());
  for (unsigned int i=0; i< ordered_restraints_.size(); ++i) {
    restraint_index_[ordered_restraints_[i]]= i;
  }
  ScoreStates score_states= access_score_states();
  IMP_LOG(VERBOSE, "Making dependency graph on " << restraint_weights_.size()
          << " restraints " << score_states.size() << " score states "
          << " and " << get_number_of_particles()
          << " particles." << std::endl);
  DependencyGraph dg= get_dependency_graph(score_states,
                                           ordered_restraints_);

  order_score_states(dg, ordered_score_states_);
  compute_restraint_dependencies(dg, ordered_restraints_,
                                 ordered_score_states_,
                                 restraint_dependencies_);
  IMP_LOG(VERBOSE, "Ordered score states are "
          << ScoreStates(ordered_score_states_) << std::endl);
}





IMP_END_NAMESPACE
