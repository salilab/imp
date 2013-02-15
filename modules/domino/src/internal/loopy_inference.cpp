/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/internal/inference_utility.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/dynamic_bitset.hpp>

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

#if 0
/** This should be sped up by tracking which neighbor states are used
    to keep alive each state in a node and then removing the
    node when these lists are empty. This would avoid rechecking
    filters and make it purely a matter of pushing ints around.
*/

namespace {
  void load_node_data(const SubsetGraph &sg,
                      const AssignmentsTable *sst,
                      base::Vector<Assignments> &data) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
    data.resize(boost::num_vertices(sg));
    for (unsigned int i=0; i< data.size(); ++i) {
      data[i]= get_node_data(subset_map[i], sst);
    }
  }
  void load_edge_data(const SubsetGraph &sg,
                      const SubsetFilterTables &sft,
                      base::Vector<EdgeDatas> &data) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
    data.resize(boost::num_vertices(sg));
    for (unsigned int i=0; i< data.size(); ++i) {
      typedef boost::graph_traits<SubsetGraph>::adjacency_iterator
        NeighborIterator;
      for (std::pair<NeighborIterator, NeighborIterator> be
             = boost::adjacent_vertices(i, sg);
           be.first != be.second; ++be.first) {
        if (*be.first < i) {
          data[i].push_back(get_edge_data(subset_map[i],
                                          subset_map[*be.first],
                                          sft));
        }
      }
    }
  }

  bool loopy_filter_edge(const Subset &s0,
                         const Subset &s1,
                         Assignments &nd0,
                         const Assignments &nd1,
                         const EdgeData &ed) {
    Ints ii0= get_index(s0, ed.intersection_subset);
    Ints ii1= get_index(s1, ed.intersection_subset);
    Ints ui0= get_index(ed.union_subset, s0);
    Ints ui1= get_index(ed.union_subset, s1);
    Ints uii= get_index(ed.union_subset, ed.intersection_subset);
    Ints to_erase;
    for (unsigned int i=0; i< nd0.size(); ++i) {
      bool ok=false;
      for (unsigned int j=0; j< nd1.size(); ++j) {
        if (get_are_equal(nd0[i], ii0,
                          nd1[j], ii1)) {
          Assignment ss= get_merged_assignment(ed.union_subset,
                                                  nd0[i], ui0,
                                                  nd1[j], ui1);
          bool filter_ok=true;
          for (unsigned int i=0; i< ed.filters.size(); ++i) {
            if (ed.filters[i]->get_is_ok(ss)) {
            } else {
              filter_ok=false;
              break;
            }
          }
          if (filter_ok) {
            ok=true;
            break;
          }
        }
      }
      if (!ok) {
        to_erase.push_back(i);
      }
    }
    for (int i=to_erase.size()-1; i>=0; --i) {
      IMP_LOG_VERBOSE( "Erasing " << nd0[to_erase[i]]
              << " from " << s0 << std::endl);
      nd0.erase(nd0.begin()+to_erase[i]);
    }
    return !to_erase.empty();
  }

  bool filter_pass(const SubsetGraph &sg,
                   const base::Vector<EdgeDatas> &eds,
                   base::Vector<Assignments> &data) {
    bool changed=false;
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
    for (unsigned int i=0; i< data.size(); ++i) {
      int j=0;
      typedef boost::graph_traits<SubsetGraph>::adjacency_iterator
        NeighborIterator;
      for (std::pair<NeighborIterator, NeighborIterator> be
             = boost::adjacent_vertices(i, sg);
           be.first != be.second; ++be.first) {
        if (*be.first < i) {
          changed |= loopy_filter_edge(subset_map[i],
                                       subset_map[*be.first],
                                       data[i], data[*be.first],
                                       eds[i][j]);
          changed |= loopy_filter_edge(subset_map[*be.first],
                                       subset_map[i],
                                       data[*be.first], data[i],
                                       eds[i][j]);
          ++j;
        }
      }
    }
    return changed;
  }


  void write(boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
             subset_map,
             const base::Vector<Assignments> &nds,
             std::string name ) {
    IMP_LOG_TERSE( name << " states are:\n");
    for (unsigned int i=0; i< nds.size(); ++i) {
      IMP_LOG_TERSE( subset_map[i] << ": ");
      for (unsigned int j=0; j< nds[i].size(); ++j) {
        IMP_LOG_TERSE( nds[i][j] << " ");
      }
      IMP_LOG_TERSE( std::endl);
    }
  }

}
Assignments loopy_get_best_conformations(const SubsetGraph &sg,
                                          const Subset&,
                                          const SubsetFilterTables &filters,
                                          const AssignmentsTable *sst,
                                          unsigned int max) {
  IMP_USAGE_CHECK(boost::num_vertices(sg) >0, "Must have a non-empty graph");
  base::Vector<Assignments> nds;
  load_node_data(sg, sst, nds);
  base::Vector<EdgeDatas> eds;
  load_edge_data(sg, filters, eds);
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
    subset_map= boost::get(boost::vertex_name, sg);
  IMP_IF_LOG(TERSE) {
    write(subset_map, nds, "Before filtering");
  }
  while (filter_pass(sg, eds, nds)) {
    IMP_IF_LOG(TERSE) {
      write(subset_map, nds, "Now");
    }
  }
  IMP_IF_LOG(TERSE) {
    write(subset_map, nds, "After filtering");
  }
  Assignments curd=nds[0];
  Subset s= subset_map[0];
  for (unsigned int i=1; i< nds.size(); ++i) {
    Subset si= subset_map[i];
    EdgeData ed= get_edge_data(s, si, filters);
    curd= get_union(s, si, curd, nds[i], ed, max);
    s= ed.union_subset;
  }
  return curd;
}




/*
namespace {
  struct FastAssignments: public Assignments {
    boost::dynamic_bitset<> alive;
    FastAssignments(const Assignments &nd): Assignments(nd) {
      alive.resize(nd.assignments.size(),true);
    }
    FastAssignments(){}
  };

  struct FastEdgeData {
    base::Vector<Ints> support;
  };
  typedef base::Vector<FastEdgeData> FastEdgeDatas;

  void fast_load_node_data(const SubsetGraph &sg,
                           const AssignmentsTable *sst,
               base::Vector<FastAssignments> &data) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
    data.resize(boost::num_vertices(sg));
    for (unsigned int i=0; i< data.size(); ++i) {
      data[i]= FastAssignments(get_node_data(subset_map[i], sst));
    }
  }
  void fast_load_edge_data(const SubsetGraph &sg,
                           const SubsetFilterTables &sft,
                 const base::Vector<FastAssignments> &nds,
                           base::Vector<FastEdgeDatas> &data) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
    data.resize(boost::num_vertices(sg));
    for (unsigned int i=0; i< data.size(); ++i) {
      for (std::pair<NeighborIterator, NeighborIterator> be
             = boost::adjacent_vertices(i, sg);
           be.first != be.second; ++be.first) {
        Subset s0= subset_map[i];
        Subset s1= subset_map[*be.first];
        EdgeData ed= get_edge_data(s0, s1,
                                   sft);
        data[i].push_back(FastEdgeData());
        data[i].back().support.resize(nds[i].assignments.size());

        Ints ii0= get_index(s0, ed.intersection_subset);
        Ints ii1= get_index(s1, ed.intersection_subset);
        Ints ui0= get_index(ed.union_subset, s0);
        Ints ui1= get_index(ed.union_subset, s1);
        Ints uii= get_index(ed.union_subset, ed.intersection_subset);
        for (unsigned int j=0; j< nds[i].assignments.size(); ++j) {
          for (unsigned int k=0; k< nds[*be.first].assignments.size(); ++k) {
            if (get_are_equal(nds[i].assignments[j], ii0,
                              nds[*be.first].assignments[k], ii1)) {
              Assignment ss= get_merged_assignment(ed.union_subset,
                                            nds[i].assignments[j], ui0,
                                      nds[*be.first].assignments[k], ui1);
              bool filter_ok=true;
              for (unsigned int l=0; l< ed.filters.size(); ++l) {
                if (ed.filters[l]->get_is_ok(ss)) {
                } else {
                  filter_ok=false;
                  break;
                }
              }
              if (filter_ok) {
                data[i].back().support[j].push_back(k);
                break;
              }
            }
          }
        }
      }
    }
  }

  bool fast_loopy_filter_edge(FastAssignments &nd0,
                              const FastAssignments &nd1,
                              const FastEdgeData &ed) {
    bool change=false;
    for (unsigned int i=0; i< nd0.assignments.size(); ++i) {
      if (!nd0.alive[i]) continue;
      bool ok=false;
      for (unsigned int j=0; j< ed.support[i].size(); ++j) {
        if (nd1.alive[ed.support[i][j]]) {
          ok=true;
        }
      }
      if (!ok) {
        nd0.alive[i]=false;
        change=true;
      }
    }
    return change;
  }

  bool fast_filter_pass(const SubsetGraph &sg,
                        const base::Vector<FastEdgeDatas> &eds,
                        base::Vector<FastAssignments> &data) {
    bool changed=false;
    for (unsigned int i=0; i< data.size(); ++i) {
      int j=0;
      for (std::pair<NeighborIterator, NeighborIterator> be
             = boost::adjacent_vertices(i, sg);
           be.first != be.second; ++be.first) {
        changed |= fast_loopy_filter_edge(data[i], data[*be.first],
                                     eds[i][j]);
        ++j;
      }
    }
    return changed;
  }


  void fast_collapse(FastAssignments &nd) {
    for (int i=nd.assignments.size()-1; i >=0; --i) {
      if (!nd.alive[i]) {
        nd.assignments.erase(nd.subset_states.begin()+i);
      }
    }
  }

  void fast_write(boost::property_map< SubsetGraph,
                                       boost::vertex_name_t>::const_type
                  subset_map,
                  const base::Vector<FastAssignments> &nds,
                  std::string name) {
    IMP_LOG_TERSE( name << " states are \n");
    for (unsigned int i=0; i< nds.size(); ++i) {
      IMP_LOG_TERSE( subset_map[i] << ": ");
      for (unsigned int j=0; j< nds[i].assignments.size(); ++j) {
        if (nds[i].alive[j]) {
          IMP_LOG_TERSE( nds[i].assignments[j] << " ");
        }
      }
      IMP_LOG_TERSE( std::endl);
    }
  }
}

namespace {
Assignments fast_loopy_get_best_conformations(const SubsetGraph &sg,
                                               const Subset& all_particles,
                                         const SubsetFilterTables &filters,
                                               const AssignmentsTable *sst,
                                               unsigned int max) {
  IMP_USAGE_CHECK(boost::num_vertices(sg) >0, "Must have a non-empty graph");
  base::Vector<FastAssignments> nds;
  fast_load_node_data(sg, sst, nds);
  base::Vector<FastEdgeDatas > eds;
  fast_load_edge_data(sg, filters, nds, eds);
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
    subset_map= boost::get(boost::vertex_name, sg);
  IMP_IF_LOG(TERSE) {
    fast_write(subset_map, nds, "Before filtering");
  }
  while (fast_filter_pass(sg, eds, nds)) {
    IMP_IF_LOG(VERBOSE) {
      fast_write(subset_map, nds, "Now");
    }
  }
  IMP_IF_LOG(TERSE) {
    fast_write(subset_map, nds, "After filtering");
  }
  for (unsigned int i=1; i< nds.size(); ++i) {
    fast_collapse(nds[i]);
  }
  Assignments curd=nds[0];
  Subset s= subset_map[0];
  for (unsigned int i=1; i< nds.size(); ++i) {
    Subset si= subset_map[i];
    EdgeData ed= get_edge_data(s, si, filters);
    curd= get_union(s, si, curd, nds[i], ed, max);
    s= ed.union_subset;
  }
  return curd.assignments;
}
}*/

#endif
IMPDOMINO_END_INTERNAL_NAMESPACE
