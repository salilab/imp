/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/flags.h>
#include <IMP/base/file.h>
#include <IMP/base/types.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/internal/static.h>
#include <IMP/base/internal/directories.h>
#if IMP_BASE_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
#include <gperftools/heap-profiler.h>
#endif

IMPBASE_BEGIN_NAMESPACE

AddStringFlag::AddStringFlag(std::string name,
                             std::string description,
                             std::string *storage) {
  internal::flags.add_options()
    (name.c_str(),  boost::program_options::value< std::string >(storage)
     ->default_value(*storage),
     description.c_str());
}

AddIntFlag::AddIntFlag(std::string name,
                       std::string description,
                       int *storage) {
  internal::flags.add_options()
    (name.c_str(), boost::program_options::value< int >(storage)
     ->default_value(*storage),
     description.c_str());
}

AddFloatFlag::AddFloatFlag(std::string name,
                       std::string description,
                       double *storage) {
  internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value< double >(storage)
     ->default_value(*storage),
     description.c_str());
}

AddBoolFlag::AddBoolFlag(std::string name,
                       std::string description,
                       bool *storage) {
  internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value< bool >(storage)->zero_tokens(),
     description.c_str());
}

std::string get_executable_name() {
  return internal::exe_name;
}

void add_string_flag(std::string name,
                     std::string default_value,
                     std::string description) {
  internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value<std::string>()->default_value(default_value),
     description.c_str());
}

std::string get_string_flag(std::string name) {
  return internal::variables_map[name].as< std::string >();
}

void add_int_flag(std::string name,
                  int default_value,
                  std::string description) {
  internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value<int>()->default_value(default_value),
     description.c_str());
}

int get_int_flag(std::string name) {
  return internal::variables_map[name].as< int >();
}

void add_bool_flag(std::string name,
                   std::string description) {
   internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value<bool>()->default_value(false)
     ->zero_tokens(),
     description.c_str());
}

bool get_bool_flag(std::string name) {
  return internal::variables_map.count(name);
}

void add_float_flag(std::string name,
                    double default_value,
                    std::string description) {
  internal::flags.add_options()
    (name.c_str(),
     boost::program_options::value<double>()->default_value(default_value),
     description.c_str());
}

double get_float_flag(std::string name) {
  return internal::variables_map[name].as< double >();
}

namespace {
  void initialize() {
    std::string exename= internal::get_file_name(internal::exe_name);
#if IMP_BASE_HAS_GPERFTOOLS
    if (internal::cpu_profile) {
      std::string name=exename+".pprof";
      ProfilerStart(name.c_str());
    }
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
    if (internal::heap_profile) {
      std::string name=exename+".hprof";
      HeapProfilerStart(name.c_str());
    }
#endif
  }
}

std::vector<std::string> setup_from_argv(int argc, char ** argv,
                                         std::string description,
                                         std::string usage,
                                          int num_positional) {
  bool help=false;
  AddBoolFlag hf("help", "Print help", &help);
  IMP_UNUSED(hf);

  bool version=false;
  AddBoolFlag vf("version", "Show version info and exit", &version);
  IMP_UNUSED(vf);


  internal::exe_name= argv[0];

  std::vector<std::string> positional;
  boost::program_options::positional_options_description m_positional;
  boost::program_options::options_description pos;
  pos.add_options()
    ("positional",
     boost::program_options::value<std::vector<std::string> >(&positional)
     ->composing(),
     "");
  boost::program_options::options_description all;
  all.add(internal::flags);
  all.add(pos);
  m_positional.add("positional", -1);
  try {
    boost::program_options::parsed_options parsed
      = boost::program_options::command_line_parser(argc, argv)
      .options(all)
      .positional(m_positional)
      .allow_unregistered()
      .run();
    boost::program_options::store(parsed, internal::variables_map);
    positional=internal::variables_map["positional"]
      .as< std::vector<std::string> >();
  } catch (...) {
    help=true;
  }

  std::cout << help << " " << num_positional << " " << positional.size()
            << std::endl;
  if (help
      || (num_positional == 0 && !positional.empty())
      || (num_positional > 0 && positional.size()
          != static_cast<unsigned int>(num_positional))
      || (num_positional < 0 && positional.size()
          < static_cast<unsigned int>(std::abs(num_positional)))) {
    std::cerr << "Usage: " << argv[0] << " " << usage << std::endl;
    std::cerr << description << std::endl;
    std::cerr << internal::flags << std::endl;
    throw IMP::base::UsageException("Bad arguments");
  }
  if (version) {
    std::cerr << "Version: \"" << get_module_version() << "\"" << std::endl;
#if IMP_BUILD==IMP_DEBUG
    std::cerr << "Build: \"debug\"" << std::endl;
#elif IMP_BUILD==IMP_RELEASE
     std::cerr << "Build: \"release\"" << std::endl;
#elif IMP_BUILD==IMP_FAST
     std::cerr << "Build: \"fast\"" << std::endl;
#endif
  }
  initialize();
  return positional;
}

base::Vector<std::string> setup_from_argv(Strings iargv,
                                          std::string description,
                                          std::string usage,
                                          int num_positional) {
  char ** argv= new char*[iargv.size()];
  for (unsigned int i=0; i < iargv.size(); ++i) {
    argv[i]=const_cast<char*>(iargv[i].c_str());
  }
  std::vector<std::string> ret= setup_from_argv(iargv.size(),
                                                &argv[0],
                                                description,
                                                usage,
                                                num_positional);
  return base::Vector<std::string>(ret.begin(), ret.end());
}

IMPBASE_END_NAMESPACE
