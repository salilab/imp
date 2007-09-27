#ifndef _XPLORREADER_H
#define _XPLORREADER_H

/*
  CLASS

  XplorReader

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT

*/
#include <math.h>
#include "MapReaderWriter.h"
#include "Map3DHeader.h"
#include <iostream>
#include <iomanip>
typedef float real;
using namespace std;  


// TODO: I think this class should be stateless !! 


class XplorReaderWriter : public MapReaderWriter {
public:
  int Read(ifstream &file);
  void Write(ostream& s) const;
protected:
  int ReadHeader(ifstream & XPLORstream);
  int  ReadMap(ifstream &XPLORstream);

};


#endif //_XPLORREADER_H
