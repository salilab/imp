/**
 *  \file path_reader_writer.cpp
 *  \brief Read and write paths.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/path_reader_writer.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <IMP/log.h>
#include <boost/algorithm/string.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
Ints parse_path_line(
                     const std::string &line){
  Ints ret;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of(" "));
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  //split returns zero length entires as well
  for(int i=0;i<(int)line_split.size();i++){
    ret.push_back(boost::lexical_cast<int>(line_split[i]));
  }
  return ret;
}
}//end namespace
IntsList read_paths(const char*txt_filename,int max_paths) {
  std::fstream in;
  IntsList ret;
  in.open(txt_filename, std::fstream::in);
  if (!in.good()) {
    IMP_WARN("Problem opening file " << txt_filename <<
                  " for reading; returning empty path list" << std::endl);
    in.close();
    return ret;
  }
  std::string line;
  getline(in, line);
  int ind=0;
  while (((!in.eof()))&&(ind<max_paths)){
    ret.push_back(parse_path_line(line));
    if (!getline(in, line)) break;
    ++ind;
  }
  //special case the last line
  if (line.size()>0) {
    ret.push_back(parse_path_line(line));
  }
  in.close();
  return ret;
}
void write_paths(const IntsList &paths,const std::string &txt_filename){
  std::ofstream out;
  out.open(txt_filename.c_str(),std::ios::out);
  for( int i=0;i<(int)paths.size();i++){
    for (int j=0;j<(int)paths[i].size();j++){
      out<<paths[i][j]<<" ";
    }
    out<<std::endl;
  }
  out.close();
}

IMPMULTIFIT_END_NAMESPACE
