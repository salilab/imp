/**
 *  \file nothing.cpp
 *  \brief nothing
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

int main(int , char **) {
  union {
    char array[4];
    int integer;
  } TestUnion;
  TestUnion.array[0] = 'a';
  TestUnion.array[1] = 'b';
  TestUnion.array[2] = 'c';
  TestUnion.array[3] = 'd';
  if (TestUnion.integer == 0x64636261) {
    return 0;
  } else {
    return 0;
  }
}
