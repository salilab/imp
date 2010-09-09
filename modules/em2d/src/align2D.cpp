/**
 *  \file align2D.copp
 *  \brief Align operations for EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/align2D.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em/noise.h"
#include "IMP/em/Image.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/em/exp.h"
#include "IMP/exception.h"
#include "IMP/constants.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <complex>
#include <math.h>

IMPEM2D_BEGIN_NAMESPACE

ResultAlign2D align2D_complete(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_d &m2,
                      bool apply,
                      int interpolation_method) {
  IMP_LOG(IMP::TERSE,"starting complete 2D alignment" << std::endl);


  algebra::Matrix2D_d autoc1,autoc2,padded1,padded2,aux1,aux2,aux3,aux4;
  algebra::Transformation2D t;
  ResultAlign2D RA;

  m1.pad(padded1,0);
  m2.pad(padded2,0);
  autocorrelation2D(padded1,autoc1);
  autocorrelation2D(padded2,autoc2);

  RA=align2D_rotational(autoc1,autoc2,false,interpolation_method);
  double angle1 = RA.first.get_rotation().get_angle();

  // Check for both angles that can be the solution
  em::rotate(m2,angle1,aux1,true);
  RA=align2D_translational(m1,aux1);
  algebra::Vector2D shift1 = RA.first.get_translation();
  t.set_rotation(angle1);
  t.set_translation(shift1);
  em::apply_Transformation2D(m2,t,aux2,true);
  double ccc1=m1.cross_correlation_coefficient(aux2);


  double angle2;
  if (angle1 < PI) {
    angle2 = angle1+PI;
  } else {
    angle2 = angle1-PI;
  }

  em::rotate(m2,angle2,aux3,true);
  RA=align2D_translational(m1,aux3);

  algebra::Vector2D shift2=RA.first.get_translation();
  t.set_rotation(angle2);
  t.set_translation(shift2);
  em::apply_Transformation2D(m2,t,aux4,true);
  double ccc2=m1.cross_correlation_coefficient(aux4);

  double max_ccc;
  if(ccc2>ccc1) {
    max_ccc=ccc2;
    if(apply) {m2.copy(aux4);}
  } else {
    max_ccc=ccc1;
    if(apply) {m2.copy(aux2);}
  }
  return ResultAlign2D(t,max_ccc);
}



ResultAlign2D align2D_rotational(algebra::Matrix2D_d &m1,
                          algebra::Matrix2D_d &m2,
                          bool apply,
                          bool pad,
                          int interpolation_method) {
  IMP_LOG(IMP::TERSE,
          "starting 2D rotational alignment" << std::endl);
  IMP_USAGE_CHECK((m1.get_number_of_rows()==m2.get_number_of_rows()) &&
                  (m1.get_number_of_columns()==m1.get_number_of_columns()),
                  "em2d::align_rotational: Matrices have different size.");

  algebra::Matrix2D_d polar1,polar2,corr;
  resample2D_polar(m1,polar1,interpolation_method);  // subject image
  resample2D_polar(m2,polar2,interpolation_method); // projection image
  ResultAlign2D RA= align2D_translational(polar1,polar2,true,false);
  algebra::Vector2D peak=RA.first.get_translation();
  double max_cc=RA.second;
  unsigned int sampling_points = polar1.get_size(1);
  double Dang = 2*PI/(double)sampling_points;
  double angle =(-1)*peak[1]*Dang; // Peak[1] is the index for the optimal angle
  if(angle < 0 ) angle += 2*PI; // keep the angle between 0 and 2PI

  // Apply the rotation if requested
  if(apply) {
    algebra::Matrix2D_d result;
    em::rotate(m2,angle,result,true);
    m2 = result;
  }
  IMP_LOG(IMP::VERBOSE," max_cc=" << max_cc << " angle= " << angle
          << std::endl);

  algebra::Transformation2D t(algebra::Vector2D(0.0,0.0));
  t.set_rotation(angle);
  return ResultAlign2D(t,max_cc);
}



ResultAlign2D align2D_translational(algebra::Matrix2D_d &m1,
                           algebra::Matrix2D_d &m2,
                            bool apply,
                            bool pad) {
  IMP_LOG(IMP::TERSE, "starting 2D translational alignment" << std::endl);
  IMP_USAGE_CHECK((m1.get_number_of_rows()==m2.get_number_of_rows()) &&
                  (m1.get_number_of_columns()==m1.get_number_of_columns()),
                  "em2d::align_translational: Matrices have different size.");
  algebra::Matrix2D_d cross_corr;
  em2d::correlation2D(m1,m2,cross_corr);
  double max_cc;
  algebra::Vector2D peak = peak_search(cross_corr,&max_cc);
  // Correct the shift applied for the fact that we are using the
  // cross correlation matrix as an image
  for(int i=0;i<2;++i) {
    int half=cross_corr.get_size(i)/2;
    peak[i]-=half;
  }
  // Apply the shift if requested
  if(apply) {
    algebra::Matrix2D_d result;
    em::shift(m1,peak,result,true);
    m1 = result;
  }
  IMP_LOG(IMP::VERBOSE," max_cc=" << max_cc << " shift= " << peak
          << std::endl);

  algebra::Transformation2D t(peak);
  return ResultAlign2D(t,max_cc);
}



double align2D_complete_no_preprocessing(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_c &M1,
                      complex_rings &fft_rings1,
                      algebra::Matrix2D_d &m2,
                      complex_rings &fft_rings2,
                      algebra::Transformation2D &t,
                      bool apply) {
  IMP_LOG(IMP::TERSE,
          "starting complete 2D alignment with no preprocessing" << std::endl);

  algebra::Matrix2D_d aux;
  algebra::Matrix2D_c AUX;
  double max_cc,angle1,angle2;

  PolarResamplingParameters polar_params(m1);
  max_cc= align2D_rotational_no_preprocessing(fft_rings1,fft_rings2,&angle1,
                                              polar_params);
  // Check for both angles that can be the solution
  algebra::Vector2D shift1(0.,0.),shift2(0.,0.);
  em::rotate(m2,angle1,aux,true);
  FFT2D fft1(aux,AUX); fft1.execute();
  align2D_translational_no_preprocessing(M1,AUX,shift1,
                          aux.get_number_of_rows(),
                          aux.get_number_of_columns());
  t.set_rotation(angle1);
  t.set_translation(shift1);
  em::apply_Transformation2D(m2,t,aux,true);
  double ccc1=m1.cross_correlation_coefficient(aux);
  if (angle1 < PI) {
    angle2 = angle1+PI;
  } else {
    angle2 = angle1-PI;
  }
  em::rotate(m2,angle2,aux,true);
  FFT2D fft2(aux,AUX); fft2.execute();
  align2D_translational_no_preprocessing(M1,AUX,shift2,
                            aux.get_number_of_rows(),
                            aux.get_number_of_columns());
  t.set_rotation(angle2);
  t.set_translation(shift2);
  em::apply_Transformation2D(m2,t,aux,true);
  double ccc2=m1.cross_correlation_coefficient(aux);
  double max_ccc;
  if(ccc2>ccc1) {
    angle1=angle2;  shift1=shift2;  max_ccc=ccc2;
  } else {
    max_ccc=ccc1;
  }
  t.set_rotation(angle1);
  t.set_translation(shift1); // Translation here means rows and columns (y,x)
  if(apply) {
    if(max_ccc == ccc1) {
      // If max_ccc was ccc1 recompute the transformation
      // (for ccc2 is already in aux)
      em::apply_Transformation2D(m2,t,aux,true);
    }
    m2 = aux;
  }
  IMP_LOG(IMP::VERBOSE," max_cc=" << max_cc << " transformation= " << t
          << std::endl);

  return max_ccc;
}





complex_rings preprocess_for_align2D_rotational(algebra::Matrix2D_d &m,
                      bool dealing_with_subjects,int interpolation_method) {
  IMP_LOG(IMP::TERSE,
          "starting preprocess for 2D rotational alignment" << std::endl);

  double_rings rings;
  resample2D_polar(m,rings,dealing_with_subjects,interpolation_method);
  unsigned int n_rings = rings.size();
  IMP_LOG(IMP::VERBOSE,"Number of rings: " << n_rings << std::endl);
  complex_rings fft_rings(rings.size());
  for(unsigned int i=0;i<n_rings;++i)  {
    // size for FFTs, half of the ring size
    unsigned int fft_size=get_fast_fft_size(rings[i].size());
    IMP_LOG(IMP::VERBOSE,"Size ring " << i << " " << fft_size << std::endl);
    std::vector<std::complex<double> > V(fft_size);
    FFT1D fft(rings[i],V); fft.execute();
    fft_rings[i]=V;
  }
  return fft_rings;
}





double align2D_rotational_no_preprocessing(
      complex_rings &fft_rings1,complex_rings &fft_rings2,
      double *angle,const PolarResamplingParameters polar_params) {

  IMP_LOG(IMP::TERSE,
    "starting 2D rotational alignment with no preprocessing" << std::endl);

  unsigned int n_rings = polar_params.get_number_of_rings();
  // Vector to store the rotational correlation
  unsigned int max_sampling_points=2*polar_params.get_sampling_points(n_rings);
  std::vector<double > rot_cc(max_sampling_points);
  // FFT of rot_cc

  unsigned int max_fft_size = get_fast_fft_size(max_sampling_points);
  std::vector<std::complex<double> > SUM(max_fft_size);
  std::complex<double> val(0.0,0.0);
  for(unsigned int k=0;k<max_fft_size;k++) { SUM[k] = val; }
  // Compute total FFT
  for(unsigned int i=0;i<n_rings;++i)  {
    for(unsigned int k=0;k<fft_rings1[i].size();k++) {
      SUM[k] += fft_rings1[i][k]*std::conj(fft_rings2[i][k]);
    }
  }
  IFFT1D ifft(SUM,rot_cc); ifft.execute();
  // Find the peak of the cross_correlation and the value
  double max_cc;
  double p = peak_search(rot_cc,&max_cc);
  double Dang = 2*PI/(double)max_sampling_points;
  max_cc = max_cc * Dang * polar_params.get_radius_step();
  // -1 because the function returns directly the angle  of maximum correlation.
  // to align the images, m2 has to be rotated by -angle
  *angle =(-1)*p*Dang;
  if(*angle < 0 ) *angle += 2*PI; // keep the angle between 0 and 2PI
  IMP_LOG(IMP::VERBOSE," max_cc=" << max_cc << " angle= " << *angle
          << std::endl);
  return max_cc;
}


double align2D_translational_no_preprocessing(
               algebra::Matrix2D_c &M1,
               algebra::Matrix2D_c &M2,
               algebra::Vector2D &v,
               unsigned int original_rows,
               unsigned int original_cols) {
  IMP_LOG(IMP::TERSE,
      "starting 2D translational alignment with no preprocessing" << std::endl);
  algebra::Matrix2D_d cross_corr;
  cross_corr.resize(original_rows,original_cols);
  em2d::correlation2D_no_preprocessing(M1,M2,cross_corr);
  // Find the peak of the cross_correlation
  double max_cc;
  v = peak_search(cross_corr,&max_cc);
  // Correct the shift applied for the fact that we are using the
  // cross correlation matrix as an image
  for(int i=0;i<2;++i) {
    int half=cross_corr.get_size(i)/2;
    v[i]-=half;
  }

  IMP_LOG(IMP::VERBOSE," max_cc=" << max_cc << " shift= " << v << std::endl);
  return max_cc;
}




void resample2D_polar(algebra::Matrix2D_d &m,
                      double_rings &rings,
                      bool dealing_with_subjects,
                      int interpolation_method) {
  IMP_LOG(IMP::TERSE,"starting 2D polar resampling " << std::endl);
   // Save origin and center
  std::vector<int> orig(2); orig[0]=m.get_start(0); orig[1]=m.get_start(1);
  m.centered_start();

  PolarResamplingParameters polar_params(m);
  unsigned int n_rings =  polar_params.get_number_of_rings();
  unsigned int sampling_points;
  for(unsigned int n=0;n<n_rings;++n) {
    sampling_points = polar_params.get_sampling_points(n);
    IMP_LOG(IMP::VERBOSE,"ring " << n << "sampling points "
                << sampling_points << std::endl);

    double Dang = polar_params.get_angle_step(n);
    std::vector<double> vi(sampling_points);
    double radius = polar_params.get_radius(n);
    for(unsigned int k = 0;k<sampling_points;++k) {
      double angle = k*Dang;
      algebra::Vector2D cartesians(radius*sin(angle),radius*cos(angle));
      vi[k]=algebra::interpolate(m,cartesians,true,interpolation_method);
      // When dealing with subjets, instead of doing the actual polar resampling
      // the value computed is radius*f(radius,angle). This helps for doing
      // the rotational correlation function
      if(dealing_with_subjects) {
        vi[k] *= radius;
      }
    }
    rings.push_back(vi);
  }
  // Restore origin
  m.set_start(orig);
}



void resample2D_polar(algebra::Matrix2D_d &m,
                      algebra::Matrix2D_d &result,
                      int interpolation_method) {
  IMP_LOG(IMP::TERSE,"starting new 2D polar resampling " << std::endl);
   // Save origin and center
  std::vector<int> orig(2); orig[0]=m.get_start(0); orig[1]=m.get_start(1);
  m.centered_start();

  PolarResamplingParameters polar_params(m);

  unsigned int n_rings =  polar_params.get_number_of_rings();
  unsigned int sampling_points = polar_params.get_sampling_points(n_rings);
  result.resize(n_rings,sampling_points);

  double Dang = polar_params.get_angle_step(n_rings);
  for(unsigned int i=0;i<n_rings;++i) {
    double radius = polar_params.get_radius(i);
    for(unsigned int j = 0;j<sampling_points;++j) {
      double angle = j*Dang;
      algebra::Vector2D cartesians(radius*sin(angle),radius*cos(angle));
      result[i][j]=algebra::interpolate(m,cartesians,true,interpolation_method);
    }
  }
  // Restore origin
  m.set_start(orig);
}




algebra::Vector2D peak_search(algebra::Matrix2D_d &m,double *value) {
  // Find maximum value and location
  IMP_LOG(IMP::VERBOSE,"starting peak seach on a matrix " << std::endl);

  std::vector<int> loc(2);
  *value=m.compute_max(loc);
  // Perform a weighted centroiding with the neighbours to find the actual
  // maximum value. Performs as well as parabolic fit (Paulo, Opt. Eng. 2007)
  int y0=m.get_start(0);    int x0=m.get_start(1);
  int yn=m.get_finish(0);   int xn=m.get_finish(1);
  algebra::Vector2D peak;
  int y=loc[0],x=loc[1];
  if((y==y0 && x==x0) || (y==y0 && x==xn) ||
     (y==yn && x==x0) || (y==yn && x==xn) ) {
    // corners
    peak[0]=y;
    peak[1]=x;
  } else if(y==y0 || y==yn) {
    // row borders
    peak[0]=y;
    peak[1]=((x-1)*m(y,x-1)+x*m(y,x)+(x+1)*m(y,x+1))/(m(y,x-1)+m(y,x)+m(y,x+1));
  } else if(x==x0 || x==xn) {
    // Column borders
    peak[0]=((y-1)*m(y-1,x)+y*m(y,x)+(y+1)*m(y+1,x))/(m(y-1,x)+m(y,x)+m(y+1,x));
    peak[1]=x;
  } else {
    // Other indexes
    int bases[2];   bases[0]=y-1; bases[1]=x-1;
    int size3x3[2]; size3x3[0]=3; size3x3[1]=3;
    peak=compute_center_of_gravity(m,size3x3,bases);
  }
  return peak;
}

double peak_search(std::vector<double> &v,double *value) {
  IMP_LOG(IMP::VERBOSE,"starting peak search on a vector " << std::endl);
  // Find the location of the maximum value
  double max_value=v[0];
  unsigned int loc=0,len=v.size();
  for(unsigned int i=0;i<len;++i) {
    if(v[i]>max_value) {
      loc = i;
      max_value = v[i];
    }
  }
  *value = max_value;
  // Perform a weighted centroiding with the neighbours to find the actual
  // maximum value. Performs as well as parabolic fit (Paulo, Opt. Eng. 2007)
  double peak;
  double dl = (double)loc;
  if(loc==0 || loc==(len-1)) {
    peak = (double)loc; // No centroiding can be done
  } else {
    peak=(v[loc-1]*(dl-1)+v[loc]*dl+v[loc+1]*(dl+1))/(v[loc-1]+v[loc]+v[loc+1]);
  }
  return peak;
}

// Note: Only works for matrices with positive values
algebra::Vector2D compute_center_of_gravity(algebra::Matrix2D_d &m,
                        int *dims,int *bases) {
  if(dims == NULL && bases == NULL) {
    dims = new int [2];
    bases = new int [2];
    dims[0]=m.get_number_of_rows();
    dims[1]=m.get_number_of_columns();
    bases[0]=m.index_bases()[0];
    bases[1]=m.index_bases()[1];
  }
  algebra::Vector2D center_of_g = algebra::get_zero_vector_d<2>();
  double denominator=0.0;
  int idx[2];
  for (int i=bases[0];i<bases[0]+dims[0];++i) {
    for (int j=bases[1];j<bases[1]+dims[1];++j) {
      denominator+=m(i,j);
      idx[0]=i; idx[1]=j;
      for(int k = 0;k<2;k++) {
        center_of_g[k]+=m(i,j)*idx[k];
      }
    }
  }
  center_of_g/=denominator;
  return center_of_g;
}

void print_vector(std::vector< std::complex<double> > &v) {
  for (unsigned int i=0;i<v.size();++i) {
     std::cout << "i " << i << " v[i] =" << v[i] << std::endl;
  }
}


double align2D_complete_with_centers(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_d &m2,
                      algebra::Transformation2D &t,
                      bool apply,
                      int interpolation_method) {

    IMP_USAGE_CHECK((m1.get_number_of_rows()==m2.get_number_of_rows()) &&
                  (m1.get_number_of_columns()==m2.get_number_of_columns()),
        "em2d::align2D_complete_with_cemters: Matrices have different size.");
  algebra::Matrix2D_d cm1,cm2; // centered versions of the matrices
    // Save the origin and center
  std::vector<int> orig1(2),orig2(2);
  orig1[0]=m1.get_start(0);  orig1[1]=m1.get_start(1);
  orig2[0]=m2.get_start(0);  orig2[1]=m2.get_start(1);
  m1.centered_start();      m2.centered_start();

  algebra::Vector2D cent1=compute_center_of_gravity(m1);
  algebra::Vector2D cent2=compute_center_of_gravity(m2);
  // Center the images to the centers
  algebra::Vector2D minus_cent1 = (-1)*cent1;
  algebra::Vector2D minus_cent2 = (-1)*cent2;
  em::shift(m1,minus_cent1,cm1,true);
  em::shift(m2,minus_cent2,cm2,true);
  ResultAlign2D RA= align2D_rotational(
                          cm1,cm2,false,interpolation_method);
  double angle=RA.first.get_rotation().get_angle();
  double max_cc = RA.second;
  t.set_rotation(angle);
  // Compute translation using the centers
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = cent1 - R.get_rotated(cent2);
  t.set_translation(displacement);
  // Apply
  if(apply) {
    em::apply_Transformation2D(m2,t,cm2,true,0.0,0);
    m2 = cm2;
  }
  m1.set_start(orig1); m2.set_start(orig2);
  return max_cc;

}


double align2D_complete_with_centers_no_preprocessing(
                      algebra::Vector2D &center1,
                      algebra::Vector2D &center2,
                      complex_rings &fft_rings1,
                      complex_rings &fft_rings2,
                      algebra::Transformation2D &t,
                      PolarResamplingParameters polar_params) {
  // Align rotationally with FFT
  double max_cc,angle;
  max_cc= align2D_rotational_no_preprocessing(fft_rings1,fft_rings2,&angle,
                                              polar_params);
  if (angle < 0) { angle += 2*PI; }
  // Compute translation using the centers
  t.set_rotation(angle);
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = center1 - R.get_rotated(center2);
  t.set_translation(displacement);
  return max_cc;
}

IMPEM2D_END_NAMESPACE
