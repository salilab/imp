/**
 *  \file IMP/rmf/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INFRASTRUCTURE_MACROS_H
#define IMPRMF_INFRASTRUCTURE_MACROS_H

#include <stdexcept>
#include <sstream>

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
#define IMP_RMF_COMPARISONS(Name)

/** @} */
#elif defined(SWIG)
#define IMP_RMF_SWIG_COMPARISONS(Name)          \
  bool __eq__(const Name &o) const;             \
  bool __ne__(const Name &o) const;             \
  bool __lt__(const Name &o) const;             \
  bool __gt__(const Name &o) const;             \
  bool __ge__(const Name &o) const;             \
  bool __le__(const Name &o) const


#define IMP_RMF_COMPARISONS(Name)               \
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

#define IMP_RMF_COMPARISONS(Name)               \
  bool operator==(const Name &o) const {        \
    return compare(o)==0;                       \
  }                                             \
  bool operator!=(const Name &o) const {        \
    return compare(o) != 0;                     \
  }                                             \
  bool operator<(const Name &o) const {         \
    return compare(o)<0;                        \
  }                                             \
  bool operator>(const Name &o) const {         \
    return compare(o)>0;                        \
  }                                             \
  bool operator>=(const Name &o) const {        \
    return compare(o) >=0;                      \
  }                                             \
  bool operator<=(const Name &o) const {        \
    return compare(o) <= 0;                     \
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



#if defined(_MSC_VER)
#  define IMP_RMF_FUNCTION __FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define IMP_RMF_FUNCTION __FUNCTION__
# else
#  define IMP_RMF_FUNCTION "<unknown>"
# endif
#else
# define IMP_RMF_FUNCTION __func__
#endif


#define IMP_RMF_USAGE_CHECK(check, message)                  \
  do {                                                       \
    if (!(check)) {                                          \
      IMP_RMF_THROW("Usage check failed: " << #check << "\n" \
                    << message, std::runtime_error);         \
    }                                                        \
  } while (false)

#define IMP_RMF_INTERNAL_CHECK(check, message)                          \
  do {                                                                  \
    if (!(check)) {                                                     \
      IMP_RMF_THROW("Internal check failed: " << #check << "\n"         \
                    << message << "\nThis is a bug in the RMF library"  \
                    << " please report it.", std::logic_error);         \
    }                                                                   \
  } while (false)

#define IMP_RMF_IF_CHECK\
  if (true)

#define IMP_RMF_NOT_IMPLEMENTED\
  IMP_RMF_THROW("The function " << IMP_RMF_FUNCTION << " is not implemented", \
                std::logic_error)

#define IMP_RMF_UNUSED(variable) if (0) std::cout << variable;


#define IMP_RMF_THROW(m,e) do {  \
    std::ostringstream oss;      \
    oss << m;                    \
    throw e(oss.str().c_str());  \
  } while (false)

/** Call a function and throw an exception if the return values is bad */
#define IMP_HDF5_CALL(v) IMP_RMF_USAGE_CHECK((v)>=0, "Error calling "<< (#v))

/** Apply the macro to each supported constant size type (eg int as opposed
    to string).

    \see IMP_RMF_FOREACH_TYPE()
*/
#define IMP_RMF_FOREACH_SIMPLE_TYPE(macroname)                          \
  macroname(int, Int, int, int,                                         \
            const Ints&, Ints);                                         \
  macroname(float, Float, double, double,                               \
            const Floats&, Floats);                                     \
  macroname(index, Index, int, int,                                     \
            const Ints&, Ints)


/** Expand to applying the macro to each type supported by
    the rmf library. The macro should take six argments
    - the lower case name of the type
    - the upper case name
    - the C++ type for accepting the value
    - the C++ type for returning the value
    - the C++ type for accepting more than one value
    - the C++ type for returning more than one value
*/
#define IMP_RMF_FOREACH_TYPE(macroname)                                 \
  IMP_RMF_FOREACH_SIMPLE_TYPE(macroname);                               \
  macroname(string, String, String, String,                             \
            const Strings &, Strings);                                  \
  macroname(node_id, NodeID, NodeID, NodeID, const NodeIDs&, NodeIDs);  \
  macroname(data_set, DataSet, String, String,                          \
            const Strings &, Strings);                                  \
  macroname(node_ids, NodeIDs, const NodeIDs&, NodeIDs,                 \
            const NodeIDsList &, NodeIDsLst);                           \
  macroname(ints, Ints, const Ints&, Ints,                              \
            const IntsList &,                                           \
            IntsList)

#endif  /* IMPRMF_INFRASTRUCTURE_MACROS_H */
