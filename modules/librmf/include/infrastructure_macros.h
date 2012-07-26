/**
 *  \file RMF/infrastructure_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INFRASTRUCTURE_MACROS_H
#define IMPLIBRMF_INFRASTRUCTURE_MACROS_H

#include <sstream>
#include <iostream>
#include <string>
#include "exceptions.h"
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/current_function.hpp>

#include <vector>

#ifdef NDEBUG
#define IMP_RMF_NDEBUG
#endif

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

#define IMP_RMF_SWIG_COMPARISONS(Name)          \
  bool __eq__(const Name &o) const {            \
    return operator==(o);                       \
  }                                             \
  bool __ne__(const Name &o) const {            \
    return operator!=(o);                       \
  }                                             \
  bool __lt__(const Name &o) const {            \
    return operator<(o);                        \
  }                                             \
  bool __gt__(const Name &o) const {            \
    return operator>(o);                        \
  }                                             \
  bool __ge__(const Name &o) const {            \
    return operator>=(o);                       \
  }                                             \
  bool __le__(const Name &o) const {            \
    return operator<=(o);                       \
  }                                             \
  int __cmp__(const Name &o) const {            \
    return compare(o);                          \
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

#ifndef SWIG
#define IMP_RMF_SHOWABLE(Name, streamed)        \
  operator Showable() const {                   \
    std::ostringstream oss;                     \
    oss << streamed;                            \
    return oss.str();                           \
  }                                             \
  void show(std::ostream &out) const {          \
    out << streamed;                            \
  }
#else
#define IMP_RMF_SHOWABLE(Name, streamed)
#endif

#define IMP_RMF_USAGE_CHECK(check, message)                             \
  do {                                                                  \
    if (!(check)) {                                                     \
      IMP_RMF_THROW(RMF::internal::get_error_message("Usage check failed: ", \
                                                #check, "\n",           \
                                                message),               \
                    RMF::UsageException);                               \
    }                                                                   \
  } while (false)

#define IMP_RMF_PATH_CHECK(path, context)                               \
  if (!boost::filesystem::exists(path)) {                               \
    IMP_RMF_THROW(RMF::internal::get_error_message(context, path,       \
                                              " does not exist."),      \
                  IOException);                                         \
  }

#ifndef IMP_RMF_NDEBUG
#define IMP_RMF_INTERNAL_CHECK(check, message)                          \
  do {                                                                  \
    if (!(check)) {                                                     \
      IMP_RMF_THROW(RMF::internal                                       \
                    ::get_error_message("Internal check failed: \"",    \
                                                #check, "\"",           \
                                                " at ", __FILE__, ":",  \
                                                __LINE__, "\n",         \
                                                message),               \
                    RMF::InternalException);                            \
    }                                                                   \
  } while (false)


#define IMP_RMF_IF_CHECK                        \
  if (true)

#else // NDEBUG
#define IMP_RMF_INTERNAL_CHECK(check, message)
#define IMP_RMF_IF_CHECK

#endif

#define IMP_RMF_NOT_IMPLEMENTED                                 \
  IMP_RMF_THROW(RMF::internal::get_error_message("Not implemented: ",   \
                                            BOOST_CURRENT_FUNCTION,     \
                                            " in ", __FILE__, ":",      \
                                            __LINE__),                  \
                RMF::InternalException)

#define IMP_RMF_UNUSED(variable) if (0) std::cout << variable;

#define IMP_RMF_NO_RETURN(type) return type()


#define IMP_RMF_THROW(m,e)\
    RMF::internal::handle_error<e>(m)

/** Call a function and throw an RMF::IOException if the return values is bad */
#define IMP_HDF5_CALL(v)                                                \
    if ((v)<0) {                                                        \
      IMP_RMF_THROW(internal::get_error_message("HDF5 call failed: ",   \
                                                #v),                    \
                    RMF::IOException);                                  \
    }

/** Create new HDF5 SharedData.handle.*/
#define IMP_HDF5_NEW_HANDLE(name, cmd, cleanup)         \
  boost::intrusive_ptr<RMF::HDF5SharedHandle> name      \
  = new RMF::HDF5SharedHandle(cmd, cleanup, #cmd)

#define IMP_HDF5_HANDLE(name, cmd, cleanup)     \
  RMF::HDF5Handle name(cmd, cleanup, #cmd)

/** Apply the macro to each supported constant size type (eg int as opposed
    to string).

    \see IMP_RMF_FOREACH_TYPE()
*/
#define IMP_RMF_FOREACH_SIMPLE_TYPE(macroname)  \
  macroname(int, Int, int, int,                 \
            const Ints&, Ints);                 \
  macroname(float, Float, double, double,       \
            const Floats&, Floats);             \
  macroname(index, Index, int, int,             \
            const Ints&, Ints)

#ifndef IMP_DOXYGEN
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
  macroname(strings, Strings, Strings, Strings,                         \
            const StringsList &, StringsList);                          \
  macroname(node_id, NodeID, NodeID, NodeID, const NodeIDs&, NodeIDs);  \
  macroname(node_ids, NodeIDs, const NodeIDs&, NodeIDs,                 \
            const NodeIDsList &, NodeIDsList);                          \
  macroname(floats, Floats, const Floats&, Floats,                      \
            const FloatsList &,                                         \
            FloatsList);                                                \
  macroname(ints, Ints, const Ints&, Ints,                              \
            const IntsList &,                                           \
            IntsList);                                                  \
  macroname(indexes, Indexes, const Indexes&, Indexes,                  \
            const IndexesList &,                                        \
            IndexesList);
#else
#define IMP_RMF_FOREACH_TYPE(macroname)         \
  macroname(type, Type, Type, Types,            \
            const Types &, Types);
#endif


#define IMP_RMF_BEGIN_OPERATION                 \
  try {

#define IMP_RMF_END_OPERATION(name)             \
  } catch (Exception &e) {                      \
    std::ostringstream oss;                     \
    oss << name;                                \
    e.set_operation_name(oss.str().c_str());    \
    throw;                                      \
  }

#define IMP_RMF_OPERATION(op, name)             \
  IMP_RMF_BEGIN_OPERATION                       \
  op;                                           \
  IMP_RMF_END_OPERATION(name)

#define IMP_RMF_BEGIN_FILE                      \
  try {

#define IMP_RMF_END_FILE(name)                  \
  } catch (Exception &e) {                      \
    std::ostringstream oss;                     \
    oss << name;                                \
    e.set_file_name(oss.str().c_str());         \
    throw;                                      \
  }

#define IMP_RMF_FILE(op, name)                  \
  IMP_RMF_BEGIN_FILE                            \
  op;                                           \
  IMP_RMF_END_FILE(name)


#define IMP_RMF_FILE_OPERATION(op, name, opname)        \
  IMP_RMF_BEGIN_FILE                                    \
  IMP_RMF_BEGIN_OPERATION                               \
  op;                                                   \
  IMP_RMF_END_FILE(name)                                \
  IMP_RMF_END_OPERATION(opname)                         \

/** Register a validator function. See Validator for more
    information.*/
#define IMP_RMF_VALIDATOR(Type)                 \
  RMF::Registrar<Type> Type##Reg(#Type);

namespace RMF {
#if !defined(SWIG)
  using std::vector;
#else
  template <class T>
  class vector{};
#endif

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
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
      t_= oss.str();
    }
    template <class T, class TT>
    Showable(const std::pair<T, TT> &p) {
      std::ostringstream oss;
      oss << "(" << p.first << ", " << p.second << ")";
      t_=oss.str();
    }
    Showable( std::string t): t_(t){}
    template <class T>
    Showable( const vector<T> &t ) {
      std::ostringstream out;
      out << "[";
      for (unsigned int i=0; i< t.size(); ++i) {
        if (i != 0) {
          out << ", ";
        }
        out << t[i];
      }
      out << "]";
      t_=out.str();
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
#define IMP_RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,\
                           hdf5_memory,hdf5_fill, null_value,           \
                           wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a)  \
  struct RMFEXPORT UCName##Traits:                                      \
    public internal::BaseTraits<UCName, UCNames, index> {               \
    static hid_t get_hdf5_disk_type() {                                 \
      return hdf5_disk;                                                 \
    }                                                                   \
    static hid_t get_hdf5_memory_type() {                               \
      return hdf5_memory;                                               \
    }                                                                   \
    static void write_value_dataset(hid_t d, hid_t is,                  \
                                    hid_t s,                            \
                                    UCName v) {                         \
      wv_ds;                                                            \
    }                                                                   \
    static UCName read_value_dataset(hid_t d, hid_t is,                 \
                                     hid_t sp){                         \
      UCName ret;                                                       \
      rv_ds;                                                            \
      return ret;                                                       \
    }                                                                   \
    static void write_values_dataset(hid_t d, hid_t is,                 \
                                     hid_t s,                           \
                                     const UCNames& v) {                \
      if (v.empty()) return;                                            \
      wvs_ds;                                                           \
    }                                                                   \
    static UCNames read_values_dataset(hid_t d, hid_t is,               \
                                       hid_t sp,                        \
                                       unsigned int sz) {               \
      UCNames ret(sz, get_null_value());                                \
      rvs_ds;                                                           \
      return ret;                                                       \
    }                                                                   \
    static void write_values_attribute(hid_t a, const UCNames &v) {     \
      if (v.empty()) return;                                            \
      wvs_a;                                                            \
    }                                                                   \
    static UCNames read_values_attribute(hid_t a, unsigned int sz) {    \
      UCNames ret(sz, get_null_value());                                \
      rvs_a;                                                            \
      return ret;                                                       \
    }                                                                   \
    static hid_t get_hdf5_fill_type() {                                 \
      return hdf5_fill;                                                 \
    }                                                                   \
    static const UCName& get_fill_value() {                             \
      return get_null_value();                                          \
    }                                                                   \
    static const UCName& get_null_value() {                             \
      static const UCName ret= null_value;                              \
      return ret;                                                       \
    }                                                                   \
    static bool get_is_null_value(const UCName& i) {                    \
      return i== get_null_value();                                      \
    }                                                                   \
    static std::string get_name() {return #lcname;}                     \
  }

/** Declare a type traits*/
#define IMP_RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory, \
                       hdf5_fill, null_value,                           \
                       wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a)      \
  IMP_RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory, \
                     hdf5_fill, null_value,                             \
                     wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a);       \
  struct UCNames##Traits:                                               \
    public internal::BaseTraits<UCNames, vector<UCNames>, index+7> {    \
    static hid_t get_hdf5_disk_type() {                                 \
      static IMP_HDF5_HANDLE(ints_type, H5Tvlen_create                  \
                             (UCName##Traits::get_hdf5_disk_type()),    \
                             H5Tclose);                                 \
      return ints_type;                                                 \
    }                                                                   \
    static hid_t get_hdf5_memory_type() {                               \
      static IMP_HDF5_HANDLE(ints_type,  H5Tvlen_create                 \
                             (UCName##Traits::get_hdf5_memory_type()),  \
                             H5Tclose);                                 \
      return ints_type;                                                 \
    }                                                                   \
    static void write_value_dataset(hid_t d, hid_t is,                  \
                                    hid_t s,                            \
                                    const UCNames& v) {                 \
      hvl_t data;                                                       \
      data.len=v.size();                                                \
      data.p= const_cast< UCName*>(&v[0]);                              \
      IMP_HDF5_CALL(H5Dwrite(d,                                         \
                             get_hdf5_memory_type(), is, s,             \
                             H5P_DEFAULT, &data));                      \
    }                                                                   \
    static UCNames read_value_dataset(hid_t d, hid_t is,                \
                                      hid_t sp) {                       \
      hvl_t data;                                                       \
      H5Dread (d, get_hdf5_memory_type(), is, sp, H5P_DEFAULT, &data);  \
      UCNames ret(data.len);                                             \
      std::copy(static_cast<UCName*>(data.p),                           \
                static_cast<UCName*>(data.p)+data.len,                  \
                ret.begin());                                           \
      free(data.p);                                                     \
      return ret;                                                       \
    }                                                                   \
    static void write_values_dataset(hid_t d, hid_t is,                 \
                                     hid_t s,                           \
                                     const vector<UCNames>& v){         \
      IMP_RMF_UNUSED(d); IMP_RMF_UNUSED(is); IMP_RMF_UNUSED(s);         \
      IMP_RMF_UNUSED(v);                                                \
      IMP_RMF_NOT_IMPLEMENTED;                                          \
    };                                                                  \
    static vector<UCNames> read_values_dataset(hid_t d, hid_t is,       \
                                               hid_t sp, unsigned int sz) { \
      IMP_RMF_UNUSED(d);                                                \
      IMP_RMF_UNUSED(is); IMP_RMF_UNUSED(sp); IMP_RMF_UNUSED(sz);       \
      IMP_RMF_NOT_IMPLEMENTED;                                          \
      return vector<UCNames>();                                         \
    }                                                                   \
    static vector<UCNames> read_values_attribute(hid_t a,               \
                                                 unsigned int size) {   \
      IMP_RMF_UNUSED(a);                                                \
      IMP_RMF_UNUSED(size);                                             \
      IMP_RMF_NOT_IMPLEMENTED;                                          \
      return vector<UCNames>();                                         \
    }                                                                   \
    static void write_values_attribute(hid_t a, const vector<UCNames> &v){ \
      IMP_RMF_UNUSED(a); IMP_RMF_UNUSED(v);                             \
      IMP_RMF_NOT_IMPLEMENTED;                                          \
    }                                                                   \
    static hid_t get_hdf5_fill_type() {                                 \
      return get_hdf5_memory_type();                                    \
    }                                                                   \
    static std::string get_name() {return UCName##Traits::get_name()+"s";} \
  };                                                                    \


/** Declare a type traits*/
#define IMP_RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, \
                              hdf5_memory, hdf5_fill, null_value)       \
  IMP_RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk,             \
                 hdf5_memory, hdf5_fill, null_value,                    \
                 IMP_HDF5_CALL(H5Dwrite(d,                              \
                                        get_hdf5_memory_type(), is, s,  \
                                        H5P_DEFAULT, &v)),              \
                 IMP_HDF5_CALL(H5Dread(d,                               \
                                       get_hdf5_memory_type(),          \
                                       is, sp, H5P_DEFAULT, &ret)),     \
                 IMP_HDF5_CALL(H5Dwrite(d,                              \
                                        get_hdf5_memory_type(), is, s,  \
                                        H5P_DEFAULT,                    \
                                        const_cast<UCName*>(&v[0]))),   \
                 IMP_HDF5_CALL(H5Dread(d,                               \
                                       get_hdf5_memory_type(),          \
                                       is, sp, H5P_DEFAULT, &ret[0])),  \
                 IMP_HDF5_CALL(H5Awrite(a, get_hdf5_memory_type(), &v[0])), \
                 IMP_HDF5_CALL(H5Aread(a, get_hdf5_memory_type(), &ret[0])) \
                 )


#else

#define IMP_RMF_TRAITS_ONE(UCName, UCNames, lcname, index, hdf5_disk,   \
                           hdf5_memory,                                 \
                           hdf5_fill, null_value,                       \
                           wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a)  \
  struct UCName##Traits{                                                \
    typedef UCName Type;                                                \
    typedef UCNames Types;                                              \
    static Type get_null_value();                                       \
  }

#define IMP_RMF_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, hdf5_memory, \
                       hdf5_fill, null_value,                           \
                       wv_ds, rv_ds, wvs_ds, rvs_ds, wvs_a, rvs_a)      \
  struct UCName##Traits{                                                \
    typedef UCName Type;                                                \
    typedef UCNames Types;                                              \
    static UCName get_null_value();                                     \
  };                                                                    \
  struct UCNames##Traits{                                               \
    typedef UCNames Type;                                               \
    static UCNames get_null_value();                                    \
  };

#define IMP_RMF_SIMPLE_TRAITS(UCName, UCNames, lcname, index, hdf5_disk, \
                              hdf5_memory, hdf5_fill, null_value)       \
  struct UCName##Traits{                                                \
    typedef UCName Type;                                                \
    typedef UCNames Types;                                              \
    static UCName get_null_value();                                     \
  };                                                                    \
  struct UCNames##Traits{                                               \
    typedef UCNames Type;                                               \
    static UCNames get_null_value();                                    \
  };

#endif


#include "internal/errors.h"

#endif  /* IMPLIBRMF_INFRASTRUCTURE_MACROS_H */
