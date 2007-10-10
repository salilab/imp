#include "DensityMap.h"


DensityMap::DensityMap(){
  loc_calculated = false;
  normalized = false;
  rms_calculated = false;
}


void DensityMap::Read(const char *filename, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  reader.Read(filename,&data_,header_);
}
void DensityMap::Write(const char *filename, MapReaderWriter &writer) {
  writer.Write(filename,data_,header_);
}

float DensityMap::voxel2loc(const int &index,int dim) {
  if (!loc_calculated) 
    calc_all_voxel2loc();
  if (dim==0) {
    return x_loc_[index];
    }
  else if (dim==1) {
    return y_loc_[index];
    }
  //TODO - add error handling, dim is not 0,1,2
  return z_loc_[index];
}

void DensityMap::calc_all_voxel2loc() {

  if (loc_calculated) 
    return;
  int nvox = header_.nx*header_.ny*header_.nz;
  x_loc_ = new float[nvox];
  y_loc_ = new float[nvox];
  z_loc_ = new float[nvox];
  loc_calculated = true;



  int ix=0,iy=0,iz=0;

  for (int ii=0;ii<nvox;ii++) {
    x_loc_[ii] =  ix * header_.Objectpixelsize + header_.xorigin;
    y_loc_[ii] =  iy * header_.Objectpixelsize + header_.yorigin;
    z_loc_[ii] =  iz * header_.Objectpixelsize + header_.zorigin;

    // bookkeeping
    ix = ix +1;
    if (ix == header_.nx) {
      ix = 0;
      iy = iy+1;
      if (iy == header_.ny) {
        iy = 0;
        iz = iz+1;
      }
    }
  }
}



void DensityMap::stdNormalize() {
  
  float inv_std = 1.0/calcRMS();
  float mean = header_.dmean;
  int nvox = header_.nx * header_.ny * header_.nz;
  for (int ii=0;ii<nvox;ii++) {
    data_[ii] = (data_[ii] - mean) * inv_std;
  }
  normalized = true;
  header_.rms=1.;
  header_.dmean=0.0;
}



float DensityMap::calcRMS() {

  if (rms_calculated)
    return header_.rms;

  int  nvox = header_.nx * header_.ny * header_.nz;
  float meanval = .0;
  float stdval = .0;

  for (int ii=0;ii<nvox;ii++) {
    meanval = meanval + data_[ii];
    stdval = stdval + powf(data_[ii], 2);
  }
  
  meanval /=  nvox;
  header_.dmean = meanval;

  stdval = stdval - (powf(meanval, 2) * nvox);
  stdval = sqrt(stdval) / nvox;
  header_.rms = stdval;

  return stdval;

}
