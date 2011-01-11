/**
 *  \file labeling_algorithm.h
 *  \brief labeling images for EM
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/


#ifndef IMPEM2D_LABELING_ALGORITHM_H
#define IMPEM2D_LABELING_ALGORITHM_H

#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/Pixel.h"
#include "IMP/algebra/Matrix2D.h"
#include <vector>
#include <algorithm>

IMPEM2D_BEGIN_NAMESPACE


IMPEM2DEXPORT void assign_final_labels(
                  algebra::Matrix2D<int> &L,std::vector<int> &P);

//! union operation. Unites trees containing nodes i and j and returns the
//! the new root
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[in] j another node
  \param[out] root the new root
*/
IMPEM2DEXPORT int p_union(std::vector<int> &P, int i, int j);


//! Sets a new label for a pixel
inline int new_label(const Pixel &p,algebra::Matrix2D<int> &L,
                    std::vector<int> &P, int label) {
  L(p) = label;
  P.push_back(label);
  int next_label=label+1;
  return next_label;
}

//! Decission tree copy function
IMPEM2DEXPORT void d_copy(const Pixel &p, const Pixel &a,
            algebra::Matrix2D<int> &L);

//! Decission tree copy function
IMPEM2DEXPORT void d_copy(const Pixel &p, const Pixel &a,const Pixel &c,
            algebra::Matrix2D<int> &L,std::vector<int> &P);

//! find root operation
/*!
  \param[in] P vector of labels
  \param[in] i a node
  \param[out] root root of the tree for i
*/
inline int find_root(const std::vector<int> &P, int i) {
  int root = i;
  while(P[root] < root) {
    root = P[root];
  }
  return root;
}



//! Set root operation
/*!
  \param[in] P vector of labels
  \param[in] i node of a tree
  \param[out] root root node of the tree of i
*/
inline void set_root(std::vector<int> &P,int i, int root) {
  while(P[i]<i) {
    int j = P[i];
    P[i] = root;
    i = j;
  }
  P[i] = root;
}



//! flattens the vector with the union-find tree of labels and
//! relabels the components
inline void flattenL(std::vector<int> &P) {
  int k=1;
  for (int i=1;i< (int)P.size();++i) {
    if(P[i]<i) {
      P[i]=P[P[i]];
    } else {
      P[i]=k;
      k++;
    }
  }
}

//! Decission tree for neighbor exploration
template<typename T>
int decission_tree(const Pixel &p,
                    Pixels &neighbors,
                    int background,
                    const algebra::Matrix2D<T> &m,
                    algebra::Matrix2D<int> &L,
                    std::vector<int> &P,int label) {
  // Set the values considering the null pixels as background
  int neigh_vals[4];
  for (int i=0;i<4;++i) {
    if(!neighbors[i].get_is_null()) {
      neigh_vals[i] = algebra::get_rounded(m(neighbors[i]));
    } else {
      neigh_vals[i] = background;
    }
  }
  // Decission tree
  if(neigh_vals[1] == background) {
    if(neigh_vals[2] == background) {
      if(neigh_vals[0] == background) {
        if(neigh_vals[3] == background) {
          label=new_label(p,L,P,label);
        } else {
          d_copy(p,neighbors[3],L);
        }
      } else {
        d_copy(p,neighbors[0],L);
      }
    } else {
      if(neigh_vals[0] == background) {
        if(neigh_vals[3] == background) {
          d_copy(p,neighbors[2],L);
        } else {
          d_copy(p,neighbors[2],neighbors[3],L,P);
        }
      } else {
        d_copy(p,neighbors[2],neighbors[0],L,P);
      }
    }
  } else {
    d_copy(p,neighbors[1],L);
  }
  return label;
}



//! Scanning function
/*!
  \param[in] m binary matrix to scan
  \param[out] L matrix with the labels
*/
template<typename T>
void scan_SCT1(const algebra::Matrix2D<T> &m,algebra::Matrix2D<int> &L,
              std::vector<int> &P) {
  int init_y = m.get_start(0); int end_y = m.get_finish(0);
  int init_x = m.get_start(1); int end_x = m.get_finish(1);
// value for the the background and first label
  int background = 0;
  P.resize(1); P[0] = background;
  int label = 1;
  // scan
  for (int i=init_y;i<=end_y;++i) {
    for (int j=init_x;j<=end_x;++j) {
      Pixel p(i,j);
      int val = algebra::get_rounded(m(p));
      if(val == background) {
        // Background
        L(p)=background;
      } else {
        // true neighbors
        Pixels ns = get_neighbors2d(p,m,8,1,false);
        // fake neighbors to take into account exceptions
        Pixels neighbors(4);
        Pixel q;q.set_null(true);
        switch(ns.size()) {
          case 0:
            // Corner pixel, no raster neighbors and 4 fake ones
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
            // Normal pixel, 4 raster neighbors
            neighbors[0]=ns[3];
            neighbors[1]=ns[0];
            neighbors[2]=ns[1];
            neighbors[3]=ns[2];
          break;
        }
       // Decission tree strategy
        label=decission_tree(p,neighbors,background,m,L,P,label);
      }
    }
  }
}



//! Labeling function for a Matrix2D
/*!
  \param[in] m binary matrix to scan. The matrix needs to contain zeros and
             ones but they can be stored as doubles, floats or ints
  \param[out] result matrix it is returned as a matrix of ints
  \param[out] labels The number of labels in the image
*/
template<typename T>
int labeling(const algebra::Matrix2D<T> &m,algebra::Matrix2D<int> &result) {
  result.reshape(m);
  std::vector<int> union_find_tree;
  scan_SCT1(m,result,union_find_tree);
//  std::cout << "result: " << std::endl << result << std::endl;
  flattenL(union_find_tree);
  assign_final_labels(result,union_find_tree);
  int labels = *max_element(union_find_tree.begin(), union_find_tree.end());
  return labels;
}

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_LABELING_ALGORITHM_H */
