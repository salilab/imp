#ifndef _MRCREADERWRITER_H
#define _MRCREADERWRITER_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Javi
  OVERVIEW TEXT

*/


#include "MapReaderWriter.h"

class MRCReaderWriter : public MapReaderWriter {
public:
  int Read(ifstream &file);
  void Write(ostream& s) const;


}


#endif //_MRCREADERWRITER_H
