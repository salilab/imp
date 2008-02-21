/**
 *  \file ObjectContainer.h    \brief A container for Objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OBJECT_CONTAINER_H
#define __IMP_OBJECT_CONTAINER_H

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
public:
  typedef std::vector<O*> Vector;
  using Vector::const_iterator;
  using Vector::iterator;
  using Vector::begin;
  using Vector::end;
  using Vector::size;
  using Vector::empty;
  ObjectContainer(){}
  ~ObjectContainer() {
    for (typename Vector::iterator it= Vector::begin(); 
         it != Vector::end(); ++it) {
      delete *it;
    }
  }
  O* operator[](I i) const {
    IMP_check(i.get_index() < Vector::size(),
              "Index " << i << " out of range",
              IndexException("Out of range"));
    return Vector::operator[](i.get_index());
  }
  I push_back(O* d) {
    IMP_CHECK_OBJECT(d);
    for (typename Vector::const_iterator it= begin(); it != end(); ++it) {
      IMP_assert(*it != d, "IMP Containers can only have one copy of "
                 << " each object");
    }
#ifndef NDEBUG
    IMP_assert(!d->get_is_owned(), "Object already owned by another pointer");
    d->set_is_owned(true);
#endif
    Vector::push_back(d);
    return I(Vector::size()-1);
  }
  template <class It>
  void insert(typename Vector::iterator c, It b, It e) {
#ifndef NDEBUG
    for (It cc= b; cc != e; ++cc) {
      IMP_CHECK_OBJECT(*cc);
      for (typename Vector::const_iterator it= begin(); it != end(); ++it) {
        IMP_assert(*it != *cc, "IMP Containers can only have one copy of "
                   << " each object");
      }
      IMP_assert(!(*cc)->get_is_owned(),
                 "Object already owned by another pointer");
      (*cc)->set_is_owned(true);
    }
#endif
    Vector::insert(c, b, e);
  }

};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_OBJECT_CONTAINER_H */
