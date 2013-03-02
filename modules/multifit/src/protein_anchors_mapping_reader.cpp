/**
 *  \file protein_anchors_mapping_reader.cpp
 *  \brief handles reading matches between a protein and its anchors
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/protein_anchors_mapping_reader.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <IMP/base/file.h>
#include <IMP/multifit/path_reader_writer.h>
#include <IMP/multifit/anchors_reader.h>
#include <boost/algorithm/string.hpp>
#include "boost/tuple/tuple.hpp"

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
#if 0
bool is_anchors_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse for anchors line:"
                                  <<line<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (boost::lexical_cast<std::string>(line_split[0]) != "anchors") {
    return false;
  }
  return true;
}
#endif

bool is_protein_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (boost::lexical_cast<std::string>(line_split[0]) != "protein") {
    return false;
  }
  return true;
}

std::string parse_anchors_line(const std::string &line) {
 typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  return boost::lexical_cast<std::string>(line_split[1]);
}

boost::tuple<std::string, std::string, IntsList>
parse_protein_line(const std::string &config, const std::string &line,
                   int max_paths)
{
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse for protein line:"
                                  <<line<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  //allow no precalculated paths
  IMP_USAGE_CHECK(line_split.size() > 1,
                  "wrong format, should look like |protein|| or "
                  "|protein|paths|"<<std::endl);
  std::cout<<"===== Line split size:"<<line_split.size()<<std::endl;
  IntsList paths;
  std::string paths_fn;
  if (line_split.size()==2) {
    IMP_WARN("No paths for :"<<boost::lexical_cast<std::string>(line_split[1])
             <<std::endl);
  }
  if (line_split.size()>2) {
    paths_fn = base::get_relative_path(config, line_split[2]);
    std::cout<<"PATH FN:"<<paths_fn<<std::endl;
    paths = read_paths(paths_fn.c_str(),max_paths);
  }
  return boost::make_tuple(
    boost::lexical_cast<std::string>(line_split[1]),
    paths_fn,
    paths);
}
}

ProteinsAnchorsSamplingSpace
read_protein_anchors_mapping(multifit::ProteomicsData *prots,
                             const std::string &anchors_prot_map_fn,
                             int max_paths)
{
  ProteinsAnchorsSamplingSpace ret(prots);
  std::fstream in;
  std::cout<<"FN:"<<anchors_prot_map_fn<<std::endl;
  in.open(anchors_prot_map_fn.c_str(), std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem opening file " << anchors_prot_map_fn <<
                  " for reading; returning empty mapping data" << std::endl);
    in.close();
    return ret;
  }
  std::string line;
  //first line should be the anchors line
  getline(in, line);
  std::string anchors_fn = base::get_relative_path(anchors_prot_map_fn,
                                                   parse_anchors_line(line));
  std::cout<<"FN:"<<anchors_fn<<std::endl;
  multifit::AnchorsData anchors_data
            = multifit::read_anchors_data(anchors_fn.c_str());
  ret.set_anchors(anchors_data);
  ret.set_anchors_filename(anchors_fn);
  while (!in.eof()){
    if (!getline(in, line)) break;
    IMP_LOG_VERBOSE("working on line:"<<line);
    IMP_USAGE_CHECK(is_protein_line(line),"the line should be a protein line");
    boost::tuple<std::string,std::string, IntsList> prot_data =
      parse_protein_line(anchors_prot_map_fn, line, max_paths);
    ret.set_paths_for_protein(boost::get<0>(prot_data),
                              boost::get<2>(prot_data));
    ret.set_paths_filename_for_protein(boost::get<0>(prot_data),
                                       boost::get<1>(prot_data));
  }
  return ret;
}
void ProteinsAnchorsSamplingSpace::show(std::ostream &s) const {
  for(std::map<std::string, IntsList>::const_iterator it =
        paths_map_.begin(); it != paths_map_.end();it++) {
    IntsList inds=it->second;
    s<<it->first<<" "<<inds.size()<<std::endl;
    for (unsigned j=0;j<inds.size();j++) {
      s<<inds[j]<<std::endl;
    }
  }
}
ProteinsAnchorsSamplingSpace get_part_of_sampling_space(
             const ProteinsAnchorsSamplingSpace &prots_ss,
             const Strings &prot_names) {
  ProteinsAnchorsSamplingSpace ret(multifit::get_partial_proteomics_data(
     prots_ss.get_proteomics_data(),prot_names));
  ret.set_anchors(prots_ss.get_anchors());
  //add the paths
  for(Strings::const_iterator it = prot_names.begin();
      it != prot_names.end(); it++) {
    ret.set_paths_for_protein(*it,prots_ss.get_paths_for_protein(*it));
  }
  return ret;
}
multifit::SettingsData *get_partial_assembly_setting_data(
             multifit::SettingsData *prots_sd,
             const Strings &prot_names) {
  IMP_NEW(multifit::SettingsData,ret,());
  std::map<std::string,multifit::ComponentHeader *> data_map;
  //store all components
  for (int i=0;i<(int)prots_sd->get_number_of_component_headers();i++) {
    data_map[prots_sd->get_component_header(i)->get_name()]
                 =prots_sd->get_component_header(i);
  }
  //get just the relevant components
  for (Strings::const_iterator it = prot_names.begin();
       it != prot_names.end(); it++) {
    IMP_INTERNAL_CHECK(data_map.find(*it) != data_map.end(),
                       "Protein:"<<*it<<" was not found\n");
    ret->add_component_header(data_map[*it]);
  }
  return ret.release();
}

void write_protein_anchors_mapping(
                   const std::string &anchors_prot_map_fn,
                   const ProteinsAnchorsSamplingSpace &pa,
                   const Strings &prot_names) {
  std::ofstream out;
  out.open(anchors_prot_map_fn.c_str(),std::ios::out);
  out<<"|anchors|"<<pa.get_anchors_filename()<<"|"<<std::endl;
  for (Strings::const_iterator it = prot_names.begin();
       it != prot_names.end(); it++) {
    //write paths
    IntsList paths=pa.get_paths_for_protein(*it);
    std::stringstream ss;
    ss<<*it<<".temp.paths";
    write_paths(paths,ss.str());
    out<<"|protein|"<<*it<<"|"<<ss.str()<<"|"<<std::endl;
  }
  out.close();
}

void write_protein_anchors_mapping(
                   const std::string &anchors_prot_map_fn,
                   const std::string &anchors_fn,
                   const std::vector<std::pair<String,String> > &prot_paths) {
  std::ofstream out;
  out.open(anchors_prot_map_fn.c_str(),std::ios::out);
  out<<"|anchors|"<<anchors_fn<<"|"<<std::endl;
  for (std::vector<std::pair<String,String> >::const_iterator it
              = prot_paths.begin(); it != prot_paths.end(); it++) {
    out<<"|protein|"<<it->first<<"|"<<it->second<<"|"<<std::endl;
  }
  out.close();
}

IMPMULTIFIT_END_NAMESPACE
