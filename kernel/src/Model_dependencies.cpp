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
#include "IMP/internal/map.h"
#include <boost/timer.hpp>
#include <set>
#include <numeric>

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
typedef internal::Map<Object*, DGTraits::vertex_descriptor> DGIndex;
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
      /*IMP_LOG(VERBOSE, "Processing inputs for \""
        << (*c)->get_name() << "\" ");*/
      {
        ContainersTemp ct= filter((*c)->get_input_containers());
        /*if (!ct.empty()) {
          IMP_LOG(VERBOSE, ", containers are "
                  << Containers(ct));
                  }*/
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_input_particles());
        /*if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are " << Particles(pt));
          }*/
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
      //IMP_LOG(VERBOSE, std::endl);
    }
  }

  template <class It>
  void build_outputs_graph(It b, It e,
                           DependencyGraph &dg,
                           DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      DGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      /*IMP_LOG(VERBOSE, "Processing outputs for \""
        << (*c)->get_name()  << "\"");*/
      {
        ContainersTemp ct= filter((*c)->get_output_containers());
        /*if (!ct.empty()) {
          IMP_LOG(VERBOSE, ", containers are "
                  << Containers(ct));
                  }*/
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          add_edge(dg, rv, cv);
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_output_particles());
        /*if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are "
                  << Particles(pt));
                  }*/
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
           add_edge(dg, rv, cv);
        }
      }
      //IMP_LOG(VERBOSE, std::endl);
    }
  }
}

DependencyGraph
get_dependency_graph(const ScoreStatesTemp &ss,
                     const RestraintsTemp &rs) {
  IMP_LOG(VERBOSE, "Making dependency graph on " << rs.size()
          << " restraints."
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

DependencyGraph
get_dependency_graph(const RestraintsTemp &irs) {
  if (irs.empty()) return DependencyGraph();
  RestraintsTemp rs= get_restraints(irs.begin(), irs.end());
  ScoreStatesTemp ss
    = irs[0]->get_model()->get_score_states(rs);
  return get_dependency_graph(ss, rs);
}


namespace {
  struct Connections {
    Ints in, out;
    Connections(int v, const DependencyGraph &g) {
      typedef boost::graph_traits<DependencyGraph> T;
      {
        typedef T::in_edge_iterator IIT;
        for (std::pair<IIT,IIT> be= boost::in_edges(v, g);
             be.first != be.second; ++be.first) {
          in.push_back(boost::source(*be.first, g));
        }
        std::sort(in.begin(), in.end());
      }
      {
        typedef T::out_edge_iterator OIT;
        for (std::pair<OIT,OIT> be= boost::out_edges(v, g);
             be.first != be.second; ++be.first) {
          out.push_back(boost::target(*be.first, g));
        }
        std::sort(out.begin(), out.end());
      }
      IMP_INTERNAL_CHECK(*this == *this, "Not equal");
    }
    int compare(const Connections &o) const {
      if (in.size() < o.in.size()) return -1;
      else if (in.size() > o.in.size()) return 1;
      else if (out.size() < o.out.size()) return -1;
      else if (out.size() > o.out.size()) return 1;
      else {
        for (unsigned int i=0; i< in.size(); ++i) {
          if (in[i] < o.in[i]) return -1;
          else if (in[i] > o.in[i]) return 1;
        }
        for (unsigned int i=0; i< out.size(); ++i) {
          if (out[i] < o.out[i]) return -1;
          else if (out[i] > o.out[i]) return 1;
        }
        return 0;
      }
    }
    typedef Connections This;
    IMP_COMPARISONS;
  };
}

DependencyGraph
get_pruned_dependency_graph(const RestraintsTemp &irs) {
  DependencyGraph full= get_dependency_graph(irs);
  typedef boost::graph_traits<DependencyGraph> T;
  bool changed=true;
  while (changed) {
    changed=false;
    IMP_LOG(VERBOSE, "Searching for vertices to prune" << std::endl);
    std::set<Connections> connections;
    for (unsigned int i=0; i< boost::num_vertices(full); ++i) {
      Connections c(i, full);
      if (connections.find(c) != connections.end()) {
        boost::property_map<DependencyGraph, boost::vertex_name_t>::type
          vm = boost::get(boost::vertex_name, full);
        IMP_LOG(VERBOSE, "Removing object " << vm[i]->get_name() << std::endl);
        for (unsigned int j=0; j< c.in.size(); ++j) {
          for (unsigned int k=0; k< c.out.size(); ++k) {
            boost::add_edge(c.in[j], c.out[k], full);
          }
        }
        boost::clear_vertex(i, full);
        boost::remove_vertex(i, full);
        changed=true;
      } else {
        connections.insert(c);
      }
    }
  }
  return full;
}


class ScoreDependencies: public boost::default_dfs_visitor {
  boost::dynamic_bitset<> &bs_;
  const internal::Map<Object*, int> &ssindex_;
  DGConstVertexMap vm_;
public:
  ScoreDependencies(boost::dynamic_bitset<> &bs,
                    const internal::Map<Object*, int> &ssindex,
                    DGConstVertexMap vm): bs_(bs), ssindex_(ssindex),
                                          vm_(vm) {}
  template <class G>
  void discover_vertex(DGTraits::vertex_descriptor u,
                       const G& g) {
    Object *o= vm_[u];
    internal::Map<Object*, int>::const_iterator it= ssindex_.find(o);
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
    internal::Map<Object *, int> ssindex;
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
  first_call_=true;
}

void Model::compute_dependencies() const {
  IMP_LOG(VERBOSE, "Ordering score states. Input list is: ");
  boost::tie(ordered_restraints_,
             restraint_weights_)
    = get_restraints_and_weights(restraints_begin(),
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
  restraint_max_scores_.resize(ordered_restraints_.size());
  for (unsigned int i=0; i< ordered_restraints_.size(); ++i) {
    if (max_scores_.find(ordered_restraints_[i]) == max_scores_.end()) {
      restraint_max_scores_[i]= std::numeric_limits<double>::max();
    } else {
      restraint_max_scores_[i]
        = max_scores_.find(ordered_restraints_[i])->second;
    }
  }
  IMP_LOG(VERBOSE, "Ordered score states are "
          << ScoreStates(ordered_score_states_) << std::endl);
  IMP_INTERNAL_CHECK(restraint_dependencies_.size()
                     ==ordered_restraints_.size(),
                     "Dependencies do not match ordered restraints "
                     << restraint_dependencies_.size() << " "
                     << ordered_restraints_.size());
  IMP_INTERNAL_CHECK(restraint_index_.size() ==ordered_restraints_.size(),
                     "Indexes do not match ordered restraints "
                     << restraint_index_.size() << " "
                     << ordered_restraints_.size());
}


ScoreStatesTemp
Model::get_score_states(const RestraintsTemp &restraints) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i= 0; i< restraints.size(); ++i) {
      Restraint *r=restraints[i];
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "Cannot pass restraint sets to get_score_states()");
    }
  }
  boost::dynamic_bitset<> bs(ordered_score_states_.size(), false);
  for (unsigned int i=0; i< restraints.size(); ++i) {
    // weight 0
    if (restraint_index_.find(restraints[i])
        == restraint_index_.end()) continue;
    int index=restraint_index_.find(restraints[i])->second;
    bs|= restraint_dependencies_[index];
  }
  ScoreStatesTemp ss;
  for (unsigned int i=0; i< ordered_score_states_.size(); ++i) {
    if (bs[i]) ss.push_back(ordered_score_states_[i]);
  }
  return ss;
}


ScoreStatesTemp get_required_score_states(const RestraintsTemp &irs) {
  if (irs.empty()) return ScoreStatesTemp();
  RestraintsTemp rs= get_restraints(irs.begin(),
                                    irs.end());
  if (rs.empty()) return ScoreStatesTemp();
  return rs[0]->get_model()->get_score_states(rs);
}


double Model::evaluate(bool calc_derivs) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  has_good_score_=true;
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  Floats ret= do_evaluate(ordered_restraints_,
                     restraint_weights_,
                          restraint_max_scores_,
                     ordered_score_states_,
                     calc_derivs);
  first_call_=false;
  return std::accumulate(ret.begin(), ret.end(), 0.0);
}

Floats Model::evaluate( RestraintsTemp restraints,
                        std::vector<double> weights,
                       bool calc_derivs)
{
  IMP_CHECK_OBJECT(this);
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< restraints.size(); ++i) {
      Restraint *r= restraints[i];
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "Cannot pass RestraintSets to Model::evaluate().");
    }
  }
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< restraints.size(); ++i) {
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(restraints[i]),
                      "Cannot pass restraint sets to model to evaluate");
      IMP_USAGE_CHECK(restraint_index_.find(restraints[i])
                      != restraint_index_.end(),
                      "You must add restraints to model before "
                      << "asking it to evaluate them");
    }
  }
  ScoreStatesTemp ss= get_score_states(restraints);
  std::vector<double> max_scores(restraints.size());
  for (unsigned int i=0; i< max_scores.size(); ++i) {
    if (max_scores_.find(restraints[i]) == max_scores_.end()) {
      max_scores[i]= std::numeric_limits<double>::max();
    } else {
      max_scores[i]= max_scores_.find(restraints[i])->second;
    }
  }
  Floats ret= do_evaluate(restraints, weights, max_scores,
                     ss, calc_derivs);
  IMP_INTERNAL_CHECK(ret.size()== restraints.size(),
                     "The number of scores doesn't match the number of"
                     << " restraints: " << ret.size()
                     << " vs " << restraints.size());
  return ret;
}


IMP_END_NAMESPACE
