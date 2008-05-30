/**
 *  \file ObjectContainer.h    \brief A container for Objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_CONTAINER_H
#define __IMP_OBJECT_CONTAINER_H

#include "Object.h"
#include "RefCountedObject.h"

#include <boost/iterator/filter_iterator.hpp>

#include <vector>

namespace IMP
{

namespace internal
{

//! A container which holds objects
/** The container owns the objects and deletes them when it is destroyed.
    All accesses are bounds checked and objects are checked to make
    sure they are still OK. This should only be used through the 
    IMP_CONTAINER macros.
    \internal

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
  unsigned int get_index(II i) const {return i.get_index();}
  unsigned int get_index(unsigned int i) const {return i;}

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
              IndexException("Out of range"));
    IMP_assert(data_.operator[](get_index(i)) != NULL,
               "Attempting to access invalid slot in container");
    return data_[get_index(i)];
  }

  I push_back(O* d) {
    IMP_CHECK_OBJECT(d);
    own(d);
    IMP_IF_CHECK(EXPENSIVE) {
      for (typename Vector::const_iterator it= data_.begin();
           it != data_.end(); ++it) {
        IMP_assert(*it != d, "IMP Containers can only have one copy of "
                   << " each object");
      }
    }
    if (free_.empty()) {
      data_.push_back(d);
      unsigned int idx= data_.size()-1;
      return I(idx);
    } else {
      unsigned int i= free_.back();
      free_.pop_back();
      data_[i]= d;
      return I(i);
    }
  }

  template <class It>
  void insert(iterator c, It b, It e) {
    IMP_assert(c== end(), "Insert position is ignored in ObjectContainer");
    IMP_IF_CHECK(EXPENSIVE) {
      for (It cc= b; cc != e; ++cc) {
        IMP_CHECK_OBJECT(*cc);
        for (typename Vector::const_iterator it= data_.begin(); 
             it != data_.end(); ++it) {
          IMP_assert(*it != *cc, "IMP Containers can only have one copy of "
                     << " each object");
        }
      }
    }
    for (It cc= b; cc != e; ++cc) {
      own(*cc);
    }
    while (!free_.empty()) {
      int i= free_.back();
      free_.pop_back();
      data_[i]= *b;
      ++b;
    }
    data_.insert(data_.end(), b, e);
  }

};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_CONTAINER_H */
