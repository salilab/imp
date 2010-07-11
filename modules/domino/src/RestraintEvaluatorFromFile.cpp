/**
 *  \file RestraintEvaluatorFromFile.cpp
 *  \brief A restraint evaluator file
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include "IMP/domino/RestraintEvaluatorFromFile.h"
#include <IMP/log.h>
IMPDOMINO_BEGIN_NAMESPACE

struct sort_by_name {
  bool operator() (Particle *p1,Particle *p2) {
    return (p1->get_value(node_name_key())<p2->get_value(node_name_key()));}
} p_sort;



void write_combinations(const std::string &filename,
                        const Combinations *combs,const Particles &ps) {
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
                                                    const char *filename) {
  IMP_INTERNAL_CHECK(restraint_data_.find(r)== restraint_data_.end(),
                     "file: "<<filename
                     <<" was already assigned to the restraint"<<std::endl);
  IMP_LOG(VERBOSE,"setting restraint file:"<<filename<<":"<<std::endl);
  restraint_data_[r]=std::string(filename);
  IMP_LOG(VERBOSE,"setting restraint file(check):"
                  <<restraint_data_[r]<<":"<<std::endl);
}

std::string RestraintEvaluatorFromFile::get_restraint_file(Restraint *r) const {
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"get restraint file for restraint:");
    IMP_LOG_WRITE(VERBOSE,r->show());
    IMP_LOG(VERBOSE,std::endl);
  }
  IMP_INTERNAL_CHECK(restraint_data_.find(r) != restraint_data_.end(),
                     "no file assigned to the restraint"<<std::endl);
  IMP_LOG(VERBOSE,"getting restraint file(check):"
                   <<restraint_data_.find(r)->second<<":"<<std::endl);
  return restraint_data_.find(r)->second;
}

void read_combinations(const std::string &filename, Combinations *combs,
                       const Particles &ps) {
  //sort particles by their names
  Particles ps_sorted = ps;
  sort(ps_sorted.begin(),ps_sorted.end(),p_sort);
  std::ifstream scores_file(filename.c_str());
  IMP_INTERNAL_CHECK(scores_file,"No such scores file " << filename<<std::endl);
  IMP_LOG(VERBOSE,"reading combinations from file: " << filename<<std::endl);
  //read the file
  //int status;
  std::string line;
  //read the header line
  getline(scores_file, line);
  //now parse the data
  typedef std::vector<std::string> split_vector_type;
  split_vector_type split_vec;
  boost::split(split_vec, line, boost::is_any_of("|"));
  IMP_INTERNAL_CHECK(ps_sorted.size()==split_vec.size()-1,
      "the number of particles in the file does not corresponde to the "
      <<"number of input particles"<<std::endl);
  for(unsigned int i=0;i<ps_sorted.size();i++){
    IMP_INTERNAL_CHECK(ps_sorted[i]->get_value(node_name_key())==split_vec[i],
            "wrong particle name:"<<ps_sorted[i]->get_value(node_name_key())
           <<":"<<split_vec[i]<<":"<<std::endl);
  }
  CombState* calc_state;
  while (!scores_file.eof()) {
    if (!getline(scores_file, line)) break;
    boost::split(split_vec, line, boost::is_any_of("|"));
    calc_state = new CombState();
    for(unsigned int i=0;i<ps_sorted.size();i++){
      calc_state->add_data_item(ps_sorted[i],atoi(split_vec[i].c_str()));
    }
    calc_state->update_total_score(0.,
                                   atof(split_vec[split_vec.size()-2].c_str()));
    (*combs)[calc_state->get_partial_key(ps_sorted)]=calc_state;
  }
  IMP_LOG(IMP::VERBOSE,"read " << combs->size() << " combinations"<<std::endl);
}
void RestraintEvaluatorFromFile::calc_scores(const Combinations &comb_states,
                 CombinationValues &comb_values,
                 Restraint *r,
                 container::ListSingletonContainer *ps) {
  //sort the particles by their names
  std::map<std::string,Particle*> to_sort_ps;
  for(unsigned int i=0;i<ps->get_number_of_particles();i++){
    Particle *p = ps->get_particle(i);
    to_sort_ps[p->get_value(node_name_key())]=p;
  }
  Particles sorted_ps;
  for(std::map<std::string,Particle *>::const_iterator it = to_sort_ps.begin();
                                             it != to_sort_ps.end(); it++) {
    sorted_ps.push_back(it->second);
  }
  Combinations read_combs;
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE,"start calculating scores from file:"
                   <<get_restraint_file(r)<<std::endl);
    IMP_LOG(TERSE,"between particles:");
    for (unsigned int i=0;i<ps->get_number_of_particles();i++) {
      IMP_LOG(TERSE,ps->get_particle(i)->get_name()<<",");
      }
    IMP_LOG(TERSE,std::endl);
  }
  std::string r_fn=get_restraint_file(r);
  read_combinations(r_fn,&read_combs,sorted_ps);
  std::string key;
  for(Combinations::const_iterator it = comb_states.begin();
      it != comb_states.end(); it++) {
    const CombState *cs = it->second;
    key=cs->get_partial_key(sorted_ps);
    IMP_INTERNAL_CHECK(read_combs.find(key) != read_combs.end(),
              "read_combs does not have a key:"<<key<<std::endl);
    IMP_INTERNAL_CHECK(read_combs.find(key)->second != NULL,
        "read_combs has a NULL value for key:"<<key<<std::endl);
    IMP_LOG(VERBOSE,"key:"<<key<<"value:"
                    <<read_combs[key]->get_total_score()<<std::endl);
    comb_values[key]=read_combs[key]->get_total_score();
  }
}

IMPDOMINO_END_NAMESPACE
