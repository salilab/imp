/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <RMF/log.h>
#include <RMF/exceptions.h>
#if RMF_HAS_LOG4CXX
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/level.h>
#endif

RMF_ENABLE_WARNINGS

namespace RMF {
#if RMF_HAS_LOG4CXX
namespace {
  template <class T>
  void use(const T &){}
struct Configurator {
  Configurator(log4cxx::ConsoleAppenderPtr ptr) {
    log4cxx::BasicConfigurator::configure(ptr);
  }
};

void do_init() {
 // "%-4r [%t] %-5p %c %x - %m%n"
  static log4cxx::PatternLayoutPtr layout
      = new log4cxx::PatternLayout(LOG4CXX_STR("%-6r %-5p %c- %m%n"));
  static log4cxx::ConsoleAppenderPtr appender
      = new log4cxx::ConsoleAppender(layout);
  static Configurator config(appender);
  use(config);
}

void init_logger() {
  log4cxx::LoggerPtr rootLogger =
      log4cxx::Logger::getRootLogger();
  bool uninit = rootLogger->getAllAppenders().empty() ? true : false;
  if (uninit) do_init();
}
}
log4cxx::LoggerPtr get_logger() {
  init_logger();
  static log4cxx::LoggerPtr ret = log4cxx::Logger::getLogger("RMF");
  return ret;
}
log4cxx::LoggerPtr get_avro_logger() {
  init_logger();
  static log4cxx::LoggerPtr ret = log4cxx::Logger::getLogger("RMF.avro");
  return ret;
}
log4cxx::LoggerPtr get_hdf5_logger() {
  init_logger();
  static log4cxx::LoggerPtr ret = log4cxx::Logger::getLogger("RMF.hdf5");
  return ret;
}
void set_log_level(std::string str) {
  try {
    get_logger()->setLevel(log4cxx::Level::toLevel(str));
    get_avro_logger()->setLevel(log4cxx::Level::toLevel(str));
    get_hdf5_logger()->setLevel(log4cxx::Level::toLevel(str));
  } catch (log4cxx::helpers::Exception &) {
    RMF_THROW(Message("Invalid log level"), UsageException);
  }
}
#else
void set_log_level(std::string str) {
}
#endif


}

RMF_DISABLE_WARNINGS
