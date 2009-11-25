/**
 *  \file RestraintEvaluatorFromFile.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/internal/RestraintEvaluatorFromFile.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

void write_combinations(const std::string &filename,
                        const Combinations *combs,Particles &ps) {
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
    const CombState *c = it->second;
    out_file<<c->get_state_num(ps[0])<<"|"<<
         c->get_state_num(ps[1])<<"|"<<c->get_total_score()<<
         "|"<<std::endl;
  }
  out_file.close();
}
void RestraintEvaluatorFromFile::calc_scores(const Combinations &comb_states,
                 CombinationValues &comb_values,
                 Restraint *r, Particles *ps) {
  Combinations read_combs;
  IMP_LOG(VERBOSE,"start calculating scores from file:"<<
                   get_restraint_file(r)<<std::endl);
  read_combinations(get_restraint_file(r),&read_combs);
  std::string key;
  for(Combinations::const_iterator it = comb_states.begin();
      it != comb_states.end(); it++) {
    const CombState *cs = it->second;
    key=cs->partial_key(ps);
    comb_values[key]=read_combs[key]->get_total_score();
  }
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
  getline(scores_file, line);
  //get the particles names
  Particle *p1,*p2;
  Particles ps; ps.push_back(p1);ps.push_back(p2);
  //now parse the data
  typedef std::vector<std::string> split_vector_type;
  split_vector_type split_vec;
  CombState*   calc_state;
  while (!scores_file.eof()) {
    if (!getline(scores_file, line)) break;
    boost::split(split_vec, line, boost::is_any_of("|"));
    calc_state = new CombState();
    calc_state->add_data_item(p1,atoi(split_vec[0].c_str()));
    calc_state->add_data_item(p2,atoi(split_vec[1].c_str()));
    (*combs)[calc_state->partial_key(&ps)]=calc_state;
  }
}
void RestraintEvaluatorFromFile::update_score_states(IMP::Particles &ps) {
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

IMPDOMINO_END_INTERNAL_NAMESPACE
