/**
 *  \file anchors_reader.h
 *  \brief handles reading of anchors data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/anchors_reader.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
IMPMULTIFIT_BEGIN_NAMESPACE

bool is_edges_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  if (line_split.size() != 1) return false;
  if (boost::lexical_cast<std::string>(line_split[0]) != "edges")
    return false;
  return true;
}

algebra::Vector3D parse_point_line(
                                   const std::string &line){
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  IMP_USAGE_CHECK(line_split.size() == 4,
          "wrong point format for line ("<<line_split.size()<<")"<<
          line<<" expecting: |point_ind|x|y|z|"<<std::endl);
  return algebra::Vector3D(
                    boost::lexical_cast<float>(line_split[1]),
                    boost::lexical_cast<float>(line_split[2]),
                    boost::lexical_cast<float>(line_split[3]));
}

IntPair parse_edge_line(const std::string &line){
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero lenght entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  IMP_USAGE_CHECK(line_split.size() == 2,"wrong edge format for line ("
                  <<line_split.size()<<")"<<
                  line<<" expecting: |point1_ind|point2_ind|"<<std::endl);
  return IntPair(
                 boost::lexical_cast<int>(line_split[0]),
                 boost::lexical_cast<int>(line_split[1]));
}

AnchorsData read_anchors_data(const char *txt_fn){
  std::fstream in;
  AnchorsData data;
  in.open(txt_fn, std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem openning file " << txt_fn <<
                  " for reading; returning empty anchors data" << std::endl);
    in.close();
    return data;
  }
  std::string line;
  getline(in, line); //skip points header line
  getline(in, line);
  while ((!in.eof()) && (!is_edges_line(line))){
    data.points_.push_back(parse_point_line(line));
    if (!getline(in, line)) break;
  }
  while (!in.eof()){
    if (!getline(in, line)) break;
    data.edges_.push_back(parse_edge_line(line));
  }
  in.close();
  return data;
}

IMPMULTIFIT_END_NAMESPACE
