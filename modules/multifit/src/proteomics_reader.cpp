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
namespace {
  bool is_protein_line(const std::string &line,
                       ProteomicsData *dp) {
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

  bool is_interaction_line(const std::string &line,
                           ProteomicsData *dp) {
  int num_allowed_violations=0;
  std::cout<<"INTERATION LINE:"<<line;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  std::cout<<"Size :: "<<line_split.size()<<std::endl;
  if ((line_split.size() != 1) && (line_split.size() != 2))
      return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "interactions")
    return false;
  if (line_split.size()==2) {
    num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
  }
  std::cout<<"num_allowed_violations:"<<num_allowed_violations<<std::endl;
  dp->set_num_allowed_violated_interactions(num_allowed_violations);
  return true;
  }


  bool is_xlink_line(const std::string &line,
                     ProteomicsData *dp) {
    int num_allowed_violations=0;
  std::cout<<"XLINK LINE:"<<line;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if ((line_split.size() != 1)&&(line_split.size() != 2)) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "residue-xlink")
    return false;
  if (line_split.size()==2) {
    num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
  }
  std::cout<<"num_allowed_violations:"<<num_allowed_violations<<std::endl;
  dp->set_num_allowed_violated_xlinks(num_allowed_violations);
  return true;
  }

  bool is_ev_line(const std::string &line,
                  ProteomicsData *dp) {
  int num_allowed_violations=0;
  std::cout<<"EV LINE:"<<line;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if ((line_split.size() != 1)&&(line_split.size() != 2)) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "ev-pairs")
    return false;
  if (line_split.size()==2) {
    num_allowed_violations=boost::lexical_cast<int>(line_split[1]);
  }
  std::cout<<"num_allowed_violations:"<<num_allowed_violations<<std::endl;
  dp->set_num_allowed_violated_ev(num_allowed_violations);
  return true;
}

void parse_xlink_line(
     const std::string &line,
     ProteomicsData *dp){
  std::cout<<"parse_xlink_line:"<<line<<std::endl;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() !=5,
                  "no data to parse. The format of the line should be:\n"<<
                  "|0/1|prot-name|residue-number|prot-name|residue-number|\n");
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  std::cout<<"PARSE:"<<line_split.size()<<std::endl;
  bool use_in_jt = true;
  if (boost::lexical_cast<int>(line_split[0])==0) {
    use_in_jt=false;
  }
  std::string name1 =  boost::lexical_cast<std::string>(line_split[1]);
  int prot1_ind = dp->find(name1);
  int res1_ind= boost::lexical_cast<int>(line_split[2]);
  std::string name2 =  boost::lexical_cast<std::string>(line_split[3]);
  int prot2_ind = dp->find(name2);
  int res2_ind= boost::lexical_cast<int>(line_split[4]);
  std::cout<<"XLINK between "<<name1<<" "<<name2<<std::endl;
  IMP_USAGE_CHECK(prot1_ind != -1,
                  "The protein "<<name1<<
                  " was not specified in the proteins list"<<std::endl);
  IMP_USAGE_CHECK(prot2_ind != -1,
                  "The protein "<<name2<<
                  " was not specified in the proteins list"<<std::endl);
  //todo - for now the residue index is not used
  dp->add_xlink_interaction(prot1_ind,res1_ind,prot2_ind,res2_ind,use_in_jt);
}

void parse_ev_line(
     const std::string &line,
     ProteomicsData *dp){
  std::cout<<"=============="<<std::endl;
  std::cout<<"parse_ev_line:"<<line<<"|"<<line.size()<<std::endl;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 2,
     "no data to parse. the last two tabs should contain header data\n");
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  std::cout<<"PARSE:"<<line_split.size()<<std::endl;
  std::string name1 =  boost::lexical_cast<std::string>(line_split[0]);
  std::string name2 =  boost::lexical_cast<std::string>(line_split[1]);
  int prot1_ind = dp->find(name1);
  int prot2_ind = dp->find(name2);
  std::cout<<"EV restraint between "<<name1<<" "<<name2<<std::endl;
  IMP_USAGE_CHECK(prot1_ind != -1,
                  "The protein "<<name1<<
                  " was not specified in the proteins list"<<std::endl);
  IMP_USAGE_CHECK(prot2_ind != -1,
                  "The protein "<<name2<<
                  " was not specified in the proteins list"<<std::endl);
  //todo - for now the residue index is not used
  dp->add_ev_pair(prot1_ind,prot2_ind);
}

void parse_protein_line(
                        const std::string &line,
                        ProteomicsData *dp){
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  IMP_USAGE_CHECK((line_split.size() >= 3),
         "wrong protein format for line ("<<line_split.size()<<")"<<
         line<<" expecting: |prot_name|start_res|"<<
         "end_res|filename|surface_filename|reference_filename|"<<std::endl);
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
  dp->add_protein(
                  boost::lexical_cast<std::string>(line_split[0]),
                  boost::lexical_cast<int>(line_split[1]),
                  boost::lexical_cast<int>(line_split[2]),
                  prot_filename,surface_filename,
                  ref_filename
                 );
}
  //The format is: |0|prot1|prot2|...|protN|header1|header2|
  //0/1 means if the restraint should be included in junction tree
  //or used just for scoring
void parse_interaction_line(
     const std::string &line,
     ProteomicsData *dp){
  std::cout<<"parse_interaction_line:"<<line<<std::endl;
  Ints inter_prots;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 4,
       "no data to parse. the last two tabs should contain header data\n."
       <<" The format is: |0/1|prot1|prot2|...|protN|header1|header2| \n");
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  std::cout<<"PARSE:"<<line_split.size()<<std::endl;
  bool used_in_jt=true;
  if (boost::lexical_cast<int>(line_split[0])==0)
    used_in_jt=false;
  for(unsigned int i=1;i<line_split.size()-2;i++) {//last two are header
    std::string name =  boost::lexical_cast<std::string>(line_split[i]);
    int index = dp->find(name);
    IMP_USAGE_CHECK(index != -1,
                 "The protein "<<name<<
                 " was not specified in the proteins list"<<std::endl);
    inter_prots.push_back(index);
  }
  dp->add_interaction(inter_prots,used_in_jt);
}
}//end namespace

ProteomicsData read_proteomics_data(const char *prot_fn) {
  std::fstream in;
  ProteomicsData data;
  std::cout<<"===read proteomics data"<<std::endl;
  in.open(prot_fn, std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem openning file " << prot_fn <<
                  " for reading; returning empty proteomics data" << std::endl);
    in.close();
    return data;
  }
  std::cout<<"start iterating lines"<<std::endl;
  std::string line;
  getline(in, line); //skip proteins header line
  std::cout<<"|-----"<<std::endl;
  std::cout<<line<<std::endl;
  getline(in, line); //skip proteins header line
  std::cout<<"|-----"<<std::endl;
  std::cout<<line<<std::endl;
  while ((!in.eof()) && (!is_interaction_line(line,
                                              &data))){
    std::cout<<line<<std::endl;
    parse_protein_line(line,&data);
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while ((!in.eof()) && (!is_xlink_line(line,&data))){
    parse_interaction_line(line,&data);
    std::cout<<"==finish"<<std::endl;
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while (!in.eof() && (!is_ev_line(line,&data))){
    parse_xlink_line(line,&data);
    if (!getline(in, line)) break;
  }
  getline(in, line);
  while (!in.eof()) { //ev lines
    parse_ev_line(line,&data);
    if (!getline(in, line)) break;
  }
  std::cout<<"====END"<<std::endl;
  in.close();
  return data;
}

ProteomicsData get_partial_proteomics_data(
                       const ProteomicsData &pd,
                       const Strings &prot_names) {
  ProteomicsData ret;
  std::map<int,int> index_map;//orig protein index, new protein index
  for (Strings::const_iterator it = prot_names.begin();
       it != prot_names.end(); it++) {
    IMP_INTERNAL_CHECK(pd.find(*it) != -1,"Protein:"<<*it<<" was not found\n");
    int cur_ind=pd.find(*it);
    index_map[cur_ind]=ret.add_protein(pd.get_protein_data(cur_ind));
  }
  //update the interaction map
  for(int i=0;i<pd.get_number_of_interactions();i++) {
    Ints inds = pd.get_interaction(i);
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
      ret.add_interaction(new_inds,pd.get_interaction_used_to_build_jt(i));
    }
  }
  return ret;
}
IMPMULTIFIT_END_NAMESPACE
