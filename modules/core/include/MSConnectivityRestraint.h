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


// Hopefully this would "fix" the problem with friend access
// in older versions of gcc
#ifdef __GNUC__
  #define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
  #if GCC_VERSION >= 40100
    #define IMPCORE_FRIEND_IS_OK
  #endif
#else
  // assume that other compilers are fine
  #define IMPCORE_FRIEND_IS_OK
#endif




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
  IMP::OwnerPointer<PairScore> ps_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  double eps_;
public:
  //! Use the given PairScore
  /** If sc is NULL, a ListSingletonContainer is created internally.
    eps is set to 0.1 by default.
   */
  MSConnectivityRestraint(PairScore* ps, double eps=0.1);
  /** @name Particles to be connected

    The following methods are used to manipulate the list of particles
    that are to be connected. Each particle should have all the
    attributes expected by the PairScore used.

    Ideally, one should pass a singleton container instead. These
    can only be used if none is passed.
   */
  /*@{*/
  size_t add_type(const Particles &ps);
  size_t add_composite(const Ints &components);
  size_t add_composite(const Ints &components, size_t parent);
  //void add_particle(Particle *p);
  //void add_particles(const Particles &ps);
  //void set_particles(const Particles &ps);
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

  Restraints create_current_decomposition() const;

  IMP_RESTRAINT(MSConnectivityRestraint);

#ifdef IMPCORE_FRIEND_IS_OK
  private:
#endif

  class ParticleMatrix
  {
  public:

    class ParticleData
    {
    public:
      ParticleData(Particle *p, size_t id)
          : particle_(p)
            , id_(id)
      {}

      Particle *get_particle() const
      {
        return particle_;
      }

      size_t get_id() const
      {
        return id_;
      }
    private:
      Particle *particle_;
      size_t id_;
    };

    ParticleMatrix(size_t number_of_classes)
        : protein_by_class_(number_of_classes)
         , min_distance_(std::numeric_limits<double>::max())
         , max_distance_(0)
         , current_id_(0)
    {}

    ParticleMatrix()
        : min_distance_(std::numeric_limits<double>::max())
         , max_distance_(0)
         , current_id_(0)
    {}

    void resize(size_t number_of_classes)
    {
      protein_by_class_.resize(number_of_classes);
    }

    size_t add_particle(Particle *p, size_t id);
    size_t add_type(const Particles &ps);
    void create_distance_matrix(const PairScore *ps);
    void clear_particles()
    {
      particles_.clear();
      for ( size_t i = 0; i < protein_by_class_.size(); ++i )
        protein_by_class_[i].clear();
    }
    size_t size() const
    {
      return particles_.size();
    }
    size_t get_number_of_classes() const
    {
      return protein_by_class_.size();
    }
    double get_distance(size_t p1, size_t p2) const
    {
      return dist_matrix_[p1*size() + p2];
    }
    std::vector<size_t> const &get_ordered_neighbors(size_t p) const
    {
      return order_[p];
    }
    ParticleData const &get_particle(size_t p) const
    {
      return particles_[p];
    }
    std::vector<size_t> const &get_all_proteins_in_class(
        size_t id) const
    {
      return protein_by_class_[id];
    }
    double max_distance() const
    {
      return max_distance_;
    }
    double min_distance() const
    {
      return min_distance_;
    }
  private:
    class DistCompare
    {
    public:
      DistCompare(size_t source, ParticleMatrix const &parent)
        : parent_(parent)
          , source_(source)
      {}

      bool operator()(size_t p1, size_t p2) const
      {
        return parent_.get_distance(source_, p1) <
          parent_.get_distance(source_, p2);
      }
    private:
      ParticleMatrix const &parent_;
      size_t source_;
    };

    std::vector<ParticleData> particles_;
    Floats dist_matrix_;
    std::vector< std::vector<size_t> > order_;
    std::vector< std::vector<size_t> > protein_by_class_;
    double min_distance_;
    double max_distance_;
    size_t current_id_;
  };

  class ExperimentalTree
  {
  public:
    ExperimentalTree()
      : root_(-1)
      , finalized_(false)
    {}

    void connect(size_t parent, size_t child);
    void finalize();
    size_t add_composite(const Ints &components);
    size_t add_composite(const Ints &components, size_t parent);

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
    void desc_to_label(const Ints &components,
                       Node::Label &label);

    std::vector<Node> nodes_;
    size_t root_;
    bool finalized_;
  };

  ParticleMatrix particle_matrix_;
  mutable ExperimentalTree tree_;

  friend class MSConnectivityScore;

};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MS_CONNECTIVITY_RESTRAINT_H */
