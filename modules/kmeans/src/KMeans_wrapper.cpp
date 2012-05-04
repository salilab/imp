/**
 *  \file KMeans_wrapper.cpp
 *  \brief
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
*/

#include "IMP/kmeans/KMeans_wrapper.h"
#include <iostream>     // C++ I/O

IMPKMEANS_BEGIN_NAMESPACE


KMeans_wrapper::KMeans_wrapper(const std::string& fname, int dim, int max_nPts)
  : term_(100, 0, 0, 0,    // run for 100 stages
          0.10,     // min consec RDL
          0.10,     // min accum RDL
          3,      // max run stages
          0.50,     // init. prob. of acceptance
          10,     // temp. run length
          0.95),      // temp. reduction factor
    dataPts_(dim, max_nPts),
    nPts_(0)
{
  using namespace std;

  // read / generate points
  good_ = false; // until succesful
  ifstream infile;
  infile.open (fname.c_str(), ifstream::in);
  if(infile.good())
    {
      good_ = readDataPts(infile, max_nPts);
      infile.close();
    }
  if(!good_)
    {
      // generate points randomly
      // TODO remove this, throw exception if !ok,
      //      or put it in a different ctr
      nPts_ = max_nPts;
      int k = 4; // generates random data in k clusters
      internal::kmClusGaussPts
        (dataPts_.getPts(), nPts_, dataPts_.getDim(), k);
      good_ = true;
    }
}



void
KMeans_wrapper::execute(int k, int stages)
{
  using namespace std;

  term_.setAbsMaxTotStage(stages);   // set number of stages

  std::cout << "Data Points:\n";     // echo data points
  for(int i = 0; i < nPts_; i++)
    printPt(std::cout, dataPts_[i]);

  dataPts_.buildKcTree();      // build filtering structure

  internal::KMfilterCenters centers(k, dataPts_);   // allocate centers

  // run each of the algorithms
  std::cout << "\nExecuting Clustering Algorithm: Lloyd's\n";
  internal::KMlocalLloyds kmLloyds(centers, term_);   // repeated Lloyd's
  centers = kmLloyds.execute();      // execute
  printSummary(kmLloyds, centers);  // print summary

  std::cout << "\nExecuting Clustering Algorithm: Swap\n";
  internal::KMlocalSwap kmSwap(centers, term_);   // Swap heuristic
  centers = kmSwap.execute();
  printSummary(kmSwap, centers);

  std::cout << "\nExecuting Clustering Algorithm: EZ-Hybrid\n";
  internal::KMlocalEZ_Hybrid kmEZ_Hybrid(centers, term_); // EZ-Hybrid heuristic
  centers = kmEZ_Hybrid.execute();
  printSummary(kmEZ_Hybrid, centers);

  std::cout << "\nExecuting Clustering Algorithm: Hybrid\n";
  internal::KMlocalHybrid kmHybrid(centers, term_);   // Hybrid heuristic
  centers = kmHybrid.execute();
  printSummary(kmHybrid, centers);
}



//----------------------------------------------------------------------
//  Reading/Printing utilities
//  readPt - read a point from input stream into data storage
//    at position i.  Returns false on error or EOF.
//  printPt - prints a points to output file
//----------------------------------------------------------------------
bool
KMeans_wrapper::readPt(std::istream& in, internal::KMpoint& p)
{
  const int dim = dataPts_.getDim();
  for(int d = 0; d < dim; d++) {
    if(!(in >> p[d])) return false;
  }
  return true;
}

void
KMeans_wrapper::printPt(std::ostream& out, const internal::KMpoint& p)
{
  const int dim = dataPts_.getDim();
  out << "(" << p[0];
  for(int i = 1; i < dim; i++) {
    out << ", " << p[i];
  }
  out << ")" << std::endl;
}

// reads points from a file
// returns true if successful
bool
KMeans_wrapper::readDataPts(std::istream &in, int max_nPts)
{
  int dim = dataPts_.getDim();
  if(!in.good())
    return false;

  nPts_ = 0;
  dataPts_.resize(dim, max_nPts);
  while(nPts_ < max_nPts && // within array size
        readPt(in, dataPts_[nPts_]))
    {
      nPts_++;
    }
  dataPts_.setNPts(nPts_); // actual number of points read

  return (nPts_ >= 1); // at least two points
}


//------------------------------------------------------------------------
//  Print summary of execution
//------------------------------------------------------------------------
void
KMeans_wrapper::printSummary
(const internal::KMlocal&    theAlg,   // the algorithm
 internal::KMfilterCenters&    centers)   // the centers
{
  using namespace std;

  cout << "Number of stages: " << theAlg.getTotalStages() << endl;
  cout << "Average distortion: " <<
    centers.getDist(false) / double(centers.getNPts()) << endl;
  // print final center points
  cout << "(Final Center Points:\n";
  centers.print();
  cout << ")\n";
  // get/print final cluster assignments
  internal::KMctrIdxArray closeCtr =
    new internal::KMctrIdx[dataPts_.getNPts()];
  double* sqDist = new double[dataPts_.getNPts()];
  centers.getAssignments(closeCtr, sqDist);

  cout  << "(Cluster assignments:" << endl
        << "    Point  Center  Squared Dist" << endl
        << "    -----  ------  ------------" << endl;
  for(int i = 0; i < dataPts_.getNPts(); i++) {
    cout  << "   " << setw(5) << i
          << "   " << setw(5) << closeCtr[i]
          << "   " << setw(10) << sqDist[i]
          << endl;
  }
  cout << ")" << endl;
  delete [] closeCtr;
  delete [] sqDist;
}




IMPKMEANS_END_NAMESPACE
