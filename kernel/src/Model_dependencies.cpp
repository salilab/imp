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
namespace {
 typedef boost::graph_traits<DependencyGraph> MDGTraits;
  typedef MDGTraits::vertex_descriptor MDGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type MDGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  MDGConstVertexMap;
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

  struct cycle_detector : public boost::default_dfs_visitor {
    std::vector<MDGVertex> cycle_;
    template <class DGVertex>
    void start_vertex(DGVertex v, const DependencyGraph&) {
      cycle_.push_back(v);
    }
    template <class DGVertex>
    void finish_vertex(DGVertex v, const DependencyGraph&) {
      IMP_USAGE_CHECK(cycle_.back()==v, "They don't match");
        cycle_.pop_back();
    }
    template <class ED>
    void back_edge(ED e, const DependencyGraph&g) {
      MDGVertex t= boost::target(e, g);
      std::vector<MDGVertex>::iterator it
        = std::find(cycle_.begin(), cycle_.end(), t);
      IMP_USAGE_CHECK(it != cycle_.end(),
                      "The vertex is not there. Conceptual bug.");
      cycle_.erase(cycle_.begin(), it);
      cycle_.push_back(t);
      throw cycle_;
    }
  };

namespace {

  std::vector<MDGVertex> get_cycle(const DependencyGraph &g) {
    cycle_detector vis;
    try {
      boost::vector_property_map<int> color(boost::num_vertices(g));
      boost::depth_first_search(g, boost::visitor(vis).color_map(color));
    } catch (std::vector<MDGVertex> cycle) {
      return cycle;
    }
    return std::vector<MDGVertex>();
  }

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
      std::vector<MDGVertex> cycle= get_cycle(dg);
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
  Floats weights;
  boost::tie(ordered_restraints_,
             weights)
    = get_restraints_and_weights(restraints_begin(),
                                 restraints_end());
  for (unsigned int i=0; i< ordered_restraints_.size(); ++i) {
    ordered_restraints_[i]->model_weight_= weights[i];
  }
  RestraintsTemp all_restraints=ordered_restraints_;
  for (unsigned int i=0; i< temp_restraints_.size(); ++i) {
    RestraintsTemp curr;
    Floats curw;
    boost::tie(curr, curw)
      = get_restraints_and_weights(RestraintsTemp(1, temp_restraints_[i]));
    for (unsigned int i=0; i< curr.size(); ++i) {
      curr[i]->model_weight_= curw[i];
    }
    all_restraints.insert(all_restraints.end(), curr.begin(), curr.end());
  }
  ScoreStates score_states= access_score_states();
  IMP_LOG(VERBOSE, "Making dependency graph on " << weights.size()
          << " restraints " << score_states.size() << " score states "
          << " and " << get_number_of_particles()
          << " particles." << std::endl);
  DependencyGraph dg= get_dependency_graph(score_states,
                                           all_restraints);
  //internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  order_score_states(dg, ordered_score_states_);
  compute_restraint_dependencies(dg, all_restraints,
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
  Floats max_scores(restraints.size());
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
