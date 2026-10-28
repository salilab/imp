/**
 *  \file TIFFImageReaderWriter.cpp
 *  \brief Management of reading/writing TIFF images
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/TIFFImageReaderWriter.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include <IMP/exception.h>
#include <IMP/log.h>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <tiffio.h>
#include <vector>

// Per-file error and warning handlers are available from libtiff 4.5
#ifdef TIFFLIB_AT_LEAST
#if TIFFLIB_AT_LEAST(4, 5, 0)
#define IMPEM2D_HAVE_TIFF_OPEN_EXT 1
#endif
#endif

IMPEM2D_BEGIN_NAMESPACE

namespace {

#ifdef IMPEM2D_HAVE_TIFF_OPEN_EXT
// Keep the first error message; returning 1 stops libtiff printing it
int handle_tiff_error(TIFF *, void *user_data, const char *module,
                 const char *fmt, va_list ap) {
  std::string *message = static_cast<std::string *>(user_data);
  if (message->empty()) {
    char buf[1024];
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    *message = std::string(module ? module : "libtiff") + ": " + buf;
  }
  return 1;
}

// Ignore warnings (e.g. about unknown tags), as OpenCV did
int handle_tiff_warning(TIFF *, void *, const char *, const char *, va_list) {
  return 1;
}
#endif

/* Open a TIFF file, storing any error message (where supported) in
   message */
TIFF *open_tiff_file(const String &filename, const char *mode,
                std::string &message) {
#ifdef IMPEM2D_HAVE_TIFF_OPEN_EXT
  TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
  TIFFOpenOptionsSetErrorHandlerExtR(opts, handle_tiff_error, &message);
  TIFFOpenOptionsSetWarningHandlerExtR(opts, handle_tiff_warning, nullptr);
  TIFF *tif = TIFFOpenExt(filename.c_str(), mode, opts);
  TIFFOpenOptionsFree(opts);
  return tif;
#else
  IMP_UNUSED(message);
  return TIFFOpen(filename.c_str(), mode);
#endif
}

/* Read the first image in a TIFF file as an 8-bit grayscale image, using
   libtiff's conversion to RGBA (which handles the many TIFF variants) and
   then converting to gray in the same way as OpenCV. Return false on
   failure, with the error message (if any) in message. */
bool read_tiff_grayscale(TIFF *tif, cv::Mat &pixels, std::string &message) {
  uint32_t width = 0, height = 0;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  char emsg[1024] = "";
  if (!TIFFRGBAImageOK(tif, emsg)) {
    message = emsg;
    return false;
  }
  std::vector<uint32_t> rgba(static_cast<size_t>(width) * height);
  if (!TIFFReadRGBAImageOriented(tif, width, height, rgba.data(),
                                 ORIENTATION_TOPLEFT, 0)) {
    return false;
  }
  pixels.create(height, width, CV_8UC1);
  for (uint32_t i = 0; i < height; ++i) {
    unsigned char *out = pixels.ptr<unsigned char>(i);
    for (uint32_t j = 0; j < width; ++j) {
      uint32_t p = rgba[static_cast<size_t>(i) * width + j];
      out[j] = static_cast<unsigned char>(
          (TIFFGetB(p) * 1868 + TIFFGetG(p) * 9617 + TIFFGetR(p) * 4899 +
           8192) >> 14);
    }
  }
  return true;
}

/* Write an 8-bit grayscale image with the same settings OpenCV used (LZW
   compression with horizontal prediction, strips of about 8KB) */
bool write_tiff_grayscale(TIFF *tif, const cv::Mat &pixels) {
  int rows_per_strip =
      std::max(1, std::min(pixels.rows, (1 << 13) / pixels.cols));
  if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, pixels.cols) ||
      !TIFFSetField(tif, TIFFTAG_IMAGELENGTH, pixels.rows) ||
      !TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8) ||
      !TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1) ||
      !TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW) ||
      !TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK) ||
      !TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||
      !TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip) ||
      !TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT) ||
      !TIFFSetField(tif, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL)) {
    return false;
  }
  // libtiff may modify the row when applying the predictor, so copy it
  std::vector<unsigned char> row(pixels.cols);
  for (int i = 0; i < pixels.rows; ++i) {
    const unsigned char *p = pixels.ptr<unsigned char>(i);
    std::copy(p, p + pixels.cols, row.begin());
    if (TIFFWriteScanline(tif, row.data(), i, 0) < 0) return false;
  }
  return true;
}

}  // namespace

void TIFFImageReaderWriter::read_from_ints(const String &filename,
                                           em::ImageHeader &header,
                                           cv::Mat &data) const {
  IMP_LOG_VERBOSE("reading with TIFFImageReaderWriter" << std::endl);
  std::string message;
  TIFF *tif = open_tiff_file(filename, "r", message);
  cv::Mat pixels;
  bool ok = tif && read_tiff_grayscale(tif, pixels, message);
  if (tif) TIFFClose(tif);
  if (!ok) {
    IMP_THROW("Error reading from TIFF Image "
                  << filename << (message.empty() ? "" : ": ") << message,
              IOException);
  }
  internal::get_from_8bit(pixels, data);
  // fill the header with default values
  header.clear();
}

void TIFFImageReaderWriter::write_to_ints(const String &filename,
                                          em::ImageHeader &header,
                                          const cv::Mat &data) const {
  IMP_UNUSED(header);
  // discard header
  IMP_LOG(IMP::WARNING,
          "Writing with TIFFImageReaderWriter "
          "discards image header "
              << std::endl);
  // check extension
  String ext = boost::filesystem::path(filename).extension().string();
  IMP_LOG_VERBOSE("TIFFImageReaderWriter writing to " << filename
                                                       << std::endl);
  if (ext != ".tiff" && ext != ".tif") {
    IMP_THROW(
        "TIFFImageReaderWriter: The filename extension is not .tiff "
        "or .tif",
        IOException);
  }

  if (data.empty()) {
    IMP_THROW("TIFFImageReaderWriter: Cannot write an empty image",
              IOException);
  }
  // Convert the data value to 8-bit so it can be written as TIFF
  cv::Mat TIFF_data;
  internal::get_8bit_scaled(data, TIFF_data);
  std::string message;
  TIFF *tif = open_tiff_file(filename, "w", message);
  bool ok = tif && write_tiff_grayscale(tif, TIFF_data);
  // Closing flushes the last strip
  if (tif) TIFFClose(tif);
  if (!ok || !message.empty()) {
    IMP_THROW("Error writing TIFF Image "
                  << filename << (message.empty() ? "" : ": ") << message,
              IOException);
  }
}

IMPEM2D_END_NAMESPACE
