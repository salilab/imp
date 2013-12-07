/**
 *  \file DensityHeader.cpp
 *  \brief Metadata for a density file.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/DensityHeader.h>

IMPEM_BEGIN_NAMESPACE

namespace {
template <class T>
void initialize(T &t) {
  if (std::numeric_limits<T>::has_signaling_NaN) {
    t = std::numeric_limits<T>::signaling_NaN();
  } else if (std::numeric_limits<T>::has_quiet_NaN) {
    t = std::numeric_limits<T>::quiet_NaN();
  } else if (std::numeric_limits<T>::has_infinity) {
    t = std::numeric_limits<T>::infinity();
  } else {
    // numerical limits for int and double have completely
    // different meanings of max/min
    t = -std::numeric_limits<T>::max();
  }
}
}
DensityHeader::DensityHeader() {
  initialize(nx_);
  initialize(ny_);
  initialize(nz_);
  initialize(data_type_);
  initialize(nxstart);
  initialize(nystart);
  initialize(nzstart);
  initialize(mx);
  initialize(my);
  initialize(mz);
  initialize(xlen);
  initialize(ylen);
  initialize(zlen);
  initialize(alpha);
  initialize(beta);
  initialize(gamma);
  initialize(mapc);
  initialize(mapr);
  initialize(maps);
  initialize(dmin);
  initialize(dmax);
  initialize(dmean);
  initialize(ispg);
  initialize(nsymbt);
  user[0] = 0;
  map[0] = 0;
  initialize(machinestamp);
  initialize(rms);
  initialize(nlabl);
  for (unsigned int i = 0; i < COMMENT_FIELD_NUM_OF; ++i) {
    comments[i][0] = 0;
  }
  initialize(magic);
  initialize(voltage);
  initialize(Cs);                 // Cs of microscope
  initialize(Aperture);           // Aperture used
  initialize(Magnification);      // Magnification
  initialize(Postmagnification);  // Postmagnification (of energy filter)
  initialize(Exposuretime);       // Exposuretime
  initialize(Microscope);         // Microscope
  initialize(Pixelsize);    // Pixelsize - used for the microscope CCD camera
  initialize(CCDArea);      // CCDArea
  initialize(Defocus);      // Defocus
  initialize(Astigmatism);  // Astigmatism
  initialize(AstigmatismAngle);   // Astigmatism Angle
  initialize(FocusIncrement);     // Focus-Increment
  initialize(CountsPerElectron);  // Counts/Electron
  initialize(Intensity);          // Intensity
  initialize(EnergySlitwidth);    // Energy slitwidth of energy filter
  initialize(EnergyOffset);       // Energy offset of Energy filter
  initialize(Tiltangle);          // Tiltangle of stage
  initialize(Tiltaxis);           // Tiltaxis
  initialize(MarkerX);            // Marker_X coordinate
  initialize(MarkerY);            // Marker_Y coordinate
  initialize(lswap);
  initialize(Objectpixelsize_);  // this is the actual pixelsize
  initialize(xtop_);
  initialize(ytop_);
  initialize(ztop_);  // The upper bound for the x,y and z grid.
  initialize(xorigin_);
  initialize(yorigin_);
  initialize(zorigin_);  // Origin used for transforms
  top_calculated_ = false;
  initialize(resolution_);
  is_resolution_set_ = false;
  top_calculated_ = false;
  // Initialize some of the parameters of the header
  nx_ = 0;
  ny_ = 0;
  nz_ = 0;
  data_type_ = 5;
  nxstart = 1;
  nystart = 1;
  nzstart = 1;
  mx = nxstart + nx_ - 1;
  my = nystart + ny_ - 1;
  mz = nzstart + nz_ - 1;
  xorigin_ = yorigin_ = zorigin_ = 0.0;
  alpha = 90.;
  beta = 90.;
  gamma = 90.;
  mapc = 1;
  mapr = 2;
  maps = 3;
  ispg = 0;
  nsymbt = 0;
  Objectpixelsize_ = 1.0;
  machinestamp = 0;
  nlabl = 0;
  strcpy(map, "MAP\0");
  magic = 6;
}

void DensityHeader::compute_xyz_top(bool force) {
  if (top_calculated_ && !force) return;
  xtop_ = xorigin_ + Objectpixelsize_ * nx_;
  ytop_ = yorigin_ + Objectpixelsize_ * ny_;
  ztop_ = zorigin_ + Objectpixelsize_ * nz_;
  top_calculated_ = true;
}

void DensityHeader::update_map_dimensions(int nnx, int nny, int nnz) {
  nx_ = nnx;
  ny_ = nny;
  nz_ = nnz;
  mx = nx_;
  my = ny_;
  mz = nz_;
  top_calculated_ = false;
  compute_xyz_top();
  update_cell_dimensions();
}

void DensityHeader::update_cell_dimensions() {
  xlen = nx_ * Objectpixelsize_;
  ylen = ny_ * Objectpixelsize_;
  zlen = nz_ * Objectpixelsize_;
}

DensityHeader create_density_header(const algebra::BoundingBoxD<3> &bb,
                                    float spacing) {
  DensityHeader header;
  algebra::Vector3D origin = bb.get_corner(0);
  algebra::Vector3D top = bb.get_corner(1);
  header.set_xorigin(origin[0]);
  header.set_yorigin(origin[1]);
  header.set_zorigin(origin[2]);
  header.Objectpixelsize_ = spacing;
  // calculate the extent of the map
  int extent[3];
  for (int i = 0; i < 3; i++) {
    extent[i] = ceil((top[i] - origin[i]) / spacing);
  }
  header.update_map_dimensions(extent[0], extent[1], extent[2]);
  header.compute_xyz_top(true);
  return header;
}
void DensityHeader::show(std::ostream &out) const {
  out << "nx: " << nx_ << " ny: " << ny_ << " nz: " << nz_ << std::endl;
  out << "data_type: " << data_type_ << std::endl;
  out << "nxstart: " << nxstart << " nystart: " << nystart
      << " nzstart: " << nzstart << std::endl;
  out << "mx: " << mx << " my:" << my << " mz: " << mz << std::endl;
  out << "xlen: " << xlen << " ylen: " << ylen << " zlen: " << zlen
      << std::endl;
  out << "alpha : " << alpha << " beta: " << beta << " gamma: " << gamma
      << std::endl;
  out << "mapc : " << mapc << " mapr: " << mapr << " maps: " << maps
      << std::endl;
  out << "dmin: " << dmin << " dmax: " << dmax << " dmean: " << dmean
      << std::endl;
  out << "ispg: " << ispg << std::endl;
  out << "nsymbt: " << nsymbt << std::endl;
  out << "user: " << user << std::endl;
  out << "xorigin: " << xorigin_ << " yorigin: " << yorigin_
      << " zorigin: " << zorigin_ << std::endl;
  out << "map: " << map << std::endl;
  out << "Objectpixelsize: " << Objectpixelsize_ << std::endl;
  out << "Resolution: " << resolution_ << std::endl;
  out << "machinestamp: " << machinestamp << std::endl;
  out << "rms: " << rms << std::endl;
  out << "nlabl: " << nlabl << std::endl;
  for (int i = 0; i < nlabl; i++)
    out << "comments[" << i << "] = ->" << comments[i] << "<-" << std::endl;
}
IMPEM_END_NAMESPACE
