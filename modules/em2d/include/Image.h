/**
 *  \file em2d/Image.h
 *  \brief IMP images for Electron Microscopy using openCV matrices
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_IMAGE_H
#define IMPEM2D_IMAGE_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em2d/ImageReaderWriter.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em/ImageHeader.h"
#include <IMP/Pointer.h>
#include <IMP/Object.h>
#include "IMP/Object.h"
#include "IMP/base_types.h"
#include <limits>
#include <typeinfo>






IMPEM2D_BEGIN_NAMESPACE

//! 2D Electron Microscopy images in IMP
class IMPEM2DEXPORT Image : public IMP::base::Object
{
public:

  Image();

  /**
   * Create an empty image
   * @param rows The number of rows
   * @param cols The number of columns
   */
  Image(int rows, int cols);

  /**
   * Create an image reading from a file
   * @param filename The name of the file containing the image
   * @param reader The image reader to use
   */
  Image(String filename, const ImageReaderWriter *reader):
    Object("Image%1%"){
    read(filename,reader);
  }

  /**
   * Access to the matrix of data
   * @return A reference to the matrix of data
   */
  cv::Mat &get_data() {
    return data_;
  }

  /**
   * Sets the entire data of the matrix
   * @param mat CV matrix containing the data for an image
   */
  void set_data(const cv::Mat &mat);

  /**
   * Sets the image to zero
   */
  void set_zeros() {
    set_value(0);
  }

  /**
   * Set all the pixels of an image
   * @param val Value to use
   * xxx
   */
  void set_value(double val) {
    cv::Scalar s(val,0,0,0);
    data_ = s;
  }

  /**
   * Sets the value for a pixel
   * @param i row
   * @param j column
   * @param val value
   */
  void set_value(int i, int j, double val) {
    data_.at<double>(i,j) = val;
  }

  /**
   * Recover the value of a pixel
   * @param i row
   * @param j colum
   * @return the value
   */
  double operator()(int i, int j) const {
    return data_.at<double>(i,j);
  }


  /**
   * Access to the image header
   * @return A reference to the header with the EM parameters
   */
  inline em::ImageHeader& get_header() {
    update_header();
    return header_;
  }

  /**
   * Sets the size of the image
   * @param rows
   * @param cols
   */
  void set_size(int rows,int cols);

  /**
   * Resize to the same size of the parameter image
   * @param img The image to get the size from
   */
  void set_size(Image *img);

  //! Adjusts the information of the imager header taking into account the
  //! dimensions of the data and setting the time, date, type, etc ...
  /**
   * Adjust the information of the image header taking into account the
   * dimensions of the data and setting the time, date, type, etc ...
   */
  void update_header();

  /**
   * Read an image from file
   * @param filename The name of the file containing the image
   * @param reader The image reader to use
   */
  void read(String filename, const ImageReaderWriter *reader) {
    Pointer<const ImageReaderWriter> ptr(reader);
    reader->read(filename,header_,data_);
  }

  //! Writes the image to a file (the image matrix of data is stored as floats
  //! when writing)
  /**
   * Writes the image to a file (the image matrix of data is stored as floats
   * when writing)
   * @param filename The output file
   * @param writer The image writer to use
   */
  void write(const String &filename, const ImageReaderWriter *writer) {
    update_header(); // adjust the header to guarantee consistence
    writer->write(filename,header_,data_);
  }

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void show(std::ostream& out) const {
    out << "Image name   : " << name_ ;
    header_.show(out);
  }

  //! Define the basic things needed by any Object
//  IMP_OBJECT_METHODS(Image)
  IMP_OBJECT_METHODS(Image);

  void destroyed_msg() {
    IMP_LOG_TERSE( "Image destroyed " << this->name_ << std::endl);
  }

  /**
   * Set the name of the image
   * @param name
   */
  void set_name(const String &name) {
    name_ = name;
  }

  /**
   * Get the name of the image
   * @return A string with the name
   */
  String get_name() const {
    return name_;
  }

  /**
   * Recover the minimum and maximum values in the image
   * @return A range. The first value is the minimum. The scond one is the
   * maximum.
   */
  FloatRange get_min_and_max_values() const;


protected:
  void set_size_data(int rows,int cols);
  void set_defaults();

  //! Name of the image. Frequently it will be the name of the file
  String name_;
  //! Matrix with the data for the image
//  cv::Mat data_;
  cv::Mat data_;
  //! Header for the image with all the pertinent information
  em::ImageHeader header_;

}; // Image

//! A vector of reference counted pointers to EM images of type double
IMP_OBJECTS(Image,Images);




//! Reads images from files (For compatibility with SPIDER format,
//! the images are read from floats)
/*!
  \param[in] names filenames of the images
  \param[in] rw  reader/writer to use
*/
IMPEM2DEXPORT Images read_images(const Strings &names,
                                 const ImageReaderWriter *rw);

//! Saves images to files (For compatibility with SPIDER format,
//! the images are written to floats)
/*!
  \param[in] images Images to save
  \param[in] names filenames of the images
  \param[in] rw  reader/writer to use
*/
IMPEM2DEXPORT void save_images(Images images, const Strings &names,
                               const ImageReaderWriter *rw);

//! Cross correlation between two images
/**
 * Get the cross correlation coefficient between 2 images
 * @param im1 First image
 * @param im2 Second image
 * @return The value of the coefficient. Ranges from 0 to 1
 */
IMPEM2DEXPORT double get_cross_correlation_coefficient(Image *im1,Image *im2);

/**
 * Compute the autocorrelation for an image
 * @param im1 The input image
 * @param im2 The output image
 */
inline void get_autocorrelation2d(Image *im1, Image *im2) {
  get_autocorrelation2d(im1->get_data(),im2->get_data());
}

/**
 * Compute the cross correlation matrix between two images
 * @param im1 First input image
 * @param im2 First second image
 * @param corr The output image of cross correlation values
 */
inline void get_correlation2d(Image *im1,Image *im2,Image *corr) {
  get_correlation2d(im1->get_data(),im2->get_data(),corr->get_data());
}

/**
 * Normalize an image subtracting the mean and dividing by the standard
 * deviation
 * @param im Image
 * @param force If true, the image is normalized even if the header says that
 * it is already normalized
 */
IMPEM2DEXPORT void do_normalize(Image *im,bool force=false);


/**
 * Gets an histogram of the values of the image
 * @param img The image
 * @param bins The number of bins to use to build the histogram
 * @return The histogram: number of points per bin
 */
inline Floats get_histogram(Image *img, int bins) {
  return get_histogram(img->get_data(),bins);
}


/**
 * Apply a variance filter to an image
 * @param input Input image
 * @param filtered The image containing the result
 * @param kernelsize The size of the kernel to use. The matrix used as kernel
 * will have the same number of rows and columns
 *
 */inline void apply_variance_filter(Image *input,
                           Image *filtered,int kernelsize) {
  apply_variance_filter(input->get_data(),filtered->get_data(),kernelsize);
}

/**
 * Apply the diffusion filter
 * @param input Input image
 * @param filtered The image containing the result
 * @param beta The beta parameter of the difussion filter
 * @param pixelsize The pixel size of the image
 * @param time_steps The number of time steps used for the diffusion
 */
inline void apply_diffusion_filter(Image *input,
                                   Image *filtered,
                                   double beta,
                                   double pixelsize,
                                   int time_steps) {
  apply_diffusion_filter(input->get_data(),
                         filtered->get_data(),
                         beta,
                         pixelsize,
                         time_steps);
}



inline void do_fill_holes(Image *input,Image *result,double n_stddevs) {
  do_fill_holes(input->get_data(),result->get_data(),n_stddevs);

}

inline void do_combined_fill_holes_and_threshold(Image *input,
                                                 Image *result,
                                                 double n_stddevs) {
  do_combined_fill_holes_and_threshold(input->get_data(),
                                       result->get_data(),
                                       n_stddevs);
}



inline void do_extend_borders(Image *im1,Image *im2,unsigned int pix) {
  do_extend_borders(im1->get_data(),im2->get_data(),pix);
}


inline void do_segmentation(Image *input,
                            Image *result,
                            const SegmentationParameters &params) {
  do_segmentation(input->get_data(),result->get_data(),params);
}

IMPEM2DEXPORT void do_remove_small_objects(Image *input,
                            double percentage,
                            int background=0,
                            int foreground=1);


/**
 * Subtract two images
 * @param first First image
 * @param second The second image is subtracted from the second
 * @param result Result image
 */
IMPEM2DEXPORT void do_subtract_images(Image *first,Image *second,
                                  Image *result);


IMPEM2DEXPORT void add_noise(Image *im1,double op1, double op2,
               const String &mode = "uniform", double df = 3);


/**
 * Resample an image to polar coordinates
 * @param im1 The input image
 * @param im2 The output image
 * @param polar_params The parameters used for the sampling
 */
IMPEM2DEXPORT void do_resample_polar(Image *im1,Image *im2,
                const PolarResamplingParameters &polar_params);

/** Crops an image.
 * @param[in] img Image to crop. It is modified in place
 * @param[in] center The pixel used as the center for cropping
 * @param[in] size The size of the new image
 */
IMPEM2DEXPORT void crop(Image *img, const IntPair &center, int size);



/**
 * Fills the values that are outside a circle centered at the center of the
 * image with the mean of the values inside the circle the matrix center
 * @param[in] img Image. It is modified in situ
 * @param[in] radius of the circle
*/
IMPEM2DEXPORT void apply_mean_outside_mask(Image *img, double radius);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_IMAGE_H */
