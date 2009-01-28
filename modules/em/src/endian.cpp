/**
 *  \file endian.cpp
 *  \brief Calculate score based on fit to EM map.
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#include <IMP/em/endian.h>

IMPEM_BEGIN_NAMESPACE

void reversed_read(void *dest, size_t size, size_t nitems, std::ifstream& f,
                     bool reverse)
{
  if (!reverse) {
    char *ptr = (char *)dest;
    f.read(ptr,size*nitems);
  } else {
    char *ptr = (char *)dest;
    bool end = false;
    for (size_t n = 0; n < nitems; n++) {
      for (int i = size - 1; i >= 0; i--) {
        f.get(ptr + i,1);
        if (f.eof() || f.bad()) {
          end = true;
          break;
        }
      }
      if (end) {
        break;
      }
      ptr += size;
    }
  }
}

void reversed_write(const void *src, size_t size, size_t nitems,
                      std::ofstream& f,bool reverse)
{
  if (!reverse) {
    char *ptr = (char *)src;
    f.write(ptr,size*nitems);
  }
  else {
    char *ptr = (char *)src;
    bool end = false;
    for (size_t n = 0; n < nitems; n++) {
      for (int i = size - 1; i >= 0; i--) {
        f.put(*(ptr + i));
        if(f.bad()) {
          end = true;
          break;
        }
      }
      if (end) {
        break;
      }
      ptr += size;
    }
  }
}

void byte_swap(unsigned char * b, int n)
{
  register int i = 0;
  register int j = n - 1;
  while (i < j) {
    std::swap(b[i], b[j]);
    i++, j--;
  }
}

bool is_little_endian(void)
{
  static const unsigned long ul = 0x00000001;
  return ((int)(*((unsigned char *) &ul))) != 0;
}


bool is_big_endian(void)
{
  static const unsigned long ul = 0x01000000;
  return ((int)(*((unsigned char *) &ul))) != 0;
}

IMPEM_END_NAMESPACE
