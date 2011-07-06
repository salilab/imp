/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/Writer.h"
#include <boost/algorithm/string/predicate.hpp>

IMPDISPLAY_BEGIN_NAMESPACE

Writer::Writer(std::string name): Object(name), frame_(-1){
}

Writer::~Writer(){
}


void Writer::set_frame(unsigned int i) {
  IMP_USAGE_CHECK(static_cast<int>(i)==frame_+1,
                  "Frame can only be changed by one.");
  frame_=i;
  do_set_frame();
}

void Writer::add_geometry(Geometry *g) {
  handle_geometry(g);
}




TextWriter::TextWriter(TextOutput fn): Writer(fn.get_name()), out_(fn) {
  set_was_used(true);
}
TextWriter::TextWriter(std::string name): Writer(name), file_name_(name) {
  set_was_used(true);
}

void TextWriter::open() {
  /*IMP_INTERNAL_CHECK(!(file_name_.find("%1%") != std::string::npos
                       && get_frame() == -1),
                       "Cant open file without a frame.");*/
  out_= TextOutput();
  out_=TextOutput(get_current_file_name());
  do_open();
}

TextWriter::~TextWriter(){
}

void TextWriter::do_set_frame() {
  if (file_name_.empty()/* || file_name_.find("%1%")== std::string::npos*/) {
    IMP_FAILURE("Cannot set frame on writer without file name pattern.");
  }
  if (out_!= TextOutput()) {
    do_close();
    out_=TextOutput();
  }
  open();
}



Writer *create_writer(std::string name) {
  for (std::map<std::string, internal::WriterFactory *>::iterator
         it= internal::get_writer_factory_table().begin();
       it != internal::get_writer_factory_table().end(); ++it) {
    if (boost::algorithm::ends_with(name, it->first)) {
      return it->second->create(name);
    }
  }
  IMP_THROW("No writer found for file " << name,
            ValueException);
}

WriterOutput::~WriterOutput(){}

IMPDISPLAY_END_NAMESPACE
