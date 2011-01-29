/**
 *  \file image_processing_helper.cpp
 *  \brief Helper functions for the image_processing main file
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */


#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/exception.h"
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
      if( q.x>= 0 && q.x<m.rows && q.y >=0 && q.y<=m.cols) {
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


IMPEM2D_END_INTERNAL_NAMESPACE
