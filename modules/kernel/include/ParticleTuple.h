/**
 *  \file ParticleTuple.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_TUPLE_H
#define IMPKERNEL_PARTICLE_TUPLE_H

#include "Particle.h"
#include "Pointer.h"
#include "WeakPointer.h"
#include "internal/ParticleTupleBase.h"

IMP_BEGIN_NAMESPACE


template <unsigned int D>
class RefCountedParticleTuple;

//! A class to store a tuple of particles.
/** \note These do not ref counting currently. SWIG prevents
    use of OwnerPointer<Particle> as the storage type without some
    gymnastics.

    Only the constructor with the correct number of arguments for the
    dimensionality can be used.

    \note ParticleTuple objects are ordered.
*/
template <unsigned int D>
class ParticleTuple:
  public internal::ParticleTupleBase<D, WeakPointer<Particle> >
{
public:
  typedef internal::ParticleTupleBase<D, WeakPointer<Particle> > P;
  ParticleTuple(){
  }
  ParticleTuple(const RefCountedParticleTuple<D> &o): P(o){}
  ParticleTuple(Particle* x, Particle* y):P( x,y) {
  }
  ParticleTuple(Particle* x, Particle* y, Particle* z): P( x,y,z) {
  }
  ParticleTuple(Particle* x0, Particle* x1, Particle* x2, Particle* x3):
    P(x0, x1, x2, x3) {
  }
#if defined(IMP_DOXYGEN) || defined(SWIG)
  Particle * operator[](unsigned int i) const;
  Particle *& operator[](unsigned int i);
  std::size_t __hash__();
#endif
};

template <unsigned int D>
class RefCountedParticleTuple:
  public internal::ParticleTupleBase<D, Pointer<Particle> >
{
public:
  typedef internal::ParticleTupleBase<D, Pointer<Particle> > P;
  RefCountedParticleTuple(){
  }
  RefCountedParticleTuple(const ParticleTuple<D> &o): P(o){}
  RefCountedParticleTuple(Particle* x, Particle* y):P( x,y) {
  }
  RefCountedParticleTuple(Particle* x, Particle* y, Particle* z): P( x,y,z) {
  }
  RefCountedParticleTuple(Particle* x0, Particle* x1,
                          Particle* x2, Particle* x3):
    P(x0, x1, x2, x3) {
  }
#if defined(IMP_DOXYGEN) || defined(SWIG)
  Particle * operator[](unsigned int i) const;
  Particle *& operator[](unsigned int i);
  std::size_t __hash__();
#endif
};

#if !defined(IMP_DOXYGEN)

typedef ParticleTuple<2> ParticlePair;
IMP_VALUES(ParticlePair, ParticlePairsTemp);
typedef IMP::compatibility::checked_vector<RefCountedParticleTuple<2> >
ParticlePairs;
typedef ParticleTuple<3> ParticleTriplet;
IMP_VALUES(ParticleTriplet, ParticleTripletsTemp);
typedef IMP::compatibility::checked_vector<RefCountedParticleTuple<3> >
ParticleTriplets;
typedef ParticleTuple<4> ParticleQuad;
IMP_VALUES(ParticleQuad, ParticleQuadsTemp);
typedef IMP::compatibility::checked_vector<RefCountedParticleTuple<4> >
ParticleQuads;
#endif

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_PARTICLE_TUPLE_H */
