#include "Map3D.h"

#ifdef INCLUDE_SHEILD

template <class dataItemT>
Map3D<dataItemT>::Map3D(int * _extent,int * _orig, int *_spacing){
  AllocateData(_extent);
  for(int i=0;i<3;i++) {
    orig[i]=_orig[i]; spacing[i] = _spacing[i];
    }
}
template <class dataItemT>
Map3D<dataItemT>::Map3D(const Map3D &copy_map) {
  for(int i=0;i<3;i++) {
    extent[i] = copy_map.extent[i];
    spacing[i] = copy_map.spacing[i];
    orig[i] = copy_map.orig[i];
  }
  AllocateData(extent);

  // TODO - add the actual content of the map ( or just point to the same pointer?)
}

template <class dataItemT>
int Map3D<dataItemT>::Release() {

  free_3d_array_range(data,0,extent[0]-1,0,extent[1]-1,0, extent[2]-1);
  return 0;
}


template <class dataItemT>
Map3D<dataItemT>::~Map3D() {
  Release();
}

template <class dataItemT>
bool Map3D<dataItemT>::IsValid(const VoxelInd &v) const{
  for(int i=0;i<3;i++) {
    if (v[i]<0) return false;
    if (v[i]>=extent[i]) return false;
  }
  return true;
}



template <class dataItemT>
const dataItemT & Map3D<dataItemT>::getData (int i, int j,int k) const {

  if (!IsValid(VoxelInd(i,j,k))) {
    cout << " Map3D::getData is out of range: " << i << "  " << j << "  " << k << " and the extent is " << extent[0] << "  " << extent[1] << "  " << extent[2] << endl;
    exit(-1);
  }

  return data[i][j][k];
}

template <class dataItemT>
void Map3D<dataItemT>::setData(int i,int j,int k, const dataItemT d) {
 if ((i<0) || (i>extent[0]-1) || (j<0) || (j>extent[1]-1) || (k<0) || (k>extent[2]-1)) {
    cout << " Map3D::getData is out of range: " << i << "  " << j << "  " << k << " and the extent is " << extent[0] << "  " << extent[1] << "  " << extent[2] << endl;
    exit(-1);
  }
 data[i][j][k] = d;
}


template <class dataItemT>
int Map3D<dataItemT>::AllocateData(int *_extent){
  for(int i=0;i<3;i++)
    extent[i]=_extent[i];

  data = allocate_3d_array_range(0,extent[0]-1,0,extent[1]-1,0,extent[2]-1);
  return 0;
}

//  allocate a float 3d array with rage[nrl..nrh][ncl..mch][ndl..ndh]
template <class dataItemT>
dataItemT *** Map3D<dataItemT>::allocate_3d_array_range(int nrl,int nrh,int ncl,int nch,int ndl,int ndh)
{
  int i,j;
  int nrow=nrh-nrl+1;
  int ncol=nch-ncl+1;
  int ndep=ndh-ndl+1;

  dataItemT ***t;
  //allocate pointers to pointers to rows
  t = (dataItemT ***)malloc((nrow+NR_END)*sizeof(dataItemT **));
  if (!t) 
    {cout <<" allocation failure 1 in allocate_3d_array_range " << (nrow+NR_END)*sizeof(dataItemT **) << endl;
 cout << "problem with: " << nrow << "  " << NR_END << endl;
 exit(0);
}
  t+=NR_END;
  t-=nrl;

  //allocate pointers to rows & set pointers to them
  t[nrl]=(dataItemT**)malloc((nrow*ncol+NR_END)*sizeof( dataItemT*));
  if (!t[nrl]) cout<< " allocation failure 2 in allocate_3d_array_range " << endl;
   t[nrl] +=NR_END;
   t[nrl] -= ncl;

   //allocate rows & set pointers to them
    t[nrl][ncl]=(dataItemT*)malloc((nrow*ncol*ndep+NR_END)*sizeof(dataItemT));
    if (!t[nrl][ncl]) cout <<" allocation failure 3 in allocate_3d_array_range " << endl;
   t[nrl][ncl] +=NR_END;
   t[nrl][ncl] -= ndl;

   for(j=ncl+1;j<=nch;j++)
     t[nrl][j]=t[nrl][j-1]+ndep;
   
   for(i=nrl+1;i<=nrh;i++)
     {
       t[i]=t[i-1]+ncol;
       t[i][ncl]=t[i-1][ncl]+ncol*ndep;
       for(j=ncl+1;j<=nch;j++) t[i][j]=t[i][j-1]+ndep;
     }
   allocated = true;
   //return pointer to array of pointers to rows
   return t;

}


// free a float 3d array with rage[nrl..nrh][ncl..mch][ndl..ndh] allocated by allocate_3d_array_range 
template <class dataItemT>
void Map3D<dataItemT>::free_3d_array_range(dataItemT ***t,int nrl,int nrh,int ncl,int nch,int ndl,int ndh){
  if (allocated){
  free((dataItemT*) (t[nrl][ncl]+ndl-NR_END));
  free((dataItemT*) (t[nrl]+ncl-NR_END));
  free((dataItemT*) (t+nrl-NR_END));
  allocated = false;
  }
}


template <class dataItemT>
int Map3D<dataItemT>::getExtent(int i) const{
  if ((i<0) || (i>2)) {
    cout << " Map3D::getExtent access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return extent[i];
}


template <class dataItemT>
int Map3D<dataItemT>::getOrig(int i) const{
 if ((i<0) || (i>2)) {
    cout << " Map3D::getOrig access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return orig[i];
}

template <class dataItemT>
int Map3D<dataItemT>::getTop(int i) const{
 if ((i<0) || (i>2)) {
    cout << " Map3D::getTop access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return top[i];
}


template <class dataItemT>
int Map3D<dataItemT>::getGrid(int i) const{
 if ((i<0) || (i>2)) {
    cout << " Map3D::getGrid access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return grid[i];
}


template <class dataItemT>
float Map3D<dataItemT>::getCellSize(int i) const{
 if ((i<0) || (i>2)) {
    cout << " Map3D::getCellSize access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return cellSize[i];
}


template <class dataItemT>
float Map3D<dataItemT>::getCellAngle(int i) const{
 if ((i<0) || (i>2)) {
    cout << " Map3D::getCellAngle access vailation index requested is " << i << endl; 
    exit(-1); 
  } 
  return cellAngle[i];
}

template <class dataItemT>
void Map3D<dataItemT>::PrintXplorFormat(ostream& s) const{
  s <<endl << "       2"<<endl << "REMARKS file name = ??? " << endl << "REMARKS Date ...... created by em lib " << endl;
  s.setf(ios::right, ios::adjustfield);
  s.width(8);
  s << setw(8)<<grid[0]<<
    setw(8)<<floor(translateGrid[0]/spacing[0])+1<<
    setw(8)<<floor(translateGrid[0]/spacing[0])+1+extent[0]-1<<
    setw(8)<<grid[1]<<
    setw(8)<<floor(translateGrid[1]/spacing[1])+1<<
    setw(8)<<floor(translateGrid[1]/spacing[1])+1 + extent[1]-1<<
    setw(8)<<grid[2]<<
    setw(8)<<floor(translateGrid[2]/spacing[2])+1<<
    setw(8)<<floor(translateGrid[2]/spacing[2])+1+extent[2]-1<<endl;
    
  s<< scientific << setprecision(5)<<setw(12)<<cellSize[0];
  s<< scientific << setprecision(5)<<setw(12)<<cellSize[1];
  s<< scientific << setprecision(5)<<setw(12)<<cellSize[2];
  s<< scientific << setprecision(5)<<setw(12)<<cellAngle[0];
  s<< scientific << setprecision(5)<<setw(12)<<cellAngle[1];
  s<< scientific << setprecision(5)<<setw(12)<<cellAngle[2]<<endl;
  s << "ZYX" << endl; // X is the slowest
  int counter = 0;
  cout << " in print2file " << endl;
  for(int k=0;k<extent[2];k++)
    { 
      if (counter != 0){
	  s << endl;
	  counter=0;
	}
        s<<setw(8)<<k<<endl;
	for(int j=0;j<extent[1];j++) {
	  for(int i=0;i<extent[0];i++) {
	    //	    s<< scientific << setprecision(5)<<setw(12)<<data[i][j][k];
	    s<< scientific << setprecision(5)<<setw(12)<<getPrintValue(i,j,k);
	    counter++;
	    if (counter == 6) {
	      counter = 0;
	      s << endl;
	    }
	  }
	}
    }
  s<<endl << "  -9999" << endl;
}

template <class dataItemT>
float Map3D<dataItemT>::getSpacing(unsigned short int i) const{
    if ( i>2) {
      cout << " Map3d::getSpacing index out of range i = " << i << endl;
      exit(-1);
    }
    return spacing[i];
  }


template <class dataItemT>
Vector3 Map3D<dataItemT>::getLocation(const VoxelInd &v) const{
  return Vector3(translateGrid[0]+spacing[0]*(v[0]+0.5),
		 translateGrid[1]+spacing[1]*(v[1]+0.5),
		 translateGrid[2]+spacing[2]*(v[2]+0.5));

}
template <class dataItemT>
VoxelInd Map3D<dataItemT>::getVoxel(const Vector3 &loc,int dummy) const {
  int v_x,v_y,v_z;
   v_x = (int)round((1.0*loc[0] - translateGrid[0])/spacing[0]-0.5);
   v_y = (int)round((1.0*loc[1] - translateGrid[1])/spacing[1]-0.5);
   v_z = (int)round((1.0*loc[2] - translateGrid[2])/spacing[2]-0.5);
//  v_x = (int)floor((1.0*loc[0] - translateGrid[0])/spacing[0]);
  // v_y = (int)floor((1.0*loc[1] - translateGrid[1])/spacing[1]);
  //v_z = (int)floor((1.0*loc[2] - translateGrid[2])/spacing[2]);

  return VoxelInd(v_x,v_y,v_z);
}
// loc is in absolute coordinates
template <class dataItemT>
VoxelInd Map3D<dataItemT>::getVoxel(const Vector3 &loc) const {

  VoxelInd voxel = getVoxel(loc,4);
  int v_x = voxel[0];
  int v_y = voxel[1];
  int v_z = voxel[2];

  if(v_x >= extent[0] || v_x <0 ||
     v_y >= extent[1] || v_y <0 ||
     v_z >= extent[2] || v_z <0) {
    cout << "Map3D::getVoxel the location in out of range " << loc << " the translate grid is " << translateGrid << endl << v_x << "  " << v_y << "   "<< v_z << endl;
    exit(-1);
  }
  return voxel;
}


template <class dataItemT>
unsigned long Map3D<dataItemT>::getIndex(const VoxelInd &v) const{
// this is raw order 
  return  (unsigned long)v[0]*(unsigned long)extent[2]*(unsigned long)extent[1] + (unsigned long)v[1] * (unsigned long)extent[2] + (unsigned long)v[2]; }



template <class dataItemT>
int Map3D<dataItemT>::DuplicateMapWithoutDensities(const Map3D &templateMap) {
  for(int i=0;i<3;i++) {
    extent[i]=templateMap.extent[i];
    orig[i] = templateMap.orig[i]; 
    top[i]= templateMap.top[i]; 
    grid[i]= templateMap.grid[i];
    spacing[i]=templateMap.spacing[i];
    cellSize[i]=templateMap.cellSize[i];
    cellAngle[i]=templateMap.cellAngle[i];
  }
  
  translateGrid=templateMap.translateGrid;
  indOrder = templateMap.indOrder;
  AllocateData(extent);
  return 0;
}



#endif
