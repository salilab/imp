/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_MACROS_H
#define IMPKERNEL_MACROS_H
#include "kernel_config.h"
#include <IMP/base/base_macros.h>
#include "decorator_macros.h"
#include "key_macros.h"
#include "restraint_macros.h"
#include "scoring_function_macros.h"
#include "optimizer_state_macros.h"
#include "optimizer_macros.h"
#include "unary_function_macros.h"
#include "sampler_macros.h"
#include "score_state_macros.h"



/** \name Macros to aid with implementing object classes

    These macros are here to aid with implementing classes that
    inherit from the various abstract base classes in the kernel. Each
    \imp object class should use one of the IMP_OBJECT(), IMP_RESTRAINT()
    etc. macros in the body of its declaration as well as an
    IMP_OBJECTS() invocation on the namespace level (and an
    IMP_SWIG_OBJECTS() call in the corresponding swig file). See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.

    Each  IMP_OBJECT()/IMP_RESTRAINT()-style macro
    declares/defines the set of needed functions. The declared
    functions should be defined in the associated \c .cpp file. By
    using the macros, you ensure that your class gets the names of the
    functions correct and it makes it easier to update your class if
    the functions should change.

    All of the IMP_OBJECT()/IMP_RESTRAINT()-style macros define the
    following methods:
    - IMP::base::Object::get_version_info()
    - an empty virtual destructor

    In addition, they all declare:
    - IMP::base::Object::do_show()

    For all macros, the Name parameter is the name of the class being
    implemented and the version_info parameter is the IMP::VersionInfo
    to use (probably get_version_info()).

    @{
*/







//! Define the basics needed for a particle refiner
/** In addition to the methods done by all the macros, it declares
    - IMP::Refiner::get_can_refine()
    - IMP::Refiner::get_number_of_refined()
    - IMP::Refiner::get_refined()
    - IMP::Refiner::get_input_particles()
    \see IMP_SIMPLE_REFINER
*/
#define IMP_REFINER(Name)                                               \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *) const;            \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  virtual ParticlesTemp get_input_particles(Particle *p) const;         \
  virtual ContainersTemp get_input_containers(Particle *p) const;       \
  IMP_OBJECT(Name)


//! Define the basics needed for a particle refiner
/** In contrast to IMP_REFINER, if this macro is used, the
    Refiner::get_refined(Particle*) method is implemented using the
    other Refiner::get_refined() method and so does not have to be
    provided.

    \see IMP_REFINER
*/
#define IMP_SIMPLE_REFINER(Name)                                        \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *a) const {          \
    ParticlesTemp ret(get_number_of_refined(a));                        \
    for (unsigned int i=0; i< ret.size(); ++i) {                        \
      ret[i]= get_refined(a,i);                                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  virtual ParticlesTemp get_input_particles(Particle *p) const;         \
  virtual ContainersTemp get_input_containers(Particle *p) const;       \
  IMP_OBJECT(Name)









//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    for a modifier which updates the passed particle based on the results
    of refinement.
*/
#define IMP_SINGLETON_MODIFIER_FROM_REFINED(Name, refiner)              \
  ParticlesTemp Name::get_input_particles(Particle *p) const {          \
    ParticlesTemp ret= refiner->get_refined(p);                         \
    ParticlesTemp ret1= refiner->get_input_particles(p);                \
    ret.insert(ret.end(), ret1.begin(), ret1.end());                    \
    return ret;                                                         \
  }                                                                     \
  ParticlesTemp Name::get_output_particles(Particle *p) const {         \
    ParticlesTemp ret(1,p);                                             \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp Name::get_input_containers(Particle *p) const {        \
    return refiner->get_input_containers(p);                            \
  }                                                                     \
  ContainersTemp Name::get_output_containers(Particle *) const {       \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out <<"refiner " << *refiner << std::endl;                          \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    - IMP::base::Object::do_show()
    for a modifier which updates the refined particles based on the one
    they are refined from.

    This macro should appear in a .cpp file.
*/
#define IMP_SINGLETON_MODIFIER_TO_REFINED(Name, refiner)                \
  ParticlesTemp Name::get_input_particles(Particle *p) const {          \
    ParticlesTemp ret= refiner->get_input_particles(p);                 \
    ParticlesTemp ret2= refiner->get_refined(p);                        \
    ret.push_back(p);                                                   \
    ret.insert(ret.end(), ret2.begin(), ret2.end());                    \
    return ret;                                                         \
  }                                                                     \
  ParticlesTemp Name::get_output_particles(Particle *p) const {         \
    ParticlesTemp ret= refiner->get_refined(p);                         \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp Name::get_input_containers(Particle *p) const {        \
    return refiner->get_input_containers(p);                            \
  }                                                                     \
  ContainersTemp Name::get_output_containers(Particle *) const {       \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out << "refiner " << *refiner << std::endl;                         \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE



//! Declare a IMP::FailureHandler
/** In addition to the standard methods it declares:
    - IMP::FailureHandler::handle_failure()
*/
#define IMP_FAILURE_HANDLER(Name)               \
  IMP_IMPLEMENT(void handle_failure());         \
  IMP_OBJECT(Name)



#ifndef SWIG
/** Report dependencies of the container Name. Add the line
    deps_(new DependenciesScoreState(this), model) to the constructor
    initializer list. The input_deps argument should add the input
    containers to a variable ret.
*/
#define IMP_CONTAINER_DEPENDENCIES(Name, input_deps)                    \
  class DependenciesScoreState: public ScoreState {                     \
    Name* back_;                                                        \
  public:                                                               \
  DependenciesScoreState(Name *n):                                      \
    ScoreState(n->get_name()+" dependencies"),                          \
    back_(n){}                                                          \
  ContainersTemp get_input_containers() const{                          \
    ContainersTemp ret;                                                 \
    input_deps                                                          \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp get_output_containers() const{                         \
    return ContainersTemp(1, back_);                                    \
  }                                                                     \
  ParticlesTemp get_input_particles() const {                           \
    return ParticlesTemp();                                             \
  }                                                                     \
  ParticlesTemp get_output_particles() const{                           \
    return ParticlesTemp();                                             \
  }                                                                     \
  void do_before_evaluate() {}                                          \
  void do_after_evaluate(DerivativeAccumulator *) {}                    \
  IMP_OBJECT_INLINE(DependenciesScoreState, {if (0) out<<1;}, {});      \
  };                                                                    \
  friend class DependenciesScoreState;                                  \
  ScopedScoreState deps_

#else
#define IMP_CONTAINER_DEPENDENCIES(Name, input_deps)
#endif


#include "singleton_macros.h"
#include "pair_macros.h"
#include "triplet_macros.h"
#include "quad_macros.h"


#endif  /* IMPKERNEL_MACROS_H */
