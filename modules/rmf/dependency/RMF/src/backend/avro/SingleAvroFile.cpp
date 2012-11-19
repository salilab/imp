/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include "SingleAvroFile.h"
#include <RMF/internal/paths.h>
#include <RMF/decorators.h>
#include <avro/Compiler.hh>
#include <stdexcept>

namespace RMF {
  namespace internal {

    SingleAvroFile::SingleAvroFile(std::string path, bool create,
                                   bool read_only): AvroKeysAndCategories(path){
      if (!create) {
        reload();
      } else {
        initialize_categories();
        initialize_node_keys();
        access_file().number_of_frames=0;
      }
      null_static_frame_data_.frame=ALL_FRAMES;
    }

    void SingleAvroFile::initialize_categories() {
      for (std::map<std::string, std::vector<RMF_internal::Data > >::const_iterator
             it= all_.category.begin(); it != all_.category.end(); ++it) {
        get_category(it->first);
      }
    }

    void SingleAvroFile::initialize_node_keys() {
      clear_node_keys();
      for (unsigned int i=0; i< all_.nodes.size(); ++i) {
        add_node_key();
      }
    }

    void SingleAvroFile::flush() {
      if (!dirty_) return;
      avro::DataFileWriter<RMF_internal::All>
        rd(get_file_path().c_str(), get_All_schema());
      rd.write(all_);
      dirty_=false;
    }

    void SingleAvroFile::reload() {
      avro::DataFileReader<RMF_internal::All>
        rd(get_file_path().c_str(), get_All_schema());
      bool ok=rd.read(all_);
      if (!ok) {
        throw IOException("Can't read input file on reload");
      }

      initialize_categories();
      initialize_node_keys();
      dirty_=false;
    }

  } // namespace internal
} /* namespace RMF */
