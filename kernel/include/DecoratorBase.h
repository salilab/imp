/**
 *  \file DecoratorBase.h    \brief The base class for decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DECORATOR_BASE_H
#define IMP_DECORATOR_BASE_H

#include "Object.h"
#include "Pointer.h"
#include "Particle.h"

IMP_BEGIN_NAMESPACE

/** \defgroup decorators Decorators

Decorators wrap particles
- maintain invariants: e.g. any the particles have all of x,y,z coordinates
- add functionality: e.g. you can get the coordinates as an IMP::Vector3D
- provide uniform names for attributes: so you don't use "x" some places
and "X" other places
- cache keys since those can be expensive to create

\note In general, you should not access particle attributes except
through decorators


The general usage of the decorators is quite simple
- Create a decorator around a particle which does not have the needed
attributes. This adds the attributes.
\verbatim
d= IMP.core.MyDecorator.create(p)
\endverbatim
- Cast a particle which has the required attributes to a decorator,
checking that it has the attributes
\verbatim
d= IMP.core.MyDecorator.cast(p)
\endverbatim
- Wrap a particle which is known to have the required attributes in a
decorator. No checks are necessarily made
\verbatim
d= IMP.core.MyDecorator(p)
\endverbatim
- Access and manipulate attributes or groups of attributes through
methods like
\verbatim
x=d.get_foo()
d.set_foo(1.0)
\endverbatim
- When needed, access the keys used by the decorator through methods like
\verbatim
d.get_foo_key()
\endverbatim

 */



/** A base class for decorators. Implementers of decorators should
    just inherit from this and then use the IMP_DECORATOR macro
*/
class DecoratorBase
{
protected:
  Pointer<Particle> particle_;
  DecoratorBase(Particle *p): particle_(p) {}
  bool is_default() const {
    return !particle_;
  }
  static bool has_required_attributes(Particle *) {
    return true;
  }
  static void add_required_attributes(Particle *) {}
  static void decorator_initialize_static_data() {}
public:
  typedef DecoratorBase This;

  DecoratorBase() {}
  IMP_COMPARISONS_1(particle_);

  /** \return the particle wrapped by this decorator*/
  Particle *get_particle() const {
    return particle_.get();
  }
  /** \return the Model containing the particle */
  Model *get_model() const {
    IMP_CHECK_OBJECT(particle_->get_model());
    return particle_->get_model();
  }

  //! create a decorator of class D from the Particle
  /**
     This is mostly for use through the D::cast function
   */
  template <class D>
  static D cast(Particle *p) {
    IMP_CHECK_OBJECT(p);
    D:: decorator_initialize_static_data();
    if (!D::has_required_attributes(p)) {
      throw InvalidStateException("Particle missing required attributes"\
                                  " in cast");
    }
    return D(p);
  }

  //! create a decorator of class D from the Particle
  /**
     This is mostly for use through the D::create function
   */
  template <class D>
  static D create(Particle *p) {
    IMP_CHECK_OBJECT(p);
    D::decorator_initialize_static_data();
    D::add_required_attributes(p);
    return D(p);
  }

};

IMP_END_NAMESPACE

#endif  /* IMP_DECORATOR_BASE_H */
