
#ifndef RMF_AVRO2_WRITE_H
#define RMF_AVRO2_WRITE_H

#include <avrocpp/api/DataFile.hh>

#include "RMF/config.h"
#include "types.h"

namespace RMF {
namespace avro2 {
struct FileDataChanges;
struct Frame;
}  // namespace avro2
}  // namespace RMF
namespace internal_avro {
class DataFileWriterBase;
template <typename T>
class DataFileReader;
}  // namespace internal_avro

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro2 {
RMFEXPORT void write(internal_avro::DataFileWriterBase *writer,
                     const Frame &fr);
RMFEXPORT void write(internal_avro::DataFileWriterBase *writer,
                     const FileDataChanges &fr);
// workarounds for the Mac os 10.8 compiler make returning it inefficient
RMFEXPORT void load_file_data(internal_avro::DataFileReader<FileData> &reader,
                              FileData &fd);
RMFEXPORT void load_frame(FrameID id,
                          internal_avro::DataFileReader<Frame> &reader,
                          Frame &frame);
}
}

RMF_DISABLE_WARNINGS
#endif  //  RMF_AVRO2_WRITE_H
