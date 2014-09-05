/**
 *  \file IMP/kernel/Decorator.h    \brief The base class for decorators.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECORATOR_H
#define IMPKERNEL_DECORATOR_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <IMP/base/Object.h>
#include <IMP/base/WeakPointer.h>
#include "Model.h"
#include "Particle.h"
#include "utility.h"
#include "Constraint.h"
#include "internal/utility.h"
#include <IMP/base/utility_macros.h>
#include <IMP/base/Vector.h>
#include <IMP/base/Value.h>

IMPKERNEL_BEGIN_NAMESPACE
class ParticleAdaptor;

/**
Representation of the structure in \imp is via a collection of
Particle objects. However, since particles are general purpose, they
provide a basic set of tools for managing the data (e.g.
IMP::kernel::Model::add_attribute(), IMP::kernel::Model::get_value()
etc). Decorators wrap (or \quote{decorate}) particles to provide a much
richer interface. For example, most particles have Cartesian
coordinates. The class IMP::core::XYZ decorates such a particle to
provide functions to get and set the Cartesian coordinates as well as
compute distances between particles.
\code
d0= IMP.core.XYZ(p0)
d1= IMP.core.XYZ(p1)
print IMP.core.distance(d0,d1)
print d0.get_coordinates()
\endcode

\par Decorator basics

\note The `get_is_setup()` and `setup_particle()` functions mentioned below
can take any of either an IMP::kernel::Model* and IMP::kernel::ParticleIndex
pair, an IMP::kernel::Paricle* or another decorator to identify the particle.
We use various of those below.

Dealing with decorators and particles has two main parts
-# setting up the particle to be used with that decorator
-# decorating the particle.

To set up a particle to be used with the IMP::core::XYZ decorator we do
\code
d0= IMP.core.XYZ.setup_particle(m, pi, IMP.algebra.Vector3D(0,2,3))
\endcode
The method calls also decorates the particle and returns the decorator
which can now be used to manipulate the particle. For example we can
access the coordinates \c (0,2,3) by doing
\code
print d0.get_coordinates()
\endcode
We now say the particle is an XYZ particle. If that particle is
encountered later when we do not have the existing decorator available,
we can decorate it again (since it is already set up) by doing
\code
d1= IMP.core.XYZ(m, pi)
\endcode

If you do not know if \c p has been set up for the XYZ decorator, you can
ask with
\code
if IMP.core.XYZ.get_is_setup(m, pi):
\endcode

More abstractly, decorators can be used to
- maintain invariants: e.g. an IMP::atom::Bond particle always connects
  two other particles, both of which are IMP::atom::Bonded particles.
- add functionality: e.g. you can get the coordinates as an
  IMP::algebra::Vector3D
- provide uniform names for attributes: so you do not use \quote{x} some places
and \quote{X} other places

To see a list of all available decorators and to see what functions
all decorators have, look at the list of classes which inherit from
IMP::Decorator, below.

See the IMP::example::ExampleDecorator %example for how to implement a
simple decorator.

\note Decorator objects are ordered based on the address of the wrapped
particle. Like pointers, they are logical values so can be in \c if
statements.

\implementation{Decorator, IMP_DECORATOR, IMP::example::ExampleDecorator}
\n\n For efficiency reasons attribute keys should always be created
lazily (at the time of the first use), and not be created as static
variables. The reason for this is that initialized attribute keys result
in space being allocated in decorators, even before they are used.\n\n
Implementors should consult IMP::example::ExampleDecorator,
IMP_DECORATOR_METHODS(), IMP_DECORATOR_WITH_TRAITS_METHODS(),
IMP_DECORATOR_GET().

A decorator can be cast to a IMP::kernel::Particle* in C++. You have to
use the Decorator::get_particle() function in Python.

\note It is undefined behavior to use a decorator constructed on
a particle that is no longer part of a model. Since constructing
decorators is very cheap, you probably should not store decorators,
and then would not have this problem.

See example::ExampleDecorator to see what a minimal decorator looks like.
*/
class IMPKERNELEXPORT Decorator : public base::Value {
 private:
  base::WeakPointer<Model> model_;
  ParticleIndex pi_;
  bool is_valid_; // false if constructed with default constructor
  int compare(base::Object* o) const {
    if (o < get_particle())
      return -1;
    else if (o > get_particle())
      return 1;
    else
      return 0;
  }

 protected:
 Decorator(Model* m, ParticleIndex pi)
   : model_(m), pi_(pi), is_valid_(true) {}
 Decorator() : is_valid_(false) {}

#ifndef IMP_DOXYGEN
 public:
#endif
  explicit Decorator(ParticleAdaptor p);

 public:
#ifdef _MSC_VER
  typedef Particle* ParticleP;
#endif
#ifndef IMP_DOXYGEN
  bool __eq__(base::Object* o) const { return operator==(o); }
  bool __ne__(base::Object* o) const { return operator!=(o); }
  bool __lt__(base::Object* o) const { return operator<(o); }
  bool __gt__(base::Object* o) const { return operator>(o); }
  bool __ge__(base::Object* o) const { return operator>=(o); }
  bool __le__(base::Object* o) const { return operator<=(o); }
#ifndef SWIG
  bool operator==(base::Object* o) const { return (compare(o) == 0); }
  bool operator!=(base::Object* o) const { return (compare(o) != 0); }
  bool operator<(base::Object* o) const { return (compare(o) < 0); }
  bool operator>(base::Object* o) const { return (compare(o) > 0); }
  bool operator>=(base::Object* o) const { return !(compare(o) < 0); }
  bool operator<=(base::Object* o) const { return !(compare(o) > 0); }

  bool operator==(Particle* o) const { return (compare(o) == 0); }
  bool operator!=(Particle* o) const { return (compare(o) != 0); }
  bool operator<(Particle* o) const { return (compare(o) < 0); }
  bool operator>(Particle* o) const { return (compare(o) > 0); }
  bool operator>=(Particle* o) const { return !(compare(o) < 0); }
  bool operator<=(Particle* o) const { return !(compare(o) > 0); }
#endif
#endif

  //! Returns the particle decorated by this decorator.
  Particle* get_particle() const {
    if (!model_)
      return nullptr;
    else {
      IMP_USAGE_CHECK(model_->get_particle(pi_),
                      "Particle " << pi_ << " is no longer part of the model.");
      return model_->get_particle(pi_);
    }
  }

#if !defined(SWIG)
  operator Particle*() const { return get_particle(); }
  Particle* operator->() const { return get_particle(); }
  operator ParticleIndex() const { return get_particle_index(); }
#endif

  //! Returns the particle index decorated by this decorator.
  ParticleIndex get_particle_index() const { return pi_; }

  //! Returns the Model containing the particle.
  Model* get_model() const { return model_; }

  /** Returns true if the decorator was constructued with a non-default
      constructor.

      @note It is guaranteed that this is a very fast method.
            However, this method might return false if the particle itself
            was invalidated after construction - use check_particle()
            in that case.
  */
  bool get_is_valid() const { return is_valid_; }

  IMP_HASHABLE_INLINE(Decorator, return boost::hash_value(get_particle()););

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  typedef boost::false_type DecoratorHasTraits;
#endif
};

#ifndef IMP_DOXYGEN
/** Check that the particle satisfies invariants registered by decorators.
 */
IMPKERNELEXPORT void check_particle(Particle* p);
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_DECORATOR_H */
