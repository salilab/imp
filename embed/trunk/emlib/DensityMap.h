#ifndef _DENSITYMAP_H
#define _DENSITYMAP_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@post.tau.ac.il)

  OVERVIEW TEXT

*/




#include "Map3D.h"
#include "Map3DHeader.h"
#include "Vector3.h"
#include <iostream>
#include <iomanip>
#include "MapReaderWriter.h"
using std::istream;
using std::ios;

typedef float real;

class DensityMap: public Map3D<float>
{
public:
  //---
  // Creates a density map from the templateP with the given resolution and spacing. 
  DensityMap(){}
  void Read(ifstream &file, MapReaderWriter &reader);
  void Write(ostream &file,MapReaderWriter &writer);
protected:
};



#endif //_DENSITYMAP_H
