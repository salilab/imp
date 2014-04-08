/**
 *  \file IMP/domino/assignment_containers.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_CONTAINERS_H
#define IMPDOMINO_ASSIGNMENT_CONTAINERS_H

#include <IMP/domino/domino_config.h>
#include "Assignment.h"
#include "Order.h"
#include "subset_scores.h"
#include <IMP/base/Vector.h>
#include <IMP/base/hash.h>
#include <boost/unordered_map.hpp>
#include <IMP/statistics/metric_clustering.h>

#include <algorithm>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/shared_array.hpp>
#include <cstdio>
#include <queue>

#if IMP_DOMINO_HAS_RMF
#include <RMF/HDF5/Group.h>
#include <RMF/HDF5/File.h>
#endif

#ifdef _MSC_VER
#include <io.h>
#endif

IMPDOMINO_BEGIN_NAMESPACE

/** The base class for containers of assignments. Assignments are stored
    in these rather than as Assignments to help increase efficiency as
    well as provide flexibility as to how and where they are stored.
*/
class IMPDOMINOEXPORT AssignmentContainer : public IMP::base::Object {
 public:
  AssignmentContainer(std::string name = "AssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const = 0;
  virtual Assignment get_assignment(unsigned int i) const = 0;
  virtual Assignments get_assignments(IntRange ir) const = 0;
  virtual Assignments get_assignments() const = 0;
  virtual void add_assignment(const Assignment &a) = 0;
  virtual void add_assignments(const Assignments &as) = 0;
  //! Get all the assignments for the ith particle
  virtual Ints get_particle_assignments(unsigned int i) const = 0;

  virtual ~AssignmentContainer();
};

IMP_OBJECTS(AssignmentContainer, AssignmentContainers);

/** Store a set of assignments in a somewhat more compact form in memory
    than the ListAssignmentContainer.
 */
class IMPDOMINOEXPORT PackedAssignmentContainer : public AssignmentContainer {
  // store all as one vector
  Ints d_;
  int width_;

 public:
  PackedAssignmentContainer(std::string name =
                                "PackedAssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(PackedAssignmentContainer);
  IMP_OBJECT_METHODS(PackedAssignmentContainer);
};

#ifndef IMP_DOXYGEN

inline unsigned int PackedAssignmentContainer::get_number_of_assignments()
    const {
  if (width_ == -1) return 0;
  return d_.size() / width_;
}

inline Assignment PackedAssignmentContainer::get_assignment(unsigned int i)
    const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  IMP_USAGE_CHECK(width_ > 0, "Uninitualized PackedAssignmentContainer.");
  return Assignment(d_.begin() + i * width_, d_.begin() + (i + 1) * width_);
}

inline void PackedAssignmentContainer::add_assignment(const Assignment &a) {
  IMP_OBJECT_LOG;
  if (width_ == -1) {
    width_ = a.size();
  }
  IMP_USAGE_CHECK(static_cast<int>(a.size()) == width_,
                  "Sizes don't match " << width_ << " vs " << a.size());
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i = 0; i < get_number_of_assignments(); ++i) {
      IMP_INTERNAL_CHECK(get_assignment(i) != a, "Assignment "
                                                     << a << " already here.");
    }
  }
  d_.insert(d_.end(), a.begin(), a.end());
}
#endif

/** Simple storage of a set of Assignments. Prefer PackedAssignmentContainer,
    I think.
 */
class IMPDOMINOEXPORT ListAssignmentContainer : public AssignmentContainer {
  // store all as one vector
  Assignments d_;

 public:
  ListAssignmentContainer(std::string name = "ListAssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(ListAssignmentContainer);
  IMP_OBJECT_METHODS(ListAssignmentContainer);
};

#ifndef IMP_DOXYGEN
inline unsigned int ListAssignmentContainer::get_number_of_assignments() const {
  return d_.size();
}

inline Assignment ListAssignmentContainer::get_assignment(unsigned int i)
    const {
  return d_[i];
}

inline void ListAssignmentContainer::add_assignment(const Assignment &a) {
  d_.push_back(a);
}
#endif

/** Store a list of k assignments chosen from all of the ones added to this
    table. The states are chosen uniformly.

    This doesn't seem very useful
 */
class IMPDOMINOEXPORT SampleAssignmentContainer : public AssignmentContainer {
  // store all as one vector
  Ints d_;
  int width_;
  unsigned int k_;
  unsigned int i_;
  boost::uniform_real<double> select_;
  boost::uniform_int<> place_;

 public:
  SampleAssignmentContainer(unsigned int k,
                            std::string name =
                                "SampleAssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(SampleAssignmentContainer);
  IMP_OBJECT_METHODS(SampleAssignmentContainer);
};

#ifndef IMP_DOXYGEN
inline unsigned int SampleAssignmentContainer::get_number_of_assignments()
    const {
  return d_.size() / width_;
}

inline Assignment SampleAssignmentContainer::get_assignment(unsigned int i)
    const {
  return Assignment(d_.begin() + i * width_, d_.begin() + (i + 1) * width_);
}
#endif

#if IMP_DOMINO_HAS_RMF || defined(IMP_DOXYGEN)
/** Store the assignments in an HDF5DataSet. Make sure to delete this
    container before trying to read from the same data set (unless
    you pass the data set explicitly, in which case it may be OK).

    The format on disk should
    not, yet, be considered stable.
 */
class IMPDOMINOEXPORT WriteHDF5AssignmentContainer
    : public AssignmentContainer {
  RMF::HDF5::IndexDataSet2D ds_;
  Order order_;
  Ints cache_;
  unsigned int max_cache_;
  void flush();
  virtual void do_destroy() IMP_OVERRIDE { flush(); }

 public:
  WriteHDF5AssignmentContainer(RMF::HDF5::Group parent, const Subset &s,
                               const kernel::ParticlesTemp &all_particles,
                               std::string name);

  WriteHDF5AssignmentContainer(RMF::HDF5::IndexDataSet2D dataset,
                               const Subset &s,
                               const kernel::ParticlesTemp &all_particles,
                               std::string name);
  void set_cache_size(unsigned int words);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(WriteHDF5AssignmentContainer);
  IMP_OBJECT_METHODS(WriteHDF5AssignmentContainer);
};

/** Store the assignments in an HDF5DataSet. The format on disk should not,
    yet, be considered stable.
 */
class IMPDOMINOEXPORT ReadHDF5AssignmentContainer : public AssignmentContainer {
  RMF::HDF5::IndexConstDataSet2D ds_;
  Order order_;
  Ints cache_;
  unsigned int max_cache_;
  void flush();

 public:
  ReadHDF5AssignmentContainer(RMF::HDF5::IndexConstDataSet2D dataset,
                              const Subset &s,
                              const kernel::ParticlesTemp &all_particles,
                              std::string name);
  void set_cache_size(unsigned int words);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(ReadHDF5AssignmentContainer);
  IMP_OBJECT_METHODS(ReadHDF5AssignmentContainer);
};
#endif

/** Store the assignments on disk as binary data. Use a ReadAssignmentContainer
    to read them back. The resulting file is not guaranteed to work on any
    platform other than the one it was created on and the format may change.
 */
class IMPDOMINOEXPORT WriteAssignmentContainer : public AssignmentContainer {
  int f_;
  Order order_;
  Ints cache_;
  unsigned int max_cache_;
  int number_;
  void flush();

  virtual void do_destroy() IMP_OVERRIDE {
    flush();
#ifdef _MSC_VER
    _close(f_);
#else
    close(f_);
#endif
  }

 public:
  WriteAssignmentContainer(std::string out_file, const Subset &s,
                           const kernel::ParticlesTemp &all_particles,
                           std::string name);
  void set_cache_size(unsigned int words);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(WriteAssignmentContainer);
  IMP_OBJECT_METHODS(WriteAssignmentContainer);
};

/** Read the assignments from binary data on disk. Use a
    WriteAssignmentContainer to write them. Make sure to destroy the
    WriteAssignmentContainer before trying to read from the file.
 */
class IMPDOMINOEXPORT ReadAssignmentContainer : public AssignmentContainer {
  int f_;
  Order order_;
  mutable Ints cache_;
  unsigned int max_cache_;
  mutable int offset_;
  int size_;
  virtual void do_destroy() IMP_OVERRIDE {
#ifdef _MSC_VER
    _close(f_);
#else
    close(f_);
#endif
  }

 public:
  ReadAssignmentContainer(std::string out_file, const Subset &s,
                          const kernel::ParticlesTemp &all_particles,
                          std::string name);
  void set_cache_size(unsigned int words);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(ReadAssignmentContainer);
  IMP_OBJECT_METHODS(ReadAssignmentContainer);
};

/** Expose a range [begin, end) of an inner assignement container to
    consumers. One cannot add assignments to this container.
 */
class IMPDOMINOEXPORT RangeViewAssignmentContainer
    : public AssignmentContainer {
  base::Pointer<AssignmentContainer> inner_;
  int begin_, end_;

 public:
  RangeViewAssignmentContainer(AssignmentContainer *inner, unsigned int begin,
                               unsigned int end);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(RangeViewAssignmentContainer);
  IMP_OBJECT_METHODS(RangeViewAssignmentContainer);
};

/** Store a set of k top scoring assignemnts
 */
class IMPDOMINOEXPORT HeapAssignmentContainer : public AssignmentContainer {
  typedef std::pair<Assignment, double> AP;
  struct GreaterSecond {
    bool operator()(const AP &a, const AP &b) { return a.second < b.second; }
  };
  typedef base::Vector<AP> C;
  C d_;
  Subset subset_;
  Slices slices_;
  kernel::Restraints rs_;
  unsigned int k_;                      // max number of assignments (heap size)
  base::Pointer<RestraintCache> rssf_;  // to score candidate assignments
 public:
  HeapAssignmentContainer(Subset subset, unsigned int k, RestraintCache *rssf,
                          std::string name = "HeapAssignmentsContainer %1%");
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(HeapAssignmentContainer);
  IMP_OBJECT_METHODS(HeapAssignmentContainer);
};

/** This is a wrapper for an AssignmentContainer that throws a ValueException
    if more than a certain number of states are added.*/
class IMPDOMINOEXPORT CappedAssignmentContainer : public AssignmentContainer {
  typedef AssignmentContainer P;
  base::Pointer<AssignmentContainer> contained_;
  unsigned int max_;
  void check_number() const;

 public:
  CappedAssignmentContainer(AssignmentContainer *contained, int max_states,
                            std::string name);
  virtual unsigned int get_number_of_assignments() const IMP_OVERRIDE;
  virtual Assignment get_assignment(unsigned int i) const IMP_OVERRIDE;
  virtual void add_assignment(const Assignment &a) IMP_OVERRIDE;
  IMP_ASSIGNMENT_CONTAINER_METHODS(CappedAssignmentContainer);
  IMP_OBJECT_METHODS(CappedAssignmentContainer);
};

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_ASSIGNMENT_CONTAINERS_H */
