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
TextOutput::TextOutput(int) {}
TextOutput::TextOutput(double) {}

TextInput::TextInput(const char *c): P(std::string(c)){}
TextInput::TextInput(std::string c): P(c){}
TextInput::TextInput(std::istream &in):P(in){}
TextInput::TextInput(TextProxy<std::istream> out): P(out){}
TextInput::TextInput(int) {}
TextInput::TextInput(double) {}




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
