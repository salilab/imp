/**
 *  \file IMP/kernel/decorator_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECORATOR_MACROS_H
#define IMPKERNEL_DECORATOR_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include "particle_index.h"
#include "Particle.h"
#include "Decorator.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/warning_macros.h>

#ifdef IMP_DOXYGEN
/** \deprecated_at{2.1} Use IMP_DECORATOR_METHODS() instead.
*/
#define IMP_DECORATOR(Name, Parent)                                     \
  public:                                                               \
  /* Should be private but SWIG accesses it through the                 \
     comparison                                                         \
     macros*/ IMP_NO_DOXYGEN(                                           \
         typedef Parent ParentDecorator);                               \
  Name() : Parent() {}                                                  \
  Name(Model *m, ParticleIndex id) : Parent(m, id) {                    \
    IMP_INTERNAL_CHECK(                                                 \
                       get_is_setup(m->get_particle(id)),               \
        "Particle " << m->get_particle(id)->get_name()                  \
                       << " missing required attributes for decorator " \
                       << #Name                                         \
        << "\n" << base::ShowFull(m->get_particle(id)));                \
  }                                                                     \
  IMP_SHOWABLE(Name)

#else
#define IMP_DECORATOR(Name, Parent)                                     \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Use IMP_DECORATOR_METHODS()");       \
  public:                                                               \
  /* Should be private but SWIG accesses it through the                 \
     comparison                                                         \
     macros*/ IMP_NO_DOXYGEN(                                           \
         typedef Parent ParentDecorator);                               \
  Name() : Parent() {}                                                  \
  Name(Model *m, ParticleIndex id) : Parent(m, id) {                    \
    IMP_INTERNAL_CHECK(                                                 \
                       get_is_setup(m->get_particle(id)),               \
        "Particle " << m->get_particle(id)->get_name()                  \
                       << " missing required attributes for decorator "\
                       << #Name                                         \
        << "\n" << base::ShowFull(m->get_particle(id)));                \
  }                                                                     \
  explicit Name(::IMP::kernel::Particle *p) : Parent(p) {               \
    IMP_INTERNAL_CHECK(                                                 \
                       get_is_setup(p),                                 \
        "Particle " << p->get_name()                                    \
                       << " missing required attributes for decorator " \
                       << #Name                                         \
        << "\n" << base::ShowFull(p));                                  \
  }                                                                     \
  static bool get_is_setup(Particle *p) { return particle_is_instance(p);} \
  static bool get_is_setup(Model *m, ParticleIndex pi) {                \
    return particle_is_instance(m->get_particle(pi));                   \
  }                                                                     \
  static bool particle_is_instance(Model *m, ParticleIndex pi) {        \
    return particle_is_instance(m->get_particle(pi));                   \
  }                                                                     \
  static Name decorate_particle(::IMP::kernel::Particle *p) {           \
    IMP_CHECK_OBJECT(p);                                                \
    if (!get_is_setup(p)) {                                             \
      return Name();                                                    \
    }                                                                   \
    return Name(p);                                                     \
  }                                                                     \
  IMP_SHOWABLE(Name)
#endif

/** Implement the needed methods for a decorator based on
    - setup_particle()
    - get_is_setup()
    methods that you provide.
*/
#define IMP_DECORATOR_METHODS(Name, Parent)                             \
  public:                                                               \
  /* Should be private but SWIG accesses it through the                 \
     comparison                                                         \
     macros*/ IMP_NO_DOXYGEN(                                           \
                             typedef Parent ParentDecorator);           \
  Name() : Parent() {}                                                  \
  Name(::IMP::kernel::Model *m, ::IMP::kernel::ParticleIndex id) :      \
    Parent(m, id) {                                                     \
    IMP_INTERNAL_CHECK(                                                 \
                       get_is_setup(m, id),                             \
                       "Particle " << m->get_particle_name(id)          \
                       << " missing required attributes for decorator " \
                       << #Name);                                       \
  }                                                                     \
  explicit Name(const IMP::kernel::ParticleAdaptor &d) : Parent(d) {    \
    IMP_INTERNAL_CHECK(                                                 \
                       get_is_setup(d.get_model(), d.get_particle_index()), \
                       "Particle "                                      \
                       << d.get_model()                                 \
                       ->get_particle_name(d.get_particle_index())      \
                       << " missing required attributes for decorator " \
                       << #Name);                                       \
  }                                                                     \
  /** \deprecated_at{2.1} Check explicitly instead. */                  \
  static Name decorate_particle(::IMP::kernel::Particle *p) {           \
    IMP_CHECK_OBJECT(p);                                                \
    if (!get_is_setup(p->get_model(), p->get_index())) {                \
      return Name();                                                    \
    }                                                                   \
    return Name(p);                                                     \
  }                                                                     \
  /** \deprecated_at{2.1} Use get_is_setup() instead. */                \
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)                               \
  static bool particle_is_instance(::IMP::kernel::Particle *p) {        \
    IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_is_setup instead."); \
    return get_is_setup(p->get_model(), p->get_index());                \
  }                                                                     \
  /** \deprecated_at{2.1} Use get_is_setup() instead. */                \
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)                               \
  static bool particle_is_instance(IMP::kernel::Model *m,               \
                                   IMP::kernel::ParticleIndex pi) {     \
    IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_is_setup instead."); \
    return get_is_setup(m, pi);                                         \
  }                                                                     \
  static bool get_is_setup(const IMP::kernel::ParticleAdaptor &p) {     \
    return get_is_setup(p.get_model(), p.get_particle_index());         \
  }                                                                     \
  IMP_SHOWABLE(Name)



/** \deprecated_at{2.1} Use IMP_DECORATOR_WITH_TRAITS_METHODS()
 */
#define IMP_DECORATOR_WITH_TRAITS(Name, Parent, TraitsType, traits_name,      \
                                  default_traits)                             \
 private:                                                                     \
  TraitsType traits_;                                                         \
                                                                              \
 public:                                                                      \
 IMPKERNEL_DEPRECATED_MACRO(2.1, "Use IMP_DECORATOR_METHODS()");        \
  IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                             \
  Name() {}                                                                   \
  Name(Model *m, ParticleIndex id, const TraitsType &tr)                      \
      : Parent(m, id), traits_(tr) {                                          \
    IMP_INTERNAL_CHECK(                                                       \
                      get_is_setup(m->get_particle(id), tr),            \
        "Particle " << m->get_particle(id)->get_name()                        \
                      << " missing required attributes for decorator "  \
                      << #Name                                          \
                    << "\n" << Showable(m->get_particle(id)));                \
  }                                                                           \
  Name(const TraitsType &tr) : traits_(tr) {}                                 \
  explicit Name(::IMP::kernel::Particle *p,                                   \
                const TraitsType &tr = default_traits)                        \
      : Parent(p), traits_(tr) {                                              \
    IMP_INTERNAL_CHECK(                                                       \
                      get_is_setup(p, tr),                              \
                      "Particle " << p->get_name()                      \
                      << " missing required attributes "                \
                    << " for decorator " << #Name << "\n" << Showable(p));    \
  }                                                                           \
  static Name decorate_particle(::IMP::kernel::Particle *p,                   \
                                const TraitsType &tr = default_traits) {      \
   if (!get_is_setup(p, tr))                                            \
      return Name();                                                          \
    else                                                                      \
      return Name(p, tr);                                                     \
  }                                                                           \
  IMP_SHOWABLE(Name);                                                         \
  const TraitsType &get_##traits_name()                                       \
      const { return get_decorator_traits(); }                                \
  typedef Parent DecoratorTraitsBase;                                         \
  typedef TraitsType DecoratorTraits;                                         \
  const DecoratorTraits &get_decorator_traits() const { return traits_; }     \
  static const DecoratorTraits &get_default_decorator_traits() {              \
    static TraitsType dt = default_traits;                                    \
    return dt;                                                                \
  }                                                                           \
  IMP_NO_DOXYGEN(typedef boost::true_type DecoratorHasTraits)


/** Implement the needed methods for a decorator based on
    - setup_particle()
    - get_is_setup()
    methods that you provide.
*/
#define IMP_DECORATOR_WITH_TRAITS_METHODS(Name, Parent, TraitsType,     \
                                          traits_name, default_traits)  \
  private:                                                              \
  TraitsType traits_;                                                   \
public:                                                                 \
 typedef TraitsType DecoratorTraits;                                    \
 const DecoratorTraits &get_decorator_traits() const { return traits_; } \
 static const DecoratorTraits &get_default_decorator_traits() {         \
   static TraitsType dt = default_traits;                               \
   return dt;                                                           \
 }                                                                      \
 /* Should be private but SWIG accesses it through the                  \
    comparison                                                          \
    macros*/ IMP_NO_DOXYGEN(                                            \
                            typedef Parent ParentDecorator);            \
 IMP_NO_DOXYGEN(typedef boost::true_type DecoratorHasTraits);           \
 Name() : Parent() {}                                                   \
 Name(::IMP::kernel::Model *m, ::IMP::kernel::ParticleIndex id,         \
      const TraitsType &tr = default_traits) :                          \
   Parent(m, id), traits_(tr) {                                         \
   IMP_INTERNAL_CHECK(                                                  \
                      get_is_setup(m, id, tr),                          \
                      "Particle " << m->get_particle_name(id)           \
                      << " missing required attributes for decorator "  \
                      << #Name);                                        \
 }                                                                      \
 explicit Name(const IMP::kernel::ParticleAdaptor &d,                   \
      const TraitsType &tr = default_traits) : Parent(d),               \
                                               traits_(tr) {            \
   IMP_INTERNAL_CHECK(                                                  \
                      get_is_setup(d.get_model(), d.get_particle_index(), tr), \
                      "Particle "                                       \
                      << d.get_model()                                  \
                      ->get_particle_name(d.get_particle_index())       \
                      << " missing required attributes for decorator "  \
                      << #Name);                                        \
                                               }                        \
 /** \deprecated_at{2.1} Check explicitly instead. */                   \
 static Name decorate_particle(::IMP::kernel::Particle *p,              \
                               const TraitsType &tr = default_traits) { \
   IMP_CHECK_OBJECT(p);                                                 \
   if (!get_is_setup(p->get_model(), p->get_index(), tr)) {             \
     return Name();                                                     \
   }                                                                    \
   return Name(p);                                                      \
 }                                                                      \
 /** \deprecated_at{2.1} Use get_is_setup() instead. */                 \
 IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)                                \
 static bool particle_is_instance(::IMP::kernel::Particle *p,           \
                                  const TraitsType &tr = default_traits) { \
   IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_is_setup instead."); \
   return get_is_setup(p->get_model(), p->get_index(), tr);             \
 }                                                                      \
 /** \deprecated_at{2.1} Use get_is_setup() instead. */                 \
 IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)                                \
 static bool particle_is_instance(IMP::kernel::Model *m,                \
                                  IMP::kernel::ParticleIndex pi,        \
                                  const TraitsType &tr = default_traits) { \
   IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_is_setup instead."); \
   return get_is_setup(m, pi, tr);                                      \
 }                                                                      \
 static bool get_is_setup(const IMP::kernel::ParticleAdaptor &p,        \
                          const TraitsType &tr = default_traits) {      \
   return get_is_setup(p.get_model(), p.get_particle_index(), tr);      \
 }                                                                      \
 IMP_SHOWABLE(Name)

/** Decorators need to be able to be set up from Particles, ParticleIndexes
    and other Decorators. To help keep things uniform, we provide macros
    to declare the setup functions. These macros expect that an appropriate
    `do_setup_particle(Model *, ParticleIndex, args...)` function is
    defined. But any docs needed before the macro invocation.
*/
#define IMP_DECORATOR_SETUP_0(Name)                                     \
  static Name setup_particle(Model *m, ParticleIndex pi) {              \
    IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "                   \
                    << m->get_particle_name(pi)                         \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi);                                           \
    return Name(m, pi);                                                 \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator) {  \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index());              \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_1(Name, FirstArgumentType, first_argument_name) \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name) {   \
    IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "                   \
                    << m->get_particle_name(pi)                         \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi, first_argument_name);                      \
    return Name(m, pi);                                                 \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,    \
                             FirstArgumentType first_argument_name) {   \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index(),               \
                          first_argument_name);                         \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_2(Name, FirstArgumentType, first_argument_name, \
                              SecondArgumentType, second_argument_name) \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name) { \
    IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "                   \
                    << m->get_particle_name(pi)                         \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi, first_argument_name, second_argument_name); \
    return Name(m, pi);                                                 \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,    \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name) { \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index(),               \
                          first_argument_name, second_argument_name);   \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_3(Name, FirstArgumentType, first_argument_name, \
                              SecondArgumentType, second_argument_name, \
                              ThirdArgumentType, third_argument_name)   \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             ThirdArgumentType third_argument_name) {   \
    IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "                   \
                    << m->get_particle_name(pi)                         \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi, first_argument_name, second_argument_name, \
                      third_argument_name);                             \
    return Name(m, pi);                                                 \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,    \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             ThirdArgumentType third_argument_name) {   \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index(),               \
                          first_argument_name, second_argument_name,    \
                          third_argument_name);                         \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_4(Name, FirstArgumentType, first_argument_name, \
                              SecondArgumentType, second_argument_name, \
                              ThirdArgumentType, third_argument_name,   \
                              FourthArgumentType, fourth_argument_name) \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             ThirdArgumentType third_argument_name,     \
                             FourthArgumentType fourth_argument_name) { \
    IMP_USAGE_CHECK(!get_is_setup(m, pi), "Particle "                   \
                    << m->get_particle_name(pi)                         \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi, first_argument_name, second_argument_name, \
                      third_argument_name, fourth_argument_name);       \
    return Name(m, pi);                                                 \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,    \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             ThirdArgumentType third_argument_name,     \
                             FourthArgumentType fourth_argument_name) { \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index(),               \
                          first_argument_name, second_argument_name,    \
                          third_argument_name, fourth_argument_name);   \
  }



/** Decorators need to be able to be set up from Particles, ParticleIndexes
    and other Decorators. To help keep things uniform, we provide macros
    to declare the setup functions. These macros expect that an appropriate
    `do_setup_particle(Model *, ParticleIndex, args...)` function is
    defined. But any docs needed before the macro invocation.
*/
#define IMP_DECORATOR_TRAITS_SETUP_0(Name)                              \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(m, pi, tr);                                       \
    return Name(m, pi, tr);                                             \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor d,            \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(d.get_model(), d.get_particle_index(), tr);       \
    return Name(d.get_model(), d.get_particle_index(), tr);             \
  }
/** \see IMP_DECORATOR_TRAITS_SETUP_0() */
#define IMP_DECORATOR_TRAITS_SETUP_1(Name, FirstArgumentType,           \
                                     first_argument_name)               \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name,     \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(m, pi, first_argument_name, tr);                  \
    return Name(m, pi, tr);                                             \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor d,            \
                             FirstArgumentType first_argument_name,     \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(d.get_model(), d.get_particle_index(),            \
                      first_argument_name, tr);                         \
    return Name(d.get_model(), d.get_particle_index(), tr);             \
  }
/** \see IMP_DECORATOR_TRAITS_SETUP_0() */
#define IMP_DECORATOR_TRAITS_SETUP_2(Name,                              \
                                     FirstArgumentType, first_argument_name, \
                                     SecondArgumentType, second_argument_name) \
  static Name setup_particle(Model *m, ParticleIndex pi,                \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(m, pi, first_argument_name, second_argument_name, tr); \
    return Name(m, pi, tr);                                             \
  }                                                                     \
  static Name setup_particle(IMP::kernel::ParticleAdaptor d,            \
                             FirstArgumentType first_argument_name,     \
                             SecondArgumentType second_argument_name,   \
                             DecoratorTraits tr                         \
                             = get_default_decorator_traits()) {        \
    do_setup_particle(d.get_model(), d.get_particle_index(),            \
                      first_argument_name, second_argument_name, tr);   \
    return Name(d.get_model(), d.get_particle_index(), tr);             \
  }

//! Perform actions dependent on whether a particle has an attribute.
/** A common pattern is to check if a particle has a particular attribute,
    do one thing if it does and another if it does not. This macro implements
    that pattern. It requires that the method get_particle() return the
    particle being used.

    \param[in] AttributeKey The key for the attribute
    \param[in] Type The type for the attribute ("Int", "Float", "String")
    \param[in] has_action The action to take if the Particle has the attribute.
    The attribute value is stored in the variable VALUE.
    \param[in] not_has_action The action to take if the Particle does not have
    the attribute.
    \see IMP_DECORATOR_GET()
    \see IMP_DECORATOR_GET_SET()

*/
#define IMP_DECORATOR_GET(AttributeKey, Type, has_action, not_has_action)     \
  do {                                                                        \
    if (get_model()->get_has_attribute(AttributeKey, get_particle_index())) { \
      Type VALUE =                                                            \
          get_model()->get_attribute(AttributeKey, get_particle_index());     \
      has_action;                                                             \
    } else {                                                                  \
      not_has_action;                                                         \
    }                                                                         \
  } while (false)

//! Set an attribute, creating it if it does not already exist.
/** Another common pattern is to have an assumed value if the attribute
    is not there. Then, you sometimes need to set the value whether it
    is there or not.
    \see IMP_DECORATOR_GET()
    \see IMP_DECORATOR_GET_SET()
*/
#define IMP_DECORATOR_SET(AttributeKey, value)                                \
  do {                                                                        \
    if (get_model()->get_has_attribute(AttributeKey, get_particle_index())) { \
      get_model()->set_attribute(AttributeKey, get_particle_index(), value);  \
    } else {                                                                  \
      get_model()->add_attribute(AttributeKey, get_particle_index(), value);  \
    }                                                                         \
  } while (false)

//! Define methods for getting and setting a particular simple field
/** This macro defines methods to get and set a particular attribute.

   \param[in] name The lower case name of the attribute
   \param[in] AttributeKey The AttributeKey object controlling
   the attribute.
   \param[in] Type The type of the attribute (upper case).
   \param[in] ReturnType The type to return from the get.
   \see IMP_DECORATOR_GET()
   \see IMP_DECORATOR_SET()
*/
#define IMP_DECORATOR_GET_SET(name, AttributeKey, Type, ReturnType)      \
  ReturnType get_##name() const {                                        \
    return static_cast<ReturnType>(                                      \
                                   get_model()->get_attribute(AttributeKey, \
                                                     get_particle_index())); \
  }                                                                      \
  void set_##name(ReturnType t) {                                        \
    get_model()->set_attribute(AttributeKey, get_particle_index(), t);   \
  }                                                                      \
  IMP_REQUIRE_SEMICOLON_CLASS(getset##name)

//! Define methods for getting and setting an optional simple field.
/** See IMP_DECORATOR_GET_SET(). The difference is that here you can provide
    a default value to use if the decorator does not have the attribute.

    \param[in] name The lower case name of the attribute
    \param[in] AttributeKey The expression to get the required attribute key.
    \param[in] Type The type of the attribute (upper case).
    \param[in] ReturnType The type to return from the get.
    \param[in] default_value The value returned if the attribute is missing.
*/
#define IMP_DECORATOR_GET_SET_OPT(name, AttributeKey, Type, ReturnType, \
                                  default_value)                        \
  ReturnType get_##name() const {                                       \
    IMP_DECORATOR_GET(AttributeKey, Type,                               \
                      return static_cast<ReturnType>(VALUE),            \
                      return default_value);                            \
  }                                                                     \
  void set_##name(ReturnType t) { IMP_DECORATOR_SET(AttributeKey, t); } \
  IMP_REQUIRE_SEMICOLON_CLASS(getset_##name)

#define IMP_DECORATORS_DECL(Name, PluralName) \
  class Name;                                 \
  typedef IMP::base::Vector<Name> PluralName

#define IMP_DECORATORS_DEF(Name, PluralName)                   \
  /* needed so there is no ambiguity with operator->*/         \
  inline std::ostream &operator<<(std::ostream &out, Name n) { \
    n.show(out);                                               \
    return out;                                                \
  }

//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS(Name, PluralName, Parent) \
  IMP_DECORATORS_DECL(Name, PluralName);         \
  IMP_DECORATORS_DEF(Name, PluralName)

//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)   \
  /* needed so there is no ambiguity with operator->*/         \
  inline std::ostream &operator<<(std::ostream &out, Name n) { \
    n.show(out);                                               \
    return out;                                                \
  }                                                            \
  typedef IMP::base::Vector<Name> PluralName


//! Create a decorator that computes some sort of summary info on a set
/** Examples include a centroid or a cover for a set of particles.

    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
*/
#define IMP_SUMMARY_DECORATOR_DECL(Name, Parent, Members)            \
  class IMPCOREEXPORT Name : public Parent {                         \
    IMP_CONSTRAINT_DECORATOR_DECL(Name);                             \
    static void do_setup_particle(Model *m, ParticleIndex pi,           \
                                  const ParticleIndexes &pis);          \
    static void do_setup_particle(Model *m, ParticleIndex pi,           \
                                  Refiner *ref);                        \
  public:                                                               \
    IMP_DECORATOR_METHODS(Name, Parent);                                \
    IMP_DECORATOR_SETUP_1(Name, ParticleIndexesAdaptor, members);       \
    IMP_DECORATOR_SETUP_1(Name, Refiner*, refiner);                     \
    static bool get_is_setup(Model *m, ParticleIndex pi) {              \
      return m->get_has_attribute(get_constraint_key(), pi);            \
    }                                                                   \
    IMP_NO_DOXYGEN(typedef boost::false_type DecoratorHasTraits);    \
                                                                     \
   private:                                                          \
    /* hide set methods*/                                            \
    void set_coordinates() {};                                       \
    void set_coordinates_are_optimized() const {}                    \
    void set_coordinate() const {}                                   \
    void set_radius() const {}                                       \
  };                                                                 \
  IMP_DECORATORS(Name, Name##s, Parent##s)

/** See IMP_SUMMARY_DECORATOR_DECL()
    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
    \param[in] create_modifier the statements to create the modifier
    which computes the summary info. It should be called mod.
*/
#define IMP_SUMMARY_DECORATOR_DEF(Name, Parent, Members, create_modifier) \
  void Name::do_setup_particle(Model *m, ParticleIndex pi,              \
                               const ParticleIndexes &pis) {            \
    Refiner *ref = new FixedRefiner(IMP::kernel::get_particles(m, pis)); \
    create_modifier;                                                    \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(mod, new DerivativesToRefined(ref), m, pi);          \
  }                                                                     \
  void Name::do_setup_particle(Model *m, ParticleIndex pi, Refiner *ref) { \
    create_modifier;                                                    \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(mod, new DerivativesToRefined(ref), m, pi);          \
  }                                                                     \
  void Name::show(std::ostream &out) const {                            \
    out << #Name << " at " << static_cast<Parent>(*this);               \
  }                                                                     \
  IMP_CONSTRAINT_DECORATOR_DEF(Name)

#endif /* IMPKERNEL_DECORATOR_MACROS_H */
