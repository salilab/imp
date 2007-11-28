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
  /** Optimize the model.                                               \
      \param[in] model The model to be optimized.\                      \
      \param[in] max_steps The maximum number of steps to take.         \
      \param[in] threshold Stop if the score goes below threshold.      \
      \return The final score.                                          \
   */                                                                   \
  virtual Float optimize(Model* model, int max_steps,                   \
                         Float threshold=-std::numeric_limits<Float>::max()); \
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
#endif  /* __IMP_UTILITY_H */
