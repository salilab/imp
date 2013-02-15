/**
 *  \file image_processing_helper.cpp
 *  \brief Helper functions for the image_processing main file
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */


#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/exception.h"
#include <IMP/log.h>
#include <vector>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE


cvPixels get_neighbors2d(const cvPixel &p,
                        const cv::Mat &m,
                        int mode,
                        int sign,
                        bool cycle) {
  cvPixels neighbors,final_neighbors;
  if(mode !=4 && mode !=8)
    IMP_THROW("Mode must be either 4 or 8",ValueException);
  if(sign !=1 && sign !=0 && sign!=-1)
    IMP_THROW("Sign must be -1,0, or 1",ValueException);

  if(mode == 4) {
    if(sign == 0) {
      neighbors.push_back(p+cvPixel(-1, 0));
      neighbors.push_back(p+cvPixel( 0, 1));
      neighbors.push_back(p+cvPixel( 1, 0));
      neighbors.push_back(p+cvPixel( 0,-1));
    } else if( sign == 1) {
      neighbors.push_back(p+cvPixel(0,1));
      neighbors.push_back(p+cvPixel(1,0));
    } else {
      neighbors.push_back(p+cvPixel(-1, 0));
      neighbors.push_back(p+cvPixel( 0,-1));
    }
  } else {
    if(sign == 0) {
      neighbors.push_back(p+cvPixel(-1, 0));
      neighbors.push_back(p+cvPixel(-1, 1));
      neighbors.push_back(p+cvPixel( 0, 1));
      neighbors.push_back(p+cvPixel( 1, 1));
      neighbors.push_back(p+cvPixel( 1, 0));
      neighbors.push_back(p+cvPixel( 1,-1));
      neighbors.push_back(p+cvPixel( 0,-1));
      neighbors.push_back(p+cvPixel(-1,-1));
    } else if( sign == 1) {
      neighbors.push_back(p+cvPixel(-1, 0));
      neighbors.push_back(p+cvPixel(-1, 1));
      neighbors.push_back(p+cvPixel( 0,-1));
      neighbors.push_back(p+cvPixel(-1,-1));
    } else {
      neighbors.push_back(p+cvPixel( 0, 1));
      neighbors.push_back(p+cvPixel( 1, 1));
      neighbors.push_back(p+cvPixel( 1, 0));
      neighbors.push_back(p+cvPixel( 1,-1));
    }
  }

  if(cycle) {
    // Cycle indexes out of the matrix
    for(unsigned int i=0;i<neighbors.size();++i) {
      cvPixel q=neighbors[i];
      if(q.x < 0)        q.x = m.rows-1;
      if(q.x >= m.rows)  q.x = 0;
      if(q.y < 0)        q.y = m.cols-1;
      if(q.y >= m.cols)  q.y = 0;
      final_neighbors.push_back(q);
    }
  } else {
    // Clean neighbors with indexes out of the matrix
    for(unsigned int i=0;i<neighbors.size();++i) {
      cvPixel q=neighbors[i];
      if( q.x>= 0 && q.x<m.rows && q.y >=0 && q.y<m.cols) {
        final_neighbors.push_back(q);
      }
    }
  }
  return final_neighbors;

}




/***************************

FUNCTIONS FOR THE LABELING ALGORITHM

***************************/


void set_final_labels(cvIntMat &mat_to_label,Ints &Labels) {
  for (int i=0;i<mat_to_label.rows;++i) {
    for (int j=0;j<mat_to_label.cols;++j) {
      cvPixel p(i,j);
      mat_to_label(p)=Labels[mat_to_label(p)];
    }
  }
}


int do_union(Ints &Labels,int i,int j) {
  int root = find_root(Labels,i);
  if(i != j) {
    int root_j = find_root(Labels,j);
    // Check which is the lowest root label
    if(root > root_j) {
      root = root_j;
    }
    // Set the lowest label
    set_root(Labels,j,root);
  }
  set_root(Labels,i,root);
  return root;
}


//! Decission tree copy function
void do_copy_tree(const cvPixel &p,
                  const cvPixel &a,
                  cvIntMat &mat_to_label) {

  mat_to_label(p) = mat_to_label(a);
}

//! Decission tree copy function
void do_copy_tree(const cvPixel &p,
                  const cvPixel &a,
                  const cvPixel &c,
                  cvIntMat &mat_to_label,
                  Ints &Labels) {
  mat_to_label(p) = do_union(Labels,mat_to_label(c),mat_to_label(a));
}


//! Sets a new label for a pixel
int get_new_label(const cvPixel &p,
              cvIntMat &mat_to_label,
              Ints &Labels,
              int label) {
  mat_to_label(p) = label;
  Labels.push_back(label);
  int next_label=label+1;
  return next_label;
}

int find_root(const Ints &Labels, int i) {
  int root = i;
  while(Labels[root] < root) {
    root = Labels[root];
  }
  return root;
}

void set_root(Ints &Labels,int i, int root) {
  while(Labels[i]<i) {
    int j = Labels[i];
    Labels[i] = root;
    i = j;
  }
  Labels[i] = root;
}

void do_flatten_labels(Ints &Labels) {
  int k=1;
  for (int i=1;i< int(Labels.size()) ;++i) {
    if(Labels[i]<i) {
      Labels[i]=Labels[Labels[i]];
    } else {
      Labels[i]=k;
      k++;
    }
  }
}



void do_binary_matrix_scan(const cvIntMat &m,
               cvIntMat &mat_to_label,
               Ints &Labels) {
// value for the the background and first label
  int background = 0;
  Labels.resize(1); Labels[0] = background;
  int label = 1;
  // scan
  for (int i=0;i<m.rows;++i) {
    for (int j=0;j<m.cols;++j) {
      cvPixel p(i,j);
      int val = m(p);
      if(val == background) {
        mat_to_label(p)=background;
      } else {
        cvPixels ns = get_neighbors2d(p,m,8,1,false);
        // fake neighbors to take into account exceptions
        cvPixels neighbors(4);
        cvPixel q(NO_VALUE,NO_VALUE);
        switch(ns.size()) {
          case 0:
            // Corner pixel, no raster neighbors and 4 NO_VALUE ones
            neighbors[0]=q;
            neighbors[1]=q;
            neighbors[2]=q;
            neighbors[3]=q;
          break;
          case 1:
            // One true neighbor, pixel in the first row (the neighbor is d)
            neighbors[0]=q;
            neighbors[1]=q;
            neighbors[2]=q;
            neighbors[3]=ns[0];
          break;
          case 2:
            // Two true neighbors, pixel in the first column
            // (neighbors are b and c)
            neighbors[0]=q;
            neighbors[1]=ns[0];
            neighbors[2]=ns[1];
            neighbors[3]=q;
          break;
          case 4:
            // Normal pixel, 4 neighbors
            neighbors[0]=ns[3];
            neighbors[1]=ns[0];
            neighbors[2]=ns[1];
            neighbors[3]=ns[2];
          break;
        }
       // Decission tree strategy
        label=get_label_using_decission_tree(p,
                             neighbors,
                             background,
                             m,
                             mat_to_label,
                             Labels,
                             label);
      }
    }
  }
}





int get_label_using_decission_tree(const cvPixel &p,
                    cvPixels &neighbors,
                    int background,
                    const cvIntMat &m,
                    cvIntMat &mat_to_label,
                    Ints &Labels,
                    int label) {
  // Set the values considering the null pixels as background
  cvPixel null_pixel(NO_VALUE,NO_VALUE);
  int neigh_vals[4];
  for (int i=0;i<4;++i) {
    if(! (neighbors[i] == null_pixel)) {
      neigh_vals[i] = m(neighbors[i]);
    } else {
      neigh_vals[i] = background;
    }
  }

  // Decission tree
  if(neigh_vals[1] == background) {
    if(neigh_vals[2] == background) {
      if(neigh_vals[0] == background) {
        if(neigh_vals[3] == background) {
          label=get_new_label(p,mat_to_label,Labels,label);
        } else {
          do_copy_tree(p,neighbors[3],mat_to_label);
        }
      } else {
        do_copy_tree(p,neighbors[0],mat_to_label);
      }
    } else {
      if(neigh_vals[0] == background) {
        if(neigh_vals[3] == background) {
          do_copy_tree(p,neighbors[2],mat_to_label);
        } else {
          do_copy_tree(p,neighbors[2],neighbors[3],mat_to_label,Labels);
        }
      } else {
        do_copy_tree(p,neighbors[2],neighbors[0],mat_to_label,Labels);
      }
    }
  } else {
    do_copy_tree(p,neighbors[1],mat_to_label);
  }
  return label;
}



algebra::Vector2D get_peak(cv::Mat &m,double *value) {
  // Find maximum value and location
  IMP_LOG_VERBOSE("starting peak seach on a matrix " << std::endl);

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
    algebra::Vector2D v = internal::get_weighted_centroid(region);
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
//      double value = m.at<double>(i-1,j-1);
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



IMPEM2D_END_INTERNAL_NAMESPACE
