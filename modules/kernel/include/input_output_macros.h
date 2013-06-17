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

#define IMP_BACKWARDS_MACRO_INPUTS                                             \
 public:                                                                       \
  /** \deprecated_at{2.1} Use get_inputs() instead. */                         \
  ParticlesTemp get_input_particles(Particle *) const;                         \
  /** \deprecated_at{2.1} Use get_inputs() instead. */                         \
  ContainersTemp get_input_containers(Particle *) const;                       \
  ModelObjectsTemp do_get_inputs(Model *m, const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                                      \
    for (unsigned int i = 0; i < pis.size(); ++i) {                            \
      ret += get_input_particles(m->get_particle(pis[i]));                     \
      ret += get_input_containers(m->get_particle(pis[i]));                    \
    }                                                                          \
    return ret;                                                                \
  }

#define IMP_BACKWARDS_MACRO_OUTPUTS                                   \
 public:                                                              \
  /** \deprecated_at{2.1} Use get_inputs() instead. */                \
  ParticlesTemp get_output_particles(Particle *) const;               \
  /** \deprecated_at{2.1} Use get_inputs() instead. */                \
  ContainersTemp get_output_containers(Particle *) const;             \
  ModelObjectsTemp do_get_outputs(Model *m,                           \
                                  const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                             \
    for (unsigned int i = 0; i < pis.size(); ++i) {                   \
      ret += get_output_particles(m->get_particle(pis[i]));           \
      ret += get_output_containers(m->get_particle(pis[i]));          \
    }                                                                 \
    return ret;                                                       \
  }

#define IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS \
 public:                                        \
  ParticlesTemp get_input_particles() const;    \
  ContainersTemp get_input_containers() const;  \
  ModelObjectsTemp do_get_inputs() const {      \
    ModelObjectsTemp ret;                       \
    ret += get_input_containers();              \
    ret += get_input_particles();               \
    return ret;                                 \
  }

#define IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS        \
 public:                                                \
  /** \deprecated_at{2.1} Use get_outputs() instead. */ \
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)               \
      ParticlesTemp get_output_particles() const;       \
  /** \deprecated_at{2.1} Use get_outputs() instead. */ \
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)               \
      ContainersTemp get_output_containers() const;     \
  IMP_COMPILER_DISABLE_WARNINGS                        \
  ModelObjectsTemp do_get_outputs() const {             \
    ModelObjectsTemp ret;                               \
    ret += get_output_containers();                     \
    ret += get_output_particles();                      \
    return ret;                                         \
  }                                                     \
  IMP_COMPILER_ENABLE_WARNINGS

#define IMP_INPUTS_DECL_BACKWARDS(Name)                     \
 public:                                                    \
  /** Get the set of particles read when applied to         \
     the                                                    \
     arguments.                                             \
      \deprecated_at{2.1}  use                              \
     get_inputs()                                           \
     instead.*/ IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)     \
      ParticlesTemp get_input_particles(Particle *p) const; \
  /** Get the set of input containers when this modifier is \
     applied                                                \
     to                                                     \
      the arguments.                                        \
      \deprecated_at{2.1} use                               \
     get_outputs()                                          \
     instead.*/ IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.0)     \
      ContainersTemp get_input_containers(Particle *p) const

#define IMP_INPUTS_DEF_BACKWARDS(Name)                                   \
  ParticlesTemp Name::get_input_particles(Particle *p) const {           \
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Use get_inputs() instead."); \
    return IMP::kernel::get_input_particles(                             \
        get_inputs(p->get_model(), ParticleIndexes(1, p->get_index()))); \
  }                                                                      \
  ContainersTemp Name::get_input_containers(Particle *p) const {         \
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Use get_inputs() instead."); \
    return IMP::kernel::get_input_containers(                            \
        get_inputs(p->get_model(), ParticleIndexes(1, p->get_index()))); \
  }

#define IMP_OUTPUTS_DECL_BACKWARDS(Name)                                     \
 public:                                                                     \
  /** Get the set of particles read when applied to                          \
     the                                                                     \
     arguments.                                                              \
      \deprecated_at{2.1} use                                                \
     get_outputs()                                                           \
     instead.*/ IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.0)                      \
      ParticlesTemp get_output_particles(                                    \
          Particle *p) const; /** Get the set of output containers when this \
                                 modifier is applied to                      \
                                  the arguments.                             \
                                  \deprecated_at{2.1} use get_outputs()      \
                                 instead.*/                                  \
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.0)                                    \
      ContainersTemp get_output_containers(Particle *p) const

#define IMP_OUTPUTS_DEF_BACKWARDS(Name)                                   \
  ParticlesTemp Name::get_output_particles(Particle *p) const {           \
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Use get_outputs() instead."); \
    return IMP::kernel::get_output_particles(                             \
        get_outputs(p->get_model(), ParticleIndexes(1, p->get_index()))); \
  }                                                                       \
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)                                 \
      ContainersTemp Name::get_output_containers(Particle *p) const {     \
    IMPKERNEL_DEPRECATED_FUNCTION_DEF(2.1, "Use get_outputs() instead."); \
    return IMP::kernel::get_output_containers(                            \
        get_outputs(p->get_model(), ParticleIndexes(1, p->get_index()))); \
  }

/** \deprecated_at{2.1} Just declare the methods yourself.*/
#define IMP_INPUTS_DECL(Name)                                           \
  IMPKERNEL_DEPRECATED_MACRO(2.1,                                       \
                             "Inherit from IMP::kernel::ParticleInputs."); \
  IMP_INPUTS_DECL_BACKWARDS(Name);                                      \
                                                                        \
public:                                                                 \
/** Get all the ModelObject objects that are read when the              \
    referenced                                                          \
    particles are passed in.*/ ModelObjectsTemp                         \
  get_inputs(Model *m, const ParticleIndexes &pis) const;               \
                                                                        \
protected: /** Override if this reads other objects during evaluate.*/  \
virtual ModelObjectsTemp do_get_inputs(Model *m,                        \
                                       const ParticleIndexes &pis) const = 0

/** Define standard input methods for things that take particles as arguments
    such as IMP::SingletonModifier and IMP::PairScore.*/
#define IMP_INPUTS_DEF(Name)                                            \
  IMP_INPUTS_DEF_BACKWARDS(Name) IMPKERNEL_DEPRECATED_MACRO(            \
      2.1, "Inherit from IMP::kernel::ParticleInputs.");                \
  ModelObjectsTemp Name::get_inputs(Model *m,                           \
                                    const ParticleIndexes &pis) const { \
    return do_get_inputs(m, pis);                                       \
  }

/** \deprecated_at{2.1} Declare the methods yourself. */
#define IMP_OUTPUTS_DECL(Name)                                          \
  IMPKERNEL_DEPRECATED_MACRO(2.1,                                       \
                             "Inherit from IMP::kernel::ParticleOutputs."); \
  IMP_OUTPUTS_DECL_BACKWARDS(Name);                                     \
                                                                        \
public                                                                  \
: /** Get all the ModelObject objects that are read when the referenced \
      particles are passed in.*/                                        \
ModelObjectsTemp get_outputs(Model *m, const ParticleIndexes &pis) const; \
                                                                             \
 protected: /** Override if this reads other objects during evaluate.*/      \
  virtual ModelObjectsTemp do_get_outputs(                                   \
      Model *m, const ParticleIndexes &pis) const = 0

/** \deprecated_at{2.1} Declare the methods yourself*/
#define IMP_OUTPUTS_DEF(Name)                                               \
  IMPKERNEL_DEPRECATED_MACRO(2.1,                                           \
                             "Inherit from IMP::kernel::ParticleOutputs."); \
  IMP_OUTPUTS_DEF_BACKWARDS(Name) ModelObjectsTemp Name::get_outputs(       \
      Model *m, const ParticleIndexes &pis) const {                         \
    return do_get_outputs(m, pis);                                          \
  }

#endif /* IMPKERNEL_INPUT_OUTPUT_MACROS_H */
