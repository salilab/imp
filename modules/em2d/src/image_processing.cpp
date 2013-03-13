/**
 *  \file image_processing.cpp
 *  \brief image processing for EM
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/CenteredMat.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/algebra/eigen_analysis.h"
#include "IMP/base/check_macros.h"
#include "IMP/exception.h"
#include "IMP/macros.h"
#include "IMP/base_types.h"
#include "boost/random.hpp"
#include "boost/version.hpp"
#include <list>
#include <queue>
#include <cmath>

IMPEM2D_BEGIN_NAMESPACE

void do_remove_small_objects(cvIntMat &m,
                          double percentage,
                          int background,
                          int foreground) {
  // Convert the image to integers in order to compute labels
  cvIntMat aux;
  int labels_found=do_labeling(m,aux); // aux contains the labels

  // Count pixels of each object
  Ints pixel_count(labels_found,0);
  for (cvIntMatIterator it = aux.begin();it!=aux.end();++it) {
    int val = (*it);
    if(val != background) pixel_count[val-1]++;
  }
  // Count the pixels of the largest object and the percentage in size of
  // The others
  unsigned int max_pixels = *max_element(pixel_count.begin(),pixel_count.end());
  Floats percentages;
  for(Ints::iterator it = pixel_count.begin(); it!=pixel_count.end(); ++it) {
    percentages.push_back(double(*it)/double(max_pixels));
  }
  // Remove objects of size lower than the percentage
  for (cvIntMatIterator it = aux.begin(); it != aux.end(); ++it) {
    int val= (*it);
    if(val != background) {
      (*it) = percentages[val-1]>percentage ? foreground : background;
    }
  }
  aux.copyTo(m);
}



void do_histogram_stretching(cv::Mat &m,
                             int boxes,
                             int offset) {

  // Number of possible values for the histogram and maximum value for
  // the stretched image
  double max_val,min_val;
  cv::minMaxLoc(m,&min_val,&max_val);
  double maxmin = max_val-min_val;
  // Histogram of boxes posible values
  Ints hist(boxes, 0);
  for (cvDoubleMatIterator it = m.begin<double>();it!=m.end<double>();++it) {
    double b = static_cast<double>(boxes);
    int k = algebra::get_rounded((b-1) * ((*it) -min_val)/(maxmin));
    hist[k]++;
  }

  // histogram mode value and index
  double h_mode_val = *std::max_element(hist.begin(),hist.end());
  double h_cut = 0.01*h_mode_val; // cut value
  // indexes of the histogram for the cut value
  int i_min=0,i_max=0;
  for (unsigned i=0;i<hist.size() ;++i ) {
    if(hist[i] > h_cut) {
      i_min=i;
      break;
    }
  }
  // must be int so it can go negative
  // and, unfortunately, unsigned - unsigned is still unsigned.
  for (int i=static_cast<int>(hist.size())-1;i>=0;--i ) {
    if(hist[i] > h_cut) {
      i_max=i;
      break;
    }
  }
  // Allow for some offset
  i_min -= offset; if(i_min <0) { i_min=0;}
  i_max += offset; if(i_max > (boxes-1)) { i_max=boxes-1; }

  // Min and max values for the new image
  max_val=min_val+(maxmin/(boxes-1))*(double(i_max));
  min_val=min_val+(maxmin/(boxes-1))*(double(i_min));
  maxmin = max_val-min_val;
  // Stretch
  for (cvDoubleMatIterator it = m.begin<double>();it!=m.end<double>();++it) {
    double val = ( *it - min_val) / maxmin;
    if(val < 0) val=0;
    if(val > 1) val=1;
    *it = (double(boxes)-1)*val;
  }
}

void do_dilate_and_shrink_warp(cv::Mat &m,
                                const cv::Mat &greyscale,
                                cv::Mat &kernel) {

  IMP_USAGE_CHECK((m.rows==greyscale.rows) && (m.cols==greyscale.cols),
        "do_dilate_and_shrink_warp: Matrices have different size.");
  int background = 0;
  int foreground = 1;
  cv::Mat mask,boundary,temp;
  m.copyTo(temp);
  unsigned int size_in_pixels,new_size_in_pixels;

  do {
    // pixels above background
    size_in_pixels=0;
    for (cvDoubleMatIterator it = temp.begin<double>();
                                          it!=temp.end<double>();++it) {
      if( algebra::get_rounded(*it) > background) size_in_pixels++;
    }
    cv::dilate(temp,mask,kernel);

    // Compute mean of the grayscale inside the mask and its size
    double mean=0.0;
    new_size_in_pixels = 0;
    for (int i=0;i<mask.rows;++i) {
      for (int j=0;j<mask.cols;++j) {
        if (algebra::get_rounded(mask.at<double>(i,j)) > background) {
          new_size_in_pixels++;
          mean += greyscale.at<double>(i,j);
        }
      }
    }
    mean /= (double)new_size_in_pixels++;
    cv::subtract(mask,temp,boundary);

//    xxx.set_data(boundary);
//    xxx.write("boundary.spi",srw);
    // Erode the mask if pixels in the grayscale are below the mean
    // and are in the boundary
    for (int i=0;i<greyscale.rows;++i) {
      for (int j=0;j<greyscale.cols;++j) {
        cvPixel p(i,j);
        if (algebra::get_rounded(mask.at<double>(p)) == background) {
          temp.at<double>(p) = background;
        } else if(algebra::get_rounded(boundary.at<double>(p))==foreground
                  && greyscale.at<double>(p)<mean) {
          // boundary pixel below the mean, erode
          temp.at<double>(p) = background;
          new_size_in_pixels--;
        } else {
          temp.at<double>(p) = foreground;
        }
      }
    }
    /*******/
//    xxx.set_data(temp);
//    xxx.write("temp.spi",srw);
    /*******/
  } while(std::abs(static_cast<int>(new_size_in_pixels)
                   -static_cast<int>(size_in_pixels))>1);
  // Now temp contains the new mask with size new_size_in_pixels
  temp.copyTo(m);
}



void apply_threshold(cv::Mat &m, cv::Mat &result, double threshold) {

  cv::Mat temp, temp2;
  m.convertTo(temp,CV_32FC1); // float, threshold does not work with doubles
  // Remove everything below threshold
  double dummy = 0.0;
  cv::threshold(temp, temp2, threshold, dummy, cv::THRESH_TOZERO);
  temp2.convertTo(result, CV_64FC1);
}


void do_combined_fill_holes_and_threshold(cv::Mat &m,
                                          cv::Mat &result,
                                          double n_stddevs,
                                          double threshold) {
  do_normalize(m);
  cv::Mat temp,temp2;
  do_fill_holes(m,temp,n_stddevs);
  do_normalize(temp);
  apply_threshold(temp, result, threshold);

//  temp.convertTo(temp2,CV_32FC1); // threshold does not work with doubles
//  // Remove everything below threshold
//  double dummy = 0.0
//  cv::threshold(temp2,temp,threshold, dummy,cv::THRESH_TOZERO);
//  temp.convertTo(result, CV_64FC1);
  do_normalize(result);
}

void do_morphologic_reconstruction(const cv::Mat &mask,
                                      cv::Mat &marker,
                                      int neighbors_mode) {

  IMP_USAGE_CHECK((mask.rows==marker.rows) && (mask.cols==marker.cols),
        "em2d::morphologic_reconstruction: Matrices have different size.");

  // Scan in raster order
  for (int i=0;i<mask.rows;++i) {
    for (int j=0;j<mask.cols;++j) {
      cvPixel p(i,j); // current pixel
      cvPixels neighbors=internal::get_neighbors2d(p,mask,neighbors_mode,1);
      Floats marker_values;
      marker_values.push_back(marker.at<double>(p));
      for (cvPixels::iterator it=neighbors.begin();it!=neighbors.end();++it) {
        marker_values.push_back(marker.at<double>(*it));
      }
      double max_val = *std::max_element(marker_values.begin(),
                                         marker_values.end());
      // Reconstruction
      marker.at<double>(p) = std::min(max_val,mask.at<double>(p));
    }
  }

  std::queue<cvPixel> propagated_pixels;
  // Scan in anti-raster order
  for (int i=mask.rows-1;i>=0;--i) {
    for (int j=mask.cols-1;j>=0;--j) {
      cvPixel p(i,j); // current pixel
      cvPixels neighbors=internal::get_neighbors2d(p,mask,neighbors_mode,-1);
      // Compute maximum
      double max_val = marker.at<double>(p);
      for (cvPixels::iterator it=neighbors.begin();it!=neighbors.end();++it) {
        max_val = std::max(marker.at<double>(*it),max_val);
        // Check if propagation is required
        if( (marker.at<double>(*it) <  marker.at<double>(p)) &&
            (marker.at<double>(*it) < mask.at<double>( *it))) {
          propagated_pixels.push(p);
        }
      }
      // Reconstruction
      marker.at<double>(p)=std::min(max_val,mask.at<double>(p));
    }
  }

  // Propagation step
  while(propagated_pixels.empty() == false ) {
    cvPixel p = propagated_pixels.front();
    propagated_pixels.pop();
    cvPixels neighbors=internal::get_neighbors2d(p,mask,neighbors_mode,0);
    // Reconstruction and propagation
    double delta=0.04;
    for (cvPixels::iterator it=neighbors.begin();it!=neighbors.end();++it) {
      if( (marker.at<double>(*it) < marker.at<double>(p)) &&
           abs(marker.at<double>(*it)-mask.at<double>(*it))> delta) {
        marker.at<double>(*it) = std::min(marker.at<double>(p),
                                          mask.at<double>(*it));
        propagated_pixels.push(*it);
      }
    }
  }
}



void do_fill_holes(const cv::Mat &m,cv::Mat &result,double h) {
  double max_m,min_m;
  cv::minMaxLoc(m,&min_m,&max_m);
  double max_plus_h = max_m +h;
  cv::Mat mask,marker;
  cv::Scalar s(max_plus_h,0,0,0);
  cv::subtract(s,m,mask); // this does max_plus_h-m
  // The result is the marker. It should be max_plus_m - m - h, but is the same
  cv::Scalar sc(max_m,0,0,0);
  cv::subtract(sc,m,marker); // this does max_m-m
  do_morphologic_reconstruction(mask,marker,8);
  cv::subtract(s,marker,result); // result = max_plus_h - marker
}



void get_domes(cv::Mat &m,cv::Mat &result,double h) {
  cv::Scalar s(h,0,0,0);
  cv::Mat marker;
  cv::subtract(m,h,marker);
  do_morphologic_reconstruction(m,marker,8);
  cv::subtract(m,marker,result);
}




void get_diffusion_filtering_partial_derivative(const cv::Mat &m,
                                                cv::Mat &der,
                                               double dx,
                                               double dy,
                                               double ang) {
  if(m.rows!=der.rows || m.cols!=der.cols)
    IMP_THROW("Diffusion filter: Derivatives matrix has incorrect size",
              ValueException);


  double c = cos(ang);
  double s = sin(ang);
  double dxdx = dx*dx;
  double dydy = dy*dy;
  double dxdy = dx*dy;
  double Ix,Iy,Ixx,Iyy,Ixy,h;
  cv::Mat_<double> M = m; // for convenience with the notation (does not copy)
  cv::Mat_<double> Der = der; // same here
  for (int i=0;i<M.rows;++i) {
    for (int j=0;j<M.cols;++j) {
      cvPixel p(i,j);
      cvPixels ns = internal::get_neighbors2d(p,m,8,0,true);
      // partial derivatives of m using finite differences
      Ix = (M(ns[2])-M(p))/dx;
      Iy = (M(ns[4])-M(p))/dy;
      h= 1/(1+Ix*Ix+Iy*Iy);   // edge indicator function (h)
      // Second derivatives with finite differences
      Ixx = (M(ns[2])+M(ns[6])-2*M(p))/dxdx;
      Iyy = (M(ns[4])+M(ns[0])-2*M(p))/dydy;
      Ixy = (M(ns[3])+M(ns[7])-M(ns[1])-M(ns[5]))/(4*dxdy);
      Der(p) = s*h*(Ixx+Iyy)+c*(-2)*h*h*(Ix*Ix*Ixx+Iy*Iy*Iyy+2*Ix*Iy*Ixy);
    }
  }
}


void apply_diffusion_filter(const cv::Mat &m,
                           cv::Mat &result,
                           double beta,
                           double pixelsize,
                           unsigned int time_steps) {
  if(beta<0 || beta>90)
    IMP_THROW("Diffusion filter: Beta not in interval [0,90]",ValueException);

  m.copyTo(result);
  double dx = pixelsize;
  double dy = pixelsize;
  double dt=0.5*(1/(dx*dx)+1/(dy*dy));
  double angle= beta * PI/180.0;
  // Integrate over time
  cv::Mat derivative_time(m.rows,m.cols,m.type());
  for (unsigned int t=0;t<time_steps;++t) {
    get_diffusion_filtering_partial_derivative(result,
                                               derivative_time,
                                               dx,
                                               dy,
                                               angle);
    result += derivative_time * dt;
  }
}

void do_segmentation(const cv::Mat &m,
                     cv::Mat &result,
                     const SegmentationParameters &params) {
  IMP_LOG_VERBOSE("Segmenting image" << std::endl);
  cv::Mat temp1,temp2; // to store doubles
  cv::Mat aux,aux2; // to store floats
  m.copyTo(temp1);
  do_normalize(temp1);
  apply_diffusion_filter(temp1, // input
                         temp2, // filtered
                         params.diffusion_beta,
                         params.image_pixel_size,
                         params.diffusion_timesteps);

  do_combined_fill_holes_and_threshold(temp2, //input
                                       temp1, // result with holes filled
                                       params.fill_holes_stddevs,
                                       params.threshold);
  // opening
  temp1.convertTo(aux,CV_32FC1);
  cv::morphologyEx(aux, // input
                   aux2, // result, opened
                   cv::MORPH_OPEN,
                   params.opening_kernel);
  // threshold the opened image to binary
  double threshold = 0.0; // for making binary images
  cv::threshold(aux2,aux,threshold,params.binary_foreground,cv::THRESH_BINARY);
  // convert to ints to remove small objects
  aux.convertTo(aux2,CV_16SC1); // aux2 now is ints
  cvIntMat Aux = aux2;
  do_remove_small_objects(Aux,
                          params.remove_sizing_percentage,
                          params.binary_background,
                          params.binary_foreground);
  apply_mask(m,result,Aux,0.0); // Aux is the mask
}

int do_labeling(const cvIntMat &m,
             cvIntMat &mat_to_label) {
  mat_to_label.create(m.rows,m.cols);
  Ints union_find_tree;
  internal::do_binary_matrix_scan(m,mat_to_label,union_find_tree);
  internal::do_flatten_labels(union_find_tree);
  internal::set_final_labels(mat_to_label,union_find_tree);
  int labels = *max_element(union_find_tree.begin(), union_find_tree.end());
  return labels;
}




void apply_mask(const cv::Mat &m,
                cv::Mat &result,
                const cvIntMat &mask,
                double val) {
  if(!(m.rows == mask.rows && m.cols == mask.cols ))
    IMP_THROW("apply_mask: The matrix and the mask have different size.",
              ValueException);
  result.create(m.rows,m.cols,m.type());
  cvDoubleMat M = m;
  cvDoubleMat R = result;
  for (int i=0;i<M.rows;++i) {
    for (int j=0;j<M.cols;++j) {
      R(i,j) = mask(i,j) == 0 ? val : M(i,j);
    }
  }
}


void apply_circular_mask(const cv::Mat &mat,
                         cv::Mat &result,
                         int radius,
                         double val) {
  cvIntMat mask = create_circular_mask(mat.rows, mat.cols, radius);
  apply_mask(mat, result, mask, val);
}


double get_mean(const cv::Mat &mat, const cvIntMat &mask) {
  IMP_USAGE_CHECK(mat.rows == mask.rows && mat.cols == mask.cols ,
                  "get_mean: The matrix and the mask have different size.");
  cvDoubleMat M = mat;
  double mean = 0;
  double pixels = 0;
  for (int i=0;i<mask.rows;++i) {
    for (int j = 0;j<mask.cols;++j) {
      if(mask(i,j) != 0) {
        pixels += 1;
        mean += M(i, j);
      }
    }
  }
  mean /= pixels;
  IMP_LOG_TERSE( "Mean within mask " << mean);

  return mean;
}

cvIntMat create_circular_mask(int rows, int cols, int radius) {
  if(rows < 0) IMP_THROW("Negative number of rows", ValueException);
  if(cols < 0) IMP_THROW("Negative number of columns", ValueException);
  if(radius < 0) IMP_THROW("Negative radius", ValueException);
//cv::Mat mask = cv::Mat::zeros(rows, cols, CV_16UC1);
  cv::Mat mask = cv::Mat::zeros(rows, cols, CV_64FC1);
  cv::Point center( floor(rows/2.), floor(cols/2.));
  cv::Scalar color(100, 0, 0, 0);
  cv::circle(mask, center, radius, color, -1);
  return mask;
}


Floats get_histogram(const cv::Mat &m, int bins) {
  Floats histogram(bins);
  double min,max;
  cv::minMaxLoc(m, &min,&max,nullptr,nullptr);
  // Step
  double step = (max - min) / static_cast<double>(bins);
  long n_points= m.rows * m.cols;
  for(cvDoubleConstMatIterator it=m.begin<double>();
                               it!=m.end<double>();++it ) {
    int index = floor((*it -min)/step);
    histogram[index] += 1.f / static_cast<double>(n_points);
  }
  return histogram;
}


void apply_variance_filter(const cv::Mat &input,
                           cv::Mat &filtered,
                           int kernelsize) {
  // Compute the mean for each value with a filter
  cv::Size ksize(kernelsize,kernelsize);
  cv::Point anchor(-1,-1);
//  cv::boxFilter(input, filtered,input.depth(),
//                ksize,anchor,true,cv::BORDER_WRAP);
  // Get the squared means
 IMP_LOG_VERBOSE("Getting squared means" << std::endl);

  cv::Mat means;
  cv::boxFilter(input, means,input.depth(),ksize,anchor,true);
  cv::Mat squared_means = means.mul(means);

  // Get the the means of squares
  IMP_LOG_VERBOSE("Getting means of the squares" << std::endl );

  cv::Mat squares = input.mul(input);
  // make filtered contain the means of the squares
  cv::boxFilter(squares, filtered,squares.depth(),ksize,anchor,true);
  // Variance
  filtered = filtered - squared_means;

  IMP_LOG_VERBOSE(
          "Adjusting variance for numerical instability " << std::endl);
  for (cvDoubleMatIterator it=filtered.begin<double>();
                                    it!=filtered.end<double>();++it) {
    if(*it < 0) *it = 0;
  }
}

IMP_GCC_PUSH_POP(diagnostic push)
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
void add_noise(cv::Mat &v,
               double op1,double op2, const String &mode, double /*df*/)
{
  IMP_LOG_TERSE( "Adding noise: mean " << op1
          << " Stddev " << op2 << std::endl);
  // Generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type generator;
  generator.seed(static_cast<unsigned long>(time(nullptr)));
  // Distribution types
  typedef boost::uniform_real<> unif_distribution;
  typedef boost::normal_distribution<> norm_distribution;
  // Variate generators (put generator and distributions together)
  typedef boost::variate_generator < base_generator_type&,
                         unif_distribution  > unif_var_generator;
  typedef boost::variate_generator < base_generator_type&,
                         norm_distribution  > norm_var_generator;
  if (mode == "uniform") {
    unif_distribution dist(op1, op2);
    unif_var_generator random_var(generator,dist);
    // Add the random numbers
    for (cvDoubleMatIterator it=v.begin<double>();it!=v.end<double>();++it) {
      *it += random_var();
    }
  } else if (mode == "gaussian") {
    norm_distribution dist(op1, op2);
    norm_var_generator random_var(generator, dist);
    // Add the random numbers
    for (cvDoubleMatIterator it=v.begin<double>();it!=v.end<double>();++it) {
      *it += random_var();
    }
  } else {
    IMP_THROW("Add_noise: Mode " + mode + " not supported.",ValueException);
  }
}
IMP_GCC_PUSH_POP(diagnostic pop)


void do_resample_polar(const cv::Mat &input, cv::Mat &resampled,
                    const PolarResamplingParameters &polar_params) {

  cv::Mat map_16SC2,map_16UC1;
  // If the resampling parameters are not initialized, build a polar map for
  // the resampling.
  if(polar_params.get_is_setup() == false) {
    PolarResamplingParameters p;
    p.setup(input.rows,input.cols);
    p.set_estimated_number_of_angles(std::min(input.rows,input.cols));
    p.create_maps_for_resampling();
    p.get_resampling_maps(map_16SC2,map_16UC1);
  } else {
    polar_params.get_resampling_maps(map_16SC2,map_16UC1);
  }
  cv::Mat temp,temp2,map2;
  input.convertTo(temp,CV_32FC1); // remap does not work with doubles, convert
  cv::remap(temp,temp2,map_16SC2,map_16UC1,cv::INTER_LINEAR,cv::BORDER_WRAP);
  temp2.convertTo(resampled,CV_64FC1); // convert back
}


void do_normalize(cv::Mat &m) {
  cv::Scalar mean,stddev;
  my_meanStdDev(m,mean,stddev); //cv::meanStdDev(m,mean,stddev);
  IMP_LOG_VERBOSE( "Matrix of mean: " << mean[0] << " stddev "
                  << stddev[0] << " normalized. " << std::endl);
  m = m - mean[0];
  m = m / stddev[0];
}

void my_meanStdDev(const cv::Mat &m,cv::Scalar &mean,cv::Scalar &stddev) {
//#if CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4
//  cv::meanStdDev(m,mean,stddev);
//#endif
//#if CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION > 3
  mean = cv::mean(m);
  cv::Mat square;
  cv::pow(m - mean[0],2,square);
  cv::Scalar sum = cv::sum(square);
  stddev[0] = std::sqrt(sum[0]/(m.rows*m.cols));
// #endif
}

//! Transform a matrix (the translation is interpreted as cols,rows)
void get_transformed(const cv::Mat &input,cv::Mat &transformed,
                                   const algebra::Transformation2D &T) {
  cv::Point2f center(input.rows/2.0,input.cols/2.0);
  double angle = 180.*T.get_rotation().get_angle()/PI;
  cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle,1.0);
  // Careful here. OpenCV convention interprets a translation as col,row
  rot_mat.at<double>(0,2) += T.get_translation()[0];
  rot_mat.at<double>(1,2) += T.get_translation()[1];
  cv::Size dsize(input.cols,input.rows);
  // WarpAffine calls remap, that does not work with doubles (CV_64FC1).
  // Use floats CV_32FC1
  cv::Mat temp,temp2;
  input.convertTo(temp,CV_32FC1);
  cv::warpAffine(temp,temp2,rot_mat,dsize,
                 cv::INTER_LINEAR,cv::BORDER_WRAP);
  temp2.convertTo(transformed,CV_64FC1); // convert back
}


// Using Centered Mat does not allow the first argument to be const.
void do_extend_borders(cv::Mat &orig, cv::Mat &dst,unsigned int pix) {
  dst.create(orig.rows+2*pix,orig.cols+2*pix,orig.type());
  dst.setTo(0.0);
  CenteredMat Orig(orig);
  CenteredMat Dst(dst);
  for (int i=Orig.get_start(0);i<=Orig.get_end(0);++i) {
    for (int j=Orig.get_start(1);j<=Orig.get_end(1);++j) {
      Dst(i,j)=Orig(i,j);
    }
  }
}

void do_morphologic_contrast_enhancement(const cv::Mat &m, cv::Mat &result,
                                        const cv::Mat &kernel,
                          unsigned int  iterations) {
  // Bovik, pg 143

  cv::Mat dilated, eroded, condition;
  cv::Mat temp;
  m.convertTo(temp,CV_32FC1); // dilate and erode don't work with doubles


  for (unsigned int i=0; i < iterations; ++i) {
    cv::dilate(temp, dilated, kernel);
    cv::erode(temp, eroded, kernel);
    condition = (eroded + dilated) * 0.5;
    for (int i = 0; i < temp.rows; ++i) {
      for (int j=0; j< temp.cols; ++j) {
        temp.at<float>(i,j) = temp.at<float>(i,j) < condition.at<float>(i,j) ?
                          eroded.at<float>(i,j) : dilated.at<float>(i,j);
      }
    }
  }
  temp.convertTo(result, CV_64FC1);
}

void get_morphologic_gradient(const cv::Mat &m, cv::Mat &result,
                                        const cv::Mat &kernel) {
  // Bovik, pg 147

  cv::Mat dilated, eroded, condition;
  cv::Mat temp;
  m.convertTo(temp, CV_32FC1); // dilate and erode don't work with doubles
  cv::dilate(temp, dilated, kernel);
  cv::erode(temp, eroded, kernel);
  temp = dilated - eroded;
  temp.convertTo(result, CV_64FC1);
}


double get_overlap_percentage(cv::Mat &m1, cv::Mat &m2,
                               const IntPair &center) {

  CenteredMat M1(m1, center.first, center.second);
  CenteredMat M2(m2);
  IMP_USAGE_CHECK((M2.get_start(0) < M1.get_start(0) ||
                   M2.get_start(1) < M1.get_start(1) ||
                   M2.get_end(0) > M1.get_end(0) ||
                   M2.get_end(1) > M1.get_end(1)),
     "em2d::get_overlap_percentage: Second Matrix is not contained in first");

  double pixels_m2 = 0;
  double pixels_overlap = 0;
  for (int i=M2.get_start(0); i < M2.get_end(0); ++i) {
    for (int j=M2.get_start(1); j< M2.get_end(1); ++j) {
      if(M2(i,j) > 0) {
        pixels_m2 += 1;
      } else {
        continue;
      }
      if(M1(i,j) > 0) pixels_overlap += 1;
    }
  }
  return pixels_overlap/pixels_m2;
}

MatchTemplateResults get_best_template_matches(const cv::Mat &m,
                          const cv::Mat &templ,
                          unsigned int  n) {
  cv::Mat result, aux_m, aux_templ;
  m.convertTo(aux_m,CV_32FC1); // float, matchTemplate
  templ.convertTo(aux_templ,CV_32FC1); // float, matchTemplate

  cv::matchTemplate(aux_m, aux_templ, result, CV_TM_CCORR_NORMED);
  /* Write the image with the correlations
  IMP_NEW(Image, img, ());
  cv::Mat temp;
  result.convertTo(temp,CV_64FC1);
  img->set_data(temp);
  IMP_NEW(SpiderImageReaderWriter, srw, ());
  img->write("matches_template.spi", srw);
  */
  std::list<cvPixel> locations;
  std::list<float> max_values;
  for (int i=0; i<result.rows; ++i) {
    for (int j=0; j<result.cols; ++j) {
      cvPixel p(i, j);
      float v = result.at<float>(p);
      if(locations.size() == 0) {
        locations.push_back(p);
        max_values.push_back( v);
      } else {
        std::list<float>::iterator itv = max_values.begin();
        std::list<cvPixel>::iterator itp = locations.begin();
        for ( ; itv != max_values.end(); ++itv, ++itp) {
          bool is_higher = v > *itv ? true : false;
          if( is_higher) {
            locations.insert(itp, p);
            max_values.insert(itv, v);
            if( max_values.size() > n) {
              locations.pop_back();
              max_values.pop_back();
            }
            break;
          }
        }
        if(itv == max_values.end() && (max_values.size() < n) ) {
          max_values.push_back( *itv);
          locations.push_back( *itp);
        }
      }
    }
  }

  std::list<cvPixel>::const_iterator itp = locations.begin();
  std::list<float>::const_iterator itv = max_values.begin();
  MatchTemplateResults best_locations;
  IMP_LOG_VERBOSE("Best template locations" << std::endl);

  // int l = 0;
  for (; itp != locations.end(); ++itp, ++itv) {
    IMP_LOG_VERBOSE( "pixel (" << itp->y << "," << itp->x
                                        << ") = " << *itv << std::endl);
    MatchTemplateResult info( IntPair(itp->y, itp->x), *itv);
    best_locations.push_back( info  );

    /* Write the template on top of the image
    cv::Mat mat;
    m.copyTo(mat);
    cv::Mat roi(mat, cv::Rect(itp->y, itp->x, templ.rows, templ.cols));
    cv::Mat temp = templ * 3;
    roi = roi + temp;
    IMP_NEW(Image, xxx, ());
    xxx->set_data(mat);
    std::ostringstream oss2;
    oss2 << "img-and-template-sol-" << l << ".spi";
    xxx->write(oss2.str(), srw);
    ++l;
    */
  }
  return best_locations;
}


cv::Mat crop(const cv::Mat &m, const IntPair &center, int size) {
  int s = std::floor(static_cast<double>(size)/2);
  int x_top_left = center.first - s + 1;
  int y_top_left = center.second - s + 1;

  if(center.first < 0 || center.first > m.rows || x_top_left < 0 ||
     (x_top_left+s) > m.rows) {
    IMP_THROW("First coordinate of center is out of bound or size is too big",
              ValueException);
  }
  if(center.second < 0 || center.second > m.cols || y_top_left < 0 ||
     (y_top_left+s) > m.cols) {
    IMP_THROW("Second coordinate of center is out of bound or size is too big",
                            ValueException);
  }
  cv::Rect region_of_interest(x_top_left, y_top_left, size, size );
  return m(region_of_interest);
}

IMPEM2D_END_NAMESPACE
