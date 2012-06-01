/**
 *  \file core/generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_RESTRAINT_H
#define IMPKERNEL_CONTAINER_RESTRAINT_H

#include "../kernel_config.h"
#include "../base_types.h"
#include "../Restraint.h"
#include "create_decomposition.h"
#include "../restraint_macros.h"

IMP_BEGIN_INTERNAL_NAMESPACE
/** When programming in C++, you can use CoreRestraint instead
    of a SingletonsRestraint, PairsRestraint, etc. The result is
    somewhat faster (20% or so).
*/
template <class Score, class Container>
class ContainerRestraint : public Restraint
{
  IMP::OwnerPointer<Score> ss_;
  IMP::OwnerPointer<Container> pc_;

public:
  ContainerRestraint(Score *ss,
                     Container *pc,
                     std::string name="GroupnamesRestraint %1%");

  IMP_RESTRAINT(ContainerRestraint);

  //! Get the container used to store Particles
  typename Container::ContainedTypes
  get_arguments() const {
    return pc_->get();
  }

  Score* get_score() const {
    return ss_;
  }

  Restraints do_create_decomposition() const;
  Restraints do_create_current_decomposition() const;
};



/** Helper to create a ContainerRestraint without specifying the types. Make
    sure the score and container passed have their real type, not Container
    or PairScore.
    \relatesalso ContainerRestraint
*/
template <class Score, class Container>
inline Restraint *create_container_restraint(Score *s, Container*c,
                            std::string name=std::string()) {
  if (name==std::string()) {
    name= s->get_name()+ " and "+c->get_name();
  }
  return new ContainerRestraint<Score, Container>(s, c, name);
}



template <class Score, class C>
ContainerRestraint<Score, C>
::ContainerRestraint(Score *ss,
                     C *pc,
                     std::string name):
  Restraint(pc->get_model(), name),
  ss_(ss), pc_(pc) {

}
template <class Score, class C>
double ContainerRestraint<Score, C>
::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(ss_);
  IMP_CHECK_OBJECT(pc_);
  return pc_->template template_evaluate<Score>(ss_.get(), accum);
}


template <class Score, class C>
ParticlesTemp ContainerRestraint<Score, C>::get_input_particles() const
{
  IMP_OBJECT_LOG;
  ParticlesTemp ret
    = IMP::internal::get_input_particles(ss_.get(),
                                         pc_->get_all_possible_particles());
  return ret;
}

template <class Score, class C>
ContainersTemp ContainerRestraint<Score, C>
::get_input_containers() const
{
  ContainersTemp ret
    = IMP::internal::get_input_containers(ss_.get(),
                                          pc_->get_all_possible_particles());
  ret.push_back(pc_);
  return ret;
}


template <class Score, class C>
Restraints ContainerRestraint<Score, C>::do_create_decomposition() const {
  return IMP::internal::create_decomposition(get_model(),
                                             ss_.get(),
                                             pc_.get(),
                                             get_name());
}

template <class Score, class C>
Restraints
ContainerRestraint<Score, C>::do_create_current_decomposition() const {
  if (get_last_score()==0) return Restraints();
  return IMP::internal::create_current_decomposition(get_model(),
                                                     ss_.get(),
                                                     pc_.get(),
                                                     get_name());
}


template <class Score, class C>
void ContainerRestraint<Score, C>::do_show(std::ostream& out) const
{
  out << "score " << *ss_ << std::endl;
  out << "container " << *pc_ << std::endl;
}


IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_CONTAINER_RESTRAINT_H */
