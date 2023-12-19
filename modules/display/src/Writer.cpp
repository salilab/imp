/**
 *  \file Writer.cpp
 *  \brief Base class for writing geometry to a file.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/Writer.h"
#include <boost/algorithm/string/predicate.hpp>
#include <IMP/display/writer_macros.h>
#include <IMP/display/PymolWriter.h>

IMPDISPLAY_BEGIN_NAMESPACE

Writer::Writer(std::string name) : Object(name), frame_(-1) {}

Writer::~Writer() {}

void Writer::set_frame(unsigned int i) {
  IMP_USAGE_CHECK(static_cast<int>(i) == frame_ + 1,
                  "Frame can only be changed by one.");
  frame_ = i;
  do_set_frame();
}

void Writer::add_geometry(Geometry *g) { handle_geometry(g); }

TextWriter::TextWriter(TextOutput fn) : Writer(fn.get_name()), out_(fn) {
  set_was_used(true);
}
TextWriter::TextWriter(std::string name) : Writer(name), file_name_(name) {
  set_was_used(true);
}

void TextWriter::open() {
  /*IMP_INTERNAL_CHECK(!(file_name_.find("%1%") != std::string::npos
                       && get_frame() == -1),
                       "Can't open file without a frame.");*/
  out_ = TextOutput();
  out_ = TextOutput(get_current_file_name());
  do_open();
}

TextWriter::~TextWriter() {}

void TextWriter::do_set_frame() {
  if (file_name_.empty() /* || file_name_.find("%1%")== std::string::npos*/) {
    IMP_FAILURE("Cannot set frame on writer without %1% being in the name.");
  }
  if (out_ != TextOutput()) {
    do_close();
    out_ = TextOutput();
  }
  open();
}

Writer *create_writer(std::string name) {
  typedef std::pair<std::string, std::shared_ptr<internal::WriterFactory> >
      MP;
  for(MP mp : internal::get_writer_factory_table()) {
    if (boost::algorithm::ends_with(name, mp.first)) {
      return mp.second->create(name);
    }
  }
  IMP_THROW("No writer found for file " << name, ValueException);
}

WriterAdaptor::~WriterAdaptor() {}

IMP_REGISTER_WRITER(PymolWriter, ".pym")

IMPDISPLAY_END_NAMESPACE
