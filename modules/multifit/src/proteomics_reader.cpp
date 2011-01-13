/**
 *  \file proteomics_reader.cpp
 *  \brief handles reading proteomics data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/proteomics_reader.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
IMPMULTIFIT_BEGIN_NAMESPACE

bool is_protein_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() != 1) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "proteins")
    return false;
  return true;
}

bool is_interaction_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() != 1) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "interactions")
    return false;
  return true;
}

void parse_protein_line(
                        const std::string &line,
                        ProteomicsData &dp){
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  IMP_USAGE_CHECK(line_split.size() == 4,
         "wrong protein format for line ("<<line_split.size()<<")"<<
         line<<" expecting: |prot_name|start_res|"<<
         "end_res|filename|"<<std::endl);
  dp.add_protein(
                 boost::lexical_cast<std::string>(line_split[0]),
                 boost::lexical_cast<int>(line_split[1]),
                 boost::lexical_cast<int>(line_split[2]),
                 boost::lexical_cast<std::string>(line_split[3]));
}
void parse_interaction_line(
     const std::string &line,
     ProteomicsData &dp){
  std::vector<int> inter_prots;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  for(std::vector<std::string>::iterator it = line_split.begin();
      it != line_split.end();it++) {
    std::string name =  boost::lexical_cast<std::string>(*it);
    int index = dp.find(name);
    IMP_USAGE_CHECK(index != -1,
                 "The protein "<<name<<
                 " was not specified in the proteins list"<<std::endl);
    inter_prots.push_back(index);
  }
  dp.add_interaction(inter_prots);
}

ProteomicsData read_proteomics_data(const char *prot_fn) {
  std::fstream in;
  ProteomicsData data;
  in.open(prot_fn, std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem openning file " << prot_fn <<
                  " for reading; returning empty proteomics data" << std::endl);
    in.close();
    return data;
  }
  std::string line;
  getline(in, line); //skip proteins header line
  getline(in, line); //skip proteins header line
  while ((!in.eof()) && (!is_interaction_line(line))){
    parse_protein_line(line,data);
    if (!getline(in, line)) break;
  }
  while (!in.eof()){
    if (!getline(in, line)) break;
    parse_interaction_line(line,data);
  }
  in.close();
  return data;
}

IMPMULTIFIT_END_NAMESPACE
