/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
#include "IMP/compatibility/map.h"
#include "IMP/dependency_graph.h"
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <numeric>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
//#include <boost/graph/lookup_edge.hpp>
#include <IMP/compatibility/vector_property_map.h>

IMP_BEGIN_NAMESPACE
typedef ::boost::graph_traits<DependencyGraph> MDGTraits;
typedef MDGTraits::vertex_descriptor VD;
typedef MDGTraits::edge_descriptor ED;
typedef compatibility::map<Object*, VD> DGIndex;
typedef boost::property_map<DependencyGraph, boost::vertex_name_t>::const_type
MDGConstVertexMap;

  struct cycle_detector : public boost::default_dfs_visitor {
    std::vector<VD> cycle_;
    template <class VD>
    void start_vertex(VD v, const DependencyGraph&) {
      cycle_.push_back(v);
    }
    template <class VD>
    void finish_vertex(VD v, const DependencyGraph&) {
      IMP_USAGE_CHECK(cycle_.back()==v, "They don't match");
        cycle_.pop_back();
    }
    template <class ED>
    void back_edge(ED e, const DependencyGraph&g) {
      VD t= boost::target(e, g);
      std::vector<VD>::iterator it= std::find(cycle_.begin(), cycle_.end(), t);
      IMP_USAGE_CHECK(it != cycle_.end(),
                      "The vertex is not there. Conceptual bug.");
      cycle_.erase(cycle_.begin(), it);
      cycle_.push_back(t);
      throw cycle_;
    }
  };

namespace {

  std::vector<VD> get_cycle(const DependencyGraph &g) {
    cycle_detector vis;
    try {
      boost::vector_property_map<int> color(boost::num_vertices(g));
      boost::depth_first_search(g, boost::visitor(vis).color_map(color));
    } catch (std::vector<VD> cycle) {
      return cycle;
    }
    return std::vector<VD>();
  }

  template <class C>
  C filter(C c) {
    std::sort(c.begin(), c.end());
    c.erase(std::unique(c.begin(), c.end()), c.end());
    IMP_INTERNAL_CHECK(c.empty() || c[0] != NULL,
                       "NULL returned for dependencies.");
    return c;
  }

  bool get_has_edge(const DependencyGraph &graph,
                    MDGTraits::vertex_descriptor va,
                    MDGTraits::vertex_descriptor vb) {
    /*std::pair<MDGTraits::out_edge_iterator,
      MDGTraits::out_edge_iterator> edges= boost::out_edges(va, graph);
    for (; edges.first != edges.second;++edges.first) {
      if (boost::target(*edges.first, graph) == vb) return true;
    }
    return false;*/
    return boost::edge(va, vb, graph).second;
  }

  void add_edge(DependencyGraph &graph,
                MDGTraits::vertex_descriptor va,
                MDGTraits::vertex_descriptor vb) {
    if (get_has_edge(graph, va, vb)) return;
    IMP_INTERNAL_CHECK(va != vb, "Can't dependend on itself " << va);
    IMP_INTERNAL_CHECK(!get_has_edge(graph, va, vb),
                       "Already has edge between " << va << " and " << vb);
    boost::add_edge(va, vb, graph);
    IMP_INTERNAL_CHECK(get_has_edge(graph, va, vb),
                       "No has edge between " << va << " and " << vb);

  }

  MDGTraits::vertex_descriptor get_vertex(DependencyGraph &dg,
                                         DGIndex &dgi,
                                         Object *o) {
    DGIndex::const_iterator it=dgi.find(o);
    if (it==dgi.end()) {
      boost::property_map<DependencyGraph, boost::vertex_name_t>::type vm
        = boost::get(boost::vertex_name, dg);
      MDGTraits::vertex_descriptor v= boost::add_vertex(dg);
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
      MDGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing inputs for \""
        << (*c)->get_name() << "\" ");
      {
        ContainersTemp ct= filter((*c)->get_input_containers());
        if (!ct.empty()) {
          IMP_LOG(VERBOSE, ", containers are "
                  << Containers(ct));
                  }
        for (unsigned int j=0; j < ct.size(); ++j) {
          MDGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_input_particles());
        if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are " << Particles(pt));
          }
        for (unsigned int j=0; j < pt.size(); ++j) {
          MDGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
          if (!get_has_edge(dg, rv, cv)) {
            add_edge(dg, cv, rv);
          }
        }
      }
      IMP_LOG(VERBOSE, std::endl);
    }
  }

  template <class It>
  void build_outputs_graph(It b, It e,
                           DependencyGraph &dg,
                           DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      MDGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing outputs for \""
        << (*c)->get_name()  << "\"");
      {
        ContainersTemp ct= filter((*c)->get_output_containers());
        IMP_IF_LOG(VERBOSE) {
          if (!ct.empty()) {
            IMP_LOG(VERBOSE, ", containers are "
                    << Containers(ct));
          }
        }
        for (unsigned int j=0; j < ct.size(); ++j) {
          MDGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          add_edge(dg, rv, cv);
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_output_particles());
        if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are "
                  << Particles(pt));
                  }
        for (unsigned int j=0; j < pt.size(); ++j) {
          MDGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
           add_edge(dg, rv, cv);
        }
      }
      IMP_LOG(VERBOSE, std::endl);
    }
  }
  DependencyGraph
  get_dependency_graph(const ScoreStatesTemp &ss,
                       const RestraintsTemp &rs) {
    IMP_LOG(VERBOSE, "Making dependency graph on " << rs.size()
            << " restraints and " << ss.size() << " score states."
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
    std::vector<std::pair<Object*, Object*> > extra;
    if (!ss.empty()) {
      extra=ss[0]->get_model()->get_extra_dependency_edges();
    } else if (!rs.empty()) {
      extra=rs[0]->get_model()->get_extra_dependency_edges();
    }
    for (unsigned int i=0; i< extra.size(); ++i) {
      int va= index[extra[i].first];
      int vb= index[extra[i].second];
      boost::add_edge(va, vb, ret);
    }
    return ret;
  }
}

DependencyGraph
get_dependency_graph(const RestraintsTemp &irs) {
  IMP_FUNCTION_LOG;
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
    IMP_HASHABLE_INLINE(Connections,
                        return boost::hash_range(in.begin(),
                                                 in.end())
                        + boost::hash_range(out.begin(),
                                            out.end()));
    IMP_COMPARISONS(Connections);
  };
  //IMP_VALUES(Connections, ConnectionsList);
#if !defined(__clang__)
  inline std::size_t hash_value(const Connections &t) {
    return t.__hash__();
  }
#endif
}

DependencyGraph
get_pruned_dependency_graph(const RestraintsTemp &irs) {
  IMP_FUNCTION_LOG;
  DependencyGraph full= get_dependency_graph(irs);
  typedef boost::graph_traits<DependencyGraph> T;
  bool changed=true;
  while (changed) {
    changed=false;
    IMP_LOG(VERBOSE, "Searching for vertices to prune" << std::endl);
    compatibility::set<Connections> connections;
    for (unsigned int i=0; i< boost::num_vertices(full); ++i) {
      Connections c(i, full);
      if (connections.find(c) != connections.end()) {
        boost::property_map<DependencyGraph, boost::vertex_name_t>::type
          vm = boost::get(boost::vertex_name, full);
        IMP_LOG(VERBOSE, "Removing object " << vm[i]->get_name() << std::endl);
        for (unsigned int j=0; j< c.in.size(); ++j) {
          for (unsigned int k=0; k< c.out.size(); ++k) {
            //if (!boost::lookup_edge(c.in[j], c.out[k], full).second) {
              // why am I doing this anyway?
              //boost::add_edge(c.in[j], c.out[k], full);
            //}
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

DependencyGraph
get_pruned_dependency_graph(Model *m) {
  RestraintsTemp rt= get_restraints(m->get_root_restraint_set());
  return get_pruned_dependency_graph(rt);
}


class ScoreDependencies: public boost::default_dfs_visitor {
  Ints &bs_;
  const compatibility::map<Object*, int> &ssindex_;
  MDGConstVertexMap vm_;
public:
  ScoreDependencies(Ints &bs,
                    const compatibility::map<Object*, int> &ssindex,
                    MDGConstVertexMap vm): bs_(bs), ssindex_(ssindex),
                                          vm_(vm) {}
  template <class G>
  void discover_vertex(MDGTraits::vertex_descriptor u,
                       const G&) {
    Object *o= vm_[u];
    //std::cout << "visiting " << o->get_name() << std::endl;
    compatibility::map<Object*, int>::const_iterator it= ssindex_.find(o);
    if (it != ssindex_.end()) {
      //std::cout << "setting " << it->second << std::endl;
      bs_.push_back(it->second);
    } else {
      IMP_INTERNAL_CHECK(!dynamic_cast<ScoreState*>(o),
                         "Score state jot in index");
    }
  }
};

namespace {
#pragma GCC diagnostic ignored "-Wunused-parameter"
  void order_score_states(const DependencyGraph &dg,
                          ScoreStatesTemp &out) {
    std::vector<MDGTraits::vertex_descriptor> sorted;
    MDGConstVertexMap om= boost::get(boost::vertex_name, dg);
    ScoreStatesTemp ret;
    try {
      boost::topological_sort(dg, std::back_inserter(sorted));
    } catch (...) {
      TextOutput out=create_temporary_file();
      internal::show_as_graphviz(dg, out);
      std::vector<VD> cycle= get_cycle(dg);
      std::ostringstream oss;
      for (unsigned int i=0; i< cycle.size(); ++i) {
        oss << om[cycle[i]]->get_name() << " -- ";
      }
      IMP_THROW("Topological sort failed, probably due to loops in "
                << " dependency graph. See \"" << out.get_name() << "\""
                << " The cycle is " << oss.str(),
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
  //#pragma GCC diagnostic warn "-Wunused-parameter"

  compatibility::map<Object*, int> get_index(const DependencyGraph &dg) {
    compatibility::map<Object*, int> ret;
    MDGConstVertexMap om= boost::get(boost::vertex_name, dg);
    for (std::pair<MDGTraits::vertex_iterator,
           MDGTraits::vertex_iterator> be= boost::vertices(dg);
         be.first != be.second; ++be.first) {
      Object *o= om[*be.first];
      ret[o]= *be.first;
    }
    return ret;
  }


  void
  compute_restraint_dependencies(const DependencyGraph &dg,
                                 const RestraintsTemp &ordered_restraints,
                                 const ScoreStatesTemp &ordered_score_states) {
    compatibility::map<Object *, int> ssindex;
    for (unsigned int i=0; i < ordered_score_states.size(); ++i) {
      ssindex[ordered_score_states[i]]=i;
    }
    compatibility::map<Object*, int> index= get_index(dg);
    MDGConstVertexMap om= boost::get(boost::vertex_name, dg);
    for (unsigned int i=0; i< ordered_restraints.size(); ++i) {
      // make sure it is in the loop so it gets reset
      boost::vector_property_map<int> color(boost::num_vertices(dg));
      /*std::cout << "Finding dependencies for "
        << ordered_restraints[i]->get_name()
        << std::endl;*/
      IMP_USAGE_CHECK(ordered_restraints[i]
                      == om[index.find(ordered_restraints[i])->second],
                      "Restraints and vertices don't match");
      Ints cur;
      boost::depth_first_visit(boost::make_reverse_graph(dg),
                               index.find(ordered_restraints[i])->second,
                               ScoreDependencies(cur, ssindex, om),
                               color);
      std::sort(cur.begin(), cur.end());
      cur.erase(std::unique(cur.begin(), cur.end()), cur.end());
      ordered_restraints[i]->model_dependencies_=cur;
    }
  }
}


void Model::reset_dependencies() {
  ordered_restraints_.clear();
  ordered_score_states_.clear();
  first_call_=true;
}

void Model::compute_dependencies() const {
  IMP_OBJECT_LOG;
  IMP_LOG(VERBOSE, "Ordering score states. Input list is: ");
  std::vector<double> weights;
  boost::tie(ordered_restraints_,
             weights)
    = get_restraints_and_weights(restraints_begin(),
                                 restraints_end());
  for (unsigned int i=0; i< ordered_restraints_.size(); ++i) {
    ordered_restraints_[i]->model_weight_= weights[i];
  }
  ScoreStates score_states= access_score_states();
  IMP_LOG(VERBOSE, "Making dependency graph on " << weights.size()
          << " restraints " << score_states.size() << " score states "
          << " and " << get_number_of_particles()
          << " particles." << std::endl);
  DependencyGraph dg= get_dependency_graph(score_states,
                                           ordered_restraints_);
  //internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  order_score_states(dg, ordered_score_states_);
  compute_restraint_dependencies(dg, ordered_restraints_,
                                 ordered_score_states_);
  IMP_LOG(VERBOSE, "Ordered score states are "
          << ScoreStates(ordered_score_states_) << std::endl);
}


ScoreStatesTemp
Model::get_score_states(const RestraintsTemp &restraints) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i= 0; i< restraints.size(); ++i) {
      IMP_CHECK_CODE(Restraint *r=restraints[i]);
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "Cannot pass restraint sets to get_score_states()");
    }
  }
  Ints bs;
  for (unsigned int i=0; i< restraints.size(); ++i) {
    IMP_USAGE_CHECK(restraints[i]->get_is_part_of_model(),
                    "Restraint must be added to model: "
                    << restraints[i]->get_name());
    // weight 0
    if (restraints[i]->model_weight_==0) {
      IMP_LOG(VERBOSE, "Restraint " << restraints[i]->get_name()
              << " has weight 0" << std::endl);
    }
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, restraints[i]->get_name() << " depends on ");
      for (unsigned int j=0; j< restraints[i]->model_dependencies_.size();
           ++j) {
        IMP_LOG(VERBOSE,
                ordered_score_states_[restraints[i]->model_dependencies_[j]]
                ->get_name()
                << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    bs.insert(bs.end(), restraints[i]->model_dependencies_.begin(),
              restraints[i]->model_dependencies_.end());
  }
  std::sort(bs.begin(), bs.end());
  bs.erase(std::unique(bs.begin(), bs.end()), bs.end());
  ScoreStatesTemp ss(bs.size());
  for (unsigned int i=0; i< bs.size(); ++i) {
    ss[i]=ordered_score_states_[bs[i]];
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
                     ordered_score_states_,
                          calc_derivs, false, false);
  first_call_=false;
  return std::accumulate(ret.begin(), ret.end(), 0.0);
}

Floats Model::do_external_evaluate(const RestraintsTemp &restraints,
                                   bool calc_derivs,
                                   bool if_good, bool if_max, double max) {
  IMP_CHECK_OBJECT(this);
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< restraints.size(); ++i) {
      IMP_CHECK_CODE(Restraint *r= restraints[i]);
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
    }
  }
  ScoreStatesTemp ss= get_score_states(restraints);
  std::vector<double> max_scores(restraints.size());
  /*for (unsigned int i=0; i< max_scores.size(); ++i) {
    if (max_scores_.find(restraints[i]) == max_scores_.end()) {
      max_scores[i]= std::numeric_limits<double>::max();
    } else {
      max_scores[i]= max_scores_.find(restraints[i])->second;
    }
    }*/
  Floats ret= do_evaluate(restraints,
                          ss, calc_derivs, if_good, if_max, max);
  IMP_INTERNAL_CHECK(ret.size()== restraints.size(),
                     "The number of scores doesn't match the number of"
                     << " restraints: " << ret.size()
                     << " vs " << restraints.size());
  return ret;
}

Floats Model::evaluate( RestraintsTemp restraints,
                       bool calc_derivs)
{
  return do_external_evaluate(restraints, calc_derivs, false, false);
}

Floats Model::evaluate_if_below( RestraintsTemp restraints,
                                bool calc_derivs,
                                double max)
{
  return do_external_evaluate(restraints, calc_derivs, true, false,
                              max);
}

Floats Model::evaluate_if_good( RestraintsTemp restraints,
                                bool calc_derivs)
{
  return do_external_evaluate(restraints, calc_derivs, true, false,
                              get_maximum_score());
}


void Model::add_dependency_edge(ScoreState *from, ScoreState *to) {
  extra_edges_.push_back(std::pair<Object*, Object*>(from, to));
}

IMP_END_NAMESPACE
