/**
 *  \file RestraintEvaluatorFromFiles.h   \brief A restraint evaluataor.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H
#define IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H

#include "config.h"
#include "DiscreteSampler.h"
#include <IMP/domino/RestraintEvaluatorI.h>
#include <IMP/base_types.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
IMPDOMINO_BEGIN_NAMESPACE

IMPDOMINOEXPORT void write_combinations(const std::string &filename,
                       const Combinations *combs,Particles &ps);
/**
The format of the combinations file is:
A|B|| (the header line, contains the names of the particles)
0|0|0.3 (data line: For A in state 0, B in state 0, the restraint score is 0.3)
0|1|1.6
1|0|2.9
1|1|3.1
 */
IMPDOMINOEXPORT void read_combinations(const std::string &filename,
                                       Combinations *combs,const Particles &ps);
//! A read restraint values from precalculated files
/**
 */
class IMPDOMINOEXPORT RestraintEvaluatorFromFile : public RestraintEvaluatorI
{
public:
  //! Constructor
  //RestraintEvaluatorFromFile(){}
  //~RestraintEvaluatorFromFile(){}
  //! Add link between a restraint and its precalculated scores
  /**\param [in] r A restraint
     \param [in] filename The restraint scores filename
   */
  void set_restraint_file(Restraint *r,char *filename);
  std::string get_restraint_file(Restraint *r) const;
 //! Score restraint with combinations of states
  /** \param [in] comb_states the combination of states
      \param [in] r the restraint to score
      \param [in] ps the particles that scored by the restraint
      \param[out] comb_values the restraint values for the combiations
   */
  void calc_scores(const Combinations &comb_states,
                   CombinationValues &comb_values,Restraint *r,
                   const Particles &ps);
  void show(std::ostream& out = std::cout) const{
    out<<"RestraintEvaluatorFromFile"<<std::endl;
  }
protected:
  //TODO - this function should be changed once we will have the
  //new kernel functionalities.
 void update_score_states(IMP::Particles &ps);
 std::map<Restraint*,std::string> restraint_data_;
};

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H */
