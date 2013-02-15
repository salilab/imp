/**
 *  \file proteomics_reader.cpp
 *  \brief handles reading proteomics data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/proteomics_reader.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
  bool is_interaction_header_line(const std::string &line, ProteomicsData *dp) {
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if ((line_split.size() != 1) && (line_split.size() != 2))
      return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "interactions")
    return false;
  try {
    int num_allowed_violations=0;
    if (line_split.size()==2) {
      num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
    }
    dp->set_num_allowed_violated_interactions(num_allowed_violations);
    return true;
  } catch (std::bad_cast &) {
    IMP_THROW("Improperly formatted interaction header line: >>" << line
              << "<< The format of the line should be: "
              << "|interactions|num_allowed_violations|", ValueException);
  }
  }


  bool is_xlink_header_line(const std::string &line, ProteomicsData *dp) {
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if ((line_split.size() != 1)&&(line_split.size() != 2)) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "residue-xlink")
    return false;

  try {
    int num_allowed_violations=0;
    if (line_split.size()==2) {
      num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
    }
    dp->set_num_allowed_violated_cross_links(num_allowed_violations);
    return true;
  } catch (std::bad_cast &) {
    IMP_THROW("Improperly formatted cross link header line: >>" << line
              << "<< The format of the line should be: "
              << "|residue-xlink|num_allowed_violations|", ValueException);
  }
  }

  bool is_ev_header_line(const std::string &line, ProteomicsData *dp) {
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if ((line_split.size() != 1)&&(line_split.size() != 2)) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "ev-pairs")
    return false;

  try {
    int num_allowed_violations=0;
    if (line_split.size()==2) {
      num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
    }
    dp->set_num_allowed_violated_ev(num_allowed_violations);
    return true;
  } catch (std::bad_cast &) {
    IMP_THROW("Improperly formatted excluded volume header line: >>" << line
              << "<< The format of the line should be: "
              << "|ev-pairs|num_allowed_violations|", ValueException);
  }

}

void error_xlink_line(const std::string &line)
{
  IMP_THROW("Improperly formatted cross link line: >>" << line << "<<"
            << " The format of the line should be: "
            << "|0/1|prot-name|residue-number|prot-name|residue-number|"
            << "linker length|", ValueException);
}

void parse_xlink_line(
     const std::string &line,
     ProteomicsData *dp){
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() != 6) {
    error_xlink_line(line);
  }
  bool use_in_jt = true;

  try {
    if (boost::lexical_cast<int>(line_split[0])==0) {
      use_in_jt=false;
    }
    std::string name1 =  boost::lexical_cast<std::string>(line_split[1]);
    int prot1_ind = dp->find(name1);
    int res1_ind= boost::lexical_cast<int>(line_split[2]);
    std::string name2 =  boost::lexical_cast<std::string>(line_split[3]);
    int prot2_ind = dp->find(name2);
    int res2_ind= boost::lexical_cast<int>(line_split[4]);
    Float linker_len=boost::lexical_cast<double>(line_split[5]);
    IMP_USAGE_CHECK(prot1_ind != -1,
                    "The protein "<<name1<<
                    " was not specified in the proteins list"<<std::endl);
    IMP_USAGE_CHECK(prot2_ind != -1,
                    "The protein "<<name2<<
                    " was not specified in the proteins list"<<std::endl);
    dp->add_cross_link_interaction(prot1_ind,res1_ind,prot2_ind,res2_ind,
                                   use_in_jt, linker_len);
  } catch (std::bad_cast &) {
    error_xlink_line(line);
  }
}

void error_ev_line(const std::string &line)
{
  IMP_THROW("Improperly formatted ev line: >>" << line << "<<"
            << " The format should be: |prot1|prot2|", ValueException);
}

void parse_ev_line(
     const std::string &line,
     ProteomicsData *dp){
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() < 2) {
    error_ev_line(line);
  }
  try {
    std::string name1 =  boost::lexical_cast<std::string>(line_split[0]);
    std::string name2 =  boost::lexical_cast<std::string>(line_split[1]);
    int prot1_ind = dp->find(name1);
    int prot2_ind = dp->find(name2);
    IMP_USAGE_CHECK(prot1_ind != -1,
                    "The protein "<<name1<<
                    " was not specified in the proteins list"<<std::endl);
    IMP_USAGE_CHECK(prot2_ind != -1,
                    "The protein "<<name2<<
                    " was not specified in the proteins list"<<std::endl);
    //todo - for now the residue index is not used
    dp->add_ev_pair(prot1_ind,prot2_ind);
  } catch (std::bad_cast &) {
    error_ev_line(line);
  }
}

void error_protein_line(const std::string &line)
{
  IMP_THROW("Improperly formatted protein line: >>" << line << "<<"
            << " The format should be: |prot_name|start_res|"
            << "end_res|filename|surface_filename|reference_filename|",
            ValueException);
}

void parse_protein_line(const std::string &line,
                        ProteomicsData *dp){
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() < 3) {
    error_protein_line(line);
  }
  try {
    std::string ref_filename="";
    if (line_split.size()==6) {
      ref_filename=boost::lexical_cast<std::string>(line_split[5]);
    }
    std::string prot_filename="";
    std::string surface_filename="";
    if (line_split.size()>=5){
      prot_filename=boost::lexical_cast<std::string>(line_split[3]);
      surface_filename=boost::lexical_cast<std::string>(line_split[4]);}
    if (!boost::iends_with(prot_filename,".pdb")) {
      prot_filename="";
    }
    if (!boost::iends_with(surface_filename,".ms")) {
      surface_filename="";
    }
    dp->add_protein(boost::lexical_cast<std::string>(line_split[0]),
                    boost::lexical_cast<int>(line_split[1]),
                    boost::lexical_cast<int>(line_split[2]),
                    prot_filename,surface_filename,
                    ref_filename);
  } catch (std::bad_cast &) {
    error_protein_line(line);
  }
}
  //The format is: |0|prot1|prot2|...|protN|linker length|header1|header2|
  //0/1 means if the restraint should be included in junction tree
  //or used just for scoring

void error_interaction_line(const std::string &line)
{
  IMP_THROW("Improperly formatted interaction line: >>" << line << "<<"
            << " The format should be: |0/1|prot1|prot2|...|protN|"
            << "linker length|header1|header2|", ValueException);
}

void parse_interaction_line(
     const std::string &line,
     ProteomicsData *dp){
  Ints inter_prots;
  if (line.size() <= 5) {
    error_interaction_line(line);
  }
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  bool used_in_filter=true;
  try {
    if (boost::lexical_cast<int>(line_split[0])==0)
      used_in_filter=false;
    float linker_len=boost::lexical_cast<float>(
                                   line_split[line_split.size()-3]);
    for(unsigned int i=1;i<line_split.size()-3;i++) {
      //last three are linker and header
      std::string name =  boost::lexical_cast<std::string>(line_split[i]);
      int index = dp->find(name);
      IMP_USAGE_CHECK(index != -1,
                      "The protein "<<name<<
                      " was not specified in the proteins list"<<std::endl);
      inter_prots.push_back(index);
    }
    dp->add_interaction(inter_prots,used_in_filter,linker_len);
  } catch (std::bad_cast &) {
    error_interaction_line(line);
  }
}



}//end namespace

ProteomicsData *read_proteomics_data(const char *prot_fn) {
  std::fstream in;
  IMP_NEW(ProteomicsData, data, ());
  in.open(prot_fn, std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem opening file " << prot_fn <<
                  " for reading; returning empty proteomics data" << std::endl);
    in.close();
    return data.release();
  }
  std::string line;
  getline(in, line); //skip proteins header line
  getline(in, line); //skip proteins header line
  while ((!in.eof()) && (!is_interaction_header_line(line, data))){
    parse_protein_line(line,data);
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while ((!in.eof()) && (!is_xlink_header_line(line,data))){
    parse_interaction_line(line,data);
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while (!in.eof() && (!is_ev_header_line(line,data))){
    parse_xlink_line(line,data);
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while (!in.eof()) { //ev lines
    parse_ev_line(line,data);
    if (!getline(in, line)) break;
    if (line.size()==0) break;
  }
  in.close();
  return data.release();
}

ProteomicsData *get_partial_proteomics_data(
                       const ProteomicsData *pd,
                       const Strings &prot_names) {
  IMP_NEW(ProteomicsData, ret, ());
  std::map<int,int> index_map;//orig protein index, new protein index
  for (Strings::const_iterator it = prot_names.begin();
       it != prot_names.end(); it++) {
    IMP_INTERNAL_CHECK(pd->find(*it) != -1,"Protein:"<<*it<<" was not found\n");
    int cur_ind=pd->find(*it);
    index_map[cur_ind]=ret->add_protein(pd->get_protein_data(cur_ind));
  }
  //update the interaction map
  for(int i=0;i<pd->get_number_of_interactions();i++) {
    Ints inds = pd->get_interaction(i);
    //check if all of the proteins are in the new list
    bool found=true;
    for(Ints::iterator it = inds.begin(); it != inds.end();it++) {
      if (index_map.find(*it) == index_map.end())
        found=false;
    }
    //add the interaction
    if (found) {
      Ints new_inds;
      for(Ints::iterator it = inds.begin(); it != inds.end();it++) {
        new_inds.push_back(index_map[*it]);
      }
      ret->add_interaction(new_inds,pd->get_interaction_part_of_filter(i),
                           pd->get_interaction_linker_length(i));
    }
  }
  return ret.release();
}
IMPMULTIFIT_END_NAMESPACE
