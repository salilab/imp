/*
 *  Base_Types.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BASE_TYPES_H
#define __IMP_BASE_TYPES_H

#include <string>

#include "IMP_config.h"

namespace IMP
{

// basic types used by IMP
typedef float Float;
typedef int Int;
typedef std::string String;

// if I don't do this as a template, I eliminate the swig errors
class IMPDLLEXPORT FloatIndex
{
public:
  FloatIndex(int idx = 0) {
    index_ = idx;
  }

  ~FloatIndex(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

class IMPDLLEXPORT IntIndex
{
public:
  IntIndex(int idx = 0) {
    index_ = idx;
  }

  ~IntIndex(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

class IMPDLLEXPORT StringIndex
{
public:
  StringIndex(int idx = 0) {
    index_ = idx;
  }

  ~StringIndex(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

// indexes for retrieving particle information
template <class T>
class IMPDLLEXPORT DataIndex
{
public:
  DataIndex(int idx = 0) {
    index_ = idx;
  }

  ~DataIndex(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

// typedefs for the particle variable and attribute indexes
// typedef DataIndex<Float> FloatIndex;
// typedef DataIndex<Int> IntIndex;
// typedef DataIndex<String> StringIndex;

} // namespace IMP

#endif  /* __IMP_BASE_TYPES_H */
