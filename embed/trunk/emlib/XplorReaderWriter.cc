#include "XplorReaderWriter.h"
int XplorReaderWriter::Read(ifstream & XPLORstream) {
  ReadHeader(XPLORstream);

  int allocation_size = header.get_extent(0)*header.get_extent(1)*header.get_extent(2)*sizeof(real);
  data =  new real[allocation_size];
  if (data == NULL) {
    cout << "XplorReader::Read can not allocated space for data - the requested size: " << allocation_size << endl;
    return -1;
  }
  ReadMap(XPLORstream);
  return 0;
}


int XplorReaderWriter::ReadHeader(ifstream & XPLORstream)
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
	header.set_grid(div,result);
      }
      else if (mod == 1) {
	header.set_orig(div,result);
      }
      else // mod == 2
	header.set_top(div,result);
      lineBreaker = strtok (NULL," ");
  }
  for(int j=0;j<3;j++) {
    header.set_extent(j,header.get_top(j)-header.get_orig(j)+1);
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
      header.set_cellsize(mod,val);
    }
    else // div == 1 {
      header.set_cellangle(mod,val);
  }
   
  lineBreaker = strtok (NULL," ");

  for(int j=0;j<3;j++)
    header.set_voxelsize(j,header.get_cellsize(j)/header.get_grid(j));

//TODO - ???????

//   translateGrid.updateX(orig[0]*spacing[0]);
//   translateGrid.updateY(orig[1]*spacing[1]);
//   translateGrid.updateZ(orig[2]*spacing[2]);


  //////////////
  // read the grid order
  //  we do not use this data. We use the user input for the grid order.
  //////////////
  XPLORstream.getline(line,lineWidht);
    
  return 0;
}


int  XplorReaderWriter::ReadMap(ifstream &XPLORstream)
{

  //reading the map according to the grid order.
  char line[300];
  int lineWidht = 300;
  int x,y,z =0; //the step size in the three directions on the grid.
  x=0;y=0;z=0;
  bool status = false;

  // update orig and top according to the values in the map
  for(int i=0;i<3;i++) {
      header.set_orig(i,9999);
      header.set_top(i,-9999);
    }

  
  int counter,densNum;
  bool keep;
  char dens[12];
  float density;

  while (!XPLORstream.eof()) {
    XPLORstream.getline(line,lineWidht);
    if (!status) // status = false means that we moved a section
      {
	status = true;
      }
    else {
      // each line have strlen/12 density values.
      // We iterater through the line
      
      counter = 0;
      keep = true;
      densNum = strlen(line)/12;
      
      while ((counter <densNum) && keep) {
	strncpy(dens,line+(counter*12),12);
	counter ++;
	density = atof(dens);   
	data[x+y*header.get_extent(0)+z*header.get_extent(0)*header.get_extent(1)] = density;
	if (x<header.get_orig(0))
	  header.set_orig(0,x);
	if (y<header.get_orig(1))
	  header.set_orig(1,y);
	if (z<header.get_orig(2))
	  header.set_orig(2,z);
	if (x>header.get_top(0))
	  header.set_top(0,x);
	if (y>header.get_top(1))
	  header.set_top(1,y);
	if (z>header.get_top(2))
	  header.set_top(2,z);

	x++;
	if (x >= header.get_extent(0) ) {
	  x=0;
	  y++;
	  if ( y>= header.get_extent(1))
	    keep = false;
	}
		
      } // while counter < densNum
	 
	     
      if (y >= header.get_extent(1)) {
	x=0;
	y=0;
	z++;
	
	status=false;
      }
    } //else
  }
  return 0;
}


void XplorReaderWriter::Write(ostream& s) const {

  s <<endl << "       2"<<endl << "REMARKS file name = ??? " << endl << "REMARKS Date ...... created by em lib " << endl;
  s.setf(ios::right, ios::adjustfield);
  s.width(8);
  float translateGrid[3];
  for (int i =0;i<3;i++){
  s << setw(8)<<header.get_grid(i)<<
    setw(8)<<floor(translateGrid[i]/header.get_voxelsize(i))+1<<
    setw(8)<<floor(translateGrid[i]/header.get_voxelsize(i))+1+header.get_extent(i)-1;
  }
  s<<endl;
  for (int i =0;i<3;i++){
    s<< scientific << setprecision(5)<<setw(12)<<header.get_cellsize(i);
  }
  for (int i =0;i<3;i++){
    s<< scientific << setprecision(5)<<setw(12)<<header.get_cellangle(i);
  }
  s << "XYZ" << endl; // Z is the slowest
  int counter = 0;
  for(int k=0;k<header.get_extent(2);k++) { 
    if (counter != 0){
      s << endl;
      counter=0;
    }
    s<<setw(8)<<k<<endl;
    for(int j=0;j<  header.get_extent(1);j++) {
      for(int i=0;i< header.get_extent(0);i++) {
	s<< scientific << setprecision(5)<<setw(12)<<data[i+j*header.get_extent(0)+k*header.get_extent(0)*header.get_extent(1)];
	counter++;
	if (counter == 6) {
	  counter = 0;
	  s << endl;
	}
      }
    }
  }
  s<<endl << "  -9999" << endl;
}
