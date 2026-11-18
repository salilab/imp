/**
 *  \file XplorReaderWriter.cpp
 *  \brief Classes to read or write density files in XPLOR format.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/XplorReaderWriter.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

IMPEM_BEGIN_NAMESPACE

namespace {
// Read a line from the stream, throwing an IOException if we cannot
std::string get_xplor_line(std::ifstream &XPLORstream) {
  std::string line;
  if (!std::getline(XPLORstream, line)) {
    IMP_THROW("Premature end of XPLOR file header", IOException);
  }
  return line;
}
}  // namespace

void XplorReaderWriter::read(const char *filename, float **data,
                             DensityHeader &header) {
  std::ifstream XPLORstream(filename);
  if (!XPLORstream) {
    IMP_THROW("Unable to open XPLOR file " << filename, IOException);
  }
  // header
  internal::XplorHeader xheader;
  read_header(XPLORstream, xheader);
  xheader.GenerateCommonHeader(header);

  // data
  size_t size = static_cast<size_t>(xheader.extent[0]) * xheader.extent[1]
                * xheader.extent[2];
  std::unique_ptr<float[]> grid(new float[size]);
  read_map(XPLORstream, grid.get(), xheader);
  XPLORstream.close();
  *data = grid.release();
}

int XplorReaderWriter::read_header(std::ifstream &XPLORstream,
                                   internal::XplorHeader &header) {
  // read the configuration values:
  for (int i = 0; i < 4; i++) {
    get_xplor_line(XPLORstream);
  }

  // read the grid size, origin and top for each axis
  {
    std::istringstream iss(get_xplor_line(XPLORstream));
    for (int j = 0; j < 3; j++) {
      if (!(iss >> header.grid[j] >> header.orig[j] >> header.top[j])) {
        IMP_THROW("Malformed XPLOR grid line: expected 9 integers",
                  IOException);
      }
    }
  }
  for (int j = 0; j < 3; j++) {
    header.extent[j] = header.top[j] - header.orig[j] + 1;
    if (header.grid[j] <= 0 || header.extent[j] <= 0) {
      IMP_THROW("Invalid XPLOR grid: grid size " << header.grid[j]
                << ", extent " << header.extent[j], IOException);
    }
  }

  ////////////////////
  // read the unit cell dimensions
  ////////////////////
  {
    std::istringstream iss(get_xplor_line(XPLORstream));
    for (int j = 0; j < 3; j++) {
      if (!(iss >> header.cellsize[j])) {
        IMP_THROW("Malformed XPLOR unit cell line: expected 6 numbers",
                  IOException);
      }
    }
    for (int j = 0; j < 3; j++) {
      if (!(iss >> header.cellangle[j])) {
        IMP_THROW("Malformed XPLOR unit cell line: expected 6 numbers",
                  IOException);
      }
    }
  }

  for (int j = 0; j < 3; j++) {
    header.voxelsize[j] = header.cellsize[j] / header.grid[j];
    header.translateGrid[j] = header.orig[j] * header.voxelsize[j];
  }

  //////////////
  // read the grid order
  //  we do not use this data. We use the user input for the grid order.
  //////////////
  get_xplor_line(XPLORstream);

  return 0;
}

int XplorReaderWriter::read_map(std::ifstream &XPLORstream, float *data,
                                internal::XplorHeader &header) {

  // reading the map according to the grid order.
  std::string line;
  int x = 0, y = 0, z = 0;  // the step size in the three directions
  bool status = false;

  // update orig and top according to the values in the map
  for (int i = 0; i < 3; i++) {
    header.orig[i] = 9999;
    header.top[i] = -9999;
  }

  char dens[13];

  // Stop once all sections have been read; anything after that (such as
  // the -9999 marker and the map mean and standard deviation) is ignored
  while (z < header.extent[2] && std::getline(XPLORstream, line)) {
    if (!status) {  // status = false means that we moved a section
      status = true;
    } else {
      // each line have strlen/12 density values.
      // We iterate through the line
      size_t densNum = line.size() / 12;

      for (size_t counter = 0; counter < densNum; ++counter) {
        memcpy(dens, line.data() + (counter * 12), 12);
        dens[12] = '\0';
        data[x + y * header.extent[0] +
             z * header.extent[0] * header.extent[1]] =
            static_cast<float>(atof(dens));
        if (x < header.orig[0]) header.orig[0] = x;
        if (y < header.orig[1]) header.orig[1] = y;
        if (z < header.orig[2]) header.orig[2] = z;
        if (x > header.top[0]) header.top[0] = x;
        if (y > header.top[1]) header.top[1] = y;
        if (z > header.top[2]) header.top[2] = z;

        x++;
        if (x >= header.extent[0]) {
          x = 0;
          y++;
          if (y >= header.extent[1]) break;
        }
      }

      if (y >= header.extent[1]) {
        x = 0;
        y = 0;
        z++;
        status = false;
      }
    }
  }
  if (z < header.extent[2]) {
    IMP_THROW("Premature end of XPLOR file: read " << z << " of "
              << header.extent[2] << " sections", IOException);
  }
  return 0;
}

void XplorReaderWriter::write(const char *filename, const float *data,
                              const DensityHeader &header_) {
  internal::XplorHeader header(header_);

  std::ofstream s(filename);

  s << std::endl << "       2" << std::endl << "REMARKS file name = ??? "
    << std::endl << "REMARKS Date ...... created by em lib " << std::endl;
  s.setf(std::ios::right, std::ios::adjustfield);
  s.width(8);
  for (int i = 0; i < 3; i++) {
    s << std::setw(8) << header.grid[i] << std::setw(8)
      << floor(header.translateGrid[i] / header.voxelsize[i]) << std::setw(8)
      << floor(header.translateGrid[i] / header.voxelsize[i]) +
             header.extent[i] - 1;
  }
  s << std::endl;
  for (int i = 0; i < 3; i++) {
    s << std::scientific << std::setprecision(5) << std::setw(12)
      << header.cellsize[i];
  }
  for (int i = 0; i < 3; i++) {
    s << std::scientific << std::setprecision(5) << std::setw(12)
      << header.cellangle[i];
  }
  s << std::endl << "XYZ" << std::endl;  // Z is the slowest
  int counter = 0;
  for (int k = 0; k < header.extent[2]; k++) {
    if (counter != 0) {
      s << std::endl;
      counter = 0;
    }

    s << std::setw(8) << k << std::endl;
    for (int j = 0; j < header.extent[1]; j++) {
      for (int i = 0; i < header.extent[0]; i++) {
        s << std::scientific << std::setprecision(5) << std::setw(12)
          << data[i + j * header.extent[0] +
                  k * header.extent[0] * header.extent[1]];
        counter++;
        if (counter == 6) {
          counter = 0;
          s << std::endl;
        }
      }
    }
  }
  s << std::endl << "  -9999" << std::endl;
  s.close();
}

IMPEM_END_NAMESPACE
