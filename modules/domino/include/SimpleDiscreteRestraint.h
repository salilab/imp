/**
 *  \file SimpleDiscreteRestraint.h
 *  \brief Simple restraint for testing
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPDOMINO_SIMPLE_DISCRETE_RESTRAINT_H
#define __IMPDOMINO_SIMPLE_DISCRETE_RESTRAINT_H

#include "domino_exports.h"
#include "domino_version_info.h"
#include "IMP/Model.h"
#include "IMP/Restraint.h"

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
  //! Constructor.
  /** \exception ErrorException the restraint file is of an invalid format.
   */
  SimpleDiscreteRestraint(Model& model_, std::string restraint_filename,
                          Particle *p1, Particle *p2);

  virtual ~SimpleDiscreteRestraint() {};
  IMP_RESTRAINT(internal::domino_version_info)
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

#endif  /* __IMPDOMINO_SIMPLE_DISCRETE_RESTRAINT_H */
