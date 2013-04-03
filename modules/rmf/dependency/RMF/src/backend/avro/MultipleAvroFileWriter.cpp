/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include "MultipleAvroFileWriter.h"
#include <RMF/internal/paths.h>
#include <RMF/decorators.h>
#include <RMF/log.h>
#include <boost/lexical_cast.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

void MultipleAvroFileWriter::set_current_frame(int frame) {
  if (frame == get_current_frame()) return;
  RMF_USAGE_CHECK(frame == ALL_FRAMES
                  || frame == frame_.index+1
                  || frame == frame_.index,
                  "Bad frame set. You probably didn't add a new frame.");
  MultipleAvroFileBase::set_current_frame(frame);
  if (frame != ALL_FRAMES && frame != frame_.index) {
    commit();
  }
}

MultipleAvroFileWriter::MultipleAvroFileWriter(std::string path,
                                               bool create, bool read_only):
  MultipleAvroFileBase(path) {
  RMF_INTERNAL_CHECK(create,     "Can only create files");
  RMF_INTERNAL_CHECK(!read_only, "Can only create files");
  boost::filesystem::remove_all(path);
  boost::filesystem::create_directory(path);
  frame_.index=-1;
  frame_.name="static";
  frame_.type="static";
  file_.version = 2;
  file_dirty_ = true;
  frames_dirty_ = true;
  nodes_dirty_ = true;
}

MultipleAvroFileWriter::~MultipleAvroFileWriter() {
  commit();
}


#define RMF_COMMIT(UCName, lcname)                                         \
  if (lcname##_dirty_) {                                                   \
    write(lcname##_, get_##UCName##_schema(), get_##lcname##_file_path()); \
  }

void MultipleAvroFileWriter::commit() {
  RMF_TRACE(get_avro_logger(), "Writing frame " << frame_.index);
  for (unsigned int i = 0; i < categories_.size(); ++i) {
    if (categories_[i].dirty) {
      if (!categories_[i].writer) {
        std::string name = get_category_dynamic_file_path(Category(i));
        try {
          categories_[i].writer
          .reset(new rmf_avro::DataFileWriter<RMF_avro_backend::Data>(name.c_str(),
                                                              get_Data_schema()));
        } catch (const std::exception &e) {
          RMF_THROW(Message(e.what()) << Component(name),
                    IOException);
        }
      }
      //show(categories_[i].data);
      RMF_INTERNAL_CHECK(categories_[i].data.frame == frame_.index,
                         "Trying to write category that is at wrong frame.");
      categories_[i].writer->write(categories_[i].data);
      categories_[i].writer->flush();
    }
    categories_[i].data = RMF_avro_backend::Data();
    // go to the about to be added frame
    categories_[i].data.frame = frame_.index+1;
  }
  for (unsigned int i = 0; i < static_categories_.size(); ++i) {
    if (static_categories_dirty_[i]) {
      std::string name = get_category_static_file_path(Category(i));
      try {
        rmf_avro::DataFileWriter<RMF_avro_backend::Data> writer(name.c_str(),
                                                        get_Data_schema());
        writer.write(static_categories_[i]);
        writer.flush();
      } catch (const std::exception &e) {
        RMF_THROW(Message(e.what()) << Component(name),
                  IOException);
      }
      //std::cout << "Writing data for " << get_category_name(Category(i)) << std::endl;
      //show(static_categories_[i]);
      static_categories_dirty_[i] = false;
    }
  }
  RMF_COMMIT(File,  file);
  RMF_COMMIT(Nodes, nodes);
  if (frames_dirty_) {
    if (!frame_writer_) {
      frame_writer_.reset(new rmf_avro::DataFileWriter<RMF_avro_backend::Frame>(get_frames_file_path().c_str(),
                                                                           get_Frame_schema()));
    }
    frame_writer_->write(frame_);
    frames_dirty_=false;
  }
}

  int MultipleAvroFileWriter::add_child_frame(int node, std::string name, int t) {
    unsigned int index = get_number_of_frames();
    RMF_TRACE(get_avro_logger(), "Adding frame " << index << " under " << node);
    set_current_frame(index);
    frame_.name=name;
    frame_.type=boost::lexical_cast<std::string>(FrameType(t));
    frame_.parents.push_back(node);
    frames_dirty_=true;
    frame_.index=index;
    return index;
  }
  void MultipleAvroFileWriter::add_child_frame(int node, int child_node) {
    if (child_node != get_current_frame()) {
      RMF_THROW(Message("RMF2 writer doesn't support adding other frames than the current as a child."),
              UsageException);
    }
    frame_.parents.push_back(node);
    frames_dirty_=true;
  }
  Ints MultipleAvroFileWriter::get_children_frame(int /*node*/) const {
    RMF_THROW(Message("RMF2 writer doesn't support getting frame children."),
              UsageException);
  }

  std::string MultipleAvroFileWriter::get_frame_name(int i) const {
    if (i==ALL_FRAMES) {
      return "static";
    } else {
      RMF_USAGE_CHECK(i==frame_.index,
                      "Can only query the name of the current frame with"\
                      " writing RMF2 files.");
      return frame_.name;
    }
  }
  unsigned int MultipleAvroFileWriter::get_number_of_frames() const {
    return frame_.index+1;
  }

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
