/**
 *  \file ObjectContainer.h    \brief A container for Objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_OBJECT_CONTAINER_H
#define IMP_OBJECT_CONTAINER_H

#include "../Object.h"
#include "../RefCountedObject.h"
#include "ref_counting.h"

#include <boost/iterator/filter_iterator.hpp>

#include <vector>

IMP_BEGIN_NAMESPACE

namespace internal
{

// A container which holds objects
/* The container owns the objects and deletes them when it is destroyed.
   All accesses are bounds checked and objects are checked to make
   sure they are still OK. This should only be used through the
   IMP_CONTAINER macros.

   \note I want the base class to be protected, but for some reason I
   couldn't get the casts to work out right.
 */
template <class O, class I>
class ObjectContainer
{
  std::vector<O*> data_;
  std::vector<int> free_;
  struct OK {
    bool operator()(const O*a) const {
      return a != NULL;
    }
  };

  template <class II>
  unsigned int get_index(II i) const {return i.get_name();}
  unsigned int get_index(unsigned int i) const {return i;}

  void check_unique(O* o) {
    IMP_CHECK_OBJECT(o);
    for (typename Vector::const_iterator it= data_.begin();
         it != data_.end(); ++it) {
      IMP_assert(*it != o, "IMP Containers can only have one copy of "
                 << " each object");
    }
  }

  template <class It>
  void check_all_unique(It b, It e) {
    for (It cc= b; cc != e; ++cc) {
      check_unique(*cc);
    }
  }

  I fill_free(O *o) {
    unsigned int i= free_.back();
    free_.pop_back();
    data_[i]= o;
    return I(i);
  }

  // hide it
  typedef std::vector<O*> Vector;

public:

  ObjectContainer(){}
  ~ObjectContainer() {
    clear();
  }

  bool empty() const {
    return data_.empty() || data_.size() == free_.size();
  }
  unsigned int size() const {
    return data_.size() - free_.size();
  }

  void clear() {
    for (typename Vector::iterator it= data_.begin();
         it != data_.end(); ++it) {
      O* t= *it;
      *it=NULL;
      if (t) disown(t);
    }
    free_.clear();
    data_.clear();
  }

  typedef boost::filter_iterator<OK, typename Vector::iterator> iterator;
  iterator begin() {return iterator(data_.begin(), data_.end());}
  iterator end() {return iterator(data_.end(), data_.end());}

  typedef boost::filter_iterator<OK, typename Vector::const_iterator>
  const_iterator;
  const_iterator begin() const {
    return const_iterator(data_.begin(), data_.end());
  }
  const_iterator end() const {
    return const_iterator(data_.end(), data_.end());
  }

  void remove(I i) {
    unsigned int id= get_index(i);
    IMP_assert(id < data_.size(),
               "Trying to remove invalid element in container");
    IMP_assert(data_[id] != NULL, "Nothing there to remove");
    O* t= data_[id];
    free_.push_back(id);
    data_[id]=NULL;
    disown(t);
  }

  O* operator[](I i) const {
    IMP_check(get_index(i) < data_.size(),
              "Index " << i << " out of range",
              IndexException);
    IMP_assert(data_.operator[](get_index(i)) != NULL,
               "Attempting to access invalid slot in container");
    return data_[get_index(i)];
  }

  I push_back(O* d) {
    own(d);
    IMP_IF_CHECK(EXPENSIVE) {
      check_unique(d);
    }
    if (free_.empty()) {
      data_.push_back(d);
      return I(data_.size()-1);
    } else {
      return fill_free(d);
    }
  }

  template <class It>
  void insert(iterator c, It b, It e) {
    IMP_assert(c== end(), "Insert position is ignored in ObjectContainer");
    IMP_IF_CHECK(EXPENSIVE) {
      check_all_unique(b,e);
    }
    for (It cc= b; cc != e; ++cc) {
      own(*cc);
    }
    while (!free_.empty()) {
      fill_free(*b);
      ++b;
    }
    data_.insert(data_.end(), b, e);
  }

};

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_OBJECT_CONTAINER_H */
