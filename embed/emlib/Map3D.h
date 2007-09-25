#ifndef _MAP3D_H
#define _MAP3D_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT
  responsibilities: holds and provides access to a 3D grid

*/

#include "Vector3.h"
#include "VoxelInd.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
using std::istream;
using std::ios;

using namespace std;
#define NR_END 1

template<class dataItemT> class Map3D;

template <class dataItemT>
class Map3D {
public:

  // constructors

  Map3D() {}
  Map3D(int * _extent,int * _orig, int *_spacing);
  Map3D(const Map3D &copy_map); // copy constuctor
  ~Map3D();


  // handle the grid

  int AllocateData(int *_extent);    
  int Release(); // free the allocated grid

  // inspections

  // given a location - check if it is part of the grid
  bool IsValid(const Vector3 &loc) const {return IsValid(getVoxel(loc,2));}
  bool IsValid(const VoxelInd &v) const;
  const dataItemT  &getData(int i, int j,int k) const;
  const dataItemT  &getData(const VoxelInd &v) const {return getData(v[0],v[1],v[2]);}
  const dataItemT  &getData(const Vector3 &loc) const {return getData(getVoxel(loc));}
  void setData(int i,int j,int k, dataItemT);
  void setData(const VoxelInd &v, dataItemT d) {
    setData(v[0],v[1],v[2],d);
  }
  void setData(const Vector3 &loc, dataItemT d) {
    setData(getVoxel(loc),d);
  }
 
  Vector3 getLocation(const VoxelInd &v) const;
  unsigned long getIndex(const VoxelInd &v) const;
  VoxelInd getVoxel(const Vector3 &loc) const ;
  VoxelInd getVoxel(const Vector3 &loc,int dummy) const ; // this means that you should not exit if out of range

  int DuplicateMapWithoutDensities(const Map3D &templateMap);

  void PrintXplorFormat(ostream& s) const;
  const dataItemT  & getPrintValue(int i, int j, int k) const { return data[i][j][k]; }
  // here we should implement an iterator the return type is voxelInd

  // header inspection
  float getSpacing(unsigned short int i) const;
  int getExtent (int i) const;
  int getOrig(int i) const;
  VoxelInd getOrigVoxel() const { return VoxelInd(orig[0],orig[1],orig[2]);}
  int getTop(int i) const;
  int getGrid(int i) const;
  float getCellSize(int i) const;
  float getCellAngle(int i) const;
  const Vector3 & getTranslateGrid() const { return translateGrid;}
  int getIndOrder() const {return indOrder;}


  int setTranslateGrid(const Vector3 _translateGrid)  {translateGrid = _translateGrid;}

  class const_iterator;
  const_iterator begin() const {
    const_iterator it(VoxelInd(0,0,0),this);
    return it;
  }
  const_iterator end() const //points to one past location in the grid
  {
    int temp = extent[0];
    const_iterator it( VoxelInd(temp,0,0),this);
    return it;
  }

  // The iterator class itself
  // Note: The size of the array must not be changed while an
  //       iterator is in use.

  class const_iterator
  {
  public:
    const_iterator(const VoxelInd &_curP, const Map3D *_data):curP(_curP),data(_data){}
    const_iterator():curP(){}
    
    const_iterator(const const_iterator &it)
      :curP(it.curP),data(it.data) {}

    bool operator==(const const_iterator &_it) const
    { return curP == _it.curP; }
    
    bool operator!=(const const_iterator &_it) const
    { return !(curP == _it.curP); }
    
    const dataItemT& operator*() const { return data->getData(curP); };
    
    const_iterator operator-(const const_iterator &it) {
      return const_iterator(curP-it.curP,data);
    }

    const_iterator operator+(const const_iterator &it) {
      return const_iterator(curP+it.curP,data);
    }
    const_iterator operator++() {
      if (curP[0] != data->getExtent(0)){
	curP.update(2,curP[2]+1);
	if (curP[2] == data->getExtent(2)){
	  curP.update(2,0);
	  curP.update(1,curP[1]+1);
	  if (curP[1] == data->getExtent(1)){
	    curP.update(1,0);
	    curP.update(0,curP[0]+1);
	  }
	}
      }
      return *this;
    }
    
    //----
    // update the pointer but return the former pointer.
    const_iterator operator++(int dummy) 
    {
      const_iterator formerP(*this);
      operator++();
      return formerP;
    }
    const VoxelInd & getPosition() const {return curP;}
  private:
    VoxelInd curP;
    const Map3D *data;
  }; // end of class const_iterator



  const dataItemT& operator[](const const_iterator &it) const{
    return getData(it.getPosition());
  }
   dataItemT operator[](const VoxelInd &v_i) const{
    return getData(v_i);
  }
  const Vector3& operator[](unsigned long index) const{
    ldiv_t dimX=ldiv(index, extent[2]*extent[1]);
    
    unsigned long i = dimX.quot;

    ldiv_t dimY=ldiv(dimX.rem, extent[2]);

    unsigned long j = dimY.quot;
    unsigned long k = dimY.rem;


    return getLocation(VoxelInd(i,j,k));
  }

protected:
  void free_3d_array_range(dataItemT ***t,int nr1,int nrh,int nc1,int nch,int ndl,int ndh);
  dataItemT *** allocate_3d_array_range(int nr1,int nrh,int nc1,int nch,int ndl,int ndh);


  // header data
  int extent[3]; // the number of allocated voxels in each dimension extent = top-orig+1
  int orig[3]; // The first allocated voxel
  int top[3]; // The last allocated voxel
  int grid[3]; // The number of voxels in cellSize: spacing = cellSize/grid
  float spacing[3]; // the spacing of the grid
  float cellSize[3]; // the size of the unit cell 
  float cellAngle[3]; // The angle of the unit cell

  // orientation data
  Vector3 translateGrid; // the left-buttom voxel of the grid is assumed to be located at (0,0,0). The exact location of the grid is found in the XPLOR file. We insert this data to the translateGrid vector
  int indOrder; // the reading order of the xplor file

  // data
  dataItemT ***data;
private:
  bool allocated;
};






#define INCLUDE_SHEILD
#include "Map3D.cc"
#undef INCLUDE_SHEILD


#endif //_MAP3D_H
