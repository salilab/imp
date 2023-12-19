/**
 *  \file IMP/container_macros.h
 *  \brief Macros to define containers of objects
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_MACROS_H
#define IMPKERNEL_CONTAINER_MACROS_H

#include <IMP/kernel_config.h>
#include <IMP/check_macros.h>
#include <IMP/internal/Vector.h>
#include <IMP/SetCheckState.h>
#include <IMP/log_macros.h>
#include <IMP/doxygen_macros.h>
#include <algorithm>

// Swig doesn't do protection right with protected members
#ifdef IMP_SWIG_WRAPPER
#define IMP_PROTECTION(protection) public:
#else
#define IMP_PROTECTION(protection) protection:
#endif

#ifndef SWIG
/** Internal use only. */
#define IMP_EXPOSE_ITERATORS(ContainerType, container_name, Ucname, Ucnames, \
                             lcname, lcnames)                                \
  IMP_SWITCH_DOXYGEN(                                                        \
      class Ucname##Iterator;                                                \
      class Ucname##ConstIterator,                                           \
      typedef ContainerType::iterator Ucname##Iterator;                      \
      typedef ContainerType::const_iterator Ucname##ConstIterator);          \
  Ucname##Iterator lcnames##_begin() { return container_name.begin(); }      \
  Ucname##Iterator lcnames##_end() { return container_name.end(); }          \
  Ucname##ConstIterator lcnames##_begin() const {                            \
    return container_name.begin();                                           \
  }                                                                          \
  Ucname##ConstIterator lcnames##_end() const { return container_name.end(); }

#else
#define IMP_EXPOSE_ITERATORS(ContainerType, container_name, Ucname, Ucnames, \
                             lcname, lcnames)
#endif  // SWIG

#ifdef GCC_VISIBILITY
#define IMP_FORCE_EXPORT(x) __attribute__((visibility("default"))) x
#else
#define IMP_FORCE_EXPORT(x) x
#endif

/* Provide Python list-like object for IMP_LIST, below */
/* This is a bit ugly here and in the output Python code because everything
   in one %pythoncode ends up on a single line */
#ifdef SWIG
#define IMP_LIST_PYTHON_IMPL(lcname, lcnames, ucnames, Ucnames)              \
%pythoncode %{                                                               \
  def __get_##lcnames(self): \
return IMP._list_util.VarList(getdimfunc=self.get_number_of_##lcnames, \
getfunc=self.get_##lcname, erasefunc=self.erase_##lcname, \
appendfunc=self.add_##lcname, extendfunc=self.add_##lcnames, \
clearfunc=self.clear_##lcnames, indexfunc=self._python_index_##lcname) \
%}                                                                           \
%pythoncode %{                                                               \
  def __set_##lcnames(self, obj):\
IMP._list_util.set_varlist(self.##lcnames, obj) \
%}                                                                           \
%pythoncode %{                                                               \
  def __del_##lcnames(self): IMP._list_util.del_varlist(self.##lcnames)      \
%}                                                                           \
%pythoncode %{                                                               \
  ##lcnames = property(__get_##lcnames, __set_##lcnames, __del_##lcnames,\
doc="List of ##ucnames") \
%}

#elif defined(IMP_DOXYGEN)
#define IMP_LIST_PYTHON_IMPL(lcname, lcnames, ucnames, Ucnames)             \
public:                                                                     \
  /** \brief A Python list of Ucnames                                       \
      @pythononlymember */                                                  \
  list lcnames;
#else
#define IMP_LIST_PYTHON_IMPL(lcname, lcnames, ucnames, Ucnames)
#endif

/**  \brief  A macro to provide a uniform interface for storing lists of
     objects.

     This macro is designed to be used in the body of an object to add
     a set of methods for manipulating a list of contained objects. It adds
     methods
     - get_foo
     - set_foo
     - set_foos
     - erase_foo
     - foos_begin, foos_end
     - remove_foo
     - remove_foos
     etc. where foo is the lcname provided. In Python, a 'foos' member
     is also provided, which acts like a regular Python list (the above
     methods can also be used if desired).

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
#define IMP_LIST(protection, Ucname, lcname, Data, PluralData)            \
  IMP_LIST_ACTION(protection, Ucname, Ucname##s, lcname, lcname##s, Data, \
                  PluralData, , , )

#if defined(SWIG) || defined(IMP_DOXYGEN)

#define IMP_LIST_ACTION(protection, Ucname, Ucnames, lcname, lcnames, Data, \
                        PluralData, OnAdd, OnChanged, OnRemoved)            \
  IMP_LIST_PYTHON_IMPL(lcname, lcnames, ucnames, Ucnames)                   \
 public:                                                                    \
  void remove_##lcname(Data d);                                             \
  unsigned int _python_index_##lcname(Data d, unsigned int start,           \
                                      unsigned int stop);                   \
  void remove_##lcnames(const PluralData& d);                               \
  void set_##lcnames(const PluralData& ps);                                 \
  void set_##lcnames##_order(const PluralData& objs);                       \
  unsigned int add_##lcname(Data obj);                                      \
  void add_##lcnames(const PluralData& objs);                               \
  void clear_##lcnames();                                                   \
  unsigned int get_number_of_##lcnames() const;                             \
  bool get_has_##lcnames();                                                 \
  Data get_##lcname(unsigned int i) const;                                  \
  PluralData get_##lcnames() const;                                         \
  void erase_##lcname(unsigned int i);                                      \
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
    \param[in] OnRemoved Code to get executed when an object is removed.
*/
#define IMP_LIST_ACTION(protection, Ucname, Ucnames, lcname, lcnames, Data,    \
                        PluralData, OnAdd, OnChanged, OnRemoved)               \
  IMP_PROTECTION(protection)                                                   \
      /** \brief Remove first occurrence of d from the container. */           \
      void remove_##lcname(Data d) {                                           \
    IMP_OBJECT_LOG;                                                            \
    bool found = false;                                                        \
    for (Ucname##Iterator it = lcnames##_begin(); it != lcnames##_end();       \
         ++it) {                                                               \
      if (*it == d) {                                                          \
        lcname##_handle_remove(*it);                                           \
        found = true;                                                          \
        lcname##_vector_.erase(it);                                            \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    IMP_UNUSED(found);                                                         \
    IMP_USAGE_CHECK(found, d << " not found in container: "                    \
                             << get_as<PluralData>(lcname##_vector_));         \
    lcname##_handle_change();                                                  \
  }                                                                            \
  unsigned int _python_index_##lcname(Data d, unsigned int start,              \
                                      unsigned int stop) {                     \
    bool found = false;                                                        \
    unsigned int num_of = get_number_of_##lcnames();                           \
    start = std::min(start, num_of);                                           \
    stop = std::min(stop, num_of);                                             \
    unsigned int indx = start;                                                 \
    for (Ucname##Iterator it = lcnames##_begin() + start;                      \
         indx < stop; ++it, ++indx) {                                          \
      if (*it == d) {                                                          \
        found = true;                                                          \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!found) {                                                              \
      IMP_THROW(d << " is not in list", ValueException);                       \
    }                                                                          \
    return indx;                                                               \
  }                                                                            \
  /** \brief Remove the ith element in the container */                        \
  void erase_##lcname(unsigned int i) {                                        \
    Ucname##Iterator it = lcnames##_begin() + i;                               \
    lcname##_handle_remove(*it);                                               \
    lcname##_vector_.erase(it);                                                \
    lcname##_handle_change();                                                  \
  }                                                                            \
  /** \brief Remove any occurrences for which f is true */                     \
  template <class F>                                                           \
  void remove_##lcnames##_if(const F& f) {                                     \
    IMP_OBJECT_LOG;                                                            \
    for (Ucname##Iterator it = lcnames##_begin(); it != lcnames##_end();       \
         ++it) {                                                               \
      if (f(*it)) lcname##_handle_remove(*it);                                 \
    }                                                                          \
    lcname##_vector_.erase(                                                    \
        std::remove_if(lcname##_vector_.begin(), lcname##_vector_.end(), f),   \
        lcname##_vector_.end());                                               \
    lcname##_handle_change();                                                  \
  }                                                                            \
  /** \brief Remove any occurrences of each item in d. */                      \
  template <class List>                                                        \
  void remove_##lcnames(List d) {                                              \
    IMP_OBJECT_LOG;                                                            \
    Vector<Data> ds(d.begin(), d.end());                                 \
    std::sort(ds.begin(), ds.end());                                           \
    for (unsigned int i = 0; i < ds.size(); ++i) {                             \
      lcname##_handle_remove(ds[i]);                                           \
    }                                                                          \
    lcname##_vector_.erase(                                                    \
        std::remove_if(lcname##_vector_.begin(), lcname##_vector_.end(),       \
                       ::IMP::internal::list_contains(ds)),              \
        lcname##_vector_.end());                                               \
  }                                                                            \
  /** \brief Set the contents of the container to ps removing all its current
      contents.
  */                                                                           \
  template <class List>                                                        \
  void set_##lcnames(List ps) {                                                \
    IMP_OBJECT_LOG;                                                            \
    /* Bad things can happen if we use a Temp, as things get unreffed
       before being reffed if they are in both lists */                        \
    clear_##lcnames();                                                         \
    add_##lcnames(ps);                                                         \
  }                                                                            \
  /** \brief Must be the same set, just in a different order. */ template <    \
      class List>                                                              \
  void set_##lcnames##_order(List ps) {                                        \
    IMP_OBJECT_LOG;                                                            \
    IMP_USAGE_CHECK(ps.size() == lcname##_vector_.size(),                      \
                    "Reordered elements don't match.");                        \
    lcname##_vector_.clear();                                                  \
    lcname##_vector_.insert(lcname##_vector_.end(), ps.begin(), ps.end());     \
  } /** \brief add obj to the container
        \return index of object within the object
     */                                                                        \
  unsigned int add_##lcname(Data obj) {                                        \
    IMP_OBJECT_LOG;                                                            \
    unsigned int index = lcname##_vector_.size();                              \
    lcname##_vector_.push_back(obj);                                           \
    IMP_UNUSED(index);                                                         \
    IMP_UNUSED(obj);                                                           \
    OnAdd;                                                                     \
    lcname##_handle_change();                                                  \
    return index;                                                              \
  } /** \brief Add several objects to the container. They are not necessarily  \
        added at the end.                                                      \
    */                                                                         \
  template <class List>                                                        \
  void add_##lcnames(List objs) {                                              \
    IMP_OBJECT_LOG;                                                            \
    unsigned int osz = lcname##_vector_.size();                                \
    lcname##_vector_.insert(lcname##_vector_.end(), objs.begin(), objs.end()); \
    for (PluralData::size_type i = 0; i < objs.size(); ++i) {                  \
      Data obj = lcname##_vector_[osz + i];                                    \
      unsigned int index(osz + i);                                             \
      OnAdd;                                                                   \
      IMP_UNUSED(obj);                                                         \
      IMP_UNUSED(index);                                                       \
    }                                                                          \
    lcname##_handle_change();                                                  \
  } /** \brief Clear all objects from the container  */                        \
  void clear_##lcnames() {                                                     \
    lcname##_vector_.clear();                                                  \
    lcname##_handle_change();                                                  \
  }                                                                            \
  unsigned int get_number_of_##lcnames() const {                               \
    return lcname##_vector_.size();                                            \
  } /** \brief return true if there are any objects in the container*/         \
  bool get_has_##lcnames() const {                                             \
    return !lcname##_vector_.empty();                                          \
  } /** Get the object referfed to by the index
        \throws IndexException in Python if the index is out of range
    */                                                                         \
  Data get_##lcname(unsigned int i) const { return lcname##_vector_[i]; }      \
  PluralData get_##lcnames() const {                                           \
    return get_as<PluralData>(lcname##_vector_);                               \
  }                                                                            \
  void reserve_##lcnames(unsigned int sz) { lcname##_vector_.reserve(sz); }    \
  IMP_EXPOSE_ITERATORS(PluralData, lcname##_vector_, Ucname, Ucnames, lcname,  \
                       lcnames);                                               \
                                                                               \
 protected: /** This method allows one to modify the contents of the container
               without
                any callbacks being made.*/                                    \
  PluralData& mutable_access_##lcnames() { return lcname##_vector_; }          \
  IMP_NO_DOXYGEN(const PluralData& access_##lcnames() const {                  \
    return lcname##_vector_;                                                   \
  }) private : void lcname##_handle_remove(Data obj) {                         \
    Ucname##DataWrapper::do_handle_remove(obj, this);                          \
  }                                                                            \
  void lcname##_handle_change() {                                              \
    OnChanged;                                                                 \
    clear_caches();                                                            \
  }                                                                            \
  struct Ucname##DataWrapper : public PluralData {                             \
    template <class TT>                                                        \
    static void do_handle_remove(Data obj, TT* container) {                    \
      IMP_UNUSED(container);                                                   \
      IMP_UNUSED(obj);                                                         \
      OnRemoved;                                                               \
    }                                                                          \
    /* Older GCC (e.g. on Mac OS X 10.4) does not correctly export the
       symbol for this destructor even when the surrounding class is itself
       exported, causing lookup failures in DSOs that use the class.
       Work around this by forcing the symbol to be exported. Ideally, we
       should have a configure check for this problem... */                    \
    IMP_FORCE_EXPORT(~Ucname##DataWrapper());                                  \
  };                                                                           \
  friend struct Ucname##DataWrapper;                                           \
  IMP_NO_DOXYGEN(Ucname##DataWrapper lcname##_vector_;)                        \
      IMP_PROTECTION(protection) IMP_REQUIRE_SEMICOLON_CLASS(list##lcname)

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
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, PluralData)            \
  IMP_LIST_ACTION_IMPL(Class, Ucname, Ucname##s, lcname, lcname##s, Data, \
                       PluralData)

#define IMP_LIST_ACTION_IMPL(Class, Ucname, Ucnames, lcname, lcnames, Data, \
                             PluralData)                                    \
  Class::Ucname##DataWrapper::~Ucname##DataWrapper() {                      \
    for (unsigned int i = 0; i < size(); ++i) {                             \
      do_handle_remove(operator[](i), static_cast<Class*>(0));              \
    }                                                                       \
  }                                                                         \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_CONTAINER_FOREACH_LOOP(ContainerType, container, operation, tname) \
  for (unsigned int _2 = 0; _2 < imp_foreach_indexes.size(); ++_2) {           \
    tname ContainerType::ContainedIndexType _1 = imp_foreach_indexes[_2];      \
    bool imp_foreach_break = false;                                            \
    operation;                                                                 \
    if (imp_foreach_break) {                                                   \
      break;                                                                   \
    }                                                                          \
  }

#define IMP_CONTAINER_FOREACH_IMPL(ContainerType, container, operation, tname) \
  IMPKERNEL_DEPRECATED_MACRO(2.2,                                              \
                             "Use get_contents() and a for loop.");            \
  do {                                                                         \
    if (container->get_provides_access()) {                                    \
      const tname ContainerType::ContainedIndexTypes& imp_foreach_indexes =    \
          container->get_access();                                             \
      IMP_CONTAINER_FOREACH_LOOP(ContainerType, container, operation, tname);  \
    } else {                                                                   \
      tname ContainerType::ContainedIndexTypes imp_foreach_indexes =           \
          container->get_indexes();                                            \
      IMP_CONTAINER_FOREACH_LOOP(ContainerType, container, operation, tname);  \
    }                                                                          \
  } while (false)

/** \see IMP_CONTAINER_FOREACH().

    This version is for use in a template function. See
    IMP_FOREACH_INDEX() for another version.
*/
#define IMP_CONTAINER_FOREACH_TEMPLATE(ContainerType, container, operation) \
  IMP_CONTAINER_FOREACH_IMPL(ContainerType, container, operation, typename)

/** These macros avoid various inefficiencies.

    The macros take the name of the container and the operation to
    perform. In operation, _1 is used to refer to the item using its
    ContainedIndexType (e.g., IMP::ParticleIndex in SingletonContainer,
    or IMP::ParticleIndexPair in PairContainer).
    The location of this item in the container itself is _2.
    Use it like:
    \code
    IMP_CONTAINER_FOREACH(SingletonContainer, sc, std::cout << "Item "
    << _2 << " has particle index " << _1 << std::endl);
    \endcode

    See IMP_CONTAINER_FOREACH_TEMPLATE() if you want to use it in a template
    function.
*/
#define IMP_CONTAINER_FOREACH(ContainerType, container, operation) \
  IMP_CONTAINER_FOREACH_IMPL(ContainerType, container, operation, )

/** Provide a block that can have efficient, direct access to the contents
    of the container in the variable imp_indexes.
*/
#define IMP_CONTAINER_ACCESS(ContainerType, container, operation) \
  do {                                                            \
    if (container->get_provides_access()) {                       \
      const ContainerType::ContainedIndexTypes& imp_indexes =     \
          container->get_access();                                \
      operation;                                                  \
    } else {                                                      \
      ContainerType::ContainedIndexTypes imp_indexes =            \
          container->get_indexes();                               \
      operation;                                                  \
    }                                                             \
  } while (false)

#endif /* IMPKERNEL_CONTAINER_MACROS_H */
