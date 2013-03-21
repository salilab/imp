/**
 *  \file RMF/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INFRASTRUCTURE_MACROS_H
#define RMF_INFRASTRUCTURE_MACROS_H

#include <sstream>
#include <iostream>
#include <string>
#include <RMF/compiler_macros.h>

#include <vector>

RMF_ENABLE_WARNINGS

#ifdef NDEBUG
#  define RMF_NDEBUG
#endif

#if defined(RMF_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
 */

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
 */
#  define RMF_COMPARISONS(Name)

/** @} */
#elif defined(SWIG)
#  define RMF_SWIG_COMPARISONS(Name) \
  bool __eq__(const Name &o) const;  \
  bool __ne__(const Name &o) const;  \
  bool __lt__(const Name &o) const;  \
  bool __gt__(const Name &o) const;  \
  bool __ge__(const Name &o) const;  \
  bool __le__(const Name &o) const


#  define RMF_COMPARISONS(Name) \
  RMF_SWIG_COMPARISONS(Name)

#else // not doxygen

#  define RMF_SWIG_COMPARISONS(Name) \
  bool __eq__(const Name &o) const { \
    return operator==(o);            \
  }                                  \
  bool __ne__(const Name &o) const { \
    return operator!=(o);            \
  }                                  \
  bool __lt__(const Name &o) const { \
    return operator<(o);             \
  }                                  \
  bool __gt__(const Name &o) const { \
    return operator>(o);             \
  }                                  \
  bool __ge__(const Name &o) const { \
    return operator>=(o);            \
  }                                  \
  bool __le__(const Name &o) const { \
    return operator<=(o);            \
  }                                  \
  int __cmp__(const Name &o) const { \
    return compare(o);               \
  }

#  define RMF_COMPARISONS(Name)          \
  bool operator==(const Name &o) const { \
    return compare(o) == 0;              \
  }                                      \
  bool operator!=(const Name &o) const { \
    return compare(o) != 0;              \
  }                                      \
  bool operator<(const Name &o) const {  \
    return compare(o) < 0;               \
  }                                      \
  bool operator>(const Name &o) const {  \
    return compare(o) > 0;               \
  }                                      \
  bool operator>=(const Name &o) const { \
    return compare(o) >= 0;              \
  }                                      \
  bool operator<=(const Name &o) const { \
    return compare(o) <= 0;              \
  }                                      \
  RMF_SWIG_COMPARISONS(Name)

#endif



#ifdef RMF_DOXYGEN

//! Implement a hash function for the class
#  define RMF_HASHABLE(name, hashret)
/** @} */
#else

#  define RMF_HASHABLE(name, hashret) \
  std::size_t __hash__() const {      \
    hashret;                          \
  }

#endif

/** @} */


#ifndef SWIG
#  define RMF_SHOWABLE(Name, streamed)  \
  operator Showable() const {           \
    std::ostringstream oss;             \
    oss << streamed;                    \
    return oss.str();                   \
  }                                     \
  void show(std::ostream & out) const { \
    out << streamed;                    \
  }
#else
#  define RMF_SHOWABLE(Name, streamed)
#endif


#define RMF_UNUSED(variable) if (0) std::cout << variable;

#define RMF_NO_RETURN(type) return type()


/** Apply the macro to each supported constant size type (eg int as opposed
    to string).

    \see RMF_FOREACH_TYPE()
 */
#define RMF_FOREACH_SIMPLE_TYPE(macroname) \
  macroname(int,   Int,   int,    int,     \
            const Ints &, Ints);           \
  macroname(float, Float, double, double,  \
            const Floats &, Floats);       \
  macroname(index, Index, int,    int,     \
            const Ints &, Ints)

#ifndef RMF_DOXYGEN
/** Expand to applying the macro to each type supported by
    the rmf library. The macro should take six argments
    - the lower case name of the type
    - the upper case name
    - the C++ type for accepting the value
    - the C++ type for returning the value
    - the C++ type for accepting more than one value
    - the C++ type for returning more than one value
 */
#  define RMF_FOREACH_TYPE(macroname)                                              \
  RMF_FOREACH_SIMPLE_TYPE(macroname);                                              \
  macroname(string,   String,  String,          String,                            \
            const Strings &, Strings);                                             \
  macroname(strings,  Strings, Strings,         Strings,                           \
            const StringsList &, StringsList);                                     \
  macroname(node_id,  NodeID,  NodeID,          NodeID, const NodeIDs &, NodeIDs); \
  macroname(node_ids, NodeIDs, const NodeIDs &, NodeIDs,                           \
            const NodeIDsList &, NodeIDsList);                                     \
  macroname(floats,   Floats,  const Floats &,  Floats,                            \
            const FloatsList &,                                                    \
            FloatsList);                                                           \
  macroname(ints,     Ints,    const Ints &,    Ints,                              \
            const IntsList &,                                                      \
            IntsList);                                                             \
  macroname(indexes,  Indexes, const Indexes &, Indexes,                           \
            const IndexesList &,                                                   \
            IndexesList);
#else
#  define RMF_FOREACH_TYPE(macroname) \
  macroname(type, Type, Type, Types,  \
            const Types &, Types);
#endif


#define RMF_DECORATOR_CATCH(extra_info) \
catch (Exception &e) {\
  RMF_RETHROW(Decorator(get_name()) extra_info, e);\
}


/** Register a validator function. See Validator for more
    information.*/
#define RMF_VALIDATOR(Type) \
  RMF::Registrar<Type> Type##Reg(#Type);

namespace RMF {

#if !defined(RMF_DOXYGEN) && !defined(SWIG)
struct Showable;
inline std::ostream &
operator<<(std::ostream &out, const Showable &t);

/** Produce hash values for boost hash tables.
 */
template <class T>
inline std::size_t hash_value(const T &t) {
  return t.__hash__();
}

struct Showable {
  std::string t_;
  template <class T>
  explicit Showable(const T &t) {
    std::ostringstream oss;
    oss << t;
    t_ = oss.str();
  }
  template <class T, class TT>
  Showable(const std::pair<T, TT> &p) {
    std::ostringstream oss;
    oss << "(" << p.first << ", " << p.second << ")";
    t_ = oss.str();
  }
  Showable( std::string t): t_(t) {
  }
  template <class T>
  Showable( const std::vector<T> &t ) {
    std::ostringstream out;
    out << "[";
    for (unsigned int i = 0; i < t.size(); ++i) {
      if (i != 0) {
        out << ", ";
      }
      out << t[i];
    }
    out << "]";
    t_ = out.str();
  }
};

inline std::ostream &
operator<<(std::ostream &out, const Showable &t) {
  out << t.t_;
  return out;
}

#endif
}

RMF_DISABLE_WARNINGS

#endif  /* RMF_INFRASTRUCTURE_MACROS_H */
