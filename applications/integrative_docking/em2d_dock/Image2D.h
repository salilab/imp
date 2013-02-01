/**
 *  \file Image2D.h   \brief A basic image class
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_IMAGE_2D_H
#define IMP_IMAGE_2D_H

#include "ImageTransform.h"

#include <IMP/algebra/Vector2D.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/base/utility.h>
#include <IMP/base/map.h>
#include <IMP/constants.h>

#include <boost/multi_array.hpp>
#include <boost/random.hpp>

#include <stack>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

namespace {
  int symm_round(double r) {
    return static_cast<int>(r > 0.0 ? floor(r + 0.5) : ceil(r - 0.5));
  }
}

template<class T = double>
class Image2D : public boost::multi_array<T, 2> {
public:
  // Constructors
  Image2D(): average_computed_(false), stddev_computed_(false) {}

  // Loads an image from the specified file
  // The file is assumed to contain an image in PGM format
  Image2D(const std::string& file_name): average_computed_(false),
                                         stddev_computed_(false)
  { read_PGM(file_name); }

  Image2D(const boost::multi_array<T, 2>& in):
    average_computed_(false), stddev_computed_(false) {
    this->resize(boost::extents[in.shape()[0]][in.shape()[1]]);
    for(unsigned int i=0; i<in.num_elements(); i++) {
      *(this->data()+i) = *(in.data()+i);
    }
  }

  Image2D(const Image2D& in) : boost::multi_array<T, 2>(),
    average_(in.average_), stddev_(in.stddev_),
    average_computed_(in.average_computed_),
    stddev_computed_(in.stddev_computed_), pca_(in.pca_), points_(in.points_) {
    this->resize(boost::extents[in.shape()[0]][in.shape()[1]]);
    for(unsigned int i=0; i<in.num_elements(); i++) {
      *(this->data()+i) = *(in.data()+i);
    }
  }

  Image2D(int height, int width): average_computed_(false),
                                  stddev_computed_(false)
  { this->resize(boost::extents[height][width]); }

  // IO functions
  void read_PGM(const std::string& file_name);
  void write_PGM(const std::string& file_name) const;
  void transform_write_PGM(const ImageTransform& transform,
                           const std::string& out_file_name) const;
  static void write_PGM(const std::vector<Image2D <> >& images,
                        const std::string& filename) ;

  // basic access functions
  int get_height() const { return this->shape()[0]; }
  int get_width() const { return this->shape()[1]; }

  // image transformations
  void rotate(Image2D<T>& out_image, double angle_RAD) const;
  void rotate_circular(Image2D<T>& out_image, double angle_RAD) const;
  void translate(Image2D<T>& out_image, int t_x=0, int t_y=0) const;
  void transform(Image2D<T>& out_image, double angle_RAD,
                 int t_x=0, int t_y=0) const;
  void transform(Image2D<T>& out_image, const ImageTransform& t) const;
  void pad(Image2D<T>& out_image, int new_width = 0, int new_height = 0,
           T pad_value = 0) const;
  void rotate(double angle_RAD);
  void rotate_circular(double angle_RAD);
  void translate(int t_x=0, int t_y=0);
  void transform(double angle_RAD, int t_x=0, int t_y=0);
  void pad(int new_width = 0, int new_height = 0, T pad_value = 0);
  void center();

  // image statistics
  double average() const;
  std::pair<double, double> weighted_average(T thr=0) const;
  double stddev() const;
  double cc_score(const Image2D<T>& other_image) const;
  double cc_score(const Image2D<T>& other_image,T thr ) const;
  double ncc_score(const Image2D<T>& other_image) const;
  double ncc_score(const Image2D<T>& other_image, T thr) const;
  void compute_PCA();

  // image operations.h
  void spherical_mask(Image2D<T>& out_image) const;
  void spherical_mask();
  void add_gaussian_noise(float mean, float std);
  void enhance(double weight);
  void normalize();

  void add(const Image2D<T>& image, double weight = 0.5);
  void convert_to_int(Image2D<int>& out_image) const;

  // image segmentation
  int get_segmentation_threshold() const;
  void get_connected_components(Image2D<int>& out_image) const;
  void get_largest_connected_component(Image2D<int>& out_image) const;
  void get_largest_connected_component();

  // image_filters.h
  void gaussian_blur(Image2D<T>& out_image, int sigma) const;
  void gaussian_blur(int sigma);

  ImageTransform pca_align(const Image2D<T>& other_image) const;
  double max_distance() const;
  int segmented_pixels() const { return (int)points_.size(); }

protected:
  double average_;
  double stddev_;
  bool average_computed_;
  bool stddev_computed_;
  IMP::algebra::PrincipalComponentAnalysis2D pca_;
  std::vector<IMP::algebra::Vector2D> points_; // segmented area
};

template<class T>
void Image2D<T>::read_PGM(const std::string& filename)
{
  std::ifstream infile(filename.c_str());
  if(!infile) {
    std::cerr << "Unable to open file " << filename << std::endl;
    return;
  }

  char c;
  char ftype = ' ';
  const int MAX_LENGTH = 1000;
  char line[MAX_LENGTH];
  while (infile.get(c)) {
    switch (c) {
     case '#':         // if comment line, read the whole line and move on
       infile.getline(line, MAX_LENGTH); break;
     case 'P': // format magick number
       infile.get(ftype); break;
     default:  // return the first character
       infile.putback(c);
       break;
    }
    if(ftype != ' ') break; // magick number read
  }

  int width, height, maxval;
  infile >> width >> height >> maxval;
  std::cerr << "Image width = " << width << " height = " << height
            << " max color val = " << maxval << std::endl;

  this->resize(boost::extents[height][width]);
  int gray_level;
  for(int i=0; i<height; i++) {
    for(int j=0; j<width; j++) {
      infile >> gray_level;
      (*this)[i][j] = (T)gray_level;
    }
  }
  infile.close();
}

template<class T>
void Image2D<T>::write_PGM(const std::string& filename) const
{
  std::ofstream outfile(filename.c_str());
  outfile << "P2" << std::endl;
  outfile << get_width() << " " << get_height() << std::endl;
  outfile << "255" << std::endl;

  // find min/max
  double max_value = std::numeric_limits<float>::min();
  double min_value = std::numeric_limits<float>::max();
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      max_value = std::max((double)(*this)[i][j], max_value);
      min_value = std::min((double)(*this)[i][j], min_value);
    }
  }
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      outfile << symm_round(255.0*((*this)[i][j]-min_value)
                            /(max_value-min_value));
      outfile << " ";
      if(j>0 && j%12 == 0) outfile << std::endl;
    }
    outfile << std::endl;
  }
  outfile.close();
}

template<class T>
void Image2D<T>::transform_write_PGM(const ImageTransform& transform,
                                     const std::string& out_file_name) const {
  Image2D<> transformed_image;
  rotate_circular(transformed_image, transform.get_angle());
  transformed_image.translate(transform.get_x(), transform.get_y());
  transformed_image.write_PGM(out_file_name);
}

template<class T>
void Image2D<T>::write_PGM(const std::vector<Image2D <> >& images,
                           const std::string& filename) {
  if(images.size() <= 0) return;
  std::ofstream outfile(filename.c_str());
  outfile << "P2" << std::endl;
  // assume same height and width for all images
  int width, height;
  if(images.size() >=10) {
    width = 10*images[0].get_width()+9;
  } else {
    int n = images.size();
    width = n*images[0].get_width()+n-1;
  }
  unsigned int rows_number = images.size()/10;
  if(images.size()%10>0) rows_number++;
  std::cerr << "height = " << rows_number
            << " all images "  << images.size() <<  std::endl;
  height= rows_number*images[0].get_height() + rows_number-1;
  outfile << width << " " << height << std::endl;
  outfile << "255" << std::endl;

  // convert each image to 0-255 color space
  std::vector<Image2D<int> > out_images(images.size());
  for(unsigned int image_index=0; image_index<images.size(); image_index++) {
    images[image_index].convert_to_int(out_images[image_index]);
  }

  // write 10 images in a row
  // iterate over 10 images
  for(unsigned int image_index=0; image_index<images.size(); image_index+=10) {
    for(int i=0; i<images[image_index].get_height(); i++) { // i goes over rows
      for(unsigned int k=0; k<10; k++) { // k iterates over images in one row
        if(image_index+k < images.size()) {
          // j goes over columns
          for(int j=0; j<images[image_index+k].get_width(); j++) {
            outfile << out_images[image_index+k][i][j];
            outfile << " ";
            if(j>0 && j%12 == 0) outfile << std::endl;
          }
          if(k != 9 && k!=images.size()-1 && image_index+k <= images.size()-1)
            outfile << " 255 "; // white frame at the end of each image row
          outfile << std::endl;
        } else {
          if(images.size() > 10) { // print black row
            for(int j=0; j<images[0].get_width(); j++) {  // j goes over columns
              outfile << "0 ";
              if(j>0 && j%12 == 0) outfile << std::endl;
            }
            if(k != 9) outfile << "255 ";// white frame at the end of each image
            outfile << std::endl;
          }
        }
      } // end image row iteration (k)
    }
    if(image_index%10 < rows_number-1) { // print white row
      for(int i=0; i<width; i++) {
        outfile << "255 ";
        if(i>0 && i%12 == 0) outfile << std::endl;
      }
      outfile << std::endl;
    }
  }
  outfile.close();
}

template<class T>
double Image2D<T>::average() const {
  if(!average_computed_) {
    T average = std::accumulate(this->data(),
                                this->data()+this->num_elements(), 0);
    Image2D<T>& i = const_cast<Image2D<T> &>(*this);
    i.average_ = (double)average/this->num_elements();
    i.average_computed_ = true;
  }
  return average_;
}

template<class T>
std::pair<double, double> Image2D<T>::weighted_average(T thr) const {
  double x=0.0, y=0.0, average=0.0;
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      if((*this)[i][j] > thr) {
        x += (*this)[i][j] * j;
        y += (*this)[i][j] * i;
        average += (*this)[i][j];
      }
    }
  }
  return std::make_pair(x/average, y/average);
}

template<class T>
double Image2D<T>::stddev() const {
  if(!stddev_computed_) {
    T avg_acc = 0, std_acc = 0;
    for(unsigned int i=0;i<this->num_elements();i++) {
      avg_acc += this->data()[i];
      std_acc += IMP::base::square(this->data()[i]);
    }
    double average = (double)avg_acc/this->num_elements();
    double stddev = (double)std_acc/this->num_elements();
    stddev -= IMP::base::square(average);
    Image2D<T>& i = const_cast<Image2D<T> &>(*this);
    i.stddev_ = sqrt(stddev);
    i.stddev_computed_ = true;
  }
  return stddev_;
}

template<class T>
double Image2D<T>::cc_score(const Image2D<T>& other_image) const {
  if(get_height() != other_image.get_height() &&
     get_width() != other_image.get_width()) {
    std::cerr << "Can't compute correlation for different size images"
              << get_height() << " vs. " << other_image.get_height()
              << get_width() << " vs. " << other_image.get_width() << std::endl;
    exit(1);
  }
  double cc = 0.0;
  for(int i=0; i<get_height(); i++)
    for(int j=0; j<get_width(); j++)
      cc += ((*this)[i][j] * other_image[i][j]);
  return cc;
}

template<class T>
double Image2D<T>::cc_score(const Image2D<T>& other_image, T thr) const {
  if(get_height() != other_image.get_height() &&
     get_width() != other_image.get_width()) {
    std::cerr << "Can't compute correlation for different size images"
              << get_height() << " vs. " << other_image.get_height()
              << get_width() << " vs. " << other_image.get_width() << std::endl;
    exit(1);
  }
  double cc = 0;
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      if((*this)[i][j] > thr && other_image[i][j] > thr)
        cc += ((*this)[i][j] * other_image[i][j]);
    }
  }
  return cc;
}

template<class T>
double Image2D<T>::ncc_score(const Image2D<T>& other_image) const {
  double ncc = cc_score(other_image);
  ncc -= this->num_elements() * average() * other_image.average();
  ncc /= this->num_elements() * stddev() * other_image.stddev();
  return ncc;
}

template<class T>
double Image2D<T>::ncc_score(const Image2D<T>& other_image, T thr) const {
  double ncc = cc_score(other_image, thr);
  ncc -= this->num_elements() * average() * other_image.average();
  ncc /= this->num_elements() * stddev() * other_image.stddev();
  return ncc;
}

template<class T>
void Image2D<T>::compute_PCA() {
  //  get_largest_connected_component();
  std::vector<double> weights; // segmented area values
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      if((*this)[i][j] > 0) {
        points_.push_back(IMP::algebra::Vector2D((double)j, (double)i));
        weights.push_back((*this)[i][j]);
      }
    }
  }

  // compute weighted mean
  double weights_sum = std::accumulate(weights.begin(), weights.end(), 0.0);
  IMP::algebra::Vector2D m(0.0, 0.0);
  for(unsigned int i=0; i<points_.size(); i++) m+= weights[i]*points_[i];
  m /= weights_sum;

  // subtract the mean
  std::vector<IMP::algebra::Vector2D> points(points_.size());
  for(unsigned int i=0; i<points.size(); i++) points[i] = points_[i]-m;

  // calculate covariance
  double varx(0.0), vary(0.0), varxy(0.0);
  for(unsigned int i=0; i<points.size(); i++) {
    varx+= weights[i]*points[i][0]*points[i][0];
    vary+= weights[i]*points[i][1]*points[i][1];
    varxy+= weights[i]*points[i][0]*points[i][1];
  }
  varxy/=weights_sum;
  varx/=weights_sum;
  vary/=weights_sum;

  // covariance matrix
  IMP::algebra::internal::TNT::Array2D<double> cov(2, 2);
  cov[0][0]=varx; cov[0][1]=varxy;
  cov[1][0]=varxy;cov[1][1]=vary;

  IMP::algebra::internal::JAMA::SVD<double> svd(cov);
  IMP::algebra::internal::TNT::Array2D<double> V(2, 2);
  IMP::algebra::internal::TNT::Array1D<double> SV;

  svd.getV(V);
  svd.getSingularValues(SV);
  //the principal components are the columns of V
  //pc1(pc2) is the vector of the largest(smallest) eigenvalue
  IMP::algebra::Vector2Ds pcs;
  pcs.push_back(IMP::algebra::Vector2D(V[0][0],V[1][0]));
  pcs.push_back(IMP::algebra::Vector2D(V[0][1],V[1][1]));
  pca_ = IMP::algebra::PrincipalComponentAnalysis2D(pcs,
                                IMP::algebra::Vector2D(SV[0],SV[1]), m);
}



template<class T>
void Image2D<T>::rotate(Image2D<T>& out_image, double angle_RAD) const {
  double cos_angle = std::cos(angle_RAD);
  double sin_angle = std::sin(angle_RAD);

  // when we rotate the image matrix grows
  int max_width = (int)abs((int)(get_width()*cos_angle)) +
    (int)abs((int)(get_height()*sin_angle));
  int max_height = (int)abs((int)(-get_width()*sin_angle)) +
    (int)abs((int)(get_height()*cos_angle));

  out_image.resize(boost::extents[max_height][max_width]);

  int max_t_x = max_width/2;
  int max_t_y = max_height/2;
  int t_x = get_width()/2;
  int t_y = get_height()/2;

  for(int i=0; i<max_height; i++) {
    for(int j=0; j<max_width; j++) {
      // translate
      int x = j - max_t_x;
      int y = i - max_t_y;
      // rotate (rotation around zero)
      double rx = x*cos_angle + y*sin_angle;
      double ry = y*cos_angle - x*sin_angle;
      // translate again
      rx += t_x;
      ry += t_y;
      if(rx>0 && ry>0 && ceil(rx)<(get_width()-1) && ceil(ry)<(get_height()-1)){
        // interpolate
        int jj = (int) floor(rx);
        int ii = (int) floor(ry);
        double t = rx - jj;
        double u = ry - ii;
        T value = (T)((1.0-t)*(1.0-u)*(*this)[ii][jj] +
                      t*(1.0-u)*(*this)[ii][jj+1] +
                      (1.0-t)*u*(*this)[ii+1][jj] + t*u*(*this)[ii+1][jj+1]);
        out_image[i][j] = value;
      }
    }
  }
}

template<class T>
void Image2D<T>::rotate_circular(Image2D<T>& out_image, double angle_RAD) const{
  double cos_angle = std::cos(angle_RAD);
  double sin_angle = std::sin(angle_RAD);

  out_image.resize(boost::extents[get_height()][get_width()]);
  int t_x = get_width()/2;
  int t_y = get_height()/2;

  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      // translate
      int x = j - t_x;
      int y = i - t_y;
      // rotate (rotation around zero)
      double rx = x*cos_angle + y*sin_angle;
      double ry = y*cos_angle - x*sin_angle;
      // translate again
      rx += t_x;
      ry += t_y;
      if(rx>0 && ry>0 && ceil(rx)<(get_width()-1) && ceil(ry)<(get_height()-1)){
        // interpolate
        int jj = (int) floor(rx);
        int ii = (int) floor(ry);
        double t = rx - jj;
        double u = ry - ii;
        T value = (T)((1.0-t)*(1.0-u)*(*this)[ii][jj] +
                      t*(1.0-u)*(*this)[ii][jj+1] +
                      (1.0-t)*u*(*this)[ii+1][jj] + t*u*(*this)[ii+1][jj+1]);
        out_image[i][j] = value;
      }
    }
  }
}

template<class T>
void Image2D<T>::rotate(double angle_RAD) {
  Image2D out_image;
  rotate(out_image, angle_RAD);
  this->resize(boost::extents[out_image.get_height()][out_image.get_width()]);
  (*this) = out_image;
}

template<class T>
void Image2D<T>::rotate_circular(double angle_RAD) {
  Image2D out_image;
  rotate_circular(out_image, angle_RAD);
  (*this) = out_image;
}

template<class T>
void Image2D<T>::translate(Image2D<T>& out_image, int t_x, int t_y) const {
  out_image.resize(boost::extents[get_height()][get_width()]);
  for(unsigned int i=0; i<out_image.num_elements(); i++)
    *(out_image.data()+i) = 0;

  for(int i=std::max(0, -t_y); i + t_y < get_height(); i++) {
    for(int j=std::max(0, -t_x); j + t_x < get_width(); j++) {
      if(i + t_y >= 0 && j + t_x >= 0 && i < get_height() && j < get_width()) {
        out_image[i+t_y][j+t_x] = (*this)[i][j];
      }
    }
  }
}

template<class T>
void Image2D<T>::translate(int t_x, int t_y) {
  Image2D out_image;
  translate(out_image, t_x, t_y);
  (*this) = out_image;
}

template<class T>
void Image2D<T>::transform(Image2D<T>& out_image, double angle_RAD,
                           int t_x, int t_y) const {
  rotate(out_image, angle_RAD);
  out_image.translate(t_x, t_y);
}

template<class T>
void Image2D<T>::transform(Image2D<T>& out_image,
                           const ImageTransform& t) const {
  rotate(out_image, t.get_angle());
  out_image.translate(t.get_x(), t.get_y());
}

template<class T>
void Image2D<T>::transform(double angle_RAD, int t_x, int t_y) {
  Image2D out_image;
  transform(out_image, angle_RAD, t_x, t_y);
  this->resize(boost::extents[out_image.get_height()][out_image.get_width()]);
  (*this) = out_image;
}

template<class T>
void Image2D<T>::pad(Image2D<T>& out_image, int new_width,
                     int new_height, T pad_value) const {
  // default frame
  if(new_width == 0) new_width = 2*get_width();
  if(new_height == 0) new_height = 2*get_height();

  // set sizes and init
  out_image.resize(boost::extents[new_height][new_width]);
  for(unsigned int i=0; i<out_image.num_elements(); i++)
    *(out_image.data()+i) = pad_value;

  int t_x = new_width/2 - get_width()/2;
  int t_y = new_height/2 - get_height()/2;
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      out_image[i+t_y][j+t_x] = (*this)[i][j];
    }
  }
}

template<class T>
void Image2D<T>::pad(int new_width, int new_height, T pad_value) {
  Image2D out_image;
  pad(out_image, new_width, new_height, pad_value);
  this->resize(boost::extents[out_image.get_height()][out_image.get_width()]);
  (*this) = out_image;
}

template<class T>
void Image2D<T>::center() {
  std::pair<double, double> center = weighted_average();
  int t_x = symm_round(get_width()/2 - center.first);
  int t_y = symm_round(get_height()/2 - center.second);
  translate(t_x, t_y);
}

template<class T>
int Image2D<T>::get_segmentation_threshold() const
{
  Image2D<int> out_image;
  convert_to_int(out_image);
  std::vector<int> histogram(256/4, 0);

  for(unsigned int i=0; i<out_image.num_elements(); i++) {
    int color = *(out_image.data()+i);
    histogram[color/4]++;
  }

  int average=0; //float std = 0;
  for(unsigned int i=0; i<histogram.size(); i++) {
    average+=histogram[i];
    //std += square(histogram[i]);
  }
  average/=histogram.size();
  // std/=histogram.size(); std -= square(average); std = sqrt(std);
  int peak_color=0;

  // find peaks
  for(unsigned int i=histogram.size()-1; i>=1; i--) {
    // we look for a large jump in the histogram
    if(histogram[i-1] - histogram[i] > average) {
      peak_color = i*4;
      break;
    }
  }

  if(peak_color == 0) { // try with lower thr
    for(unsigned int i=histogram.size()-1; i>=1; i--) {
      if(histogram[i-1] - histogram[i] > average/2) {
        peak_color = i*4;
        break;
      }
    }
  }
  return peak_color;
}


#include <utility>

namespace {
  typedef IMP::base::map<int, int> HashMap;
  typedef IMP::base::map<int, HashMap> DoubleHashMap;
  class EquivalenceTable : public DoubleHashMap {
  public:
    EquivalenceTable() { max_color_=0; }
    // add a pair of numbers
    void add(const int color1, const int color2) {
      DoubleHashMap::iterator iter1 = find(color1);
      if(iter1 != end()) {
        HashMap& hashMap = iter1->second;
        HashMap::iterator iter2 = hashMap.find(color2);
        if(iter2 == hashMap.end()) hashMap[color2] = 1;
      } else {
        // the given pair is a new one
        HashMap hashMap;
        hashMap.insert(std::make_pair(color2,1));
        (*this)[color1] = hashMap;
      }
      if(color2 > max_color_) max_color_ = color2;
    }

    void build_equivalence_classes(std::vector<std::set<int> >& classes) {
      // status of the current color: 0 - not processed, 1 - assigned a class,
      //2 - equivalent colors assigned the same class
      std::vector<int> colors(max_color_, 0);
      std::vector<int> color_classes(max_color_, -1); // pointers to classes
      std::stack<int> st; // colors that have to be processed

      // iterate colors
      for(int i=0; i<max_color_; i++) {
        int color1 = i+1;
        if(colors[i] == 0) { // not processed yet
          // start a new class
          std::set<int> new_class;
          new_class.insert(color1);
          classes.push_back(new_class);
          colors[i] = 1; // class assigned
          color_classes[i] = classes.size()-1;
          st.push(color1);
        }
        while (!st.empty()) {
          int color = st.top();
          st.pop();
          // go over all pairs of this color and add them to a class
          DoubleHashMap::iterator iter1 = find(color);
          if(iter1 != end()) {
            for(HashMap::iterator iter2 = iter1->second.begin();
                iter2 !=iter1->second.end(); iter2++) {
              int color2 = iter2->first;
              int j = color2-1;
              // set the class for color2
              if(colors[j] == 0) { // new color
                colors[j] = 1;
                color_classes[j] = color_classes[i]; // same class
                // add color2 to class
                classes[color_classes[i]].insert(color2);
                st.push(color2);
              }
            }
          }
          colors[i] = 2; // done
        }
      }
    }
  private:
    int max_color_;
  };
};

template<class T>
void Image2D<T>::get_connected_components(Image2D<int>& out_image) const {
  convert_to_int(out_image);
  int cc_number = 1;
  int thr = get_segmentation_threshold();
  if(thr == 0) return; // How to handle non-segmentable images?
  EquivalenceTable equivalent_colors;
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      if(out_image[i][j] >= thr && i>0 && j>0) {
        // option 1: no neighbours, start new component
        if(out_image[i-1][j] == 0 && out_image[i-1][j-1]==0 &&
           out_image[i][j-1]==0) {
          out_image[i][j]=cc_number;
          cc_number++;
        } else { // option2: connect to existing component
          int color=0;
          if(out_image[i-1][j] > 0) {
            out_image[i][j]=out_image[i-1][j];
            color = out_image[i-1][j];
          }
          if(out_image[i-1][j-1] > 0) {
            out_image[i][j]=out_image[i-1][j-1];
            // add to equivalent list
            if(color > 0 && color != out_image[i-1][j-1]) {
              equivalent_colors.add(color, out_image[i-1][j-1]);
              equivalent_colors.add(out_image[i-1][j-1], color);
            }
          }
          if(out_image[i][j-1] > 0) {
            out_image[i][j]=out_image[i][j-1];
            // add to equivalent list
            if(color > 0 && color != out_image[i][j-1]) {
              equivalent_colors.add(color, out_image[i][j-1]);
              equivalent_colors.add(out_image[i][j-1], color);
            }
          }
        }
      } else {
        out_image[i][j]=0;
      }
    }
  }
  //  out_image.write_PGM("cc.pgm");

  // go over equivalent colors
  std::vector<std::set<int> > classes;
  equivalent_colors.build_equivalence_classes(classes);

  for(unsigned int i=0; i<out_image.num_elements(); i++)
    if(*(out_image.data()+i) > 0)
      for(unsigned int ii=0; ii<classes.size(); ii++) {
        int first_color = *(classes[ii].begin());
        for(std::set<int>::iterator it = classes[ii].begin();
            it != classes[ii].end(); it++) {
          if(*(out_image.data()+i) == *it)  *(out_image.data()+i) = first_color;
        }
      }
  //  out_image.write_PGM("cc1.pgm");
}

template<class T>
void Image2D<T>::get_largest_connected_component(Image2D<int>& out_image) const{
  get_connected_components(out_image);
  // count the number of pixels in each cc
  IMP::base::map<int, int> counts;

  for(unsigned int i=0; i<out_image.num_elements(); i++) {
    int color = *(out_image.data()+i);
    if(color > 0) {
      if(counts.find(color) != counts.end()) counts[color]++;
      else counts[color] = 1;
    }
  }

  int largest_size = 0; int largest_color=1;
  for(IMP::base::map<int, int>::iterator it=counts.begin();
      it!=counts.end(); it++) {
    if(it->second > largest_size) {
      largest_size = it->second;
      largest_color = it->first;
    }
  }
  std::cerr << " largest_size = " << largest_size
            << " color " << largest_color << std::endl;

  for(unsigned int i=0; i<out_image.num_elements(); i++)
    if(*(out_image.data()+i) != largest_color) *(out_image.data()+i)=0;//-1;
    else *(out_image.data()+i) = *((*this).data()+i); //10
}

template<class T>
void Image2D<T>::get_largest_connected_component() {
  Image2D<int> out_image;
  get_connected_components(out_image);
  // count the number of pixels in each cc
  IMP::base::map<int, int> counts;

  for(unsigned int i=0; i<out_image.num_elements(); i++) {
    int color = *(out_image.data()+i);
    if(color > 0) {
      if(counts.find(color) != counts.end()) counts[color]++;
      else counts[color] = 1;
    }
  }

  int largest_size = 0; int largest_color=1;
  for(IMP::base::map<int, int>::iterator it=counts.begin();
      it!=counts.end(); it++) {
    if(it->second > largest_size) {
      largest_size = it->second;
      largest_color = it->first;
    }
  }

  for(unsigned int i=0; i<out_image.num_elements(); i++) {
    if(*(out_image.data()+i) != largest_color) *((*this).data()+i)=0;
  }
  //out_image.write_PGM("cc1.pgm");
}

template<class T>
void Image2D<T>::convert_to_int(Image2D<int>& out_image) const
{
  out_image.resize(boost::extents[get_height()][get_width()]);
  // find min/max
  double max_value = std::numeric_limits<float>::min();
  double min_value = std::numeric_limits<float>::max();
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      max_value = std::max((double)(*this)[i][j], max_value);
      min_value = std::min((double)(*this)[i][j], min_value);
    }
  }
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      out_image[i][j] =
        symm_round(255.0*((*this)[i][j]-min_value)/(max_value-min_value));
    }
  }
}

template<class T>
void Image2D<T>::add(const Image2D<T>& image, double weight) {
  for(int i=0; i<get_height(); i++) {
    for(int j=0; j<get_width(); j++) {
      if(i<image.get_height() && j<image.get_width())
        (*this)[i][j] += weight*image[i][j];
    }
  }
}

template<class T>
double Image2D<T>::max_distance() const {
  int max_dist2 = 0;
  for(int i=0; i<get_height(); i++)
    for(int j=0; j<get_width(); j++)
      if((*this)[i][j] > 0) { // compute distance to other pixels
        for(int ii=0; ii<get_height(); ii++)
          for(int jj=0; jj<get_width(); jj++)
            if((*this)[ii][jj] > 0 && i!=ii && j!=jj) {
              int dist2 = IMP::base::square(i-ii)+IMP::base::square(j-jj);
              if(dist2 > max_dist2) max_dist2 = dist2;
            }
      }
  return sqrt((double)max_dist2);
}


template<class T>
ImageTransform Image2D<T>::pca_align(const Image2D<T>& image) const {
  IMP::algebra::Vector2D translation =
    pca_.get_centroid() - image.pca_.get_centroid();
  IMP::algebra::Vector2D ev1 = pca_.get_principal_component(0);
  IMP::algebra::Vector2D ev2 = image.pca_.get_principal_component(0);
  double angle = atan2(ev1[1], ev1[0]) - atan2(ev2[1], ev2[0]);
  Image2D<> transformed_image1(image), transformed_image2(image);
  transformed_image1.rotate_circular(angle);
  transformed_image2.rotate_circular(angle+IMP::PI);
  // no need to translate, it is centered anyway
  // transformed_image1.translate(translation[0], translation[1]);
  // transformed_image2.translate(translation[0], translation[1]);
  double score1 = ncc_score(transformed_image1);
  double score2 = ncc_score(transformed_image2);
  double score = (score1>score2)?score1:score2;
  if(score1<=score2) angle = angle+IMP::PI;
  // std::cerr << "angle = " << RAD_2_DEG(angle) << " score " << score
  //           << " p1-p2 " << points1.size() << " - " << points2.size()
  //           << " % " << area_score << std::endl;
  return ImageTransform(angle, (int)translation[0],
                        (int)translation[1], score, 0.0); // 0 for area_score
}



#endif /* IMP_IMAGE_2D_H */
