#ifndef _MAPREADERWRITER_H
#define _MAPREADERWRITER_H

/*
  CLASS
  MapReader

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT
  an abstract class for reading a map
*/



#include "Map3DHeader.h"
#include <iostream>
#include <fstream>
using namespace std;
typedef float real;
class MapReaderWriter {
public:
//   friend  ostream& operator<<(ostream& s, const MapReaderWriter &m) { // write the map
//     m.Write(s);
//   }
//   friend istream& operator>>(istream& s, MapReaderWriter &m) { //read map
//     m.Read(s);
//   }
 
  virtual int Read(ifstream &file) {} ;
  virtual void Write(ostream& s) const {};


  real* get_data() const { return data;}
  const Map3DHeader& get_header()const {return header;}

  
  
  //protected:
  real *data;
  Map3DHeader header;

};


#endif //_MAPREADERWRITER_H
