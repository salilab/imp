#include "DensityMap.h"
#include "XplorReaderWriter.h"
#include <iostream>
#include <fstream>
using namespace std;


int main(int argc,char *argv[]) {

  const char* em_filenameI, *em_filenameO;
  em_filenameI = argv[1];
  em_filenameO = argv[2];
  // load the EM density map
  DensityMap scene;
  ifstream mapFile(em_filenameI);
  XplorReaderWriter xrw;

  scene.Read(mapFile,xrw);

  mapFile.close();
		   
  // write the map
  ofstream outFile(em_filenameO);

  scene.Write(outFile,xrw);

  outFile.close();

		   

  return 0;
}
