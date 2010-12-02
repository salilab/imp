/**
 *  \file pca_features_extraction.h
 *  \brief pca features for an image
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PCA_FEATURES_EXTRACTION_H
#define IMPEM2D_PCA_FEATURES_EXTRACTION_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Matrix2D.h"


IMPEM2D_BEGIN_NAMESPACE

IMPEM2DEXPORT void center_coordinates(algebra::Vector2Ds &v,
                                    algebra::Matrix2D_d &m);

//! True if the pca features match within a given percentage
IMPEM2DEXPORT bool pca_features_match(algebra::Vector3D v1,
                                    algebra::Vector3D v2,
                                    double percentage);

//! PCA analysis for the points of a matrix
/*!
 \param[in] threshold all pixels with values in the matrix above the threshold
            are used.
 \return a vector with the eigenvalues of the covariance matrix
*/
IMPEM2DEXPORT algebra::Vector3D pca_features(const algebra::Matrix2D_d &m,
                            double treshold=0.0);

//! PCA analysis for each matrix of a set of images
/*!
 \param[in] threshold all pixels with values in the matrix above the threshold
            are used.
 \return a set of vectors with the eigenvalues for each image
*/
IMPEM2DEXPORT algebra::Vector3Ds pca_features(em::Images images,
                            double treshold=0.0);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PCA_FEATURES_EXTRACTION_H */
