/**
 * \file common.h
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_COMMON_H
#define RMF_COMMON_H
#include <boost/program_options.hpp>
#include <RMF/profile.h>
#include <RMF/utility.h>
#include <RMF/HDF5/ConstFile.h>
#include <RMF/log.h>

namespace {
extern std::string description;

std::vector<std::string> positional_names;
boost::program_options::options_description options,
                                            positional_options;
boost::program_options::variables_map variables_map;
bool verbose = false;
boost::program_options::positional_options_description
  positional_options_description;
void print_help_and_exit(char *argv[]) {
  std::cerr << description << std::endl;
  std::cerr << "Usage: " << argv[0] << " ";
  for (unsigned int i = 0; i < positional_names.size(); ++i) {
    std::cerr << positional_names[i] << " ";
  }
  std::cerr << std::endl;

  std::cerr << options << std::endl;
  exit(1);
}


boost::program_options::variables_map process_options(int argc, char *argv[]) {
  boost::program_options::options_description all;
  std::string log_level("Info");
  options.add_options() ("help,h", "Get help on command line arguments.")
    ("verbose,v", "Produce more output.")
      ("hdf5-errors", "Show hdf5 errors.")
      ("log-level", boost::program_options::value< std::string >(&log_level),
       "What log level to use: Trace, Info, Warn, Error, Off");
#ifdef RMF_USE_GOOGLE_PERFTOOLS_PROFILE
  options.add_options() ("profile", "Profile execution.");
#endif
  all.add(positional_options).add(options);
  boost::program_options::store(
    boost::program_options::command_line_parser(argc,
                                                argv).options(all)
    .positional(positional_options_description).run(),
    variables_map);
  boost::program_options::notify(variables_map);
  if (variables_map.count("help")) {
    print_help_and_exit(argv);
  }
  if (variables_map.count("verbose")) {
    verbose = true;
  }
  if (variables_map.count("hdf5-errors")) {
    RMF::HDF5::set_show_errors(true);
  }
  if (variables_map.count("profile")) {
    RMF::set_is_profiling(true);
  }
  RMF::set_log_level(log_level);
  return variables_map;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
inline void increment_frames(int &current_frame, const int frame_step,
                      int &frame_iteration) {
  if (frame_iteration % 10 == 0) {
    std::cout << "processed frame " << current_frame << std::endl;
  }
  current_frame += frame_step;
  ++frame_iteration;
}
#pragma clang diagnostic pop
}

#define RMF_ADD_INPUT_FILE(type)                                         \
  std::string input;                                                     \
  positional_names.push_back(type);                                      \
  positional_options.add_options()                                       \
  ("input-file,i", boost::program_options::value< std::string >(&input), \
   "input "                                                              \
   type                                                                  \
   " file");                                                             \
  positional_options_description.add("input-file", 1)

#define RMF_ADD_OUTPUT_FILE(type)                                          \
  std::string output;                                                      \
  positional_names.push_back(type);                                        \
  positional_options.add_options()                                         \
  ("output-file,i", boost::program_options::value< std::string >(&output), \
   "output "                                                               \
   type                                                                    \
   " file");                                                               \
  positional_options_description.add("output-file", 1)

#define RMF_ADD_FRAMES                                                        \
  int frame_option = 0;                                                       \
  int begin_frame, end_frame, frame_step;                                     \
  options.add_options() ("frame,f",                                           \
                         boost::program_options::value< int >(&frame_option), \
                         "Frame to use, if negative, use every kth frame");

#define RMF_FOR_EACH_FRAME(nf)                                              \
  if (frame_option >= 0) {                                                  \
    begin_frame = frame_option;                                             \
    end_frame = begin_frame + 1;                                            \
    frame_step = 1;                                                         \
  } else {                                                                  \
    begin_frame = 0;                                                        \
    end_frame = nf;                                                         \
    frame_step = -frame_option;                                             \
  }                                                                         \
  std::cout << "Processing frames [" << begin_frame                         \
            << "..." << end_frame << ":" << frame_step << ")" << std::endl; \
  for (int current_frame = begin_frame, frame_iteration = 0;                \
       current_frame < end_frame;                                           \
       increment_frames(current_frame, frame_step, frame_iteration))
#endif  /* RMF_COMMON_H */
