/*
 *  Model_Loader.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__model_loader_h)
#define __model_loader_h 1

#include "IMP_config.h"

namespace imp
{


class Score_Func;
class Model;
class Particle;
class Restraint;
class Restraint_Set;
class Optimizer;
class Rigid_Body;

class IMPDLLEXPORT Model_Loader
{
public:
  Model_Loader();
  ~Model_Loader();

  // Model
  bool load_model(Model& model, const std::string& fname);
  void load_particle(Particle& particle, std::istream& in);
  void load_restraint_set(Model& model, Restraint_Set& restraint_set, std::istream& in);
  void load_distance_restraints(Model& model, Restraint_Set& restraint_set, std::istream& in);
  Score_Func* get_score_func(const std::string score_func_name);

};

} // namespace imp

#endif
