/**
 * \file KMCenters.h \brief Holds the centers of the clusters
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_CENTERS_H
#define IMPSTATISTICS_INTERNAL_KM_CENTERS_H

#include "KMData.h"
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//! Manages the centers of the data points
/**
   \unstable{KMCenters}
 */
class IMPSTATISTICSEXPORT KMCenters {
 public:
  KMCenters() {}
  KMCenters(int k, KMData* p);
  virtual ~KMCenters();
  //! Copy constrator
  /** Deep copy the centers and shallow copy the data_points_
/note This is needed for the local sampling algorithm.
   */
  KMCenters(const KMCenters& other) {
    data_points_ = other.data_points_;
    centers_ = new KMPointArray();
    copy_points(other.centers_, centers_);
  }

  KMCenters& operator=(const KMCenters& other) {
    if (this != &other) {  // avoid self assignment (x=x)
      data_points_ = other.data_points_;
      centers_ = new KMPointArray();
      copy_points(other.centers_, centers_);
    }
    return *this;
  }
  int get_dim() const { return data_points_->get_dim(); }
  int get_number_of_points() const {
    return data_points_->get_number_of_points();
  }
  int get_number_of_centers() const { return centers_->size(); }
  KMData* get_data() { return data_points_; }
  KMPoint* operator[](int i) { return (*centers_)[i]; }
  const KMPoint* operator[](int i) const { return (*centers_)[i]; }
  virtual void show(std::ostream& out = std::cout) const;

 protected:
  KMData* data_points_;
  KMPointArray* centers_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_CENTERS_H */
