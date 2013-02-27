/**
 *  \file IMP/kernel/input_output_macros.h
 *  \brief macros for inputs and outputs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INPUT_OUTPUT_MACROS_H
#define IMPKERNEL_INPUT_OUTPUT_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/deprecation_macros.h>
#include "particle_index.h"
#include "input_output.h"


#define IMP_BACKWARDS_MACRO_INPUTS                                      \
  public:                                                               \
  IMP_IMPLEMENT(ParticlesTemp get_input_particles(Particle*) const);    \
  IMP_IMPLEMENT(ContainersTemp get_input_containers(Particle*) const);  \
  IMP_IMPLEMENT_INLINE(ModelObjectsTemp                                 \
                       do_get_inputs(Model *m,                          \
                                     const ParticleIndexes &pis) const, { \
                         ModelObjectsTemp ret;                          \
                         for (unsigned int i=0; i< pis.size(); ++i) {   \
                           ret+=get_input_particles(m->get_particle(pis[i])); \
                           ret+=get_input_containers(m->get_particle(pis[i])); \
                         }                                              \
                         return ret;                                    \
                       })

#define IMP_BACKWARDS_MACRO_OUTPUTS                   \
  public:                                                               \
  IMP_IMPLEMENT(ParticlesTemp get_output_particles(Particle*) const);   \
  IMP_IMPLEMENT(ContainersTemp get_output_containers(Particle*) const); \
  IMP_IMPLEMENT_INLINE(ModelObjectsTemp                                 \
                       do_get_outputs(Model *m,                         \
                                      const ParticleIndexes &pis)       \
                       const, {                            \
                         ModelObjectsTemp ret;                          \
                         for (unsigned int i=0; i< pis.size(); ++i) {   \
                           ret+=get_output_particles(m->get_particle(pis[i])); \
                           ret+=get_output_containers(m->get_particle(pis[i]));\
                         }                                              \
                         return ret;                                    \
                       })

#define IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS                         \
  public:                                                               \
  IMP_IMPLEMENT(ParticlesTemp get_input_particles() const);             \
  IMP_IMPLEMENT(ContainersTemp get_input_containers() const);           \
  IMP_IMPLEMENT_INLINE(ModelObjectsTemp                                 \
                       do_get_inputs() const, {            \
                         ModelObjectsTemp ret;                          \
                         ret += get_input_containers();                 \
                         ret += get_input_particles();                  \
                         return ret;                                    \
                       })

#define IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS                        \
  public:                                                               \
  IMP_IMPLEMENT(ParticlesTemp get_output_particles() const);            \
  IMP_IMPLEMENT(ContainersTemp get_output_containers() const);          \
  IMP_IMPLEMENT_INLINE(ModelObjectsTemp                                 \
                       do_get_outputs() const , {           \
                         ModelObjectsTemp ret;                          \
                         ret += get_output_containers();                \
                         ret += get_output_particles();                 \
                         return ret;                                    \
                       })

#if IMP_HAS_DEPRECATED
#define IMP_INPUTS_DECL_BACKWARDS(Name)                                \
  public:                                                               \
  /** Get the set of particles read when applied to the arguments.
      \deprecated use get_inputs() instead.*/   \
IMP_DEPRECATED_WARN ParticlesTemp               \
get_input_particles(Particle* p) const;         \
  /** Get the set of input containers when this modifier is applied to
      the arguments.
      \deprecated use get_outputs() instead.*/  \
IMP_DEPRECATED_WARN ContainersTemp              \
get_input_containers(Particle *p) const

#define IMP_INPUTS_DEF_BACKWARDS(Name)                                  \
    ParticlesTemp                                                       \
    Name::get_input_particles(Particle* p) const {                      \
      return                                                            \
        IMP::kernel::get_input_particles(get_inputs(p->get_model(),     \
                                        ParticleIndexes(1,              \
                                                        p->get_index()))); \
    }                                                                   \
    ContainersTemp                                                      \
    Name::get_input_containers(Particle *p) const {                     \
      return                                                            \
        IMP::kernel::get_input_containers(get_inputs(p->get_model(),    \
                                               ParticleIndexes(1,       \
                                                     p->get_index()))); \
    }


#define IMP_OUTPUTS_DECL_BACKWARDS(Name)                                \
  public:                                                               \
  /** Get the set of particles read when applied to the arguments.
      \deprecated use get_outputs() instead.*/                          \
IMP_DEPRECATED_WARN ParticlesTemp                                       \
get_output_particles(Particle* p) const;                                \
/** Get the set of output containers when this modifier is applied to
    the arguments.
    \deprecated use get_outputs() instead.*/    \
IMP_DEPRECATED_WARN ContainersTemp              \
get_output_containers(Particle *p) const

#define IMP_OUTPUTS_DEF_BACKWARDS(Name)                                 \
    ParticlesTemp                                                       \
    Name::get_output_particles(Particle* p) const {                     \
      return                                                            \
        IMP::kernel::get_output_particles(get_outputs(p->get_model(),   \
                                                ParticleIndexes(1,      \
                                                     p->get_index()))); \
    }                                                                   \
  ContainersTemp                                                        \
  Name::get_output_containers(Particle *p) const {                      \
    return                                                              \
      IMP::kernel::get_output_containers(get_outputs(p->get_model(),    \
                                               ParticleIndexes(1,       \
                                                       p->get_index()))); \
  }

#else
#define IMP_INPUTS_DECL_BACKWARDS(Name)
#define IMP_INPUTS_DEF_BACKWARDS(Name)
#define IMP_OUTPUTS_DECL_BACKWARDS(Name)
#define IMP_OUTPUTS_DEF_BACKWARDS(Name)

#endif

/** Declare standard input methods for things that take particles as arguments
    such as IMP::SingletonModifier and IMP::PairScore.*/
#define IMP_INPUTS_DECL(Name)                                           \
  IMP_INPUTS_DECL_BACKWARDS(Name);                                      \
  public:                                                               \
  /** Get all the ModelObject objects that are read when the referenced
        particles are passed in.*/                                      \
ModelObjectsTemp get_inputs(Model *m,                                   \
                            const ParticleIndexes &pis) const;          \
/** Override if this reads other objects during evaluate.*/             \
IMP_PROTECTED_METHOD(virtual ModelObjectsTemp, do_get_inputs,           \
                     (Model *m,                                         \
                      const ParticleIndexes &pis), const, =0)

/** Define standard input methods for things that take particles as arguments
    such as IMP::SingletonModifier and IMP::PairScore.*/
#define IMP_INPUTS_DEF(Name)                                            \
  IMP_INPUTS_DEF_BACKWARDS(Name)                                        \
  ModelObjectsTemp Name::get_inputs(Model *m,                           \
                                    const ParticleIndexes &pis) const { \
    return do_get_inputs(m, pis);                                       \
  }



/** Declare standard output methods for things that take particles as arguments
    such as IMP::SingletonModifier and IMP::PairScore.*/
#define IMP_OUTPUTS_DECL(Name)                                          \
  IMP_OUTPUTS_DECL_BACKWARDS(Name);                                     \
public:                                                                 \
/** Get all the ModelObject objects that are read when the referenced
      particles are passed in.*/                                        \
ModelObjectsTemp get_outputs(Model *m,                                  \
                             const ParticleIndexes &pis) const;         \
/** Override if this reads other objects during evaluate.*/             \
IMP_PROTECTED_METHOD(virtual ModelObjectsTemp, do_get_outputs,          \
                     (Model *m,                                         \
                      const ParticleIndexes &pis), const, =0)

/** Define standard output methods for things that take particles as arguments
    such as IMP::SingletonModifier and IMP::PairScore.*/
#define IMP_OUTPUTS_DEF(Name)                                           \
  IMP_OUTPUTS_DEF_BACKWARDS(Name)                                       \
  ModelObjectsTemp Name::get_outputs(Model *m,                          \
                                     const ParticleIndexes &pis) const { \
    return do_get_outputs(m, pis);                                      \
  }


#endif  /* IMPKERNEL_INPUT_OUTPUT_MACROS_H */
