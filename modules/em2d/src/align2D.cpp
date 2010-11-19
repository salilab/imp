/**
 *  \file align2D.copp
 *  \brief Align operations for EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/align2D.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/scores2D.h"
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


ResultAlign2D align2D_complete(const cv::Mat &input,
          cv::Mat &m_to_align,bool apply) {
  IMP_LOG(IMP::TERSE,"starting complete 2D alignment with OpenCV matrices"
                                                               << std::endl);
  cv::Mat autoc1,autoc2,aux1,aux2,aux3;
  algebra::Transformation2D transformation1,transformation2;
  ResultAlign2D RA;
  autocorrelation2D(input,autoc1);
  autocorrelation2D(m_to_align,autoc2);
  RA=align2D_rotational(autoc1,autoc2,false);
  double angle1 = RA.first.get_rotation().get_angle();
  get_transformed(m_to_align,aux1,RA.first); // rotate
  RA=align2D_translational(input,aux1);
  algebra::Vector2D shift1 = RA.first.get_translation();
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  get_transformed(m_to_align,aux2,transformation1);
  double ccc1=cross_correlation_coefficient(input,aux2);
  // Check for both angles that can be the solution
  double angle2;
  if (angle1 < PI) {
    angle2 = angle1+PI;
  } else {
    angle2 = angle1-PI;
  }
  // rotate
  algebra::Rotation2D R2(angle2);
  algebra::Transformation2D tr(R2);
  get_transformed(m_to_align,aux3,tr);

  RA = align2D_translational(input,aux3);
  algebra::Vector2D shift2=RA.first.get_translation();
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  get_transformed(m_to_align,aux3,transformation2);
  double ccc2=cross_correlation_coefficient(input,aux3);
  if(ccc2>ccc1) {
    if(apply) {aux3.copyTo(m_to_align);}
    IMP_LOG(IMP::VERBOSE," Transformation= "  << transformation2
                    << " cross_correlation = " << ccc2 << std::endl);
    return em2d::ResultAlign2D(transformation2,ccc2);
  } else {
    if(apply) {aux2.copyTo(m_to_align);}
    IMP_LOG(IMP::VERBOSE," Transformation= "  << transformation1
                    << " cross_correlation = " << ccc1 << std::endl);
    return em2d::ResultAlign2D(transformation1,ccc1);
  }
}



ResultAlign2D align2D_rotational(const cv::Mat &input,
                          cv::Mat &m_to_align,bool apply) {
  IMP_LOG(IMP::TERSE,
          "starting 2D rotational alignment" << std::endl);
  IMP_USAGE_CHECK((input.rows==m_to_align.rows) &&
                  (input.cols==m_to_align.cols),
                  "em2d::align_rotational: Matrices have different size.");

  cv::Mat polar1,polar2,corr;
  // Build maps for resampling
  PolarResamplingParameters polar_params(input.rows,input.cols);
  polar_params.set_estimated_number_of_angles(std::min(input.rows,input.cols));

  polar_params.build_maps_for_resampling();
  resample_polar(input,polar1,polar_params);  // subject image
  resample_polar(m_to_align,polar2,polar_params); // projection image

  ResultAlign2D RA= align2D_translational(polar1,polar2);
  algebra::Vector2D shift=RA.first.get_translation();
  // column shift[0] is the optimal angle to bring m_to_align to input
  double angle =shift[0]*polar_params.get_angle_step();
  RA.first.set_rotation(angle);
  RA.first.set_translation(algebra::Vector2D(0.0,0.0));
  // Apply the rotation if requested
  if(apply) {
    cv::Mat result;
    get_transformed(m_to_align,result,RA.first);
    result.copyTo(m_to_align);
  }
  IMP_LOG(IMP::VERBOSE,"Rotational alingment: Transformation= "
          << RA.first << " cross_correlation = " << RA.second << std::endl);
  return RA;
}




ResultAlign2D align2D_translational(const cv::Mat &input,
                           cv::Mat &m_to_align,
                            bool apply) {
  IMP_LOG(IMP::TERSE, "starting 2D translational alignment" << std::endl);
  IMP_USAGE_CHECK(
            (input.rows==m_to_align.rows) &&
            (input.cols==m_to_align.cols),
                  "em2d::align_translational: Matrices have different size.");
  cv::Mat corr;
  correlation2D(input,m_to_align,corr);
  double max_cc;
  algebra::Vector2D peak = peak_search(corr,&max_cc);
  // Convert the pixel with the maximum to a shift respect to the center
  algebra::Vector2D shift(peak[0]-(double)corr.cols/2.,
                          peak[1]-(double)corr.rows/2.);
  algebra::Transformation2D t(shift);
  // Apply the shift if requested
  if(apply) {
    cv::Mat result;
    get_transformed(m_to_align,result,t);
    result.copyTo(m_to_align);
  }
  IMP_LOG(IMP::VERBOSE," Transformation= "
          << t << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
}



ResultAlign2D align2D_complete_no_preprocessing(const cv::Mat &input,
                      const cv::Mat &INPUT,const cv::Mat &POLAR1,
                      cv::Mat &m_to_align,const cv::Mat &POLAR2,bool apply) {

  IMP_LOG(IMP::TERSE,
          "starting complete 2D alignment with no preprocessing" << std::endl);

  cv::Mat aux1,aux2,aux3,aux4; //auxiliary matrices
  cv::Mat AUX1,AUX2,AUX3; // ffts
  algebra::Transformation2D transformation1,transformation2;
  double angle1=0,angle2=0;
  ResultAlign2D RA = align2D_rotational_no_preprocessing(POLAR1,POLAR2);
  angle1 = RA.first.get_rotation().get_angle();
  get_transformed(m_to_align,aux1,RA.first); // rotate
  get_fft_using_optimal_size(aux1,AUX1);
  RA = align2D_translational_no_preprocessing(INPUT,AUX1);
  algebra::Vector2D shift1 = RA.first.get_translation();
  transformation1.set_rotation(angle1);
  transformation1.set_translation(shift1);
  get_transformed(m_to_align,aux2,transformation1); // rotate
  double ccc1=cross_correlation_coefficient(input,aux2);
  // Check the opposed angle
  if (angle1 < PI) {
    angle2 = angle1+PI;
  } else {
    angle2 = angle1-PI;
  }
  algebra::Rotation2D R2(angle2);
  algebra::Transformation2D tr(R2);
  get_transformed(m_to_align,aux3,tr); // rotate
  get_fft_using_optimal_size(aux3,AUX3);

  RA = align2D_translational_no_preprocessing(INPUT,AUX3);
  algebra::Vector2D shift2 = RA.first.get_translation();
  transformation2.set_rotation(angle2);
  transformation2.set_translation(shift2);
  get_transformed(m_to_align,aux3,transformation2);
  double ccc2=cross_correlation_coefficient(input,aux3);

  if(ccc2>ccc1) {
    if(apply) {aux3.copyTo(m_to_align);}
    IMP_LOG(IMP::VERBOSE," Align2D complete Transformation= "
      << transformation2 << " cross_correlation = " << ccc2 << std::endl);
    return ResultAlign2D(transformation2,ccc2);
  } else {
    if(apply) {aux3.copyTo(m_to_align);}
    IMP_LOG(IMP::VERBOSE," Align2D complete Transformation= "
      << transformation1 << " cross_correlation = " << ccc1 << std::endl);
    return ResultAlign2D(transformation1,ccc1);
  }
}


ResultAlign2D align2D_rotational_no_preprocessing(const cv::Mat &POLAR1,
                                                  const cv::Mat &POLAR2) {
  IMP_LOG(IMP::TERSE,
    "starting 2D rotational alignment with no preprocessing" << std::endl);

  IMP_USAGE_CHECK(((POLAR1.rows==POLAR2.rows) && (POLAR1.cols==POLAR2.cols)),
    "align2D_rotational_no_preprocessing: Matrices have different size.");

  ResultAlign2D RA =align2D_translational_no_preprocessing(POLAR1,POLAR2);
  algebra::Vector2D shift=RA.first.get_translation();
  // The number of columns of the polar matrices
  // are the number of angles considered. Init a PolarResamplingParameters
  // to get the angle_step
  PolarResamplingParameters polar_params;
  polar_params.set_estimated_number_of_angles(POLAR1.cols);
  double angle =shift[0]*polar_params.get_angle_step();
  RA.first.set_rotation(angle);
  RA.first.set_translation(algebra::Vector2D(0.0,0.0));
  IMP_LOG(IMP::VERBOSE,"Rotational Transformation= "
          << RA.first << " cross_correlation = " << RA.second << std::endl);
  return RA;
}


ResultAlign2D align2D_translational_no_preprocessing(const cv::Mat &M1,
                                                     const cv::Mat &M2) {
  IMP_LOG(IMP::TERSE,
      "starting 2D translational alignment with no preprocessing" << std::endl);
  IMP_USAGE_CHECK(((M1.rows==M2.rows) && (M1.cols == M2.cols)),
    "align2D_translational_no_preprocessing: Matrices have different size.");

  cv::Mat corr;
  corr.create(M1.rows,M1.cols,CV_64FC1);
  correlation2D_no_preprocessing(M1,M2,corr); // corr must be allocated!
  // Find the peak of the cross_correlation
  double max_cc;
  algebra::Vector2D peak = peak_search(corr,&max_cc);

  // Convert the pixel with the maximum to a shift respect to the center
  algebra::Vector2D shift(peak[0]-(double)corr.cols/2.,
                          peak[1]-(double)corr.rows/2.);
  algebra::Transformation2D t(shift);
  IMP_LOG(IMP::VERBOSE," Translational Transformation = "  << t
                  << " cross_correlation = " << max_cc << std::endl);
  return ResultAlign2D(t,max_cc);
}



algebra::Vector2D peak_search(cv::Mat &m,double *value) {
  // Find maximum value and location
  IMP_LOG(IMP::VERBOSE,"starting peak seach on a matrix " << std::endl);

  algebra::Vector2D peak;
  double minVal,maxVal;
  cv::Point minLoc,maxLoc;
  cv::minMaxLoc(m, &minVal,&maxVal, &minLoc,&maxLoc);
  *value = maxVal;
  // Perform a weighted centroiding with the neighbours to find the actual
  // maximum value. Performs as well as parabolic fit (Paulo, Opt. Eng. 2007)
  // Careful here. I interpret translations as row,col, and OpenCV as col,row
  int col0=0,row0=0;
  int coln=m.cols-1;
  int rown=m.rows-1;
  int col=maxLoc.x,row=maxLoc.y;

  if((row==row0 && col==col0) || (col==col0 && row==rown) ||
     (col==coln && row==row0) || (col==coln && row==rown) ) {
    // For corners just return the values
    peak[0]=col; peak[1]=row;
  } else if(row==row0 || row==rown) {
    // row borders, average
    peak[1]=row;
    double w1 = m.at<double>(row,col-1);
    double w2 = m.at<double>(row,  col);
    double w3 = m.at<double>(row,col+1);
    peak[0]=((col-1) *w1 + col*w2 + (col+1) * w3)/(w1+w2+w3);
  } else if(col==col0 || col==coln) {
    // Column borders, average
    double w1 = m.at<double>(row-1,col);
    double w2 = m.at<double>(row  ,col);
    double w3 = m.at<double>(row+1,col);
    peak[1]=((row-1)*w1+row*w2+(row+1)*w3)/(w1+w2+w3);
    peak[0]=col;
  } else {
    // Points inside the matrix
    unsigned int row_origin = row-1;
    unsigned int col_origin = col-1;
    // Weight on a region 3x3
    cv::Mat region(m,cv::Rect(col_origin,row_origin,3,3));
    algebra::Vector2D v = get_weighted_centroid(region);
    peak[0]=col_origin + v[0];
    peak[1]=row_origin + v[1];
  }
  return peak;
}




algebra::Vector2D get_weighted_centroid(const cv::Mat &m) {
  algebra::Vector2D center(0.,0.);
  double denominator=0.0;
  for (int i=1;i<=m.cols;++i) {
    for (int j=1;j<=m.rows;++j) {
      double value = m.at<double>(i-1,j-1);
      denominator+= value;
      center[0] += value*i;
      center[1] += value*j;
    }
  }
  // Adjust center for the fact that the  indices of m start at 0.
  center = center/denominator;
  // Following OpenCV convention the center is (col,row)
  center[0] -= 1;
  center[1] -= 1;
  return center;
}




void print_vector(std::vector< std::complex<double> > &v) {
  for (unsigned int i=0;i<v.size();++i) {
     std::cout << "i " << i << " v[i] =" << v[i] << std::endl;
  }
}



ResultAlign2D align2D_complete_with_centers_no_preprocessing(
                  const algebra::Vector2D &center1,
                  const algebra::Vector2D &center2,
                  const cv::Mat &AUTOC_POLAR1,
                  const cv::Mat &AUTOC_POLAR2) {
  // Align rotationally with FFT
  ResultAlign2D RA= align2D_rotational_no_preprocessing(AUTOC_POLAR1,
                                                        AUTOC_POLAR2);
  double angle = RA.first.get_rotation().get_angle();
  if (angle < 0) {
    angle += 2*PI;
  }
  // Compute translation using the centers
  algebra::Rotation2D R(angle);
  algebra::Vector2D displacement = center1 - R.get_rotated(center2);
  algebra::Transformation2D t(R,displacement);
  return ResultAlign2D(t,RA.second);
}

IMPEM2D_END_NAMESPACE
