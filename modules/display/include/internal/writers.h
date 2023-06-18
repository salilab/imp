/**
 *  \file writers.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_INTERNAL_WRITERS_H
#define IMPDISPLAY_INTERNAL_WRITERS_H

#include <IMP/display/display_config.h>
#include <IMP/file.h>
#include <memory>
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
  Writer *create(std::string name) const override {
    return new Wr(TextOutput(name));
  }
};

std::map<std::string, std::shared_ptr<WriterFactory> > &
    get_writer_factory_table();

struct WriterFactoryRegistrar {
  WriterFactoryRegistrar(std::string suffix,
                         std::shared_ptr<WriterFactory> wf) {
    get_writer_factory_table()[suffix] = wf;
  }
};

IMPDISPLAY_END_INTERNAL_NAMESPACE

#endif /* IMPDISPLAY_INTERNAL_WRITERS_H */
