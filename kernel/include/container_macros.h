/**
 *  \file container_macros.h
 *  \brief Macros to define containers of objects
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMP_CONTAINER_MACROS_H
#define IMP_CONTAINER_MACROS_H

#include "RefCounted.h"
#include "VectorOfRefCounted.h"
#include "internal/Vector.h"
#include "macros.h"




// Swig doesn't do protection right with protected members
#ifdef IMP_SWIG_WRAPPER
#define IMP_PROTECTION(protection) public:
#else
#define IMP_PROTECTION(protection) protection:
#endif





/**  \brief  A macro to provide a uniform interface for storing lists of
     objects.

     This macro is designed to be used in the body of an object to add
     a set of methods for manipulating a list of contained objects. It adds
     methods
     - get_foo
     - set_foo
     - set_foos
     - foos_begin, foos_end
     - remove_foo
     - remove_foos
     etc. where foo is the lcname provided.

     \param[in] protection The level of protection for the container
     (public, private).
     \param[in] Ucname The name of the type of container in uppercase.
     \param[in] lcname The name of the type of container in lower case.
     \param[in] Data The type of the data to store.
     \param[in] PluralData The plural of the data name. This should be a
     container type.

     An accompanying IMP_LIST_IMPL must go in a \c .cpp file.

     \note This macro should be given an appropriate name and wrapped in a
     doxygen comment block such as by starting with a C++ comment like
     \verbatim
     @name short description
     longer description
     @{
     \endverbatim
     and ending with one like
     \verbatim
     @}
     \endverbatim
 */
#define IMP_LIST(protection, Ucname, lcname, Data, PluralData)          \
  IMP_PROTECTION(protection)                                            \
  /** \brief Remove any occurences of d from the container. */          \
  void remove_##lcname(Data d);                                         \
  /** \brief Remove any occurrences for which f is true */              \
  template <class F>                                                    \
  void remove_##lcname##s_if(const F &f) {                              \
    for (Ucname##Iterator it= lcname##s_begin(); it != lcname##s_end(); \
         ++it) {                                                        \
      if (f(*it)) handle_remove(*it);                                   \
    }                                                                   \
    IMP::internal::remove_if(lcname##_vector_, (f));                    \
  }                                                                     \
  /** \brief Remove any occurences of each item in d. */                \
  void remove_##lcname##s(const Ucname##s& d);                          \
  /** Set the contents of the container to ps removing all its current
      contents. */                                                      \
  void set_##lcname##s(const Ucname##s &ps) {                           \
    clear_##lcname##s();                                                \
    add_##lcname##s(ps);                                                \
  }                                                                     \
/** \return index of object within the object
*/                                                                      \
unsigned int add_##lcname(Data obj);                                    \
/** Add several objects to the container. They are not necessarily
    added at the end.
*/                                                                      \
void add_##lcname##s(const PluralData& obj);                            \
void clear_##lcname##s();                                               \
unsigned int get_number_of_##lcname##s() const {                        \
  return lcname##_vector_.size();}                                      \
/** \brief return true if there are any objects in the container*/      \
bool get_has_##lcname##s() const {                                      \
  return !lcname##_vector_.empty();}                                    \
/** Get the object refered to by the index
    \throws IndexException if the index is out of range
*/                                                                     \
Data get_##lcname(unsigned int i) const {                              \
  return lcname##_vector_[i];                                          \
}                                                                      \
void reserve_##lcname##s(unsigned int sz) {                             \
  lcname##_vector_.reserve(sz);                                         \
}                                                                       \
IMP_NO_DOXYGEN(typedef IMP::VectorOfRefCounted<Data>                    \
               ::iterator Ucname##Iterator;)                            \
IMP_NO_DOXYGEN(typedef IMP::VectorOfRefCounted<Data>::const_iterator    \
               Ucname##ConstIterator;)                                  \
IMP_ONLY_DOXYGEN(class Ucname##Iterator; class Ucname##ConstIterator;)  \
Ucname##Iterator lcname##s_begin() {return lcname##_vector_.begin();}   \
Ucname##Iterator lcname##s_end() {return lcname##_vector_.end();}       \
Ucname##ConstIterator lcname##s_begin() const {                         \
  return lcname##_vector_.begin();}                                     \
Ucname##ConstIterator lcname##s_end() const {                           \
  return lcname##_vector_.end();}                                       \
IMP_NO_DOXYGEN(private:)                                                \
void handle_remove(Data d);                                             \
IMP_NO_DOXYGEN(PluralData lcname##_vector_;)                            \
IMP_PROTECTION(protection)                                              \




//! This should go in a .cpp file for the respective class.
/**
 This code should go in a .cpp file. One macro for each IMP_CONTAINER.
 \param[in] Class The name of the class containing this container.
 \param[in] Ucname The name of the type of container in uppercase.
 \param[in] lcname The name of the type of container in lower case.
 \param[in] Data The type of the data to store.
 \param[in] PluralData The plural of the data name. This should be a
 container type.
 \param[in] OnAdd Code to modify the passed in object. The object is obj
 its index index.
 \param[in] OnChanged Code to get executed when the container changes.
 \param[in] OnRemoved Code to get executed when the an object is removed.

 For all of these the current object is called obj.
*/
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, PluralData, OnAdd,   \
                      OnChanged, OnRemoved)                             \
  void Class::handle_remove( Data obj){                                 \
    if (0) std::cout << obj;                                            \
    OnRemoved;                                                          \
  }                                                                     \
  unsigned int Class::add_##lcname(Data obj) {                          \
    unsigned int index= lcname##_vector_.size();                        \
    lcname##_vector_.push_back(obj);                                    \
    OnAdd;                                                              \
    OnChanged;                                                          \
    if (false) {index=index; obj=obj;};                                 \
    return index;                                                       \
  }                                                                     \
  void Class::add_##lcname##s(const PluralData &objs) {                 \
    unsigned int osz= lcname##_vector_.size();                          \
    lcname##_vector_.insert(lcname##_vector_.end(), objs.begin(),       \
                            objs.end());                                \
    for (unsigned int i=0; i< objs.size(); ++i) {                       \
      Data obj= lcname##_vector_[osz+i];                                \
      unsigned int index(osz+i);                                        \
      OnAdd;                                                            \
      if (false) {obj=obj; index=index;}                                \
    }                                                                   \
    OnChanged;                                                          \
  }                                                                     \
  void Class::remove_##lcname##s(const Ucname##s& d) {                  \
    std::vector<Data> ds(d.begin(), d.end());                           \
    std::sort(ds.begin(), ds.end());                                    \
    for (unsigned int i=0; i< ds.size(); ++i) {                         \
      handle_remove(ds[i]);                                             \
    }                                                                   \
    IMP::internal::remove_if(lcname##_vector_,                          \
                             ::IMP::internal::list_contains(ds));       \
  }                                                                     \
  void Class::clear_##lcname##s(){                                      \
    lcname##_vector_.clear();                                           \
    OnChanged;                                                          \
  }                                                                     \
  void Class::remove_##lcname(Data d) {                                 \
    for (Ucname##Iterator it= lcname##s_begin();                        \
         it != lcname##s_end(); ++it) {                                 \
      if (*it == d) {                                                   \
        handle_remove(*it);                                             \
        lcname##_vector_.erase(it); break;                              \
      }                                                                 \
    }                                                                   \
    OnChanged;                                                          \
  }                                                                     \

#endif  /* IMP_CONTAINER_MACROS_H */
