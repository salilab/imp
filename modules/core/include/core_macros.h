/**
 *  \file core_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MACROS_H
#define IMPCORE_MACROS_H

/** Helper macro for implementing IMP::core::Mover. In
    addition to the IMP_OBJECT methods, it declares
    - IMP::core::Mover::propose_move()
    - IMP::core::Mover::reset_move()
 */
#define IMP_MOVER(Name)                                 \
  virtual void propose_move(Float size);                \
  virtual void reset_move();                            \
  IMP_OBJECT(Name)

/** Helper macro for implementing
    IMP::core::ClosePairsFinder objects. In addition to the
    IMP_OBJECT methods it declares:
    - IMP::core::get_close_pairs()
*/
#define IMP_CLOSE_PAIRS_FINDER(Name)                                    \
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pa) const;     \
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pa,            \
                                    const ParticlesTemp &pb) const;     \
  ParticlesTemp get_input_particles(const ParticlesTemp &ps) const;     \
  ContainersTemp get_input_containers(const ParticlesTemp &ps) const;   \
  IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const;   \
  IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,          \
                           const algebra::BoundingBox3Ds &bbs) const;   \
  ParticlePairsTemp get_close_pairs(SingletonContainer *pc) const {     \
    return get_close_pairs(pc->get_particles());                        \
  } /* swig doesn't support using */                                    \
  ParticlePairsTemp get_close_pairs(SingletonContainer *pca,            \
                                    SingletonContainer *pcb) {          \
  return get_close_pairs(pca->get_particles(),                          \
                         pcb->get_particles());                         \
  }                                                                     \
  IMP_OBJECT(Name)

#endif  /* IMPCORE_MACROS_H */
