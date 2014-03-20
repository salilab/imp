/**
 *  \file ImageHeader.cpp
 *  \brief Header for EM images
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
**/

#include "IMP/em/ImageHeader.h"
#include <IMP/base/exception.h>
#include <ctime>
#include <string.h>

IMPEM_BEGIN_NAMESPACE

void ImageHeader::print_hard(std::ostream& out) const {
  out << "fNslice=" << spider_header_.fNslice << std::endl;
  out << "fNrow=" << spider_header_.fNrow << std::endl;
  out << "fNcol=" << spider_header_.fNcol << std::endl;
  out << "fNlabel=" << spider_header_.fNlabel << std::endl;
  out << "fNrec=" << spider_header_.fNrec << std::endl;
  out << "fIform=" << spider_header_.fIform << std::endl;
  out << "fImami=" << spider_header_.fImami << std::endl;
  out << "fFmax=" << spider_header_.fFmax << std::endl;
  out << "fFmin=" << spider_header_.fFmin << std::endl;
  out << "fAv=" << spider_header_.fAv << std::endl;
  out << "fSig=" << spider_header_.fSig << std::endl;
  out << "fIhist=" << spider_header_.fIhist << std::endl;
  out << "fLabrec=" << spider_header_.fLabrec << std::endl;
  out << "fIangle=" << spider_header_.fIangle << std::endl;
  out << "fPhi=" << spider_header_.fPhi << std::endl;
  out << "fTheta=" << spider_header_.fTheta << std::endl;
  out << "fPsi=" << spider_header_.fPsi << std::endl;
  out << "fXoff=" << spider_header_.fXoff << std::endl;
  out << "fYoff=" << spider_header_.fYoff << std::endl;
  out << "fZoff=" << spider_header_.fZoff << std::endl;
  out << "fScale=" << spider_header_.fScale << std::endl;
  out << "fLabbyt=" << spider_header_.fLabbyt << std::endl;
  out << "fLenbyt=" << spider_header_.fLenbyt << std::endl;
}

bool ImageHeader::read(std::ifstream& f, bool skip_type_check,
                       bool force_reversed, bool skip_extra_checkings) {
  float tmp;
  unsigned tmpSize;
  unsigned long size;

/* Determine reverse status according to the next table .................
     (computed in Linux) */
#define TYPE_TABLE_SIZE 10

  int type_table[TYPE_TABLE_SIZE][5] = {{0, 0, 48, 65, 11},
                                        {0, 0, 32, 65, 10},
                                        {0, 0, 16, 65, 9},
                                        {0, 0, 0, 65, 8},
                                        {0, 0, 64, 64, 3},
                                        {0, 0, 128, 63, 1},
                                        {0, 0, 128, 191, -1},
                                        {0, 0, 64, 192, -3},
                                        {0, 0, 160, 192, -5},
                                        {0, 0, 224, 192, -7}};
  union {
    unsigned char c[4];
    float f;
  } file_type;

  unsigned long current_position = f.tellg();

  if (!skip_type_check) {
    // Read file type
    f.seekg(current_position + 16, std::ios::beg);
    for (int i = 0; i < 4; i++) {
      f.read(reinterpret_cast<char*>(&(file_type.c[i])), sizeof(char));
    }
    f.seekg(current_position + 0, std::ios::beg);
    // Select type
    int correct_type = 0;
    reversed_ = false;

#define IS_TYPE(n)                            \
  (type_table[n][0] == (int)file_type.c[0] && \
   type_table[n][1] == (int)file_type.c[1] && \
   type_table[n][2] == (int)file_type.c[2] && \
   type_table[n][3] == (int)file_type.c[3])
#define IS_REVERSE_TYPE(n)                                                     \
  (type_table[n][0] == file_type.c[3] && type_table[n][1] == file_type.c[2] && \
   type_table[n][2] == file_type.c[1] && type_table[n][3] == file_type.c[0])

    for (int i = 0; i < TYPE_TABLE_SIZE; i++) {
      if (IS_TYPE(i)) {
        correct_type = type_table[i][4];
        break;
      } else if (IS_REVERSE_TYPE(i)) {
        correct_type = type_table[i][4];
        reversed_ = true;
        break;
      }
    }
    if (correct_type == 0) {
      return false;
    }
    // Now check this machine type
    file_type.f = 1;
    if (IS_REVERSE_TYPE(5)) {
      reversed_ = !reversed_;
    }
  } else {
    reversed_ = force_reversed;
  }

  // Read header
  if (!reversed_) {
    f.read(reinterpret_cast<char*>(&spider_header_), sizeof(SpiderHeader));
  }
      // Read numerical fields reversed
      else {
    IMP::algebra::reversed_read(&spider_header_, sizeof(float), 36, f, true);
    IMP::algebra::reversed_read(&spider_header_.fGeo_matrix, sizeof(double), 9,
                                f, true);
    // 14 is the number of fields in the SpiderHeader struct after fGeo_matrix
    IMP::algebra::reversed_read(&spider_header_.fAngle1, sizeof(float), 14, f,
                                true);
    IMP::algebra::reversed_read(&spider_header_.empty, sizeof(char), 752, f,
                                true);
  }

  unsigned long usfNcol = (unsigned long)spider_header_.fNcol;
  unsigned long usfNrow = (unsigned long)spider_header_.fNrow;
  if (usfNcol == 0 || usfNrow == 0) {
    IMP_THROW("Zero size read for image", IOException);
  }
  unsigned long usfNslice = (unsigned long)spider_header_.fNslice;
  unsigned long usfHeader = (unsigned long)get_spider_header_size();

  // Get file size
  current_position = f.tellg();
  unsigned long init_pos, file_size;
  f.seekg(0, std::ios::beg);
  init_pos = f.tellg();
  f.seekg(0, std::ios::end);
  file_size = (unsigned long)f.tellg() - init_pos;

  f.seekg(current_position, std::ios::beg);

  // Check if it is an "aberrant" image
  if (spider_header_.fIform == IMG_IMPEM) {
    if ((usfNcol * usfNrow * sizeof(float)) == file_size) {
      usfNrow = (unsigned long)(--spider_header_.fNrow);
      --spider_header_.fNrec;
    }
  }

  // Extra checkings
  if (!skip_extra_checkings) {
    switch ((int)spider_header_.fIform) {
      case IMG_BYTE:
        size = usfNcol * usfNrow * sizeof(float);
        if ((size != file_size)) {
          return false;
        }
        break;
      case IMG_IMPEM:
        size = usfHeader + usfNcol * usfNrow * sizeof(float);
        if ((size != file_size) || (spider_header_.fIform != 1)) {
          return false;
        } else if (skip_type_check) {
          spider_header_.fIform = 1;
        }

        break;
      case IMG_INT:
        size = usfHeader + usfNcol * usfNrow * sizeof(float);
        if ((size != file_size) || (spider_header_.fIform != 9)) {
          return false;
        } else if (skip_type_check) {
          spider_header_.fIform = 9;  // This is done to recover
          // files which are not properly converted from other packages
        }
        break;
      case VOL_BYTE:
        size = usfNslice * usfNcol * usfNrow * sizeof(float);
        if ((size != file_size)) {
          return false;
        }
        break;
      case VOL_IMPEM:
        size = usfHeader + usfNslice * usfNcol * usfNrow * sizeof(float);
        if ((size != file_size) || (spider_header_.fIform != 3)) {
          return false;
        } else if (skip_type_check) {
          spider_header_.fIform = 3;
        }
        break;
      case VOL_INT:
        size = usfHeader + usfNslice * usfNcol * usfNrow * sizeof(float);
        if ((size != file_size) || (spider_header_.fIform != 10)) {
          return false;
        } else if (skip_type_check) {
          spider_header_.fIform = 10;
        }
        break;
      case IMG_FOURIER:
        size = usfHeader + 2 * usfNcol * usfNrow * sizeof(float);
        // The term 2 is to take into account that IMG_FOURIER
        // stores complex numbers with 2 floats for each one.
        if ((size != file_size) ||
            (spider_header_.fIform != -5 && spider_header_.fIform != -1))
          return false;
        else if (skip_type_check) {
          spider_header_.fIform = -1;
        }
        break;
      case VOL_FOURIER:
        size = usfHeader + 2 * usfNcol * usfNrow * sizeof(float);
        // The term 2 is to take into account that VOL_FOURIER
        // stores complex numbers with 2 floats for each one.
        if ((size != file_size) ||
            (spider_header_.fIform != -7 && spider_header_.fIform != -3)) {
          return false;
        } else if (skip_type_check) {
          spider_header_.fIform = -3;
        }
        break;
    }
  }

  /* Spider images contain:
    - a header with the size of the struct SpiderHeader,
    - a "filling" empty space
    - the data of size cols*rows*sizeof(float).
  */
  spider_header_.fLabrec = (float)ceil((float)256 / spider_header_.fNcol);
  // Size of whole header
  tmpSize = (int)(spider_header_.fNcol * spider_header_.fLabrec * 4);
  // Subtrat the real header
  tmpSize -= sizeof(SpiderHeader);
  // read empty filling space
  for (unsigned i = 0; i < tmpSize / 4; i++) {
    IMP::algebra::reversed_read(&tmp, sizeof(float), 1, f, reversed_);
  }
  return true;
}

int ImageHeader::read(const String filename, bool skip_type_check,
                      bool force_reversed, bool skip_extra_checkings) {
  bool result;
  std::ifstream f;
  // Clear Header
  clear();
  // Read
  f.open(filename.c_str(), std::ios::in | std::ios::binary);
  if (f.fail()) {
    IMP_THROW("ImageHeader::read: file " + filename + " not found",
              IOException);
  }
  result = read(f, skip_type_check, force_reversed, skip_extra_checkings);
  f.close();
  return result;
}

void ImageHeader::write(std::ofstream& f, bool force_reversed) {
  float tmp;
  unsigned tmpSize;
  if (get_number_of_rows() == 0 || get_number_of_columns() == 0 ||
      get_number_of_slices() == 0) {
    return;
  }
  // Set consistent header before saving
  set_header();
  set_time();
  set_date();
  // Write header
  if (algebra::xorT(reversed_, force_reversed)) {
    reversed_ = true;
    IMP::algebra::reversed_write(&spider_header_, sizeof(float), 36, f, true);
    IMP::algebra::reversed_write(&spider_header_.fGeo_matrix, sizeof(double), 9,
                                 f, true);
    // 14 is the number of float fields after fGeo_matrix in the eader_ struct
    IMP::algebra::reversed_write(&spider_header_.fAngle1, sizeof(float), 14, f,
                                 true);
    // 752 is the number of chars to the end of the file
    f.write(spider_header_.empty, sizeof(char) * 752);
  } else {
    reversed_ = false;
    f.write(reinterpret_cast<char*>(&spider_header_), sizeof(SpiderHeader));
  }

  // Write empty filling space (filled with zeros)
  tmpSize = get_spider_header_size();  // Size of whole header
  tmpSize -= sizeof(SpiderHeader);     // Decrease the real header
  tmp = (float)0.0;
  for (unsigned i = 0; i < tmpSize / 4; i++) {
    f.write(reinterpret_cast<char*>(&tmp), sizeof(float));
  }
}

void ImageHeader::write(const String& filename, bool force_reversed) {
  std::ofstream f;
  f.open(filename.c_str(), std::ios::out | std::ios::binary);
  if (f.fail()) {
    IMP_THROW("ImageHeader::write: file " + filename + " not found",
              IOException);
  }
  write(f, force_reversed);
  f.close();
}

void ImageHeader::clear() {
  memset(&spider_header_, 0, sizeof(SpiderHeader));
  set_header();  // Set a consistent header
  reversed_ = false;
}

void ImageHeader::set_header() {
  // "dark" stuff for SPIDER compatibility needed in the spider_header_.
  // Adopted from Xmipp Libraries
  if ((spider_header_.fNcol != 0) && (spider_header_.fNrow != 0)) {
    spider_header_.fNlabel = (float)((int)(256 / spider_header_.fNcol + 1));
    spider_header_.fLabrec =
        (float)std::ceil((float)(256 / (float)spider_header_.fNcol));
    if ((1024 % (int)spider_header_.fNcol != 0)) {
      spider_header_.fNrec = spider_header_.fNrow + 1;
    } else {
      spider_header_.fNrec = spider_header_.fNrow;
    }
    spider_header_.fLabbyt = get_spider_header_size();
    spider_header_.fLenbyt = (float)spider_header_.fNcol * 4;
  }

  // file type
  switch ((int)spider_header_.fIform) {
    case IMG_BYTE:
      spider_header_.fIform = 0;  // for a 2D image.
      break;
    case IMG_IMPEM:
      spider_header_.fIform = 1;  // for a 2D image.
      break;
    case IMG_INT:
      spider_header_.fIform = 9;  // for a 2D int image.
      break;
    case VOL_BYTE:
      spider_header_.fIform = 2;  // for a 3D volume.
      break;
    case VOL_IMPEM:
      spider_header_.fIform = 3;  // for a 3D volume.
      break;
    case VOL_INT:
      spider_header_.fIform = 10;  // for a 3D int volume.
      break;
    case IMG_FOURIER:
      spider_header_.fIform = -1;  // for a 2D Fourier transform.
      break;
    case VOL_FOURIER:
      spider_header_.fIform = -3;  // for a 3D Fourier transform.
      break;
  }

  // Set scale to 1 (never used but kept for compatibility with Spider)
  spider_header_.fScale = 1;
  // Set Geometrical transformation matrix to Identity
  // (compatibility with Spider again)
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (i == j) {
        spider_header_.fGeo_matrix[i][j] = (double)1.0;
      } else {
        spider_header_.fGeo_matrix[i][j] = (double)0.0;
      }
    }
  }
}

char* ImageHeader::get_date() const { return (char*)spider_header_.szIDat; }

void ImageHeader::set_date() {
  time_t lTime;
  struct tm* tmTime;
  time(&lTime);                // Get seconds since January 1 1970
  tmTime = localtime(&lTime);  // Get current time
  tmTime->tm_mon++;            // Months are from 0..11 so put ti from 1..12
  tmTime->tm_year -= 100;      // Years are from 1900 and we want it from 2000
  sprintf(spider_header_.szIDat, "%02d%s%02d%s%02d", tmTime->tm_mday, "-",
          tmTime->tm_mon, "-", tmTime->tm_year);
}

char* ImageHeader::get_time() const { return (char*)spider_header_.szITim; }

void ImageHeader::set_time() {
  time_t lTime;
  struct tm* tmTime;
  time(&lTime);                // Get seconds since January 1 1970
  tmTime = localtime(&lTime);  // Get current time
  sprintf(spider_header_.szITim, "%02d%s%02d", tmTime->tm_hour, ":",
          tmTime->tm_min);
}

char* ImageHeader::get_title() const { return (char*)spider_header_.szITit; }

void ImageHeader::set_title(String newName) {
  strcpy(spider_header_.szITit, newName.c_str());
}

double ImageHeader::get_fGeo_matrix(unsigned int i, unsigned int j) const {
  return spider_header_.fGeo_matrix[i][j];
}

IMPEM_END_NAMESPACE
