/**
 *  \file core_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
  IMP_IMPLEMENT(virtual ParticlesTemp propose_move(Float size));        \
  IMP_IMPLEMENT(virtual void reset_move());                             \
  IMP_OBJECT(Name)

/** Helper macro for implementing IMP::core::MonteCarlo. In
    addition to the IMP_OBJECT methods, it declares
    - IMP::core::MonteCarlo::do_step()
 */
#define IMP_MONTE_CARLO(Name)                           \
  IMP_IMPLEMENT(virtual void do_step());                \
  IMP_OBJECT(Name)

/** Helper macro for implementing
    IMP::core::ClosePairsFinder objects. In addition to the
    IMP_OBJECT methods it declares:
    - IMP::core::get_close_pairs()
*/
#define IMP_CLOSE_PAIRS_FINDER(Name)                                    \
  IMP_IMPLEMENT(ParticlePairsTemp get_close_pairs(const ParticlesTemp   \
                                                  &pa) const);          \
  IMP_IMPLEMENT(ParticlePairsTemp get_close_pairs(const ParticlesTemp &pa, \
                                                  const ParticlesTemp &pb) \
                const);                                                 \
  IMP_IMPLEMENT(ParticlesTemp get_input_particles(const ParticlesTemp &ps)\
                const);                                                 \
  IMP_IMPLEMENT(ContainersTemp get_input_containers(const ParticlesTemp &ps)\
                const);                                                 \
  IMP_IMPLEMENT(IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) \
                const);                                                 \
  IMP_IMPLEMENT(IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas, \
                                         const algebra::BoundingBox3Ds &bbs)\
                const);                                                 \
  using ClosePairsFinder::get_close_pairs;                              \
  IMP_NO_SWIG(IMP_NO_DOXYGEN(ParticlePairsTemp                          \
              get_close_pairs(SingletonContainer *pc) const {           \
    return get_close_pairs(pc->get_particles());                        \
  } /* swig doesn't support using */                                    \
  ParticlePairsTemp get_close_pairs(SingletonContainer *pca,            \
                                    SingletonContainer *pcb) {          \
  return get_close_pairs(pca->get_particles(),                          \
                         pcb->get_particles());                         \
  }))                                                                   \
  IMP_OBJECT(Name)

#endif  /* IMPCORE_MACROS_H */
