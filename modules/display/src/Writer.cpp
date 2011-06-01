/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/Writer.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

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




TextWriter::TextWriter(TextOutput fn): Writer(fn.get_name()), out_(fn){
  set_was_used(true);
}
TextWriter::TextWriter(std::string name): Writer(name), file_name_(name),
                                          out_(name){
  set_was_used(true);
}

TextWriter::~TextWriter(){
}

void TextWriter::do_set_frame() {
  if (file_name_.empty() || file_name_.find("%1%")== std::string::npos) {
    IMP_FAILURE("Cannot set frame on writer without file name pattern.");
  }
  std::ostringstream oss;
  oss << boost::format(file_name_)%get_frame();
  out_= TextOutput(oss.str());
}



Writer *create_writer(std::string name, bool append) {
  for (std::map<std::string, internal::WriterFactory *>::iterator
         it= internal::get_writer_factory_table().begin();
       it != internal::get_writer_factory_table().end(); ++it) {
    if (boost::algorithm::ends_with(name, it->first)) {
      return it->second->create(name, append);
    }
  }
  IMP_THROW("No writer found for file " << name,
            ValueException);
}

IMPDISPLAY_END_NAMESPACE
