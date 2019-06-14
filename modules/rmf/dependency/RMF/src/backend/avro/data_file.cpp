/**
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include "DataFile.hh"
#include "Specific.hh"
#include "avrocpp/api/DataFile.hh"
#include "avrocpp/api/Specific.hh"
#include "encode_decode.h"  // IWYU pragma: keep
#include "data_file.h"

namespace RMF {
namespace avro2 {
struct FileDataChanges;
struct Frame;
}  // namespace avro2
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro2 {
void write(internal_avro::DataFileWriterBase *writer, const Frame &fr) {
  writer->syncIfNeeded();
  internal_avro::encode(writer->encoder(), fr);
  writer->incr();
}
void write(internal_avro::DataFileWriterBase *writer,
           const FileDataChanges &fr) {
  writer->syncIfNeeded();
  internal_avro::encode(writer->encoder(), fr);
  writer->incr();
}

void load_frame(FrameID id, internal_avro::DataFileReader<Frame> &reader,
                Frame &frame) {
  RMF_TRACE("Looking for frame " << id);

  do {
    frame = Frame();
    reader.read(frame);
    if (frame.id == id) {
      RMF_INFO("Loaded frame " << frame.id << " " << id << std::endl);
      return;
    } else {
      RMF_INFO("Found frame for frame " << frame.id);
    }
  } while (true);
}

void load_file_data(internal_avro::DataFileReader<FileData> &reader,
                    FileData &fd) {
  // clear it
  clear(fd);
  try {
    while (reader.read(fd)) {
      if (fd.cur_id != FrameID()) {
        fd.frame_block_offsets[fd.cur_id] = reader.blockOffsetBytes();
      }
    }
  }
  catch (const std::exception &e) {
    // ick, ick, ewe
    if (std::string(e.what()) != "EOF reached") {
      RMF_THROW(Message(e.what()) << Operation("loading file data"),
                IOException);
    }
  }
}
}
}
RMF_DISABLE_WARNINGS
