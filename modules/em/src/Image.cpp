/**
 *  \file Image.cpp
 *  \brief Management of IMP images for Electron Microscopy
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em/Image.h"
#include "IMP/Pointer.h"

IMPEM_BEGIN_NAMESPACE

Images read_images(Strings names,ImageReaderWriter<double> &rw) {
  unsigned long size = names.size();
  Images v(size);
  for(unsigned long i=0;i<size;++i) {
    IMP_NEW(Image, img, ());
    img->read_from_floats(names[i],rw);
    v.set(i,img); //  v[i] = img; does not work
  }
  return v;
}


void save_images(Images images, Strings names,
                      ImageReaderWriter<double> &rw) {
  for(unsigned long i=0;i<images.size();++i) {
    images[i]->write_to_floats(names[i],rw);
  }
}

IMPEM_END_NAMESPACE
