/**
 *  \file container_macros.h
 *  \brief Macros to define containers of objects
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMP_CONTAINER_MACROS_H
#define IMP_CONTAINER_MACROS_H

#include "RefCounted.h"
#include "Object.h"
#include "internal/Vector.h"
#include "macros.h"




// Swig doesn't do protection right with protected members
#ifdef IMP_SWIG_WRAPPER
#define IMP_PROTECTION(protection) public:
#else
#define IMP_PROTECTION(protection) protection:
#endif

#ifndef SWIG
/** Internal use only. */
#define IMP_EXPOSE_ITERATORS(ContainerType, container_name, Ucname, Ucnames, \
                             lcname, lcnames)                           \
  IMP_SWITCH_DOXYGEN(class Ucname##Iterator;                            \
                     class Ucname##ConstIterator,                       \
                     typedef ContainerType::iterator Ucname##Iterator;  \
                     typedef ContainerType::const_iterator              \
                     Ucname##ConstIterator);                            \
  Ucname##Iterator lcnames##_begin() {return container_name.begin();}   \
  Ucname##Iterator lcnames##_end() {return container_name.end();}       \
  Ucname##ConstIterator lcnames##_begin() const {                       \
    return container_name.begin();                                      \
  }                                                                     \
  Ucname##ConstIterator lcnames##_end() const {                         \
    return container_name.end();}                                       \

#else
#define IMP_EXPOSE_ITERATORS(ContainerType, container_name,     \
                             Ucname, Ucnames,lcname, lcnames)
#endif // SWIG

#ifdef GCC_VISIBILITY
# define IMP_FORCE_EXPORT(x) __attribute__ ((visibility("default"))) x
#else
# define IMP_FORCE_EXPORT(x) x
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
#define IMP_LIST(protection, Ucname, lcname, Data, PluralData)  \
  IMP_LIST_ACTION(protection, Ucname, Ucname##s, lcname,        \
                  lcname##s, Data, PluralData,,,)

#ifdef SWIG

#define IMP_LIST_ACTION(protection, Ucname, Ucnames, lcname, lcnames,   \
                        Data, PluralData,OnAdd,                         \
                        OnChanged, OnRemoved)                           \
  public:                                                               \
  void remove_##lcname(Data d);                                         \
  void remove_##lcnames(const PluralData& d);                           \
  void set_##lcnames(const PluralData&  ps);                            \
  void set_##lcnames##_order(const PluralData& objs);                   \
  unsigned int add_##lcname(Data obj);                                  \
  void add_##lcnames(const PluralData& objs);                           \
  void clear_##lcnames();                                               \
  unsigned int get_number_of_##lcnames() const;                         \
  bool get_has_##lcnames();                                             \
  Data get_##lcname(unsigned int i) const;                              \
  void reserve_##lcnames(unsigned int sz)


#else
/** A version of IMP_LIST() for types where the spelling of the plural is
    irregular (eg geometry-> geometries) and where actions can be taken
    upon addition and removal:
    \param[in] protection The level of protection for the container
    (public, private).
    \param[in] Ucname The name of the type of container in uppercase.
    \param[in] lcname The name of the type of container in lower case.
    \param[in] Data The type of the data to store.
    \param[in] PluralData The plural of the data name. This should be a
    container type.
    \param[in] OnAdd Code to modify the passed in object. The object is obj
    and its index index.
    \param[in] OnChanged Code to get executed when the container changes.
    \param[in] OnRemoved Code to get executed when the an object is removed.
*/
#define IMP_LIST_ACTION(protection, Ucname, Ucnames, lcname, lcnames,   \
                        Data, PluralData,OnAdd,                         \
                        OnChanged, OnRemoved)                           \
  IMP_PROTECTION(protection)                                            \
  /** \brief Remove any occurences of d from the container. */          \
  void remove_##lcname(Data d) {                                        \
    for (Ucname##Iterator it= lcnames##_begin();                        \
         it != lcnames##_end(); ++it) {                                 \
      if (*it == d) {                                                   \
        lcname##_handle_remove(*it);                                    \
        lcname##_vector_.erase(it); break;                              \
      }                                                                 \
    }                                                                   \
    lcname##_handle_change();                                           \
  }                                                                     \
  /** \brief Remove any occurrences for which f is true */              \
  template <class F>                                                    \
  void remove_##lcnames##_if(const F &f) {                              \
    for (Ucname##Iterator it= lcnames##_begin(); it != lcnames##_end(); \
         ++it) {                                                        \
      if (f(*it)) lcname##_handle_remove(*it);                          \
    }                                                                   \
    IMP::internal::remove_if(lcname##_vector_, f);                      \
    lcname##_handle_change();                                           \
  }                                                                     \
  /** \brief Remove any occurences of each item in d. */                \
  template <class List>                                                 \
  void remove_##lcnames(List d) {                                       \
    std::vector<Data> ds(d.begin(), d.end());                           \
    std::sort(ds.begin(), ds.end());                                    \
    for (unsigned int i=0; i< ds.size(); ++i) {                         \
      lcname##_handle_remove(ds[i]);                                    \
    }                                                                   \
    lcname##_vector_.remove_if(::IMP::internal::list_contains(ds));     \
  }                                                                     \
  /** Set the contents of the container to ps removing all its current
      contents. */                                                      \
template <class List>                                                   \
void set_##lcnames(List ps) {                                           \
  /* Bad things can happen if we use a Temp, as things get unreffed
     before being reffed if they are in both lists */                   \
  clear_##lcnames();                                                    \
  add_##lcnames(ps);                                                    \
}                                                                       \
/** Must be the same set, just in a different order. */                 \
template <class List>                                                   \
void set_##lcnames##_order(List ps) {                                   \
  IMP_USAGE_CHECK(ps.size() == lcname##_vector_.size(),                 \
                  "Reordered elements don't match.");                   \
  lcname##_vector_.clear();                                             \
  lcname##_vector_.insert(lcname##_vector_.end(),                       \
                          ps.begin(), ps.end());                        \
}                                                                       \
/** \return index of object within the object
 */                                                                     \
unsigned int add_##lcname(Data obj) {                                   \
  unsigned int index= lcname##_vector_.size();                          \
  lcname##_vector_.push_back(obj);                                      \
  IMP_UNUSED(index); IMP_UNUSED(obj);                                   \
  OnAdd;                                                                \
  lcname##_handle_change();                                             \
  return index;                                                         \
}                                                                       \
/** Add several objects to the container. They are not necessarily
    added at the end.
*/                                                                      \
template <class List>                                                   \
void add_##lcnames(List objs) {                                         \
  unsigned int osz= lcname##_vector_.size();                            \
  lcname##_vector_.insert(lcname##_vector_.end(), objs.begin(),         \
                          objs.end());                                  \
  for (PluralData::size_type i=0; i< objs.size(); ++i) {                \
    Data obj= lcname##_vector_[osz+i];                                  \
    unsigned int index(osz+i);                                          \
    OnAdd;                                                              \
    if (false) {obj=obj; index=index;}                                  \
  }                                                                     \
  lcname##_handle_change();                                             \
}                                                                       \
void clear_##lcnames() {                                                \
  lcname##_vector_.clear();                                             \
  lcname##_handle_change();                                             \
}                                                                       \
unsigned int get_number_of_##lcnames() const {                          \
  return lcname##_vector_.size();}                                      \
/** \brief return true if there are any objects in the container*/      \
bool get_has_##lcnames() const {                                        \
  return !lcname##_vector_.empty();}                                    \
/** Get the object refered to by the index
    \throws IndexException in Python if the index is out of range
*/                                                                      \
Data get_##lcname(unsigned int i) const {                               \
  return lcname##_vector_[i];                                           \
}                                                                       \
void reserve_##lcnames(unsigned int sz) {                               \
  lcname##_vector_.reserve(sz);                                         \
}                                                                       \
IMP_EXPOSE_ITERATORS(IMP::VectorOfRefCounted<Data>,                     \
                     lcname##_vector_, Ucname, Ucnames, lcname, lcnames); \
private:                                                                \
const PluralData &access_##lcnames() const {return lcname##_vector_;}   \
void lcname##_handle_remove( Data obj) {                                \
  Ucname##DataWrapper::do_handle_remove(obj, this);                     \
}                                                                       \
void lcname##_handle_change() {                                         \
  OnChanged;                                                            \
}                                                                       \
struct Ucname##DataWrapper: public PluralData {                         \
  template <class F>                                                    \
  void remove_if(const F &f) {                                          \
    IMP::internal::remove_if(*static_cast<PluralData*>(this), f);       \
  }                                                                     \
  template <class TT>                                                   \
  static void do_handle_remove( Data obj, TT *container){               \
    IMP_UNUSED(container);                                              \
    IMP_UNUSED(obj);                                                    \
    OnRemoved;                                                          \
  }                                                                     \
  /* Older GCC (e.g. on Mac OS X 10.4) does not correctly export the
     symbol for this destructor even when the surrounding class is itself
     exported, causing lookup failures in DSOs that use the class.
     Work around this by forcing the symbol to be exported. Ideally, we
     should have a configure check for this problem... */               \
  IMP_FORCE_EXPORT(~Ucname##DataWrapper());                             \
};                                                                      \
friend struct Ucname##DataWrapper;                                      \
IMP_NO_DOXYGEN(Ucname##DataWrapper lcname##_vector_;)                   \
IMP_PROTECTION(protection)                                              \
IMP_REQUIRE_SEMICOLON_CLASS(list##lcname)

#endif


//! This should go in a .cpp file for the respective class.
/**
   This code should go in a .cpp file. One macro for each IMP_CONTAINER.
   \param[in] Class The name of the class containing this container.
   \param[in] Ucname The name of the type of container in uppercase.
   \param[in] lcname The name of the type of container in lower case.
   \param[in] Data The type of the data to store.
   \param[in] PluralData The plural of the data name. This should be a
   container type.

   For all of these the current object is called obj and is of type Data.
*/
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, PluralData)          \
  IMP_LIST_ACTION_IMPL(Class, Ucname, Ucname##s, lcname, lcname##s,     \
                       Data, PluralData)

#define IMP_LIST_ACTION_IMPL(Class, Ucname, Ucnames, lcname, lcnames,   \
                             Data, PluralData)                          \
  Class::Ucname##DataWrapper::~Ucname##DataWrapper() {                  \
    for (unsigned int i=0; i< size(); ++i) {                            \
      do_handle_remove(operator[](i), static_cast<Class*>(0));          \
    }                                                                   \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#endif  /* IMP_CONTAINER_MACROS_H */
