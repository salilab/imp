/**
 *  \file KMeansWrapper.cpp
 *  \brief
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
*/

#include "IMP/kmeans/KMeansWrapper.h"
#include <iostream>     // C++ I/O

IMPKMEANS_BEGIN_NAMESPACE


KMeansWrapper::KMeansWrapper(const std::string& fname, int dim, int max_nPts)
  :
    is_initialized_(false),
    is_executed_(false),
    term_(100, 0, 0, 0,    // run for 100 stages
          0.10,     // min consec RDL
          0.10,     // min accum RDL
          3,      // max run stages
          0.50,     // init. prob. of acceptance
          10,     // temp. run length
          0.95),      // temp. reduction factor
    dataPts_(dim, max_nPts),
    nPts_(0),
    pCenters_()
{
  using namespace std;

  // read / generate points
  ifstream infile;
  infile.open (fname.c_str(), ifstream::in);
  if(infile.good())
    {
      is_initialized_ = readDataPts(infile, max_nPts);
      infile.close();
    }
  if(!is_initialized_)
    {
      // generate points randomly
      // TODO remove this, throw exception if !ok,
      //      or put it in a different ctr
      nPts_ = max_nPts;
      int k = 4; // generates random data in k clusters
      internal::kmClusGaussPts
        (dataPts_.getPts(), nPts_, dataPts_.getDim(), k);
      is_initialized_ = true;
    }
}



  // executed the selected algorithms:
void
KMeansWrapper::execute(int k, KM_ALG_TYPE alg_type, int stages)
{
  using namespace std;

  // TODO: don't output anything ;
  std::cout << "Data Points:\n";     // echo data points
  for(int i = 0; i < nPts_; i++)
    printPt(std::cout, dataPts_[i]);

  bool is_exectued = false; // till proven otherwise
  term_.setAbsMaxTotStage(stages);   // set number of stages
  dataPts_.buildKcTree();      // build filtering structure
  pCenters_ =
    new internal::KMfilterCenters( k, dataPts_ );
  switch(alg_type)
    // TODO: don't output anything ;
    {
    case KM_LLOYDS:
      {
        // repeated Lloyd's
        std::cout << "\nExecuting Clustering Algorithm: Lloyd's\n";
        internal::KMlocalLloyds kmLloyds(*pCenters_, term_);
        *pCenters_ = kmLloyds.execute();
        printSummary(kmLloyds);
        break;
      }
    case KM_LOCAL_SWAP:
      {
        // Swap heuristic
        std::cout << "\nExecuting Clustering Algorithm: Swap\n";
        internal::KMlocalSwap kmSwap(*pCenters_, term_);
        *pCenters_ = kmSwap.execute();
        printSummary(kmSwap);
        break;
      }
    case KM_LOCAL_EZ_HYBRID:
      {
        // EZ-Hybrid heuristic
        std::cout << "\nExecuting Clustering Algorithm: EZ-Hybrid\n";
        internal::KMlocalEZ_Hybrid kmEZ_Hybrid(*pCenters_, term_);
        *pCenters_ = kmEZ_Hybrid.execute();
        printSummary(kmEZ_Hybrid);
        break;
      }
    case KM_HYBRID:
      {
        // Hybrid heuristic
        std::cout << "\nExecuting Clustering Algorithm: Hybrid\n";
        internal::KMlocalHybrid kmHybrid(*pCenters_, term_);
        *pCenters_ = kmHybrid.execute();
        printSummary(kmHybrid);
        break;
      }
    }
  is_executed_ = true;
}


  /** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i center number in range (1..k)
   */
std::vector<double>
KMeansWrapper::getCenter(int i) const
{
  // TODO: verify is_executed and 1 <= i <= k
  i--; // convert from (1..k) to [0..(k-1)]

  // Convert from KMcenter (aka double*)
  // to std::vector<double>
  int dim = pCenters_->getDim();
  const internal::KMcenter iCenter = (*pCenters_)[i];
  std::vector<double> retValue( dim );
  for(int j = 0; j < dim; j++)
    retValue[j] = iCenter[j];
  return retValue;
}


//----------------------------------------------------------------------
//  Reading/Printing utilities
//  readPt - read a point from input stream into data storage
//    at position i.  Returns false on error or EOF.
//  printPt - prints a points to output file
//----------------------------------------------------------------------
bool
KMeansWrapper::readPt(std::istream& in, internal::KMpoint& p)
{
  const int dim = dataPts_.getDim();
  for(int d = 0; d < dim; d++) {
    if(!(in >> p[d])) return false;
  }
  return true;
}

void
KMeansWrapper::printPt(std::ostream& out, const internal::KMpoint& p)
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
KMeansWrapper::readDataPts(std::istream &in, int max_nPts)
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
KMeansWrapper::printSummary
(const internal::KMlocal&    theAlg)   // the algorithm
{
  using namespace std;

  cout << "Number of stages: " << theAlg.getTotalStages() << endl;
  cout << "Average distortion: " <<
    pCenters_->getDist(false) / double(pCenters_->getNPts()) << endl;
  // print final center points
  cout << "(Final Center Points:\n";
  pCenters_->print();
  cout << ")\n";
  // get/print final cluster assignments
  internal::KMctrIdxArray closeCtr =
    new internal::KMctrIdx[dataPts_.getNPts()];
  double* sqDist = new double[dataPts_.getNPts()];
  pCenters_->getAssignments(closeCtr, sqDist);

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
