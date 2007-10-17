/**
 *  \file ModelLoader.h   \brief Classes to populate a model from a file.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_LOADER_H
#define __IMP_MODEL_LOADER_H

#include <fstream>

#include "IMP_config.h"
#include "Model.h"
#include "Particle.h"
#include "log.h"
#include "restraints/RestraintSet.h"

namespace IMP
{

//! Class to populate a model from a file.
class IMPDLLEXPORT ModelLoader
{
public:
  ModelLoader();
  ~ModelLoader();

  //! Load model from the given file into the given model structure.
  /** \param[in] model Model structure that model should be loaded into.
      \param[in] fname Name of file containing the model definition.

      \return true if the load succeeded.
   */
  bool load_model(Model& model, const std::string& fname);

  //! Load particle from the given input stream.
  /** Typical particle in file will look as follows:
      \verbatim
      PARTICLE 1
        X VAR 1.0 T
        Y VAR 0.0 T
        Z VAR 0.0 T
        radius ATTR FLOAT 1.0
        protein ATTR INT 1
      END_PARTICLE
      \endverbatim
      \param[in] particle Particle structure that particle should be
                          loaded into.
      \param[in] in Input stream that the particle should be read from.
   */
  void load_particle(Particle& particle, std::istream& in);

  //! Load restraint set from the given input stream.
  /** RestraintSets can contain unnamed restraints or other restraint sets.
      \param[in] model Model structure that model should be loaded into.
      \param[in] restraint_set RestraintSet that all embedded restraints
                               and restraint sets should be added to.
      \param[in] in Input stream that the restraints should be read from.
   */
  void load_restraint_set(Model& model, RestraintSet& restraint_set,
                          std::istream& in);

  //! Load distance restraints and add them to the given restraint set.
  /** \param[in] model Model structure that model should be loaded into.
      \param[in] restraint_set RestraintSet that distance restraints should
                               be added to.
      \param[in] in Input stream that the restraints should be read from.
   */
  void load_distance_restraints(Model& model, RestraintSet& restraint_set,
                                std::istream& in);

  //! Get pointer to the specified scoring function.
  /** \param[in] score_func_name Name of the scoring function desired.
      \return Pointer to specified scoring function.
   */
  ScoreFunc* get_score_func(const std::string score_func_name);

};

} // namespace IMP

#endif  /* __IMP_MODEL_LOADER_H */
