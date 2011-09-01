/**
 *  \file domino/Assignment.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_CONTAINERS_H
#define IMPDOMINO_ASSIGNMENT_CONTAINERS_H

#include "domino_config.h"
#include "Assignment.h"
#include "subset_filters.h"
#ifdef IMP_DOMINO_USE_IMP_RMF
#include <IMP/rmf/HDF5Group.h>
#endif
#include <boost/shared_array.hpp>
#include <algorithm>
#include <IMP/compatibility/hash.h>
#include <IMP/compatibility/checked_vector.h>

#include <queue>
#include <IMP/random.h>
#include <boost/random.hpp>

IMPDOMINO_BEGIN_NAMESPACE


/** The base class for containers of assignments. Assignments are stored
    in these rather than as Assignments to help increase efficiency as
    well as provide flexibility as to how and where they are stored.
*/
class IMPDOMINOEXPORT AssignmentContainer: public Object {
 public:
  AssignmentContainer(std::string name="AssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const=0;
  virtual Assignment get_assignment(unsigned int i) const=0;
  virtual Assignments get_assignments(IntRange ir) const=0;
  virtual Assignments get_assignments() const=0;
  virtual void add_assignment(const Assignment& a)=0;
  virtual void add_assignments(const Assignments &as)=0;
  //! Get all the assignments for the ith particle
  virtual Ints get_particle_assignments(unsigned int i) const=0;

  virtual ~AssignmentContainer();
};


IMP_OBJECTS(AssignmentContainer, AssignmentContainers);

/** Store a set of assignments in a somewhat more compact form in memory
    than the ListAssignmentContainer.
 */
class IMPDOMINOEXPORT PackedAssignmentContainer: public AssignmentContainer {
  // store all as one vector
  Ints d_;
  int width_;
 public:
  PackedAssignmentContainer(std::string name="PackedAssignmentsContainer %1%");
  IMP_ASSIGNMENT_CONTAINER(PackedAssignmentContainer);
};


inline unsigned int
PackedAssignmentContainer::get_number_of_assignments() const {
  if (width_==-1) return 0;
  return d_.size()/width_;
}

inline Assignment
PackedAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  IMP_USAGE_CHECK(width_ >0, "Uninitualized PackedAssignmentContainer.");
  return Assignment(d_.begin()+i*width_,
                    d_.begin()+(i+1)*width_);
}

inline void PackedAssignmentContainer::add_assignment(const Assignment& a) {
  if (width_==-1) {
    width_=a.size();
  }
  IMP_USAGE_CHECK(static_cast<int>(a.size())== width_,
                  "Sizes don't match " << width_
                  << " vs " << a.size());
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< get_number_of_assignments(); ++i) {
      IMP_INTERNAL_CHECK(get_assignment(i) != a,
                         "Assignment " << a << " already here.");
    }
  }
  d_.insert(d_.end(), a.begin(), a.end());
}

/** Simple storage of a set of Assignments. Prefer PackedAssignmentContainer,
    I think.
 */
class IMPDOMINOEXPORT ListAssignmentContainer: public AssignmentContainer {
  // store all as one vector
  Assignments d_;
 public:
  ListAssignmentContainer(std::string name="ListAssignmentsContainer %1%");
  IMP_ASSIGNMENT_CONTAINER(ListAssignmentContainer);
};

inline unsigned int ListAssignmentContainer::get_number_of_assignments() const {
  return d_.size();
}

inline Assignment
ListAssignmentContainer::get_assignment(unsigned int i) const {
  return d_[i];
}

inline void ListAssignmentContainer::add_assignment(const Assignment& a) {
  d_.push_back(a);
}






/** Store a list of k assignments chosen from all of the ones added to this
    table. The states are chosen uniformly.
 */
class IMPDOMINOEXPORT SampleAssignmentContainer: public AssignmentContainer {
  // store all as one vector
  Ints d_;
  int width_;
  unsigned int k_;
  unsigned int i_;
  boost::uniform_real<double> select_;
  boost::uniform_int<> place_;
 public:
  SampleAssignmentContainer(unsigned int k,
                            std::string name="SampleAssignmentsContainer %1%");
  IMP_ASSIGNMENT_CONTAINER(SampleAssignmentContainer);
};


inline unsigned int
SampleAssignmentContainer::get_number_of_assignments() const {
  return d_.size()/width_;
}

inline Assignment
SampleAssignmentContainer::get_assignment(unsigned int i) const {
  return Assignment(d_.begin()+i*width_,
                    d_.begin()+(i+1)*width_);
}





#if defined(IMP_DOMINO_USE_IMP_RMF) || defined(IMP_DOXYGEN)
/** Store the assignments in an HDF5DataSet
 */
class IMPDOMINOEXPORT HDF5AssignmentContainer: public AssignmentContainer {
  RMF::HDF5IndexDataSet2D ds_;
  bool init_;
  Ints order_;
 public:
  HDF5AssignmentContainer(RMF::HDF5Group parent,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name);

  HDF5AssignmentContainer(RMF::HDF5IndexDataSet2D dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name);

  IMP_ASSIGNMENT_CONTAINER(HDF5AssignmentContainer);
};
#endif



/** Store a set of k top scoring assignemnts
 */
class IMPDOMINOEXPORT HeapAssignmentContainer: public AssignmentContainer {
  typedef std::pair<Assignment,double> AP;
  struct GreaterSecond {
    bool operator()(const AP &a,
                    const AP &b) {
      return a.second < b.second;
    }
  };
  typedef compatibility::checked_vector<AP> C;
  C d_;
  unsigned int k_; // max number of assignments (heap size)
  Pointer<RestraintScoreSubsetFilter> rssf_;//to score candidate assignments
 public:
  HeapAssignmentContainer(unsigned int k,
                          RestraintScoreSubsetFilter *rssf,
                          std::string name="HeapAssignmentsContainer %1%");
  IMP_ASSIGNMENT_CONTAINER(HeapAssignmentContainer);
};

/** Store the centers of clusters of the assignments. For now, the embedding
 used cannot be varied. It is a concatenation of embeddings provided by
 the particle states.*/
class IMPDOMINOEXPORT ClusteredAssignmentContainer:
  public AssignmentContainer {
  unsigned int k_;
  Subset s_;
  Pointer<ParticleStatesTable> pst_;
  double r_;
  typedef std::pair<algebra::VectorKD, Assignment> AP;
  compatibility::checked_vector<AP> d_;
  algebra::VectorKD get_embedding(const Assignment &a) const;
  bool get_in_cluster(const algebra::VectorKD &v) const;
  double get_minimum_distance() const;
  void recluster();
  double get_distance_if_smaller_than(const algebra::VectorKD &a,
                                    const algebra::VectorKD &b,
                                    double max) const;
 public:
  ClusteredAssignmentContainer(unsigned int k,
                               Subset s,
                               ParticleStatesTable *pst);
  /** Return the r parameter defining the maximum size of the cluster.
   */
  double get_r() const {return r_;}
  IMP_ASSIGNMENT_CONTAINER(ClusteredAssignmentContainer);
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_ASSIGNMENT_CONTAINERS_H */
