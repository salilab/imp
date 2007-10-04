#include "DensityMap.h"

void DensityMap::Read(const char *filename, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  reader.Read(filename,&data,header);
}
void DensityMap::Write(const char *filename, MapReaderWriter &writer) {
  writer.Write(filename,data,header);
}

float DensityMap::voxel2loc(const int &index,int dim) {
  if (!loc_calculated) 
    calc_all_voxel2loc();
  if (dim==0) {
    return x_loc[index];
    }
  else if (dim==1) {
    return y_loc[index];
    }
  //TODO - add error handling, dim is not 0,1,2
  return z_loc[index];
}

void DensityMap::calc_all_voxel2loc() {

  if (loc_calculated) 
    return;
  int nvox = header.nx*header.ny*header.nz;
  x_loc = new float[nvox];
  y_loc = new float[nvox];
  z_loc = new float[nvox];
  loc_calculated = true;



  int ix=0,iy=0,iz=0;

  for (int ii=0;ii<nvox;ii++) {
    x_loc[ii] =  ix * header.Objectpixelsize + header.xorigin;
    y_loc[ii] =  iy * header.Objectpixelsize + header.yorigin;
    z_loc[ii] =  iz * header.Objectpixelsize + header.zorigin;

    // bookkeeping
    ix = ix +1;
    if (ix == header.nx) {
      ix = 0;
      iy = iy+1;
      if (iy == header.ny) {
        iy = 0;
        iz = iz+1;
      }
    }
  }
}



float DensityMap::calcRMS() {


  int  nvox = header.nx * header.ny * header.nz;
  float meanval = .0;
  float stdval = .0;

  for (int ii=0;ii<nvox;ii++) {
    meanval = meanval + data[ii];
    stdval = stdval + powf(data[ii], 2);
  }
  
  meanval /=  nvox;
  header.dmean = meanval;

  stdval = stdval - (powf(meanval, 2) * nvox);
  stdval = sqrt(stdval) / nvox;
  header.rms = stdval;

  return stdval;

}
