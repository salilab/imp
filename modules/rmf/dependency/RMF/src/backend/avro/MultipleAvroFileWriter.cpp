/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include "MultipleAvroFileWriter.h"
#include <RMF/internal/paths.h>
#include <RMF/decorators.h>
#include <avro/Compiler.hh>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdexcept>

namespace RMF {
  namespace internal {

    void MultipleAvroFileWriter::set_current_frame(int frame) {
      if (frame == get_current_frame()) return;
      if (frame == get_file().number_of_frames) {
        commit();
        for (unsigned int i=0; i< categories_.size(); ++i) {
          categories_[i].data=RMF_internal::Data();
          categories_[i].data.frame=frame;
        }
      } else {
        RMF_USAGE_CHECK(frame == ALL_FRAMES
                        || frame == get_file().number_of_frames,
                        "Bad frame set");
      }
      MultipleAvroFileBase::set_current_frame(frame);
    }

    MultipleAvroFileWriter::MultipleAvroFileWriter(std::string path,
                                                   bool create, bool read_only):
      MultipleAvroFileBase(path){
      RMF_INTERNAL_CHECK(create, "Can only create files");
      RMF_INTERNAL_CHECK(!read_only, "Can only create files");
      boost::filesystem::remove_all(path);
      boost::filesystem::create_directory(path);
      file_.number_of_frames=0;
      file_.version=1;
      file_dirty_=true;
      frames_dirty_=true;
      nodes_dirty_=true;
    }

    MultipleAvroFileWriter::~MultipleAvroFileWriter() {
      set_current_frame(get_file().number_of_frames);
    }

#define RMF_COMMIT(UCName, lcname)                                       \
    if (lcname##_dirty_) {                                              \
      avro::DataFileWriter<UCName>                                      \
        wr(get_##lcname##_file_path().c_str(), get_##UCName##_schema()); \
      wr.write(lcname##_);                                              \
      wr.flush();                                                       \
    }

    void MultipleAvroFileWriter::commit() {
      RMF_COMMIT(Nodes, nodes);
      RMF_COMMIT(Frames, frames);
      for (unsigned int i=0; i< categories_.size(); ++i) {
        if (categories_[i].dirty) {
          if (!categories_[i].writer) {
            std::string name= get_category_dynamic_file_path(Category(i));
            categories_[i].writer
              .reset(new avro::DataFileWriter<RMF_internal::Data>(name.c_str(),
                                                                  get_Data_schema()));
          }
          /*std::cout << "Writing data for " << get_category_name(Category(i))
            << " at frame " << categories_[i].data.frame << std::endl;*/
          //show(categories_[i].data);
          categories_[i].writer->write(categories_[i].data);
          categories_[i].writer->flush();
        }
      }
      for (unsigned int i=0; i< static_categories_.size(); ++i) {
        if (static_categories_dirty_[i]) {
          std::string name= get_category_static_file_path(Category(i));
          avro::DataFileWriter<RMF_internal::Data> writer(name.c_str(),
                                                          get_Data_schema());
          writer.write(static_categories_[i]);
          writer.flush();
        }
      }
      // must be last
      //std::cout << "Committed num frames= " << file_.number_of_frames << std::endl;
      RMF_COMMIT(File, file);
    }
  } // namespace internal
} /* namespace RMF */
