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
#include <cstdio>
#include <stdexcept>

namespace RMF {
  namespace internal {

    void MultipleAvroFileWriter::set_current_frame(int frame) {
      if (frame == get_current_frame()) return;
      RMF_USAGE_CHECK(frame == ALL_FRAMES
                      || frame == get_frames().size()-2,
                      "Bad frame set");
      MultipleAvroFileBase::set_current_frame(frame);
    }

    MultipleAvroFileWriter::MultipleAvroFileWriter(std::string path,
                                                   bool create, bool read_only):
      MultipleAvroFileBase(path){
      RMF_INTERNAL_CHECK(create, "Can only create files");
      RMF_INTERNAL_CHECK(!read_only, "Can only create files");
      boost::filesystem::remove_all(path);
      boost::filesystem::create_directory(path);
      file_.version=1;
      file_dirty_=true;
      frames_dirty_=true;
      nodes_dirty_=true;
    }

    MultipleAvroFileWriter::~MultipleAvroFileWriter() {
      commit();
    }

#if BOOST_VERSION < 104400
// boost::rename is broken
#define RMF_COMMIT(UCName, lcname)                                      \
    if (lcname##_dirty_) {                                              \
      std::string path=get_##lcname##_file_path().c_str();              \
      std::string temppath=path+".new";                                 \
      avro::DataFileWriter<UCName>                                      \
        wr(temppath.c_str(), get_##UCName##_schema());                  \
      wr.write(lcname##_);                                              \
      wr.flush();                                                       \
      std::rename(temppath.c_str(), path.c_str());                      \
    }
#else
#define RMF_COMMIT(UCName, lcname)                                      \
    if (lcname##_dirty_) {                                              \
      std::string path=get_##lcname##_file_path().c_str();              \
      std::string temppath=path+".new";                                 \
      avro::DataFileWriter<UCName>                                      \
        wr(temppath.c_str(), get_##UCName##_schema());                  \
      wr.write(lcname##_);                                              \
      wr.flush();                                                       \
      boost::filesystem::rename(temppath, path);                        \
    }
#endif
    void MultipleAvroFileWriter::commit() {
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
          RMF_INTERNAL_CHECK(categories_[i].data.frame==get_frames().size()-2,
                             "Trying to write category that is at wrong frame.");
          categories_[i].writer->write(categories_[i].data);
          categories_[i].writer->flush();
        }
        categories_[i].data=RMF_internal::Data();
        // go to the about to be added frame
        categories_[i].data.frame=get_frames().size()-1;
      }
      for (unsigned int i=0; i< static_categories_.size(); ++i) {
        if (static_categories_dirty_[i]) {
          std::string name= get_category_static_file_path(Category(i));
          avro::DataFileWriter<RMF_internal::Data> writer(name.c_str(),
                                                          get_Data_schema());
          writer.write(static_categories_[i]);
          writer.flush();
          //std::cout << "Writing data for " << get_category_name(Category(i)) << std::endl;
          //show(static_categories_[i]);
          static_categories_dirty_[i]=false;
        }
      }
      RMF_COMMIT(File, file);
      RMF_COMMIT(Nodes, nodes);
      RMF_COMMIT(Nodes, frames);
    }
  } // namespace internal
} /* namespace RMF */
