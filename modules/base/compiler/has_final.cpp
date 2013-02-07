/**
 *  \file nothing.cpp
 *  \brief nothing
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
struct Base {
    virtual void f() = 0;
};

struct Derived : Base {
  virtual void f() final {}
};

int main()
{
  return 0;
}
