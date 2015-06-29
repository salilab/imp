/**
 * \file IMP/integrative_docking/Result.cpp
 * \brief
 *
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/integrative_docking/internal/Result.h>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

int read_results_file(const std::string file_name,
                      std::vector<Result>& results) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IMP::IOException);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line);  // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("|"),
                 boost::token_compress_on);
    if (split_results.size() < 5) continue;
    int number = atoi(split_results[0].c_str());
    float score = atof(split_results[1].c_str());
    bool filtered = true;  // + value
    if (split_results[2].find("-") != std::string::npos)
      filtered = false;  // - found
    float z_score = atof(split_results[3].c_str());
    std::string transformation = split_results[split_results.size() - 1];
    boost::trim(transformation);

    // parse transformation
    std::vector<std::string> split_results2;
    boost::split(split_results2, transformation, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results2.size() != 6) continue;

    IMP::algebra::Rotation3D rotation =
        IMP::algebra::get_rotation_from_fixed_xyz(
            atof(split_results2[0].c_str()), atof(split_results2[1].c_str()),
            atof(split_results2[2].c_str()));
    IMP::algebra::Vector3D translation(atof(split_results2[3].c_str()),
                                       atof(split_results2[4].c_str()),
                                       atof(split_results2[5].c_str()));
    IMP::algebra::Transformation3D trans(rotation, translation);
    Result r(number, score, filtered, z_score, trans);
    results.push_back(r);
  }
  in_file.close();
  return results.size();
}

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE
