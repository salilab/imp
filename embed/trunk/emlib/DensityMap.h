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
#include "DensityHeader.h"
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

  // header inspection 
  const DensityHeader &get_header()const {return header;}

protected:
  DensityHeader header;
};



#endif //_DENSITYMAP_H
