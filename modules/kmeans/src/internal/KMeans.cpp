/**
 *  \file KMeans.cpp
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

//----------------------------------------------------------------------
//      File:           KMeans.cc
//      Programmer:     David Mount
//      Last modified:  05/14/04
//      Description:    Shared utilities for k-means.
//----------------------------------------------------------------------
// Copyright (C) 2004-2005 David M. Mount and University of Maryland
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.  See the file Copyright.txt in the
// main directory.
//
// The University of Maryland and the authors make no representations
// about the suitability or fitness of this software for any purpose.
// It is provided "as is" without express or implied warranty.
//----------------------------------------------------------------------

#include <iostream>  // C++ I/O
#include <iomanip>

#include "IMP/kmeans/internal/KMeans.h"  // kmeans includes
#include "IMP/kmeans/internal/KCtree.h"  // kc tree
#include "IMP/kmeans/internal/KMrand.h"  // random number generators
#include <IMP/base/log_macros.h>

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE

//------------------------------------------------------------------------
//  Global data (shared by all files)
//      The following variables are used by all the procedures and are
//      initialized in kmInitGlobals().  kmInitTime is the CPU time
//      needed to initialize things before the first stage.
//------------------------------------------------------------------------

StatLev kmStatLev = SILENT;        // global stats output level
std::ostream* kmOut = &std::cout;  // standard output stream
std::ostream* kmErr = &std::cerr;  // output error stream
std::istream* kmIn = &std::cin;    // input stream

//----------------------------------------------------------------------
//  Output utilities
//----------------------------------------------------------------------

void kmPrintPt(  // print a point
    KMpoint p,   // the point
    int dim,     // the dimension
    bool fancy)  // print plain or fancy?
{
  if (fancy) *kmOut << "[ ";
  for (int i = 0; i < dim; i++) {
    *kmOut << std::setw(8) << p[i];
    if (i < dim - 1) *kmOut << " ";
  }
  if (fancy) *kmOut << " ]";
}

void kmLogPt(                      // print a point
    base::LogLevel ll, KMpoint p,  // the point
    int dim,                       // the dimension
    bool fancy)                    // print plain or fancy?
{
  if (fancy) IMP_LOG(ll, "[ ");
  for (int i = 0; i < dim; i++) {
    IMP_LOG(ll, std::setw(8) << p[i]);
    if (i < dim - 1) IMP_LOG(ll, " ");
  }
  if (fancy) IMP_LOG(ll, " ]");
}

void kmPrintPts(        // print points
    std::string title,  // name of point set
    KMpointArray pa,    // the point array
    int n,              // number of points
    int dim,            // the dimension
    bool fancy)         // print plain or fancy?
{
  *kmOut << "  (" << title << ":\n";
  for (int i = 0; i < n; i++) {
    *kmOut << "    " << i << "\t";
    kmPrintPt(pa[i], dim, fancy);
    *kmOut << "\n";
  }
  *kmOut << "  )" << std::endl;
}

void kmLogPts(                             // print points
    base::LogLevel ll, std::string title,  // name of point set
    KMpointArray pa,                       // the point array
    int n,                                 // number of points
    int dim,                               // the dimension
    bool fancy)                            // print plain or fancy?
{
  IMP_LOG(ll, "  (" << title << ":" << std::endl);
  for (int i = 0; i < n; i++) {
    IMP_LOG(ll, "    " << i << "\t");
    kmLogPt(ll, pa[i], dim, fancy);
    IMP_LOG(ll, std::endl);
  }
  *kmOut << "  )" << std::endl;
}

//------------------------------------------------------------------------
//  kmError - print error message
//        If KMerr is KMabort we also abort the program.
//------------------------------------------------------------------------

void kmError(                // error routine
    const std::string& msg,  // error message
    KMerr level)             // abort afterwards
{
  if (level == KMabort) {
    *kmErr << "kmlocal: ERROR------->" << msg << "<-------------ERROR"
           << std::endl;
    *kmOut << "kmlocal: ERROR------->" << msg << "<-------------ERROR"
           << std::endl;
    kmExit(1);
  } else {
    *kmErr << "kmlocal: WARNING----->" << msg << "<-------------WARNING"
           << std::endl;
    *kmOut << "kmlocal: WARNING----->" << msg << "<-------------WARNING"
           << std::endl;
  }
}

//------------------------------------------------------------------------
//  kmExit - exit from program
//        This is used because some Windows implementations create a
//      tempoarary window, which is removed immediately on exit.
//      This keeps until the user verifies termination.
//------------------------------------------------------------------------

void kmExit(int status)  // exit program
{
#ifdef WAIT_FOR_CONFIRM
  char ch;
  if (kmIn == &cin) {  // input from std in
    cerr << "Hit return to continue..." << std::endl;
    kmIn->get(ch);
  }
#endif
  exit(status);
}

IMPKMEANS_END_INTERNAL_NAMESPACE
