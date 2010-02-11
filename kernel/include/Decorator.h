/**
 *  \file Decorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DECORATOR_H
#define IMP_DECORATOR_H

#include "Object.h"
#include "Pointer.h"
#include "utility.h"
#include "Particle.h"
#include "Constraint.h"
#include "Model.h"
#include "internal/IndexingIterator.h"

IMP_BEGIN_NAMESPACE

/**
Representation of the structure in \imp is via a collection of
Particle objects. These provide a very simple set of tools for
managing the data. Decorators wrap (or Decorator) particles to provide
a much richer interface to make management of representation
easier. For example, most particles have Cartesian coordinates. The
IMP::core::XYZ decorator provides functions to get and set the
Cartesian coordinates as well as compute distances between particles.

    \code
    d0= IMP.core.XYZ(p0)
    d1= IMP.core.XYZ(p1)
    print IMP.core.distance(d0,d1)
    print d0.get_coordinates()
    \endcode

\par Decorator basics

If you want to use a particular particle, \c p, to represent a piece
of the model with coordinates, you first have to add the
coordinates. To do this, we setup the particle to have the coordinates
0,2,3 by

\code
d0= IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0,2,3))
\endcode

We can now say the particle is an XYZ particle. And that particle can
now be decorated by doing

\code
d0= IMP.core.XYZ(p)
\endcode
If you do not know if \c p is an XYZ particle, you can ask by doing
\code
if IMP.core.XYZ.particle_is_instance(p):
\endcode

More abstractly, decorators can be used to

- maintain invariants: e.g. each of the XYZ particles have all of
  x,y,z coordinates

- add functionality: e.g. you can get the coordinates as an IMP::Vector3D

- provide uniform names for attributes: so you don't use "x" some places
and "X" other places

- cache keys since those can be expensive to create


To see a list of all available decorators and to see what functions
all decorators have, look the list of classes which inherit from
IMP::Decorator, below.

See the IMP::examples::ExampleDecorator example for how to implement a
simple decorator.

\note Decorator objects are ordered based on the address of the wrapped
particle. Like pointers, they are logical values so can be in \c if
statements.

\cpp Implementers of decorators should just inherit from this and then
use the IMP_DECORATOR() macro to provide the key implementation
pieces.\n\n Remember that attribute keys should always be created
lazily (at the time of the first use), and not be created as static
variables.\n\n Implementors should consult IMP::examples::Example,
IMP_DECORATOR(), IMP_DECORATOR_TRAITS(), IMP_DECORATOR_GET(),
IMP_DECORATOR_ARRAY_DECL()

\advanceddoc Lists of decorators behave like lists of \ref values
"objects rather than values" even though decorators themselves are
values. This means that, eg, an IMP::core::XYZs reference counts the
particles, where as an IMP::core::XYZ does not. For more efficiency
you can use the non-reference counted version, IMP::core::XYZsTemp
instead. This should only
be done when it is known to be safe. If you can't figure out
that it is, don't do it.

    A Decorator can be cast to a Particle*.
    \see Decorators
    \see DecoratorsWithTraits

    \ingroup null_default
    \ingroup comparable
*/
class Decorator
{
private:
  Particle *particle_;
#ifndef SWIG
  friend bool operator==(Decorator, Particle*);
#endif
protected:
  Decorator(Particle *p): particle_(p) {}
  Decorator() :particle_(NULL)
  {}
public:
#ifdef _MSC_VER
  // needed to get Particle in VectorOfRefCounted
  typedef Particle* ParticleP;
#endif

  IMP_NO_DOXYGEN(typedef Decorator This;)

  IMP_COMPARISONS_1(particle_);

  /** \name Methods provided by the Decorator class
      The following methods are provided by the Decorator class.
      @{
  */

  /** \return the particle wrapped by this decorator*/
  Particle *get_particle() const {
    IMP_USAGE_CHECK(particle_,
                    "You must give the decorator a particle to decorate.",
                    InvalidStateException);
    IMP_CHECK_OBJECT(particle_);
    return particle_;
  }

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  operator Particle*() const {
    return particle_;
  }
  Particle* operator->() const {
    return particle_;
  }
#endif

  /** \return the Model containing the particle */
  Model *get_model() const {
    IMP_CHECK_OBJECT(particle_->get_model());
    return particle_->get_model();
  }
  // here just to make the docs symmetric
private:
  IMP_ONLY_DOXYGEN(int blah_;);
  //! @}
public:
#ifdef IMP_DOXYGEN

  /** \name Methods that all decorators must have
      All decorators must have the following methods. Decorators
      which are parameterized (for example IMP::core::XYZR)
      take an (optional) extra parameter after the Particle in
      setup_particle(), cast() and particle_is_instance().
      Note that these are
      not actually methods of the Decorator class itself.
      @{
  */
  /** Add the needed attributes to the particle and initialize them
      with values taken from initial_values.

      It is an error to call this twice on the same particle for
      the same type of decorator.
  */
  static Decorator setup_particle(Particle *p, extra_arguments);

  /** Create a decorator from a particle which has already had
      Decorator::setup_particle() called on it.

      \return The Decorator(p) if p has been set up or Decorator() if not.
  */
  static Decorator decorate_particle(Particle *p);

  /** Return true if the particle can be cast to the decorator. */
  static bool particle_is_instance(Particle *p);

  /** Write a description of the wrapped Particle as seen by
      the decorator to a stream.
  */
  void show(std::ostream &out) const;

  /** Create an instance of the Decorator from the particle has
      already been setup.  The key difference between this constructor
      and decorate_particle() is that there is not necessarily any
      error checking performed.
  */
  Decorator(Particle *p);
  /** The default constructor must be defined and create a NULL decorator,
      analogous to a \c NULL pointer in C++ or a \c None object in Python.
  */
  Decorator();
  //! @}
#endif
  IMP_NO_DOXYGEN(bool is_null() const {return !particle_;});
  IMP_NO_DOXYGEN(typedef void (Decorator::*bool_type)() const;)
  IMP_NO_DOXYGEN(void safe_bool_function() const {})

};


#ifndef IMP_DOXYGEN
inline bool operator==(Decorator d, Particle *p) {
  return d.particle_==p;
}
inline bool operator==(Particle *p, Decorator d) {
  return d==p;
}
#endif

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#define IMP_DECORATORS_METHODS(test, on_add_decorator, on_add_particle, \
                               swap)                                    \
  struct Accessor {                                                     \
  typedef Accessor This;                                                \
  typedef WrappedDecorator result_type;                                 \
  typedef unsigned int argument_type;                                   \
  result_type operator()(argument_type i) const {                       \
    return o_->operator[](i);                                           \
  }                                                                     \
  Accessor(ThisDecorators *pc): o_(pc){}                                \
  Accessor(): o_(NULL){}                                                \
  IMP_COMPARISONS_1(o_);                                                \
private:                                                                \
/* This should be ref counted, but swig memory management
   is broken */                                                         \
ThisDecorators* o_;                                                     \
};                                                                      \
void check(Particle *p) {                                               \
  IMP_USAGE_CHECK(test,                                                 \
                  "Particle \"" << (p)->get_name()                      \
                    << "\" missing required attributes",                \
                  ValueException);                                      \
}                                                                       \
template <class It>                                                     \
void check(It b, It e) {                                                \
  for (It c= b; c!= e; ++c) {                                           \
    check(*c);                                                          \
  }                                                                     \
}                                                                       \
public:                                                                 \
typedef const WrappedDecorator const_reference;                         \
typedef WrappedDecorator value_type;                                    \
typedef Proxy reference;                                                \
const ParticlesTemp &get_particles() const {return *this;}              \
void push_back(WrappedDecorator d) {                                    \
  on_add_decorator;                                                     \
  ParentDecorators::push_back(d);                                       \
}                                                                       \
void push_back(Particle *p) {                                           \
  check(p);                                                             \
  on_add_particle;                                                      \
  ParentDecorators::push_back(p);                                       \
}                                                                       \
void set(unsigned int i, WrappedDecorator d) {                          \
  ParentDecorators::operator[](i)= d;                                   \
}                                                                       \
WrappedDecorator back() const {                                         \
  IMP_USAGE_CHECK(!ParentDecorators::empty(),                           \
                  "Can't call back on empty Decorators",                \
                  InvalidStateException);                               \
  return WrappedDecorator(ParentDecorators::back());                    \
}                                                                       \
WrappedDecorator front() const {                                        \
  IMP_USAGE_CHECK(!ParentDecorators::empty(),                           \
                  "Can't call front on empty Decorators",               \
                  InvalidStateException);                               \
  return WrappedDecorator(ParentDecorators::front());                   \
}                                                                       \
typedef internal::IndexingIterator<Accessor> iterator;                  \
typedef internal::IndexingIterator<Accessor> const_iterator;            \
iterator begin() const {                                                \
  return iterator(Accessor(const_cast<ThisDecorators*>(this)), 0);      \
}                                                                       \
iterator end() const {                                                  \
  return iterator(Accessor(const_cast<ThisDecorators*>(this)),          \
                  ParentDecorators::size());                            \
}                                                                       \
template <class It>                                                     \
void insert(iterator loc, It b, It e) {                                 \
  check(b,e);                                                           \
  for (It c=b; c!= e; ++c) {                                            \
    on_add_particle;                                                    \
  }                                                                     \
  ParentDecorators::insert(ParentDecorators::begin()+(loc-begin()),     \
                           b, e);                                       \
}                                                                       \
void swap_with(ThisDecorators &o) {                                     \
  swap;                                                                 \
  ParentDecorators::swap_with(o);                                       \
}                                                                       \

#elif defined(SWIG)
#define IMP_DECORATORS_METHODS(test, on_add_decorator, on_add_particle, \
                               swap)                                    \
  public:                                                               \
  const ParticlesTemp &get_particles() const;                           \
  void push_back(WrappedDecorator d);                                   \
  void push_back(Particle *p);                                          \
  WrappedDecorator back() const;                                        \
  WrappedDecorator front() const;

#else
// doxygen
#define IMP_DECORATORS_METHODS(test, on_add_decorator, on_add_particle, \
                               swap)                                    \
  public:                                                               \
  typedef const WrappedDecorator const_reference;                       \
  typedef WrappedDecorator value_type;                                  \
  typedef Proxy reference;                                              \
  const ParticlesTemp &get_particles() const;                           \
  void push_back(WrappedDecorator d);                                   \
  void push_back(Particle *p);                                          \
  WrappedDecorator &operator[](unsigned int i);                         \
  WrappedDecorator operator[](unsigned int i) const;                    \
  WrappedDecorator back() const;                                        \
  WrappedDecorator front() const;                                       \
  class const_iterator;                                                 \
  iterator begin() const;                                               \
  iterator end() const;                                                 \
  void insert(iterator loc, It b, It e);

#endif
/** A collection of Decorator objects. It supports construction
    from a collection of particles. The interface should be that of
    a std::vector or python list, with the exception that changing
    elements in the container must be done using Decorators::set().
    Any other differences are due to laziness on our part and should
    be reported.

    In general, a Decorators is convertable to a Decorators of a
    parent type of the Decorator or to a Particles.

    \see Decorator
    \see DecoratorsWithTraits
*/
template <class WrappedDecorator, class ParentDecorators>
class Decorators: public ParentDecorators {
  typedef Decorators<WrappedDecorator, ParentDecorators> ThisDecorators;
  struct Proxy: public WrappedDecorator {
    typedef typename ParentDecorators::reference Ref;
    Ref d_;
    Proxy(Ref t):
      WrappedDecorator(t), d_(t){
    }
    Proxy(Ref p, bool): WrappedDecorator(), d_(p){}
    void operator=(WrappedDecorator v) {
      WrappedDecorator::operator=(v);
      d_=v;
    }
#ifdef _MSC_VER
    // for VC, it can't otherwise figure out the conversion chain
    operator Particle*() {
      if (WrappedDecorator()==*this) return NULL;
      else return WrappedDecorator::get_particle();
    }
#endif
  };
  Proxy get_proxy(unsigned int i) {
    if (ParentDecorators::operator[](i)) {
      return Proxy(ParentDecorators::operator[](i));
    } else {
      return Proxy(ParentDecorators::operator[](i), false);
    }
  }

  IMP_DECORATORS_METHODS(WrappedDecorator::particle_is_instance(p),,,)
  public:
  explicit Decorators(const Particles &ps): ParentDecorators(ps) {
    check(ps.begin(), ps.end());
  }
  explicit Decorators(const ParticlesTemp &ds): ParentDecorators(ds){
    check(ds.begin(), ds.end());
  }
  explicit Decorators(unsigned int i): ParentDecorators(i){}
  explicit Decorators(WrappedDecorator d): ParentDecorators(1, d){}
  explicit Decorators(unsigned int n,
                      WrappedDecorator d): ParentDecorators(n, d){}
  Decorators(){}
#ifndef SWIG
#ifndef IMP_DOXYGEN
  Proxy
  operator[](unsigned int i) {
    return get_proxy(i);
  }
#else
  WrappedDecorator& operator[](unsigned int i);
#endif
#endif

#ifndef SWIG
  WrappedDecorator operator[](unsigned int i) const {
    return WrappedDecorator(ParentDecorators::operator[](i));
  }
#endif
};

#ifndef IMP_DOXYGEN
IMP_SWAP_2(Decorators);
#endif

/** A version for decorators which required traits. See Decorators
    for more full docs.

    A DecoratorsWithTraits can be cast to a Particles or its parent
    type. All decorators in it must have the same traits.
    \see Decorators*/
template <class WrappedDecorator, class ParentDecorators, class Traits>
class DecoratorsWithTraits: public ParentDecorators {
  typedef DecoratorsWithTraits<WrappedDecorator, ParentDecorators,
                               Traits> ThisDecorators;

  struct Proxy: public WrappedDecorator {
    typedef typename ParentDecorators::reference Ref;
    Ref d_;
    Proxy(Ref t, Traits tr):
      WrappedDecorator(t, tr), d_(t){
    }
    Proxy(Ref p, bool): WrappedDecorator(), d_(p){}
    void operator=(WrappedDecorator v) {
      // traits should match, but not checked
      WrappedDecorator::operator=(v);
      d_=v;
    }
#ifdef _MSC_VER
    // for VC, it can't otherwise figure out the conversion chain
    operator Particle*() {
      if (WrappedDecorator()==*this) return NULL;
      else return WrappedDecorator::get_particle();
    }
#endif
  };
  Proxy get_proxy(unsigned int i, Traits t) {
    if (ParentDecorators::operator[](i)) {
      return Proxy(ParentDecorators::operator[](i), t);
    } else {
      return Proxy(ParentDecorators::operator[](i), false);
    }
  }
  Traits tr_;
  bool has_traits_;
  IMP_DECORATORS_METHODS(WrappedDecorator::particle_is_instance(p, tr_),{
      if (!has_traits_) {
        tr_= d.get_traits();
        has_traits_=true;
      } else {
        IMP_USAGE_CHECK(tr_ == d.get_traits(),
                        "Traits don't match",
                        ValueException);
      }
    },{
      IMP_USAGE_CHECK(has_traits_, "Need to add a decorator first to get "
                      << "traits class.", UsageException);
    }, {
      std::swap(tr_, o.tr_);
      std::swap(has_traits_, o.has_traits_);
    })
  public:
  explicit DecoratorsWithTraits(Traits tr): tr_(tr), has_traits_(true){}
  explicit DecoratorsWithTraits(WrappedDecorator d): ParentDecorators(1,d),
                                              tr_(d.get_traits()),
                                              has_traits_(true){}
  explicit DecoratorsWithTraits(unsigned int n, WrappedDecorator d):
    ParentDecorators(n, d),
    tr_(d.get_traits()),
    has_traits_(true) {}
  DecoratorsWithTraits(const Particles &ps,
                       Traits tr): tr_(tr), has_traits_(true) {
    ParentDecorators::resize(ps.size());
    for (unsigned int i=0; i< ps.size(); ++i) {
      ParentDecorators::operator[](i)=WrappedDecorator(ps[i], tr);
    }
  }
  DecoratorsWithTraits(unsigned int i,
                       Traits tr): ParentDecorators(i), tr_(tr),
                                   has_traits_(true){}
  DecoratorsWithTraits(): has_traits_(false){}

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  Proxy
  operator[](unsigned int i) {
    IMP_USAGE_CHECK(has_traits_, "Can only use operator[] on a decorator "
                    << "container "
                    << "which is non-empty. This is a bug, but hard to fix.",
                    UsageException);
    return get_proxy(i, tr_);
  }
#else
  IMP_NO_SWIG(WrappedDecorator& operator[](unsigned int i));
#endif

#ifndef SWIG
  WrappedDecorator operator[](unsigned int i) const {
    return WrappedDecorator(ParentDecorators::operator[](i), tr_);
  }
#endif
};

#ifndef IMP_DOXYGEN
IMP_SWAP_3(DecoratorsWithTraits);
#endif

#ifndef IMP_DOXYGEN
/** A version for decorators which provide traits for their parents.

    A DecoratorsWithTraits can be cast to a Particles or its parent
    type. All decorators in it must have the same traits.
    \see Decorators*/
template <class WrappedDecorator, class ParentDecorators>
class DecoratorsWithImplicitTraits: public ParentDecorators {
  struct Proxy: public WrappedDecorator {
    typedef typename ParentDecorators::reference Ref;
    Ref d_;
    Proxy(Ref t):
      WrappedDecorator(t), d_(t){
    }
    Proxy(Ref p, bool): WrappedDecorator(), d_(p){}
    void operator=(WrappedDecorator v) {
      // traits should match, but not checked
      WrappedDecorator::operator=(v);
      d_=v;
    }
#ifdef _MSC_VER
    // for VC, it can't otherwise figure out the conversion chain
    operator Particle*() {
      if (WrappedDecorator()==*this) return NULL;
      else return WrappedDecorator::get_particle();
    }
#endif
  };
  Proxy get_proxy(unsigned int i) {
    if (ParentDecorators::operator[](i)) {
      return Proxy(ParentDecorators::operator[](i));
    } else {
      return Proxy(ParentDecorators::operator[](i), false);
    }
  }
  typedef DecoratorsWithImplicitTraits<WrappedDecorator, ParentDecorators>
  ThisDecorators;
  IMP_DECORATORS_METHODS(WrappedDecorator::particle_is_instance(p),,,)
  public:
  explicit DecoratorsWithImplicitTraits():
  ParentDecorators(WrappedDecorator::get_traits()) {}
  explicit DecoratorsWithImplicitTraits(const Particles &ps):
    ParentDecorators(ps, WrappedDecorator::get_traits()){
  }
  explicit DecoratorsWithImplicitTraits(unsigned int i):
    ParentDecorators(i, WrappedDecorator::get_traits()){}
  explicit DecoratorsWithImplicitTraits(WrappedDecorator d):
    ParentDecorators(1,d){}
  explicit DecoratorsWithImplicitTraits(unsigned int n,WrappedDecorator d):
    ParentDecorators(n, d){}
  explicit DecoratorsWithImplicitTraits(const ParticlesTemp &ds):
    ParentDecorators(ds, WrappedDecorator::get_traits()){
    check(ds.begin(), ds.end());
  }
#ifndef SWIG
#ifndef IMP_DOXYGEN
  Proxy
  operator[](unsigned int i) {
    return get_proxy(i);
  }
#else
  WrappedDecorator& operator[](unsigned int i);
#endif
#endif

#ifndef SWIG
  WrappedDecorator operator[](unsigned int i) const {
    return WrappedDecorator(ParentDecorators::operator[](i));
  }
#endif
};

IMP_SWAP_2(DecoratorsWithImplicitTraits);


/** A class to add ref counting to a decorator */
template <class D>
class RefCountingDecorator: public D {
public:
  RefCountingDecorator(){}
  RefCountingDecorator(const D &d): D(d){internal::ref(D::get_particle());}
  ~RefCountingDecorator(){ internal::unref(D::get_particle());}
#ifndef SWIG
  void operator=(const D &d) {
    if (*this) {
      internal::unref(D::get_particle());
    }
    D::operator=(d);
    if (*this) {
      internal::ref(D::get_particle());
    }
  }
#endif
};
#endif


#ifndef IMP_DOXYGEN
#define IMP_CONSTRAINT_DECORATOR_DECL(Name)                             \
  private:                                                              \
  static ObjectKey get_constraint_key();                                \
  static void set_constraint(SingletonModifier* before,                 \
                             SingletonModifier *after, Particle *p);    \
public:                                                                 \
 Constraint *get_constraint() const {                                   \
   return dynamic_cast<Constraint*>(get_particle()                      \
                                    ->get_value(get_constraint_key())); \
 }


#define IMP_CONSTRAINT_DECORATOR_DEF(Name)                              \
  ObjectKey Name::get_constraint_key() {                                \
    static ObjectKey ret(#Name " score state");                         \
    return ret;                                                         \
  }                                                                     \
  void Name::set_constraint(SingletonModifier* before,                  \
                            SingletonModifier *after,                   \
                            Particle *p) {                              \
    Constraint *ss= new SingletonConstraint(before,                     \
                                            after, p,                   \
                      std::string(#Name "updater for ")+p->get_name()); \
    p->add_attribute(get_constraint_key(), ss);                         \
    p->get_model()->add_score_state(ss);                                \
  }                                                                     \

#endif

IMP_END_NAMESPACE

#endif  /* IMP_DECORATOR_H */
