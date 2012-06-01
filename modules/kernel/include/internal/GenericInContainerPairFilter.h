/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_GENERIC_IN_CONTAINER_PAIR_FILTER_H
#define IMPKERNEL_GENERIC_IN_CONTAINER_PAIR_FILTER_H

#include "../kernel_config.h"
#include "../PairPredicate.h"
#include "../pair_macros.h"

IMP_BEGIN_INTERNAL_NAMESPACE




/** A templated version of InContainerPairFilter.
 */
template <class Container>
class GenericInContainerPairFilter: public PairPredicate {
  Pointer<Container> c_;
public:
  GenericInContainerPairFilter( Container* c,
                                std::string name):
      PairPredicate(name),
      c_(c){}
  IMP_PAIR_PREDICATE(GenericInContainerPairFilter);
};

template <class Container>
inline GenericInContainerPairFilter<Container>*
create_in_container_filter(Container *c, std::string name=std::string()) {
  if (name==std::string()) {
    name= std::string("InContainer ") + c->get_name();
  }
  return new GenericInContainerPairFilter<Container>(c, name);
}





template <class C>
int GenericInContainerPairFilter<C>
::get_value(const ParticlePair& p) const {
  return c_->C::get_contains_particle_pair(p)
    || c_->C::get_contains_particle_pair(ParticlePair(p[1], p[0]));
}

template <class C>
ParticlesTemp GenericInContainerPairFilter<C>
::get_input_particles(Particle*) const {
  // not quite right
  return ParticlesTemp();
}
template <class C>
ContainersTemp GenericInContainerPairFilter<C>
::get_input_containers(Particle*) const {
  return ContainersTemp(1, c_);
}

template <class C>
void GenericInContainerPairFilter<C>
::do_show(std::ostream &out) const {
  out << "Filtering from container " << c_->get_name() << std::endl;
}


IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_GENERIC_IN_CONTAINER_PAIR_FILTER_H */
