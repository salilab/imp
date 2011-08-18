/**
 *  \file IMP/rmf/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INFRASTRUCTURE_MACROS_H
#define IMPRMF_INFRASTRUCTURE_MACROS_H

#if defined(IMP_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
*/

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] field the first field to compare on
*/
#define IMP_RMF_COMPARISONS_1(Name, field)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] f0 the first field to compare on
    \param[in] f1 the second field to compare on
*/
#define IMP_RMF_COMPARISONS_2(Name, f0, f1)

/** @} */
#elif defined(SWIG)
#define IMP_RMF_SWIG_COMPARISONS(Name)          \
  bool __eq__(const Name &o) const;             \
  bool __ne__(const Name &o) const;             \
  bool __lt__(const Name &o) const;             \
  bool __gt__(const Name &o) const;             \
  bool __ge__(const Name &o) const;             \
  bool __le__(const Name &o) const


#define IMP_RMF_COMPARISONS_1(Name, field)      \
  IMP_RMF_SWIG_COMPARISONS(Name)

#define IMP_RMF_COMPARISONS_2(Name, f0, f1)     \
  IMP_RMF_SWIG_COMPARISONS(Name)

#else // not doxygen

#define IMP_RMF_SWIG_COMPARISONS(Name)                                  \
  bool __eq__(const Name &o) const {                                    \
    return operator==(o);                                               \
  }                                                                     \
  bool __ne__(const Name &o) const {                                    \
    return operator!=(o);                                               \
  }                                                                     \
  bool __lt__(const Name &o) const {                                    \
    return operator<(o);                                                \
  }                                                                     \
  bool __gt__(const Name &o) const {                                    \
    return operator>(o);                                                \
  }                                                                     \
  bool __ge__(const Name &o) const {                                    \
    return operator>=(o);                                               \
  }                                                                     \
  bool __le__(const Name &o) const {                                    \
    return operator<=(o);                                               \
  }                                                                     \
  int __cmp__(const Name &o) const {                                    \
    return compare(o);                                                  \
  }

#define IMP_RMF_COMPARISONS_1(Name, field)      \
  bool operator==(const Name &o) const {        \
    return (field== o.field);                   \
  }                                             \
  bool operator!=(const Name &o) const {        \
    return (field!= o.field);                   \
  }                                             \
  bool operator<(const Name &o) const {         \
    return (field< o.field);                    \
  }                                             \
  bool operator>(const Name &o) const {         \
    return (field> o.field);                    \
  }                                             \
  bool operator>=(const Name &o) const {        \
    return (field>= o.field);                   \
  }                                             \
  bool operator<=(const Name &o) const {        \
    return (field<= o.field);                   \
  }                                             \
  int compare(const Name &o) const {            \
    if (operator<(o)) return -1;                \
    else if (operator>(o)) return 1;            \
    else return 0;                              \
  }                                             \
  IMP_RMF_SWIG_COMPARISONS(Name)

#define IMP_RMF_COMPARISONS_2(Name, f0, f1)     \
  bool operator==(const Name &o) const {        \
    return (f0== o.f0 && f1==o.f1);             \
  }                                             \
  bool operator!=(const Name &o) const {        \
    return (f0!= o.f0 || f1 != o.f1);           \
  }                                             \
  bool operator<(const Name &o) const {         \
    if (f0< o.f0) return true;                  \
    else if (f0 > o.f0) return false;           \
    else return f1 < o.f1;                      \
  }                                             \
  bool operator>(const Name &o) const {         \
    if (f0 > o.f0) return true;                 \
    else if (f0 < o.f0) return false;           \
    else return f1 > o.f1;                      \
  }                                             \
  bool operator>=(const Name &o) const {        \
    return operator>(o) || operator==(o);       \
  }                                             \
  bool operator<=(const Name &o) const {        \
    return operator<(o) || operator==(o);       \
  }                                             \
  int compare(const Name &o) const {            \
    if (operator<(o)) return -1;                \
    else if (operator>(o)) return 1;            \
    else return 0;                              \
  }                                             \
  IMP_RMF_SWIG_COMPARISONS(Name)

#endif



#ifdef IMP_DOXYGEN

//! Implement a hash function for the class
#define IMP_RMF_HASHABLE(name, hashret)
/** @} */
#else

#define IMP_RMF_HASHABLE(name, hashret)         \
  std::size_t __hash__() const {                \
    hashret;                                    \
  }

#endif

/** @} */


#ifdef IMP_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation.
*/
#define IMP_RMF_GRAPH(Name, type, VertexName, EdgeName) \
  /** See \ref graphs "Graphs" for more information.*/  \
  typedef boost::graph Name

#elif defined(SWIG)
#define IMP_RMF_GRAPH(Name, type, VertexName, EdgeName)  class Name
#else
#define IMP_RMF_GRAPH(Name, type, VertexName, EdgeName)                 \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
                                boost::type##S,                         \
                                boost::property<boost::vertex_name_t,   \
                                                VertexName>,            \
                                boost::property<boost::edge_name_t,     \
                                                EdgeName> > Name
#endif

#endif  /* IMPRMF_INFRASTRUCTURE_MACROS_H */
