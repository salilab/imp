/**
 *  \file KMData.h   \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_DATA_H
#define IMPSTATISTICS_INTERNAL_KM_DATA_H

#include <vector>
#include <iomanip>
#include <iostream>
#include <string>
#include <IMP/statistics/statistics_config.h>
#include <IMP/base_types.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

typedef enum {      // distributions
  UNIFORM,          // uniform over cube [-1,1]^d.
  GAUSS,            // Gaussian with mean 0
  LAPLACE,          // Laplacian, mean 0 and var 1
  CO_GAUSS,         // correlated Gaussian
  CO_LAPLACE,       // correlated Laplacian
  CLUS_GAUSS,       // clustered Gaussian
  CLUS_ORTH_FLATS,  // clustered on orthog flats
  CLUS_ELLIPSOIDS,  // clustered on ellipsoids
  MULTI_CLUS,       // multi-sized clusters
  N_DISTRIBS} DistributionType;

typedef Floats KMPoint;
typedef base::Vector<KMPoint *> KMPointArray;
//! Holds the data points to cluster
/**
   \unstable{KMData}
 */
class IMPSTATISTICSEXPORT KMData {
 public:
  //! Constructor
  /**
\param[in] d the dimension of the points
\param[in] n the number of points
   */
  KMData(int d, int n);
  //! Get the dimension of the points
  int get_dim() const { return dim_; }
  //! Get the number of points
  int get_number_of_points() const { return points_->size(); }
  //! Get the points
  KMPointArray *get_points() const { return points_; }
  /*   //! Get the tree */
  /*   KMCentersTree* get_tree() const { */
  /*     return tree_; */
  /*   } */
  //! Get a point
  KMPoint *operator[](int i) { return (*points_)[i]; }
  //! Get a const point
  const KMPoint *operator[](int i) const { return (*points_)[i]; }
  //  //! Build tree from points
  // void buildKcTree();
  //! Sample a center point. The center is selected randomly from the
  //! set of data points
  /**
   \param[in] offset after selecting a point randomly shift it
                     within offset radius
   */
  KMPoint sample_center(double offset = 0.);
  //! Sample few centers
  //! \param[in] sample where the samples will be stored
  //! param[in] k the number of centers to sample
  //! param[in] offset after selecting a point randomly shift it
  //!                  within offset radius
  //! param[in] allow_duplicate can the centers be the same
  virtual void sample_centers(KMPointArray *sample, int k, double offset = 0.,
                              bool allow_duplicate = false);
  //! Show the data points - TODO - add
  void show() {}
  virtual ~KMData();

 protected:
  int dim_;               // the dimension of the data points
  KMPointArray *points_;  // the data points
};
//! Allocate a point array of n points
/**
\param[in] n the number of points
\param[in] dim the dimension of each point
 */
inline KMPointArray *allocate_points(int n, int dim) {
  KMPointArray *points = new KMPointArray();
  for (int i = 0; i < n; i++) {
    KMPoint *p = new KMPoint();
    p->insert(p->end(), dim, 0.0);
    points->push_back(p);
  }
  return points;
}
//! Clear a vector of KMPoints and make sure the memory allocated
//! for the individual points is deallocated
inline void clear_points(KMPointArray *points) {
  if (points == nullptr) return;
  for (unsigned int i = 0; i < points->size(); i++) {
    delete (*points)[i];
  }
  points->clear();
}
//! Dellocate a point array
inline void deallocate_points(KMPointArray *points) {
  if (points == nullptr) return;
  for (unsigned int i = 0; i < points->size(); i++) {
    delete (*points)[i];
  }
  delete points;
}
inline void copy_point(const KMPoint *p_from, KMPoint *p_to) {
  p_to->clear();
  for (unsigned int i = 0; i < p_from->size(); i++) {
    p_to->push_back((*p_from)[i]);
  }
}
void copy_points(KMPointArray *from, KMPointArray *to);
//! Print a point
void print_point(const KMPoint &p, std::ostream &out = std::cout);
//! Print points
void print_points(const std::string &title, const KMPointArray &pa,
                  std::ostream &out = std::cout);

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_DATA_H */
