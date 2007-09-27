#include "DensityMap.h"
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
  scene.ReadXplor(mapFile,1);
  mapFile.close();
		   
  // write the map
  ofstream outFile(em_filenameO);
  scene.PrintXplorFormat(outFile);
  outFile.close();

		   

  return 0;
}
