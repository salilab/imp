/*
 *  Model_Loader.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Imp.h"

namespace imp
{

/**
  Constructor
 */

Model_Loader::Model_Loader ()
{
}


/**
  Destructor
 */

Model_Loader::~Model_Loader ()
{
}


/**
  Load model from the given file into the given model structure.

  \param[in] model Model structure that model should be loaded into.
  \param[in] fname Name of file containing the model definition.

  \return true if the load succeeded.
 */

bool Model_Loader::load_model(Model& model, const std::string& fname)
{
  try {
    std::ifstream fin;
    std::string next_word;
    Particle* particle;

    fin.open(fname.c_str());

    while (!fin.eof()) {
      next_word = "";
      fin >> next_word;
      if (next_word == "PARTICLE") {
        std::cout << "Add Particle: " << std::endl;
        particle = new Particle();
        model.add_particle(particle);
        load_particle(*particle, fin);
      }

      else if (next_word == "RESTRAINT_SET") {
        std::string name;
        fin >> name;
        std::cout << "Add restraint set " << name << ": " << std::endl;
        Restraint_Set* rs = new Restraint_Set(name);
        model.add_restraint_set(rs);
        load_restraint_set(model, *rs, fin);
      }

      else if (next_word == "") {
      }

      else {
        std::cout << "Unexpected word in file: " << next_word << std::endl;
      }
    }

    fin.close();
  }

  catch (...) {
    ErrorMsg("Failed to load model from " << fname << std::endl);
    return false;
  }

  return true;
}


/**
  Load particle from the given input stream. Typical particle in file will
  look as follows:
  PARTICLE 1
   X VAR 1.0 T
   Y VAR 0.0 T
   Z VAR 0.0 T
   radius ATTR FLOAT 1.0
   protein ATTR INT 1
  END_PARTICLE

  \param[in] particle Particle structure that particle should be loaded into.
  \param[in] in Input stream that the particle should be read from.
 */

void Model_Loader::load_particle(Particle& particle, std::istream& in)
{
  try {
    std::string next_word;
    in >> next_word;
    while ((next_word != "END_PARTICLE") && (!in.eof())) {
      std::string name = next_word;
      in >> next_word;

      // add a particle variable
      Float fvalue;
      Int ivalue;
      String svalue;
      std::string optimize;
      std::string attr_type;
      if (next_word == "VAR") {
        in >> fvalue;
        in >> optimize;

        particle.add_float(name, fvalue, optimize == "T");
      }

      // add a particle attribute
      else if (next_word == "ATTR") {
        in >> attr_type;
        if (attr_type == "FLOAT") {
          in >> fvalue;
          particle.add_float(name, fvalue);
        }

        else if (attr_type == "INT") {
          in >> ivalue;
          particle.add_int(name, ivalue);
        }

        else if (attr_type == "STRING") {
          in >> svalue;
          particle.add_string(name, svalue);
        }

        else {
          ErrorMsg("Unsupported attribute type " << attr_type << ", expected FLOAT, INT or STRING." << std::endl);
        }
      }

      // something went wrong
      else {
        ErrorMsg("Expected VAR or ATTR after " << name << " in particle." << std::endl);
      }

      in >> next_word;
    }
  }

  catch (...) {
    ErrorMsg("Failed to load particle" << std::endl);
  }
}



/**
  Load restraint set from the given input stream. Restraint sets can contain
  unnamed restraints or other restraint sets. Typical restraint set in file will
  look as follows:

  \param[in] restraint_set Restraint set that all embedded restraints and restraint sets should be added to.
  \param[in] in Input stream that the restraints should be read from.
 */

void Model_Loader::load_restraint_set(Model& model, Restraint_Set& restraint_set, std::istream& in)
{
  try {
    std::string next_word;
    in >> next_word;
    while ((next_word != "END_RESTRAINT_SET") && (!in.eof())) {
      if (next_word == "RESTRAINT_SET") {
        std::string name;
        in >> name;
        std::cout << "Add restraint set " << name << ": " << std::endl;
        Restraint_Set* rs = new Restraint_Set(name);
        restraint_set.add_restraint_set(rs);
        load_restraint_set(model, *rs, in);
      }

      else if (next_word == "RESTRAINT") {
        std::string restraint_type;
        in >> restraint_type;
        if (restraint_type == "DISTANCE") {
          load_distance_restraints(model, restraint_set, in);
        }

        else {
          ErrorMsg("Unknown restraint type:" << restraint_type << std::endl);
        }
      }

      // something went wrong
      else {
        ErrorMsg("Expected RESTRAINT_SET or RESTRAINT within the restraint set." << std::endl);
      }

      in >> next_word;
    }
  }

  catch (...) {
    ErrorMsg("Failed to load particle" << std::endl);
  }
}

/**
  Load distance restraints and add them to the given restraint set.

  \param[in] restraint_set Restraint set that distance restraints should be added to.
  \param[in] in Input stream that the restraints should be read from.
 */

void Model_Loader::load_distance_restraints(Model& model, Restraint_Set& restraint_set, std::istream& in)
{
  try {
    std::string next_word;
    std::string format, particle_format;
    in >> next_word;
    Score_Func* score_func = get_score_func(next_word);
    Float mean, sd;
    int num_restraints;

    in >> format;
    if (format == "MEAN") {
      in >> mean;
      in >> next_word;
      if (next_word != "SD") {
        ErrorMsg("Expecting SD in DISTANCE restraint." << std::endl);
      }
      in >> sd;

      in >> particle_format;
      if (particle_format != "PARTICLE_PAIRS") {
        ErrorMsg("Unknown particle specifiers in DISTANCE restraint." << std::endl);
      }
    }

    // get the number of restraints to generate
    in >> num_restraints;

    Particle *p1, *p2;
    int idx1, idx2;
    for (int i = 0; i < num_restraints; i++) {
      // get particle indexes
      if ((particle_format == "PARTICLE_PAIRS") || (format == "RSR_LIST")) {
        in >> idx1;
        in >> idx2;
      } else {
        // use particle indexes generator
      }

      p1 = model.particle(idx1);
      p2 = model.particle(idx2);

      if (format == "RSR_LIST") {
        in >> mean;
        in >> sd;
      }

      restraint_set.add_restraint(
        (Restraint*) new RSR_Distance(model, p1, p2, mean, sd, score_func));
    }
  }

  catch (...) {
    ErrorMsg("Failed to load DISTANCE restraint" << std::endl);
  }
}

/**
  Get pointer to the specified math form.

  \param[in] score_func_name Name of the math form desired.

  \return Pointer to specified math form functor.
 */

Score_Func* Model_Loader::get_score_func(const std::string score_func_name)
{
  if (score_func_name == "HARMONIC") {
    return (Score_Func*) new Harmonic();
  }

  else if (score_func_name == "HARMONIC_LOWER_BOUND") {
    return (Score_Func*) new Harmonic_Lower_Bound();
  }

  else if (score_func_name == "HARMONIC_UPPER_BOUND") {
    return (Score_Func*) new Harmonic_Upper_Bound();
  }

  else {
    ErrorMsg("Unknown math form: " << score_func_name << std::endl);
  }
}

}  // namespace imp

