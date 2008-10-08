/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_MODEL_H
#define IMP_MODEL_H

#include "IMP_config.h"
#include "Object.h"
#include "internal/kernel_version_info.h"
#include "base_types.h"
#include "VersionInfo.h"

IMP_BEGIN_NAMESPACE

class Particle;
class Restraint;
class ScoreState;
typedef std::vector<Restraint*> Restraints;
typedef std::vector<ScoreState*> ScoreStates;

//! Class for storing model, its restraints, and particles.
/** The Model maintains a standard IMP container for each of Particle,
    ScoreState and Restraint object types.
    \ingroup kernel
 */
class IMPDLLEXPORT Model: public Object
{
 private:
  friend class Restraint;
  unsigned int iteration_;
public:
  Model();
  ~Model();

  IMP_CONTAINER(Particle, particle, ParticleIndex);
  IMP_CONTAINER(ScoreState, score_state, ScoreStateIndex);
  IMP_CONTAINER(Restraint, restraint, RestraintIndex);
 public:


  //! Evaluate all of the restraints in the model and return the score.
  /** \param[in] calc_derivs If true, also evaluate the first derivatives.
      \return The score.

      All of the stored ScoreState objects are updated before the
      restraints are evaluated.
   */
  Float evaluate(bool calc_derivs);

  //! Show the model contents.
  /** \param[in] out Stream to write model description to.
   */
  void show (std::ostream& out = std::cout) const;

  //! \return version and authorship information.
  VersionInfo get_version_info() const {
    return internal::kernel_version_info;
  }
};

IMP_OUTPUT_OPERATOR(Model);

IMP_END_NAMESPACE

#endif  /* IMP_MODEL_H */
