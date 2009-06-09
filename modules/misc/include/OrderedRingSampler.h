/**
 *  \file OrderedRingSampler.h
    \brief Ordered ring sampler
 *
 */
#ifndef IMPMISC_ORDERED_RING_SAMPLER_H
#define IMPMISC_ORDERED_RING_SAMPLER_H

//#include "IMP/IMP_config.h"
#include <IMP/Particle.h>
#include <map>
#include  <sstream>
#include "IMP/domino/DiscreteSampler.h"
#include "IMP/base_types.h"
#include "RingSampler.h"
IMPMISC_BEGIN_NAMESPACE

//! Sample an ordered ring (for ring of N particles, N samples)
class IMPMISCEXPORT OrderedRingSampler :  public RingSampler
{
  //! Samples all registrations between a ring of components to a
  //! ring of anchor points.
public:
  //! Costructor
  /**
    \param[in]  aps a set of anchor points that represent the centoirds of
                components of the ring. It is assumed that the anchor points
                are stored in the correctr ring order in the subset.
    \param[in]  ps  The particles that build the ring. It is assmumed that
                    the order of the vector is the one of the ring.
  */
  OrderedRingSampler(domino::DiscreteSet *aps, Particles *ps);
  ~OrderedRingSampler(){}

void populate_states_of_particles(Particles *particles,
                   std::map<std::string, domino::CombState *> *states) const;

  /*long number_of_states(Particles *particles) const {
  return aps_->get_number_of_states();
  }*/
  void show(std::ostream& out = std::cout) const;
protected:
  void construct();
  std::vector<domino::CombState *> all_comb_;

};
IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_ORDERED_RING_SAMPLER_H */
