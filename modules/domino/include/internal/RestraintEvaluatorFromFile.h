/**
 *  \file RestraintEvaluatorFromFile.h   \brief A restraint evaluator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H
#define IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H

#include "config.h"
#include "DiscreteSampler.h"
#include "CombState.h"
#include <IMP/base_types.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

void write_combinations(const std::string &filename,
                        const Combinations *combs,Particle &ps) {
  //write the combinations
   std::ofstream out_file(filename.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << filename << " for writing",
              IOException);
  }
  //add all combinations
  out_file<<ps[0]->get_name()<<"|"<<ps[1]->get_name()<<"||"<<std::endl;

  for (Combinations::const_iterator it = combs->begin();
       it != combs->end(); it++) {
    CombState *c=(*it)->second();
    out_file<<c->get_state_num(ps[0])<<"|"<<
         get_state_num(ps[1])<<"|"<<c->get_total_score()<<"|"<<std::endl;
  }
  out_file.close();
}

void read_combinations(const std::string &filename, Combinations *combs) {
  std::ifstream scores_file(filename.c_str());
  if (!scores_file) {
    IMP_THROW("No such scores file " << filename,
              IOException);
  }
  //read the file
  int status;
  std::string line;
  //read the header line
  getline(in, line);
  //get the particles names
  Particle *p1,*p2;
  Particles ps; ps.push_back(p1);ps.push_back(p2);
  //now parse the data
  typedef vector< string > split_vector_type;
  split_vector_type split_vec;
  while (!in.eof()) {
    if (!getline(in, line)) break;
    split(split_vec, line, is_any_of("|"));
    calc_state = new CombState();
    calc_state->add_data_item(p1,int(split_vec[0]));
    calc_state->add_data_item(p2,int(split_vec[1]));
    (*combs)[calc_state->partial_key(ps)]=calc_state;
  }
}


//! A read restraint values from precalculated files
/**
 */
class IMPDOMINOEXPORT RestraintEvaluatorFromFile
{
public:
  //! Constructor
  /** \param [in] ds A discrete sampling space
      \param [in] directory_path A path for precalculated restraint files
   */
  RestraintEvaluator(DiscreteSampler *ds,
                     const std::string &directory_path="./") {
    ds_=ds;
    directory_path_ = directory_path;
  }
  //! Add link between a restraint and its precalculated scores
  /**\param [in] r A restraint
     \param [in] filename The restraint scores filename
   */
  void add_restraint_file(Restraint *r,const std::string &filename);
  std::string get_restraint_file(Restraint *r);
 //! Score restraint with combinations of states
  /** \param [in] cs the combination of states
      \param [in] r the restraint to score
      \param [in] ps the particles that scored by the restraint
      \return the value of the restraint when its particles are in
               a combination defined by cs.
   */
void calc_scores(const Combinations &comb_states_,
                 std::map<std::string, float> &result_cache,
                 Restraint *r, Particles *ps) {
  Combinations read_combs;
  read_combinations(get_restraint_file(r),read_combs);
  std::string key;
  for(Combinations::const_iterator it = comb_states_.begin();
      it != comb_states_.end(); it++) {
    CombState *cs = (*it)->second;
    key=cs->partial_key(ps);
    result_cache[key]=read_combs[key].get_total_score();
  }
}
protected:
  //TODO - this function should be changed once we will have the
  //new kernel functionalities.
  void update_score_states(IMP::Particles &ps) {
    if (ps.size() == 0) {
      IMP_WARN("calling update score states with no particles");
      return;
    }
    Model *m = ps[0]->get_model();
    ScoreStates score_states =get_used_score_states(ps,m);
    IMP_LOG(VERBOSE,"there are " << score_states.size() <<
            " score states"<<std::endl);
    for(ScoreStates::const_iterator it = score_states.begin();
        it != score_states.end(); it++) {
      (*it)->before_evaluate();
    }
  }
  DiscreteSampler *ds_;
};

IMPDOMINO_END_INTERNAL_NAMESPACE

#endif /* IMPDOMINO_RESTRAINT_EVALUATOR_FROM_FILE_H */
