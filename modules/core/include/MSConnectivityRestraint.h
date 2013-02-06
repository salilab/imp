/**
 *  \file IMP/core/MSConnectivityRestraint.h
 *  \brief Mass Spec Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MS_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_MS_CONNECTIVITY_RESTRAINT_H

#include <vector>
#include <string>
#include <IMP/core/core_config.h>
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
    to restrain the connectivity of sets of
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
  /** If sc is nullptr, a ListSingletonContainer is created internally.
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
  unsigned int add_type(const ParticlesTemp &ps);
  unsigned int add_composite(const Ints &components);
  unsigned int add_composite(const Ints &components, unsigned int parent);
  //void add_particle(Particle *p);
  //void add_particles(const Particles &ps);
  //void set_particles(const Particles &ps);
  /*@}*/

  //! Return the set of pairs which are connected by the restraint
  /** This set of pairs reflects the current configuration at the time of
    the get_connected_pairs() call, not the set at the time of the last
    evaluate() call.
   */
  ParticlePairsTemp get_connected_pairs() const;

  //! Return the pair score used for scoring
  PairScore *get_pair_score() const {
    return ps_;
  }

  Restraints do_create_current_decomposition() const;

  double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) const;

  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(MSConnectivityRestraint);

#ifdef IMPCORE_FRIEND_IS_OK
  private:
#endif

  class ParticleMatrix
  {
  public:

    class ParticleData
    {
    public:
      ParticleData(Particle *p, unsigned int id)
          : particle_(p)
            , id_(id)
      {}

      Particle *get_particle() const
      {
        return particle_;
      }

      unsigned int get_id() const
      {
        return id_;
      }
    private:
      Particle *particle_;
      unsigned int id_;
    };

    ParticleMatrix(unsigned int number_of_classes)
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

    void resize(unsigned int number_of_classes)
    {
      protein_by_class_.resize(number_of_classes);
    }

    unsigned int add_particle(Particle *p, unsigned int id);
    unsigned int add_type(const ParticlesTemp &ps);
    void create_distance_matrix(const PairScore *ps);
    void clear_particles()
    {
      particles_.clear();
      for ( unsigned int i = 0; i < protein_by_class_.size(); ++i )
        protein_by_class_[i].clear();
    }
    unsigned int size() const
    {
      return particles_.size();
    }
    unsigned int get_number_of_classes() const
    {
      return protein_by_class_.size();
    }
    double get_distance(unsigned int p1, unsigned int p2) const
    {
      return dist_matrix_[p1*size() + p2];
    }
    Ints const &get_ordered_neighbors(unsigned int p) const
    {
      return order_[p];
    }
    ParticleData const &get_particle(unsigned int p) const
    {
      return particles_[p];
    }
    Ints const &get_all_proteins_in_class(
        unsigned int id) const
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
      DistCompare(unsigned int source, ParticleMatrix const &parent)
        : parent_(parent)
          , source_(source)
      {}

      bool operator()(unsigned int p1, unsigned int p2) const
      {
        return parent_.get_distance(source_, p1) <
          parent_.get_distance(source_, p2);
      }
    private:
      ParticleMatrix const &parent_;
      unsigned int source_;
    };

    base::Vector<ParticleData> particles_;
    Floats dist_matrix_;
    base::Vector< Ints > order_;
    base::Vector< Ints > protein_by_class_;
    double min_distance_;
    double max_distance_;
    unsigned int current_id_;
  };

  class ExperimentalTree
  {
  public:
    ExperimentalTree()
      : root_(-1)
      , finalized_(false)
    {}

    void connect(unsigned int parent, unsigned int child);
    void finalize();
    unsigned int add_composite(const Ints &components);
    unsigned int add_composite(const Ints &components, unsigned int parent);

    class Node
    {
      public:
        Node()
          : visited_(false)
        {}
        unsigned int get_number_of_parents() const
        {
          return parents_.size();
        }
        bool is_root() const
        {
          return get_number_of_parents() == 0;
        }
        unsigned int get_number_of_children() const
        {
          return children_.size();
        }
        bool is_leaf() const
        {
          return get_number_of_children() == 0;
        }
        unsigned int get_parent(unsigned int idx) const
        {
          return parents_[idx];
        }
        unsigned int get_child(unsigned int idx) const
        {
          return children_[idx];
        }
        typedef base::Vector< std::pair<unsigned int, int> > Label;
        const Label &get_label() const
        {
          return label_;
        }


        base::Vector<unsigned int> parents_;
        base::Vector<unsigned int> children_;
        Label label_;
        bool visited_;
    };

    bool find_cycle(unsigned int node_index);
    bool is_consistent(unsigned int node_index) const;

    const Node *get_node(unsigned int index) const
    {
      return &nodes_[index];
    }
    unsigned int get_number_of_nodes() const
    {
      return nodes_.size();
    }
    unsigned int get_root() const
    {
      return root_;
    }
    void desc_to_label(const Ints &components,
                       Node::Label &label);

    base::Vector<Node> nodes_;
    unsigned int root_;
    bool finalized_;
  };

  ParticleMatrix particle_matrix_;
  mutable ExperimentalTree tree_;

  friend class MSConnectivityScore;

};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MS_CONNECTIVITY_RESTRAINT_H */
