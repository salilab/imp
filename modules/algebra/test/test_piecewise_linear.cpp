/**
 *  \file test_grid.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include <IMP/piecewise_linear_distribution.h>
#include <IMP/exception.h>
#include <IMP/types.h>
#include <IMP/random.h>
#include <IMP/utility.h>
#include <IMP/flags.h>

namespace {
template <class RNG>
void check_one(RNG rng, double mean, double sigma) {
  const int num = 1000;
  IMP::Floats sample(num);
  for (int i = 0; i < num; ++i) {
    sample[i] = rng(IMP::random_number_generator);
  }
  double nmean = std::accumulate(sample.begin(), sample.end(), 0.0) / num;
  double ssq = 0;
  for (unsigned int i = 0; i < sample.size(); ++i) {
    ssq += IMP::square(sample[i] - nmean);
  }
  double ssd = std::sqrt(ssq / num);
  std::cout << mean << " " << sigma << " " << nmean << " " << ssd << std::endl;
  if (std::abs(mean - nmean) > .1 * (mean + nmean)) {
    IMP_THROW("Means don't match: " << mean << " " << nmean,
              IMP::ValueException);
  }
  if (std::abs(sigma - ssd) > .1 * (sigma + ssd)) {
    IMP_THROW("Stddevs don't match: " << sigma << " " << ssd,
              IMP::ValueException);
  }
}

void check_gaussian() {
  double s = 3.0;
  double m = 4.0;
  IMP::Floats steps;
  for (unsigned int i = 0; i < 15; ++i) {
    steps.push_back(.1 * std::pow(1.2, static_cast<double>(i)));
  }
  IMP::Floats locs0(1, 0);
  for (unsigned int i = 0; i < steps.size(); ++i) {
    locs0.push_back(locs0.back() + steps[i]);
  }
  using IMP::operator<<;
  // std::cout << locs0 << std::endl;
  IMP::Floats locs(locs0.rbegin(), locs0.rend());
  for (unsigned int i = 0; i < locs.size(); ++i) {
    locs[i] = -locs[i];
  }
  // std::cout << locs << std::endl;
  locs.pop_back();
  locs.insert(locs.end(), locs0.begin(), locs0.end());
  // std::cout << locs << std::endl;
  for (unsigned int i = 0; i < locs.size(); ++i) {
    locs[i] += m;
  }
  IMP::Floats vals(locs.size());
  for (unsigned int i = 0; i < vals.size(); ++i) {
    vals[i] = 1.0 / std::sqrt(2.0 * 3.1415 * s * s) *
              std::exp(-IMP::square(locs[i] - m) / (2 * s * s));
  }
  IMP::piecewise_linear_distribution<> pld(locs.begin(), locs.end(),
                                                 vals.begin());
  check_one(pld, m, s);
}

void check_triangle() {
  double locs[] = {0, 1, 2};
  double weights[] = {0, 1, 0};
  IMP::piecewise_linear_distribution<> pld(locs, locs + 3, weights);
  check_one(pld, 1, 1.0 / std::sqrt(6.0));
}
}

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test piecewise linear.");
  check_triangle();
  check_gaussian();
  return 0;
}
