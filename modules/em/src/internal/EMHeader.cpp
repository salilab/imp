/**
 *  \file EMHeader.cpp
 *  \brief Header information for files in EM format.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/internal/EMHeader.h>

IMPEM_BEGIN_INTERNAL_NAMESPACE

EMHeader::EMHeader(const DensityHeader &header) {
  nx = header.get_nx();
  ny = header.get_ny();
  nz = header.get_nz();
  magic = header.magic;
  type = header.get_data_type();
  for (short i = 0; i < DensityHeader::COMMENT_FIELD_SINGLE_SIZE; i++) {
    comment[i] = header.comments[0][i];
  }
  voltage = header.voltage;
  Cs = header.Cs;
  Aperture = header.Aperture;
  Magnification = header.Magnification;
  Postmagnification = header.Postmagnification;
  Exposuretime = header.Exposuretime;
  Objectpixelsize = header.get_spacing();
  Microscope = header.Microscope;
  Pixelsize = header.Pixelsize;
  CCDArea = header.CCDArea;
  Defocus = header.Defocus;
  Astigmatism = header.Astigmatism;
  AstigmatismAngle = header.AstigmatismAngle;
  FocusIncrement = header.FocusIncrement;
  CountsPerElectron = header.CountsPerElectron;
  Intensity = header.Intensity;
  EnergySlitwidth = header.EnergySlitwidth;
  EnergyOffset = header.EnergyOffset;
  Tiltangle = header.Tiltangle;
  Tiltaxis = header.Tiltaxis;
  MarkerX = header.MarkerX;
  MarkerY = header.MarkerY;
  lswap = header.lswap;
}

void EMHeader::generate_common_header(DensityHeader &header) {
  header.Objectpixelsize_ = Objectpixelsize;
  header.update_map_dimensions(nx, ny, nz);
  header.magic = magic;
  header.set_data_type(type);
  for (short i = 0; i < DensityHeader::COMMENT_FIELD_SINGLE_SIZE; i++) {
    header.comments[0][i] = comment[i];
  }

  header.voltage = voltage;
  header.Cs = Cs;
  header.Aperture = Aperture;
  header.Magnification = Magnification;
  header.Postmagnification = Postmagnification;
  header.Exposuretime = Exposuretime;
  header.Microscope = Microscope;
  header.Pixelsize = Pixelsize;
  header.CCDArea = CCDArea;
  header.Defocus = Defocus;
  header.Astigmatism = Astigmatism;
  header.AstigmatismAngle = AstigmatismAngle;
  header.FocusIncrement = FocusIncrement;
  header.CountsPerElectron = CountsPerElectron;
  header.Intensity = Intensity;
  header.EnergySlitwidth = EnergySlitwidth;
  header.EnergyOffset = EnergyOffset;
  header.Tiltangle = Tiltangle;
  header.Tiltaxis = Tiltaxis;
  header.MarkerX = MarkerX;
  MarkerY = header.MarkerY;
  header.lswap = lswap;
}

IMPEM_END_INTERNAL_NAMESPACE
