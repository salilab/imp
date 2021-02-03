/**
 *  \file endian.cpp
 *  \brief Functions to deal with endianess
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
*/

#include <IMP/algebra/endian.h>

IMPALGEBRA_BEGIN_NAMESPACE

void reversed_read(void *dest, size_t size, size_t nitems, std::ifstream &f,
                   bool reverse) {
  if (!reverse) {
    char *ptr = (char *)dest;
    f.read(ptr, size * nitems);
  } else {
    char *ptr = (char *)dest;
    bool end = false;
    for (size_t n = 0; n < nitems; n++) {
      for (int i = size - 1; i >= 0; i--) {
        f.get(ptr[i]);
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
                    std::ofstream &f, bool reverse) {
  if (!reverse) {
    char *ptr = (char *)src;
    f.write(ptr, size * nitems);
  } else {
    char *ptr = (char *)src;
    bool end = false;
    for (size_t n = 0; n < nitems; n++) {
      for (int i = size - 1; i >= 0; i--) {
        f.put(*(ptr + i));
        if (f.bad()) {
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

bool get_is_little_endian() {
  const unsigned long ul = 0x00000001;
  return ((int)(*((unsigned char *)&ul))) != 0;
}

bool get_is_big_endian() {
  static const unsigned long ul = 0x01000000;
  return ((int)(*((unsigned char *)&ul))) != 0;
}

IMPALGEBRA_END_NAMESPACE
