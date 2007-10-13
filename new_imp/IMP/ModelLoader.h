/*
 *  ModelLoader.h
 *  IMP
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

class IMPDLLEXPORT ModelLoader
{
public:
  ModelLoader();
  ~ModelLoader();

  // Model
  bool load_model(Model& model, const std::string& fname);
  void load_particle(Particle& particle, std::istream& in);
  void load_restraint_set(Model& model, RestraintSet& restraint_set, std::istream& in);
  void load_distance_restraints(Model& model, RestraintSet& restraint_set, std::istream& in);
  ScoreFunc* get_score_func(const std::string score_func_name);

};

} // namespace IMP

#endif  /* __IMP_MODEL_LOADER_H */
