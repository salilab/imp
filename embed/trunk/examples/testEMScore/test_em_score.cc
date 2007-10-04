#include "DensityMap.h"
#include "EMReaderWriter.h"
#include "SampledDensityMap.h"
#include "CoarseCC.h"
#include "DensityHeader.h"
#include <iostream>
#include <fstream>
using namespace std;


int main(int argc,char *argv[]) {

  char* em_filenameI, *em_filenameO;
  em_filenameI = argv[1];



  em_filenameO = argv[2];
  // load the EM density map
  DensityMap scene;
  EMReaderWriter erw;

  scene.Read(em_filenameI,erw);
  scene.setResolution(2.0);
  //init particles
  float x[3];
  float y[3];
  float z[3];
  float dx[3];
  float dy[3];
  float dz[3];
  float r[3];
  float w[3];

  x[0]=9.;
  y[0]=9.;
  z[0]=9.;


  x[1]=12.;
  y[1]=3.;
  z[1]=3.;

  x[2]=3.;
  y[2]=12.;
  z[2]=12.;

  r[0]=1.;
  r[1]=1.;
  r[2]=1.;

  w[0]=1.;
  w[1]=1.;
  w[2]=1.;

  SampledDensityMap model_map(scene.get_header());
  int ierr;
  model_map.ReSample(x,y,z,

		     3,	
		     r,w,	
		     ierr);
  cout << " after resmaple " << endl;

// write the map
  model_map.Write(em_filenameO,erw);

  CoarseCC cc;

  model_map.calcRMS();
  scene.calcRMS();
  float res =   cc.evaluate(scene,
	      model_map,
	      x,y,z,
	      dx,dy,dz,     
	      3,	
	      r,w,	
	      1.0,
	      false,
	      ierr);
 cout << " cc is : " << res << endl;

 // write the map
 // scene.Write(em_filenameO,erw);


  return 0;
}
