/**
 *  \file SettingsData.cpp
 *  \brief stored a multifit settings data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/SettingsData.h>
#include <IMP/base/file.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <fstream>

IMPMULTIFIT_BEGIN_NAMESPACE

IMP_LIST_IMPL(SettingsData, ComponentHeader, component_header,
              ComponentHeader*, ComponentHeaders);

namespace {

ComponentHeader *parse_component_line(
   const std::string &config,const std::string &line) {
  try {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  IMP_USAGE_CHECK(line_split.size() == 10,
           "Wrong format of input line : not enough fields in line:"<<line);
  IMP_NEW(ComponentHeader, comp, ());
  comp->set_name(boost::lexical_cast<std::string>(line_split[0]));
  comp->set_filename(base::get_relative_path(config, line_split[1]));
  comp->set_surface_fn(base::get_relative_path(config, line_split[2]));
  comp->set_txt_ap_fn(base::get_relative_path(config, line_split[3]));
  try {
    comp->set_num_ap(boost::lexical_cast<int>(line_split[4]));
  }
  catch(boost::bad_lexical_cast &) {
    comp->set_num_ap(0);
    IMP_WARN("Can not cast num_ap field for protein: "
             <<comp->get_name()<<std::endl);
  }
  comp->set_txt_fine_ap_fn(base::get_relative_path(config, line_split[5]));
  try{
    comp->set_num_fine_ap(boost::lexical_cast<int>(line_split[6]));
  }
  catch(boost::bad_lexical_cast &) {
    comp->set_num_fine_ap(0);
    IMP_WARN("Can not cast num_fine_ap field for protein: "<<comp->get_name()
             <<" seeting to 0"<<std::endl);
  }
  comp->set_transformations_fn(base::get_relative_path(config, line_split[7]));
  if (line_split[8].size() > 0) {
    comp->set_reference_fn(base::get_relative_path(config, line_split[8]));
  } else {
    comp->set_reference_fn("");
  }
  return comp.release();
  }
  catch (IMP::base::Exception &) {
    IMP_THROW("can not parse line:"<<line,IOException);
  }
}
AssemblyHeader *parse_assembly_line(
   const std::string & config,const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  IMP_USAGE_CHECK(line_split.size() == 12,
     "Expecting 12 fields in input line, got "<<
     line_split.size() << " : " <<line);
  IMP_NEW(AssemblyHeader, dens, ());
  dens->set_dens_fn(base::get_relative_path(config, line_split[0]));
  try{
  dens->set_resolution(boost::lexical_cast<float>(line_split[1]));
  }
  catch(boost::bad_lexical_cast &) {
    dens->set_resolution(1.);
    IMP_WARN("Can not cast resolution field, setting to 1\n");
  }
  try{
    dens->set_spacing(boost::lexical_cast<float>(line_split[2]));
  }
  catch(boost::bad_lexical_cast &) {
    dens->set_spacing(1.);
    IMP_WARN("Can not cast spacing field, setting to 1\n");
  }
  try{
    dens->set_threshold(boost::lexical_cast<float>(line_split[3]));
  }
  catch(boost::bad_lexical_cast &) {
    dens->set_threshold(0.);
    IMP_WARN("Can not cast threshold field, setting to 0\n");
  }
  try{
  dens->set_origin(algebra::Vector3D(
    boost::lexical_cast<float>(line_split[4]),
    boost::lexical_cast<float>(line_split[5]),
    boost::lexical_cast<float>(line_split[6])));
  }
  catch(boost::bad_lexical_cast &) {
    dens->set_origin(algebra::Vector3D(0,0,0));
    IMP_WARN("Can not cast origin field, setting to 0\n");
  }
  dens->set_coarse_ap_fn(base::get_relative_path(config, line_split[7]));
  dens->set_coarse_over_sampled_ap_fn(base::get_relative_path(config,
                                                              line_split[8]));
  dens->set_fine_ap_fn(base::get_relative_path(config, line_split[9]));
  dens->set_fine_over_sampled_ap_fn(base::get_relative_path(config,
                                                            line_split[10]));
  return dens.release();
}
}

SettingsData *read_settings(const char *filename) {
  std::fstream in;
  in.open(filename, std::fstream::in);
  if (!in.good()){
    IMP_THROW("Problem opening file " << filename <<
              " for reading " << std::endl,ValueException);
  }
  std::string line;
  IMP_NEW(SettingsData, header, ());
  getline(in, line); //skip header line
  int status=0;
  while (!in.eof()) {
    getline(in, line); //skip header line
    std::vector<std::string> line_split;
    boost::split(line_split, line, boost::is_any_of("|"));
    if ((line_split.size() == 10) && (status == 0)) {//protein  line
      IMP_LOG_VERBOSE("parsing component line:"<<line<<std::endl);
      header->add_component_header(parse_component_line(filename, line));
    }
    else if (status==0) {//map header line
      status=1;
    }
    else if (status==1){ //map line
      IMP_LOG_VERBOSE("parsing EM line:"<<line<<std::endl);
      header->set_assembly_header(parse_assembly_line(filename, line));
      status=2;
    }
    else if (line.length() > 0) { // don't warn about empty lines
      IMP_WARN("the line was not parsed:"<<line
               <<"| with status:"<<status<<std::endl);
    }
  }
  in.close();
  header->set_assembly_filename(filename);
  header->set_data_path(".");
  return header.release();
}
void write_settings(const char *filename, const SettingsData *sd) {
  std::ofstream out;
  out.open (filename);
  sd->show_component_header_line(out);
  for (unsigned int i=0;i<sd->get_number_of_component_headers();i++) {
    sd->get_component_header(i)->show(out);
  }
  sd->show_density_header_line(out);
  sd->get_assembly_header()->show(out);
}
IMPMULTIFIT_END_NAMESPACE
