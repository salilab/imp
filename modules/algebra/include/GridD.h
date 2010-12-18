/**
 *  \file Grid3D.h   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
  template <unsigned int D>
  class ExtendedGridIndexD {
    int d_[D];
    int compare(const ExtendedGridIndexD<D> &o) const {
      return internal::lexicographical_compare(d_, d_+D,
                                               o.d_, o.d_+D);
    }
  public:
#ifndef IMP_DOXYGEN
    typedef ExtendedGridIndexD This;
#endif
    //! Create a grid cell from three arbitrary indexes
    ExtendedGridIndexD(Ints vals) {
      IMP_USAGE_CHECK(vals.size()==D, "Wrong number of values provided");
      for (unsigned int i=0; i< D; ++i) {
        d_[i]=vals[i];
      }
    }
#ifndef SWIG
    template <class It>
    ExtendedGridIndexD(It b, It e) {
      IMP_USAGE_CHECK(std::distance(b,e)==D, "Wrong number of values provided");
      std::copy(b,e, d_);
    }
#endif
    ExtendedGridIndexD(int x, int y, int z) {
      IMP_USAGE_CHECK(D==3, "Can only use explicit constructor in 3D");
      d_[0]=x;
      d_[1]=y;
      d_[2]=z;
    }
    ExtendedGridIndexD() {
      for (unsigned int i=0; i < D; ++i) {
        d_[i]=std::numeric_limits<int>::max();
      }
    }
    IMP_COMPARISONS;
    //! Get the ith component (i=0,1,2)
    IMP_CONST_BRACKET(int, unsigned int,
                      i <D,
                      IMP_USAGE_CHECK(d_[i] != std::numeric_limits<int>::max(),
                                      "Using uninitialized grid index");
                      return d_[i]);
    IMP_SHOWABLE_INLINE(ExtendedGridIndexD, {
        out << "(";
        for (unsigned int i=0; i< D; ++i) {
          out<< d_[i];
          if (i != D-1) out << ", ";
        }
        out << ")";
      });
#ifndef IMP_DOXYGEN
    static const unsigned int DIMENSION=D;
#endif

#ifndef SWIG
    typedef const int* iterator;
    iterator begin() const {return d_;}
    iterator end() const {return d_+D;}
#endif
#ifndef IMP_DOXYGEN
    unsigned int __len__() const { return D;}
#endif
    IMP_HASHABLE_INLINE(ExtendedGridIndex3D,
                        return boost::hash_range(begin(), end()));
    ExtendedGridIndexD<D> get_uniform_offset(int ii) const {
      ExtendedGridIndexD<D> ret;
      for (unsigned int i=0; i< D; ++i) {
        ret.d_[i]= d_[i]+ii;
      }
      //std::cout << "Offset " << *this << " to get " << ret << std::endl;
      return ret;
    }
    ExtendedGridIndexD<D> get_offset(int i, int j, int k) const {
      IMP_USAGE_CHECK(D==3, "Only for 3D");
      ExtendedGridIndexD<D> ret;
      ret.d_[0]= d_[0]+i;
      ret.d_[1]= d_[1]+j;
      ret.d_[2]= d_[2]+k;
      return ret;
    }
  };

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template <unsigned int D>
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
  template <unsigned int D>
  class GridIndexD
  {
    int d_[D];
    int compare(const GridIndexD<D> &o) const {
      return internal::lexicographical_compare(d_, d_+D,
                                               o.d_, o.d_+D);
    }
  public:
    GridIndexD() {
      for (unsigned int i=0; i < D; ++i) {
        d_[i]=std::numeric_limits<int>::max();
      }
    }


#ifndef IMP_DOXYGEN
    //! Get the ith component (i=0,1,2)
    IMP_CONST_BRACKET(int, unsigned int,
                      i <D,
                      IMP_USAGE_CHECK(d_[i] != std::numeric_limits<int>::max(),
                                      "Using uninitialized grid index");
                      return d_[i]);
    IMP_SHOWABLE_INLINE(GridIndexD, {
        out << "(";
        for (unsigned int i=0; i< D; ++i) {
          out<< d_[i];
          if (i != D-1) out << ", ";
        }
        out << ")";
      });
#ifndef SWIG
    typedef const int* iterator;
    iterator begin() const {return d_;}
    iterator end() const {return d_+D;}
    GridIndexD(Ints vals) {
      IMP_USAGE_CHECK(vals.size()==D, "Wrong number of values provided");
      for (unsigned int i=0; i< D; ++i) {
        d_[i]=vals[i];
      }
    }
    template <class It>
    GridIndexD(It b, It e) {
      IMP_USAGE_CHECK(std::distance(b,e)==D, "Wrong number of values");
      std::copy(b, e, d_);
    }
    static unsigned int get_d() {return D;}
#endif
    unsigned int __len__() const { return D;}
    typedef GridIndexD This;
    static const unsigned int DIMENSION=D;
#endif
    IMP_COMPARISONS;
    IMP_HASHABLE_INLINE(GridIndexD,
                        return boost::hash_range(begin(), end()));
  };


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template <unsigned int D>
  inline std::size_t hash_value(const GridIndexD<D> &ind) {
    return ind.__hash__();
  }
#endif
  IMP_OUTPUT_OPERATOR_D(GridIndexD);







#if !defined(IMP_DOXYGEN)
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
#endif




  /** The base for storing a grid on all of space (in 3D).
   */
  template <unsigned int D>
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
    void set_size(int,int,int){
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

    bool get_has_index(const ExtendedGridIndexD<D>& v) const {
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

















  /** Store a grid as a sparse set of voxels (only the voxels which have
      been added are actually stored). The
      get_has_voxel() functions allow one to tell if a voxel has been added.
      \unstable{SparseGridStorageD}

      Base should be one of BoundedGridStorage3D or UnboundedGridStorage3D.
      \see Grid3D
  */
  template <unsigned int D, class VT, class Base,
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
  template <unsigned int D,
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
      inverse_unit_cell_= VectorD<D>();
      for (unsigned int i=0; i < D; ++i) {
        inverse_unit_cell_[i]=1.0/unit_cell_[i];
      }
    }
    void set_origin(const VectorD<D> &c) {
      origin_=c;
    }
    template <class O>
    VectorD<D> get_elementwise_product(const VectorD<D> &v0,
                                       const O &v1) const {
      VectorD<D> ret;
      for (unsigned int i=0; i< D; ++i) {
        ret[i]= v0[i]*v1[i];
      }
      return ret;
    }
    template <class O>
    VectorD<D> get_uniform_offset(const O &v0,
                                  double o) const {
      VectorD<D> ret;
      for (unsigned int i=0; i< D; ++i) {
        ret[i]= v0[i]+o;
      }
      return ret;
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
    VectorD<3> nuc(side, side, side);
    int dd[3];
    for (unsigned int i=0; i< 3; ++i ) {
      double bside= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      double d= bside/side;
      double cd= std::ceil(d);
      dd[i]= std::max<int>(1, static_cast<int>(cd));
    }
    Storage::set_size(dd[0], dd[1], dd[2]);
    set_unit_cell(algebra::Vector3D(side, side, side));
    origin_= bb.get_corner(0);
    IMP_LOG(TERSE, "Constructing grid with side "
            << unit_cell_[0] << " and box "
            << algebra::BoundingBox3D(origin_,origin_+
                                      algebra::Vector3D(unit_cell_[0]
                                             *Storage::get_number_of_voxels(0),
                                             unit_cell_[1]
                                             *Storage::get_number_of_voxels(1),
                                             unit_cell_[2]
                                         *Storage::get_number_of_voxels(2)))
            << " from request with " << side << " and " << bb << std::endl);
  }


    //! Construct and infinite grid with the given origin and cell size
    /** You had better use a sparse, unbounded storage (eg
        \c SparseGridStorage3D<VT, UnboundedGridStorage3D>)
    */
    GridD(double side,
          const VectorD<D> &origin=get_zero_vector_d<D>(),
          const Value& def= Value()):
      Storage(def),
      origin_(origin){
      set_unit_cell(get_ones_vector_d<D>()*side);
    }
    //! An empty, undefined grid.
    GridD(): Storage(Value()){
    }
    const VectorD<D> get_origin() const {
      return origin_;
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
                Storage::operator[](get_index(i)));
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
    /** Return true if the point falls in a valid grid cell.*/
    bool get_has_index(const VectorD<D>& pt) const {
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      return Storage::get_has_index(ei);
    }
    //! Return the index of the voxel containing the point.
    /** get_has_index() must be true.
     */
    GridIndexD<D> get_index(const VectorD<D>& pt) const {
      ExtendedGridIndexD<D> ei= get_extended_index(pt);
      return Storage::get_index(ei);
    }
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
    GridIndex3D get_index(const ExtendedGridIndexD<D> &i) const;
    void add_voxel(const ExtendedGridIndexD<D> &i,
                   const Value &vt);
#endif


    //! Return the index that would contain the voxel if the grid extended there
    /** For example vectors below the "lower left" corner of the grid have
        indexes with all negative components. This operation will always
        succeed.
    */
    ExtendedGridIndexD<D> get_extended_index(const VectorD<D>& pt) const {
      int index[D];
      for (unsigned int i=0; i< D; ++i ) {
        float d = pt[i] - origin_[i];
        float fi= d*inverse_unit_cell_[i];
        index[i]= static_cast<int>(std::floor(fi));
      }
      return ExtendedGridIndexD<D>(index, index+D);
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
    VectorD<3> get_center(const GridIndexD<D>& gi) const {
      return origin_+ get_elementwise_product(get_unit_cell(),
                                              get_uniform_offset(gi, .5));
    }
    /** @} */


    BoundingBoxD<3> get_bounding_box() const {
      IMP_USAGE_CHECK(Storage::get_is_bounded(),
                      "Get_bounding_box() with no arguments only works on "
                      << "bounded grids.");
      VectorD<D> top= get_origin();
      for (unsigned int i=0; i< D; ++i) {
        top[i]+= get_unit_cell()[i]*Storage::get_number_of_voxels(i);
      }
      return BoundingBoxD<D>(get_origin(), top);
    }

    //! Change the bounding box but not the grid or contents
    /** The origin is set to corner 0 of the new bounding box and the grid
        voxels are resized as needed.
    */
    void set_bounding_box(const BoundingBoxD<D> &bb3) {
      VectorD<D> nuc;
      for (unsigned int i=0; i< D; ++i) {
        double side= bb3.get_corner(1)[i]- bb3.get_corner(0)[i];
        IMP_USAGE_CHECK(side>0, "Can't have flat grid");
        nuc[i]= side/Storage::get_number_of_voxels(i);
      }
      set_unit_cell(nuc);
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
      int is[3];
      for (unsigned int i=0; i< D; ++i) {
        is[i]= std::max(0, ei[i]);
        is[i]= std::min<int>(Storage::get_number_of_voxels(i)-1, is[i]);
      }
      return ExtendedGridIndexD<3>(is, is+D);
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
template <unsigned int D, class VT>
struct SparseUnboundedGridD:
  public grids::GridD<D, grids::SparseGridStorageD<D, VT,
                                           grids::UnboundedGridStorageD<D> > >{
  typedef grids::GridD<D, grids::SparseGridStorageD<D, VT,
                                         grids::UnboundedGridStorageD<D> > > P;
  SparseUnboundedGridD(double side,
                       const VectorD<D> &origin,
                       VT def=VT()): P(side, origin, def){}
  SparseUnboundedGridD(){}

};

#endif
IMPALGEBRA_END_NAMESPACE



#endif  /* IMPALGEBRA_GRID_D_H */
