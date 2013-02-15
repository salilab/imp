/**
 *  \file fitting_solutions_reader_writer.cpp
 *  \brief handles reading and writing of fitting solutions
 *           volume calculation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
FittingSolutionRecord parse_fitting_line(const std::string &line) {
  IMP_LOG_VERBOSE("line:"<<line<<std::endl);
  FittingSolutionRecord fit_sol;
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  IMP_USAGE_CHECK(line.size() > 0,"no data to parse"<<std::endl);
  IMP_LOG_VERBOSE("going to parse:"<<line);
  std::vector<std::string> line_split,fit_rotation_split,fit_translation_split,
    dock_rotation_split,dock_translation_split;
  boost::split(line_split, line, boost::is_any_of("|"));
  IMP_USAGE_CHECK(line_split.size() == 11,
     "FittingSolutionRecord::parse_fitting_line Wrong format of input line : "<<
      "not enough fields:"<<line);
  boost::split(fit_rotation_split, line_split[2], boost::is_any_of(" "));
  IMP_USAGE_CHECK(fit_rotation_split.size() == 4,
     "Wrong format of input line: wrong rotation format "<<
     "(expected 4 blocks and got "<< fit_rotation_split.size()<<")"<<std::endl);
  IMP_LOG_VERBOSE("going to parse translation:"<<line_split[3]<<std::endl);
  boost::split(fit_translation_split, line_split[3], boost::is_any_of(" "));
  IMP_USAGE_CHECK(fit_translation_split.size() == 3,
           "Wrong format of input line: wrong translation format"<<std::endl);
  boost::split(dock_rotation_split, line_split[8], boost::is_any_of(" "));
  IMP_USAGE_CHECK(dock_rotation_split.size() == 4,
     "Wrong format of input line: wrong rotation format "<<
     "(expected 4 blocks and got "<< fit_rotation_split.size()<<")"<<std::endl);
  IMP_LOG_VERBOSE("going to parse translation:"<<line_split[9]<<std::endl);
  boost::split(dock_translation_split, line_split[9], boost::is_any_of(" "));
  IMP_USAGE_CHECK(dock_translation_split.size() == 3,
           "Wrong format of input line: wrong translation format"<<std::endl);
  fit_sol.set_index(boost::lexical_cast<int>(line_split[0]));
  fit_sol.set_solution_filename(
    boost::lexical_cast<std::string>(line_split[1]));
  fit_sol.set_fit_transformation(
     algebra::Transformation3D(
       algebra::Rotation3D(
         boost::lexical_cast<float>(fit_rotation_split[0]),
         boost::lexical_cast<float>(fit_rotation_split[1]),
         boost::lexical_cast<float>(fit_rotation_split[2]),
         boost::lexical_cast<float>(fit_rotation_split[3])),
       algebra::Vector3D(
         boost::lexical_cast<float>(fit_translation_split[0]),
         boost::lexical_cast<float>(fit_translation_split[1]),
         boost::lexical_cast<float>(fit_translation_split[2]))));
  IMP_LOG_VERBOSE("tranformation is set:"
     <<fit_sol.get_fit_transformation()<<std::endl);
  fit_sol.set_match_size(boost::lexical_cast<int>(line_split[4]));
  fit_sol.set_match_average_distance(
     boost::lexical_cast<float>(line_split[5]));
  fit_sol.set_envelope_penetration_score(
     boost::lexical_cast<float>(line_split[6]));
  fit_sol.set_fitting_score(boost::lexical_cast<float>(line_split[7]));
  fit_sol.set_dock_transformation(
     algebra::Transformation3D(
       algebra::Rotation3D(
         boost::lexical_cast<float>(dock_rotation_split[0]),
         boost::lexical_cast<float>(dock_rotation_split[1]),
         boost::lexical_cast<float>(dock_rotation_split[2]),
         boost::lexical_cast<float>(dock_rotation_split[3])),
       algebra::Vector3D(
         boost::lexical_cast<float>(dock_translation_split[0]),
         boost::lexical_cast<float>(dock_translation_split[1]),
         boost::lexical_cast<float>(dock_translation_split[2]))));
  fit_sol.set_rmsd_to_reference(boost::lexical_cast<float>(line_split[10]));
  IMP_LOG_VERBOSE("finish parsing line"<<std::endl);
  return fit_sol;
}
}

FittingSolutionRecords read_fitting_solutions(const char *fitting_fn) {
  std::fstream in;
  FittingSolutionRecords sols;
  in.open(fitting_fn, std::fstream::in);
  if (! in.good()) {
    IMP_WARN("Problem opening file " << fitting_fn <<
             " for reading; returning 0 solutions" << std::endl);
    in.close();
    return sols;
  }
  std::string line;
  getline(in, line); //skip header line
  while (!in.eof()) {
    if (!getline(in, line)) break;
    sols.push_back(parse_fitting_line(line));
  }
  in.close();
  return sols;
}

void write_fitting_solutions(const char *fitting_fn,
                             const FittingSolutionRecords &fit_sols,
                             int num_sols) {
  if (num_sols==-1) {
    num_sols = fit_sols.size();
  }
  std::fstream out;
  out.open(fitting_fn, std::fstream::out);
  IMP_USAGE_CHECK(out.good(), "Problem opening file " <<
                  fitting_fn << " for writing"<<std::endl);
  //write header
  out<<FittingSolutionRecord::get_record_header();
  for(int i=0;i<num_sols;i++) {
    fit_sols[i].show(out);
    out<<std::endl;
  }
  out.close();
}

FittingSolutionRecords convert_em_to_multifit_format(
     const em::FittingSolutions &em_fits) {
  multifit::FittingSolutionRecords output;
  for(int i=0;i<em_fits.get_number_of_solutions();i++) {
    multifit::FittingSolutionRecord rec;
    rec.set_index(i);
    rec.set_fit_transformation(em_fits.get_transformation(i));
    rec.set_fitting_score(em_fits.get_score(i));
    output.push_back(rec);
  }
  return output;
}

em::FittingSolutions convert_multifit_to_em_format(
    const FittingSolutionRecords &multifit_fits) {
  em::FittingSolutions output;
  for(FittingSolutionRecords::const_iterator it = multifit_fits.begin();
      it != multifit_fits.end();it++) {
  for(unsigned int i=0;i<multifit_fits.size();i++)
    output.add_solution(it->get_fit_transformation(),
                        it->get_fitting_score());
  }
  return output;
}

FittingSolutionRecords
  convert_transformations_to_multifit_format(
        const algebra::Transformation3Ds &trans) {
  FittingSolutionRecords ret;
  for (unsigned int i=0;i<trans.size();i++) {
    FittingSolutionRecord rec;
    rec.set_fit_transformation(trans[i]);
    ret.push_back(rec);
  }
  return ret;
}

algebra::Transformation3Ds
  convert_multifit_format_to_transformations(
       const FittingSolutionRecords &recs) {
  algebra::Transformation3Ds ret;
  for (unsigned int i=0;i<recs.size();i++) {
    ret.push_back(recs[i].get_fit_transformation());
  }
  return ret;
}

IMPMULTIFIT_END_NAMESPACE
