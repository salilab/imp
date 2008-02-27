/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MACROS_H
#define __IMP_MACROS_H

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field)                                        \
  /** */ bool operator==(const This &o) const {                         \
    return (field== o.field);                                           \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (field!= o.field);                                           \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field< o.field);                                            \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field> o.field);                                            \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field>= o.field);                                           \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field<= o.field);                                           \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)                                       \
  /** */ bool operator==(const This &o) const {                         \
    return (f0== o.f0 && f1==o.f1);                                     \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (f0!= o.f0 || f1 != o.f1);                                   \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    else return f1 < o.f1;                                              \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    else return f1 > o.f1;                                              \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_3(f0, f1, f2)                                   \
  /** */ bool operator==(const This &o) const {                         \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);                       \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);                     \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    if (f1< o.f1) return true;                                          \
    else if (f1 > o.f1) return false;                                   \
    else return f2 < o.f2;                                              \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    if (f1 > o.f1) return true;                                         \
    else if (f1 < o.f1) return false;                                   \
    else return f2 > o.f2;                                              \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement operator<< on class name, assuming it has one template argument
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_1(name) /** write to a stream*/             \
template <class L>                                                      \
 inline std::ostream& operator<<(std::ostream &out, const name<L> &i)   \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name, assuming it has two template arguments
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_2(name) /** write to a stream*/             \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR(name)   /** write to a stream*/             \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                 \
    return out;                                                  \
  }

//! Define the basic things you need for a Restraint.
/** These are: show, evaluate, get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_RESTRAINT(version_info)                       \
  /** evaluate the restraint*/                                          \
  virtual Float evaluate(DerivativeAccumulator *accum);                 \
  /** write information about the restraint to the stream*/             \
  virtual void show(std::ostream &out=std::cout) const;                 \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basic things you need for an optimizer.
/** These are: optimize, get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER(version_info)                                     \
  /** \short Optimize the model.
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */                                                                   \
  virtual Float optimize(unsigned int max_steps);                       \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }


//! Define the basics needed for an OptimizerState
/** This macro declares the required functions 
    - void update()
    - void show(std::ostream &out) const
    and defines the function
    - get_version_info

    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER_STATE(version_info)                               \
  /** update the state*/                                                \
  virtual void update();                                                \
  /** write information about the state to the stream*/                 \
  virtual void show(std::ostream &out=std::cout) const;                 \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! See IMP_OPTIMIZER_STATE
#define IMP_SCORE_STATE(version_info)\
  IMP_OPTIMIZER_STATE(version_info)

//! Use the swap_with member function to swap two objects
#define IMP_SWAP(name) \
  inline void swap(name &a, name &b) {          \
    a.swap_with(b);                             \
  }

//! swap two member variables assuming the other object is called o
#define IMP_SWAP_MEMBER(var_name) \
  std::swap(var_name, o.var_name);



//! use a copy_from method to create a copy constructor and operator=
#define IMP_COPY_CONSTRUCTOR(TC) TC(const TC &o){copy_from(o);}  \
  TC& operator=(const TC &o) {copy_from(o); return *this;}




/** \internal
 */
#define IMP_CONTAINER_CORE(protection, Ucname, lcname, Data, IndexType,\
                           Container)                                  \
  protection:                                                          \
  /** \short Add an object.
      \param[in] obj Pointer to the object
      \return index of object within the object
  */                                                                    \
  IndexType add_##lcname(Data obj);                                     \
  /** \short Add several objects.
      \param[in] obj a vector of pointers
  */                                                                    \
  void add_##lcname##s(const std::vector<Ucname*>& obj);                \
  /** \short Get object refered to by the index
      \throws IndexException if the index is out of range
   */                                                                   \
  Data get_##lcname(IndexType i) const {                                \
    return lcname##_vector_[i];                                         \
  }                                                                     \
  /** \short return the number of objects*/                             \
  unsigned int number_of_##lcname##s() const {                          \
    return lcname##_vector_.size();}                                    \
  /** \short Get a container of all the objects. 
     This is for Python as the container can be used like a Python list*/\
  const Ucname##s &get_##lcname##s() const {                             \
    return static_cast< const Ucname##s &>(lcname##_vector_);}           \
  /** \short An iterator through the objects.
      The value type is a pointer.*/                                     \
  typedef Container::iterator Ucname##Iterator;                          \
  /** \short A const iterator through the objects. 
      The value type is a pointer.*/                                     \
  typedef Container::const_iterator Ucname##ConstIterator;               \
  Ucname##Iterator lcname##s_begin() {return lcname##_vector_.begin();}  \
  Ucname##Iterator lcname##s_end() {return lcname##_vector_.end();}      \
  Ucname##ConstIterator lcname##s_begin() const {                        \
    return lcname##_vector_.begin();}                                    \
  Ucname##ConstIterator lcname##s_end() const {                          \
    return lcname##_vector_.end();}                                      \
 private:                                                                \
  /** \internal */                                                       \
  Container lcname##_vector_;                                            \
 protection:

/** \internal
 */
#define IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Data, IndexType, \
                                Init_obj, Onchanged)                    \
  IndexType Class::add_##lcname(Data obj) {                             \
    unsigned int osz=lcname##_vector_.size();                           \
    IndexType index(osz);                                               \
    lcname##_vector_.push_back(obj);                                    \
    Init_obj;                                                           \
    Onchanged;                                                          \
    return index;                                                       \
  }                                                                     \
  void Class::add_##lcname##s(const std::vector<Data> &objs) {          \
    unsigned int osz= lcname##_vector_.size();                          \
    lcname##_vector_.insert(lcname##_vector_.end(), objs.begin(),       \
                            objs.end());                                \
    for (unsigned int i=0; i< objs.size(); ++i) {                       \
      Data obj= lcname##_vector_[osz+i];                                \
      IndexType index(osz+i);                                           \
      Init_obj;                                                         \
    }                                                                   \
    Onchanged;                                                          \
  }

//! Use this to add a container of IMP objects
/**
   Such a container adds public methods add_foo, get_foo, number_of_foo 
   and a private type foo_iterator, with methods foo_begin, foo_end.
   \param[in] protection The level of protection for the container.
   \param[in] Ucname The name of the type in uppercase
   \param[in] lcname The name of the type in lower case
   \param[in] Data The type of the data to store.
   \param[in] Onchanged Code to get executed every time the container
   changes

   \note the type Ucnames must be declared and be a vector of
   Data.
 */
#define IMP_LIST(protection, Ucname, lcname, Data)                      \
  protection:                                                           \
  /** \short Clear the contents of the container */                     \
  void clear_##lcname##s();                                             \
  /** \short Remove any occurences of d from the container */           \
  void erase_##lcname(Data d);                                          \
  IMP_CONTAINER_CORE(protection, Ucname, lcname, Data, unsigned int,    \
                     IMP::internal::Vector<Data>)



//! Use this to add a container of IMP objects
/**
   This code should go in a .cpp file. One macro for each IMP_CONTAINER.
   \param[in] init Code to modify the passed in object. The object is obj
   its index index.
   \param[in] OnChanged Code to get executed when the container changes.
 */
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, init, OnChanged)     \
  IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Data, unsigned int,    \
                          init, OnChanged)                              \
  /** \short Clear the contents of the container */                     \
  void Class::clear_##lcname##s(){                                      \
    lcname##_vector_.clear();                                           \
    OnChanged;                                                          \
  }                                                                     \
  /** \short Remove any occurences of d from the container */           \
  void Class::erase_##lcname(Data d) {                                  \
    for (Ucname##Iterator it= lcname##s_begin();                        \
         it != lcname##s_end(); ++it) {                                 \
      if (*it == d) {                                                   \
        lcname##_vector_.erase(it); break;                              \
      }                                                                 \
    }                                                                   \
    OnChanged;                                                          \
  }                                                                     \



//! Use this to add a set of IMP objects owned by the containing one
/**
   Such a container adds public methods add_foo, get_foo, number_of_foo 
   and a private type foo_iterator, with methods foo_begin, foo_end.
   \param[in] Ucname The name of the type in uppercase
   \param[in] lcname The name of the type in lower case
   \param[in] IndexType The type to use for the index. This should be
   an instantiation of Index<T> or something similar.

   \note The type Ucnames must be declared and be a vector of
   Data.
   \note these containers are always public
 */
#define IMP_CONTAINER(Ucname, lcname, IndexType)                \
  private:                                                      \
  /** \internal 
      This is an implementation detail.*/                       \
  typedef IMP::internal::ObjectContainer<Ucname, IndexType>     \
  Ucname##Container;                                            \
  IMP_CONTAINER_CORE(public, Ucname, lcname, Ucname*, IndexType,\
                     Ucname##Container)



//! Use this to add a container of IMP objects
/**
   This code should go in a .cpp file. One macro for each 
   IMP_CONTAINER.
   \param[in] init Code to modify the passed in object. The object is obj
   its index index.
 */
#define IMP_CONTAINER_IMPL(Class, Ucname, lcname, IndexType, init) \
  IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Ucname*, IndexType,    \
                          init,)



//! Call the assert_is_valid method in the object base
#define IMP_CHECK_OBJECT(obj) do {              \
    IMP_assert(obj != NULL, "NULL object");     \
    (obj)->assert_is_valid();                   \
  } while (false)


// They use IMP_CHECK_OBJECT and so must be included at the end

#include "internal/Vector.h"
#include "internal/ObjectContainer.h"


#endif  /* __IMP_MACROS_H */
