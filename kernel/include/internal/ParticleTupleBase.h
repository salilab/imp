/**
 *  \file ParticleTuple.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_PARTICLE_TUPLE_BASE_H
#define IMP_INTERNAL_PARTICLE_TUPLE_BASE_H

#include "../Particle.h"
#include <boost/array.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE


//! A class to store a tuple of particles.
/** \note These do not ref counting currently. SWIG prevents
    use of OwnerPointer<Particle> as the storage type without some
    gymnastics.

    Only the constructor with the correct number of arguments for the
    dimensionality can be used.

    \note ParticleTuple objects are ordered.
*/
template <unsigned int D, class Pointer>
class ParticleTupleBase
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
: public boost::array<Pointer, D>
#endif
{
  typedef boost::array<Pointer, D> P;
  int compare(const ParticleTupleBase<D, Pointer> &o) const {
    for (unsigned int i=0;i<D; ++i) {
      if (P::operator[](i) < o[i]) return -1;
      else if (P::operator[](i) > o[i]) return 1;
    }
    return 0;
  }
public:

  typedef ParticleTupleBase<D, Pointer> This;
  static unsigned int get_dimension() {return D;};
  ParticleTupleBase(){
  }
  template <class OP>
  ParticleTupleBase(const ParticleTupleBase<D, OP> &o) {
    for (unsigned int i=0; i< D; ++i) {
      P::operator[](i)=o[i];
    }
  }
  ParticleTupleBase(Particle* x, Particle* y) {
    IMP_USAGE_CHECK(D==2, "Need " << D << " to construct a "
              << D << "-tuple.");
    P::operator[](0) = x;
    P::operator[](1) = y;
  }
  ParticleTupleBase(Particle* x, Particle* y, Particle* z) {
    IMP_USAGE_CHECK(D==3, "Need " << D << " to construct a "
              << D << "-tuple.");
    P::operator[](0) = x;
    P::operator[](1) = y;
    P::operator[](2) = z;
  }
  ParticleTupleBase(Particle* x0, Particle* x1, Particle* x2, Particle* x3) {
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
  IMP_HASHABLE_INLINE(ParticleTupleBase<D>, std::size_t seed = 0;
               for (unsigned int i=0; i< D; ++i) {
                 boost::hash_combine(seed,
                                     P::operator[](i));
               }
               return seed;);
  IMP_COMPARISONS(This);
  std::string get_name() const {
    std::string ret="\"";
    for (unsigned int i=0; i< D; ++i) {
      if (i>0) {
        ret+= "\" and \"";
      }
      ret+=P::operator[](i)->get_name();
    }
    ret+="\"";
    return ret;
  }
  IMP_SHOWABLE_INLINE(ParticleTupleBase, {
      out << get_name();
    });
#if defined(IMP_DOXYGEN) || defined(SWIG)
  Particle * operator[](unsigned int i) const;
  Particle *& operator[](unsigned int i);
#endif
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template<unsigned int D, class Pointer>
std::ostream &operator<<(std::ostream &out,
                         const ParticleTupleBase<D, Pointer> &d) {
  d.show(out);
  return out;
}
#endif

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_PARTICLE_TUPLE_BASE_H */
