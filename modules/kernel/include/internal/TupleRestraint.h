/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_TUPLE_RESTRAINT_H
#define IMPKERNEL_TUPLE_RESTRAINT_H

#include "../Constraint.h"
#include "../Restraint.h"
#include "container_helpers.h"
#include "singleton_helpers.h"
#include "pair_helpers.h"
#include "triplet_helpers.h"
#include "quad_helpers.h"

IMP_BEGIN_INTERNAL_NAMESPACE

template <class Score>
class TupleRestraint : public Restraint
{
  IMP::OwnerPointer<Score> ss_;
  typename Score::IndexArgument v_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Groupname and the Groupname.
  */
  TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name="TupleRestraint %1%");

  Score* get_score() const {return ss_;}
  typename Score::Argument get_argument() const {
    return get_particle(Restraint::get_model(),
                                  v_);
  }

  IMP_RESTRAINT(TupleRestraint);
};


template <class Score>
TupleRestraint<Score>
::TupleRestraint(Score *ss,
                 const typename Score::Argument& vt,
                 std::string name):
  Restraint(IMP::internal::get_model(vt), name),
  ss_(ss),
  v_(get_index(vt))
{
}

template <class Score>
double TupleRestraint<Score>
::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(ss_);
  return call_evaluate_index(Restraint::get_model(), ss_.get(), v_, accum);
}


template <class Score>
ParticlesTemp TupleRestraint<Score>::get_input_particles() const
{
  return IMP::internal::get_input_particles(ss_.get(),
                                            get_argument());
}

template <class Score>
ContainersTemp TupleRestraint<Score>::get_input_containers() const
{
  return IMP::internal::get_input_containers(ss_.get(),
                                             get_argument());
}

template <class Score>
void TupleRestraint<Score>::do_show(std::ostream& out) const
{
  out << "score " << Showable(ss_) << std::endl;
  out << "data " << Showable(get_argument()) << std::endl;
}
template <class Score>
inline Restraint* create_restraint(Score *s,
                            const typename Score::Argument &t,
                            std::string name= std::string()) {
  if (name==std::string()) {
    std::ostringstream oss;
    oss << s->get_name() << " on " << t;
      name= oss.str();
  }
  return new internal::TupleRestraint<Score>(s, t, name);
}








IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_TUPLE_RESTRAINT_H */
