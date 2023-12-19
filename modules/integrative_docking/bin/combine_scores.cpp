/**
 *  \file combine_scores.cpp \brief A program for combination of score files
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/integrative_docking/internal/Result.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <IMP/exception.h>
#include <IMP/check_macros.h>

using namespace IMP::integrative_docking::internal;

int main(int argc, char** argv) {
  try {
    if (argc < 3) {
      std::cerr << "Usage " << argv[0] << " <file1> <weight1> <file2> "
                << "<weight2> <file3> <weight3>...\n";
      std::cerr << "combines files with same docking transformations"
                << " in the following format:\n";
      std::cerr << "# | score | filtered(+/-) |"
                << " z-score | ... | Transformation\n";
      std::cerr << "only filtered values of + are combined. "
                << "The output score is the weighted sum of Z-Scores\n";
      exit(1);
    }

    // print command
    for (int i = 0; i < argc; i++) {
      std::cerr << argv[i] << " ";
      std::cerr << std::endl;
    }

    // read files
    std::vector<std::vector<Result> > results((argc - 1) / 2);
    std::vector<float> weights;
    for (int i = 1; i < argc; i += 2) {
      int trans_num = read_results_file(argv[i], results[(i - 1) / 2]);
      float weight = atof(argv[i + 1]);
      weights.push_back(weight);
      std::cerr << trans_num << " were read from file " << argv[i] << " weight "
                << weight << std::endl;
    }

    // validate same transformation number
    for (unsigned int i = 1; i < results.size(); i++) {
      if (results[i - 1].size() != results[i].size()) {
        IMP_THROW("different number of transformations in input files: "
                      << argv[i] << " " << results[i - 1].size() << " vs. "
                      << argv[i + 2] << " " << results[i].size(),
                  IMP::ValueException);
      }
    }

    // compute new z_scores
    float average = 0.0;
    float std = 0.0;
    int counter = 0;

    for (unsigned int i = 0; i < results[0].size(); i++) {
      float score = 0;
      bool filtered = true;

      for (unsigned int j = 0; j < results.size(); j++) {
        if (results[j][i].is_filtered()) {
          score += weights[j] * results[j][i].get_z_score();
        } else {
          filtered = false;
          break;
        }
      }

      if (filtered) {
        average += score;
        std += (score * score);
        counter++;
      }
    }

    average /= counter;
    std /= counter;
    std -= (average * average);
    std = sqrt(std);

    // output combined file
    // header
    std::cout << "     # |  Score | filt| ZScore | ";
    for (unsigned int j = 0; j < results.size(); j++)
      std::cout << "Score" << j << " | Zscore" << j << " |";
    std::cout << "Transformation" << std::endl;

    // output transforms
    for (unsigned int i = 0; i < results[0].size(); i++) {
      float score = 0;
      bool filtered = true;

      for (unsigned int j = 0; j < results.size(); j++) {
        if (results[j][i].is_filtered()) {
          score += weights[j] * results[j][i].get_z_score();
        } else {
          filtered = false;
          break;
        }
      }

      // compute z-score
      if (filtered) {
        float z_score = (score - average) / std;
        std::cout.width(6);
        std::cout << results[0][i].get_number() << " | ";
        std::cout.setf(std::ios::fixed, std::ios::floatfield);
        std::cout.width(6);
        std::cout.precision(3);
        std::cout << score << " |  +  | " << z_score << " | ";
        for (unsigned int j = 0; j < results.size(); j++) {
          std::cout.width(6);
          std::cout.precision(3);
          std::cout << results[j][i].get_score() << " | ";
          std::cout.width(6);
          std::cout.precision(3);
          std::cout << results[j][i].get_z_score() << " | ";
        }
        IMP::algebra::FixedXYZ euler_angles =
            IMP::algebra::get_fixed_xyz_from_rotation(
                results[0][i].get_transformation().get_rotation());
        IMP::algebra::Vector3D translation =
            results[0][i].get_transformation().get_translation();
        std::cout.precision(3);
        std::cout << " " << euler_angles.get_x() << " " << euler_angles.get_y()
                  << " " << euler_angles.get_z() << " " << translation[0] << " "
                  << translation[1] << " " << translation[2] << std::endl;
        // std::cout << results[0][i].get_transformation() << std::endl;
      }
    }
  }
  catch (IMP::Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
