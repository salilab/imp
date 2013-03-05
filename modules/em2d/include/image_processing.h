/**
 *  \file image_processing.h
 *  \brief Image processing functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_IMAGE_PROCESSING_H
#define IMPEM2D_IMAGE_PROCESSING_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/base_types.h"
#include <algorithm>
#include <cmath>


IMPEM2D_BEGIN_NAMESPACE


//! Class to provide all the parameters to the segmentation function
class IMPEM2DEXPORT SegmentationParameters {
public:
  double image_pixel_size;
  double diffusion_beta;
  double diffusion_timesteps;
  double fill_holes_stddevs;
  cv::Mat opening_kernel;
  double remove_sizing_percentage;
  int binary_background;
  int binary_foreground;
  double threshold; // values of the image below the threshold will be set to 0

  SegmentationParameters(): image_pixel_size(1),
                            diffusion_beta(45),
                            diffusion_timesteps(200),
                            fill_holes_stddevs(1.0),
                            remove_sizing_percentage (0.1),
                            binary_background(0),
                            binary_foreground(1),
                            threshold(0.0) {
    opening_kernel = cv::Mat::ones(3,3,CV_64FC1);
  }

  SegmentationParameters(double apix,
                         double diff_beta,
                         unsigned int diff_timesteps,
                         double fh_stddevs,
                         const cv::Mat &kr,
                         int background,
                         int foreground):
                               image_pixel_size(apix),
                               diffusion_beta(diff_beta),
                               diffusion_timesteps(diff_timesteps),
                               fill_holes_stddevs(fh_stddevs),
                               opening_kernel(kr),
                               binary_background(background),
                               binary_foreground(foreground) {};

  void show(std::ostream &out = std::cout) const {
    out << "Diffusion parameters: " << std::endl;
    out << "image_pixel_size " << image_pixel_size << std::endl;
    out << "diffusion_beta " << diffusion_beta << std::endl;
    out << "diffusion_timesteps " << diffusion_timesteps << std::endl;
    out << "fill_holes_stddevs " << fill_holes_stddevs << std::endl;
    out << "Opening kernel " << opening_kernel.rows
        << "x" << opening_kernel.cols << std::endl;
    out << "binary_background " << binary_background << std::endl;
    out << "binary_foreground " << binary_foreground << std::endl;
  }
};
IMP_VALUES(SegmentationParameters,SegmentationParametersList);


/*! Information for the match of a template in one image. The pair is the
    pixel of the image where the match is found.
*/
class IMPEM2DEXPORT MatchTemplateResult {
public:
  IntPair pair;
  double cross_correlation;
  MatchTemplateResult(IntPair p,double cc): pair(p), cross_correlation(cc) {};

  void show(std::ostream &out = std::cout) const {
    out << "MatchTemplateResult: Pair (" << pair.first << "," << pair.second
        << ") ccc = " << cross_correlation << std::endl;
  }

};
IMP_VALUES(MatchTemplateResult, MatchTemplateResults);


//! Applies a binary mask to an image.
/*!
  \param[in] m Input matrix
  \param[in] result matrix with the result
  \param[in] mask If the mask is 0, the result has the value of val. Otherwise
             is the value of m.
  \param[in] value value to apply when the mask is 0
*/
IMPEM2DEXPORT void apply_mask(const cv::Mat &m,
                              cv::Mat &result,
                              const cvIntMat &mask,
                              double val);

//! Applies a circular to a matrix. The center of the mask is the center of the
//! matrx.
/*!
  \param[in] Radius of the mask
*/
IMPEM2DEXPORT void apply_circular_mask(const cv::Mat &mat,
                                       cv::Mat &result,
                                       int radius,
                                       double value=0.0);



/*! Mean of an matrix inside a mask
  \param[in] mat The matrix
  \param[in] mask If a pixel is not zero, is considered for computing the mean
  \returns The mean
*/
IMPEM2DEXPORT double get_mean(const cv::Mat &mat, const cvIntMat &mask);


/*! Creates a circular matrix that is a mask
  \param[in] rows
  \param[in] cols
  \param[in] radius The radius of the mask. The center of the mask is the
             center of the matrix
  \returns A binary matrix (0/1) with value 1 inside the radius
*/
IMPEM2DEXPORT cvIntMat create_circular_mask(int rows, int cols, int radius);


//! Removes small objects from a matrix of integers.
/*!
  \param[in] m the matrix
  \param[in] percentage The percentage respect to the largest object that
             other objects have to be in order to survive the removal.
  \param[in] background value for the background after removed
  \param[in] background value for the foreground after removed
*/
IMPEM2DEXPORT void do_remove_small_objects(cvIntMat &m,
                          double percentage,
                          int background=0,
                          int foreground=1);


 IMPEM2DEXPORT void do_histogram_stretching(cv::Mat &m,
                                           int boxes,
                                           int offset);

//! (U. Adiga, 2005)
/*!
  \param[in] m binary matrix to dilate and shrink
  \param[in] greyscale greyscale matrix that controls the shrinking
  \param[in] kernel dilation kernel
  \note Only tested with binary matrices m with background =0 and foreground = 1
*/
 void IMPEM2DEXPORT do_dilate_and_shrink_warp(cv::Mat &m,
                                const cv::Mat &greyscale,
                                cv::Mat &kernel);



//! morphologic grayscale reconstruction (L Vincent, 1993)
/*!
  \param[in] mask image to reconstruct
  \param[out] marker this image contains the initial marker points and will
              contain the final result
  \param[in] neighbors_mode number of neighbors for a pixel to consider when
             doing the morphologic reconstruction (values: 4, 8).
*/
void IMPEM2DEXPORT do_morphologic_reconstruction(const cv::Mat &mask,
                                      cv::Mat &marker,
                                      int neighbors_mode=4);



//! Labeling function for a matrix
/*!
  \param[in] m binary matrix to scan. The matrix needs to contain zeros and
             ones but they can be stored as doubles, floats or ints
  \param[out] result matrix it is returned as a matrix of ints
  \param[out] labels The number of labels in the image
*/
IMPEM2DEXPORT int do_labeling(const cvIntMat &m,
                cvIntMat &mat_to_label);


//! Segmentation of images
/*!
  \param[in] m The EM image to segment
  \param[in] result The segmented image, with the shape of the molecule
*/
IMPEM2DEXPORT void do_segmentation(const cv::Mat &m,
                                   cv::Mat &result,
                                   const SegmentationParameters &params);

//! Smoothing filter by application of the reaction-diffusion
//! equation of Beltrami flow. Adiga, JSB, 2005
/*!
  \param [in] beta contribution of diffusion versus edge enhancement.
              0 - pure reaction, 90 - pure diffusion
  \note The function only works for matrices containing doubles
*/
IMPEM2DEXPORT void apply_diffusion_filter(const cv::Mat &m,
                           cv::Mat &result,
                           double beta,
                           double pixelsize,
                           unsigned int time_steps);

//! Partial derivative with respect to time for an image filtered with
//! difusion-reaction
/*!
  \param[in] m input matrix
  \param[in] der output derivative matrix
  \param[in] dx - step for x
  \param[in] dy - step for y
  \param[in] ang - parameter for weight diffusion and edge detection (90-0)
*/
IMPEM2DEXPORT void get_diffusion_filtering_partial_derivative(const cv::Mat &m,
                                                cv::Mat &der,
                                               double dx,
                                               double dy,
                                               double ang);


//! Fills the holes in the matrix m of height h
/*!
  \param[in] m the input matrix
  \param[in] result the result matrix
  \param[in] h the height
  \note The function does not work in-place
*/
IMPEM2DEXPORT void do_fill_holes(const cv::Mat &m,
                                 cv::Mat &result,
                                 double h);


//! Gets the domes of m with height h
/*!
  \param[in] m the input matrix
  \param[in] result the result matrix
  \param[in] h the height
  \note The function does not work in-place
*/
IMPEM2DEXPORT void get_domes(cv::Mat &m,cv::Mat &result,double h) ;


//! Combines the fill holes and tresholding operations together with normalize
/*!
  \param[in] m Input matrix
  \param[out] result the result matrix
  \param[in] n_stddevs Number of standard deviations used to compute the holes
  \param[in] threshold Removes values below the threshold value
  \note The function does normalize -> fill_holes -> normalize -> threshold ->
    normalize
*/
IMPEM2DEXPORT void do_combined_fill_holes_and_threshold(cv::Mat &m,
                                     cv::Mat &result,
                                     double n_stddevs,
                                     double threshold=0.0);



//! Computes the histogram of a matrix.
/*!
  \param[in] m Matrix with the data
  \param[in] bins Number of bins to use in the histogram
  \param[out] vector with the values for each bin
*/
IMPEM2DEXPORT Floats get_histogram(const cv::Mat &m, int bins);


//! Variance filter for an image. Computes the variance for each pixel using
//! the surrounding ones.
 /*!
  \param[in] input image with the data
  \param[out] filtered matrix result of the filtering with the variances
  \param[in] kernelsize The variance is computed using kernelsize x kernelsize
              pixels around each one. Kernelsize can only be odd.
*/
IMPEM2DEXPORT void apply_variance_filter(const cv::Mat &input,
                                   cv::Mat &filtered,int kernelsize);


//!Add noise to the values of a matrix.
/*!
    Supported distributions:
   - uniform distribution, giving the range (lower, upper). DEFAULT
   - gaussian distribution, giving the mean and the standard deviation
   \code
   add_noise(v1,0, 1);
   // uniform distribution between 0 and 1

   v1.add_noise(0, 1, "uniform");
   // the same

   v1.add_noise(0, 1, "gaussian");
  // gaussian distribution with 0 mean and stddev=1

   \endcode
*/
IMPEM2DEXPORT void add_noise(cv::Mat &v, double op1, double op2,
               const String &mode = "uniform", double df = 3);


//! Resamples a matrix to polar coordinates.
/*!
  \param[in] m matrix to resample
  \param[out] result matrix to contain the resampling
  \param[in] polar params Parameters used for the resampling. Extremely useful
            for speeding up the procedure if they are given with the
            transformation maps, that can be built in the
            PolarResamplingParameters class
*/
IMPEM2DEXPORT void do_resample_polar(const cv::Mat &input,
                                  cv::Mat &resampled,
                               const PolarResamplingParameters &polar_params);

//! Normalize a openCV matrix to mean 0 and stddev 1. It is done in place
IMPEM2DEXPORT void do_normalize(cv::Mat &m);


IMPEM2DEXPORT void my_meanStdDev(const cv::Mat &m,
                                       cv::Scalar &mean,
                                       cv::Scalar &stddev);


//! Applies a transformation to a matrix. First rotates the matrix using the
//! matrix center as the origin of the rotation, and then applies
//! the translation
IMPEM2DEXPORT void get_transformed(const cv::Mat &input,
                                   cv::Mat &transformed,
                                   const algebra::Transformation2D &T);



/*! Extends the bordes of an image
 \param[in] orig The image to extend
 \param[in] dst The image destination
 \param[in] pix number of pixels to extend the borders
*/
IMPEM2DEXPORT void do_extend_borders(cv::Mat &orig,
                                  cv::Mat &dst,
                                  unsigned int pix);


/*! Applys a threshold to an image
  \param[in] threshold. all pixels below this value are set to zero
*/
IMPEM2DEXPORT void apply_threshold(cv::Mat &m,
                                   cv::Mat &result,
                                   double threshold=0.0);

/*! morphologic enhancement of the contrast
  This function detects areas in the images and enhances the contrast between
  them
  \param[in] m Imput matrix
  \param[out] result
  \param[in] kernel morphologic kernel to use
  \param[in] iterations Higher number, more contrast
*/
IMPEM2DEXPORT void do_morphologic_contrast_enhancement(const cv::Mat &m,
                                                      cv::Mat &result,
                                                      const cv::Mat &kernel,
                                                      unsigned int  iterations);
/*! Morphologic gradient: dilation-erosion
  \param[in] m Input matrix
  \param[out] result
  \param[in] kernel morphologic kernel
*/
IMPEM2DEXPORT void get_morphologic_gradient(const cv::Mat &m, cv::Mat &result,
                                        const cv::Mat &kernel);

/*! Get the percentage of overlap between two matrices.
  Two images are overlapping in a pixel if both have values > 0.
  The overlap is (pixels overlapping) / (pixels > 0 in m2)

  \param[in] m1 First matrix
  \param[in] m2 Matrix used to check the overlap. This matrix can be of the
            same size of m1 or smaller.
  \param[in] center Indicates the position of m1 where to put
             the center m2. E.g., if center is (32,16) the center
             of m2 will be in the pixel (32,16) of m1.
*/
IMPEM2DEXPORT double get_overlap_percentage( cv::Mat &m1,
                                             cv::Mat &m2,
                                             const IntPair &center);


/*! Gets the n first matches between an image and a template
  \param[in] m Matrix
  \param[in] templ Matrix with a template to be found in m
  \param[in] n Number of positions to recover
*/
IMPEM2DEXPORT MatchTemplateResults get_best_template_matches(const cv::Mat &m,
                                        const cv::Mat &templ,
                                        unsigned int  n);

/*! Crop an image
  \param[in] m Matrix to crop
  \param[in] center The pixel used as the center for cropping
  \param[in] size The size of the new image
  \return A matrix with the cropped region
*/
IMPEM2DEXPORT cv::Mat crop(const cv::Mat &m,
                           const IntPair &center,
                           int size);



IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_IMAGE_PROCESSING_H */
