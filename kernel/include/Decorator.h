/**
 *  \file Decorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DECORATOR_H
#define IMP_DECORATOR_H

#include "Object.h"
#include "Pointer.h"
#include "utility.h"
#include "Particle.h"
#include "Model.h"
#include "internal/IndexingIterator.h"

IMP_BEGIN_NAMESPACE

/** A base class for decorators. To read more about decorators go to the
    \ref decorators "Decorator introduction".

    \note Decorator objects are ordered based on the address of the wrapped
    particle. Like pointers, they are logical values so can be in \c if
    statements.

    \cpp Implementers of decorators should just inherit from this and
    then use the IMP_DECORATOR macro to provide the key implementation
    pieces.\n\n Remember that attribute keys should always be created
    lazily (at the time of the first use), and not be created as
    static variables.\n\n Implementors should consult
    IMP::examples::Example, IMP_DECORATOR(),
    IMP_DECORATOR_TRAITS(), IMP_DECORATOR_GET(),
    IMP_DECORATOR_ARRAY_DECL()


    A Decorator can be cast to a Particle*.
    \see Decorators
    \see DecoratorsWithTraits
*/
class Decorator: public NullDefault, public Comparable
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
  IMP_NO_DOXYGEN(typedef Decorator This;)

  IMP_COMPARISONS_1(particle_);

 /** \name Methods provided by the Decorator class
      The following methods are provided by the Decorator class.
      @{
  */

  /** \return the particle wrapped by this decorator*/
  Particle *get_particle() const {
    IMP_check(particle_, "You must give the decorator a particle to decorate.",
              InvalidStateException);
    IMP_CHECK_OBJECT(particle_);
    return particle_;
  }

#ifndef IMP_DOXYGEN
#ifndef SWIG
  operator Particle*() const {
    return particle_;
  }
#endif
#endif

  /** \return the Model containing the particle */
  Model *get_model() const {
    IMP_CHECK_OBJECT(particle_->get_model());
    return particle_->get_model();
  }
  // here just to make the docs symmetric
private:
  IMP_ONLY_DOXYGEN(int blah_;)
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
template <class Decorator, class ParentDecorators>
class Decorators: public ParentDecorators {
  struct Accessor: public NullDefault {
    typedef Accessor This;
    typedef Decorator result_type;
    typedef unsigned int argument_type;
    result_type operator()(argument_type i) const {
      return o_->operator[](i);
    }
    Accessor(Decorators<Decorator,ParentDecorators> *pc): o_(pc){}
    Accessor(): o_(NULL){}
    IMP_COMPARISONS_1(o_);
  private:
    // This should be ref counted, but swig memory management is broken
    Decorators<Decorator,ParentDecorators>* o_;
  };



 public:
  typedef const Decorator const_reference;
  typedef Decorator value_type;
  typedef const Decorator reference;
  explicit Decorators(const Particles &ps): ParentDecorators(ps) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_check(Decorator::particle_is_instance(ps[i]), "Particle "
                << ps[i]->get_name() << " missing required attributes",
                ValueException);
    }
  }
  explicit Decorators(unsigned int i): ParentDecorators(i){}
  explicit Decorators(Decorator d): ParentDecorators(d){}
  Decorators(){}
  void push_back(Decorator d) {
    ParentDecorators::push_back(d);
  }
  void push_back(Particle *p) {
    IMP_check(Decorator::particle_is_instance(p),
              "Particle is missing required attributes",
              ValueException);
    ParentDecorators::push_back(p);
  }
  Decorator operator[](unsigned int i) const {
    return Decorator(ParentDecorators::operator[](i));
  }
  void set(unsigned int i, Decorator d) {
    IMP_check(i < ParentDecorators::size(), "Index out of range",
              IndexException);
    ParentDecorators::set(i, d);
  }
  Decorator back() const {
    IMP_check(!ParentDecorators::empty(),
              "Can't call back on empty Decorators",
              InvalidStateException);
    return Decorator(ParentDecorators::back());
  }
  Decorator front() const {
    IMP_check(!ParentDecorators::empty(),
              "Can't call front on empty Decorators",
              InvalidStateException);
    return Decorator(ParentDecorators::front());
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  typedef internal::IndexingIterator<Accessor> iterator;
  typedef internal::IndexingIterator<Accessor> const_iterator;
#else
  class iterator;
  class const_iterator;
#endif
  iterator begin() const {
    return iterator(Accessor(const_cast<Decorators<Decorator,
                             ParentDecorators>*>(this)), 0);
  }
  iterator end() const {
    return iterator(Accessor(const_cast<Decorators<Decorator,
                             ParentDecorators>*>(this)),
                    ParentDecorators::size());
  }
  template <class It>
  void insert(iterator loc, It b, It e) {
    for (It c=b; c!= e; ++c) {
      IMP_check(Decorator::particle_is_instance(*c), "Particle "
                << " missing required attributes",
                ValueException);
    }
    ParentDecorators::insert(ParentDecorators::begin()+(loc-begin()), b, e);
  }
#ifndef IMP_DOXYGEN
  void swap_with(Decorators &o) {
    ParentDecorators::swap_with(o);
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
template <class Decorator, class ParentDecorators, class Traits>
class DecoratorsWithTraits: public ParentDecorators {
  Traits tr_;
  bool has_traits_;
  struct Accessor: public NullDefault {
    typedef Accessor This;
    typedef Decorator result_type;
    typedef unsigned int argument_type;
    result_type operator()(argument_type i) const {
      return o_->operator[](i);
    }
    Accessor(DecoratorsWithTraits<Decorator,ParentDecorators,
             Traits> *pc): o_(pc){}
    Accessor(): o_(NULL){}
    IMP_COMPARISONS_1(o_);
  private:
    // This should be ref counted, but swig memory management is broken
    DecoratorsWithTraits<Decorator,ParentDecorators, Traits>* o_;
  };

public:
  typedef const Decorator const_reference;
  typedef Decorator value_type;
  typedef const Decorator reference;
  explicit DecoratorsWithTraits(Traits tr): tr_(tr), has_traits_(true){}
  explicit DecoratorsWithTraits(Decorator d): ParentDecorators(d),
                                              tr_(d.get_traits()),
                           has_traits_(true){}
  DecoratorsWithTraits(const Particles &ps,
                       Traits tr): tr_(tr), has_traits_(true) {
    ParentDecorators::resize(ps.size());
    for (unsigned int i=0; i< ps.size(); ++i) {
      ParentDecorators::set(i, Decorator(ps[i], tr));
    }
  }
  DecoratorsWithTraits(unsigned int i,
                       Traits tr): ParentDecorators(i), tr_(tr),
                                   has_traits_(true){}
  DecoratorsWithTraits(): has_traits_(false){}
  void push_back(Decorator d) {
    if (!has_traits_) {
      tr_= d.get_traits();
      has_traits_=true;
    } else {
      IMP_check(tr_ == d.get_traits(),
                "Traits don't match",
                ValueException);
    }
    ParentDecorators::push_back(d);
  }
  void push_back(Particle *p) {
    IMP_check(has_traits_, "Must set traits before adding particles",
              InvalidStateException);
    IMP_check(Decorator::particle_is_instance(p, tr_),
              "Particle is missing required attributes",
              ValueException);
    ParentDecorators::push_back(p);
  }
  Decorator operator[](unsigned int i) const {
    return Decorator(ParentDecorators::operator[](i), tr_);
  }
  void set(unsigned int i, Decorator d) {
    IMP_check(i < ParentDecorators::size(),
              "Index out of range", IndexException);
    if (!has_traits_) {
      tr_= d.get_traits();
      has_traits_=true;
    } else {
      IMP_check(tr_ == d.get_traits(),
                "Traits don't match",
                ValueException);
    }
    ParentDecorators::set(i, d);
  }
  Decorator back() const {
    IMP_check(!ParentDecorators::empty(),
              "Can't call back on empty Decorators",
              InvalidStateException);
    return Decorator(ParentDecorators::back(), tr_);
  }
  Decorator front() const {
    IMP_check(!ParentDecorators::empty(),
              "Can't call back on empty Decorators",
              InvalidStateException);
    return Decorator(ParentDecorators::front(), tr_);
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  typedef internal::IndexingIterator<Accessor> iterator;
  typedef internal::IndexingIterator<Accessor> const_iterator;
#else
  class iterator;
  class const_iterator;
#endif
  iterator begin() const {
    return iterator(Accessor(const_cast<DecoratorsWithTraits<Decorator,
                             ParentDecorators,
                             Traits>* >(this)),
                    0);
  }
  iterator end() const {
    return iterator(Accessor(const_cast<DecoratorsWithTraits<Decorator,
                             ParentDecorators,
                             Traits>* >(this)),
                    ParentDecorators::size());
  }
  template <class It>
  void insert(iterator loc, It b, It e) {
    if (b==e) return;
    if (!has_traits_) {
      tr_= b->get_traits();
      has_traits_=true;
    }
    for (It c=b; c!= e; ++c) {
      IMP_check(Decorator::particle_is_instance(*c, tr_), "Particle "
                << " missing required attributes",
                ValueException);
    }
    ParentDecorators::insert(ParentDecorators::begin()+(loc-begin()), b, e);
  }
#ifndef IMP_DOXYGEN
  void swap_with(DecoratorsWithTraits &o) {
    ParentDecorators::swap_with(o);
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
template <class D, class P>
class DecoratorsWithImplicitTraits: public P {
public:
  DecoratorsWithImplicitTraits() {}
  DecoratorsWithImplicitTraits(const Particles &ps):
    P(ps, D::get_traits()){
  }
  DecoratorsWithImplicitTraits(unsigned int i): P(i, D::get_traits()){}
  DecoratorsWithImplicitTraits(D d): P(d){}
  void swap_with(DecoratorsWithImplicitTraits &o) {
    P::swap_with(o);
  }
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

IMP_END_NAMESPACE

#endif  /* IMP_DECORATOR_H */
