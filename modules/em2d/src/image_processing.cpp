/**
 *  \file image_processing.cpp
 *  \brief image processing for EM
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/CenteredMat.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/em/filters.h"
#include "IMP/algebra/eigen_analysis.h"
#include "IMP/exception.h"
#include "IMP/macros.h"
#include "boost/random.hpp"
#include "boost/version.hpp"
#include <deque>
#include <list>
#include <queue>
#include <cmath>

IMPEM2D_BEGIN_NAMESPACE


void do_morphological_reconstruction(algebra::Matrix2D_d &mask,
                      algebra::Matrix2D_d &marker,
                      int neighbors_mode) {

  IMP_USAGE_CHECK((mask.get_number_of_rows()==marker.get_number_of_rows()) &&
            (mask.get_number_of_columns()==marker.get_number_of_columns()),
        "em2d::morfological_reconstruction: Matrices have different size.");
  unsigned int rows = mask.get_size(0);
  unsigned int cols = mask.get_size(1);
  int dims[2]; dims[0]=rows;dims[1]=cols;
  // save origin
  Pixel origin(mask.get_start(0),mask.get_start(1));
  Pixel zero(0,0);
  mask.set_start(zero);
  marker.set_start(zero);
  // Scan in raster order
  for (unsigned int i=0;i<rows;++i) {
    for (unsigned int j=0;j<cols;++j) {
      Pixel p(i,j); // current pixel
      Pixels neighbors=get_neighbors2d(p,mask,neighbors_mode,1);
      neighbors.push_back(p);
      // Compute maximum
      double max_val = marker(neighbors[0]);
      for (unsigned int k=1;k<neighbors.size();++k) {
        if(marker(neighbors[k]) > max_val) { max_val = marker(neighbors[k]); }
      }
      // Reconstruction
      marker(p)=std::min(max_val,mask(p));
    }
  }

  std::queue<Pixel,std::deque<Pixel> > propagated_pixels;
  // Scan in anti-raster order
  for (int i=rows-1;i>=0;--i) {
    for (int j=cols-1;j>=0;--j) {
      Pixel p(i,j); // current pixel
      Pixels neighbors =get_neighbors2d(p,mask,neighbors_mode,-1);
      // Compute maximum
      double pixel_val = marker(p);
      double max_val = pixel_val;
      for (unsigned int k=0;k<neighbors.size();++k) {
        if(marker(neighbors[k]) > max_val) { max_val = marker(neighbors[k]); }
        // Check if propagation is required
        if(  (marker(neighbors[k]) < pixel_val) &&
             (marker(neighbors[k]) < mask(neighbors[k]) )  ) {
          propagated_pixels.push(p);
        }
      }
      // Reconstruction
      marker(p)=std::min(max_val,mask(p));
    }
  }

  // Propagation step
  while(propagated_pixels.empty() == false ) {
    Pixel p = propagated_pixels.front();
    propagated_pixels.pop();
    Pixels neighbors=get_neighbors2d(p,mask,neighbors_mode,0);
    // Reconstruction and propagation
    for (unsigned int k=0;k<neighbors.size();++k) {
      if(  (marker(neighbors[k]) < marker(p)) &&
           (!algebra::get_are_almost_equal(marker(neighbors[k]),
                                   mask(neighbors[k]),1e-4)) ) {
        marker(neighbors[k]) = std::min(marker(p),mask(neighbors[k]));
        propagated_pixels.push(neighbors[k]);
      }
    }
  }
  // restore origin and assign the same to the result
  mask.set_start(origin);  marker.set_start(origin);
}


Pixels get_neighbors2d(const Pixel &p,const algebra::Matrix2D_d &m,
                            int mode,int sign,bool cycle) {
  Pixels neighbors,final_neighbors;
  switch(mode) {
   case 4:
    switch(sign) {
     case 0:
      neighbors.resize(4);
      neighbors[0]=p+Pixel(-1, 0);
      neighbors[1]=p+Pixel( 0, 1);
      neighbors[2]=p+Pixel( 1, 0);
      neighbors[3]=p+Pixel( 0,-1);
      break;
     case -1:
      neighbors.resize(2);
      neighbors[0]=p+Pixel( 0, 1);
      neighbors[1]=p+Pixel( 1, 0);
      break;
     case 1:
      neighbors.resize(2);
      neighbors[0]=p+Pixel(-1, 0);
      neighbors[1]=p+Pixel( 0,-1);
      break;
    }
    break;
   case 8:
    switch(sign) {
     case 0:
      neighbors.resize(8);
      neighbors[0]=p+Pixel(-1, 0);
      neighbors[1]=p+Pixel(-1, 1);
      neighbors[2]=p+Pixel( 0, 1);
      neighbors[3]=p+Pixel( 1, 1);
      neighbors[4]=p+Pixel( 1, 0);
      neighbors[5]=p+Pixel( 1,-1);
      neighbors[6]=p+Pixel( 0,-1);
      neighbors[7]=p+Pixel(-1,-1);
      break;
     case 1:
      neighbors.resize(4);
      neighbors[0]=p+Pixel(-1, 0);
      neighbors[1]=p+Pixel(-1, 1);
      neighbors[2]=p+Pixel( 0,-1);
      neighbors[3]=p+Pixel(-1,-1);
      break;
     case -1:
      neighbors.resize(4);
      neighbors[0]=p+Pixel( 0, 1);
      neighbors[1]=p+Pixel( 1, 1);
      neighbors[2]=p+Pixel( 1, 0);
      neighbors[3]=p+Pixel( 1,-1);
      break;
    }
    break;
  }
  if(cycle) {
    // Cycle indexes out of the matrix
    for(unsigned int i=0;i<neighbors.size();++i) {
      Pixel q=neighbors[i];
      if( q[0]<m.get_start(0) ) { q[0] = m.get_finish(0); }
      if( q[0]>m.get_finish(0) ) { q[0] = m.get_start(0); }
      if( q[1]<m.get_start(1) ) { q[1] = m.get_finish(1); }
      if( q[1]>m.get_finish(1) ) { q[1] = m.get_start(1); }
      final_neighbors.push_back(q);
    }
  } else {
    // Clean neighbors with indexes out of the matrix
    for(unsigned int i=0;i<neighbors.size();++i) {
      Pixel q=neighbors[i];
      if( q[0]>=m.get_start(0)  && q[1]>=m.get_start(1) &&
          q[0]<=m.get_finish(0) && q[1]<=m.get_finish(1)) {
        final_neighbors.push_back(q);
      }
    }
  }
  return final_neighbors;
}



void do_fill_holes(algebra::Matrix2D_d &m,
                algebra::Matrix2D_d &result,double h) {
  algebra::Matrix2D_d  mask;
  double max_m = m.compute_max();
  double max_plus_h = max_m +h;
  mask = max_plus_h - m;
  // The result is the marker. It should be max_plus_m - m - h, but is the same
  result = max_m - m;
  do_morphological_reconstruction(mask,result,8);
  result = max_plus_h - result;
}


void get_domes(algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,double h) {
  result = m - h;
  do_morphological_reconstruction(m,result,8);
  result = m - result;

}


void do_preprocess_em2d(algebra::Matrix2D_d &m,
                     algebra::Matrix2D_d &result,
                     double n_stddevs) {
  m.normalize();
  do_fill_holes(m,result,1.0); // no standard devs. Fill holes of depth 1
  result.normalize();
  em::FilterByThreshold<double,2> thres;
  thres.set_parameters(0,0); // Threshold 0, clean everything below
//  thres.set_parameters(1,0); // Threshold 1, clean everything below
  thres.apply(result);
  result.normalize();
}


void do_dilation(const algebra::Matrix2D_d &m,
              algebra::Matrix2D_d &kernel,
              algebra::Matrix2D_d &result) {
  // Prepare the kernel
  Pixel korigin(kernel.get_start(0),kernel.get_start(1));
  kernel.centered_start();
  // Do the dilation
  for (int i=m.get_start(0);i<=m.get_finish(0);++i) {
    for (int j=m.get_start(1);j<=m.get_finish(1);++j) {
      double max_val=m(i,j)+kernel(0,0);
      for (int ki=kernel.get_start(0);ki<=kernel.get_finish(0);++ki) {
        for (int kj=kernel.get_start(1);kj<=kernel.get_finish(1);++kj) {
          Pixel p(i+ki,j+kj);
          if(m.is_logical_element(p)) {
            double val = m(p)+kernel(ki,kj);
            if(val>max_val) { max_val = val;}
          }
        }
      }
      result(i,j)=max_val;
    }
  }
  // Set the origins to the previous values
  kernel.set_start(korigin);
}


void do_erosion(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &kernel,
             algebra::Matrix2D_d &result) {
  // Prepare the kernel
  Pixel korigin(kernel.get_start(0),kernel.get_start(1));
  kernel.centered_start();
  // Do the dilation
  for (int i=m.get_start(0);i<=m.get_finish(0);++i) {
    for (int j=m.get_start(1);j<=m.get_finish(1);++j) {
      double min_val=m(i,j)-kernel(0,0);
      for (int ki=kernel.get_start(0);ki<=kernel.get_finish(0);++ki) {
        for (int kj=kernel.get_start(1);kj<=kernel.get_finish(1);++kj) {
          Pixel p(i+ki,j+kj);
          if(m.is_logical_element(p)) {
            double val = m(p)-kernel(ki,kj);
            if(val<min_val) {min_val = val;}
          }
        }
      }
      result(i,j)=min_val;
    }
  }
  // Set the origins to the previous values
  kernel.set_start(korigin);
}


void do_opening(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &kernel,
             algebra::Matrix2D_d &result) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
            (m.get_number_of_columns()==result.get_number_of_columns()),
          "em2d::do_opening: Matrices have different size.");

  algebra::Matrix2D_d temp(m);
  do_erosion(m,kernel,temp);
  do_dilation(temp,kernel,result);
}


void do_closing(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &kernel,
             algebra::Matrix2D_d &result) {

  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::colsing: Matrices have different size.");

  algebra::Matrix2D_d temp(m);
  do_dilation(m,kernel,temp);
  do_erosion(temp,kernel,result);
}


void apply_threshold(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &result,
             double threshold,int mode) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::apply_threshold: Matrices have different size.");


  for(unsigned int i=0;i<m.num_elements();++i) {
    if((mode ==  1 && m.data()[i]>threshold) ||
       (mode == -1 && m.data()[i]<threshold) ) {
      result.data()[i] = 1;
    } else {
      result.data()[i] = 0;
    }
  }
}

void do_masking(const algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,
          const algebra::Matrix2D<int> &mask,double value) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::do_masking: Matrices have different size.");

  for (unsigned int i=0;i<m.num_elements();++i) {
    if(mask.data()[i]==1) {
      result.data()[i] = m.data()[i];
    } else {
      result.data()[i] = value;
    }
  }
}



//! Partial derivative with respect to time for an image filtered with
//! difusion-reaction
/*!
  \param[in] I input
  \param[in] deriv output derivative
  \param[in] temp_x temporary matrix to store dI/dx
  \param[in] deriv_y temporary matrix to store dI/dy
  \param[in] h temporary matrix to store the "edge indicator function"
  \param[in] dx - step for x
  \param[in] dy - step for y
  \param[in] ang - parameter for weight diffusion and edge detection (90-0)
*/
void get_diffusion_filtered_partial_der_t(
                      const algebra::Matrix2D_d &I,
                      algebra::Matrix2D_d &It,
                       double dx, double dy, double ang) {
  int init_y = I.get_start(0); int end_y = I.get_finish(0);
  int init_x = I.get_start(1); int end_x = I.get_finish(1);
  double c = cos(ang);
  double s = sin(ang);
  double dxdx = dx*dx;
  double dydy = dy*dy;
  double dxdy = dx*dy;
  double Ix,Iy,Ixx,Iyy,Ixy,h;
  // Compute derivatives and result at the same time. Normal cases:
  for (int i=init_y;i<=end_y;++i) {
    for (int j=init_x;j<=end_x;++j) {
      Pixel p(i,j);
      Pixels ns = get_neighbors2d(p,I,8,0,true);
      // partial derivatives of I(x,y) using finite differences
      Ix = (I(ns[2])-I(p))/dx;
      Iy = (I(ns[4])-I(p))/dy;
      h= 1/(1+Ix*Ix+Iy*Iy);   // edge indicator function (h)
      // Second derivatives with finite differences
      Ixx = (I(ns[2])+I(ns[6])-2*I(p))/dxdx;
      Iyy = (I(ns[4])+I(ns[0])-2*I(p))/dydy;
      Ixy = (I(ns[3])+I(ns[7])-I(ns[1])-I(ns[5]))/(4*dxdy);
      It(p) = s*h*(Ixx+Iyy)+c*(-2)*h*h*(Ix*Ix*Ixx+Iy*Iy*Iyy+2*Ix*Iy*Ixy);
    }
  }
}



void get_diffusion_filtered(const algebra::Matrix2D_d &I,
             algebra::Matrix2D_d &result,
              double beta,
              double pixelsize,
              unsigned int t_steps) {

  algebra::Matrix2D_d deriv_t,deriv_x,deriv_y,h;
  deriv_x.reshape(I);
  deriv_y.reshape(I);
  deriv_t.reshape(I);
  h.reshape(I);
  result = I;
  double dx=pixelsize;
  double dy=pixelsize;
  double dt=0.5*(1/(dx*dx)+1/(dy*dy));
  double ang = beta*PI/180.0;
  int init_y = I.get_start(0); int end_y = I.get_finish(0);
  int init_x = I.get_start(1); int end_x = I.get_finish(1);

  // Integrate over time a number of steps
  for (unsigned t=0;t<t_steps;++t) {
//    get_diffusion_filtered_partial_der_t(result,deriv_t,deriv_x,deriv_y,h,
//                                      dx,dy,ang);
    get_diffusion_filtered_partial_der_t(result,deriv_t,dx,dy,ang);
    for (int i=init_y;i<=end_y;++i) {
      for (int j=init_x;j<=end_x;++j) {
        result(i,j) += deriv_t(i,j)*dt;
      }
    }
  }
}


void do_dilate_and_shrink_warp(algebra::Matrix2D_d &m,
                            const algebra::Matrix2D_d &greyscale,
                            algebra::Matrix2D_d &kernel) {


  IMP_USAGE_CHECK((m.get_number_of_rows()==greyscale.get_number_of_rows()) &&
            (m.get_number_of_columns()==greyscale.get_number_of_columns()),
            "em2d::do_dilate_an_shrink: Matrices have different size.");

  int background = 0;
  int foreground = 1;
  algebra::Matrix2D_d mask, temp,boundary;
  mask.reshape(m);
  temp.reshape(m);
  boundary.reshape(m);
  temp.copy(m);
  algebra::Matrix2D_d temp_kernel(3,3);
  temp_kernel.set_zero();
  unsigned int size_in_pixels,new_size_in_pixels;
  do {
    size_in_pixels=0;
    for (unsigned int i=0;i<temp.num_elements();++i) {
      if (algebra::get_rounded(temp.data()[i])>background) { size_in_pixels++; }
    }
    // Dilate to get a new mask
    do_dilation(temp,kernel,mask);
    // Compute mean of the grayscale inside the mask and its size
    double mean=0.0;
    new_size_in_pixels = 0;
    for (unsigned int i=0;i<mask.num_elements();++i) {
      if (algebra::get_rounded(mask.data()[i])>background) {
        new_size_in_pixels++;
        mean += greyscale.data()[i];
      }
    }
    mean /= (double)new_size_in_pixels++;

    boundary = mask - temp;
//    xxx.set_data(boundary);
//    xxx.write("boundary.spi",srw);
    // Erode the mask if pixels in the grayscale are below the mean
    // and are in the boundary
    for (unsigned int i=0;i<greyscale.num_elements();++i) {
      if(algebra::get_rounded(mask.data()[i]) == background) {
        // pixel outside the mask
        temp.data()[i] = background;
      } else if(algebra::get_rounded(boundary.data()[i])==foreground &&
                greyscale.data()[i]<mean) {
        // boundary pixel below the mean, erode
        temp.data()[i] = background;
        new_size_in_pixels--;
      } else {
        temp.data()[i] = foreground;
      }
    }
    // Now temp contains the new mask with size new_size_in_pixels
    /*******/
//    xxx.set_data(temp);
//    xxx.write("temp.spi",srw);
    /*******/
  } while( std::abs(static_cast<int>(new_size_in_pixels)
                    -static_cast<int>(size_in_pixels))>1);
//  } while( new_size_in_pixels != size_in_pixels);
  m.copy(temp);
}


void do_histogram_stretching(algebra::Matrix2D_d &m,
                          int boxes,int offset) {
  // Number of possible values for the histogram and maximum value for
  // the stretched image
  double max_val = m.compute_max();
  double min_val = m.compute_min();
  double maxmin = max_val-min_val;
  // Histogram of boxes posible values
  std::vector<int> hist(boxes);
  for (unsigned i=0;i<hist.size() ;++i ) { hist[i]=0;}
  for (unsigned i=0;i<m.num_elements() ;++i ) {
    double xx = m.data()[i];
    int j = algebra::get_rounded(((double)boxes-1)*(xx-min_val)/(maxmin));
//    int j = (boxes-1)*algebra::round((m.data()[i]-min_val)/(maxmin));
    hist[j]++;
  }
  // histogram mode value and index
  std::vector<int>::iterator it;
  it = std::max_element(hist.begin(),hist.end());
  double h_mode_val = (double)*it;
  // cut value
  double h_cut = 0.01*h_mode_val;
  // indexes of the histogram for the cut value
  int i_min=0,i_max=0;
  for (unsigned i=0;i<hist.size() ;++i ) {
    if(hist[i] > h_cut) {
      i_min=i;
      break;
    }
  }
  for (unsigned i=hist.size()-1;i>=0;--i ) {
    if(hist[i] > h_cut) {
      i_max=i;
      break;
    }
  }
  // Allow for some offset
  i_min -= offset; if(i_min <0) { i_min=0;}
  i_max += offset; if(i_max > (boxes-1)) { i_max=boxes-1; }
  // Min and max values for the new image
  max_val=min_val+(maxmin/(boxes-1))*((double)i_max);
  min_val=min_val+(maxmin/(boxes-1))*((double)i_min);
  maxmin = max_val-min_val;
  // Stretch
  for (unsigned i=0;i<m.num_elements() ;++i ) {
//    m.data()[i] = ((double)boxes-1)*(m.data()[i]-min_val)/maxmin;
    double val = (m.data()[i]-min_val)/maxmin;
    if(val < 0) { val = 0; }
    if(val > 1) { val = 1; }
    m.data()[i] = ((double)boxes-1)*val;
  }
}



Floats get_histogram(const cv::Mat &m, int bins) {
  Floats histogram(bins);
  double min,max;
  cv::minMaxLoc(m, &min,&max,NULL,NULL);
  // Step
  double step = (max-min)/(double)bins;
  double n_points= (double)m.rows*(double)m.cols;
  for(cv::MatConstIterator_<double> it = m.begin<double>();
            it != m.end<double>(); ++it ) {
    int index = floor((*it -min)/step);
    histogram[index] += 1.f/n_points;
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
 IMP_LOG(IMP::VERBOSE,"Getting squared means" << std::endl);

  cv::Mat means;
  cv::boxFilter(input, means,input.depth(),ksize,anchor,true);
  cv::Mat squared_means = means.mul(means);

  // Get the the means of squares
  IMP_LOG(IMP::VERBOSE,"Getting means of the squares" << std::endl );

  cv::Mat squares = input.mul(input);
  // make filtered contain the means of the squares
  cv::boxFilter(squares, filtered,squares.depth(),ksize,anchor,true);
  // Variance
  filtered = filtered - squared_means;

  IMP_LOG(IMP::VERBOSE,
          "Adjusting variance for numerical instability " << std::endl);
  for (CVDoubleMatIterator it=filtered.begin<double>();
                                it!=filtered.end<double>();++it) {
    if(*it < 0) {
      *it = 0;
    }
  }
}


void add_noise(cv::Mat &v,double op1,double op2, const String &mode, double df)
{
  // Generator
  typedef boost::mt19937 base_generator_type;
  base_generator_type generator;
  generator.seed(static_cast<unsigned long>(time(NULL)));
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
    for (CVDoubleMatIterator it=v.begin<double>();
                                it!=v.end<double>();++it) {
      *it += random_var();
    }
  } else if (mode == "gaussian") {
    norm_distribution dist(op1, op2);
    norm_var_generator random_var(generator, dist);
    // Add the random numbers
    for (CVDoubleMatIterator it=v.begin<double>();
                                it!=v.end<double>();++it) {
      *it += random_var();
    }
  } else {
    IMP_THROW("Add_noise: Mode " + mode + " not supported.",ValueException);
  }
}


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
  temp2.convertTo(resampled,CV_64FC1);
}




void do_normalize(cv::Mat &m) {
  cv::Scalar mean,stddev;
  cv::meanStdDev(m,mean,stddev);
  IMP_LOG(IMP::VERBOSE, "Matrix of mean: " << mean[0] << " stddev "
                  << stddev[0] << " normalized. " << std::endl);
  m = m - mean[0];
  m = m / stddev[0];
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
  cv::Size dsize(input.rows,input.cols);
  // WarpAffine calls remap, that does not work with doubles (CV_64FC1).
  // Use floats CV_32FC1
  cv::Mat temp,temp2;
  input.convertTo(temp,CV_32FC1);
  cv::warpAffine(temp,temp2,rot_mat,dsize,
                 cv::INTER_LINEAR,cv::BORDER_WRAP);
  temp2.convertTo(transformed,CV_64FC1);
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




IMPEM2D_END_NAMESPACE
