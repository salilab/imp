/**
 *  \file IMP/container_base.h
 *  \brief Abstract base class for containers of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_CONTAINER_BASE_H
#define IMP_CONTAINER_BASE_H

#include "kernel_config.h"
#include "Object.h"
#include "VectorOfRefCounted.h"
#include "macros.h"
#include "WeakPointer.h"

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
IMP_OBJECTS(Particle,Particles);

template <unsigned int D>
class ParticleTuple;

class Container;
IMP_OBJECTS(Container,Containers);

//! Abstract class for containers of particles
/** Containers store sets of tuples of particles. The degree of the tuple
    (i.e. whether each tuple contains one, two, three or four
    particles) is constant for each container. That is, a
    SingletonContainer is a set of single particles, a PairContainer
    is a set of pairs of particles etc.

    These sets can come from a variety of sources, such as
    - user-provided lists, e.g. IMP::core::ListSingletonContainer
    - operations on other containers e.g. IMP::core::PairContainerSet
    - computations based on particle attributes
      e.g. IMP::Core::ClosePairContainer

    Containers provide a variety of methods to
    - get the number of tuples
    - get the ith tuple
    - iterate through the tuples
    - determine if a tuple is contained in the set

    \note If nothing uses the added and removed containers they may not
    be updated.

    \note Containers store \em sets and so are fundamentally unordered.
 */
class IMPEXPORT Container : public Object
{
  friend class Model;
  friend class Particle;
  WeakPointer<Model> m_;

 protected:
  bool is_ok(Particle *p);
  template <unsigned int D>
    bool is_ok(const ParticleTuple<D> &p) {
    for (unsigned int i=0; i< D; ++i) {
      if (!is_ok(p[i])) return false;
    }
    return true;
  }
  template <class It>
    bool is_ok(It b, It e) {
    for (; b!= e; ++b) {
      if (!is_ok(*b)) return false;
    }
    return true;
  }
  static Model *get_model(Particle *p);
  template <unsigned int D>
   static  Model *get_model(const ParticleTuple<D> &p) {
    return p[0]->get_model();
  }
  template <class It>
  static Model *get_model(It b, It e) {
    IMP_USAGE_CHECK(b != e,
                    "Cannot pass empty range to container constructor.");
    return get_model(*b);
  }
  bool get_has_model() const {
    return m_;
  }
  Container(Model *m, std::string name="Container %1%");
  // added or removed containers
  Container(std::string name="added or removed container %1%"): Object(name) {};
 public:
  //! Get whether the set of particles changed since last evaluation
  virtual bool get_contained_particles_changed() const=0;
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticlesTemp get_contained_particles() const=0;

  Model *get_model() const {return m_;}

#ifndef IMP_DOXYGEN
  virtual bool get_is_up_to_date() const=0;
#endif

  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMP_END_NAMESPACE

#endif  /* IMP_CONTAINER_BASE_H */
