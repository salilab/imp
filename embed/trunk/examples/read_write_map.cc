#include "DensityMap.h"
#include "EMReaderWriter.h"
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
  cout <<" after read"<<endl;
  // write the map
  scene.Write(em_filenameO,erw);


  return 0;
}
