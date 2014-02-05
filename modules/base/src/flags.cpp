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
#include <IMP/base/internal/log.h>
#include <IMP/base/internal/static.h>
#include <IMP/base/random.h>
#include <boost/program_options/parsers.hpp>

#if IMP_BASE_HAS_GPERFTOOLS
#include <gperftools/profiler.h>
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
#include <gperftools/heap-profiler.h>
#endif

IMPBASE_BEGIN_NAMESPACE
AddStringFlag::AddStringFlag(std::string name, std::string description,
                             std::string *storage) {
  internal::flags.add_options()(
      name.c_str(), boost::program_options::value<std::string>(storage)
                        ->default_value(*storage),
      description.c_str());
}

AddIntFlag::AddIntFlag(std::string name, std::string description,
                       boost::int64_t *storage) {
  internal::flags.add_options()(
      name.c_str(), boost::program_options::value<boost::int64_t>(storage)
                        ->default_value(*storage),
      description.c_str());
}

AddFloatFlag::AddFloatFlag(std::string name, std::string description,
                           double *storage) {
  internal::flags.add_options()(
      name.c_str(),
      boost::program_options::value<double>(storage)->default_value(*storage),
      description.c_str());
}

AddBoolFlag::AddBoolFlag(std::string name, std::string description,
                         bool *storage) {
  internal::flags.add_options()(
      name.c_str(), boost::program_options::value<bool>(storage)->zero_tokens(),
      description.c_str());
}

std::string get_executable_name() { return internal::exe_name; }

void add_string_flag(std::string name, std::string default_value,
                     std::string description) {
  internal::flags.add_options()(
      name.c_str(), boost::program_options::value<std::string>()->default_value(
                        default_value),
      description.c_str());
}

std::string get_string_flag(std::string name) {
  return internal::variables_map[name].as<std::string>();
}

void add_int_flag(std::string name, size_t default_value,
                  std::string description) {
  internal::flags.add_options()(
      name.c_str(),
      boost::program_options::value<size_t>()->default_value(default_value),
      description.c_str());
}

size_t get_int_flag(std::string name) {
  return internal::variables_map[name].as<size_t>();
}

void add_bool_flag(std::string name, std::string description) {
  internal::flags.add_options()(name.c_str(),
                                boost::program_options::value<bool>()
                                    ->default_value(false)
                                    ->zero_tokens(),
                                description.c_str());
}

bool get_bool_flag(std::string name) {
  if (!internal::variables_map.count(name)) {
    return false;
  }
  return internal::variables_map[name].as<bool>();
}

void add_float_flag(std::string name, double default_value,
                    std::string description) {
  internal::flags.add_options()(
      name.c_str(),
      boost::program_options::value<double>()->default_value(default_value),
      description.c_str());
}

double get_float_flag(std::string name) {
  return internal::variables_map[name].as<double>();
}

namespace {
void initialize() {
  std::string exename = internal::get_file_name(internal::exe_name);
#if IMP_BASE_HAS_GPERFTOOLS
  if (internal::cpu_profile) {
    std::string name = exename + ".pprof";
    ProfilerStart(name.c_str());
  }
#endif
#if IMP_BASE_HAS_TCMALLOC_HEAPPROFILER
  if (internal::heap_profile) {
    std::string name = exename + ".hprof";
    HeapProfilerStart(name.c_str());
  }
#endif
#if IMP_BASE_HAS_LOG4CXX
  internal::init_logger();
  // since it isn't read from here
  set_log_level(LogLevel(internal::log_level));
#endif

  random_number_generator.seed(
      static_cast<boost::int64_t>(internal::random_seed));
}

bool already_run = false;
}

void write_help(std::ostream &out) {
  out << "Usage: " << get_executable_name() << " " << internal::exe_usage
      << std::endl;
  out << internal::exe_description << std::endl;
  out << internal::flags << std::endl;
  if (internal::help_advanced) {
    out << "Advanced options:" << std::endl;
    out << internal::advanced_flags << std::endl;
  }
  out << "This program is part of IMP, the Integrative Modeling Platform,"
      << std::endl;
  out << "which is Copyright 2007-2013 IMP Inventors." << std::endl;
  out << "For additional information about IMP, "
      << "see <http://integrativemodeling.org>." << std::endl;
}

namespace {

Strings setup_from_argv_internal(int argc, char **argv, std::string description,
                                 std::string usage, int num_positional,
                                 bool allow_unknown) {
  if (already_run) {
    IMP_FAILURE("IMP.base.setup_from_argv can only be run once per invocation"
                << " of IMP.");
  }
  already_run = true;

  if (num_positional != 0) {
    IMP_USAGE_CHECK(!usage.empty(),
                    "You must have a usage string describing your "
                        << "positional arguments");
  }
  internal::exe_usage = usage;
  internal::exe_description = description;

  internal::exe_name = argv[0];

  std::vector<std::string> positional;
  boost::program_options::positional_options_description m_positional;
  boost::program_options::options_description pos;
  pos.add_options()("positional",
                    boost::program_options::value<std::vector<std::string> >(
                        &positional)->composing(),
                    "");
  boost::program_options::options_description all;
  all.add(internal::flags);
  all.add(internal::advanced_flags);
  all.add(pos);
  m_positional.add("positional", -1);
  try {
    boost::program_options::parsed_options parsed =
        boost::program_options::command_line_parser(argc, argv)
            .options(all)
            .positional(m_positional)
            .allow_unregistered()
            .run();
    boost::program_options::store(parsed, internal::variables_map);
    boost::program_options::notify(internal::variables_map);
    if (internal::variables_map.count("positional") != 0) {
      positional =
          internal::variables_map["positional"].as<std::vector<std::string> >();
    }
    if (internal::version) {
      std::cerr << "Version: \"" << get_module_version() << "\"" << std::endl;
      std::cerr << "Checks: " << IMP_HAS_CHECKS << std::endl;
      std::cerr << "Log: " << IMP_HAS_LOG << std::endl;
      exit(0);
    }
    if (internal::help || internal::help_advanced) {
      write_help(std::cerr);
      exit(0);
    }
    if (!allow_unknown) {
      std::vector<std::string> unreq =
          boost::program_options::collect_unrecognized(
              parsed.options, boost::program_options::exclude_positional);
      if (!unreq.empty()) {
        std::ostringstream oss;
        oss << "Unknown options: " << Strings(unreq.begin(), unreq.end());
        throw IMP::base::UsageException(oss.str().c_str());
      }
      if ((num_positional == 0 && !positional.empty()) ||
          (num_positional > 0 &&
           positional.size() != static_cast<unsigned int>(num_positional)) ||
          (num_positional < 0 &&
           positional.size() <
               static_cast<unsigned int>(std::abs(num_positional)))) {
        throw IMP::base::UsageException("Wrong number of positional arguments");
      }
    }

    if (internal::show_seed) {
      std::cerr << "Random seed: " << internal::random_seed << std::endl;
    }

    initialize();
    if (allow_unknown) {
      try {
        std::vector<std::string> ret =
            boost::program_options::collect_unrecognized(
                parsed.options, boost::program_options::include_positional);
        return Strings(ret.begin(), ret.end());
      }
      catch (...) {
        std::cerr << "Error gathering unknow arguments" << std::endl;
        write_help(std::cerr);
        throw IMP::base::UsageException("Bad arguments");
      }
    } else {
      return Strings(positional.begin(), positional.end());
    }
  }
  catch (const std::runtime_error &e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    write_help(std::cerr);
    throw IMP::base::UsageException(e.what());
  }
}
}

void setup_from_argv(int argc, char **argv, std::string description) {
  setup_from_argv(argc, argv, description, "", 0);
}

Strings setup_from_argv(const Strings &iargv, std::string description,
                        std::string usage, int num_positional) {
  char **argv = new char *[iargv.size()];
  for (unsigned int i = 0; i < iargv.size(); ++i) {
    argv[i] = const_cast<char *>(iargv[i].c_str());
  }
  return setup_from_argv(iargv.size(), &argv[0], description, usage,
                         num_positional);
}

Strings setup_from_argv(int argc, char **argv, std::string description,
                        std::string usage, int num_positional) {
  return setup_from_argv_internal(argc, argv, description, usage,
                                  num_positional, false);
}
void setup_from_argv(const Strings &iargv, std::string description) {
  setup_from_argv(iargv, description, std::string(), 0);
}

Strings setup_from_argv_allowing_unknown(int argc, char **argv,
                                         std::string description) {
  return setup_from_argv_internal(argc, argv, description, "various arguments",
                                  -1, true);
}

IMPBASE_END_NAMESPACE
