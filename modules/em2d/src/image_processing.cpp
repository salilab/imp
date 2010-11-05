/**
 *  \file image_processing.cpp
 *  \brief image processing for EM
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2d/image_processing.h"
//#include "IMP/em2d/internal/radon.h"
#include "IMP/em/Image.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/em/filters.h"
#include "IMP/algebra/eigen_analysis.h"
#include "IMP/exception.h"
#include "IMP/macros.h"
#include <deque>
#include <list>
#include <queue>
#include <cmath>






IMPEM2D_BEGIN_NAMESPACE


void wiener_filter_2D(algebra::Matrix2D_d &m,
                      algebra::Matrix2D_d &result,
                      const unsigned int kernel_rows,
                      const unsigned int kernel_cols) {
  unsigned int rows=m.get_size(0);
  unsigned int cols=m.get_size(1);
  algebra::Matrix2D_d mean(rows,cols),variance(rows,cols);
//  mean.resize(rows,cols);
//  variance.resize(rows,cols);
  result.resize(rows,cols);
  // save origins and set to zero
  Pixel origin(m.get_start(0),m.get_start(1));
  Pixel zero(0,0); m.set_start(zero); result.set_start(zero);
  // Set kernel init and end
  Pixel k_init((int)floor(kernel_rows/2),(int)floor(kernel_cols/2));
  Pixel k_end(kernel_rows-k_init[0], kernel_cols-k_init[1]);
  double NM=(double)(kernel_rows*kernel_cols); // useful later
  for (unsigned int i=k_init[0];i<rows-k_end[0]+1;++i) {
    for (unsigned int j=k_init[1];j<cols-k_end[1]+1;++j) {
      // Apply the filter
      double mean_i=0, var_i=0;
      // Compute mean and variance in the kernel
      for (int ik=-k_init[0];ik<k_end[0];++ik) {
        for (int jk=-k_init[1];jk<k_end[1];++jk) {
          double value = m(i+ik,j+jk);
          mean_i+=value;
          var_i+=value*value;
         }
      }
      mean(i,j)=mean_i/NM;
      variance(i,j)=(var_i-mean(i,j)*mean(i,j))/NM;
    }
  }
  double avg_variance=variance.compute_avg() ;
//  std::cout << " avg variance: " << avg_variance << std::endl;
  // Filter
  for (unsigned int i=k_init[0];i<rows-k_end[0]+1;++i) {
    for (unsigned int j=k_init[1];j<cols-k_end[1]+1;++j) {
      result(i,j)=mean(i,j)+(1-avg_variance/variance(i,j))*(m(i,j)-mean(i,j));
    }
  }
  // Transfer the pixels that could not be filtered
  for (unsigned int j=0;j<cols;++j) {
    for (int i=0;i<k_init[0];++i) { result(i,j)=m(i,j); }
    for (unsigned int i=rows-k_end[0]+1;i<rows;++i) { result(i,j)=m(i,j); }
  }
  for (unsigned int i=0;i<rows;++i) {
    for (int j=0;j<k_init[1];++j) { result(i,j)=m(i,j); }
    for (unsigned int j=cols-k_end[1]+1;j<cols;++j) { result(i,j)=m(i,j); }
  }
  // restore origin and assign the same to the result
  m.set_start(origin);  result.set_start(origin);
}

void morphological_reconstruction(algebra::Matrix2D_d &mask,
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
      Pixels neighbors=compute_neighbors_2D(p,mask,neighbors_mode,1);
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
      Pixels neighbors =compute_neighbors_2D(p,mask,neighbors_mode,-1);
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
    Pixels neighbors=compute_neighbors_2D(p,mask,neighbors_mode,0);
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


Pixels compute_neighbors_2D(const Pixel &p,const algebra::Matrix2D_d &m,
                            const int mode,const int sign,const bool cycle) {
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



void fill_holes(algebra::Matrix2D_d &m,
                algebra::Matrix2D_d &result,double h) {
  algebra::Matrix2D_d  mask;
  double max_m = m.compute_max();
  double max_plus_h = max_m +h;
  mask = max_plus_h - m;
  // The result is the marker. It should be max_plus_m - m - h, but is the same
  result = max_m - m;
  morphological_reconstruction(mask,result,8);
  result = max_plus_h - result;
}


void get_domes(algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,double h) {
  result = m - h;
  morphological_reconstruction(m,result,8);
  result = m - result;

}


void preprocess_em2d(algebra::Matrix2D_d &m,
                     algebra::Matrix2D_d &result,
                     double n_stddevs) {
  m.normalize();
  fill_holes(m,result,1.0); // no standard devs. Fill holes of depth 1
  result.normalize();
  em::FilterByThreshold<double,2> thres;
  thres.set_parameters(0,0); // Threshold 0, clean everything below
//  thres.set_parameters(1,0); // Threshold 1, clean everything below
  thres.apply(result);
  result.normalize();
}


void dilation(const algebra::Matrix2D_d &m,
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


void erosion(const algebra::Matrix2D_d &m,
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


void opening(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &kernel,
             algebra::Matrix2D_d &result) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
            (m.get_number_of_columns()==result.get_number_of_columns()),
          "em2d::opening: Matrices have different size.");

  algebra::Matrix2D_d temp(m);
  erosion(m,kernel,temp);
  dilation(temp,kernel,result);
}


void closing(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &kernel,
             algebra::Matrix2D_d &result) {

  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::colsing: Matrices have different size.");

  algebra::Matrix2D_d temp(m);
  dilation(m,kernel,temp);
  erosion(temp,kernel,result);
}


void thresholding(const algebra::Matrix2D_d &m,
             algebra::Matrix2D_d &result,
             const double threshold,const int mode) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::thresholding: Matrices have different size.");


  for(unsigned int i=0;i<m.num_elements();++i) {
    if((mode ==  1 && m.data()[i]>threshold) ||
       (mode == -1 && m.data()[i]<threshold) ) {
      result.data()[i] = 1;
    } else {
      result.data()[i] = 0;
    }
  }
}

void masking(const algebra::Matrix2D_d &m,algebra::Matrix2D_d &result,
          const algebra::Matrix2D<int> &mask,double value) {
  IMP_USAGE_CHECK((m.get_number_of_rows()==result.get_number_of_rows()) &&
                  (m.get_number_of_columns()==result.get_number_of_columns()),
                  "em2d::masking: Matrices have different size.");

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
/**
  \param[in] I input
  \param[in] deriv output derivative
  \param[in] temp_x temporary matrix to store dI/dx
  \param[in] deriv_y temporary matrix to store dI/dy
  \param[in] h temporary matrix to store the "edge indicator function"
  \param[in] dx - step for x
  \param[in] dy - step for y
  \param[in] ang - parameter for weight diffusion and edge detection (90-0)
**/
void diffusion_filtering_partial_der_t(
                      const algebra::Matrix2D_d &I,
                      algebra::Matrix2D_d &It,
                      const double dx,const double dy,const double ang) {
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
      Pixels ns = compute_neighbors_2D(p,I,8,0,true);
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



void diffusion_filtering(const algebra::Matrix2D_d &I,
             algebra::Matrix2D_d &result,
             const double beta,
             const double pixelsize,
             const unsigned int t_steps) {

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
//    diffusion_filtering_partial_der_t(result,deriv_t,deriv_x,deriv_y,h,
//                                      dx,dy,ang);
    diffusion_filtering_partial_der_t(result,deriv_t,dx,dy,ang);
    for (int i=init_y;i<=end_y;++i) {
      for (int j=init_x;j<=end_x;++j) {
        result(i,j) += deriv_t(i,j)*dt;
      }
    }
  }
}


void dilate_and_shrink_warp(algebra::Matrix2D_d &m,
                            const algebra::Matrix2D_d &greyscale,
                            algebra::Matrix2D_d &kernel) {


  IMP_USAGE_CHECK((m.get_number_of_rows()==greyscale.get_number_of_rows()) &&
            (m.get_number_of_columns()==greyscale.get_number_of_columns()),
            "em2d::dilate_an_shrink: Matrices have different size.");

  int background = 0;
  int foreground = 1;
  algebra::Matrix2D_d mask, temp,boundary;
  mask.reshape(m);
  temp.reshape(m);
  boundary.reshape(m);
  temp.copy(m);
  algebra::Matrix2D_d temp_kernel(3,3);
  temp_kernel.set_zero();

  /********/
  em::SpiderImageReaderWriter<double> srw;
  /********/

  unsigned int size_in_pixels,new_size_in_pixels;
  do {
    size_in_pixels=0;
    for (unsigned int i=0;i<temp.num_elements();++i) {
      if (algebra::get_rounded(temp.data()[i])>background) { size_in_pixels++; }
    }
    // Dilate to get a new mask
    dilation(temp,kernel,mask);
    /********/
//    em::Image xxx;
//    xxx.set_data(mask);
//    xxx.write_to_floats("mask.spi",srw);
    /********/
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
//    xxx.write_to_floats("boundary.spi",srw);
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
    /********/
//    xxx.set_data(temp);
//    xxx.write_to_floats("temp.spi",srw);
    /********/
  } while( abs(new_size_in_pixels-size_in_pixels)>1);
//  } while( new_size_in_pixels != size_in_pixels);
  m.copy(temp);
}


void histogram_stretching(algebra::Matrix2D_d &m,
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


Floats get_histogram(em2d::Image &img, int bins) {
  return get_histogram(img.get_data(),bins);
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


void apply_variance_filter(em2d::Image &input,
                           em2d::Image &filtered,int kernelsize) {
  apply_variance_filter(input.get_data(),filtered.get_data(),kernelsize);
}


void apply_variance_filter(
            const cv::Mat &input,cv::Mat &filtered,int kernelsize) {
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

IMPEM2D_END_NAMESPACE
