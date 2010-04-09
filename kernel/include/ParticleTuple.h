/**
 *  \file ParticleTuple.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_PARTICLE_TUPLE_H
#define IMP_PARTICLE_TUPLE_H

#include "Particle.h"
#include <boost/array.hpp>

IMP_BEGIN_NAMESPACE


//! A class to store a tuple of particles.
/** \note These do not ref counting currently. SWIG prevents
    use of internal::OwnerPointer<Particle> as the storage type without some
    gymnastics.

    Only the constructor with the correct number of arguments for the
    dimensionality can be used.

    \note ParticleTuple objects are ordered.
*/
template <unsigned int D>
class ParticleTuple
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
: public boost::array<Particle*, D>
#endif
{
  typedef boost::array<Particle*, D> P;
  int compare(const ParticleTuple<D> &o) const {
    for (unsigned int i=0;i<D; ++i) {
      if (P::operator[](i) < o[i]) return -1;
      else if (P::operator[](i) > o[i]) return 1;
    }
    return 0;
  }
public:
  static const unsigned int get_dimension() {return D;};
  typedef ParticleTuple<D> This;
  ParticleTuple(){
    for (unsigned int i=0; i< D; ++i) {P::operator[](i)=NULL;}
  }
  ParticleTuple(Particle* x, Particle* y) {
    IMP_USAGE_CHECK(D==2, "Need " << D << " to construct a "
              << D << "-tuple.");
    P::operator[](0) = x;
    P::operator[](1) = y;
  }
  ParticleTuple(Particle* x, Particle* y, Particle* z) {
    IMP_USAGE_CHECK(D==3, "Need " << D << " to construct a "
              << D << "-tuple.");
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
  }
  ParticleTuple(Particle* x0, Particle* x1, Particle* x2, Particle* x3) {
    IMP_USAGE_CHECK(D==4, "Need " << D << " to construct a "
              << D << "-tuple.");
    P::operator[](0) = x0;
    P::operator[](1) = x1;
    P::operator[](2) = x2;
    P::operator[](3) = x3;
  }
  Particle *get(unsigned int i) const {
    return P::operator[](i);
  }
  IMP_COMPARISONS;
  std::string get_name() const {
    bool first=true;
    std::string ret;
    for (unsigned int i=0; i< D; ++i) {
      if (!first) {
        ret+= " and ";
        first=false;
      }
      ret+=P::operator[](i)->get_name();
    }
    return ret;
  }
  IMP_SHOWABLE_INLINE({
      out << get_name();
    });
#if defined(IMP_DOXYGEN) || defined(SWIG)
  Particle * operator[](unsigned int i) const;
  Particle *& operator[](unsigned int i);
#endif
};

IMP_OUTPUT_OPERATOR_D(ParticleTuple);

#if !defined(IMP_DOXYGEN)

template <unsigned int D>
struct RefCountParticleTuple {
  template <class O>
  static void ref(O o) {
    for (unsigned int i=0; i< D; ++i) {
      internal::ref(o[i]);
    }
  }
  template <class O>
  static void unref(O o) {
    for (unsigned int i=0; i< D; ++i) {
      internal::unref(o[i]);
    }
  }
};

typedef ParticleTuple<2> ParticlePair;
typedef std::vector<ParticleTuple<2> > ParticlePairsTemp;
typedef VectorOfRefCounted<ParticleTuple<2>,
                           RefCountParticleTuple<2> > ParticlePairs;
typedef ParticleTuple<3> ParticleTriplet;
typedef std::vector<ParticleTuple<3> > ParticleTripletsTemp;
typedef VectorOfRefCounted<ParticleTuple<3>,
                           RefCountParticleTuple<3> > ParticleTriplets;
typedef ParticleTuple<4> ParticleQuad;
typedef std::vector<ParticleTuple<4> > ParticleQuadsTemp;
typedef VectorOfRefCounted<ParticleTuple<4>,
                           RefCountParticleTuple<4> > ParticleQuads;
#endif

IMP_END_NAMESPACE

#endif  /* IMP_PARTICLE_TUPLE_H */
