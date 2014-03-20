/**
 *  \file writers.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/display/Writer.h>
#include <IMP/display/internal/writers.h>

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE
std::map<std::string, boost::shared_ptr<WriterFactory> >&
get_writer_factory_table() {
  static std::map<std::string, boost::shared_ptr<WriterFactory> > table;
  return table;
}

WriterFactory::WriterFactory() {}
WriterFactory::~WriterFactory() {}
IMPDISPLAY_END_INTERNAL_NAMESPACE
