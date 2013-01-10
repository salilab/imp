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

#if RMF_USE_DEBUG_VECTOR
#  include <debug/vector>
#else
#  include <vector>
#endif

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


#ifdef RMF_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation.
 */
#  define RMF_GRAPH(Name, type, VertexName, EdgeName)  \
  /** See \ref graphs "Graphs" for more information.*/ \
  typedef boost::graph Name

#elif defined(SWIG)
#  define RMF_GRAPH(Name, type, VertexName, EdgeName)  class Name
#else
#  define RMF_GRAPH(Name, type, VertexName, EdgeName)                 \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,             \
                                boost::type##S,                       \
                                boost::property<boost::vertex_name_t, \
                                                VertexName>,          \
                                boost::property<boost::edge_name_t,   \
                                                EdgeName> > Name
#endif


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



/** Call a function and throw an RMF::IOException if the return values is bad */
#define RMF_HDF5_CALL(v)                                     \
  if ((v) < 0) {                                             \
    RMF_THROW(Message("HDF5 call failed") << Expression(#v), \
              RMF::IOException);                             \
  }

/** Create new HDF5 SharedData.handle.*/
#define RMF_HDF5_NEW_HANDLE(name, cmd, cleanup)    \
  boost::intrusive_ptr<RMF::HDF5SharedHandle> name \
    = new RMF::HDF5SharedHandle(cmd, cleanup, #cmd)

#define RMF_HDF5_HANDLE(name, cmd, cleanup) \
  RMF::HDF5Handle name(cmd, cleanup, #cmd)

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




/** Register a validator function. See Validator for more
    information.*/
#define RMF_VALIDATOR(Type) \
  RMF::Registrar<Type> Type##Reg(#Type);

namespace RMF {
#if !defined(SWIG)
#  if RMF_USE_DEBUG_VECTOR
using  __gnu_debug::vector;
#  else
using std::vector;
#  endif
#else
template <class T>
class vector {};
#endif

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
  Showable( const vector<T> &t ) {
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

#ifndef SWIG
#  define RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,            \
                         hdf5_memory, hdf5_fill, avro_type, null_value,        \
                         null_test,                                            \
                         wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a,           \
                         multiple)                                             \
  struct RMFEXPORT UCName##Traits:                                             \
    public internal::BaseTraits<UCName, UCNames, avro_type, index, multiple> { \
    static hid_t get_hdf5_disk_type() {                                        \
      return hdf5_disk;                                                        \
    }                                                                          \
    static hid_t get_hdf5_memory_type() {                                      \
      return hdf5_memory;                                                      \
    }                                                                          \
    static void write_value_dataset(hid_t d, hid_t is,                         \
                                    hid_t s,                                   \
                                    UCName v) {                                \
      wv_ds;                                                                   \
    }                                                                          \
    static UCName read_value_dataset(hid_t d, hid_t is,                        \
                                     hid_t sp) {                               \
      UCName ret;                                                              \
      rv_ds;                                                                   \
      return ret;                                                              \
    }                                                                          \
    static void write_values_dataset(hid_t d, hid_t is,                        \
                                     hid_t s,                                  \
                                     const UCNames &v) {                       \
      if (v.empty()) return;                                                   \
      wvs_ds;                                                                  \
    }                                                                          \
    static UCNames read_values_dataset(hid_t d, hid_t is,                      \
                                       hid_t sp,                               \
                                       unsigned int sz) {                      \
      UCNames ret(sz, get_null_value());                                       \
      rvs_ds;                                                                  \
      return ret;                                                              \
    }                                                                          \
    static void write_values_attribute(hid_t a, const UCNames &v) {            \
      if (v.empty()) return;                                                   \
      wvs_a;                                                                   \
    }                                                                          \
    static UCNames read_values_attribute(hid_t a, unsigned int sz) {           \
      UCNames ret(sz, get_null_value());                                       \
      rvs_a;                                                                   \
      return ret;                                                              \
    }                                                                          \
    static hid_t get_hdf5_fill_type() {                                        \
      return hdf5_fill;                                                        \
    }                                                                          \
    static const UCName& get_fill_value() {                                    \
      return get_null_value();                                                 \
    }                                                                          \
    static const UCName& get_null_value() {                                    \
      static const UCName ret = null_value;                                    \
      return ret;                                                              \
    }                                                                          \
    template <class V>                                                         \
    static bool get_is_null_value(const V &i) {                                \
      return null_test;                                                        \
    }                                                                          \
    static std::string get_name() {return #lcname; }                           \
  }

/** Declare a type traits*/
#  define RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory,  \
                     hdf5_fill, avro_type, null_value, null_test,             \
                     wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch)       \
  RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory,      \
                 hdf5_fill, avro_type, null_value, null_test,                 \
                 wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch);          \
  struct UCNames##Traits:                                                     \
    public internal::BaseTraits<UCNames, vector<UCNames>,                     \
                                std::vector<avro_type>,                       \
                                index + 7, false> {                           \
    static hid_t get_hdf5_disk_type() {                                       \
      static RMF_HDF5_HANDLE(ints_type, H5Tvlen_create                        \
                               (UCName##Traits::get_hdf5_disk_type()),        \
                             H5Tclose);                                       \
      return ints_type;                                                       \
    }                                                                         \
    static hid_t get_hdf5_memory_type() {                                     \
      static RMF_HDF5_HANDLE(ints_type, H5Tvlen_create                        \
                               (UCName##Traits::get_hdf5_memory_type()),      \
                             H5Tclose);                                       \
      return ints_type;                                                       \
    }                                                                         \
    static void write_value_dataset(hid_t d, hid_t is,                        \
                                    hid_t s,                                  \
                                    const UCNames &v) {                       \
      hvl_t data;                                                             \
      data.len = v.size();                                                    \
      if (data.len > 0) {                                                     \
        data.p = const_cast< UCName*>(&v[0]);                                 \
      } else {                                                                \
        data.p = NULL;                                                        \
      }                                                                       \
      RMF_HDF5_CALL(H5Dwrite(d,                                               \
                             get_hdf5_memory_type(), is, s,                   \
                             H5P_DEFAULT, &data));                            \
    }                                                                         \
    static UCNames read_value_dataset(hid_t d, hid_t is,                      \
                                      hid_t sp) {                             \
      hvl_t data;                                                             \
      H5Dread (d, get_hdf5_memory_type(), is, sp, H5P_DEFAULT, &data);        \
      UCNames ret(data.len);                                                  \
      std::copy(static_cast<UCName*>(data.p),                                 \
                static_cast<UCName*>(data.p) + data.len,                      \
                ret.begin());                                                 \
      free(data.p);                                                           \
      return ret;                                                             \
    }                                                                         \
    static void write_values_dataset(hid_t d, hid_t is,                       \
                                     hid_t s,                                 \
                                     const vector<UCNames>&v) {               \
      RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(s);                           \
      RMF_UNUSED(v);                                                          \
      RMF_NOT_IMPLEMENTED;                                                    \
    };                                                                        \
    static vector<UCNames> read_values_dataset(hid_t d, hid_t is,             \
                                               hid_t sp, unsigned int sz) {   \
      RMF_UNUSED(d);                                                          \
      RMF_UNUSED(is); RMF_UNUSED(sp); RMF_UNUSED(sz);                         \
      RMF_NOT_IMPLEMENTED;                                                    \
      return vector<UCNames>();                                               \
    }                                                                         \
    static vector<UCNames> read_values_attribute(hid_t a,                     \
                                                 unsigned int size) {         \
      RMF_UNUSED(a);                                                          \
      RMF_UNUSED(size);                                                       \
      RMF_NOT_IMPLEMENTED;                                                    \
      return vector<UCNames>();                                               \
    }                                                                         \
    static void write_values_attribute(hid_t a, const vector<UCNames> &v) {   \
      RMF_UNUSED(a); RMF_UNUSED(v);                                           \
      RMF_NOT_IMPLEMENTED;                                                    \
    }                                                                         \
    static hid_t get_hdf5_fill_type() {                                       \
      return get_hdf5_memory_type();                                          \
    }                                                                         \
    template <class O>                                                        \
    static bool get_is_null_value(const O &o) {                               \
      return o.empty();                                                       \
    }                                                                         \
    static std::string get_name() {return UCName##Traits::get_name() + "s"; } \
  };                                                                          \


/** Declare a type traits*/
#  define RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,     \
                            hdf5_memory, hdf5_fill, avro_type, null_value) \
  RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,                    \
             hdf5_memory, hdf5_fill, avro_type, null_value,                \
             i == get_null_value(),                                        \
             RMF_HDF5_CALL(H5Dwrite(d,                                     \
                                    get_hdf5_memory_type(), is, s,         \
                                    H5P_DEFAULT, &v)),                     \
             RMF_HDF5_CALL(H5Dread(d,                                      \
                                   get_hdf5_memory_type(),                 \
                                   is, sp, H5P_DEFAULT, &ret)),            \
             RMF_HDF5_CALL(H5Dwrite(d,                                     \
                                    get_hdf5_memory_type(), is, s,         \
                                    H5P_DEFAULT,                           \
                                    const_cast<UCName*>(&v[0]))),          \
             RMF_HDF5_CALL(H5Dread(d,                                      \
                                   get_hdf5_memory_type(),                 \
                                   is, sp, H5P_DEFAULT, &ret[0])),         \
             RMF_HDF5_CALL(H5Awrite(a, get_hdf5_memory_type(), &v[0])),    \
             RMF_HDF5_CALL(H5Aread(a, get_hdf5_memory_type(), &ret[0])),   \
             true)


#else

#  define RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,    \
                         hdf5_memory,                                  \
                         hdf5_fill, avro_value, null_value, null_test, \
                         wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a,   \
                         multiple)                                     \
  struct UCName##Traits {                                              \
    typedef UCName Type;                                               \
    typedef UCNames Types;                                             \
    static Type get_null_value();                                      \
  }

#  define RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory, \
                     hdf5_fill, avro_traits, null_value, null_test,          \
                     wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a, batch)      \
  struct UCName##Traits {                                                    \
    typedef UCName Type;                                                     \
    typedef UCNames Types;                                                   \
    static UCName get_null_value();                                          \
  };                                                                         \
  struct UCNames##Traits {                                                   \
    typedef UCNames Type;                                                    \
    static UCNames get_null_value();                                         \
  };

#  define RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,     \
                            hdf5_memory, hdf5_fill, avro_type, null_value) \
  struct UCName##Traits {                                                  \
    typedef UCName Type;                                                   \
    typedef UCNames Types;                                                 \
    static UCName get_null_value();                                        \
  };                                                                       \
  struct UCNames##Traits {                                                 \
    typedef UCNames Type;                                                  \
    static UCNames get_null_value();                                       \
  };

#endif

#endif  /* RMF_INFRASTRUCTURE_MACROS_H */
