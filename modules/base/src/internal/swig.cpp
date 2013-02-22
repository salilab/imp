/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base/internal/swig.h>
#include <IMP/base/log_macros.h>
IMPBASE_BEGIN_INTERNAL_NAMESPACE
namespace {
void test_log_1() {
  IMP_FUNCTION_LOG;
  IMP_LOG_TERSE( "Hi" << std::endl);
}
void test_log_0() {
  IMP_FUNCTION_LOG;
  test_log_1();
}
}

void _test_log() {
  IMP_FUNCTION_LOG;
  test_log_0();
}


std::string _test_ifile(base::TextInput a) {
  std::string read;
  while (true) {
    std::string cur;
    a.get_stream() >> cur;
    if (!a) break;
    read= read+cur;
  }
  std::cout << read;
  return read;
}
std::string _test_ofile(base::TextOutput a) {
  static_cast<std::ostream &>(a) << "hi\n"
                                 << " there, how are things"<< std::endl;
  return "hi\n";
}

DerivativePair
_pass_pair(const DerivativePair &p) {
  std::cout << p.first << " " << p.second << std::endl;
  return p;
}

std::string _test_ifile_overloaded(base::TextInput a, std::string) {
  return _test_ifile(a);
}
std::string _test_ofile_overloaded(base::TextOutput a, std::string) {
  return _test_ofile(a);
}
std::string _test_ifile_overloaded(base::TextInput a, int) {
  return _test_ifile(a);
}
std::string _test_ofile_overloaded(base::TextOutput a, int) {
  return _test_ofile(a);
}

Floats _pass_floats(const Floats& in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}
Ints _pass_ints( Ints in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}
const Strings& _pass_strings(const Strings& in) {
  std::cout << in << std::endl;
  return in;
}
IntsList _pass_ints_list(const IntsList &in) {
  std::cout << "IntsList of length " << in.size();
  return in;
}
IntsLists _pass_ints_lists(const IntsLists &in) {
  std::cout << "IntsLists of length " << in.size();
  return in;
}
std::pair<double, double>
_pass_plain_pair(std::pair<double, double> p) {
  std::cout << p.first << " " << p.second << std::endl;
  return p;
}
int _test_intranges(const IntRanges &ips) {
  return ips.size();
}


IntRange _test_intrange(const IntRange &ips) {
  return ips;
}

IntRange _test_intrange() {
  return IntRange(-1,-1);
}

Strings _pass_overloaded_strings(const Strings &a,
                                 int) {
  return a;
}
Strings _pass_overloaded_strings(const Strings &a) {
  return a;
}

IMPBASE_END_INTERNAL_NAMESPACE
