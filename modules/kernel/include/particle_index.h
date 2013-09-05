/**
 *  \file IMP/kernel/particle_index.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_INDEX_H
#define IMPKERNEL_PARTICLE_INDEX_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <boost/shared_ptr.hpp>

IMPKERNEL_BEGIN_NAMESPACE

class Particle;
class Decorator;

/** Get the indexes from a list of particles.*/
IMPKERNELEXPORT ParticleIndexes get_indexes(const ParticlesTemp &ps);

/** Get the particles from a list of indexes.*/
IMPKERNELEXPORT ParticlesTemp get_particles(kernel::Model *m,
                                            const ParticleIndexes &ps);

/** Get the indexes from a list of particle pairs. */
IMPKERNELEXPORT ParticleIndexPairs get_indexes(const ParticlePairsTemp &ps);

/** Take Decorator, Particle or ParticleIndex. */
class IMPKERNELEXPORT ParticleIndexAdaptor
#ifndef SWIG
    // suppress swig warning that doesn't make sense and I can't make go away
    : public ParticleIndex, base::InputAdaptor
#endif
      {
 public:
  ParticleIndexAdaptor(Particle *p);
  ParticleIndexAdaptor(ParticleIndex pi) : ParticleIndex(pi) {}
  ParticleIndexAdaptor(const Decorator &d);
#if !defined(SWIG) && !defined(IMP_DOXYGEN) && !defined(IMP_SWIG_WRAPPER)
  ParticleIndexAdaptor(base::WeakPointer<Particle> p);
  ParticleIndexAdaptor(base::Pointer<Particle> p);
#endif
};

/** Take Decorator, Particle or ParticleIndex. */
class IMPKERNELEXPORT ParticleIndexesAdaptor
#ifndef SWIG
    // suppress swig warning that doesn't make sense and I can't make go away
  : public base::InputAdaptor
#else
  : public base::InputAdaptor
#endif
      {
        boost::shared_ptr<ParticleIndexes> tmp_;
        const ParticleIndexes *val_;
 public:
#if !defined(SWIG)
  template <class PS>
    ParticleIndexesAdaptor(const PS &ps): tmp_(new ParticleIndexes(ps.size())),
    val_(tmp_.get()) {
    for (unsigned int i = 0; i < ps.size(); ++i) {
      (*tmp_)[i] = ps[i].get_particle_index();
    }
  }
  ParticleIndexesAdaptor(const Particles &ps);
#endif
  ParticleIndexesAdaptor(const ParticlesTemp &ps);
  ParticleIndexesAdaptor(const ParticleIndexes& pi) : val_(&pi) {}
  ParticleIndexesAdaptor(): tmp_(new ParticleIndexes()), val_(tmp_.get()) {}
#ifndef SWIG
  typedef ParticleIndexes::value_type value_type;
  typedef ParticleIndexes::const_reference reference;
  typedef ParticleIndexes::const_pointer pointer;
  typedef ParticleIndexes::size_type size_type;
  typedef ParticleIndexes::const_reference const_reference;
  typedef ParticleIndexes::const_pointer const_pointer;
  operator const ParticleIndexes &() const {return *val_;}
  ParticleIndex operator[](unsigned int i) const {return (*val_)[i];}
  unsigned int size() const {return val_->size();}
  typedef ParticleIndexes::const_iterator const_iterator;
  typedef ParticleIndexes::const_iterator iterator;
  const_iterator begin() const {return val_->begin();}
  const_iterator end() const {return val_->end();}
#endif
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  ParticleIndexesAdaptor& operator=(const ParticleIndexesAdaptor &o) {
    tmp_ = o.tmp_;
    val_ = o.val_;
    return *this;
  }
  ParticleIndexesAdaptor(const ParticleIndexesAdaptor &o):
    tmp_(o.tmp_), val_(o.val_) {}
#endif
};


/** Take ParticlePairs or ParticleIndexPairs. */
class IMPKERNELEXPORT ParticleIndexPairsAdaptor
#ifndef SWIG
    // suppress swig warning that doesn't make sense and I can't make go away
  : public ParticleIndexPairs, base::InputAdaptor
#endif
      {
 public:
#if !defined(SWIG)
  template <class PS>
    ParticleIndexPairsAdaptor(const PS &ps) {
    resize(ps.size());
    for (unsigned int i = 0; i < ps.size(); ++i) {
      operator[](i) = ParticleIndexPair(ps[i][0].get_particle_index(),
                                        ps[i][1].get_particle_index());
    }
  }
#endif
  ParticleIndexPairsAdaptor(const ParticlePairsTemp &ps);
  ParticleIndexPairsAdaptor(const ParticleIndexPairs& pi)
    : ParticleIndexPairs(pi) {}
  ParticleIndexPairsAdaptor() {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_PARTICLE_INDEX_H */
