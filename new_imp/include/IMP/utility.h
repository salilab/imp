/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_UTILITY_H
#define __IMP_UTILITY_H


//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field) \
  /** */ bool operator==(const This &o) const {  \
    return (field== o.field);      \
  }         \
  /** */ bool operator!=(const This &o) const {    \
    return (field!= o.field);      \
  }         \
  /** */ bool operator<(const This &o) const {     \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field< o.field);      \
  }         \
  /** */ bool operator>(const This &o) const {     \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field> o.field);      \
  }         \
  /** */ bool operator>=(const This &o) const {    \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field>= o.field);      \
  }         \
  /** */ bool operator<=(const This &o) const {    \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field<= o.field);      \
  }

//! Implement operator<< on class name, assuming it has one template argument
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_1(name) /** write to a stream*/             \
template <class L>                                                      \
  inline std::ostream &operator<<(std::ostream &out, const name<L> &i)  \
  {                                                                     \
    return i.show(out);                                                 \
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
/** These are: show, evaluate, version, last_modified_by
    \param[in] version_string The version string.
    \param[in] lmb_string The name of the last modifier.
*/
#define IMP_RESTRAINT(version_string, lmb_string)                       \
  /** evaluate the restraint*/                                          \
  virtual Float evaluate(DerivativeAccumulator *accum);                 \
  /** write information about the restraint to the stream*/             \
  virtual void show(std::ostream &out) const;                           \
  /** \return the current version*/                                     \
  virtual std::string version() const {return std::string(version_string);}\
  /** \return the last person to modify this restraint */               \
  virtual std::string last_modified_by() const {return std::string(lmb_string);}

//! Define the basic things you need for an optimizer.
/** These are: optimize, version, last_modified_by
    \param[in] version_string The version string.
    \param[in] lmb_string The name of the last modifier.
*/
#define IMP_OPTIMIZER(version_string, lmb_string)                       \
  /** \short Optimize the model.
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */                                                                   \
  virtual Float optimize(unsigned int max_steps);                       \
  /** \return the current version*/                                     \
  virtual std::string version() const {return std::string(version_string);}\
  /** \return the last person to modify this restraint */               \
  virtual std::string last_modified_by() const {return std::string(lmb_string);}



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




//! Use this to add a container of IMP objects
/**
   Such a container adds public methods add_foo, get_foo, number_of_foo 
   and a private type foo_iterator, with methods foo_begin, foo_end.
   \param[in] Ucname The name of the type in uppercase
   \param[in] lcname The name of the type in lower case
   \param[in] IndexType The type to use for the index. This should be
   an instantiation of Index<T> or something similar.

   Eventually we can add removal and correctness checks.
 */
#define IMP_CONTAINER(Ucname, lcname, IndexType)                        \
  public:                                                               \
  /** \short Add an object.
      \param[in] obj Pointer to the object
      \return index of object within the object
  */                                                                    \
  IndexType add_##lcname(Ucname *obj);                                  \
  /** \short Get object refered to by the index
      \throws IndexException if the index is out of range
   */                                                                   \
  Ucname *get_##lcname(IndexType i) const ;                             \
  /** \short return the number of objects*/                             \
  unsigned int number_of_##lcname##s() const {                          \
    return lcname##_vector_.size();}                                    \
/** \short Get a container of all the objects. 
    This is for python as the container can be used like a python list*/\
const std::vector<Ucname*> &get_##lcname##s() const {                   \
  return lcname##_vector_;}                                             \
/** \short An iterator through the objects
    The value type is a pointer.*/                                      \
 typedef std::vector<Ucname*>::iterator Ucname##Iterator;               \
/** \short A const iterator through the objects. 
    The value type is a pointer.*/                                      \
 typedef std::vector<Ucname*>::const_iterator Ucname##ConstIterator;    \
 Ucname##Iterator lcname##s_begin() {return lcname##_vector_.begin();}   \
 Ucname##Iterator lcname##s_end() {return lcname##_vector_.end();}       \
 Ucname##ConstIterator lcname##s_begin() const {                         \
   return lcname##_vector_.begin();}                                    \
 Ucname##ConstIterator lcname##s_end() const {                           \
   return lcname##_vector_.end();}                                      \
private:                                                                \
 std::vector<Ucname*> lcname##_vector_;



//! Use this to add a container of IMP objects
/**
   This code should go in a .cpp file. One macro for each IMP_CONTAINER.
   \param[in] init Code to modify the passed in object. The object is obj
   its index index.
 */
#define IMP_CONTAINER_IMPL(Class, Ucname, lcname, IndexType, init)           \
  IndexType Class::add_##lcname(Ucname *obj) {                          \
    IndexType index(lcname##_vector_.size());                           \
    lcname##_vector_.push_back(obj);                                    \
    init;                                                               \
    return index;                                                       \
  }                                                                     \
  Ucname *Class::get_##lcname(IndexType i) const {                      \
    IMP_check(i.get_index() < lcname##_vector_.size(),                  \
              "Index " << i << " out of range",                         \
              IndexException(#Ucname));                                 \
    return lcname##_vector_[i.get_index()];                             \
  }                                                                     \


//! Destroy the things in the container
/**
   Put this in the containing object desctructor when you use the
   IMP_CONTAINER macro.
   \param[in] Ucname the uppercase name
   \param[in] lcname the lower case name
 */
#define IMP_CONTAINER_DELETE(Ucname, lcname)  \
  for (Ucname##Iterator it= lcname##s_begin(); \
       it != lcname##s_end(); ++it) {          \
    delete *it;                               \
  }                                           \
  lcname##_vector_.clear();

#endif  /* __IMP_UTILITY_H */
