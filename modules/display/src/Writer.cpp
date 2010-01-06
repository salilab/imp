/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/Writer.h"
#include <boost/algorithm/string/predicate.hpp>


IMPDISPLAY_BEGIN_NAMESPACE

Writer::Writer(std::string name): Object(name){
  file_name_=name;
}

Writer::~Writer(){
}


void Writer::add_geometry(Geometry *g) {
  process_geometry(g);
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
            UsageException);
}

IMPDISPLAY_END_NAMESPACE
