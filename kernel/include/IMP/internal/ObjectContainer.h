/**
 *  \file ObjectContainer.h    \brief A container for Objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_CONTAINER_H
#define __IMP_OBJECT_CONTAINER_H

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
class ObjectContainer: public std::vector<O*>
{
  std::vector<int> free_;
  struct OK {
    bool operator()(const O*a) const {
      return a != NULL;
    }
  };

  template <class II>
  unsigned int get_index(II i) const {return i.get_index();}
  unsigned int get_index(unsigned int i) const {return i;}

  void ref(O*o) {
    if (o) o->ref();
  }

  void unref(O* o) {
    if (o) {
      o->unref();
      if (!o->get_has_ref()) {
        delete o;
      }
    }
  }
  // hide it
  void erase(){}
public:
  typedef std::vector<O*> Vector;
  using Vector::size;
  using Vector::empty;
  ObjectContainer(){}
  ~ObjectContainer() {
    clear();
  }

  void clear() {
    for (typename Vector::iterator it= Vector::begin(); 
         it != Vector::end(); ++it) {
      unref(*it);
    }
    free_.clear();
    Vector::clear();
  }

  typedef boost::filter_iterator<OK, typename Vector::iterator> iterator;
  iterator begin() {return iterator(Vector::begin(), Vector::end());}
  iterator end() {return iterator(Vector::end(), Vector::end());}

  typedef boost::filter_iterator<OK, typename Vector::const_iterator>
  const_iterator;
  const_iterator begin() const {
    return const_iterator(Vector::begin(), Vector::end());
  }
  const_iterator end() const {
    return const_iterator(Vector::end(), Vector::end());
  }

  void remove(I i) {
    unsigned int id= get_index(i);
    IMP_assert(Vector::operator[](id) != NULL, "Nothing there to remove");
    unref(Vector::operator[](id));
    Vector::operator[](id)=NULL;
    free_.push_back(id);
  }

  O* operator[](I i) const {
    IMP_check(get_index(i) < Vector::size(),
              "Index " << i << " out of range",
              IndexException("Out of range"));
    IMP_assert(Vector::operator[](get_index(i)) != NULL,
               "Attempting to access invalid slot in container");
    return Vector::operator[](get_index(i));
  }
  I push_back(O* d) {
    IMP_CHECK_OBJECT(d);
    ref(d);
    for (typename Vector::const_iterator it= Vector::begin();
         it != Vector::end(); ++it) {
      IMP_assert(*it != d, "IMP Containers can only have one copy of "
                 << " each object");
    }
    if (free_.empty()) {
      Vector::push_back(d);
      unsigned int idx= Vector::size()-1;
      return I(idx);
    } else {
      unsigned int i= free_.back();
      free_.pop_back();
      Vector::operator[](i)= d;
      return I(i);
    }
  }
  template <class It>
  void insert(iterator c, It b, It e) {
#ifndef NDEBUG
    for (It cc= b; cc != e; ++cc) {
      IMP_CHECK_OBJECT(*cc);
      for (typename Vector::const_iterator it= Vector::begin(); 
           it != Vector::end(); ++it) {
        IMP_assert(*it != *cc, "IMP Containers can only have one copy of "
                   << " each object");
      }
   }
#endif
    for (It cc= b; cc != e; ++cc) {
      ref(*cc);
    }
    while (!free_.empty()) {
      push_back(*b);
      ++b;
    }
    Vector::insert(c.base(), b, e);
  }

};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_CONTAINER_H */
