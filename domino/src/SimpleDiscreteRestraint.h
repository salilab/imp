/**
 *  \file SimpleDiscreteRestraint.h
 *  \brief Simple restraint for testing
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_SIMPLE_DISCRETE_RESTRAINT_H
#define __IMP_SIMPLE_DISCRETE_RESTRAINT_H

#include "domino_exports.h"
#include "IMP/Model.h"
#include "IMP/Restraint.h"
#include "IMP/VersionInfo.h"

#include <string>
#include <climits>

#define KEY_OPT "OPT"

namespace IMP
{

namespace domino
{

class IMPDOMINOEXPORT SimpleDiscreteRestraint : public Restraint
{
public:
  SimpleDiscreteRestraint(Model& model_, std::string restraint_filename,
                          Particle *p1, Particle *p2);

  virtual ~SimpleDiscreteRestraint() {};
  IMP_RESTRAINT(VersionInfo("Keren", "0.0.1"))
protected:
  void load_restraints(std::string restraint_filename);
  Model *model;
  std::pair<int, int> key;
  Particle *p1, *p2;
  std::map<std::pair<int, int>, std::map<std::pair<int, int>,
           float> > states2values; // should be static and in a different class
};

} // namespace domino

} // namespace IMP

#endif  /* __IMP_SIMPLE_DISCRETE_RESTRAINT_H */
