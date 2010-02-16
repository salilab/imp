/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/file.h>


IMP_BEGIN_NAMESPACE

TextOutput::TextOutput(const char *c):
  out_(new internal::LazyFileStorage<std::ostream, std::ofstream>(c)){}
TextOutput::TextOutput(std::string c):
  out_(new internal::LazyFileStorage<std::ostream, std::ofstream>(c)){}
TextOutput::TextOutput(std::ostream &in):
  out_(new internal::StreamStorage<std::ostream>(in)){}
TextOutput::TextOutput(TextProxy<std::ostream> out):
  out_(new internal::OwnedStreamStorage<std::ostream>(*out.str_,
                                                      out.ptr_)){}
TextOutput::TextOutput(int) {
  IMP_THROW("Wrong argument type", IOException);
}
TextOutput::TextOutput(double) {
  IMP_THROW("Wrong argument type", IOException);
}

TextInput::TextInput(const char *c):
  in_(new internal::FileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::string c):
  in_(new internal::FileStorage<std::istream, std::ifstream>(c)){}
TextInput::TextInput(std::istream &in):
  in_(new internal::StreamStorage<std::istream>(in)){}
TextInput::TextInput(TextProxy<std::istream> out):
  in_(new internal::OwnedStreamStorage<std::istream>(*out.str_,
                                                     out.ptr_)){}
TextInput::TextInput(int) {
 IMP_THROW("Wrong argument type", IOException);
}
TextInput::TextInput(double) {
  IMP_THROW("Wrong argument type", IOException);
}




namespace {
  void testf(TextInput in) {
  }
  void testcall() {
    testf(std::string("filename"));

    std::ifstream inf("infile");
    testf(inf);
  }
}

IMP_END_NAMESPACE
