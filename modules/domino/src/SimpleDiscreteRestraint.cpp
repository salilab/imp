/**
 *  \file SimpleDiscreteRestraint.h
 *  \brief Simple restraint for testing
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/domino/SimpleDiscreteRestraint.h>
#include <IMP/domino/CombState.h>
#include <IMP/container_macros.h>


IMPDOMINO_BEGIN_NAMESPACE

void SimpleDiscreteRestraint::load_restraints(std::string restraint_filename)
{
  std::ifstream myfile(restraint_filename.c_str());
  char line[1024];
  std::vector<std::string> v;
  std::pair<int, int> last_key;
  while (myfile.getline(line, 1024)) {
    v.clear();
    char *pch = strtok(line, "|");
    while (pch != NULL) {
      v.push_back(std::string(pch));
      pch = strtok(NULL, "|");
    }
    if (v.size() == 2) {
      last_key = std::pair<int, int>(atoi(v[0].c_str()), atoi(v[1].c_str()));
      states2values[last_key] = std::map<std::pair<int, int>, float>();
    } else if (v.size() == 3)  {
      states2values[last_key][std::pair<int,int>(atoi(v[0].c_str()),
                                                 atoi(v[1].c_str()))]
          = atof(v[2].c_str());
    } else {
      std::ostringstream msg;
      msg << "SimpleDiscreteRestraint::load_restraints the line : "
          << line << " is of the wrong format";
      IMP_failure(msg.str().c_str(), ErrorException);
    }
  }
}

SimpleDiscreteRestraint::SimpleDiscreteRestraint(Model& model_,
    std::string restraint_filename, Particle * p1_, Particle *p2_)
{
  load_restraints(restraint_filename);
  Int p1_ind = internal::particle_index(p1_);
  Int p2_ind = internal::particle_index(p2_);
  if (p1_ind < p2_ind) {
    p1 = p1_;
    p2 = p2_;
    add_particle(p1);
    add_particle(p2);
    key = std::pair<int, int>(p1_ind, p2_ind);
  } else {
    p1 = p2_;
    p2 = p1_;
    add_particle(p2);
    add_particle(p1);
    key = std::pair<int, int>(p2_ind, p1_ind);
  }
  model = &model_;
}

IMP_LIST_IMPL(SimpleDiscreteRestraint, Particle, particle,Particle*,  {
              IMP_assert(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                         "All particles in Restraint must belong to the "
                         "same Model.");
  },,);




Float SimpleDiscreteRestraint::evaluate(DerivativeAccumulator *accum)
{
  //build state key
  int a1 = int(p1->get_value(IMP::FloatKey(KEY_OPT)));
  int a2 = int(p2->get_value(IMP::FloatKey(KEY_OPT)));
  return states2values[key][std::pair<int,int>(a1,a2)];
}

void SimpleDiscreteRestraint::show(std::ostream& out) const
{
  std::cout << "=========" << std::endl;
  if (get_is_active()) {
    out << "simple discrete restraint (active):" << std::endl;
  } else {
    out << "simple discrete restraint (inactive):" << std::endl;
  }
  get_version_info().show(out);
  out << "  particles: " << get_particle(0)->get_name();
  out << " and " << get_particle(1)->get_name();
  out << std::endl;
}

IMPDOMINO_END_NAMESPACE
