/**
 *  \file restraints_utils.cpp
 *  \brief restraints setup utilities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/restraints_utils.h>
#include <IMP/domino/RestraintGraph.h>


IMPMULTIFIT_BEGIN_NAMESPACE
std::string RestraintFilenameGenerator::get_single_filename(Particle *p) {
  std::stringstream ss;
  ss<<path_<<"singleton_terms_"<<p->get_value(domino::node_name_key())<<".dat";
  return ss.str();
}
std::string
  RestraintFilenameGenerator::get_pairwise_filename(Particle *p1,Particle *p2) {
  std::stringstream ss;
  std::string name1=p1->get_value(domino::node_name_key());
  std::string name2=p2->get_value(domino::node_name_key());
  ss<<path_<<"pairwise_terms_";
  if (name1<name2) {
    ss<<name1<<"_"<<name2<<".dat";
    }
  else {
    ss<<name2<<"_"<<name1<<".dat";
  }
  return ss.str();
}
Floats read_singleton_restraint_values(const std::string &filename) {
  std::ifstream scores_file(filename.c_str());
  IMP_INTERNAL_CHECK(scores_file,"No such scores file "<<filename<<std::endl);
  IMP_LOG(VERBOSE,"reading restraint values from file: "<<filename<<std::endl);
  //read the file
  //int status;
  std::string line;
  //read the header line
  getline(scores_file, line);
  //now parse the data
  typedef std::vector<std::string> split_vector_type;
  split_vector_type split_vec;
  boost::split(split_vec, line, boost::is_any_of("|"));
  IMP_INTERNAL_CHECK(split_vec.size()==2,
                   "file format does not match singleton restraint format\n");
  Floats values;
  while (!scores_file.eof()) {
    if (!getline(scores_file, line)) break;
    boost::split(split_vec, line, boost::is_any_of("|"));
    values.push_back(atof(split_vec[split_vec.size()-2].c_str()));
  }
  IMP_LOG(IMP::VERBOSE,"read " << values.size() << " combinations"<<std::endl);
  return values;
}
PairsValues read_pairwise_restraint_values(const std::string &filename,
      Int first_len,Int second_len) {
  std::ifstream scores_file(filename.c_str());
  IMP_INTERNAL_CHECK(scores_file,"No such scores file "<<filename<<std::endl);
  IMP_LOG(VERBOSE,"reading restraint values from file: "<<filename<<std::endl);
  //read the file
  //int status;
  std::string line;
  //read the header line
  getline(scores_file, line);
  //now parse the data
  typedef std::vector<std::string> split_vector_type;
  split_vector_type split_vec;
  boost::split(split_vec, line, boost::is_any_of("|"));
  IMP_INTERNAL_CHECK(split_vec.size()==3,
                     "file format does not match singleton restraint format\n");
  PairsValues values(first_len,second_len);
  Int values_read=0;
  while (!scores_file.eof()) {
    if (!getline(scores_file, line)) break;
    boost::split(split_vec, line, boost::is_any_of("|"));
    values.set_value(atoi(split_vec[0].c_str()),
                     atoi(split_vec[1].c_str()),
                     atof(split_vec[2].c_str()));
    values_read++;
  }
  IMP_INTERNAL_CHECK(values_read==first_len*second_len,
                     "did not read enough values: " << values_read<<
                     "!="<<first_len*second_len<<" \n");
  return values;
}
IMPMULTIFIT_END_NAMESPACE
