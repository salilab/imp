#ifndef _MAP3DHEADER_H
#define _MAP3DHEADER_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT

*/


class Map3DHeader {

public:
  Map3DHeader() {}
  Map3DHeader(const Map3DHeader & other_header) {
    for (int i=0;i<3;i++) {
      extent[i] = other_header.extent[i];
      orig[i] = other_header.orig[i]; 
      top[i]= other_header.top[i]; 
      grid[i]= other_header.grid[i]; 
      voxel_size[i]= other_header.voxel_size[i];
      cell_size[i]= other_header.cell_size[i]; 
      cell_angle[i]= other_header.cell_angle[i]; 
    }
  }

  int get_extent (int i) const { return extent[i];}
  int get_orig(int i) const { return orig[i];}
  int get_top(int i) const {return top[i];}
  int get_grid(int i) const {return grid[i];}
  float get_voxelsize(unsigned short int i) const {return voxel_size[i];}
  float get_cellsize(int i) const {return cell_size[i];}
  float get_cellangle(int i) const {return cell_angle[i];}


  void set_extent (int i,int data_) {extent[i] = data_;}
  void set_orig(int i,int data_) {orig[i]=data_;}
  void set_top(int i,int data_) {top[i]=data_;}
  void set_grid(int i,int data_) {grid[i]=data_;}
  void set_voxelsize( int i,float data_) {voxel_size[i]=data_;}
  void set_cellsize(int i,float data_) {cell_size[i]=data_;}
  void set_cellangle(int i,float data_) {cell_angle[i]=data_;}


private:




  int extent[3]; // the number of allocated voxels in each dimension extent = top-orig+1
  int orig[3]; // The first allocated voxel
  int top[3]; // The last allocated voxel
  int grid[3]; // The number of voxels in cellSize: spacing = cellSize/grid
  float voxel_size[3]; // the spacing of the grid
  float cell_size[3]; // the size of the unit cell 
  float cell_angle[3]; // The angle of the unit cell


};


#endif //_MAP3DHEADER_H
