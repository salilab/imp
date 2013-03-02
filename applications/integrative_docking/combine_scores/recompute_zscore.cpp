/**
 *  \file recompute_zscore.cpp \brief A program for combination of score files
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <math.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <IMP/exception.h>
#include <boost/algorithm/string.hpp>

class Result {
public:
  Result(int number, float score, bool filtered, float z_score,
         std::string& transformation) :
    number_(number), score_(score), filtered_(filtered), z_score_(z_score),
    transformation_(transformation) {}

  int get_number() const { return number_; }
  float get_score() const { return score_; }
  bool is_filtered() const { return filtered_; }
  float get_z_score() const { return z_score_; }
  std::string get_transformation() const { return transformation_; }

  void set_z_score(float z_score) { z_score_ = z_score; }

  friend std::ostream& operator<<(std::ostream& s, const Result& p) {
    s.width(6); s << p.number_ << " | ";
    s.precision(3); s.width(6); s << p.score_;
    if(p.filtered_) s << " |  -   | "; else s << " |  +   | ";
    s.width(6); s << p.z_score_ << " | ";
    s.precision(4);
    s << p.transformation_;
    return s;
  }

protected:
  int number_;
  float score_;
  bool filtered_;//true when the Result is good(+ value in file), passes filters
  float z_score_;
  std::string transformation_; // 6 parameters: 3 rotations + 3 translations
};
namespace {
int read_results_file(const std::string file_name,
                      std::vector<Result>& results) {
  std::ifstream in_file(file_name.c_str());
  if(!in_file) {
    IMP_THROW("Can't open file " << file_name, IMP::IOException);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("|"),
                 boost::token_compress_on);
    if (split_results.size() < 5) continue;
    int number = atoi(split_results[0].c_str());
    float score = atof(split_results[1].c_str());
    bool filtered = true; // + value
    if(split_results[2].find("-") != std::string::npos)
      filtered = false; // - found
    float z_score = atof(split_results[3].c_str());
    std::string transformation = split_results[split_results.size()-1];
    Result r(number, score, filtered, z_score, transformation);
    results.push_back(r);
  }
  in_file.close();
  return results.size();
}
}

int main(int argc, char** argv) {
  try {
    if(argc != 2 && argc != 3) {
      std::cerr << "Usage " << argv[0] << " <score_file> [reverse scores]\n";
      std::cerr
        << "recompute z-score column for a file in the following format:\n";
      std::cerr
        << "# | score | filtered (+/-) | z-score | ... | Transformation\n";
      std::cerr << "only filtered values of + are considered. \n";
      std::cerr << "If reverse is set to true (default=false),"
                << "higher scores are better\n";
      exit(1);
    }

    // print command
    for(int i=0; i<argc; i++) {
      std::cerr << argv[i] << " "; std::cerr << std::endl;
    }

    // read files
    std::vector<Result> results;
    std::vector<float> weights;
    int trans_num = read_results_file(argv[1], results);
    std::cerr << trans_num << " were read from file " << argv[1] << std::endl;

    bool reverse_scores = false;
    if(argc == 3) reverse_scores = true;

    // compute new z_scores
    float average = 0.0;
    float std = 0.0;
    int counter = 0;

    float score = 0;
    for(unsigned int i=0; i<results.size(); i++) {
      if(results[i].is_filtered()) {
        score = results[i].get_score();
        average += score;
        std += (score*score);
        counter++;
      }
    }

    average /= counter;
    std /=counter;
    std -= (average*average);
    if(std <= 0.0) std = 0.01;
    std = sqrt(std);
    std::cerr << "average = " << average << " std " << std << std::endl;

    // output combined file
    // header
    std::cout << "     # |  Score | filt| ZScore | ";
    std::cout << "Transformation" << std::endl;

    // output transforms
    for(unsigned int i=0; i<results.size(); i++) {
      score = results[i].get_score();
      std::cout.width(6);
      std::cout << results[i].get_number() << " | ";
      std::cout.setf(std::ios::fixed, std::ios::floatfield);
      std::cout.width(6); std::cout.precision(3);
      std::cout << score;
      float z_score = 0.0;
      if(results[i].is_filtered()) {
        z_score = (score-average)/std;
        if(reverse_scores) z_score = -(score-average)/std;
        std::cout << " |  +  | ";
      } else {
        std::cout << " |  -  | ";
      }
      std::cout << z_score << " | "
                << results[i].get_transformation() << std::endl;
    }
  } catch (IMP::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
