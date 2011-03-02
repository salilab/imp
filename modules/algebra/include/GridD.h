/**
 *  \file Grid3D.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_D_H
#define IMPALGEBRA_GRID_D_H

#include "algebra_config.h"

#include <IMP/base_types.h>
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "internal/grid_3d.h"
#include <boost/iterator/transform_iterator.hpp>
#include <IMP/internal/map.h>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

namespace grids {

  //! An index in an infinite grid on space
  /* The index entries can be positive or negative and do not need to correspond
     directly to cells in the grid. They get mapped on to actual grid cells
     by various functions.
     \see Grid3D
  */
  template < int D>
  class ExtendedGridIndexD {
    internal::VectorData<int, D, true> data_;
    int compare(const ExtendedGridIndexD<D> &o) const {
      IMP_USAGE_CHECK(get_dimension() == o.get_dimension(),
                      "Dimensions don't match");
      return internal::lexicographical_compare(begin(), end(),
                                               o.begin(), o.end());
    }
  public:
    //! Create a grid cell from three arbitrary indexes
    ExtendedGridIndexD(Ints vals) {
      data_.set_coordinates(vals.begin(), vals.end());
    }
#ifndef SWIG
    template <class It>
    ExtendedGridIndexD(It b, It e) {
      data_.set_coordinates(b,e);
    }
#endif
    ExtendedGridIndexD(int x, int y, int z) {
      IMP_USAGE_CHECK(D==3, "Can only use explicit constructor in 3D");
      int v[]={x,y,z};
      data_.set_coordinates(v, v+3);
    }
    ExtendedGridIndexD() {
    }
    unsigned int get_dimension() const {
      return data_.get_dimension();
    }
    IMP_COMPARISONS(ExtendedGridIndexD);
    //! Get the ith component (i=0,1,2)
    IMP_CONST_BRACKET(int, unsigned int,
                      i <get_dimension(),
                      IMP_USAGE_CHECK(!data_.get_is_null(),
                                      "Using uninitialized grid index");
                      return data_.get_data()[i]);
    IMP_SHOWABLE_INLINE(ExtendedGridIndexD, {
        out << "(";
        for (unsigned int i=0; i< get_dimension(); ++i) {
          out<< operator[](i);
          if (i != get_dimension()-1) out << ", ";
        }
        out << ")";
      });

#ifndef SWIG
    typedef const int* iterator;
    iterator begin() const {return data_.get_data();}
    iterator end() const {return data_.get_data()+get_dimension();}
#endif
#ifndef IMP_DOXYGEN
    unsigned int __len__() const { return get_dimension();}
#endif
    IMP_HASHABLE_INLINE(ExtendedGridIndexD,
                        return boost::hash_range(begin(), end()));
    ExtendedGridIndexD<D> get_uniform_offset(int ii) const {
      ExtendedGridIndexD<D> ret;
      for (unsigned int i=0; i< get_dimension(); ++i) {
        ret.data_.get_data()[i]= operator[](i)+ii;
      }
      //std::cout << "Offset " << *this << " to get " << ret << std::endl;
      return ret;
    }
    ExtendedGridIndexD<D> get_offset(int i, int j, int k) const {
      IMP_USAGE_CHECK(D==3, "Only for 3D");
      int v[]={operator[](0)+i,
               operator[](1)+j,
               operator[](2)+k};
      ExtendedGridIndexD<D> ret(v, v+3);
      return ret;
    }
  };

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template < int D>
  inline std::size_t hash_value(const ExtendedGridIndexD<D> &ind) {
    return ind.__hash__();
  }
#endif

  IMP_OUTPUT_OPERATOR_D(ExtendedGridIndexD);











  //! Represent a real cell in a grid (one within the bounding box)
  /* These indexes represent an actual cell in the grid.
     They can only be constructed by the grid (since only it knows what
     are the actual cells).
     \see Grid3D
  */
  template <int D>
  class GridIndexD
  {
    internal::VectorData<int, D, true> data_;
    int compare(const GridIndexD<D> &o) const {
      IMP_USAGE_CHECK(get_dimension() == o.get_dimension(),
                      "Dimensions don't match");
      return internal::lexicographical_compare(begin(), end(),
                                               o.begin(), o.end());
    }
  public:
    GridIndexD() {
    }

    unsigned int get_dimension() const {return data_.get_dimension();}

#ifndef IMP_DOXYGEN
    //! Get the ith component (i=0,1,2)
    IMP_CONST_BRACKET(int, unsigned int,
                      i < get_dimension(),
                      IMP_USAGE_CHECK(!data_.get_is_null(),
                                      "Using uninitialized grid index");
                      return data_.get_data()[i]);
    IMP_SHOWABLE_INLINE(GridIndexD, {
        out << "(";
        for (unsigned int i=0; i< get_dimension(); ++i) {
          out<< operator[](i);
          if (i != get_dimension()-1) out << ", ";
        }
        out << ")";
      });
#ifndef SWIG
    typedef const int* iterator;
    iterator begin() const {return data_.get_data();}
    iterator end() const {return data_.get_data()+get_dimension();}
    GridIndexD(Ints vals) {
      data_.set_coordinates(vals.begin(), vals.end());
    }
    template <class It>
    GridIndexD(It b, It e) {
      data_.set_coordinates(b,e);
    }
#endif
    unsigned int __len__() const { return get_dimension();}
#endif
    IMP_COMPARISONS(GridIndexD);
    IMP_HASHABLE_INLINE(GridIndexD,
                        return boost::hash_range(begin(), end()));
  };


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template < int D>
  inline std::size_t hash_value(const GridIndexD<D> &ind) {
    return ind.__hash__();
  }
#endif
  IMP_OUTPUT_OPERATOR_D(GridIndexD);







#if !defined(IMP_DOXYGEN)
  typedef GridIndexD<1> GridIndex1D;
  typedef ExtendedGridIndexD<1> ExtendedGridIndex1D;
  typedef std::vector<GridIndex1D> GridIndex1Ds;
  typedef std::vector<ExtendedGridIndex1D> ExtendedGridIndex1Ds;


  typedef GridIndexD<2> GridIndex2D;
  typedef ExtendedGridIndexD<2> ExtendedGridIndex2D;
  typedef std::vector<GridIndex2D> GridIndex2Ds;
  typedef std::vector<ExtendedGridIndex2D> ExtendedGridIndex2Ds;

  typedef GridIndexD<3> GridIndex3D;
  typedef ExtendedGridIndexD<3> ExtendedGridIndex3D;
  typedef std::vector<GridIndex3D> GridIndex3Ds;
  typedef std::vector<ExtendedGridIndex3D> ExtendedGridIndex3Ds;

  typedef GridIndexD<4> GridIndex4D;
  typedef ExtendedGridIndexD<4> ExtendedGridIndex4D;
  typedef std::vector<GridIndex4D> GridIndex4Ds;
  typedef std::vector<ExtendedGridIndex4D> ExtendedGridIndex4Ds;

  typedef GridIndexD<5> GridIndex5D;
  typedef ExtendedGridIndexD<5> ExtendedGridIndex5D;
  typedef std::vector<GridIndex5D> GridIndex5Ds;
  typedef std::vector<ExtendedGridIndex5D> ExtendedGridIndex5Ds;

  typedef GridIndexD<6> GridIndex6D;
  typedef ExtendedGridIndexD<6> ExtendedGridIndex6D;
  typedef std::vector<GridIndex6D> GridIndex6Ds;
  typedef std::vector<ExtendedGridIndex6D> ExtendedGridIndex6Ds;

  typedef GridIndexD<-1> GridIndexKD;
  typedef ExtendedGridIndexD<-1> ExtendedGridIndexKD;
  typedef std::vector<GridIndexKD> GridIndexKDs;
  typedef std::vector<ExtendedGridIndexKD> ExtendedGridIndexKDs;
#endif




  /** The base for storing a grid on all of space (in 3D).
   */
  template <int D>
  class UnboundedGridStorageD {
  public:
    typedef GridIndexD<D> Index;
    typedef ExtendedGridIndexD<D> ExtendedIndex;
    UnboundedGridStorageD(){}
#ifndef IMP_DOXYGEN
    // for swig
    UnboundedGridStorageD(int, int, int){
      IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                      << " with unbounded storage.");
    }
    void set_number_of_voxels(Ints){
      IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                      << " with unbounded storage.");
    }
    unsigned int get_number_of_voxels(int) const {
      IMP_USAGE_CHECK(false, "The method/constructor cannot be used"
                      << " with unbounded storage.");
      return std::numeric_limits<int>::max();
    }
    static bool get_is_bounded() {
      return false;
    }
#endif

    bool get_has_index(const ExtendedGridIndexD<D>& ) const {
      return true;
    }

#ifndef SWIG
#ifndef IMP_DOXYGEN
    typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                           internal::AllItHelp<ExtendedGridIndexD<D>,
                                               ExtendedGridIndexD<D> > >
    ExtendedIndexIterator;
#else
    class ExtendedIndexIterator;
#endif
    ExtendedIndexIterator
    extended_indexes_begin(const ExtendedGridIndexD<D>& lb,
                           const ExtendedGridIndexD<D>& ub) const {
      ExtendedGridIndexD<D> eub=ub.get_uniform_offset(1);
      IMP_INTERNAL_CHECK(internal::get_is_non_empty(lb, eub),
                         "empty range");
      return ExtendedIndexIterator(lb, eub);
    }
    ExtendedIndexIterator
    extended_indexes_end(const ExtendedGridIndexD<D>&,
                         const ExtendedGridIndexD<D>&) const {
      return ExtendedIndexIterator();
    }
#endif
    std::vector<ExtendedGridIndexD<D> >
    get_extended_indexes(const ExtendedGridIndexD<D>& lb,
                         const ExtendedGridIndexD<D>& ub) const {
      return std::vector<ExtendedGridIndexD<D> >(extended_indexes_begin(lb, ub),
                                                 extended_indexes_end(lb, ub));
    }
  };












/** This is a base class for storage types which refer to a bounded number
    of cells.
*/
  template <int D>
  class BoundedGridStorageD {
    ExtendedGridIndexD<D> d_;
  public:
    typedef GridIndexD<D> Index;
    typedef ExtendedGridIndexD<D> ExtendedIndex;
#ifndef IMP_DOXYGEN
    static bool get_is_bounded() {
      return true;
    }
#endif
    BoundedGridStorageD() {
    }
    BoundedGridStorageD(int i, int j, int k) {
      IMP_USAGE_CHECK(D==3, "Can only use 3 constructor for 3D");
      Ints d(3);
      d[0]=i;
      d[1]=j;
      d[2]=k;
      d_=ExtendedGridIndexD<D>(d);
    }
    BoundedGridStorageD(int i, int j) {
      IMP_USAGE_CHECK(D==2, "Can only use 2 constructor for 2D");
      Ints d(2);
      d[0]=i;
      d[1]=j;
      d_=ExtendedGridIndexD<D>(d);
    }
    BoundedGridStorageD(Ints bds) {
      set_number_of_voxels(bds);
    }
    void set_number_of_voxels(Ints bds) {
      IMP_USAGE_CHECK(bds.size() ==D, "Wrong number of dimensions");
      d_=ExtendedGridIndexD<D>(bds);
    }
    //! Return the number of voxels in a certain direction
    unsigned int get_number_of_voxels(unsigned int i) const {
      IMP_INTERNAL_CHECK(i < D, "Only D: "<< i);
      return d_[i];
    }


    /** \name All Index iterators
        The value type is a GridIndexD;
        @{
    */
#ifndef SWIG
#ifdef IMP_DOXYGEN
    class AllIndexIterator;
#else
    typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                                   internal::AllItHelp<ExtendedGridIndexD<D>,
                                                            GridIndexD<D> > >
    AllIndexIterator;
#endif
  AllIndexIterator all_indexes_begin() const {
    return indexes_begin(ExtendedGridIndexD<D>(Ints(D, 0)),
                         d_);
  }
  AllIndexIterator all_indexes_end() const {
    return indexes_end(ExtendedGridIndexD<D>(Ints(D, 0)),
                       d_);
  }
#endif
  std::vector<GridIndexD<D> > get_all_indexes() const {
    std::vector<GridIndexD<D> > ret(all_indexes_begin(),
                                    all_indexes_end());
    return ret;
  }
  /** @} */



    /** \name Index Iterators

        Iterate through a range of actual indexes. The value
        type for the iterator is an GridIndexD<D>.

        The range is defined by a pair of indexes. It includes
        all indexes in the axis aligned box defined by lb
        as the lower corner and the second as the ub. That is, if
        lb is \f$(l_x, l_y, l_z)\f$ and ub is \f$(u_x, u_y, u_z)\f$,
        then the range includes all
        indexes \f$(i_x, i_y, i_z)\f$ such that \f$l_x \leq i_x \leq u_x\f$,
        \f$l_y \leq i_y \leq u_y\f$
        and \f$l_z \leq i_z \leq u_z\f$.

        @{
    */
#ifndef SWIG
#ifndef IMP_DOXYGEN
    typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                                 internal::AllItHelp<ExtendedGridIndexD<D>,
                                                            GridIndexD<D> > >
                           IndexIterator;
    typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                                    internal::AllItHelp<ExtendedGridIndexD<D>,
                                                    ExtendedGridIndexD<D> > >
                           ExtendedIndexIterator;
#else
    class IndexIterator;
    class ExtendedIndexIterator;
#endif
    IndexIterator indexes_begin(const ExtendedGridIndexD<D>& lb,
                                const ExtendedGridIndexD<D>& ub) const {
      ExtendedGridIndexD<D> eub=ub.get_uniform_offset(1);
      std::pair<ExtendedGridIndexD<D>, ExtendedGridIndexD<D> > bp
        = internal::intersect<ExtendedGridIndexD<D> >(lb,eub, d_);
      if (bp.first== bp.second) {
        return IndexIterator();
      } else {
        IMP_INTERNAL_CHECK(internal::get_is_non_empty(bp.first,
                                                      bp.second),
                           "empty range");
        return IndexIterator(bp.first, bp.second);
      }
    }
    IndexIterator indexes_end(const ExtendedGridIndexD<D>&,
                              const ExtendedGridIndexD<D>&) const {
      //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
      return IndexIterator();
    }
  ExtendedIndexIterator
  extended_indexes_begin(const ExtendedGridIndexD<D>& lb,
                         const ExtendedGridIndexD<D>& ub) const {
    ExtendedGridIndexD<D> eub=ub.get_uniform_offset(1);
    return ExtendedIndexIterator(lb, eub);
  }
  ExtendedIndexIterator
  extended_indexes_end(const ExtendedGridIndexD<D>&,
                       const ExtendedGridIndexD<D>&) const {
    //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
    return ExtendedIndexIterator();
  }
#endif

    std::vector<GridIndexD<D> > get_indexes(const ExtendedGridIndexD<D>& lb,
                                    const ExtendedGridIndexD<D>& ub) const {
    return std::vector<GridIndexD<D> >(indexes_begin(lb, ub),
                                       indexes_end(lb, ub));
  }
  std::vector<ExtendedGridIndexD<D> >
  get_extended_indexes(const ExtendedGridIndexD<D>& lb,
                       const ExtendedGridIndexD<D>& ub) const {
    return std::vector<ExtendedGridIndexD<D> >(extended_indexes_begin(lb, ub),
                                               extended_indexes_end(lb, ub));
  }
  /* @} */

  //! Convert a ExtendedIndex into a real Index if possible
  /** The passed index must be part of the grid
   */
    GridIndexD<D> get_index(const ExtendedGridIndexD<D>& v) const {
      IMP_USAGE_CHECK(get_has_index(v), "Passed index not in grid "
                      << v);
      return GridIndexD<D>(v.begin(), v.end());
    }
    //! Return true if the ExtendedIndex is also a normal index value
    bool get_has_index(const ExtendedGridIndexD<D>& v) const {
      for (unsigned int i=0; i< D; ++i) {
        if (v[i] < 0
            || v[i] >= static_cast<int>(get_number_of_voxels(i))) {
          return false;
        }
      }
      return true;
    }
  };





  /** Store a grid as a densely packed set of voxels.
      The mapping from GridIndex3D to index in the data array is:
      \code
      i[2]*BoundedGridStorageD::get_number_of_voxels(0)
      *BoundedGridStorage3D::get_number_of_voxels(1)
      + i[1]*BoundedGridStorage3D::get_number_of_voxels(0)+i[0]
      \endcode
      \see Grid3D
  */
  template <int D, class VT>
  class DenseGridStorageD: public BoundedGridStorageD<D> {
    typedef boost::scoped_array<VT> Data;
    Data data_;
    unsigned int extent_;
    VT default_;

    template <class I>
    unsigned int index(const I &i) const {
        unsigned int ii=0;
        for ( int d=D-1; d >=0; --d) {
          unsigned int cur= i[d];
          for ( int ld= d-1; ld >=0; --ld) {
            cur*=BoundedGridStorageD<D>::get_number_of_voxels(ld);
          }
          ii+=cur;
        }
        IMP_IF_CHECK(USAGE) {
          if (D==3) {
            unsigned int check= i[2]
              *BoundedGridStorageD<D>::get_number_of_voxels(0)
              *BoundedGridStorageD<D>::get_number_of_voxels(1)
              + i[1]*BoundedGridStorageD<D>::get_number_of_voxels(0)+i[0];
            IMP_USAGE_CHECK(check== ii, "Wrong value returned");
          }
        }
        IMP_INTERNAL_CHECK(ii < get_extent(), "Invalid grid index "
                           << i);
        return ii;
    }
#ifndef SWIG
    struct NonDefault {
      VT default_;
      NonDefault(const VT &def): default_(def){}
      template <class P>
      bool operator()(const P &def) const {
        return def.second != default_;
      }
    };
#endif
    unsigned int get_extent() const {
      return extent_;
    }
    void copy_from(const DenseGridStorageD &o) {
      default_= o.default_;
      extent_= o.extent_;
      data_.reset(new VT[extent_]);
      std::copy(o.data_.get(), o.data_.get()+o.extent_,
                data_.get());
    }
  public:
    IMP_COPY_CONSTRUCTOR(DenseGridStorageD);
    typedef VT Value;
    DenseGridStorageD(int i, int j, int k, const VT &def):
      default_(def) {
      Ints dims(3);
      dims[0]=i;
      dims[1]=j;
      dims[2]=k;
      set_number_of_voxels(dims);
    }
    IMP_BRACKET(VT, GridIndexD<D>, true, return data_[index(i)]);
#ifndef IMP_DOXYGEN
    DenseGridStorageD(const VT &def=VT()): extent_(0), default_(def) {
    }
  void set_number_of_voxels(Ints dims) {
    extent_=1;
    for (unsigned int i=0; i < D; ++i) {
      extent_*=dims[i];
    }
    data_.reset(new VT[extent_]);
    std::fill(data_.get(), data_.get()+get_extent(), default_);
    BoundedGridStorageD<D>::set_number_of_voxels(dims);
  }
  static bool get_is_dense() {
    return true;
  }
#endif
  IMP_SHOWABLE_INLINE(DenseGridStorageD,);
  const VT* get_raw_data() const {
    return data_.get();
  }

#ifndef IMP_DOXYGEN
    void add_voxel(const ExtendedGridIndexD<D>&, const VT&) {
    IMP_USAGE_CHECK(false, "Cannot add voxels to dense grid");
  }
#endif
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    VT &get_voxel_always(const ExtendedGridIndexD<D> &i) {
      GridIndexD<D> gi(i.begin(), i.end());
      return operator[](gi);
    }
    const VT &get_value_always(const ExtendedGridIndexD<D> &i) const {
      GridIndexD<D> gi(i.begin(), i.end());
      return operator[](gi);
    }
#endif

    /** \name All voxel iterators
        The value type is VT.
        @{
    */
#ifndef SWIG
    typedef VT* AllVoxelIterator;
    typedef const VT* AllVoxelConstIterator;
    AllVoxelIterator all_voxels_begin() {
      return data_.get();
    }
    AllVoxelIterator all_voxels_end() {
      return data_.get()+get_extent();
    }
    AllVoxelConstIterator all_voxels_begin() const {
      return data_.get();
    }
    AllVoxelConstIterator all_voxels_end() const {
      return data_.get()+get_extent();
    }
#endif
    /** @} */
  };











  /** Store a grid as a sparse set of voxels (only the voxels which have
      been added are actually stored). The
      get_has_index() functions allow one to tell if a voxel has been added.
      \unstable{SparseGridStorageD}

      Base should be one of BoundedGridStorage3D or UnboundedGridStorage3D.
      \see Grid3D
  */
  template <int D, class VT, class Base,
            class Map=typename IMP::internal::Map<GridIndexD<D>, VT> >
  class SparseGridStorageD: public Base {
    typedef Map Data;
    struct GetIndex {
      typedef GridIndexD<D> result_type;
      typedef typename Data::const_iterator::value_type argument_type;
      template <class T>
      GridIndexD<D> operator()(const T&t) const {
        return t.first;
      }
    };
    struct ItHelper {
      const SparseGridStorageD<D, VT, Base> *stor_;
      ItHelper(const SparseGridStorageD<D, VT, Base> *stor): stor_(stor){}
      bool get_is_good(const ExtendedGridIndexD<D> &ei) {
        return stor_->get_has_index(ei);
      }
      typedef GridIndexD<D> ReturnType;
      ReturnType get_return(const ExtendedGridIndexD<D> &ei) const {
        return stor_->get_index(ei);
      }
      ItHelper(): stor_(NULL){}
    };

    Data data_;
    VT default_;
  public:
    typedef VT Value;
    SparseGridStorageD(int i, int j, int k,
                        const VT &def): Base(i,j,k),
                                        default_(def) {
    }
    IMP_SHOWABLE_INLINE(SparseGridStorage3D, out << "Sparse grid with "
                        << data_.size() << " cells set");
    //! Add a voxel to the storage, this voxel will now have a GridIndex3D
    void add_voxel(const ExtendedGridIndexD<D>& i, const VT& gi) {
      IMP_USAGE_CHECK(Base::get_has_index(i), "Out of grid domain "
                      << i);
      data_[GridIndexD<D>(i.begin(), i.end())]=gi;
    }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    SparseGridStorageD(const VT &def): default_(def) {
    }
    static bool get_is_dense() {
      return false;
    }
#endif
    //! Return true if the voxel has been added
    bool get_has_index(const ExtendedGridIndexD<D>&i) const {
      return data_.find(GridIndexD<D>(i.begin(), i.end())) != data_.end();
    }
    //! requires get_has_index(i) is true.
    GridIndexD<D> get_index(const ExtendedGridIndexD<D> &i) const {
      IMP_USAGE_CHECK(get_has_index(i), "Index is not a valid "
                      << "voxel " << i);
      return GridIndexD<D>(i.begin(), i.end());
    }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    VT &get_voxel_always(const ExtendedGridIndexD<D> &i) {
      GridIndexD<D> gi(i.begin(), i.end());
      typename Map::iterator it= data_.find(gi);
      if (it == data_.end()) {
        return data_.insert(std::make_pair(gi, default_)).first->second;
      } else {
        return it->second;
      }
    }
    const VT &get_value_always(const ExtendedGridIndexD<D> &i) const {
      GridIndexD<D> gi(i.begin(), i.end());
      typename Map::const_iterator it= data_.find(gi);
      if (it == data_.end()) {
        return default_;
      } else {
        return it->second;
      }
    }
#endif
    /** \name Operator []
        Operator[] isn't very useful at the moment as it can only
        be used with a cell which has already been set. This
        behavior/the existence of these functions is likely to change.
        @{
    */
    IMP_BRACKET(VT, GridIndexD<D>, true,
                return data_.find(i)->second);
    /** @} */

    /** \name Iterators through set cells
        Iterate through the voxels which have been set. The value
        type is a pair of GridIndex3D and VT.
        @{
    */
#ifndef SWIG
    typedef typename Data::const_iterator AllConstIterator;
    AllConstIterator all_begin() const {
      return data_.begin();
    }
    AllConstIterator all_end() const {
      return data_.end();
    }
#endif

    std::vector<GridIndexD<D> > get_all_indexes() const {
      return std::vector<GridIndexD<D> >
        (boost::make_transform_iterator(all_begin(), GetIndex()),
         boost::make_transform_iterator(all_end(), GetIndex()));
    }
    /** @} */



    /** \name Index Iterators

        Iterate through a range of actual indexes. The value
        type for the iterator is an GridIndex3D.

        The range is defined by a pair of indexes. It includes
        all indexes in the axis aligned box defined by lb
        as the lower corner and the second as the ub. That is, if
        lb is \f$(l_x, l_y, l_z)\f$ and ub is \f$(u_x, u_y, u_z)\f$,
        then the range includes all
        indexes \f$(i_x, i_y, i_z)\f$ such that \f$l_x \leq i_x \leq u_x\f$,
        \f$l_y \leq i_y \leq u_y\f$
        and \f$l_z \leq i_z \leq u_z\f$.

        @{
    */
#ifndef SWIG
#ifndef IMP_DOXYGEN

    typedef internal::GridIndexIterator<ExtendedGridIndexD<D>,
                                        ItHelper > IndexIterator;

#else
    class IndexIterator;
#endif
    IndexIterator indexes_begin(const ExtendedGridIndexD<D>& lb,
                                const ExtendedGridIndexD<D>& ub) const {
      ExtendedGridIndexD<D> eub=ub.get_offset(1,1,1);
      if (lb == ub) {
        return IndexIterator();
      } else {
        IMP_INTERNAL_CHECK(internal::get_is_non_empty(lb, eub),
                           "empty range");
        return IndexIterator(lb, ub, ItHelper(this));
      }
    }
    IndexIterator indexes_end(const ExtendedGridIndexD<D>&,
                              const ExtendedGridIndexD<D>&) const {
      //IMP_INTERNAL_CHECK(lb <= ub, "empty range");
      return IndexIterator();
    }
#endif

    std::vector<GridIndexD<D> >
    get_indexes(const ExtendedGridIndexD<D>& lb,
                const ExtendedGridIndexD<D>& ub) const {
      return std::vector<GridIndexD<D> >(indexes_begin(lb, ub),
                                         indexes_end(lb, ub));
    }
    /** @} */
  };















  //! A voxel grid in d-dimensional space space.
  /** See \ref grids "Grids" for more information.

      \see DenseGridStorage3D
      \see SparseGridStorageD
  */
  template <int D,
            class Storage,
            // swig needs this for some reason
            class Value=typename Storage::Value>
  class GridD: public Storage
  {
  private:
    VectorD<D> origin_;
    VectorD<D> unit_cell_;
    // inverse
    VectorD<D> inverse_unit_cell_;
#ifndef IMP_DOXYGEN
  protected:
    struct GetVoxel {
      mutable GridD<D, Storage> *home_;
      GetVoxel(GridD<D, Storage> *home): home_(home) {}
      typedef Value& result_type;
      typedef const GridIndexD<D>& argument_type;
      result_type operator()(argument_type i) const {
        std::cout << i << std::endl;
        return home_->operator[](i);
      }
    };

    struct ConstGetVoxel {
      const GridD<D, Storage> *home_;
      ConstGetVoxel(const GridD<D, Storage> *home): home_(home) {}
      typedef const Value& result_type;
      typedef const GridIndexD<D>& argument_type;
      result_type operator()(argument_type i) const {
        std::cout << i << std::endl;
        return home_->operator[](i);
      }
    };

    void set_unit_cell(const VectorD<D> &c) {
      unit_cell_=c;
      Floats iuc(get_dimension());
      for (unsigned int i=0; i < get_dimension(); ++i) {
        iuc[i]=1.0/unit_cell_[i];
      }
      inverse_unit_cell_=VectorD<D>(iuc.begin(), iuc.end());
    }
    void set_origin(const VectorD<D> &c) {
      origin_=c;
    }
    template <class O>
    VectorD<D> get_elementwise_product(VectorD<D> v0,
                                       const O &v1) const {
      for (unsigned int i=0; i< get_dimension(); ++i) {
        v0[i]*= v1[i];
      }
      return v0;
    }
    template <class O>
    VectorD<D> get_uniform_offset(const O &v0,
                                  double o) const {
      Floats ret(get_dimension());
      for (unsigned int i=0; i< get_dimension(); ++i) {
        ret[i]= v0[i]+o;
      }
      return VectorD<D>(ret.begin(), ret.end());
    }
#endif
  public:

  //! Initialize the grid
  /** \param[in] xd The number of voxels in the x direction
      \param[in] yd The number of voxels in the y direction
      \param[in] zd The number of voxels in the z direction
      \param[in] bb The bounding box.
      \param[in] def The default value for the voxels

      The origin in the corner 0 of the bounding box.
   */
  GridD(int xd, int yd, int zd,
         const BoundingBoxD<3> &bb,
         Value def=Value()):
    Storage(xd, yd, zd, def),
    origin_(bb.get_corner(0)) {
    IMP_USAGE_CHECK(xd > 0 && yd>0 && zd>0,
                    "Can't have empty grid");
    IMP_USAGE_CHECK(D==3, "Only in 3D");
    set_bounding_box(bb);
  }
  //! Initialize the grid
  /** \param[in] side The side length for the voxels
      \param[in] bb The bounding box. Note that the final bounding
      box might be slightly different as the actual grid size
      must be divisible by the voxel side.
      \param[in] def The default value for the voxels

      The origin in the corner 0 of the bounding box.
   */
  GridD(double side,
         const BoundingBoxD<D> &bb,
         const Value& def=Value()):
    Storage(def) {
    IMP_USAGE_CHECK(Storage::get_is_bounded(),
              "This grid constructor can only be used with bounded grids.");
    IMP_USAGE_CHECK(side>0, "Side cannot be 0");
    VectorD<D> nuc(side, side, side);
    int dd[D];
    for (unsigned int i=0; i< D; ++i ) {
      double bside= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      double d= bside/side;
      double cd= std::ceil(d);
      dd[i]= std::max<int>(1, static_cast<int>(cd));
    }
    Storage::set_number_of_voxels(Ints(dd, dd+D));
    set_unit_cell(side*get_ones_vector_d<D>());
    origin_= bb.get_corner(0);
    IMP_LOG(TERSE, "Constructing grid with side "
            << unit_cell_[0]
            << " from request with " << side << " and " << bb << std::endl);
  }


    //! Construct and infinite grid with the given origin and cell size
    /** You had better use a sparse, unbounded storage (eg
        \c SparseGridStorage3D<VT, UnboundedGridStorage3D>)
    */
    GridD(double side,
          const VectorD<D> &origin,
          const Value& def= Value()):
      Storage(def),
      origin_(origin){
      set_unit_cell(get_ones_vector_kd(origin.get_dimension())*side);
    }
    //! Construct and infinite grid with the cell size
    /** You had better use a sparse, unbounded storage (eg
        \c SparseGridStorage3D<VT, UnboundedGridStorage3D>)
    */
    GridD(double side,
          const Value& def= Value()):
      Storage(def),
      origin_(get_zero_vector_d<D>()){
      set_unit_cell(get_ones_vector_kd(D)*side);
    }
    //! Construct and infinite grid with the cell size and the given dimension
    /** You had better use a sparse, unbounded storage (eg
        \c SparseGridStorage3D<VT, UnboundedGridStorage3D>)
    */
    GridD(double side, unsigned int d, const Value& def= Value()):
      Storage(def),
      origin_(get_zero_vector_kd(d)){
      IMP_USAGE_CHECK(D==-1, "Only for variable dimensional");
      set_unit_cell(get_ones_vector_kd(d)*side);
    }
    //! An empty, undefined grid.
    GridD(): Storage(Value()){
    }
    const VectorD<D> get_origin() const {
      return origin_;
    }

    unsigned int get_dimension() const {
      return get_origin().get_dimension();
    }

    //! Return the unit cell, relative to the origin.
    /** That is, the unit cell is
        \code
        BoundingBoxD<D>(get_zeros_vector_d<D>(),get_unit_cell());
        \endcode
    */
    const VectorD<D>& get_unit_cell() const {
      return unit_cell_;
    }
    /* \name Indexing
       The vector must fall in a valid voxel to get and must be within
       the volume of the grid to set.
       @{
    */
    IMP_BRACKET(Value, VectorD<D>,
                Storage::get_has_index(get_extended_index(i)),
                return Storage::operator[](get_index(get_extended_index(i))));
    /** @} */

#ifdef SWIG
    const Value& __getitem__(const GridIndexD<D> &i) const;
    void __setitem__(const GridIndexD<D> &i, const Value &vt);
#else
    using Storage::__getitem__;
    using Storage::__setitem__;
    using Storage::operator[];
#endif

#ifndef IMP_DOXYGEN
    //! Return the vector (1/u[0], 1/u[1], 1/u[2])
    const VectorD<D>& get_inverse_unit_cell() const {
      return inverse_unit_cell_;
    }
#endif

    // ! Add a voxel to a sparse grid.
    void add_voxel(const VectorD<D>& pt, const Value &vt) {
      IMP_USAGE_CHECK(!Storage::get_is_dense(),
                      "add_voxel() only works on sparse grids.");
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      Storage::add_voxel(ei, vt);
    }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    Value &get_voxel_always(const VectorD<D>& pt) {
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      return Storage::get_voxel_always(ei);
    }
    const Value &
    get_value_always(const VectorD<D>& pt) const {
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      return Storage::get_value_always(ei);
    }
#endif
#ifndef SWIG
    using Storage::get_has_index;
    using Storage::get_index;
    using Storage::add_voxel;
#else
    bool get_has_index(const ExtendedGridIndexD<D>&i) const;
    GridIndexD<D> get_index(const ExtendedGridIndexD<D> &i) const;
    void add_voxel(const ExtendedGridIndexD<D> &i,
                   const Value &vt);
#endif


    //! Return the index that would contain the voxel if the grid extended there
    /** For example vectors below the "lower left" corner of the grid have
        indexes with all negative components. This operation will always
        succeed.
    */
    ExtendedGridIndexD<D> get_extended_index(const VectorD<D>& pt) const {
      boost::scoped_array<int> index(new int[get_dimension()]);
      for (unsigned int i=0; i< get_dimension(); ++i ) {
        float d = pt[i] - origin_[i];
        float fi= d*inverse_unit_cell_[i];
        index[i]= static_cast<int>(std::floor(fi));
      }
      return ExtendedGridIndexD<D>(index.get(), index.get()+get_dimension());
    }
    //! Convert an index back to an extended index
    ExtendedGridIndexD<D> get_extended_index(const GridIndexD<D> &index) const {
      return ExtendedGridIndexD<D>(index.begin(), index.end());
    }


    /** \name Bounding box
        Return the bounding box of the voxel.
        @{
    */
    BoundingBoxD<D> get_bounding_box(const ExtendedGridIndexD<D>& v) const {
      VectorD<D> l=origin_+ get_elementwise_product(get_unit_cell(), v);
      VectorD<D> u=origin_+ get_elementwise_product(get_unit_cell(),
                                                    get_uniform_offset(v,1));
      return BoundingBoxD<D>(l,u);
    }
    BoundingBoxD<D> get_bounding_box(const GridIndexD<D>& v) const {
      return get_bounding_box(ExtendedGridIndexD<D>(v.begin(), v.end()));
    }
    /** @} */

    /** \name Center
        Return the coordinates of the center of the voxel.
        @{
    */
    VectorD<D> get_center(const ExtendedGridIndexD<D>& gi) const {
      return origin_+ get_elementwise_product(get_unit_cell(),
                                              get_uniform_offset(gi, .5));
    }
    VectorD<D> get_center(const GridIndexD<D>& gi) const {
      return origin_+ get_elementwise_product(get_unit_cell(),
                                              get_uniform_offset(gi, .5));
    }
    /** @} */


    BoundingBoxD<D> get_bounding_box() const {
      IMP_USAGE_CHECK(Storage::get_is_bounded(),
                      "Get_bounding_box() with no arguments only works on "
                      << "bounded grids.");
      VectorD<D> top= get_origin();
      for (unsigned int i=0; i< get_dimension(); ++i) {
        top[i]+= get_unit_cell()[i]*Storage::get_number_of_voxels(i);
      }
      return BoundingBoxD<D>(get_origin(), top);
    }

    //! Change the bounding box but not the grid or contents
    /** The origin is set to corner 0 of the new bounding box and the grid
        voxels are resized as needed.
    */
    void set_bounding_box(const BoundingBoxD<D> &bb3) {
      Floats nuc(get_dimension());
      for (unsigned int i=0; i< get_dimension(); ++i) {
        double side= bb3.get_corner(1)[i]- bb3.get_corner(0)[i];
        IMP_USAGE_CHECK(side>0, "Can't have flat grid");
        nuc[i]= side/Storage::get_number_of_voxels(i);
      }
      set_unit_cell(VectorD<D>(nuc.begin(), nuc.end()));
      set_origin(bb3.get_corner(0));
    }

    /** \name Get nearest
      If the point is in the bounding box of the grid, this is the index
      of the voxel containing the point,
      otherwise it is the closest one in the bounding box. This can only be
      used with bounded grids, right now.
      @{
   */
    GridIndexD<D> get_nearest_index(const VectorD<D>& pt) const {
      IMP_USAGE_CHECK(Storage::get_is_dense(), "get_nearest_index "
                      << "only works on dense grids.");
      ExtendedGridIndexD<D> ei= get_nearest_extended_index(pt);
      return get_index(ei);
    }
    ExtendedGridIndexD<D>
    get_nearest_extended_index(const VectorD<D>& pt) const {
      IMP_USAGE_CHECK(Storage::get_is_bounded(), "get_nearest_index "
                      << "only works on bounded grids.");
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      boost::scoped_array<int> is(new int[get_dimension()]);
      for (unsigned int i=0; i< get_dimension(); ++i) {
        is[i]= std::max(0, ei[i]);
        is[i]= std::min<int>(Storage::get_number_of_voxels(i)-1, is[i]);
      }
      return ExtendedGridIndexD<3>(is.get(), is.get()+get_dimension());
    }
  /** @} */

    /** \name Voxel iterators

        These iterators go through a range of voxels in the grid. These voxels
        include any that touch or are contained in the shape passed to the
        begin/end calls.
        @{
    */
#ifndef SWIG
#ifdef IMP_DOXYGEN
    class VoxelIterator;
    class VoxelConstIterator;
#else
    typedef boost::transform_iterator<GetVoxel, typename Storage::IndexIterator>
    VoxelIterator;
    typedef boost::transform_iterator<ConstGetVoxel,
                                      typename Storage::IndexIterator>
    VoxelConstIterator;
#endif
    VoxelIterator voxels_begin(const BoundingBoxD<D> &bb) {
      ExtendedGridIndexD<3> lb= get_extended_index(bb.get_corner(0));
      ExtendedGridIndexD<3> ub= get_extended_index(bb.get_corner(1));
      return VoxelIterator(Storage::indexes_begin(lb, ub), GetVoxel(this));
    }
    VoxelIterator voxels_end(const BoundingBoxD<3> &) {
      //ExtendedIndex lb= get_extended_index(bb.get_corner(0));
      //ExtendedIndex ub= get_extended_index(bb.get_corner(1));
      return VoxelIterator(Storage::indexes_end(ExtendedGridIndexD<3>(),
                                                ExtendedGridIndexD<3>()),
                           GetVoxel(this));
    }

    VoxelConstIterator voxels_begin(const BoundingBoxD<D> &bb) const {
      ExtendedGridIndexD<3> lb= get_extended_index(bb.get_corner(0));
      ExtendedGridIndexD<3> ub= get_extended_index(bb.get_corner(1));
      return VoxelConstIterator(Storage::indexes_begin(lb, ub),
                                ConstGetVoxel(this));
    }
    VoxelConstIterator voxels_end(const BoundingBoxD<D> &bb) const {
      ExtendedGridIndexD<3> lb= get_extended_index(bb.get_corner(0));
      ExtendedGridIndexD<3> ub= get_extended_index(bb.get_corner(1));
      return VoxelConstIterator(Storage::indexes_end(ExtendedGridIndexD<3>(),
                                                     ExtendedGridIndexD<3>()),
                                ConstGetVoxel(this));
    }
#endif
    /** @} */
  };

} // namespace grids

// They are created with %template in swig to get around inclusion order issues
#ifndef SWIG

/** A sparse, infinite grid of values. In python SparseUnboundedIntGrid3D
    is provided.*/
template <int D, class VT>
struct SparseUnboundedGridD:
  public grids::GridD<D, grids::SparseGridStorageD<D, VT,
                                           grids::UnboundedGridStorageD<D> > >{
  typedef grids::GridD<D, grids::SparseGridStorageD<D, VT,
                                         grids::UnboundedGridStorageD<D> > > P;
  SparseUnboundedGridD(double side,
                       const VectorD<D> &origin,
                       VT def=VT()): P(side, origin, def){}
  SparseUnboundedGridD(){}
  SparseUnboundedGridD(double side, unsigned int d,
                       const VT& def= VT()): P(side, d, def) {}

};

#endif
IMPALGEBRA_END_NAMESPACE



#endif  /* IMPALGEBRA_GRID_D_H */
