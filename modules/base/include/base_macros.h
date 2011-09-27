/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_BASE_MACROS_H
#define IMPBASE_BASE_MACROS_H
#include "base_config.h"
#include <IMP/compatibility/checked_vector.h>

#ifdef IMP_DOXYGEN
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
#elif defined(SWIG)
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
#elif defined(__clang__)
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
#else
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)                       \
  IMP_NO_DOXYGEN(IMP_NO_SWIG(friend void dummy_f_##Name()))
#define IMP_REQUIRE_SEMICOLON_NAMESPACE void dummy_f()
#endif

#ifdef IMP_DOXYGEN
//! Hide something from doxygen
/** */
#define IMP_NO_DOXYGEN(x)
//! Only show something to doxygen
/** */
#define IMP_ONLY_DOXYGEN(x) x IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x,y) x
#else
#define IMP_NO_DOXYGEN(x) x
#define IMP_ONLY_DOXYGEN(x) IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x,y) y
#endif

#if defined(SWIG)
#define IMP_NO_SWIG(x)
#else
//! Hide the line when SWIG is compiled or parses it
/** */
#define IMP_NO_SWIG(x) x
#endif

#if defined(IMP_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
*/

//! Implement comparison in a class using a compare function
/** The compare function should take a const Name & and return -1, 0, 1 as
    appropriate.
*/
#define IMP_COMPARISONS(Name)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] field the first field to compare on
 */
#define IMP_COMPARISONS_1(Name, field)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] f0 the first field to compare on
    \param[in] f1 the second field to compare on
 */
#define IMP_COMPARISONS_2(Name, f0, f1)

//! Implement comparison in a class using field as the variable to compare
/** \param[in] Name the name of the class
    \param[in] f0 the first field to compare on
    \param[in] f1 the second field to compare on
    \param[in] f2 the third field to compare on
 */
#define IMP_COMPARISONS_3(Name, f0, f1, f2)
/** @} */
#elif defined(SWIG)
#define IMP_SWIG_COMPARISONS(Name)                                      \
  bool __eq__(const Name &o) const;                                     \
  bool __ne__(const Name &o) const;                                     \
  bool __lt__(const Name &o) const;                                     \
  bool __gt__(const Name &o) const;                                     \
  bool __ge__(const Name &o) const;                                     \
  bool __le__(const Name &o) const


#define IMP_COMPARISONS(Name)                   \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_1(Name, field)          \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_2(Name, f0, f1)         \
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_3(Name f0, f1, f2)           \
  IMP_SWIG_COMPARISONS(Name)

#else // not doxygen
#define IMP_SWIG_COMPARISONS(Name)                                      \
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
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(comparisons)

#define IMP_COMPARISONS(Name)                                           \
  IMP_SWIG_COMPARISONS(Name);                                           \
  bool operator==(const Name &o) const {                                \
    return (Name::compare(o) == 0);                                     \
  }                                                                     \
  bool operator!=(const Name &o) const {                                \
    return (Name::compare(o) != 0);                                     \
  }                                                                     \
  bool operator<(const Name &o) const {                                 \
    return (Name::compare(o) <0);                                       \
  }                                                                     \
  bool operator>(const Name &o) const {                                 \
    return (compare(o) > 0);                                            \
  }                                                                     \
  bool operator>=(const Name &o) const {                                \
    return !(Name::compare(o) < 0);                                     \
  }                                                                     \
  bool operator<=(const Name &o) const {                                \
    return !(Name::compare(o) > 0);                                     \
  }                                                                     \
  template <class T> friend int compare(const T&a, const T&b)

#define IMP_COMPARISONS_1(Name, field)          \
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
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_2(Name, f0, f1)         \
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
  IMP_SWIG_COMPARISONS(Name)

#define IMP_COMPARISONS_3(Name, f0, f1, f2)             \
  bool operator==(const Name &o) const {                \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);       \
  }                                                     \
  bool operator!=(const Name &o) const {                \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);     \
  }                                                     \
  bool operator<(const Name &o) const {                 \
    if (f0< o.f0) return true;                          \
    else if (f0 > o.f0) return false;                   \
    if (f1< o.f1) return true;                          \
    else if (f1 > o.f1) return false;                   \
    else return f2 < o.f2;                              \
  }                                                     \
  bool operator>(const Name &o) const {                 \
    if (f0 > o.f0) return true;                         \
    else if (f0 < o.f0) return false;                   \
    if (f1 > o.f1) return true;                         \
    else if (f1 < o.f1) return false;                   \
    else return f2 > o.f2;                              \
  }                                                     \
  bool operator>=(const Name &o) const {                \
    return operator>(o) || operator==(o);               \
  }                                                     \
  bool operator<=(const Name &o) const {                \
    return operator<(o) || operator==(o);               \
  }                                                     \
  int compare(const Name &o) const {                    \
    if (operator<(o)) return -1;                        \
    else if (operator>(o)) return 1;                    \
    else return 0;                                      \
  }                                                     \
  IMP_SWIG_COMPARISONS(Name)
#endif

#if !defined(SWIG)
/** \name Swap helpers

    Use the swap_with member function to swap two objects. The two
    objects mustbe of the same type (Name) and define
    the method \c swap_with(). The number suffix is the number of template
    arguments, all of which must be of class type.
    @{
*/
#define IMP_SWAP(Name)                                  \
  inline void swap(Name &a, Name &b) {a.swap_with(b);}  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_SWAP_1(Name)                                        \
  template <class A>                                            \
  inline void swap(Name<A> &a, Name<A> &b) {a.swap_with(b);}    \
  IMP_REQUIRE_SEMICOLON_NAMESPACE



#define IMP_SWAP_2(Name)                                \
  template <class A, class B>                           \
  inline void swap(Name<A,B> &a, Name<A,B> &b) {        \
    a.swap_with(b);                                     \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_SWAP_3(Name)                                \
  template <class A, class B, class C>                  \
  inline void swap(Name<A,B,C> &a, Name<A,B,C> &b) {    \
    a.swap_with(b);                                     \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

/** @} */
#else
#define IMP_SWAP(Name)

#define IMP_SWAP_1(Name)

#define IMP_SWAP_2(Name)

#define IMP_SWAP_3(Name)
#endif


//! Swap two member variables assuming the other object is called o
/** Swap the member \c var_name of the two objects (this and o).
 */
#define IMP_SWAP_MEMBER(var_name)               \
  swap(var_name, o.var_name)



//! Use a copy_from method to create a copy constructor and operator=
/** This macro is there to aid with classes which require a custom
    copy constructor. It simply forwards \c operator= and the copy
    constructor to a method \c copy_from() which should do the copying.

    You should think very hard before implementing a class which
    requires a custom copy custructor as it is easy to get wrong
    and you can easily wrap most resources with RAII objects
    (\external{http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization,
    wikipedia entry}).
*/
#define IMP_COPY_CONSTRUCTOR(Name, Base) Name(const Name &o): Base()    \
  {copy_from(o);}                                                       \
  IMP_NO_SWIG(Name& operator=(const Name &o) {copy_from(o); return *this;}) \
  IMP_REQUIRE_SEMICOLON_CLASS(copy)





#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Name)        \
  public:                                       \
  virtual ~Name(){}                             \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(SWIG)
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                \
  public:                                               \
  virtual ~Name(){}                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(IMP_DOXYGEN)
/* The destructor is unprotected for SWIG since if it is protected
   SWIG does not wrap the Python proxy distruction and so does not
   dereference the ref counted pointer. SWIG also gets confused
   on template friends.
*/
//! Ref counted objects should have private destructors
/** This macro defines a private destructor and adds the appropriate
    friend methods so that the class can be used with ref counting.
    By defining a private destructor, you make it so that the object
    cannot be declared on the stack and so must be ref counted.

    \see IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR()
    \see IMP::base::RefCounted
*/
#define IMP_REF_COUNTED_DESTRUCTOR(Name)


/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is only
    declared, not defined.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)

/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is declared
    inline.
*/
#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, destructor)


#else
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                                \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  virtual ~Name(){}                                                     \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)                   \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  virtual ~Name(){dest}                                                 \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)                     \
  protected:                                                            \
  template <class T, class E> friend struct IMP::base::internal::RefStuff; \
  virtual ~Name();                                                      \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)



#endif

#ifdef IMP_DOXYGEN
/** Implement operator[] for C++ and python. The index type is
    Index and the expression that returns the value is expr. If the
    bounds_check_expr is false, then a UsageException is thrown
    in C++ or and IndexException if called from python.
*/
#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)       \
  const Value operator[](Index) const;                           \
  Value& operator[](Index);

/** Implement operator[] for C++ and python. The index type is
    Index and the expression that returns the value is expr.
    The value returned is not mutable. If the
    bounds_check_expr is false, then a UsageException is thrown
    in C++ or and IndexException if called from python.
*/
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)   \
  const Value operator[](Index) const;                             \
  Value& operator[](Index);



#elif !defined(SWIG)
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)        \
  const Value& operator[](Index i) const {                              \
    IMP_USAGE_CHECK((bounds_check_expr), "Index out of range: "<< i);   \
    expr;                                                               \
  }                                                                     \
  const Value& __getitem__(Index i) const {                             \
    if (!(bounds_check_expr)) {                                         \
      IMP_THROW("Bad index " << i, IndexException);                     \
    }                                                                   \
    expr;                                                               \
  }                                                                     \

#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)              \
  Value& operator[](Index i)  {                                         \
    IMP_USAGE_CHECK((bounds_check_expr), "Index out of range: "<< i);   \
    expr;                                                               \
  }                                                                     \
  void __setitem__(Index i, const Value &v) {                           \
    operator[](i)=v;                                                    \
  }                                                                     \
  IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)



#else
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)        \
  const Value& __getitem__(Index i) const {                             \
    if (!(bounds_check_expr)) {                                         \
      IMP_THROW("Bad index " << i, IndexException);                     \
    }                                                                   \
    expr;                                                               \
  }

#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)              \
  void __setitem__(Index i, const Value &v) {                           \
    operator[](i)=v;                                                    \
  }                                                                     \
  IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)

#endif


#ifdef IMP_DOXYGEN
//! Define the type for storing sets of values
/** The macro defines the type Names. PluralName should be
    Names unless the English spelling is
    different. This macro also defines the output operator
    for the type.

    See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.
 */
#define IMP_VALUES(Name, PluralName)
#else
#define IMP_VALUES(Name, PluralName)                            \
  IMP_OUTPUT_OPERATOR(Name);                                    \
  typedef IMP::compatibility::checked_vector<Name> PluralName
#endif


#ifdef IMP_DOXYGEN
/** This is like IMP_VALUES() but for built in types that have
    now show.
 */
#define IMP_BUILTIN_VALUES(Name, PluralName)
#else
#define IMP_BUILTIN_VALUES(Name, PluralName)                     \
  typedef IMP::compatibility::checked_vector<Name> PluralName
#endif


#ifdef IMP_DOXYGEN
/** \name Showable
    Declare the methods needed by an object that can be printed,
    both from C++ and Python. Each value-type class should have an
    IMP_SHOWABLE() call internal to it and an IMP_OUTPUT_OPERATOR()
    call external to it.

    The suffixs are the number of template arguments that the
    object has (eg _1 means one template argument). _D means
    one integer template argument.
    @{
*/

/** This macro declares the method
    - void show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE().

    Do not use with IMP::Object objects as they have their
    own show mechanism.
*/
#define IMP_SHOWABLE(Name)

//! Declare the methods needed by an object that can be printed
/** This macro declares the method
    - \c void \c show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE()
*/
#define IMP_SHOWABLE_INLINE(Name, how_to_show)


//! Implement operator<< on class name, assuming it has one template argument
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_1(name)
//! Implement operator<< on class name, assuming it has two template arguments
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_2(name)
//! Implement operator<< on class name
/** The class named should define the method
    \c void \c show(std::ostream&).
*/
#define IMP_OUTPUT_OPERATOR(name)

//! Implement operator<< on class name templated by the dimension
/** The class named should define the method
    \c void \c show(std::ostream&).
*/
#define IMP_OUTPUT_OPERATOR_D(name)

//! Implement a hash function for the class
#define IMP_HASHABLE_INLINE(name, hashret)
/** @} */


#else
#define IMP_SHOWABLE(Name)                              \
  void show(std::ostream &out=std::cout) const;         \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#define IMP_SHOWABLE_INLINE(Name, how_to_show)          \
  void show(std::ostream &out=std::cout) const{         \
    how_to_show;                                        \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#define IMP_HASHABLE_INLINE(name, hashret)\
  std::size_t __hash__() const {          \
    hashret;                              \
  }

#if !defined(SWIG)
#define IMP_OUTPUT_OPERATOR_1(name)                                     \
  template <class L>                                                    \
  inline std::ostream& operator<<(std::ostream &out, const name<L> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_OUTPUT_OPERATOR_2(name)                                     \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_OUTPUT_OPERATOR(name)                                       \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_OUTPUT_OPERATOR_D(name)                                     \
  template < int D>                                                     \
  inline std::ostream &operator<<(std::ostream &out, const name<D> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_OUTPUT_OPERATOR_UD(name)                                     \
  template <unsigned int D>                                             \
  inline std::ostream &operator<<(std::ostream &out, const name<D> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#else
#define IMP_OUTPUT_OPERATOR_1(name)
#define IMP_OUTPUT_OPERATOR_2(name)
#define IMP_OUTPUT_OPERATOR(name)
#define IMP_OUTPUT_OPERATOR_D(name)
#define IMP_OUTPUT_OPERATOR_UD(name)
#endif
#endif

/** @} */


#ifdef IMP_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation.
 */
#define IMP_GRAPH(Name, type, VertexName, EdgeName)     \
  /** See \ref graphs "Graphs" for more information.*/  \
  typedef boost::graph Name

#elif defined(SWIG)
#define IMP_GRAPH(Name, type, VertexName, EdgeName)  class Name
#else
#define IMP_GRAPH(Name, type, VertexName, EdgeName)                \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
  boost::type##S,                                                       \
  boost::property<boost::vertex_name_t, VertexName>,                    \
  boost::property<boost::edge_name_t,                                   \
  EdgeName> > Name
#endif



/** Disable unused variable warning for a variable.
 */
#define IMP_UNUSED(variable) if (0) std::cout << variable;

#ifndef IMP_DOXYGEN
#ifdef __GNUC__
//! Use this to label a function with no side effects
/** \advanced */
#define IMP_NO_SIDEEFFECTS __attribute__ ((pure))
//! Use this to make the compiler (possibly) warn if the result is not used
/** \advanced */
#define IMP_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
//! restrict means that a variable is not aliased with this function
#define IMP_RESTRICT __restrict__
#define IMP_STRINGIFY(x) #x
#define IMP_WARN_PREPROCESS(msg) _Pragma(IMP_STRINGIFY(message #msg))

//#if __GNUC_PREREQ(4,2)
#define IMP_GCC_DISABLE_WARNING(name)\
_Pragma(IMP_STRINGIFY(GCC diagnostic ignored name))

/*#else
#define IMP_GCC_DISABLE_WARNING(name)
#endif*/

#else
#define IMP_NO_SIDEEFFECTS
#define IMP_WARN_UNUSED_RESULT
#define IMP_RESTRICT
#define IMP_WARN_PREPROCESS(message)
#define IMP_GCC_DISABLE_WARNING(name)
#endif

#endif



//! Declare a RAII-style class
/** Since such classes are typically quite small and simple, all
    the implementation is inline. The macro declares
    - default constructor
    - RAII::set()
    - RAII::reset()
    - destructor
    The Set and Reset arguments are the code to run in the respective
    functions. "args" are the argument string (in parens) for the
    constructor and set.
*/
#define IMP_RAII(Name, args, Initialize, Set, Reset, Show)              \
  Name() {Initialize;}                                                  \
  explicit Name args {Initialize; Set;}                                 \
  void set args {reset();                                               \
    Set;}                                                               \
  void reset() {Reset;}                                                 \
  ~Name () {reset();}                                                   \
  IMP_SHOWABLE_INLINE(Name, out << #Name << '('; Show; out << ')');     \
private:                                                                \
 void operator=(const Name &){}                                         \
 Name(const Name &){}                                                   \
public:                                                                 \
 IMP_REQUIRE_SEMICOLON_CLASS(raii)










//! Define the basic things needed by any Object
/** This defines
    - IMP::Object::get_version_info()
    - a private destructor
    and declares
    - IMP::Object::do_show()
*/
#define IMP_OBJECT(Name)                                                \
  public:                                                               \
  virtual std::string get_type_name() const {return #Name;}             \
  virtual ::IMP::base::VersionInfo get_version_info() const {           \
  return ::IMP::base::VersionInfo(get_module_name(),                    \
                                  get_module_version());                \
  }                                                                     \
  /** \brief For python, cast a generic Object to this type. Throw a
      ValueException of object is not the right type.*/                 \
static Name* get_from(IMP::base::Object *o) {                           \
  return IMP::base::object_cast<Name>(o);                               \
  }                                                                     \
IMP_NO_DOXYGEN(virtual void do_show(std::ostream &out) const);          \
IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, Object::_on_destruction();)



//! Define the basic things needed by any Object
/** This defines
    - IMP::base::Object::get_version_info()
    - a private destructor
    and declares
    - IMP::base::Object::do_show()
*/
#define IMP_OBJECT_INLINE(Name, show, destructor)                       \
  public:                                                               \
  virtual std::string get_type_name() const {return #Name;}             \
  virtual ::IMP::base::VersionInfo get_version_info() const {           \
    return ::IMP::base::VersionInfo(get_module_name(),                 \
                                     get_module_version());             \
  }                                                                     \
  /** \brief For python, cast a generic Object to this type. Throw a
      ValueException of object is not the right type.*/                 \
static Name* get_from(IMP::base::Object *o) {                           \
  return IMP::base::object_cast<Name>(o);                               \
  }                                                                     \
  IMP_NO_DOXYGEN (virtual void do_show(std::ostream &out) const {       \
      show;                                                             \
    });                                                                 \
  IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, Object::_on_destruction();     \
                                    destructor;)



//! Define the basic things needed by any internal Object
/** \see IMP_OBJECT
    This version also defines IMP::base::Object::do_show()
*/
#define IMP_INTERNAL_OBJECT(Name)                                       \
  public:                                                               \
  virtual ::IMP::base::VersionInfo get_version_info() const {           \
    return  ::IMP::base::VersionInfo(get_module_name(),                 \
                                     get_module_version());             \
  }                                                                     \
  virtual std::string get_type_name() const {                           \
    return #Name;                                                       \
  }                                                                     \
private:                                                                \
virtual void do_show(std::ostream & =std::cout) const {                 \
}                                                                       \
IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name,                                 \
                                   Object::_on_destruction();)


#ifdef IMP_DOXYGEN
//! Define the types for storing sets of objects
/** The macro defines the types PluralName and PluralNameTemp.
    PluralName should be Names unless the English spelling is
    different.
 */
#define IMP_OBJECTS(Name, PluralName)
#define IMP_OBJECTS_TYPEDEF(Name, PluralName)
#define IMP_OBJECTS_IO(Name, PluralName)
#else

#if IMP_BUILD < IMP_FAST
#define IMP_OBJECTS_TEMP_POINTER(Name) IMP::base::WeakPointer<Name>
#else
#define IMP_OBJECTS_TEMP_POINTER(Name) Name*
#endif

#ifdef SWIG
#define IMP_OBJECTS_IO(Name, PluralName)
#else
#define IMP_OBJECTS_IO(Name, PluralName)                                \
  inline std::ostream &operator<<(std::ostream &out,                    \
                                  const PluralName &os) {               \
    show_objects(os, out);                                              \
    return out;                                                         \
  }                                                                     \
  inline std::ostream& operator<<(std::ostream &out,                    \
                                  const PluralName##Temp &os) {         \
    show_objects(os, out);                                              \
    return out;                                                         \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE
#endif

#define IMP_OBJECTS_TYPEDEF(Name, PluralName)                           \
  typedef IMP::compatibility::checked_vector<IMP::base::Pointer<Name> > \
  PluralName;                                                           \
  typedef IMP::compatibility::checked_vector<IMP_OBJECTS_TEMP_POINTER(Name) > \
  PluralName##Temp;



#define IMP_OBJECTS(Name, PluralName)                           \
  IMP_OBJECTS_TYPEDEF(Name, PluralName);                        \
  IMP_OBJECTS_IO(Name, PluralName)
#endif


#endif  /* IMPBASE_BASE_MACROS_H */
