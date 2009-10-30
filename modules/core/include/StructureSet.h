/**
 *  \file StructureSet.h    \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_STRUCTURE_SET_H
#define IMPCORE_STRUCTURE_SET_H

#include "config.h"

#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/Model.h>
#include <vector>

IMPCORE_BEGIN_NAMESPACE


//! A class to maintain a set of optimization structures.
/** The class maintains a set of structures found by the optimization
    protocol and allows them to be queried and loaded and saved. Addition
    to the set can be filtered by a number of criteria which currently
    includes:
    - total score for the model
    - the score for any particular restraint so you can ensure that all
    restraints are satisfied

    \unstable{StructureSet}
    \untested{StructureSet}
    \see Mover
 */
class IMPCOREEXPORT StructureSet: public Object
{
  IMP::internal::OwnerPointer<Model> m_;
  struct Data {
    double energy;
    std::string name;
    std::string state;
  };
  std::vector<Data> structures_;
  double threshold_;
  std::map<Restraint*, double> restraint_thresholds_;
public:
  StructureSet(Model *m);
  typedef unsigned int StructureIndex;
  void add_structure(std::string name="");
  void load_structure(StructureIndex i);
  Ints get_all_structures() const;
  //! A structure must have a score better than this to be successfully added
  void set_filter_threshold(double v);
  //! A structure must have a better score than this for the restraint
  void set_restraint_filter(Restraint* r, double v);
  IMP_OBJECT(StructureSet, get_module_version_info());
};

IMP_OUTPUT_OPERATOR(StructureSet);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_STRUCTURE_SET_H */
