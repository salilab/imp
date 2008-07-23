#include "XplorReaderWriter.h"
#include <cstdlib>

void XplorReaderWriter::Read(const char *filename, float **data,
                            DensityHeader &header)
{
  std::ifstream XPLORstream(filename);
  //header
  XplorHeader xheader;
  ReadHeader(XPLORstream,xheader);
  xheader.GenerateCommonHeader(header);

  //data
  int size = xheader.extent[0]*xheader.extent[1]*xheader.extent[2];
  *data =  new float[size];
  if (*data == NULL) {
    std::ostringstream msg;
    msg << "XplorReader::Read can not allocated space for data - the "
     << "requested size: " << size * sizeof(float) << std::endl;
    throw EMBED_IOException(msg.str().c_str());
  }
  ReadMap(XPLORstream, *data, xheader);
  XPLORstream.close();
} 


int XplorReaderWriter::ReadHeader(std::ifstream & XPLORstream,
                                  XplorHeader &header)
{

  char line[300];
  int lineWidht = 300;

  // read the configuration values:
  for (int i=0;i<4;i++) {
      XPLORstream.getline(line,lineWidht);
    }
  char* lineBreaker;

  XPLORstream.getline(line,lineWidht);

  int div,mod,result;

  lineBreaker = strtok(line," ");
  for (int j=0;j<9;j++) {
      result = atoi(lineBreaker);
      div = (int)(floor(j/3.0));
      mod = j - 3 * div;
      if (mod == 0) {
        header.grid[div]=result;
      }
      else if (mod == 1) {
        header.orig[div]=result;
      }
      else // mod == 2
        header.top[div]=result;
      lineBreaker = strtok (NULL," ");
  }
  for(int j=0;j<3;j++) {
    header.extent[j]=header.top[j]-header.orig[j]+1;
  }

  ////////////////////
  // read the unit cell dimensions
  ////////////////////
  XPLORstream.getline(line,lineWidht);

  lineBreaker = strtok(line," ");

  float val;
  for (int j=0;j<6;j++) {
    val = atof(lineBreaker);
    div = (int)(floor(j/3.0));
    mod = j - 3 * div;
    if (div == 0) {
      header.cellsize[mod]=val;
    }
    else {  // div == 1 
      header.cellangle[mod]=val;
    }
    lineBreaker = strtok (NULL," ");
  }

  for(int j=0;j<3;j++) {
    header.voxelsize[j]=header.cellsize[j]/header.grid[j];
    header.translateGrid[j]=header.orig[j]*header.voxelsize[j];
  }

  //////////////
  // read the grid order
  //  we do not use this data. We use the user input for the grid order.
  //////////////
  XPLORstream.getline(line,lineWidht);

  return 0;
}


int XplorReaderWriter::ReadMap(std::ifstream &XPLORstream, float *data,
                               XplorHeader &header)
{

  //reading the map according to the grid order.
  char line[300];
  int lineWidht = 300;
  int x,y,z =0; //the step size in the three directions on the grid.
  x=0;y=0;z=0;
  bool status = false;

  // update orig and top according to the values in the map
  for(int i=0;i<3;i++) {
      header.orig[i]=9999;
      header.top[i]=-9999;
    }


  int counter,densNum;
  bool keep;
  char dens[12];
  float density;

  while (!XPLORstream.eof()) {
    XPLORstream.getline(line,lineWidht);
    if (!status) {// status = false means that we moved a section
      status = true;
    } else {
      // each line have strlen/12 density values.
      // We iterater through the line

      counter = 0;
      keep = true;
      densNum = strlen(line)/12;

      while ((counter <densNum) && keep) {
        strncpy(dens,line+(counter*12),12);
        counter++;
        density = atof(dens);   
        data[x + y*header.extent[0]
             + z*header.extent[0]*header.extent[1]] = density;
        if (x<header.orig[0])
          header.orig[0]=x;
        if (y<header.orig[1])
          header.orig[1]=y;
        if (z<header.orig[2])
          header.orig[2]=z;
        if (x>header.top[0])
          header.top[0]=x;
        if (y>header.top[1])
          header.top[1]=y;
        if (z>header.top[2])
          header.top[2]=z;

        x++;
        if (x >= header.extent[0]) {
          x=0;
          y++;
          if (y >= header.extent[1])
            keep = false;
        }

      } // while counter < densNum


      if (y >= header.extent[1]) {
        x=0;
        y=0;
        z++;
        status=false;
      }
    } //else
  }
  return 0;
}


void XplorReaderWriter::Write(const char *filename,const float *data,
                              const DensityHeader &header_ )
{
  XplorHeader header(header_);


  std::ofstream s(filename);

  s << std::endl << "       2" << std::endl << "REMARKS file name = ??? "
    << std::endl << "REMARKS Date ...... created by em lib " << std::endl;
  s.setf(std::ios::right, std::ios::adjustfield);
  s.width(8);
  for (int i =0;i<3;i++){
  s << std::setw(8)<<header.grid[i]<<
    std::setw(8) << floor(header.translateGrid[i]/header.voxelsize[i])<<
    std::setw(8) << floor(header.translateGrid[i]/header.voxelsize[i])
                    +header.extent[i]-1;
  }
  s << std::endl;
  for (int i =0;i<3;i++){
    s << std::scientific << std::setprecision(5) << std::setw(12)
      << header.cellsize[i];
  }
  for (int i =0;i<3;i++){
    s << std::scientific << std::setprecision(5) << std::setw(12)
      << header.cellangle[i];
  }
  s << std::endl << "XYZ" << std::endl; // Z is the slowest
  int counter = 0;
  for(int k=0;k<header.extent[2];k++) { 
    if (counter != 0){
      s << std::endl;
      counter=0;
    }

    s << std::setw(8) << k << std::endl;
    for(int j=0;j<  header.extent[1];j++) {
      for(int i=0;i< header.extent[0];i++) {
        s << std::scientific << std::setprecision(5) << std::setw(12)
          << data[i + j*header.extent[0] + k*header.extent[0]*header.extent[1]];
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
