/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/file.h>


IMP_BEGIN_NAMESPACE

TextOutput::TextOutput(const char *c): P(std::string(c)){}
TextOutput::TextOutput(std::string c): P(c){}
TextOutput::TextOutput(std::ostream &in):P(in){}
TextOutput::TextOutput(TextProxy<std::ostream> out): P(out){}
TextOutput::TextOutput(int) {
  IMP_THROW("Wrong argument type", IOException);
}
TextOutput::TextOutput(double) {
  IMP_THROW("Wrong argument type", IOException);
}

TextInput::TextInput(const char *c): P(std::string(c)){}
TextInput::TextInput(std::string c): P(c){}
TextInput::TextInput(std::istream &in):P(in){}
TextInput::TextInput(TextProxy<std::istream> out): P(out){}
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
