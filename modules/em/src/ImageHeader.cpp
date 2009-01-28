/**
 *  \file ImageHeader.cpp
 *  \brief Header for EM images
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#include <IMP/em/ImageHeader.h>
#include <IMP/em/endian.h>
#include <ctime>
#include <string.h>

IMPEM_BEGIN_NAMESPACE


void ImageHeader::print_hard(std::ostream &out) const
{
  out << "fNslice=" << header_.fNslice << std::endl;
  out << "fNrow=" <<   header_.fNrow << std::endl;
  out << "fNrec=" <<   header_.fNrec << std::endl;
  out << "fNlabel=" << header_.fNlabel << std::endl;
  out << "fIform=" <<  header_.fIform << std::endl;
  out << "fImami=" <<  header_.fImami << std::endl;
  out << "fFmax=" <<   header_.fFmax << std::endl;
  out << "fFmin=" <<   header_.fFmin << std::endl;
  out << "fAv=" <<     header_.fAv << std::endl;
  out << "fSig=" <<    header_.fSig << std::endl;
  out << "fIhist=" <<  header_.fIhist << std::endl;
  out << "fNcol=" <<   header_.fNcol << std::endl;
  out << "fLabrec=" << header_.fLabrec << std::endl;
  out << "fIangle=" << header_.fIangle << std::endl;
  out << "fPhi=" <<    header_.fPhi << std::endl;
  out << "fTheta=" <<  header_.fTheta << std::endl;
  out << "fPsi=" <<    header_.fPsi << std::endl;
  out << "fXoff=" <<   header_.fXoff << std::endl;
  out << "fYoff=" <<   header_.fYoff << std::endl;
  out << "fZoff=" <<   header_.fZoff << std::endl;
  out << "fScale=" <<  header_.fScale << std::endl;
  out << "fLabbyt=" << header_.fLabbyt << std::endl;
  out << "fLenbyt=" << header_.fLenbyt << std::endl;
}



//#define DEBUG
bool ImageHeader::read(std::ifstream& f, bool skip_type_check,
        bool force_reversed,bool skip_extra_checkings)
{
  float tmp;
  unsigned tmpSize;
  unsigned long size;

  /* Determine reverse status according to the next table .................
       (computed in Linux) */
#define TYPE_TABLE_SIZE 10

  int type_table[TYPE_TABLE_SIZE][5] = {{0, 0,  48,  65, 11},
    {0, 0,  32,  65, 10},
    {0, 0,  16,  65,  9},
    {0, 0,   0,  65,  8},
    {0, 0,  64,  64,  3},
    {0, 0, 128,  63,  1},
    {0, 0, 128, 191, -1},
    {0, 0,  64, 192, -3},
    {0, 0, 160, 192, -5},
    {0, 0, 224, 192, -7}
  };
  union {
    unsigned char c[4];
    float f;
  } file_type;

  unsigned long current_position = f.tellg();

  if (!skip_type_check) {
    // Read file type
    f.seekg(current_position + 16, std::ios::beg);
    for (int i = 0; i < 4; i++) {
      f.read(reinterpret_cast< char* >(&(file_type.c[i])), sizeof(char));
    }
    f.seekg(current_position + 0, std::ios::beg);
    // Select type
    int correct_type = 0;
    reversed_ = false;

#define IS_TYPE(n) \
    (type_table[n][0]==(int)file_type.c[0] && \
     type_table[n][1]==(int)file_type.c[1] && \
     type_table[n][2]==(int)file_type.c[2] && \
     type_table[n][3]==(int)file_type.c[3])
#define IS_REVERSE_TYPE(n) \
    (type_table[n][0]==file_type.c[3] && \
     type_table[n][1]==file_type.c[2] && \
     type_table[n][2]==file_type.c[1] && \
     type_table[n][3]==file_type.c[0])

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
    f.read(reinterpret_cast< char* >(&header_), sizeof(SpiderHeader));
  }
  // Read numerical fields reversed
  else {
    reversed_read(&header_,             sizeof(float),  36, f, true);
    reversed_read(&header_.fGeo_matrix, sizeof(double),  9, f, true);
    // 14 is the number of fields in the SpiderHeader struct after fGeo_matrix
    reversed_read(&header_.fAngle1,     sizeof(float),  14, f, true);
    reversed_read(&header_.empty,      sizeof(char),  752, f, true);
  }

  unsigned long usfNcol = (unsigned long) header_.fNcol;
  unsigned long usfNrow = (unsigned long) header_.fNrow;
  unsigned long usfNslice = (unsigned long) header_.fNslice;
  unsigned long usfHeader = (unsigned long) get_header_size();

  // Get file size
  current_position = f.tellg();
  unsigned long init_pos, file_size;
  f.seekg(0, std::ios::beg);
  init_pos = f.tellg();
  f.seekg(0, std::ios::end);
  file_size = (unsigned long)f.tellg() - init_pos;

  f.seekg(current_position, std::ios::beg);

  // Check if it is an "aberrant" image
  if (im_ == IMG_IMPEM || header_.fIform == 1) {
    if ((usfNcol*usfNrow*sizeof(float)) == file_size) {
      usfNrow = (unsigned long)(--header_.fNrow);
      --header_.fNrec;
    }
  }
  // Extra checkings
  if (!skip_extra_checkings) {
    switch (im_) {
    case IMG_BYTE:
      size = usfNcol * usfNrow * sizeof(float);
      if ((size != file_size)) {
        return false;
      }
      break;
    case IMG_IMPEM:

      size = usfHeader + usfNcol * usfNrow * sizeof(float);

      if ((size != file_size) || (header_.fIform != 1)) {
        return false;
      } else if (skip_type_check) {
        header_.fIform = 1;
        // This is done to recover files which are not
        // properly converted from other packages
      }
      break;
    case IMG_INT:
      size = usfHeader + usfNcol * usfNrow * sizeof(float);
      if ((size != file_size) || (header_.fIform != 9)) {
        return false;
      } else if (skip_type_check) {
        header_.fIform = 9; // This is done to recover
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
      if ((size != file_size) || (header_.fIform != 3)) {
        return false;
      } else if (skip_type_check) {
        header_.fIform = 3;
      }
      break;
    case VOL_INT:
      size = usfHeader + usfNslice * usfNcol * usfNrow * sizeof(float);
      if ((size != file_size) || (header_.fIform != 10)) {
        return false;
      } else if (skip_type_check) {
        header_.fIform = 10;
      }
      break;
    case IMG_FOURIER:
      size = usfHeader + 2 * usfNcol * usfNrow * sizeof(float);
      // The term 2 is to take into account that IMG_FOURIER
      // stores complex numbers with 2 floats for each one.
      if ((size != file_size) ||
          (header_.fIform != -5 && header_.fIform != -1))
        return false;
      else if (skip_type_check) {
        header_.fIform = -1;
      }
      break;
    case VOL_FOURIER:
      size = usfHeader + 2 * usfNslice * usfNcol * usfNrow * sizeof(float);
      // The term 2 is to take into account that VOL_FOURIER
      // stores complex numbers with 2 floats for each one.
      if ((size != file_size) ||
          (header_.fIform != -7 && header_.fIform != -3)) {
        return false;
      } else if (skip_type_check) {
        header_.fIform = -3;
      }
      break;
    }
  }

  /* This is Spider stuff, it's an acient trick.
    Spider images contain:
    - a header with the size of the struct SpiderHeader,
    - a "filling" empty space
    - the data of size cols*rows*sizeof(float).
  */

  header_.fLabrec = (float) ceil((float) 256 / header_.fNcol);
  tmpSize = (int)(header_.fNcol * header_.fLabrec * 4); //Size of whole header
  tmpSize -= sizeof(SpiderHeader);         //Decrease the real header

  // read empty filling space
  for (unsigned i = 0; i < tmpSize / 4; i++) {
    reversed_read(&tmp, sizeof(float), 1, f, reversed_);
  }
  return true;
}
#undef DEBUG

int ImageHeader::read(const String filename, bool skip_type_check,
                      bool force_reversed, bool skip_extra_checkings)
{
  bool result;
  std::ifstream f;
  // Clear Header
  clear();
  // Read
  f.open(filename.c_str(), std::ios::in | std::ios::binary);
  if (f.fail()) {
    String msg = "ImageHeader::read: file " + filename + " not found";
    throw ErrorException(msg.c_str());
  }
  result = read(f, force_reversed);
  f.close();
  return result;
}


void ImageHeader::write(std::ofstream& f, bool force_reversed)
{
  float tmp;
  unsigned tmpSize;

  if (get_rows() == 0 || get_columns() == 0 || get_slices() == 0) {
    return;
  }
  // Set consistent header before saving
  set_header();
  set_time();
  set_date();
  // Write header
  if (xorT(reversed_, force_reversed)) {
    reversed_ = true;
    reversed_write(&header_,             sizeof(float),  36, f, true);
    reversed_write(&header_.fGeo_matrix, sizeof(double),  9, f, true);
    // 14 is the number of float fields after fGeo_matrix in the eader_ struct
    reversed_write(&header_.fAngle1,     sizeof(float),  14, f, true);
    // 752 is the number of chars to the end of the file
    f.write(header_.empty, sizeof(char)*752);
  } else {
    reversed_ = false;
    f.write(reinterpret_cast< char* >(&header_),
            sizeof(ImageHeader::SpiderHeader));
  }
  // Write empty filling space (filled with zeros)
  tmpSize = get_header_size(); //Size of whole header
  tmpSize -= sizeof(SpiderHeader);             //Decrease the real header
  tmp = (float) 0.0;
  for (unsigned i = 0; i < tmpSize / 4; i++) {
    f.write(reinterpret_cast< char* >(&tmp), sizeof(float));
  }
}


void ImageHeader::write(const String &filename, bool force_reversed)
{
  std::ofstream f;
  f.open(filename.c_str(), std::ios::out | std::ios::binary);
  if (f.fail()) {
    String msg = "ImageHeader::write: file " + filename + " not found";
    throw ErrorException(msg.c_str());
  }
  write(f, force_reversed);
  f.close();
}


void ImageHeader::clear()
{
  memset(&header_, 0, sizeof(SpiderHeader));
  set_header(); // Set a consistent header
  reversed_ = false;
}

void ImageHeader::set_header()
{
  // "dark" stuff for SPIDER compatibility needed in the header_.
  // Adopted from Xmipp Libraries
  if ((header_.fNcol != 0) && (header_.fNrow != 0)) {
    header_.fNlabel = (float)((int)(256 / header_.fNcol + 1));
    header_.fLabrec = (float) std::ceil((float)(256 / (float)header_.fNcol));
    if ((1024 % (int)header_.fNcol != 0)) {
      header_.fNrec = header_.fNrow + 1;
    } else {
      header_.fNrec = header_.fNrow;
    }
    header_.fLabbyt = get_header_size();
    header_.fLenbyt = (float) header_.fNcol * 4;
  }

  // file type
  switch (im_) {
  case IMG_BYTE:
    header_.fIform = 0;    // for a 2D image.
    break;
  case IMG_IMPEM:
    header_.fIform = 1;    // for a 2D image.
    break;
  case IMG_INT:
    header_.fIform = 9;    // for a 2D int image.
    break;
  case VOL_BYTE:
    header_.fIform = 2;    // for a 3D volume.
    break;
  case VOL_IMPEM:
    header_.fIform = 3;    // for a 3D volume.
    break;
  case VOL_INT:
    header_.fIform = 10;    // for a 3D int volume.
    break;
  case IMG_FOURIER:
    header_.fIform = -1;    // for a 2D Fourier transform.
    break;
  case VOL_FOURIER:
    header_.fIform = -3;    // for a 3D Fourier transform.
    break;
  }

  // Set scale to 1 (never used but kept for compatibility with Spider)
  header_.fScale = 1;
  // Set Geometrical transformation Matrix to Identity
  // (compatibility with Spider again)
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (i == j) {
        header_.fGeo_matrix[i][j] = (double)1.0;
      } else {
        header_.fGeo_matrix[i][j] = (double)0.0;
      }
    }
  }
  // Satistical information (compatibility with Spider again)
  header_.fSig = -1;
  header_.fImami = 0;
}


void ImageHeader::set_dimensions(float Ydim, float Xdim)
{
  header_.fNrow = Ydim;
  header_.fNcol = Xdim;
}

void ImageHeader::get_dimensions(float &Ydim, float &Xdim) const
{
  Ydim = header_.fNrow;
  Xdim = header_.fNcol;
}

char* ImageHeader::get_date() const
{
  return (char*) header_.szIDat;
}

void ImageHeader::set_date()
{
  time_t lTime;
  struct tm *tmTime;
  time(&lTime);                // Get seconds since January 1 1970
  tmTime = localtime(&lTime);  // Get current time
  tmTime->tm_mon++;            // Months are from 0..11 so put ti from 1..12
  tmTime->tm_year -= 100;      // Years are from 1900 and we want it from 2000
  sprintf(header_.szIDat, "%02d%s%02d%s%02d", tmTime->tm_mday, "-",
          tmTime->tm_mon, "-", tmTime->tm_year);
}

char* ImageHeader::get_time() const
{
  return (char*) header_.szITim;
}

void ImageHeader::set_time()
{
  time_t lTime;
  struct tm *tmTime;
  time(&lTime);                // Get seconds since January 1 1970
  tmTime = localtime(&lTime);  // Get current time
  sprintf(header_.szITim, "%02d%s%02d", tmTime->tm_hour, ":", tmTime->tm_min);
}

char* ImageHeader::get_title() const
{
  return (char*) header_.szITit;
}

void ImageHeader::set_title(String newName)
{
  strcpy(header_.szITit, newName.c_str());
}

void ImageHeader::set_origin_offsets(float Xoff, float Yoff)
{
  header_.fXoff = Xoff;
  header_.fYoff = Yoff;
}

void ImageHeader::set_euler_angles(float Phi, float Theta, float Psi)
{
  // sets flag and angles
  header_.fIangle = 1;
  header_.fPhi = Phi;
  header_.fTheta = Theta;
  header_.fPsi = Psi;
}

void ImageHeader::set_euler_angles1(float Phi1, float Theta1, float Psi1)
{
  // sets flag and angles
  if (header_.fFlag != 2.f) {
    header_.fFlag = 1.f;
  }
  header_.fPhi1 = Phi1;
  header_.fTheta1 = Theta1;
  header_.fPsi1 = Psi1;
}

void ImageHeader::set_euler_angles2(float Phi2, float Theta2, float Psi2)
{
  // sets flag
  header_.fFlag = 2;
  header_.fPhi2 = Phi2;
  header_.fTheta2 = Theta2;
  header_.fPsi2 = Psi2;
}

void ImageHeader::get_origin_offsets(float &Xoff, float &Yoff) const
{
  Xoff =  header_.fXoff;
  Yoff =  header_.fYoff;
}

algebra::Matrix2D<double> ImageHeader::get_fGeo_matrix()
{
  algebra::Matrix2D<double> retval(3, 3);
  retval(0, 0) = header_.fGeo_matrix[0][0];
  retval(0, 1) = header_.fGeo_matrix[0][1];
  retval(0, 2) = header_.fGeo_matrix[0][2];
  retval(1, 0) = header_.fGeo_matrix[1][0];
  retval(1, 1) = header_.fGeo_matrix[1][1];
  retval(1, 2) = header_.fGeo_matrix[1][2];
  retval(2, 0) = header_.fGeo_matrix[2][0];
  retval(2, 1) = header_.fGeo_matrix[2][1];
  retval(2, 2) = header_.fGeo_matrix[2][2];
  return retval;
}

IMPEM_END_NAMESPACE
