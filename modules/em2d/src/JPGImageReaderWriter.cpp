/**
 *  \file JPGImageReaderWriter.cpp
 *  \brief Management of JPG format for EM images
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/JPGImageReaderWriter.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include <IMP/exception.h>
#include <IMP/log.h>
#include <boost/filesystem.hpp>
#include <csetjmp>
#include <cstdio>
#include <jpeglib.h>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

namespace {

/* libjpeg calls exit() on errors by default; instead, jump back to the
   caller (which must have called setjmp() on jump) with the message */
struct ErrorManager {
  jpeg_error_mgr pub;  // must be first
  std::jmp_buf jump;
  char message[JMSG_LENGTH_MAX];
};

void handle_jpeg_error(j_common_ptr cinfo) {
  ErrorManager *err = reinterpret_cast<ErrorManager *>(cinfo->err);
  (*cinfo->err->format_message)(cinfo, err->message);
  std::longjmp(err->jump, 1);
}

// Gray value of a CMYK pixel, as converted by OpenCV
unsigned char get_gray_from_cmyk(int c, int m, int y, int k) {
  c = k - ((255 - c) * k >> 8);
  m = k - ((255 - m) * k >> 8);
  y = k - ((255 - y) * k >> 8);
  return static_cast<unsigned char>((1868 * y + 9617 * m + 4899 * c + 8192) >>
                                    14);
}

/* Read a JPG file as an 8-bit grayscale image. Return an empty string on
   success, otherwise the error message. Color images are converted to
   grayscale by libjpeg; CMYK images (which libjpeg cannot convert) in the
   same way as OpenCV did. */
std::string read_jpeg_grayscale(std::FILE *f, cv::Mat &pixels) {
  jpeg_decompress_struct cinfo;
  ErrorManager err;
  cinfo.err = jpeg_std_error(&err.pub);
  err.pub.error_exit = handle_jpeg_error;
  std::vector<unsigned char> cmyk_row;
  if (setjmp(err.jump)) {
    jpeg_destroy_decompress(&cinfo);
    return err.message;
  }
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, f);
  jpeg_read_header(&cinfo, TRUE);
  bool cmyk = cinfo.num_components == 4;
  cinfo.out_color_space = cmyk ? JCS_CMYK : JCS_GRAYSCALE;
  jpeg_start_decompress(&cinfo);
  pixels.create(cinfo.output_height, cinfo.output_width, CV_8UC1);
  if (cmyk) cmyk_row.resize(4 * cinfo.output_width);
  while (cinfo.output_scanline < cinfo.output_height) {
    unsigned char *out = pixels.ptr<unsigned char>(cinfo.output_scanline);
    JSAMPROW row = cmyk ? cmyk_row.data() : out;
    jpeg_read_scanlines(&cinfo, &row, 1);
    if (cmyk) {
      for (unsigned int j = 0; j < cinfo.output_width; ++j) {
        const unsigned char *p = &cmyk_row[4 * j];
        out[j] = get_gray_from_cmyk(p[0], p[1], p[2], p[3]);
      }
    }
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return std::string();
}

/* Write an 8-bit grayscale image to a JPG file with quality 100. Return an
   empty string on success, otherwise the error message. */
std::string write_jpeg_grayscale(std::FILE *f, const cv::Mat &pixels) {
  jpeg_compress_struct cinfo;
  ErrorManager err;
  cinfo.err = jpeg_std_error(&err.pub);
  err.pub.error_exit = handle_jpeg_error;
  if (setjmp(err.jump)) {
    jpeg_destroy_compress(&cinfo);
    return err.message;
  }
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, f);
  cinfo.image_width = pixels.cols;
  cinfo.image_height = pixels.rows;
  cinfo.input_components = 1;
  cinfo.in_color_space = JCS_GRAYSCALE;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 100, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW row = const_cast<unsigned char *>(
        pixels.ptr<unsigned char>(cinfo.next_scanline));
    jpeg_write_scanlines(&cinfo, &row, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  return std::string();
}

}  // namespace

void JPGImageReaderWriter::read_from_ints(const String &filename,
                                          em::ImageHeader &header,
                                          cv::Mat &data) const {
  IMP_LOG_VERBOSE("reading with JPGImageReaderWriter" << std::endl);
  std::FILE *f = std::fopen(filename.c_str(), "rb");
  if (!f) {
    IMP_THROW("Error reading from JPG Image " << filename
                                              << ": cannot open file",
              IOException);
  }
  cv::Mat pixels;
  std::string error = read_jpeg_grayscale(f, pixels);
  std::fclose(f);
  if (!error.empty()) {
    IMP_THROW("Error reading from JPG Image " << filename << ": " << error,
              IOException);
  }
  internal::get_from_8bit(pixels, data);
  // fill the header with default values
  header.clear();
}

void JPGImageReaderWriter::write_to_ints(const String &filename,
                                         em::ImageHeader &header,
                                         const cv::Mat &data) const {
  IMP_UNUSED(header);
  // discard header
  IMP_LOG(IMP::WARNING,
          "Writing with JPGImageReaderWriter "
          "discards image header "
              << std::endl);
  // check extension
  String ext = boost::filesystem::path(filename).extension().string();
  IMP_LOG_VERBOSE("JPGImageReaderWriter writing to " << filename
                                                      << std::endl);
  if (ext != ".jpg" && ext != ".jpeg") {
    IMP_THROW(
        "JPGImageReaderWriter: The filename extension is not .jpg "
        "or .jpeg",
        IOException);
  }

  if (data.empty()) {
    IMP_THROW("JPGImageReaderWriter: Cannot write an empty image",
              IOException);
  }
  // Convert the data value to 8-bit so it can be written as JPG
  cv::Mat jpg_data;
  internal::get_8bit_scaled(data, jpg_data);
  std::FILE *f = std::fopen(filename.c_str(), "wb");
  if (!f) {
    IMP_THROW("Error writing JPG Image " << filename << ": cannot open file",
              IOException);
  }
  std::string error = write_jpeg_grayscale(f, jpg_data);
  bool closed = std::fclose(f) == 0;
  if (!error.empty() || !closed) {
    IMP_THROW("Error writing JPG Image " << filename << ": "
                                         << (closed ? error : "write failed"),
              IOException);
  }
}

IMPEM2D_END_NAMESPACE
