#ifndef _DENSITYMAP_H
#define _DENSITYMAP_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@post.tau.ac.il)

  Copyright: SAMBA group, Tel-Aviv Univ. Israel, 2006.

  OVERVIEW TEXT

*/
// This should inheret from Map3D and basicly know how to read explor write 
// HelixFittingMap - this should store the results of the fitting ( which we will now do using 
//For the segmentation we should use the method we wrote with max. You have two days to finish this.

#include "Map3D.h"
#include "Vector3.h"
#include <iostream>
#include <iomanip>
#include "VoxelInd.h"

using std::istream;
using std::ios;


#define XYZ_ORDER 0  // X slowest
#define ZYX_ORDER 1
#define YXZ_ORDER 2
#define ZXY_ORDER 3 //  slowest


class DensityMap: public Map3D<float>
{
public:
  //---
  // Creates a density map from the templateP with the given resolution and spacing. 
  DensityMap(){}
  int ReadXplor(istream & XPLORstream,int _indOrder);
protected:

  int ReadMap(istream &XPLORstream);
  int ReadHeader(istream &XPLORstream);
};



#endif //_DENSITYMAP_H
