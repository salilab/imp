/**
 *  \file anchors_reader.cpp
 *  \brief handles reading of anchors data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/anchors_reader.h>
#include <IMP/atom/SecondaryStructureResidue.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
bool is_edges_line(const std::string &line) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
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
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
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
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  //split returns zero length entires as well
  line_split.erase( std::remove_if(line_split.begin(),line_split.end(),
    boost::bind( &std::string::empty, _1 ) ),line_split.end() );
  IMP_USAGE_CHECK(line_split.size() == 2,"wrong edge format for line ("
                  <<line_split.size()<<")"<<
                  line<<" expecting: |point1_ind|point2_ind|"<<std::endl);
  return IntPair(
                 boost::lexical_cast<int>(line_split[0]),
                 boost::lexical_cast<int>(line_split[1]));
}
}

AnchorsData read_anchors_data(const char *txt_fn){
  std::fstream in;
  AnchorsData data;
  in.open(txt_fn, std::fstream::in);
  if (!in.good()) {
    std::cerr<<"Problem opening file " << txt_fn <<
                  " for reading; returning empty anchors data" << std::endl;
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
void write_txt(const std::string &txt_filename,
               const AnchorsData &ad) {
    std::ofstream out;
  out.open(txt_filename.c_str(),std::ios::out);
  out<<"|points|"<<std::endl;
  for( int i=0;i<ad.get_number_of_points();i++) {
    algebra::Vector3D xyz = ad.points_[i];
    out<<"|"<<i<<"|"<<xyz[0]<<"|"<<xyz[1]<<"|"<<xyz[2]<<"|"<<std::endl;
  }
  out<<"|edges|"<<std::endl;
  IntPairs edges=ad.edges_;
  for(IntPairs::const_iterator it = edges.begin(); it != edges.end();it++) {
    out<<"|"<<it->first<<"|"<<it->second<<"|"<<std::endl;
  }
  out.close();
}

namespace {
void write_cmm_helper(std::ostream &out,
                      const std::string &marker_set_name,
                      const algebra::Vector3Ds &nodes,
                      const IntPairs &edges, Floats radii){
  out << "<marker_set name=\"" <<marker_set_name << "\">"<<std::endl;
  for(unsigned int i=0;i<nodes.size();i++) {
    double x = nodes[i][0];
    double y = nodes[i][1];
    double z = nodes[i][2];
    std::string name="";
    double radius;
    if(radii.size() >0 ) {
      radius = radii[i];
    } else {
      radius=1;
    }
    out << "<marker id=\"" << i << "\""
        << " x=\"" << x << "\""
        << " y=\"" << y << "\""
        << " z=\"" << z << "\""
        << " radius=\"" << radius << "\"/>" << std::endl;
    }
  for(IntPairs::const_iterator it = edges.begin(); it != edges.end();it++) {
    out << "<link id1= \"" << it->first
        << "\" id2=\""     << it->second
        << "\" radius=\"1.0\"/>" << std::endl;
  }
  out << "</marker_set>" << std::endl;
}
}

void write_cmm(const std::string &cmm_filename,
               const std::string &marker_set_name,
               const AnchorsData &ad) {
  Floats radii;
  //algebra::get_enclosing_sphere(dpa.get_cluster_vectors(i));
  radii.insert(radii.begin(),ad.get_number_of_points(),5.);
  std::ofstream out;
  out.open(cmm_filename.c_str(),std::ios::out);
  write_cmm_helper(out,marker_set_name,ad.points_,ad.edges_,radii);
  out.close();
}

void AnchorsData::setup_secondary_structure(Model *mdl){
  for (int anum=0;anum<(int)points_.size();anum++){
    IMP_NEW(Particle,ssr_p,(mdl));
    atom::SecondaryStructureResidue default_ssr=
      atom::SecondaryStructureResidue::setup_particle(ssr_p);
    secondary_structure_ps_.push_back(ssr_p);
  }
}

void AnchorsData::set_secondary_structure_probabilities(
                                                 const Particles &ssres_ps,
                                                 const Ints &indices){

  IMP_USAGE_CHECK(secondary_structure_ps_.size()==points_.size(),
                  "Secondary structure has not been set up, "
                  "run AnchorsData::setup_secondary_structure() first");
  int anum;
  for (int ssnum=0;ssnum<(int)ssres_ps.size();ssnum++){
    IMP_USAGE_CHECK(atom::SecondaryStructureResidue::
                    particle_is_instance(ssres_ps[ssnum]),
                    "SSE Particles must be decorated as"
                    "SecondaryStructureResidues");
    if (indices.size()==0) anum=ssnum;
    else anum=indices[ssnum];
    atom::SecondaryStructureResidue(secondary_structure_ps_[anum])
      .set_prob_helix(atom::SecondaryStructureResidue(ssres_ps[ssnum])
                      .get_prob_helix());
    atom::SecondaryStructureResidue(secondary_structure_ps_[anum])
      .set_prob_strand(atom::SecondaryStructureResidue(ssres_ps[ssnum])
                       .get_prob_strand());
    atom::SecondaryStructureResidue(secondary_structure_ps_[anum])
      .set_prob_coil(atom::SecondaryStructureResidue(ssres_ps[ssnum])
                     .get_prob_coil());
  }
}
IMPMULTIFIT_END_NAMESPACE
