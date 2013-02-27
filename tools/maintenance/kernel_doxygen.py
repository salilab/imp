#!/usr/bin/env python

import IMP
import IMP.base
bad=["sys", "weakref", "weakref_proxy", "i_m_p", "kernel", "base"]

def ok(name):
    if name.startswith("_"):
        return False;
    if name.find("swigregister") != -1:
        return False;
    if name in bad:
        return False
    if name[0].upper() == name[0] and name.find("_") != -1:
        return False
    if name[0].lower() == name[0] and name.lower() != name.lower():
        return False
    if name in dir(IMP.base):
        return False
    if eval("type(IMP.%s) == type(IMP)"%name):
        return False
    return True;

def main():
    names= dir(IMP)
    print """/**
 *  \\file IMP/kernel/doxygen.h
 *  \\brief Import kernel classes into the IMP scope in docs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DOXYGEN_H
#define IMPKERNEL_DOXYGEN_H

   #include <IMP/kernel.h>

#ifdef IMP_DOXYGEN
/** \\namespace IMP
    All \imp code is in the \imp namespace. For convenience and backwards
    compatibility, the contents of IMP::kernel are hoisted into the \imp
    namespace too.
*/
namespace IMP {
"""
    for n in [x for x in names if ok(x)]:
        print "/** Import IMP::kernel::%s\n    in the IMP namespace.*/" %n
        if n[0].upper()==n[0]:
            print "typedef IMP::kernel::%s\n    %s;"%(n,n)
        else:
            print "using IMP::kernel::%s;"%n
    print "} // namespace"
    print "#endif //doxygen"
    print "#endif  /* IMPKERNEL_DOXYGEN_H */"

if __name__ == '__main__':
    main()
