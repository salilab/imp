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
  algebra::Transformation2D transformation1,transformation2;
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
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  em::apply_Transformation2D(m2,transformation1,aux2,true);
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
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  em::apply_Transformation2D(m2,transformation2,aux4,true);
  double ccc2=m1.cross_correlation_coefficient(aux4);
  if(ccc2>ccc1) {
    if(apply) {m2.copy(aux4);}
    IMP_LOG(IMP::VERBOSE," Transformation= "  << transformation2
                    << " cross_correlation = " << ccc2 << std::endl);
    return ResultAlign2D(transformation2,ccc2);
  } else {
    if(apply) {m2.copy(aux2);}
    IMP_LOG(IMP::VERBOSE," Transformation= "  << transformation1
                    << " cross_correlation = " << ccc1 << std::endl);
    return ResultAlign2D(transformation1,ccc1);
  }
}



ResultAlign2D align2D_rotational(algebra::Matrix2D_d &m1,
                          algebra::Matrix2D_d &m2,
                          bool apply,
                          int interpolation_method) {
  IMP_LOG(IMP::TERSE,
          "starting 2D rotational alignment" << std::endl);
  IMP_USAGE_CHECK((m1.get_number_of_rows()==m2.get_number_of_rows()) &&
                  (m1.get_number_of_columns()==m1.get_number_of_columns()),
                  "em2d::align_rotational: Matrices have different size.");

  algebra::Matrix2D_d polar1,polar2,corr;
  resample2D_polar(m1,polar1,interpolation_method);  // subject image
  resample2D_polar(m2,polar2,interpolation_method); // projection image
  ResultAlign2D RA= align2D_translational(polar1,polar2);
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

  algebra::Transformation2D t(algebra::Vector2D(0.0,0.0));
    t.set_rotation(angle);
  IMP_LOG(IMP::VERBOSE," Transformation= "
          << t << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
}



ResultAlign2D align2D_translational(algebra::Matrix2D_d &m1,
                           algebra::Matrix2D_d &m2,
                            bool apply) {
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
  algebra::Transformation2D t(peak);
  IMP_LOG(IMP::VERBOSE," Transformation= "
          << t << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
}




ResultAlign2D align2D_complete_no_preprocessing(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_c &M1,
                      algebra::Matrix2D_c &AUTOC_POLAR1,
                      algebra::Matrix2D_d &m2,
                      algebra::Matrix2D_c &AUTOC_POLAR2,
                      bool apply) {

  IMP_LOG(IMP::TERSE,
          "starting complete 2D alignment with no preprocessing" << std::endl);

  algebra::Matrix2D_d aux1,aux2,aux3,aux4;
  algebra::Matrix2D_c AUX1,AUX2,AUX3;
  algebra::Transformation2D transformation1,transformation2;
  double angle1=0,angle2=0;
  // From the dimensions of m1, compute the dimensions of the autocorrelation,
  // (m1 is padded before computing the autocorrelation),
  // and those of the polar resampling of the autocorrelation
  unsigned int m1_rows = m1.get_number_of_rows();
  unsigned int m1_cols = m1.get_number_of_columns();
  unsigned int autoc_rows = 2*m1_rows; // because of padding
  unsigned int autoc_cols = 2*m1_cols; // because of padding
  PolarResamplingParameters polar_params(autoc_rows,autoc_cols);
  unsigned int n_rings = polar_params.get_number_of_rings();
  unsigned int sampling_points=polar_params.get_sampling_points(n_rings);

  ResultAlign2D RA = align2D_rotational_no_preprocessing(AUTOC_POLAR1,
                              AUTOC_POLAR2,n_rings,sampling_points);

  angle1 = RA.first.get_rotation().get_angle();
  // Check for both angles that can be the solution
  em::rotate(m2,angle1,aux1,true);
  FFT2D fft1(aux1,AUX1); fft1.execute();
  RA = align2D_translational_no_preprocessing(M1,AUX1,m1_rows,m1_cols);
  algebra::Vector2D shift1 = RA.first.get_translation();
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  em::apply_Transformation2D(m2,transformation1,aux2,true);
  double ccc1=m1.cross_correlation_coefficient(aux2);
  // Check the opposed angle
  if (angle1 < PI) {
    angle2 = angle1+PI;
  } else {
    angle2 = angle1-PI;
  }
  em::rotate(m2,angle2,aux3,true);
  FFT2D fft2(aux3,AUX3); fft2.execute();
  RA = align2D_translational_no_preprocessing(M1,AUX3,m1_rows,m1_cols);
  algebra::Vector2D shift2 = RA.first.get_translation();
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  em::apply_Transformation2D(m2,transformation2,aux4,true);
  double ccc2=m1.cross_correlation_coefficient(aux4);
  if(ccc2>ccc1) {
    if(apply) {m2.copy(aux4);}
    IMP_LOG(IMP::VERBOSE," Align2D complete Transformation= "
      << transformation2 << " cross_correlation = " << ccc2 << std::endl);
    return ResultAlign2D(transformation2,ccc2);
  } else {
    if(apply) {m2.copy(aux2);}
    IMP_LOG(IMP::VERBOSE," Align2D complete Transformation= "
      << transformation1 << " cross_correlation = " << ccc1 << std::endl);
    return ResultAlign2D(transformation1,ccc1);
  }
}




ResultAlign2D align2D_rotational_no_preprocessing(
              algebra::Matrix2D_c &AUTOC_POLAR1,
              algebra::Matrix2D_c &AUTOC_POLAR2,
              unsigned int n_rings, unsigned int sampling_points) {

  IMP_LOG(IMP::TERSE,
    "starting 2D rotational alignment with no preprocessing" << std::endl);

  ResultAlign2D RA =align2D_translational_no_preprocessing(
               AUTOC_POLAR1,AUTOC_POLAR2,n_rings,sampling_points);

  algebra::Vector2D peak=RA.first.get_translation();
  double max_cc=RA.second;

  double Dang = 2*PI/(double)sampling_points;
  double angle =(-1)*peak[1]*Dang; // Peak[1] is the index for the optimal angle
  if(angle < 0 ) angle += 2*PI; // keep the angle between 0 and 2PI

  algebra::Transformation2D t(algebra::Vector2D(0.0,0.0));
  t.set_rotation(angle);
  IMP_LOG(IMP::VERBOSE,"Rotational Transformation= "
          << t << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
}



ResultAlign2D align2D_translational_no_preprocessing(
               algebra::Matrix2D_c &M1,
               algebra::Matrix2D_c &M2,
               unsigned int m1_rows,
               unsigned int m1_cols) {
  IMP_LOG(IMP::TERSE,
      "starting 2D translational alignment with no preprocessing" << std::endl);
  IMP_LOG(IMP::VERBOSE,
      "m1_rows = " << m1_rows << " m1_cols = " << m1_cols << std::endl);
  algebra::Matrix2D_d cross_corr;
  cross_corr.resize(m1_rows,m1_cols);
  em2d::correlation2D_no_preprocessing(M1,M2,cross_corr);
  // Find the peak of the cross_correlation
  double max_cc;
  algebra::Vector2D peak=peak_search(cross_corr,&max_cc);
  // Correct the shift applied for the fact that we are using the
  // cross correlation matrix as an image
  for(int i=0;i<2;++i) {
    int half=cross_corr.get_size(i)/2;
    peak[i]-=half;
  }
  algebra::Transformation2D t(peak);
  IMP_LOG(IMP::VERBOSE," Translational Transformation = "  << t
                  << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
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
//    algebra::Vector2D size3x3(3,3);
//    algebra::Vector2D bases(y-1,x-1);
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

ResultAlign2D align2D_complete_with_centers(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_d &m2,
                      bool apply,
                      int interpolation_method) {

    IMP_USAGE_CHECK((m1.get_number_of_rows()==m2.get_number_of_rows()) &&
                  (m1.get_number_of_columns()==m2.get_number_of_columns()),
        "em2d::align2D_complete_with_centers: Matrices have different size.");
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
  // Compute translation using the centers
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = cent1 - R.get_rotated(cent2);
  algebra::Transformation2D t(R,displacement);
  // Apply
  if(apply) {
    em::apply_Transformation2D(m2,t,cm2,true,0.0,0);
    m2.copy(cm2);
  }
  m1.set_start(orig1); m2.set_start(orig2);
  return ResultAlign2D(t,max_cc);
}


ResultAlign2D align2D_complete_with_centers_no_preprocessing(
                      algebra::Vector2D &center1,
                      algebra::Vector2D &center2,
                      algebra::Matrix2D_c &AUTOC_POLAR1,
                      algebra::Matrix2D_c &AUTOC_POLAR2,
                      unsigned int n_rings, unsigned int sampling_points) {
  // Align rotationally with FFT
  ResultAlign2D RA= align2D_rotational_no_preprocessing(AUTOC_POLAR1,
              AUTOC_POLAR2,n_rings,sampling_points);
  double angle = RA.first.get_rotation().get_angle();
  if (angle < 0) { angle += 2*PI; }
  // Compute translation using the centers
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = center1 - R.get_rotated(center2);
  algebra::Transformation2D t(R,displacement);
  return ResultAlign2D(t,RA.second);
}

IMPEM2D_END_NAMESPACE
