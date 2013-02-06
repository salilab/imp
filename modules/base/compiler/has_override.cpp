/**
 *  \file nothing.cpp
 *  \brief nothing
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

struct Base {
    virtual void some_func(float);
};

struct Derived : Base {
  virtual void some_func(float) override;
};

int main()
{
  auto a=6;
  return 0;
}
