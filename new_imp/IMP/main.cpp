#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>

#include "log.h"
#include "Particle.h"
#include "Model.h"
#include "Model_Loader.h"
#include "Complexes_Restraints.h"
#include "Optimizer.h"

using namespace imp;

int main (int argc, char * const argv[])
{
  Model model;
  Model_Loader model_loader;

  model_loader.load_model(model, std::string("/Users/bret/Sites/sali/new_imp/IMP/tests/misc/particles.txt"));
  std::vector<int> particle_indexes;

  particle_indexes.push_back(0);
  particle_indexes.push_back(1);

  Restraint_Set* rs2 = new Restraint_Set("connect");
  RSR_Connectivity* cr2 = new RSR_Connectivity(model, particle_indexes, "protein", "radius", (Float) 0.1, (Score_Func*) new Harmonic());
  rs2->add_restraint((Restraint*) cr2);
  model.add_restraint_set(rs2);

  /*
  pidx1.set_index(1);
  p1 = model.particle(pidx1);
  p1->show(std::cout);

  pidx2.set_index(2);
  p2 = model.particle(pidx2);
  p2->show(std::cout);

  RSR_Distance* dr = new RSR_Distance(model, pidx1, pidx2, (Float) 2.0, (Float) 0.1, (Score_Func*) new Harmonic());
  model.set_var(p1->var_index(std::string("X")), (Float) 8.0);
  std::cout << "Score: " << dr->evaluate(model, true) << std::endl;
  p1->show(std::cout);
   p2->show(std::cout);
   
   Restraint_Set* rs = new Restraint_Set();
   rs->add_restraint((Restraint*) dr);
   
   model.add_restraint(std::string("dist1"), (Restraint*) rs);
  */

  Float score = model.evaluate(true);
  std::cout << "Model score: " << score << std::endl;
  model.show();

  Particle* particle = model.particle(1);
  particle->set_is_active(false);
  score = model.evaluate(true);
  std::cout << "Model score: " << score << std::endl;
  model.show();

  model.set_up_trajectory();
  particle->set_is_active(true);
  score = model.evaluate(true);
  std::cout << "Model score: " << score << std::endl;

  Model_Data* model_data = model.get_model_data();
  particle = model.particle(1);
  std::cout << "Particle: " <<  model_data->get_float(particle->float_index(std::string("X"))) << ", "
  <<  model_data->get_float(particle->float_index(std::string("Y"))) << ", "
  <<  model_data->get_float(particle->float_index(std::string("Z"))) << std::endl;

  Restraint_Set* rs = new Restraint_Set("test");
  RSR_Coordinate* cr = new RSR_Coordinate(model, model.particle(0), "XYZ_SPHERE", (Float) 1.0, (Float) 0.1, (Score_Func*) new Harmonic_Upper_Bound());
  rs->add_restraint((Restraint*) cr);
  cr = new RSR_Coordinate(model, model.particle(1), "XYZ_SPHERE", (Float) 1.0, (Float) 0.1, (Score_Func*) new Harmonic_Upper_Bound());
  rs->add_restraint((Restraint*) cr);
  cr = new RSR_Coordinate(model, model.particle(2), "XYZ_SPHERE", (Float) 1.0, (Float) 0.1, (Score_Func*) new Harmonic_Upper_Bound());
  rs->add_restraint((Restraint*) cr);
  model.add_restraint_set(rs);

  model.show();

  Optimizer* steepest_descent = (Optimizer*) new Steepest_Descent();
  steepest_descent->optimize(model, 40, 0.001);

  score = model.evaluate(false);
  model.show();

  return 0;
}
