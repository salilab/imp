/**
 *  \file writers.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_INTERNAL_WRITERS_H
#define IMPDISPLAY_INTERNAL_WRITERS_H

#include <IMP/display/display_config.h>
#include <IMP/base/file.h>
IMPDISPLAY_BEGIN_NAMESPACE
class Writer;
IMPDISPLAY_END_NAMESPACE

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE

class WriterFactory {
 public:
  WriterFactory();
  virtual ~WriterFactory();
  virtual Writer *create(std::string name) const = 0;
};

template <class Wr>
class WriterFactoryHelper : public WriterFactory {
 public:
  WriterFactoryHelper() {}
  Writer *create(std::string name) const {
    return new Wr(base::TextOutput(name));
  }
};

std::map<std::string, WriterFactory *> &get_writer_factory_table();

struct WriterFactoryRegistrar {
  WriterFactoryRegistrar(std::string suffix, WriterFactory *wf) {
    get_writer_factory_table()[suffix] = wf;
  }
};

IMPDISPLAY_END_INTERNAL_NAMESPACE

#endif /* IMPDISPLAY_INTERNAL_WRITERS_H */
