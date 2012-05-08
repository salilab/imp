/**
 *  \file KMeansWrapper.cpp
 *  \brief
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
*/

#include "IMP/kmeans/KMeansWrapper.h"
#include <iostream>     // C++ I/O

IMPKMEANS_BEGIN_NAMESPACE

/***********************  Constructors  **************************/

/**
   Initialize the KMeansWrapper object with data from fname_data,
   assuming input data of dimension dim

   @param[in] fname_data Input filename. Input is assumed to be textual,
   whitespace separated
   @param[in] dim Dimension of points
   @param[in] max_nPts Maximal number of points to be read from file
*/
KMeansWrapper::KMeansWrapper
(const std::string& fname,
 int dim,
 unsigned int max_nPts)
  : is_executed_(false),
    pKMDataPts_(),
    is_KM_data_synced_(false),
    pCenters_(),
    terminationConditions_
    (100, 0, 0, 0,    // run for 100 stages
     0.10,     // min consec RDL
     0.10,     // min accum RDL
     3,      // max run stages
     0.50,     // init. prob. of acceptance
     10,     // temp. run length
     0.95)      // temp. reduction factor
{
  using namespace std;

   // read / generate points
  ifstream infile;
  infile.open(fname.c_str(), ifstream::in);
  if(infile.good())
    {
      readDataPtsFromStream(infile, dim, max_nPts);
      infile.close();
    }
  // TODO: Warn if no data was read or throw error if bad file?
  //       Right now, the indication of bad file would be that
  //       STLDataPts_ is empty
}


/** Empty constructor for all default initializations -
    object data is not considered initialized after this call
 */
KMeansWrapper::KMeansWrapper()
      : is_executed_(false),
        pKMDataPts_(),
        is_KM_data_synced_(false),
        pCenters_(),
        terminationConditions_
        (100, 0, 0, 0,    // run for 100 stages
         0.10,     // min consec RDL
         0.10,     // min accum RDL
         3,      // max run stages
         0.50,     // init. prob. of acceptance
         10,     // temp. run length
         0.95)      // temp. reduction factor
{
}




/*********************** Public methods **************************/

// execute the selected algorithms:
void
KMeansWrapper::execute(int k, KM_ALG_TYPE alg_type, int stages)
{
  using namespace std;

  assert(STLDataPts_.size() >= k ); // TODO: exception? warning?

  // TODO: remove debug outputting
  std::cout << "Data Points:\n";     // echo data points
  for(unsigned int i = 0; i < STLDataPts_.size(); i++)
    printPtToStream(std::cout, STLDataPts_[i]);

  // synchronize STL points to wrapped internal::KMdata points
  // allocate points array
  this->syncKMDataPtsFromSTL();
  terminationConditions_.setAbsMaxTotStage(stages);   // set number of stages
  // TODO: annoying that we must remember to call buildKcTree explicitly
  //       - sounds like bug-prone voodoo
  pKMDataPts_->buildKcTree();      // build filtering structure
  pCenters_ =
    new internal::KMfilterCenters( k, *pKMDataPts_ );
  switch(alg_type)
    // TODO: don't output anything ;
    {
    case KM_LLOYDS:
      {
        // repeated Lloyd's
        std::cout << "\nExecuting Clustering Algorithm: Lloyd's\n";
        internal::KMlocalLloyds kmLloyds
          (*pCenters_, terminationConditions_);
        *pCenters_ = kmLloyds.execute();
        is_executed_ = true;
        printSummary(kmLloyds);
        break;
      }
    case KM_LOCAL_SWAP:
      {
        // Swap heuristic
        std::cout << "\nExecuting Clustering Algorithm: Swap\n";
        internal::KMlocalSwap kmSwap
          (*pCenters_, terminationConditions_);
        *pCenters_ = kmSwap.execute();
        is_executed_ = true;
        printSummary(kmSwap);
        break;
      }
    case KM_LOCAL_EZ_HYBRID:
      {
        // EZ-Hybrid heuristic
        std::cout << "\nExecuting Clustering Algorithm: EZ-Hybrid\n";
        internal::KMlocalEZ_Hybrid kmEZ_Hybrid
          (*pCenters_, terminationConditions_);
        *pCenters_ = kmEZ_Hybrid.execute();
        is_executed_ = true;
        printSummary(kmEZ_Hybrid);
        break;
      }
    case KM_HYBRID:
      {
        // Hybrid heuristic
        std::cout << "\nExecuting Clustering Algorithm: Hybrid\n";
        internal::KMlocalHybrid kmHybrid
          (*pCenters_, terminationConditions_);
        *pCenters_ = kmHybrid.execute();
          is_executed_ = true;
        printSummary(kmHybrid);
        break;
      }
    }
}

/**
   Add a data point for the next clustering.
*/
void
KMeansWrapper::addDataPt(const std::vector<double>& p)
{
  is_executed_ = false;
  is_KM_data_synced_ = false;
  // verify same dimension as existing points
  if(STLDataPts_.size() > 0){
    assert(STLDataPts_[0].size() == p.size());
  }
  STLDataPts_.push_back(p);
}

/**
   Clears all data in object
*/
void
KMeansWrapper::clearData()
{
  is_executed_ = false;
  is_KM_data_synced_ = false;
  STLDataPts_.clear();
}



/** Returns the i'th point in the dataset

    @param[in] i Center number in range (0,...,nPts-1)
*/
const std::vector<double>&
KMeansWrapper::getDataPoint(unsigned int i) const
{
  assert(i < STLDataPts_.size()); // TODO: exception?
  return STLDataPts_[i];
}


/** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i center number in range (0,...,k-1)
   */
std::vector<double>
KMeansWrapper::getCenter(unsigned int i) const
{
  // TODO: exception instead of assertion?
  assert(is_executed_ && i < pCenters_->getNPts());

  // Convert from KMCenter (aka double*)
  // to std::vector<double>
  int dim = pCenters_->getDim();
  const internal::KMcenter& iCenter = (*pCenters_)[i];
  std::vector<double> retValue( dim );
  for(int j = 0; j < dim; j++)
    retValue[j] = iCenter[j];
  return retValue;
}


/*************************** Private Methods **************************/


/** Updates the wrapped data pts structure from the internal 2D STL vector
    array.
    This method invalidates any prior information about clustering results
*/
void
KMeansWrapper::syncKMDataPtsFromSTL()
{
  assert(STLDataPts_.size() > 0); // exception?
  if(is_KM_data_synced_)
    return; // already synced
  is_executed_ = false; // invalidate any preexisting clustering info
  unsigned int nPts = STLDataPts_.size();
  unsigned int dim = STLDataPts_[0].size();
  // copy points to internal data structure
  pKMDataPts_ = new internal::KMdata(dim, nPts);
  for(unsigned int i = 0; i < nPts; i++){
    for(unsigned int j = 0; j < dim; j++){
      (*pKMDataPts_)[i][j] = STLDataPts_[i][j];
    }
  }
  is_KM_data_synced_ = true;
}


/**
   Read a point (using dimension from dataPts_)

   @param[in]  in    input stream to read from
   @param[out] p     output point
   @param[in]  dim   dimension of each data point

   @return false on error or EOF.
*/
bool
KMeansWrapper::readPtFromStream
(std::istream& in,
 std::vector<double>& p,
 unsigned int dim)
{
  for(int d = 0; d < dim; d++) {
    if(!(in >> p[d])) return false;
  }
  return true;
}


// reads points from a stream
void
KMeansWrapper::readDataPtsFromStream
(std::istream &in,
 unsigned int dim,
 unsigned int max_nPts)
{
  is_executed_ = false; // invalidate
  is_KM_data_synced_ = false; // pKMDataPts_ now out of sync with STLData_
  assert(in.good()); // TODO: exception?
  STLDataPts_.clear();
  for(unsigned int i = 0; i < max_nPts; i++)
    {
      std::vector<double> newPoint(dim);
      bool ok = readPtFromStream(in, newPoint, dim);
      if(ok)
        STLDataPts_.push_back(newPoint);
      else
        break;
    }
}


/**
   print a point

   @param[in] out stream for printing the point
   @param[in] p   the point
*/
void
KMeansWrapper::printPtToStream(std::ostream& out, const std::vector<double>& p)
{
  const int dim = p.size();
  if(dim == 0){
    out << "()" << std::endl;
    return;
  }
  out << "(" << p[0];
  for(int i = 1 ; i < dim; i++) {

    out << ", " << p[i];
  }
  out << ")" << std::endl;
}

//------------------------------------------------------------------------
//  Print summary of execution
//------------------------------------------------------------------------
void
KMeansWrapper::printSummary
(const internal::KMlocal&    theAlg)   // the algorithm
{
  using namespace std;

  assert(is_executed_);
  cout << "Number of stages: " << theAlg.getTotalStages() << endl;
  cout << "Average distortion: " <<
    pCenters_->getDist(false) / double(pCenters_->getNPts()) << endl;
  // print final center points
  cout << "(Final Center Points:\n";
  pCenters_->print();
  cout << ")\n";
  // get/print final cluster assignments
  internal::KMctrIdxArray closeCtr =
    new internal::KMctrIdx[pKMDataPts_->getNPts()];
  double* sqDist = new double[pKMDataPts_->getNPts()];
  pCenters_->getAssignments(closeCtr, sqDist);

  cout  << "(Cluster assignments:" << endl
        << "    Point  Center  Squared Dist" << endl
        << "    -----  ------  ------------" << endl;
  for(int i = 0; i < pKMDataPts_->getNPts(); i++) {
    cout  << "   " << setw(5) << i
          << "   " << setw(5) << closeCtr[i]
          << "   " << setw(10) << sqDist[i]
          << endl;
  }
  cout << ")" << endl;
  delete [] closeCtr;
  delete [] sqDist;
}

// print the centers (assuming exectute() was applied)
void
KMeansWrapper::printCenters() const
{
  assert( is_executed_ ); // TODO: exception?
  if(pCenters_ && is_executed_)
    {
      pCenters_->print();
    }
}





IMPKMEANS_END_NAMESPACE
