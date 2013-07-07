/**
 *  \file IMP/statistics/GaussianComponent.h
 *  \brief A component of a GMM
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */


#ifndef IMPSTATISTICS_GAUSSIANCOMPONENT_H
#define IMPSTATISTICS_GAUSSIANCOMPONENT_H

#include "internal/DataPoints.h"
#include <IMP/algebra/Ellipsoid3D.h>
#include <IMP/statistics/statistics_config.h>
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE
typedef algebra::internal::TNT::Array2D<double> Array2DD;
typedef algebra::internal::TNT::Array1D<double> Array1DD;

//#define EPS 1e20 //TODO - change

//! Hold a single component of the GMM
class IMPSTATISTICSEXPORT GaussianComponent : public IMP::base::Object{
public:

  //! Constructor. Set initial mean and sigma
  /**
  \param[in] mean    initial mean. We randomize these values.
  \param[in] sigma   initial sigma
  \param[in] prior   initial prior
  \param[in] index   the component index
   */
  GaussianComponent(const Array1DD &m,
                    const Array2DD &s, double prior, int index,
                    std::string name= "GaussianComponent%1%");
  //! Constructor using an Ellipsoid3D
  /**
  \param[in] e       Get center and extent from this ellipsoid
  \param[in] prior   initial prior
  \param[in] index   the component index
   */
  GaussianComponent(const IMP::algebra::Ellipsoid3D &e,
                    float prior,int index,
                    std::string name= "GaussianComponent%1%");

  //! Calculate p(x|i), the probably to see data point x given this component
  double pdf(const Array1DD &x) const;
  /* Update the parameters of the component Calculate p(x|i),
     the probably to see data point x given this component */
  void update(const std::vector<Array1DD> &data, const Array2DD &data_r);
  double get_prior() const {return prior_;}
  void set_prior(double p) {prior_=p;}
  void show(std::ostream &out=std::cout) const;
  Array1DD get_mean() const {return mean_;}
  Array2DD get_sigma() const {return sigma_;}
  void set_mean(const Array1DD m) {mean_=m;}
  void set_sigma(double v) {
    for(int i=0;i<sigma_.dim1();i++){
      for(int j=0;j<sigma_.dim2();j++) {
        sigma_[i][j] = v + id_mat_[i][j];//add 1 to make the matrix invertable
      }
    }
  }
  void show_single_line(std::ostream &out=std::cout) const;
  inline int get_dim() const {return dim_;}
  IMP_OBJECT_METHODS(GaussianComponent);
protected:
  //! Update parameters according to current mean and sigma values
  void init_component(const Array1DD &m,const Array2DD &s,
                      double prior, int index);
  void update_parameters();
  void update_sigma(const std::vector<Array1DD> &data, const Array2DD &data_r);
  void initial_sigma(const std::vector<Array1DD> &data);
  int dim_;
  Array1DD mean_;
  Array2DD sigma_;
  Array2DD id_mat_;
  Array2DD sigma_inv_;
  double sigma_det_;
  double mult_factor_;
  double prior_;
  int ind_;
  const double GMM_EPS=0.00000000000000000000001; //TODO - change?
};
IMP_OBJECTS(GaussianComponent, GaussianComponents);

//! Transform a 3D Gaussian to an ellipsoid
IMP::algebra::Ellipsoid3D get_ellipsoid3D(GaussianComponent * gc);
IMP::algebra::Sphere3D get_sphere3D(GaussianComponent * gc);

// GaussianComponent * read_gaussian_component(const stringstream &data) {
// }
// void write_gaussian_component(const string &filename) {
// }

IMPSTATISTICS_END_NAMESPACE
#endif /* IMPSTATISTICS_GAUSSIANCOMPONENT_H */
