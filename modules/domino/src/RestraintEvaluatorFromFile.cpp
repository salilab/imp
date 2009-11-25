/**
 *  \file RestraintEvaluatorFromFile.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/RestraintEvaluatorFromFile.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_NAMESPACE

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

void RestraintEvaluatorFromFile::set_restraint_file(Restraint *r,
                                                    char *filename) {
  IMP_INTERNAL_CHECK(restraint_data_.find(r)== restraint_data_.end(),
                     "file: "<<filename
                     <<" was already assigned to the restraint"<<std::endl);
  IMP_LOG(VERBOSE,"setting restraint file:"<<filename<<":"<<std::endl);
  restraint_data_[r]=std::string(filename);
  IMP_LOG(VERBOSE,"setting restraint file(check):"
                  <<restraint_data_[r]<<":"<<std::endl);
}

std::string RestraintEvaluatorFromFile::get_restraint_file(Restraint *r) const {
  IMP_INTERNAL_CHECK(restraint_data_.find(r) != restraint_data_.end(),
                     "no file assigned to the restraint"<<std::endl);
  IMP_LOG(VERBOSE,"getting restraint file(check):"
                   <<restraint_data_.find(r)->second<<":"<<std::endl);
  return restraint_data_.find(r)->second;
}

void read_combinations(const std::string &filename, Combinations *combs,
                       const Particles &ps) {
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
  //TODO : make sure that the particle names are ok.

  //now parse the data
  typedef std::vector<std::string> split_vector_type;
  split_vector_type split_vec;
  boost::split(split_vec, line, boost::is_any_of("|"));
  IMP_INTERNAL_CHECK(ps.size()==split_vec.size()-1,
      "the number of particles in the file does not corresponde to the "
      <<"number of input particles"<<std::endl);
  for(int i=0;i<ps.size();i++){
    IMP_INTERNAL_CHECK(ps[i]->get_value(node_name_key())==split_vec[i],
          "wrong particle name:"<<ps[i]->get_value(node_name_key())
           <<":"<<split_vec[i]<<":"<<std::endl);
  }
  CombState*   calc_state;
  while (!scores_file.eof()) {
    if (!getline(scores_file, line)) break;
    boost::split(split_vec, line, boost::is_any_of("|"));
    calc_state = new CombState();
    for(int i=0;i<ps.size();i++){
      calc_state->add_data_item(ps[i],atoi(split_vec[i].c_str()));
    }
    calc_state->update_total_score(0.,
                                   atof(split_vec[split_vec.size()-2].c_str()));
    (*combs)[calc_state->partial_key(&ps)]=calc_state;
  }
}
void RestraintEvaluatorFromFile::calc_scores(const Combinations &comb_states,
                 CombinationValues &comb_values,
                 Restraint *r, const Particles &ps) {
  Combinations read_combs;
  IMP_LOG(VERBOSE,"start calculating scores from file:"
                   <<get_restraint_file(r)<<std::endl);
  std::string r_fn=get_restraint_file(r);
  read_combinations(r_fn,&read_combs,ps);
  std::string key;
  for(Combinations::const_iterator it = comb_states.begin();
      it != comb_states.end(); it++) {
    const CombState *cs = it->second;
    key=cs->partial_key(&ps);
    IMP_LOG(VERBOSE,"key:"<<key<<"value:"
                    <<read_combs[key]->get_total_score()<<std::endl);
    comb_values[key]=read_combs[key]->get_total_score();
  }
}

IMPDOMINO_END_NAMESPACE
