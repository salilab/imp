/**
 *  \file SettingsData.cpp
 *  \brief stored a multifit settings data
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/SettingsData.h>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
  std::string join_path(const std::string &path, const std::string &name)
  {
    return path + "/" + boost::lexical_cast<std::string>(name);
  }
}

ComponentHeader parse_component_line(
   const std::string &path,const std::string &line) {
  try {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  IMP_USAGE_CHECK(line_split.size() == 7,
           "Wrong format of input line : not enough fields in line:"<<line);
  ComponentHeader comp;
  comp.set_name(boost::lexical_cast<std::string>(line_split[0]));
  comp.set_filename(join_path(path, line_split[1]));
  comp.set_pdb_ap_fn(join_path(path, line_split[2]));
  comp.set_num_ap(boost::lexical_cast<int>(line_split[3]));
  comp.set_transformations_fn(join_path(path, line_split[4]));
  comp.set_reference_fn(join_path(path, line_split[5]));
  return comp;
  }
  catch (IMP::Exception &e) {
    IMP_THROW("can not parse line:"<<line,IOException);
  }
}
AssemblyHeader parse_assembly_line(
   const std::string & path,const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG(VERBOSE,"going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  IMP_USAGE_CHECK(line_split.size() == 10,
     "Expecting 10 fileds in input line, got "<<
     line_split.size() << " : " <<line);
  AssemblyHeader dens;
  dens.set_dens_fn(join_path(path, line_split[0]));
  dens.set_resolution(boost::lexical_cast<float>(line_split[1]));
  dens.set_spacing(boost::lexical_cast<float>(line_split[2]));
  dens.set_origin(algebra::Vector3D(
    boost::lexical_cast<float>(line_split[3]),
    boost::lexical_cast<float>(line_split[4]),
    boost::lexical_cast<float>(line_split[5])));
  dens.set_pdb_fine_ap_fn(join_path(path, line_split[6]));
  dens.set_pdb_coarse_ap_fn(join_path(path, line_split[7]));
  dens.set_junction_tree_fn(join_path(path, line_split[8]));
  return dens;
}

SettingsData read_settings(const char *filename,const char *data_path) {
  std::fstream in;
  in.open(filename, std::fstream::in);
  IMP_USAGE_CHECK(in.good(), "Problem openning file " << filename <<
                  " for reading " << std::endl);
  std::string line;
  SettingsData header;
  getline(in, line); //skip header line
  int status=0;
  while (!in.eof()) {
    getline(in, line); //skip header line
    std::vector<std::string> line_split;
    boost::split(line_split, line, boost::is_any_of("|"));
    if ((line_split.size() == 7) && (status == 0)) {//protein  line
      header.add_component_header(parse_component_line(data_path,line));
    }
    else if (status==0) {//map header line
      status=1;
    }
    else if (status==1){ //map line
      header.set_assembly_header(parse_assembly_line(data_path,line));
      status=2;
    }
    else {//(status == 2)
      IMP_WARN("the line was not parsed:"<<line);
    }
  }
  in.close();
  header.set_asmb_fn(filename);
  header.set_data_path(data_path);
  return header;
}
void write_settings(const char *filename, const SettingsData &sd) {
  std::ofstream out;
  out.open (filename);
  sd.show_component_header_line(out);
  for (int i=0;i<sd.get_number_of_component_headers();i++) {
    sd.get_component_header(i).show(out);
  }
  sd.show_density_header_line(out);
  sd.get_assembly_header().show(out);
}
IMPMULTIFIT_END_NAMESPACE
