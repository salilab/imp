/**
 * \file common.h
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_COMMON_H
#define RMF_COMMON_H
#include <boost/program_options.hpp>
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
  std::string log_level("Off");
  options.add_options() ("help,h", "Get help on command line arguments.")
    ("verbose,v", "Produce more output.")
      ("hdf5-errors", "Show hdf5 errors.")
      ("log-level", boost::program_options::value< std::string >(&log_level),
       "What log level to use: Trace, Info, Warn, Error, Off");
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
  RMF::set_log_level(log_level);
  return variables_map;
}
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

#define RMF_ADD_FRAMES                                                  \
  int start_frame = 0;                                                  \
  options.add_options() ("frame,f",                                     \
                         boost::program_options::value< int >(&start_frame), \
                         "First (or only) frame to use");               \
  int step_frame = std::numeric_limits<int>::max();                     \
  options.add_options() ("frame_step,s",                                \
                         boost::program_options::value< int >(&step_frame), \
                         "The step size for frames. Must be > 0.");


#endif  /* RMF_COMMON_H */
