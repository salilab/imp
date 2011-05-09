/**
 *  \file MSConnectivityRestraint.h    \brief Mass Spec Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MS_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_MS_CONNECTIVITY_RESTRAINT_H

#include <vector>
#include <map>
#include <string>
#include "core_config.h"
#include "DistanceRestraint.h"

#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

class IMPCOREEXPORT ExperimentalTree
    : public IMP::Object
{
public:

  ExperimentalTree()
    : root_(-1)
    , number_of_proteins_(0)
    , finalized_(false)
  {}

  size_t add_node(const std::string &desc);
  void connect(size_t parent, size_t child);
  void finalize();

  IMP_OBJECT(ExperimentalTree);

private:

  friend class MSConnectivityScore;

  class Node
  {
  public:
    Node()
      : visited_(false)
    {}
    size_t get_number_of_parents() const
    {
      return parents_.size();
    }
    bool is_root() const
    {
      return get_number_of_parents() == 0;
    }
    size_t get_number_of_children() const
    {
      return children_.size();
    }
    bool is_leaf() const
    {
      return get_number_of_children() == 0;
    }
    size_t get_parent(size_t idx) const
    {
      return parents_[idx];
    }
    size_t get_child(size_t idx) const
    {
      return children_[idx];
    }
    typedef std::vector< std::pair<size_t, int> > Label;
    const Label &get_label() const
    {
      return label_;
    }


    friend class ExperimentalTree;

  private:
    std::vector<size_t> parents_;
    std::vector<size_t> children_;
    Label label_;
    bool visited_;
  };

  bool find_cycle(size_t node_index);
  bool is_consistent(size_t node_index) const;

  const Node *get_node(size_t index) const
  {
    return &nodes_[index];
  }
  size_t get_number_of_nodes() const
  {
    return nodes_.size();
  }
  size_t get_root() const
  {
    return root_;
  }
  size_t classify_protein(const std::string &desc) const;
  std::string const &class_name(size_t id) const
  {
    return id_to_protein_[id];
  }
  size_t get_number_of_classes() const
  {
    return number_of_proteins_;
  }

  void desc_to_label(const std::string &desc, Node::Label &label);
  size_t protein_to_id(const std::string &desc);

  typedef std::map<std::string, size_t> ProteinMap;
  ProteinMap protein_to_id_;
  std::vector<Node> nodes_;
  std::vector<std::string> id_to_protein_;
  size_t root_;
  size_t number_of_proteins_;
  bool finalized_;
};


//! Ensure that a set of particles remains connected with one another.
/** The restraint implements ambiguous connectivity. That is, it takes
    several particles including multiple copies and ensures that they remain
    connected, but allows how they are connected to change. If you wish
    to restraint the connectivity of sets of
    particles (i.e. each protein is represented using a set of balls)
    use an appropriate PairScore which calls a Refiner (such
    as ClosePairsPairScore).

    \verbinclude ms_connectivity_restraint.py

    More precisely, the restraint scores by computing the MST on the complete
    graph connecting all the particles. The edge weights are given by
    the value of the PairScore for the two endpoints of the edge.
 */
class IMPCOREEXPORT MSConnectivityRestraint : public Restraint
{
  IMP::internal::OwnerPointer<PairScore> ps_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  IMP::internal::OwnerPointer<ExperimentalTree> tree_;
  double eps_;
public:
  //! Use the given PairScore
  /** If sc is NULL, a ListSingletonContainer is created internally.
      eps is set to 0.1 by default.
   */
  MSConnectivityRestraint(PairScore* ps, ExperimentalTree *tree,
    double eps=0.1, SingletonContainer *sc=NULL);
  /** @name Particles to be connected

       The following methods are used to manipulate the list of particles
       that are to be connected. Each particle should have all the
       attributes expected by the PairScore used.

       Ideally, one should pass a singleton container instead. These
       can only be used if none is passed.
  */
  /*@{*/
  void add_particle(Particle *p);
  void add_particles(const Particles &ps);
  void set_particles(const Particles &ps);
  /*@}*/

  //! Return the set of pairs which are connected by the restraint
  /** This set of pairs reflects the current configuration at the time of
      the get_connected_pairs() call, not the set at the time of the last
      evaluate() call.
  */
  ParticlePairs get_connected_pairs() const;

  //! Return the pair score used for scoring
  PairScore *get_pair_score() const {
    return ps_;
  }

  Restraints get_instant_decomposition() const;

  IMP_RESTRAINT(MSConnectivityRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MS_CONNECTIVITY_RESTRAINT_H */
