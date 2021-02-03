/**
 *  \file IMP/foxs/Gnuplot.h   \brief A class for printing gnuplot scripts
 *   for profile viewing
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPFOXS_GNUPLOT_H
#define IMPFOXS_GNUPLOT_H

#include <IMP/foxs/foxs_config.h>
#include <IMP/saxs/FitParameters.h>

#include <string>
#include <vector>

IMPFOXS_BEGIN_INTERNAL_NAMESPACE

#if defined(_WIN32) || defined(_WIN64)
// Simple basename implementation on platforms that don't have libgen.h
namespace {
const char* basename(const char* path) {
  int i;
  for (i = path ? strlen(path) : 0; i > 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      return &path[i + 1];
    }
  }
  return path;
}
}
#else
#include <libgen.h>
#endif

class IMPFOXSEXPORT Gnuplot {
 public:
  // output profile
  static void print_profile_script(const std::string pdb);

  // output multiple profiles
  static void print_profile_script(const std::vector<std::string>& pdbs);

  // output multiple profiles - canvas gnuplot terminal
  static void print_canvas_script(const std::vector<std::string>& pdbs,
                                  int max_num);

  // output fit - png & eps gnuplot terminal
  static void print_fit_script(const IMP::saxs::FitParameters& fp);

  // output multiple fits - png gnuplot terminal
  static void print_fit_script(
      const std::vector<IMP::saxs::FitParameters>& fps);

  // output multiple fits - canvas gnuplot terminal
  static void print_canvas_script(
      const std::vector<IMP::saxs::FitParameters>& fps, int max_num);
};

IMPFOXS_END_INTERNAL_NAMESPACE

#endif /* IMPFOXS_GNUPLOT_H */
