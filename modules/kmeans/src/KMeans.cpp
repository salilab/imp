/**
 *  \file KMeans.cpp
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/kmeans/KMeans.h"
#include <IMP/base/log.h>
#include <iostream>
#include <iomanip>
#include <string>

IMPKMEANS_BEGIN_NAMESPACE

namespace {  // Anonymous
std::ostream& operator<<(std::ostream& out, const IMP::Floats& p) {
  const int dim = p.size();
  if (dim == 0) {
    out << "()" << std::endl;
  } else {
    out << "(" << p[0];
    for (int i = 1; i < dim; i++) {
      out << ", " << p[i];
    }
    out << ")" << std::endl;
  }
  return out;
}
}

/***********************  Constructors  **************************/

const std::string CONST_kmeans_type = "kmeans %1%";

/**
   Initialize the KMeans object with data from fname_data,
   assuming input data of dimension dim

   @param[in] fname_data Input filename. Input is assumed to be textual,
   whitespace separated
   @param[in] dim Dimension of points
   @param[in] max_nPts Maximal number of points to be read from file
*/
KMeans::KMeans(const std::string& fname, int dim, unsigned int max_nPts)
    : Object(CONST_kmeans_type),
      is_executed_(false),
      pKMDataPts_(nullptr),
      is_KM_data_synced_(false),
      pCenters_(nullptr),
      terminationConditions_(100, 0, 0, 0,  // run for 100 stages
                             0.10,          // min consec RDL
                             0.10,          // min accum RDL
                             3,             // max run stages
                             0.50,          // init. prob. of acceptance
                             10,            // temp. run length
                             0.95)          // temp. reduction factor
{
  using namespace std;

  // read / generate points
  ifstream infile;
  infile.open(fname.c_str(), ifstream::in);
  if (infile.good()) {
    read_data_pts_from_stream(infile, dim, max_nPts);
    infile.close();
  }
  // TODO: Warn if no data was read or throw error if bad file?
  //       Right now, the indication of bad file would be that
  //       STLDataPts_ is empty
}

/** Empty constructor for all default initializations -
    object data is not considered initialized after this call
 */
KMeans::KMeans()
    : Object(CONST_kmeans_type),
      is_executed_(false),
      pKMDataPts_(nullptr),
      is_KM_data_synced_(false),
      pCenters_(nullptr),
      terminationConditions_(100, 0, 0, 0,  // run for 100 stages
                             0.10,          // min consec RDL
                             0.10,          // min accum RDL
                             3,             // max run stages
                             0.50,          // init. prob. of acceptance
                             10,            // temp. run length
                             0.95)          // temp. reduction factor
{}

/*********************** Public methods **************************/

// execute the selected algorithms:
void KMeans::execute(unsigned int k, KM_ALG_TYPE alg_type, int stages) {
  using namespace std;

  assert(STLDataPts_.size() >= k);  // TODO: exception? warning?

  IMP_LOG(PROGRESS, "Data Points:" << std::endl);  // echo data points
  for (unsigned int i = 0; i < STLDataPts_.size(); i++)
    IMP_LOG(PROGRESS, STLDataPts_[i]);

  // synchronize STL points to wrapped internal::KMdata points
  // allocate points array
  this->sync_KMdata_pts_from_STL();
  terminationConditions_.setAbsMaxTotStage(stages);  // set number of stages
  // TODO: annoying that we must remember to call buildKcTree explicitly
  //       - sounds like bug-prone voodoo
  pKMDataPts_->buildKcTree();  // build filtering structure
  pCenters_ = new internal::KMfilterCenters(k, *pKMDataPts_);
  switch (alg_type)
      // TODO: don't output anything ;
  {
    case KM_LLOYDS: {
      // repeated Lloyd's
      IMP_LOG(base::PROGRESS, "\nExecuting Clustering Algorithm: Lloyd's\n");
      internal::KMlocalLloyds kmLloyds(*pCenters_, terminationConditions_);
      *pCenters_ = kmLloyds.execute();
      is_executed_ = true;
      print_summary(kmLloyds, base::PROGRESS);
      break;
    }
    case KM_LOCAL_SWAP: {
      // Swap heuristic
      IMP_LOG(base::PROGRESS, "\nExecuting Clustering Algorithm: Swap\n");
      internal::KMlocalSwap kmSwap(*pCenters_, terminationConditions_);
      *pCenters_ = kmSwap.execute();
      is_executed_ = true;
      print_summary(kmSwap, base::PROGRESS);
      break;
    }
    case KM_LOCAL_EZ_HYBRID: {
      // EZ-Hybrid heuristic
      IMP_LOG(base::PROGRESS, "\nExecuting Clustering Algorithm: EZ-Hybrid\n");
      internal::KMlocalEZ_Hybrid kmEZ_Hybrid(*pCenters_,
                                             terminationConditions_);
      *pCenters_ = kmEZ_Hybrid.execute();
      is_executed_ = true;
      print_summary(kmEZ_Hybrid, base::PROGRESS);
      break;
    }
    case KM_HYBRID: {
      // Hybrid heuristic
      IMP_LOG(base::PROGRESS, "\nExecuting Clustering Algorithm: Hybrid\n");
      internal::KMlocalHybrid kmHybrid(*pCenters_, terminationConditions_);
      *pCenters_ = kmHybrid.execute();
      is_executed_ = true;
      print_summary(kmHybrid, base::PROGRESS);
      break;
    }
  }

  // assign data points to nearest clusters
  int n = pKMDataPts_->getNPts();
  internal::KMctrIdxArray closeCtr = new internal::KMctrIdx[n];
  double* sqDist = new double[n];
  pCenters_->getAssignments(closeCtr, sqDist);
  // store in class internal variables
  centerAssignments_.resize(n);
  ptsSqrDistToCenters_.resize(n);
  for (int i = 0; i < n; i++) {
    centerAssignments_[i] = closeCtr[i];
    ptsSqrDistToCenters_[i] = sqDist[i];
  }
  // get rid of temporary arrays
  if (closeCtr) delete[] closeCtr;
  if (sqDist) delete[] sqDist;
}

/**
   Add a data point for the next clustering.
*/
void KMeans::add_data_pt(const IMP::Floats& p) {
  is_executed_ = false;
  is_KM_data_synced_ = false;
  // verify same dimension as existing points
  if (STLDataPts_.size() > 0) {
    assert(STLDataPts_[0].size() == p.size());
  }
  STLDataPts_.push_back(p);
}

/**
   Clears all data in object
*/
void KMeans::clear_data() {
  is_executed_ = false;
  is_KM_data_synced_ = false;
  STLDataPts_.clear();
}

/** Returns the i'th point in the dataset

    @param[in] i Center number in range (0,...,nPts-1)
*/
const IMP::Floats& KMeans::get_data_point(unsigned int i) const {
  assert(i < STLDataPts_.size());  // TODO: exception?
  return STLDataPts_[i];
}

/** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i center number in range (0,...,k-1)
   */
IMP::Floats KMeans::get_center(unsigned int i) const {
  // TODO: exception instead of assertion?
  assert(is_executed_ && i < (unsigned int)pCenters_->getNPts());

  // Convert from KMCenter (aka double*)
  // to IMP::Floats
  int dim = pCenters_->getDim();
  const internal::KMcenter& iCenter = (*pCenters_)[i];
  IMP::Floats retValue(dim);
  for (int j = 0; j < dim; j++) retValue[j] = iCenter[j];
  return retValue;
}

/*************************** Private Methods **************************/

/** Updates the wrapped data pts structure from the internal 2D STL vector
    array (IMP::Floats).
    This method invalidates any prior information about clustering results
*/
void KMeans::sync_KMdata_pts_from_STL() {
  assert(STLDataPts_.size() > 0);  // exception?
  if (is_KM_data_synced_) return;  // already synced
  is_executed_ = false;            // invalidate any preexisting clustering info
  unsigned int nPts = STLDataPts_.size();
  unsigned int dim = STLDataPts_[0].size();
  // copy points to internal data structure
  pKMDataPts_ = new internal::KMdata(dim, nPts);
  for (unsigned int i = 0; i < nPts; i++) {
    for (unsigned int j = 0; j < dim; j++) {
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
bool KMeans::read_pt_from_stream(std::istream& in, IMP::Floats& p,
                                 unsigned int dim) {
  for (unsigned int d = 0; d < dim; d++) {
    if (!(in >> p[d])) return false;
  }
  return true;
}

// reads points from a stream
void KMeans::read_data_pts_from_stream(std::istream& in, unsigned int dim,
                                       unsigned int max_nPts) {
  is_executed_ = false;        // invalidate
  is_KM_data_synced_ = false;  // pKMDataPts_ now out of sync with STLData_
  assert(in.good());           // TODO: exception?
  STLDataPts_.clear();
  for (unsigned int i = 0; i < max_nPts; i++) {
    IMP::Floats newPoint(dim);
    bool ok = read_pt_from_stream(in, newPoint, dim);
    if (ok)
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
void KMeans::print_pt_to_stream(std::ostream& out, const IMP::Floats& p) {
  out << p;
}

//------------------------------------------------------------------------
//  Print summary of execution
//------------------------------------------------------------------------
void KMeans::print_summary(const internal::KMlocal& theAlg,
                           base::LogLevel ll)  // the algorithm
{
  using namespace std;

  assert(is_executed_);
  IMP_LOG(ll, "Number of stages: " << theAlg.getTotalStages() << endl);
  IMP_LOG(ll, "Average distortion: " << pCenters_->getDist(false) /
                                            double(pCenters_->getNPts())
                                     << endl);
  // print final center points
  IMP_LOG(ll, "(Final Center Points:" << endl);
  pCenters_->log(ll);
  IMP_LOG(ll, ")" << endl);
  // get/print final cluster assignments
  internal::KMctrIdxArray closeCtr =
      new internal::KMctrIdx[pKMDataPts_->getNPts()];
  double* sqDist = new double[pKMDataPts_->getNPts()];
  pCenters_->getAssignments(closeCtr, sqDist);
  IMP_LOG(ll,
          "(Cluster assignments:" << endl << "    Point  Center  Squared Dist"
                                  << endl << "    -----  ------  ------------"
                                  << endl);
  for (int i = 0; i < pKMDataPts_->getNPts(); i++) {
    IMP_LOG(ll, "   " << setw(5) << i << "   " << setw(5) << closeCtr[i]
                      << "   " << setw(10) << sqDist[i] << endl);
  }
  IMP_LOG(ll, ")" << endl);
  delete[] closeCtr;
  delete[] sqDist;
}

// print the centers (assuming execute() was applied)
void KMeans::print_centers(base::LogLevel ll) const {
  assert(is_executed_);  // TODO: exception?
  if (pCenters_ && is_executed_) {
    pCenters_->log(ll);
  }
}

IMPKMEANS_END_NAMESPACE
