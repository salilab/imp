/**
 *  \file internal/union_of_balls.cpp
 *  \brief computation of molecular volumetrics :
 *   surface and area of an union of balls
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/cgal/internal/union_of_balls.h>


IMP_CLANG_PRAGMA(+ diagnostic ignored "-Wconversion")
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Triangulation_cell_base_3.h>
#include <CGAL/Alpha_shape_cell_base_3.h>
//#include <CGAL/Triangulation_data_structure_3.h>
#include <CGAL/Weighted_alpha_shape_euclidean_traits_3.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/version.h>

#if CGAL_VERSION_NR > 1030701000
#include <CGAL/Fixed_alpha_shape_3.h>
#include <CGAL/Fixed_alpha_shape_vertex_base_3.h>
#include <CGAL/Fixed_alpha_shape_cell_base_3.h>
#endif

/* To access specialized predicates for orthocenter localization
 uncomment one of the followings :*/
//#include <CGAL/Regular_triangulation_euclidean_traits_3.h>
//#include <CGAL/Regular_triangulation_filtered_traits_3.h>

#include <CGAL/Alpha_shape_3.h>



IMPCGAL_BEGIN_INTERNAL_NAMESPACE

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef K::Point_3 Point;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;
typedef K::Line_3 Line;
typedef K::Triangle_3 Triangle3;

//typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
#if CGAL_VERSION_NR > 1030701000
typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
typedef CGAL::Fixed_alpha_shape_vertex_base_3<Gt> Vb;
typedef CGAL::Fixed_alpha_shape_cell_base_3<Gt> Fb;
typedef CGAL::Triangulation_data_structure_3<Vb, Fb> TDS;
typedef CGAL::Regular_triangulation_3<Gt, TDS> Triangulation;
typedef CGAL::Fixed_alpha_shape_3<Triangulation> Alpha_shape;
#else
typedef CGAL::Weighted_alpha_shape_euclidean_traits_3<K> Gt;
typedef CGAL::Triangulation_vertex_base_3<Gt> Vf;
typedef CGAL::Alpha_shape_vertex_base_3<Gt, Vf> Vb;

typedef CGAL::Triangulation_cell_base_3<Gt> Df;
typedef CGAL::Alpha_shape_cell_base_3<Gt, Df> Fb;
typedef CGAL::Triangulation_data_structure_3<Vb,Fb> Tds;
typedef CGAL::Regular_triangulation_3<Gt,Tds> Triangulation;

typedef CGAL::Alpha_shape_3<Triangulation> Alpha_shape;

#endif

typedef Gt::Point Wpoint;
typedef Alpha_shape::Cell Cell;
typedef Alpha_shape::Vertex Vertex;
typedef Alpha_shape::Edge Edge;
typedef Alpha_shape::Facet Facet;
typedef Alpha_shape::Cell_handle Cell_handle;
typedef Alpha_shape::Vertex_handle Vertex_handle;

typedef Alpha_shape::Cell_circulator Cell_circulator;
typedef Alpha_shape::Locate_type Locate_type;

typedef Alpha_shape::Cell_iterator Cell_iterator;
typedef Alpha_shape::Vertex_iterator Vertex_iterator;
typedef Alpha_shape::Edge_iterator Edge_iterator;


namespace {
  double safe_sqrt(double v) {
    if (v<0) return 0;
    else return std::sqrt(v);
  }
}

/*!
 * \addtogroup volumetrics
 */
//@{
 // begin group volumetrics

/*! This class mainly stores geometric functions for the computation
 * of volumetrics on balls and lines it is parameterized with Gt, a
 * Geometrical traits arguments that will most of the time be
 * somehting like CGAL::Weighted_alpha_shape_euclidean_traits_3<K>
 *
 * \todo should singleton this class, what about static members ?
 * \todo should const every functions
 * \todo should reconsider constructor design (concerns the
 * init of internal some objects)
 * \todo should consider to give access to internal objects via getters
 */
template < typename Gt >
class SpacefillingVolumetric
{
public :
 // types
 typedef typename Gt::Vector_3 Vector;
 typedef typename Gt::Point_3 Point;
 typedef typename Gt::Point Wpoint;


public:
 /*!
 * constructor
 */
 SpacefillingVolumetric();

 /*!
 * trivial destructor
 */
 virtual ~SpacefillingVolumetric(){}

 /*!\name *. rebinds on some Constants
 * A few helpers and bindings with CGAL or MATH.H constants
 */
 //\@{
 // begin name *
 //! the constant Pi=3.1415926...
 //float const PI=3.1415926
 double PI;
 //float const PI=acos(-1.0)
 //! the oposite of Pi
 //#define OP_PI=0.318309892
 double OP_PI;
 //#define OP_PI (1./acos(-1.0))
 //! the oposite of 2Pi
 double OP_2PI;
 //! length of a vector
 //@}
 // end name *

 /***********************************************************
 * A. PREDICATES
 */
 /*!\name A. global predicates and construct objects
 */
 //\@{
 //begin name A

 // a global instanciation for the geometric traits class
 /* a global instanciation for the geometric traits class
 * \todo
 * it shall disappear (brieffly appear in the constructor) when we'll
 * host all that stuff in a class it is only needed to instanciate
 * the 3 construction predicates.
 */
 //Gt gt;

 //! a global function object that constructs the radical center of
 //! some weighted points
 typename Gt::Construct_weighted_circumcenter_3 radicalCenter;
 //! a global function object that computes the radius of the radical
 //! center of some weighted points
 typename Gt::Compute_squared_radius_smallest_orthogonal_sphere_3 radicalRadius;
 //! a global that says if a simplex intersect it's dual support
 /*! needed to decide attachedness see cap_H() and segment_H() functions
 */
 typename Gt::Does_simplex_intersect_dual_support_3 doesIntersectDual;
 // end name A
 //\@}

 /*! \name B. Vectors manipulation
 */
 //@{
 double vector_length(Vector const &);
 //! squared length of a vector
 double vector_squared_length(Vector const &);
 //! normalized vector
 Vector normalize(Vector const &);

 //! normalized cross product
 Vector normalized_cross_product(Vector const &,Vector const &);
 //@}

 /*! \name C. Attachedness compute simplex attachedness as described
 * in \ref pageRefWeighted .  Basicaly these functions are provided by
 * the alpha shape alf package, so that they are not described in \ref
 * pageRefMeasure .  Anyway, as for the orthogonal centers stuff,
 * these are functions related with the alpha shape construction, so
 * it is worse a rebind on CGAL stuff.
 *
 * About attachedness :
 *
 * A k-simplex t is said <b>attached</b> (to it's (k+1)-simplex s, t
 * beeing a facet of s) when t and s appear at the same time in the
 * alpha filter.  A k-simplex t is said attached if it is attached to
 * one of it's parent (k+1)-simplices That is, it is a simplex that is
 * never singular no matter alpha.  Edelsbrunner (see 5.2 in \ref
 * pageRefWeighted) demonstrates that there is a correpondance between
 * "t is attached" and "y_t is conflict free" where y_t is the
 * orthocenter of t and conflicts reffers to the power center
 * propertie (the power of any site out of t to y_t is > 0).
 *
 * For a simple 2D exemple of attachedness, consider three intersectig
 * disks, two of which intersecting inside the third and not outside
 * it (a Gabriel Edge).  In this case, the two disks already
 * intersect, so the segment that links their centers could appear in
 * the dual complex, but since the intersection is hidden by the third
 * ball it do not appear yet in the dual complex (remember that the
 * dual complex refers to intersection of balls clipped in their power
 * cells).  So this edge will appear only when the two balls have
 * inflated enough so that they intesect on the border of the third
 * one...  which also reveals the underlying triangle
 *
 * About hiddenness :
 *
 * A sub-simplex t of s is said to be <b>hidden</b> by s if it is
 * attached to s
 */
 //\@{
 // begin name C
 //! is a point hidden by an edge ?
 bool is_hidden(Wpoint const &,Wpoint const &);
 //! is an edge hidden by a triangle ?
 bool is_hidden(Wpoint const &,Wpoint const &,Wpoint const &);
 //! [NOT YET IMPLEMENTED] is a triangle hidden by a tetrahedron ?
 bool is_hidden(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 // end name C
 //\@}







 /*!\name 14. Inclusion-exclusion formula
 * Correspond to terms of the straight inclusion-exclusion formula
 *
 */
 //\@{
 //! ball volume
 double ball_V(Wpoint const &v);
 //! ball area
 double ball_A(Wpoint const &v);
 //! ball radius
 double ball_R(Wpoint const &v);
 //! squared ball radius
 double ball_R2(Wpoint const &v);
 //! volume of the intersection of two balls
 double ballInter2_V(Wpoint const &,Wpoint const &);
 //! area of the intersection of two balls
 double ballInter2_A(Wpoint const &,Wpoint const &);
 //! length of the intersection of two balls
 double ballInter2_L(Wpoint const &,Wpoint const &);
 //! volume of the intersection of three balls
 double ballInter3_V(Wpoint const &,Wpoint const &,Wpoint const &);
 //! area of the intersection of three balls
 double ballInter3_A(Wpoint const &,Wpoint const &,Wpoint const &);
 //! length of the intersection of three balls
 double ballInter3_L(Wpoint const &,Wpoint const &,Wpoint const &);
 //! volume of the intersection of four balls
 double ballInter4_V(Wpoint const &,Wpoint const &,Wpoint const &,
                     Wpoint const &);
 //! area of the intersection of four balls
 double ballInter4_A(Wpoint const &,Wpoint const &,Wpoint const &,
                     Wpoint const &);
 //! length of the intersection of four balls
 double ballInter4_L(Wpoint const &,Wpoint const &,Wpoint const &,
                     Wpoint const &);
 //! volume of the intersection of four balls less the subsequent half
 //! volumes of the intersections of three spheres
 double ballInter4and3_V(Wpoint const &,Wpoint const &,Wpoint const &,
                         Wpoint const &);
 //! area of the intersection of four balls less the subsequent
 //! half areas of the intersections of three spheres
 double ballInter4and3_A(Wpoint const &,Wpoint const &,Wpoint const &,
                         Wpoint const &);
 //\@}


 /*! \name 15. Decomposable formula
 * Functions that correspond to terms of the decomposable formula.
 * Measures of the intersections of a tetrahedron with some balls
 * centered on it's corners.
 * - a sector is the intersection with one ball
 * - a wedge is the intersection with two balls
 * - a pawn is the intersectino with three balls
 */
 //\@{
 //! volume of a sector
 double sector_V(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! area of a sector
 double sector_A(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! volume of a wedge
 double wedge_V(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! area of a wedge
 double wedge_A(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! length of a wedge
 double wedge_L(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! volume of a pawn
 double pawn_V(Wpoint const &,Wpoint const &,Wpoint const &);
 //! area of a pawn
 double pawn_A(Wpoint const &,Wpoint const &,Wpoint const &);
 //! length of a pawn
 double pawn_L(Wpoint const &,Wpoint const &,Wpoint const &);
 //! tetrahedron volume
 double tetra_V(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //\@}


 /*! \name 16. 3D measures
 * Functions that measure 3D objects, such as caps etc...
 * - a cap is the intersection of a sphere with a half space.
 * It can also be seen as one part of the lentle that is made of the
 * intersection of two spheres
 * - the intersection of two caps along in the weighted Delaunay diagram
 * coincides with
 * that part of the intersection of the three balls that is inside one of the
 * balls's power cell.
 * - the intersection of three caps along in the weighted Delaunay diagram
 *  coincides with
 * one of the six parts that divide the "three facets gumbo" of the
 * intersection of the three balls
 */
 //\@{
 //! volume of a cap
 double cap_V(Wpoint const &, Wpoint const &);
 //! area of a cap
 double cap_A(Wpoint const &, Wpoint const &);
 //! height of a cap
 double cap_H(Wpoint const &, Wpoint const &);
 //! volume intersection of two caps
 double capInter2_V(Wpoint const &,Wpoint const &,Wpoint const &);
 //! area intersection of two caps
 double capInter2_A(Wpoint const &,Wpoint const &,Wpoint const &);
 //! volume intersection of three caps
 double capInter3_V(Wpoint const &,Wpoint const &,Wpoint const &,
                    Wpoint const &);
 //! area intersection of three caps
 double capInter3_A(Wpoint const &,Wpoint const &,Wpoint const &,
                    Wpoint const &);
 //! volume of the intersection of three caps less the subsequent three half
  //! volumes of the intersections of two caps
 double capInter3and2_V(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &);
 //! volume of the intersection of three caps less the subsequent
 //! three half volumes of the intersections of two caps
 double capInter3and2_V(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &,double const surf);
 //! area of the intersection of three caps less the subsequent three
 //! half areass of the intersections of two caps
 double capInter3and2_A(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &);

 //\@}


 /*! \name 17. 2D measures
 * Functions that measure 2D objects, such as disks, segments, etc...
 * - a disk is the intersection of a sphere with a plane
 * - a segment is the intersection (in the plane) of a disk with a half plane
 * (the half plane of another ball's power cell)
 * - it's length is the length of it's bounding arc
 * - it's angle is the angle of the bounding arc
 * - it's height is the measure of how deep is the plane entered in the disk
 * - the intersection of two segments relates to the intersection of
 * two balls of a 3-simplex within the disk swept by the two other
 * balls
 */
 //\@{
 //! disk area
 double disk_A(Wpoint const &,Wpoint const &);
 //! disk length
 double disk_L(Wpoint const &,Wpoint const &);
 //! disk radius
 double disk_R(Wpoint const &,Wpoint const &);
 //! segment area
 double segment_A(Wpoint const &,Wpoint const &,Wpoint const &);
 //! segment angle
 double segment_a(Wpoint const &,Wpoint const &,Wpoint const &);
 //! segment length
 double segment_L(Wpoint const &,Wpoint const &,Wpoint const &);
 //! segment height
 double segment_H(Wpoint const &,Wpoint const &,Wpoint const &);
 //! area of the intersection of two segments
 double segmentInter2_A(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &);
 //! angle of the arc of the intersection of two segments
 double segmentInter2_a(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &);
 //! length of the arc of the intersection of two segments
 double segmentInter2_L(Wpoint const &,Wpoint const &,Wpoint const &,
                        Wpoint const &);
 //\@}


 /*! \name 18. Orthogonal center
 * compute orthogonal centers and dual of a triangle
 *
 * A few definitions
 * - Let the <b>power of a point x to a sphere
 *   P=(p,w_P)</b> be pi_P(x) = |p-x|^2 - w_P^2
 * - two spheres P and Q are said <b>orthogonal</b> when pi_P(Q) = w_Q^2
 * (which can be rewritten |p-q|^2 = w_P^2 + w_Q^2 and obviously
 * implies pi_Q(P) = w_P^2
 * (sometimes it is also rewritten pi(P,Q):=|p-q|^2 - w_P^2 - w_Q^2=0) )
 * - the <b>radical center</b> (Edelsbrunner calls it <b>orthocenter</b>, it is
 *   also known as the <b>weighted circumcenter</b> (cf CGAL))
 * of a set of points P_1,P_2,... , if it exists, is a weighted point R
 * orthogonal to all P_i with minimal radius.
 * - locus of the orthogonal spheres :
 * given a set of spheres P_i, we discuss the set of possible spheres R
 * orthogonal to every P_i
 * - spheres R orthogonal to one sphere P :
 * Writing the equation pi(P,R)=0 and taking any R in the space leads to an
 * equation with one unknown to fix the radius.
 * So, any point in the space is the center of a unique sphere orthogonal to P
 * - spheres R orthogonal to two sphere P,Q :
 * Rewritting pi(P,R) = pi(Q,R) (= 0) in |p-z|^2 - |q-z|^2 = w_P^2 - w_Q^2
 * (which can also be expressed as pi_P(r) = pi_Q(r) ) leads to a linear
 * equation with d (dimension) unknown.
 * That is, the locus for centers of spheres orthogonal to P and Q is a
 * hyperplane, usually reffered as the
 * <b>radical plane</b> or <b>chordale</b> of P and Q. To fix the radius of such
 * orthogonal sphere, just remember pi(Q,R)=0
 * - sphere orthogonal to more spheres :
 * Just remark that R orthogonal to all implies R orthogonal to every pairs.
 * The locus is then the intersection of every hyperplanes taken two by two.
 * Note that these equations are redundant : for three points it is not
 * necessary to consider all pairs, but just two.
 *
 *
 * for more informations refer to some books or collect in publications such as
 * \ref pageRefTopoflip .
 */
 //\@{
 //! orthogonal center of 4 balls
 Point orthoCenter(Wpoint const &,Wpoint const &,Wpoint const &,Wpoint const &);
 //! orthogonal center of 3 balls
 Point orthoCenter(Wpoint const &,Wpoint const &,Wpoint const &);
 //! orthogonal center of 2 balls
 Point orthoCenter(Wpoint const &,Wpoint const &);
 //! dual point of an oriented triangle (surface intersection point of the three
  //! spheres)
 Point triangleDual(Wpoint const &,Wpoint const &,Wpoint const &);
 //\@}


 /*! \name 19. 3D vectors
 * computations related with 3D vectors
 */
 //\@{
 //! solid angle
 double solid_angle(Point const &,Point const &,Point const &,Point const &);
 //! solid angle
 double dihedral_angle(Point const &,Point const &,Point const &,Point const &);
 //! cross product
 Vector cross_prod(Vector const &,Vector const &);
 //! dot product
 double dot_prod(Vector const &,Vector const &);
 //! tells if i sees (j,k,l) in conter clock wise order
 bool ccw_orientation(Point const&,Point const&,Point const&,Point const&);
 //\@}


 /*! \name T. Triangles and balls
 * Computations related with the intersections of triangles and spheres
 * ´Si les triangles avaient un dieu, ils lui donneraient trois cÙtÈs.ª
 * (Montesquieu)
 *
 */
 //\@{
 //! Computes the area of a triangle
 double triangle_A(Point const &i,Point const &j,Point const &k);
 //! Computes the area of a triangle from the size of it's edges (Heron formula)
 double triangle_A(double i,double j,double k);
 //! Computes the squared area of a triangle from the size of it's edges
 double squared_triangle_A(double i,double j,double k);
 //! Computes the unsigned angles of a triangle from it's corners
 void triangle_a(Point const &i,Point const &j,Point const &k,double &I,
                 double &J,double &K);
 //! computes the area of a sector
 double sector_A(Wpoint const &p,double a);
 //! length of a sector
 double sector_L(Wpoint const &p,double a);
 //! computes half the area of the intersection of two disks
 double halfDiskInter2_A(Wpoint const &i,Wpoint const &j);
 //! computes half the area and length of the intersection of two disks
 void halfDiskInter2_A(Wpoint const &i,Wpoint const &j, double &A, double &L);
 //! Computes the area of the part of a triangle that is not in any spheres
 //! composing the triangle
 double triangle_diff_spheres(Wpoint const &i,Wpoint const &j,Wpoint const &k);
 //! Computes the area and length of the part of a triangle that is not in any
 //! spheres composing the triangle
 void triangle_diff_spheres(Wpoint const &i,Wpoint const &j,Wpoint const
                            &k,double &A,double &L);
 //@}
};
//@}




/***********************************************************
 * B. VECTORS
 */
/*! length of a vector
 */
template <typename Gt>
inline double SpacefillingVolumetric<Gt>::vector_length(Vector const &v){
 return sqrt(v.x()*v.x() + v.y()*v.y() + v.z()*v.z());
}

/*! squared length of a vector
 */
template <typename Gt>
inline double
SpacefillingVolumetric<Gt>::vector_squared_length(Vector const &v){
 return v.x()*v.x() + v.y()*v.y() + v.z()*v.z();
}
/*! normalized vector
 */
template <typename Gt>
inline typename SpacefillingVolumetric<Gt>::Vector
SpacefillingVolumetric<Gt>::normalize(Vector const &v){
 return (v / sqrt(v.x()*v.x() + v.y()*v.y() + v.z()*v.z()));
}
/*! normalized cross product
 */
template <typename Gt>
inline typename SpacefillingVolumetric<Gt>::Vector
 SpacefillingVolumetric<Gt>::normalized_cross_product(Vector const &u,
                                                      Vector const &v){
 Vector w = cross_product(u,v);
 return (w / vector_length(w));
}
/***********************************************************
 * C. ATTACHEDNESS
 */

/*! is a vertex hidden by an edge ?
 *
 * Occurs when two spheres are such that the bigger one owns the second one and
 * that when shrinking alpha,
 * the small sphere reduces to a point before it gets out of the bigger one.
 * \param i a first sphere
 * \param j a second sphere
 * \result true if i is hidden by j
 *
 * \todo check if there is nothing easyer,
 * for instance check if the orthocenter o is such that i is inbetween o and j.
 *
 */
template <typename Gt>
inline bool SpacefillingVolumetric<Gt>::is_hidden(Wpoint const &i,
                                                  Wpoint const &j){
// return !doesIntersectDual(i,j);
double dist = vector_squared_length(i-j);
return (0.5 - (j.weight()-i.weight())/(2*dist))<0;
}
/*! is an edge hidden by a sphere ?
 * \param i a sphere
 * \param j a sphere
 * \param k a sphere
 * \return true if k hides the segment ij, that is ij is attached to ijk
 *
 * \todo check correctness of this algorithm : ij is hidden by k if
 * k and the orthocenter o of ijk are on different sides of ij
 * \todo check a ore efficient algorithm : try to avoid the two calls to
 * orthocenter()
 * at the moment the algorithm is a bit greedy : to check if j and o are on the
 * same side
 * I consider their projection o2 on the affine hull of ij, which is the
 * orthocenter of i and j
 * and I consider the sign of the crossproduct from o2 to both other points
 *
 */
template <typename Gt>
inline
bool SpacefillingVolumetric<Gt>::is_hidden(Wpoint const &i,
                                           Wpoint const &j,Wpoint const &k){
// return !doesIntersectDual(i,j,k);
 Point o2=orthoCenter(i,j);
 Point o=orthoCenter(i,j,k);
 return ((k.point()-o2)*(o-o2)<0);
}
/*! is a triangle hidden by a tetrahedron ?
 *
 * When three balls intersect in a third one and not outside.
 */
//bool is_hidden2();


/***********************************************************
 * 14. STRAIGHT INCLUSION-EXCLUSION FORMULA
 */
/*! ball volume
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ball_V( Wpoint const &v ){
 double w=ball_R(v);
//TRACE("ball_V :"<<4*PI*w*w*w/3.0<<" with ball_R(v):"<<w);
 return 4*PI*w*w*w/3.0;
}
/*! ball area
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ball_A( Wpoint const &v ){
 double w=ball_R(v);
 return 4*PI*w*w;
}
/*! ball radius
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ball_R(Wpoint const &v){
 return sqrt(v.weight());
}

/*! squared ball radius
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ball_R2(Wpoint const &v){
 return v.weight();
}

/*! volume of the intersection of two balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter2_V(Wpoint const &i,
                                                Wpoint const &j){
 return cap_V(i,j) + cap_V(j,i);
}
/*! area of the intersection of two balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter2_A(Wpoint const &i,
                                                Wpoint const &j){
 return cap_A(i,j) + cap_A(j,i);
}
/*! length of the intersection of two balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter2_L(Wpoint const &i,
                                                Wpoint const &j){
 return disk_L(i,j);
}
/*! volume of the intersection of three balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter3_V(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k){
 return capInter2_V(i,j,k)+capInter2_V(j,i,k)+capInter2_V(k,i,j);
}
/*! area of the intersection of three balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter3_A(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k){
 return capInter2_A(i,j,k)+capInter2_A(j,i,k)+capInter2_A(k,i,j);
}
/*! length of the intersection of three balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter3_L(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k){
 return segment_L(i,j,k)+segment_L(i,k,j)+segment_L(j,k,i);
}
/*! volume of the intersection of four balls
 * \param i a sphere
 * \param j a sphere
 * \param k a sphere
 * \param l a sphere
 * \return the volume of the intersection of the four spheres
 * The volume is decomposed in four parts and thus computed as the sum of four
 * volumes.
 * These parts are all intersections of three caps.
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter4_V(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k,
                                                Wpoint const &l){
 return capInter3_V(i,j,k,l)+capInter3_V(j,i,k,l)+capInter3_V(k,i,j,l)
   +capInter3_V(l,i,j,k);
}
/*! area of the intersection of four balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter4_A(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k,
                                                Wpoint const &l){
 return capInter3_A(i,j,k,l)+capInter3_A(j,i,k,l)+capInter3_A(k,i,j,l)
   +capInter3_A(l,i,j,k);
}
/*! volume of the intersection of four balls less the four half volumes of the
 * three sphere intersections
 * \param i first sphere
 * \param j second sphere
 * \param k third sphere
 * \param l fourth sphere
 * \return the volume of the intersection of four balls less the four half
 * volumes of the intersection of the three spheres
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter4and3_V(Wpoint const &i,
                                                    Wpoint const &j,
                                                    Wpoint const &k,
                                                    Wpoint const &l){
 return capInter3and2_V(i,j,k,l)+capInter3and2_V(j,i,k,l)
   +capInter3and2_V(k,i,j,l)+capInter3and2_V(l,i,j,k);
}

/*! area of the intersection of four balls less the four half area of the three
 * sphere intersections
 * \param i first sphere
 * \param j second sphere
 * \param k third sphere
 * \param l fourth sphere
 * \return the area of the intersection of four balls less the four half areas
 * of the intersection of the three spheres
 */
template <typename Gt>
inline
double
SpacefillingVolumetric<Gt>::ballInter4and3_A(Wpoint const &i,
                                             Wpoint const &j,
                                             Wpoint const &k,
                                             Wpoint const &l){
 return capInter3and2_A(i,j,k,l)+capInter3and2_A(j,i,k,l)
   +capInter3and2_A(k,i,j,l)+capInter3and2_A(l,i,j,k);
}
/*! length of the intersection of four balls
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::ballInter4_L(Wpoint const &i,
                                                Wpoint const &j,
                                                Wpoint const &k,
                                                Wpoint const &l){
 return segmentInter2_L(i,j,k,l) +segmentInter2_L(i,k,l,j)
   +segmentInter2_L(i,l,j,k)+
 segmentInter2_L(j,k,i,l) +segmentInter2_L(j,l,k,i) +segmentInter2_L(k,l,i,j);
}



/***********************************************************
 * 15. DECOMPOSABLE FORMULA
 */

/*! volume of a sector
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::sector_V(Wpoint const &i,
                                            Wpoint const &j,
                                            Wpoint const &k,
                                            Wpoint const &l){
 double tmp=solid_angle(i,j,k,l)*ball_V(i);
//TRACE(" sector_V: "<<tmp);
 return tmp;
}
/*! area of a sector
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::sector_A(Wpoint const &i,
                                            Wpoint const &j,
                                            Wpoint const &k,
                                            Wpoint const &l){
//TRACE(" sector_A");
 return solid_angle(i,j,k,l)*ball_A(i);
}
/*! volume of a wedge
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::wedge_V(Wpoint const &i,
                                           Wpoint const &j,
                                           Wpoint const &k,Wpoint const &l){
//TRACE(" wedge_V da:"<<dihedral_angle(i,j,k,l)<<" bi:"<<ballInter2_V(i,j));
 return dihedral_angle(i,j,k,l)*ballInter2_V(i,j);
}
/*! area of a wedge
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::wedge_A(Wpoint const &i,
                                           Wpoint const &j,
                                           Wpoint const &k,
                                           Wpoint const &l){
//TRACE(" wedge_A");
 return dihedral_angle(i,j,k,l)*ballInter2_A(i,j);
}
/*! length of a wedge
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::wedge_L(Wpoint const &i,
                                           Wpoint const &j,
                                           Wpoint const &k,
                                           Wpoint const &l){
//TRACE(" wedge_L");
 return dihedral_angle(i,j,k,l)*disk_L(i,j);
}
/*! volume of a pawn
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::pawn_V(Wpoint const &i,
                                          Wpoint const &j,
                                          Wpoint const &k){
//TRACE(" pawn_V ballInter2_V:"<<ballInter2_V(i,j)<<"
// ballInter3_V:"<<ballInter3_V(i,j,k));
 return 0.5*ballInter3_V(i,j,k);
}
/*! area of a pawn
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::pawn_A(Wpoint const &i,
                                          Wpoint const &j,
                                          Wpoint const &k){
//TRACE(" pawn_A");
 return 0.5*ballInter3_A(i,j,k);
}
/*! length of a pawn
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::pawn_L(Wpoint const &i,
                                          Wpoint const &j,
                                          Wpoint const &k){
//TRACE(" pawn_L");
 return 0.5*ballInter3_L(i,j,k);
}
/*! tetrahedron
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::tetra_V(Wpoint const &i,
                                           Wpoint const &j,
                                           Wpoint const &k,
                                           Wpoint const &l){
 typename Gt::Tetrahedron_3 t(i.point(),j.point(),k.point(),l.point());
 return t.volume();
}

/***********************************************************
 * 16. CAPS and 3D
 */
 /*! volume of a cap
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::cap_V(Wpoint const &i, Wpoint const &j){
//TRACE("cap_V ball_R:"<<ball_R(i)<<" cap_A:"<<cap_A(i,j)<<
// "cap_H:"<<cap_H(i,j)<<" disk_A:"<<disk_A(i,j));
 return (ball_R(i)*cap_A(i,j)-(ball_R(i)-cap_H(i,j))*disk_A(i,j))/3.0;
}
/*! area of a cap
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::cap_A(Wpoint const &i, Wpoint const &j){
 return 2*PI*ball_R(i)*cap_H(i,j);
}
/*! height of a cap
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::cap_H(Wpoint const &i, Wpoint const &j){
 return is_hidden(i,j) ?
 (ball_R(i) + vector_length(i.point() - orthoCenter(i,j))):
 (ball_R(i) - vector_length(i.point() - orthoCenter(i,j)));
}
/*! volume intersection of two caps
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter2_V(Wpoint const &i,
                                               Wpoint const &j,
                                               Wpoint const &k){
 return ( ball_R(i)*capInter2_A(i,j,k) -
 (ball_R(i)-cap_H(i,j))*segment_A(i,j,k) -
 (ball_R(i)-cap_H(i,k))*segment_A(i,k,j))/3.0;
}
/*! area intersection of two caps
 * \todo for speedup, the first term can be replaced :
 * in abscence of roundoff errors the two dihedral angles are
 * supposed to be the same
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter2_A(Wpoint const &i,
                                               Wpoint const &j,
                                               Wpoint const &k){
 return 0.5*ball_A(i)
   *(1-dihedral_angle(i.point(),
                      triangleDual(i,j,k),j.point(),k.point())
     -dihedral_angle(i.point(),triangleDual(i,k,j),j.point(),k.point())) -
   2*PI*ball_R(i)*segment_a(i,j,k)*(ball_R(i)-cap_H(i,j)) -
   2*PI*ball_R(i)*segment_a(i,k,j)*(ball_R(i)-cap_H(i,k));
}
/*! volume intersection of the intersection of three caps
 * \param i a first sphere
 * \param j a second sphere
 * \param k a third sphere
 * \param l a fourth sphere
 * \return the volume of the intersection of the three caps :
 * part of i in j <b>inter</b> part of i in k <b>inter</b> part of i in l
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter3_V(Wpoint const &i,
                                               Wpoint const &j,
                                               Wpoint const &k,
                                               Wpoint const &l){
 return (
 ball_R(i)*capInter3_A(i,j,k,l) -
 (ball_R(i)-cap_H(i,j))*segmentInter2_A(i,j,k,l) -
 (ball_R(i)-cap_H(i,k))*segmentInter2_A(i,k,j,l) -
 (ball_R(i)-cap_H(i,l))*segmentInter2_A(i,l,j,k)
 )/3.0;
}
/*! volume intersection of three caps from which are substracted the three
 * subsequent intersections of two caps
 * \param i a first sphere
 * \param j a second sphere
 * \param kk a third sphere
 * \param ll a fourth sphere
 * \return the volume of the intersection of the three caps less the volumes of
 * the three intersections of two caps:
 * part of i in j <b>Inter</b> part of i in k <b>Inter</b> part of i in l
 * less part of i in j <b>Inter</b> part of i in k
 * less part of i in j <b>Inter</b> part of i in l
 * less part of i in k <b>Inter</b> part of i in l
 *
 * The result of this function is the same as
 * capInter3_V(i,j,k,l) - capInter2_V(i,j,k)/2 - capInter2_V(i,j,k)/2 -
 * capInter2_V(i,j,k)/2
 * but it is quicker since it relies on some simplifications in the formula
 * it is thus used to fasten computations see \ref pageTechVol.
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter3and2_V(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &kk,
                                                   Wpoint const &ll){
 Wpoint k,l;
 if (ccw_orientation(i,j,kk,ll)) {k=kk;l=ll;} else {k=ll;l=kk;}
 double Br=ball_R(i);
 double rj,rk,rl;
 rj=disk_R(i,j); rk=disk_R(i,k); rl=disk_R(i,l);
 Point center4,center3ijk,center3ijl,center3ikl,dualijk,dualikj,
   dualijl,dualilj,dualikl,dualilk;
 center4=orthoCenter(i,j,k,l);
 center3ijk=orthoCenter(i,j,k);
 center3ijl=orthoCenter(i,j,l); center3ikl=orthoCenter(i,k,l);
 dualijk=triangleDual(i,j,k); dualijl=triangleDual(i,j,l);
 dualikl=triangleDual(i,k,l);
 dualikj=triangleDual(i,k,j); dualilj=triangleDual(i,l,j);
 dualilk=triangleDual(i,l,k);
 double /*dijk,*/dijl,/*dikl,*/dikj,/*dilj,*/dilk;
 //I could pythagore instead of explicit computations, but since dual are
 // computed for center4 I also use it with center3
 //dijk=vector_length(center4-dualijk) - vector_length(center3ijk-dualijk);
 dijl=vector_length(center4-dualijl) - vector_length(center3ijl-dualijl);
 //dikl=vector_length(center4-dualikl) - vector_length(center3ikl-dualikl);
 dikj=vector_length(center4-dualikj) - vector_length(center3ijk-dualikj);
 //dilj=vector_length(center4-dualilj) - vector_length(center3ijl-dualilj);
 dilk=vector_length(center4-dualilk) - vector_length(center3ikl-dualilk);
 return (
 2*Br*capInter3and2_A(i,j,k,l)
 + (Br-cap_H(i,j)) * ( 2*PI*rj*rj*dihedral_angle(i,j,k,l) +
(rj-segment_H(i,j,k))*dikj + (rj-segment_H(i,j,l))*dijl )
 + (Br-cap_H(i,k)) * ( 2*PI*rk*rk*dihedral_angle(i,k,j,l) +
(rk-segment_H(i,k,j))*dikj + (rk-segment_H(i,k,l))*dilk )
 + (Br-cap_H(i,l)) * ( 2*PI*rl*rl*dihedral_angle(i,l,j,k) +
(rl-segment_H(i,l,j))*dijl + (rl-segment_H(i,l,k))*dilk )
 )/6.0;
}

/*! volume intersection of three caps from which are substracted the three
 * subsequent intersections of two caps
 * \param i a first sphere
 * \param j a second sphere
 * \param kk a third sphere
 * \param ll a fourth sphere
 * \param surf the external surface of the element from wich we want to compute
 * the volume (capInter3and2_A)
 * When this has been computed before it is no use to compute it again.
 * \return the volume of the intersection of the three caps less the volumes of
 * the three intersections of two caps:
 * part of i in j <b>Inter</b> part of i in k <b>Inter</b> part of i in l
 * less part of i in j <b>Inter</b> part of i in k
 * less part of i in j <b>Inter</b> part of i in l
 * less part of i in k <b>Inter</b> part of i in l
 *
 * The result of this function is the same as
 * capInter3_V(i,j,k,l) - capInter2_V(i,j,k)/2 - capInter2_V(i,j,k)/2 -
 * capInter2_V(i,j,k)/2
 * but it is quicker since it relies on some simplifications in the formula
 * it is thus used to quicken computations see \ref pageTechVol.
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter3and2_V(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &kk,
                                                   Wpoint const &ll,
                                                   double const surf){
 Wpoint k,l;
 if (ccw_orientation(i,j,kk,ll)) {k=kk;l=ll;} else {k=ll;l=kk;}
 double Br=ball_R(i);
 double rj,rk,rl;
 rj=disk_R(i,j); rk=disk_R(i,k); rl=disk_R(i,l);
 Point center4,center3ijk,center3ijl,center3ikl,dualijk,dualikj,dualijl,
   dualilj,dualikl,dualilk;
 center4=orthoCenter(i,j,k,l);
 center3ijk=orthoCenter(i,j,k); center3ijl=orthoCenter(i,j,l);
 center3ikl=orthoCenter(i,k,l);
 dualijk=triangleDual(i,j,k); dualijl=triangleDual(i,j,l);
 dualikl=triangleDual(i,k,l);
 dualikj=triangleDual(i,k,j); dualilj=triangleDual(i,l,j);
 dualilk=triangleDual(i,l,k);
 double dijk,dijl,dikl,dikj,dilj,dilk;
 // I could pythagore instead of explicit computations, but since dual are
 // computed for center4 I also use it with center3
 dijk=vector_length(center4-dualijk) - vector_length(center3ijk-dualijk);
 dijl=vector_length(center4-dualijl) - vector_length(center3ijl-dualijl);
 dikl=vector_length(center4-dualikl) - vector_length(center3ikl-dualikl);
 dikj=vector_length(center4-dualikj) - vector_length(center3ijk-dualikj);
 dilj=vector_length(center4-dualilj) - vector_length(center3ijl-dualilj);
 dilk=vector_length(center4-dualilk) - vector_length(center3ikl-dualilk);
 return (
 2*Br*surf
 + (Br-cap_H(i,j)) * ( 2*PI*rj*rj*dihedral_angle(i,j,k,l) +
(rj-segment_H(i,j,k))*dikj + (rj-segment_H(i,j,l))*dijl )
 + (Br-cap_H(i,k)) * ( 2*PI*rk*rk*dihedral_angle(i,k,j,l) +
(rk-segment_H(i,k,j))*dikj + (rk-segment_H(i,k,l))*dilk )
 + (Br-cap_H(i,l)) * ( 2*PI*rl*rl*dihedral_angle(i,l,j,k) +
(rl-segment_H(i,l,j))*dijl + (rl-segment_H(i,l,k))*dilk )
 )/6.0;
}

/*! area of the intersection of three caps
 * \param i a first sphere
 * \param j a second sphere
 * \param kk a third sphere
 * \param ll a fourth sphere
 * \return the area of the spherical part of the intersection of the three
 * caps :
 * part of i in j <b>Inter</b> part of i in k <b>Inter</b> part of i in l
 * \todo looks like the orientation does not matter : I should check that and
 * remove
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter3_A(Wpoint const &i,
                                               Wpoint const &j,
                                               Wpoint const &kk,
                                               Wpoint const &ll){
 Wpoint k,l;
 if (ccw_orientation(i,j,kk,ll)) { k=kk;l=ll; } else { k=ll;l=kk; }
 double A1= 0.5*ball_A(i)*(
 1
 -dihedral_angle(i.point(),triangleDual(i,k,j),k.point(),j.point())
 -dihedral_angle(i.point(),triangleDual(i,k,l),l.point(),k.point())
 -dihedral_angle(i.point(),triangleDual(i,j,l),j.point(),l.point())
 );

 double A2= 2*PI*ball_R(i)*segmentInter2_a(i,j,k,l)*(ball_R(i)-cap_H(i,j));
 double A3= 2*PI*ball_R(i)*segmentInter2_a(i,k,l,j)*(ball_R(i)-cap_H(i,k));
 double A4= 2*PI*ball_R(i)*segmentInter2_a(i,l,j,k)*(ball_R(i)-cap_H(i,l));
 return A1-A2-A3-A4;
}
/*! area of the intersection of three caps from which are substracted the three
 * areas of the subsequent intersections of two caps
 * \param i a first sphere
 * \param j a second sphere
 * \param k a third sphere
 * \param l a fourth sphere
 * \return the area of the spherical part of the intersection of the three caps
 * less the areas of the three susequent spherical parts of the intersections of
 * two caps:
 * part of i in j <b>inter</b> part of i in k <b>inter</b> part of i in l
 * less part of i in j <b>inter</b> part of i in k
 * less part of i in j <b>inter</b> part of i in l
 * less part of i in k <b>inter</b> part of i in l
 *
 * The result of this function is the same as
 * capInter3_A(i,j,k,l) - capInter2_A(i,j,k)/2 - capInter2_A(i,j,k)/2 -
 * capInter2_A(i,j,k)/2
 * but it is quicker since it relies on some simplifications in the formula
 * it is thus used to quicken computations see \ref pageTechVol.
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::capInter3and2_A(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &k,
                                                   Wpoint const &l){
 return 2*PI*ball_R(i)*(
 (ball_R(i)-cap_H(i,j))*dihedral_angle(i,j,k,l)
 +(ball_R(i)-cap_H(i,k))*dihedral_angle(i,k,j,l)
 +(ball_R(i)-cap_H(i,l))*dihedral_angle(i,l,j,k)
 -0.5*ball_R(i)
 );
}

/************************************************************
 * 17. 2D MEASURES
 */
/*! disk area
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::disk_A(Wpoint const &i,Wpoint const &j){
 return PI*disk_R(i,j)*disk_R(i,j);
}
/*! disk length
 * \param i a first sphere
 * \param j a second sphere
 * \return the length of the disk delineated by the intersection of spheres i
 * and j
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::disk_L(Wpoint const &i,Wpoint const &j){
 return 2*PI*disk_R(i,j);
}
/*! disk radius
 * \param i a first sphere
 * \param j a second sphere
 * \return the radius of the disk delineated by intersection of spheres i and j
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::disk_R(Wpoint const &i,Wpoint const &j){
 return safe_sqrt(cap_H(i,j) * (2*ball_R(i)-cap_H(i,j)));
}
/*! segment area
 * \param i a first sphere
 * \param j a second sphere
 * \param k a third sphere
 * \return the area of the disk segment that k delineates in the intersection
 * circle from spheres i and j
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segment_A(Wpoint const &i,
                                             Wpoint const &j,
                                             Wpoint const &k){
 double R=disk_R(i,j);
 double H=(R-segment_H(i,j,k));
 return R*segment_L(i,j,k)*0.5 - H*safe_sqrt(R*R-H*H);
// return (R*segment_L(i,j,k) -
// H*vector_length(triangleDual(i,j,k)-triangleDual(i,k,j)) ) * .5;
}
/*! segment angle
 * \todo speedup : both dihedral angles are theoretically equal, I should
 * compute one and double the result
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segment_a(Wpoint const &i,
                                             Wpoint const &j,
                                             Wpoint const &k){
 return dihedral_angle(i.point(),j.point(),k.point(),triangleDual(i,j,k)) +
 dihedral_angle(i.point(),j.point(),k.point(),triangleDual(i,k,j));
}
/*! segment length
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segment_L(Wpoint const &i,
                                             Wpoint const &j,
                                             Wpoint const &k){
 return segment_a(i,j,k)*disk_L(i,j);
}
/*! segment height
 * \callergraph
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segment_H(Wpoint const &i,
                                             Wpoint const &j,
                                             Wpoint const &k){
 return is_hidden(i,j,k) ?
 (disk_R(i,j) + vector_length(orthoCenter(i,j)-orthoCenter(i,j,k))) :
 (disk_R(i,j) - vector_length(orthoCenter(i,j)-orthoCenter(i,j,k))) ;
}
/*! area of the intersection of two segments
 * \param i a weighted point i
 * \param j a weighted point i
 * \param kk a weighted point i
 * \param ll a weighted point i
 * \return the area of the intersection of the two segments computed as the
 * difference between the disk sector
 * (from the center of the disk along the segment angle) and the two bording
 * triangles
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segmentInter2_A(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &kk,
                                                   Wpoint const &ll){
 Wpoint k,l;
 if (ccw_orientation(i,j,kk,ll)) {k=kk;l=ll;} else {k=ll;l=kk;}
 Point Y=orthoCenter(i,j,k,l);
 return (
 disk_R(i,j)*segmentInter2_L(i,j,k,l) -
 (disk_R(i,j)-segment_H(i,j,k))*vector_length(triangleDual(i,k,j)-Y) -
 (disk_R(i,j)-segment_H(i,j,l))*vector_length(triangleDual(i,j,l)-Y)
 )*0.5;
}
/*! angle of the arc of the intersection of two segments
 * \todo check correctness
 * \warning might be a problem here
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segmentInter2_a(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &k,
                                                   Wpoint const &l){
 return dihedral_angle(i.point(),j.point(),k.point(),triangleDual(i,k,j)) +
 dihedral_angle(i.point(),j.point(),l.point(),triangleDual(i,j,l)) -
 dihedral_angle(i.point(),j.point(),k.point(),l.point());
}
/*! length of the arc of the intersection of two segments
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::segmentInter2_L(Wpoint const &i,
                                                   Wpoint const &j,
                                                   Wpoint const &k,
                                                   Wpoint const &l){
 return segmentInter2_a(i,j,k,l)*disk_L(i,j);
}

/***********************************************************
 * 18. ORTHOGONAL CENTERS
 */
 /*! orthogonal center of 4 balls
 */
template <typename Gt>
inline
typename SpacefillingVolumetric<Gt>::Point
 SpacefillingVolumetric<Gt>::orthoCenter(Wpoint const &p,
                                         Wpoint const &q,
                                         Wpoint const &r,
                                         Wpoint const &s){
 //return Wpoint(radicalCenter(p,q,r,s),radicalRadius(p,q,r,s));
 return radicalCenter(p,q,r,s);
}
/*! orthogonal center of 3 balls
 */
template <typename Gt>
inline
typename SpacefillingVolumetric<Gt>::Point
 SpacefillingVolumetric<Gt>::orthoCenter(Wpoint const &p,
                                         Wpoint const &q,
                                         Wpoint const &r){
 //return Wpoint(radicalCenter(p,q,r),radicalRadius(p,q,r));
 return radicalCenter(p,q,r);
}
/*! orthogonal center of 2 balls
 */
template <typename Gt>
inline
typename SpacefillingVolumetric<Gt>::Point
 SpacefillingVolumetric<Gt>::orthoCenter(Wpoint const &p,Wpoint const &q){
 //return Wpoint(radicalCenter(p,q),radicalRadius(p,q));
 return radicalCenter(p,q);
}
/*! dual point of an oriented triangle
 *
 * Here, the "dual" term has nothing to do with Voronoi - delaunay duality.
 * We are only interested by the intersection of the three spheres P,Q,R
 * that sees their centers p,q,r in a conter clock wise order
 *
 * \todo this function might essentially be used when one needs to compute a
 * distance between a point and a triangledual point
 * in these cases it is mostly possible to replace the explicit vector
 * construction and computation by a simple pythagore :
 * I might track these triangledual() calls and replace such cases by the
 * appropriate pythagore
 *
 */
template <typename Gt>
inline
typename SpacefillingVolumetric<Gt>::Point
 SpacefillingVolumetric<Gt>::triangleDual(Wpoint const &p,
                                          Wpoint const &q,
                                          Wpoint const &r){
 // first, compute radical center of the triangle
 // second, compute vector normal to the triangle
 // third, project radical center on the surface of one of the three spheres
 // (let's take the first)
 //Wpoint radC = orthoCenter(radicalCenter(p,q,r),radicalRadius(p,q,r));
 Point radC = radicalCenter(p,q,r);
 //\todo look if there is something better in CGAL, or if this can't be
 // simplified in another way
 Vector N = cross_product(q.point()-p.point() , r.point()-p.point());
 double S1,S2,S3;
 S1 = dot_prod(radC-p.point(),N);
// S2 = dot_prod(N,N);
 S2 = vector_squared_length(N);
 //S3 = scalar_product(radC-p.point(),radC-p.point());
 S3 = vector_squared_length(radC-p.point());
 return radC + ((safe_sqrt(S1*S1-S3*S2+p.weight()*S2)-S1)/S2)*N;
}

/***********************************************************
 * 19. VECTOR STUFF
 */
/*! computes the solid angle,
 * that is the the surface of the sphere that is swept by the angle
 * we compute it as a proportion of the whole sphere surface : from 0 to 1
 */
 /*
inline double solid_angle(Point const &i,Point const &j,Point const &k,
Point const &l){
 return (0.5 * ( dihedral_angle(i,j,l,k) + dihedral_angle(i,k,j,l) +
 dihedral_angle(i,l,k,j) ) - 0.25);
}*/
/*! solid angle
 * \callergraph
 * Point version
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::solid_angle(Point const &i,
                                               Point const &j,
                                               Point const &k,
                                               Point const &l){
 double tmp = 0.5 * ( dihedral_angle(i,j,l,k) + dihedral_angle(i,k,j,l)
                      + dihedral_angle(i,l,k,j) ) - 0.25;
//TRACE("solid_angle :"<<tmp);
 return tmp;
}
/*! dihedral angle,
 * the angle between two facets sharing an edge
 */
 /*
inline double dihedral_angle(Point const &i,Point const &j,
Point const &k,Point const &l){
 Vector M1 = normalized_cross_product( k-i , k-j );
 Vector M2 = normalized_cross_product( l-i , l-j );
 return (
 OP_PI *
 acos( dot_prod(M1,M2) )
 );
}*/

/*! dihedral angle,
 * Point version
 * \callergraph
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::dihedral_angle(Point const &i,
                                                  Point const &j,
                                                  Point const &k,
                                                  Point const &l){
 Vector M1 = normalized_cross_product( k-i, k-j);
 Vector M2 = normalized_cross_product( l-i, l-j);
//TRACE("dihedral_angle "<<OP_2PI*acos( dot_prod(M1,M2) ));
 return (
 OP_2PI *
 acos( dot_prod(M1,M2) )
 );
}
/*! cross product,
 * rebind from CGAL
 */
template <typename Gt>
inline
typename SpacefillingVolumetric<Gt>::Vector
 SpacefillingVolumetric<Gt>::cross_prod(Vector const &u,
                                        Vector const &v){
 return CGAL::cross_product(u,v);
}
/*! dot product,
 * rebind from CGAL
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::dot_prod(Vector const &u,Vector const &v){
 return u*v;
}
/*! tells if i sees (j,k,l) in conter clock wise order
 */
template <typename Gt>
inline
bool SpacefillingVolumetric<Gt>::ccw_orientation(Point const&i,
                                                 Point const&j,
                                                 Point const&k,
                                                 Point const&l){
 //CGAL considers the viewer on last point, contrary to Edelsbrunner
 return (CGAL::orientation(j,k,l,i)==CGAL::COUNTERCLOCKWISE);
}
/*! tells if i sees (j,k,l) in conter clock wise order
 */
/*bool ccw_orientation(Wpoint const&i,Wpoint const&j,
  Wpoint const&k,Wpoint const&l){
 //CGAL considers the viewer on last point, contrary to Edelsbrunner
 return (CGAL::orientation(j.point().point(),k.point().point(),
 l.point().point(),i.point().point())==CGAL::COUNTERCLOCKWISE);

} */


/***********************************************************
 * T. TRIANGLES STUFF
 */

/*! Computes the area of a triangle
 * \param[in] i a first point composing the triangle
 * \param[in] j a second point composing the triangle
 * \param[in] k a third point composing the triangle
 * \return the area of the triangle
 *
 * basicly, just link this function to the one related with edges length
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::triangle_A(Point const &i,
                                              Point const &j,
                                              Point const &k) {
 return triangle_A(vector_length(i-j),vector_length(k-j),vector_length(i-k));
}

/*! Computes the area of a triangle from the size of it's edges
 * \param[in] i the length of the first edge of the triangle
 * \param[in] j the length of the second edge of the triangle
 * \param[in] k the length of the third edge of the triangle
 * \return the area of the triangle
 *
 * Magical formula :
 *
 * 1/4 sqrt((a+b+c)(-a+b+c)(a-b+c)(a+b-c))
 *
 * Or (Heron formula)
 *
 * sqrt(p (p-a) (p-b) (p-c)) where p is a half perimeter
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::triangle_A(double i,double j,double k) {
// double p=(i+j+k)/2.;
// return sqrt(p*(p-i)*(p-j)*(p-k));
 return sqrt( squared_triangle_A(i,j,k) );
}

/*! Computes the squared area of a triangle from the size of it's edges
 * \param[in] i the length of the first edge of the triangle
 * \param[in] j the length of the second edge of the triangle
 * \param[in] k the length of the third edge of the triangle
 * \return the squared area of the triangle
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::squared_triangle_A(double i,
                                                      double j,double k) {
 double p=(i+j+k)/2.;
 return p*(p-i)*(p-j)*(p-k);
}

/*! Computes the unsigned angles of a triangle
 * \param[in] i a first point composing the triangle
 * \param[in] j a second point composing the triangle
 * \param[in] k a third point composing the triangle
 * \param[out] I the first angle (at point i) of the triangle
 * \param[out] J the second angle (at point j) of the triangle
 * \param[out] K the third angle (at point k) of the triangle
 *
 * I,J,K are passed by references and updated by the application
 *
 * remember that angles are normalized : they are between O and 1 (in
 * revolution)
 *
 * \todo make sure dot prod does not exceed 1
 */
template <typename Gt>
inline
void SpacefillingVolumetric<Gt>::triangle_a(Point const &i,
                                            Point const &j,
                                            Point const &k,double &I,
                                            double &J,double &K) {
// Vector vij=normalize(j-i);
// Vector vik=normalize(k-i);
// Vector vjk=normalize(k-j);
// Vector vji=normalize(i-j);
// I = OP_2PI * acos( dot_prod(vij,vik) );
// J = OP_2PI * acos( dot_prod(vjk,vji) );
 I = OP_2PI * acos( dot_prod(normalize(j-i),normalize(k-i)) );
 J = OP_2PI * acos( dot_prod(normalize(k-j),normalize(i-j)) );
 K=.5-I-J;
}

/*! computes the area of a sector
 * \param[in] p a weighted point
 * \param[in] a a normalized angle (between 0 and 1)
 * \return the area of a sector of angle a from the ball p
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::sector_A(Wpoint const &p,double a) {
 return PI*ball_R2(p)*a;
}


/*! computes the length of a sector
 * \param[in] p a weighted point
 * \param[in] a a normalized angle (between 0 and 1)
 * \return the area of a sector of angle a from the ball p
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::sector_L(Wpoint const &p,double a) {
 return 2*PI*ball_R(p)*a;
}

/*! computes half the area of the intersection of two disks
 * \param[in] i a first disk
 * \param[in] j a second disk
 * \return half the area of the intersection of two coplanar disks issued from i
 * and j,
 * returns 0 when the spheres do not intersect.
 *
 * let a plane cross the two spheres i and j in their centers, we compute half
 * the area of these two delineated disks.
 * This is simply done by considering the triangle i,j,k where k is one of the
 * two two-circles intersections.
 * We then summ areas of both half sectors issued from i and j and substract the
 * triangle area.
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::halfDiskInter2_A(Wpoint const &i,
                                                    Wpoint const &j){
 double A=0; // area
 Vector v=i-j;
 double d = vector_length(v);
 if (ball_R(i)+ball_R(j)-d>0) {
 double d2 = d*d;
// law of cosines states :
// let a triangle with edges a,b,c and angles A,B,C, so we have
// a2 = b2 + c2 - 2bc cos(A)
 double I; // angle at i
 double J; // angle at j
 I = (ball_R2(i)-ball_R2(j)+d2) / (2.*d*ball_R(i));
 J = (ball_R2(j)-ball_R2(i)+d2) / (2.*d*ball_R(j));
 // let's check numerical problems
 //! \remark Patrice does that qith an epsilon that he eventually
 //! substracts or adds to I or J
 if (I < -1) { I=PI; } else if (I > 1) { I=0; } else {I=acos(I);};
 if (J < -1) { J=PI; } else if (J > 1) { J=0; } else {J=acos(J);};
 // remember that the angles I and J are between 0 and Pi, so, the
 // respective normalized angles will be between 0 and 1/2 hence, when
 // computing the sector area as I*ball_R2(i) we have a factor 2 : in
 // fact we compute the double of the area
 A = 0.5*(I*ball_R2(i) + J*ball_R2(j)) - triangle_A(ball_R(i),ball_R(j),d);
 };
 return A;
}

/*! computes half the area and length of the intersection of two disks
 * \param[in] i a first disk
 * \param[in] j a second disk
 * \param[out] A
 * \param[out] L
 * returns 0 for both A and L when the spheres do not intersect.
 *
 * let a plane cross the two spheres i and j in their centers, we compute half
 * the area of these two delineated disks.
 * This is simply done by considering the triangle i,j,k where k is one of the
 * two two-circles intersections.
 * We then summ areas of both half sectors issued from i and j and substract the
 * triangle area.
 */
template <typename Gt>
inline void SpacefillingVolumetric<Gt>::halfDiskInter2_A(Wpoint const &i,
                                                         Wpoint const &j,
                                                         double &A, double &L){
 A=0;
 L=0;
 Vector v=i-j;
 double d = vector_length(v);
 if (ball_R(i)+ball_R(j)-d>0) {
 double d2 = d*d;
// law of cosines states :
// let a triangle with edges a,b,c and angles A,B,C, so we have
// a2 = b2 + c2 - 2bc cos(A)
 double I; // angle at i
 double J; // angle at j
 I = (ball_R2(i)-ball_R2(j)+d2) / (2.*d*ball_R(i));
 J = (ball_R2(j)-ball_R2(i)+d2) / (2.*d*ball_R(j));
 // let's check numerical problems ! \remark Patrice does that qith an
 //epsilon that he eventually substracts or adds to I or J
 if (I < -1) { I=PI; } else if (I > 1) { I=0; } else {I=acos(I);};
 if (J < -1) { J=PI; } else if (J > 1) { J=0; } else {J=acos(J);};
 // remember that the angles I and J are between 0 and Pi, so, the
 // respective normalized angles will be between 0 and 1/2 hence, when
 // computing the sector area as I*ball_R2(i) we have a factor 2 : in
 // fact we compute the double of the area
 A = 0.5*(I*ball_R2(i) + J*ball_R2(j)) - triangle_A(ball_R(i),ball_R(j),d);
 L = I*ball_R(i) + J*ball_R(j);
 };
}


/*! Computes the area of the part of a triangle that is not in the spheres that
 * composes the triangle
 * \param[in] i a first weighted point composing the triangle
 * \param[in] j a second weighted point composing the triangle
 * \param[in] k a third weighted point composing the triangle
 * \return the area of that part that is in the triangle linking the three
 * sphere centers,
 * and that is not in any of the three bording spheres
 *
 * This is computed as an inclusion exclusion formula, see \ref pageTechVolPck.
 * \warning still have to validate (understand these functions)
 */
template <typename Gt>
inline
double SpacefillingVolumetric<Gt>::triangle_diff_spheres(Wpoint const &i,
                                                         Wpoint const &j,
                                                         Wpoint const &k) {
 double I,J,K; // angles at resp. i,j,k
 triangle_a(i,j,k,I,J,K);//! \warning patrice has a condition 'if
 // (redinfo(vertex)!=0) then compute sector area'... I didn't see the point
 return triangle_A(i,j,k)
 - PI*(I*ball_R2(i) + J*ball_R2(j) + K*ball_R2(k))
 + halfDiskInter2_A(i,j) + halfDiskInter2_A(i,k) + halfDiskInter2_A(j,k);
}

/*! Computes the area and length of the part of a triangle that is not in the
 * spheres that composes the triangle
 * \param[in] i a first weighted point composing the triangle
 * \param[in] j a second weighted point composing the triangle
 * \param[in] k a third weighted point composing the triangle
 * \param[out] A a reference to the area
 * \param[out] L a reference to the length
 *
 * These are computed with an inclusion exclusion formula, see \ref
 * pageTechVolPck.
 * \warning still have to validate (understand these functions)
 *
 * \todo consider to write things using general functions such as
 * sector_L etc...
 */
template <typename Gt>
inline
void SpacefillingVolumetric<Gt>::triangle_diff_spheres(Wpoint const &i,
                                                       Wpoint const &j,
                                                       Wpoint const &k,
                                                       double &A,double &L) {
 double I,J,K; // angles at resp. i,j,k
 double Aij,Lij,Aik,Lik,Ajk,Ljk; // Areas and Length of half circles
 // intersection
 triangle_a(i,j,k,I,J,K);
 halfDiskInter2_A(i,j,Aij,Lij);
 halfDiskInter2_A(i,k,Aik,Lik);
 halfDiskInter2_A(j,k,Ajk,Ljk);
 //! \warning patrice has a condition 'if (redinfo(vertex)!=0) then compute
 // sector area'... I didn't see what it meant
 A=triangle_A(i,j,k)
 - PI*(I*ball_R2(i) + J*ball_R2(j) + K*ball_R2(k))
 + Aij + Aik + Ajk;
 L= 2*PI*(I*ball_R(i) + J*ball_R(j) + K*ball_R(k))
 - Lij - Lik - Ljk;
}

template <typename Gt>
SpacefillingVolumetric<Gt>::SpacefillingVolumetric(){
#ifdef M_PI
 PI=M_PI;
 OP_PI=M_1_PI;
#else
 PI=3.1415926;
 OP_PI=(1.0/3.1415926);
#endif
 OP_2PI=(0.5*OP_PI);
 Gt gt;
 radicalCenter=gt.construct_weighted_circumcenter_3_object();
 radicalRadius=gt.compute_squared_radius_smallest_orthogonal_sphere_3_object();
 doesIntersectDual=gt.does_simplex_intersect_dual_support_3_object();
}



namespace {
std::pair<double,double> computeVolumetrics(Alpha_shape const &A){

 SpacefillingVolumetric<Gt> spacefill;
 double volumeAccumulator = 0;
 double surfaceAreaAccumulator = 0;
 double tmpV = 0;
 double tmpA = 0;
 // VERTICES (BALLS)
 {std::list<Vertex_handle> vertices;
#if CGAL_VERSION_NR > 1030701000
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::INTERIOR);
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::REGULAR);
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::SINGULAR);
#else
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::INTERIOR,0);
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::REGULAR,0);
 A.get_alpha_shape_vertices(std::back_inserter(vertices),
                            Alpha_shape::SINGULAR,0);
#endif
 for (std::list<Vertex_handle>::iterator cit
        =vertices.begin();cit!=vertices.end();++cit){
 tmpV += spacefill.ball_V((*cit)->point());
 tmpA += spacefill.ball_A((*cit)->point());
 }
 }
 volumeAccumulator +=tmpV;
 surfaceAreaAccumulator +=tmpA;
 tmpV = 0;
 tmpA = 0;

 // EDGES
 {std::list<Edge> edges;
#if CGAL_VERSION_NR > 1030701000
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::INTERIOR);
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::REGULAR);
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::SINGULAR);
#else
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::INTERIOR,0);
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::REGULAR,0);
 A.get_alpha_shape_edges(std::back_inserter(edges),Alpha_shape::SINGULAR,0);
#endif
 for (std::list<Edge>::iterator cit=edges.begin();cit!=edges.end();++cit){
 tmpV -= spacefill.ballInter2_V(cit->first->vertex(cit->second)->point(),
 cit->first->vertex(cit->third)->point());
 tmpA -= spacefill.ballInter2_A(cit->first->vertex(cit->second)->point(),
 cit->first->vertex(cit->third)->point());
 }
 }
 volumeAccumulator +=tmpV;
 surfaceAreaAccumulator +=tmpA;
 tmpV = 0;
 tmpA = 0;

 // FACETS
 {std::list<Facet> facets;
 // A.get_alpha_shape_facets(std::back_inserter(facets),
   // Alpha_shape::INTERIOR,0);
 // int s1=facets.size();
 // A.get_alpha_shape_facets(std::back_inserter(facets),Alpha_shape::REGULAR,0);
 // int s2=facets.size()-s1;
 // A.get_alpha_shape_facets(std::back_inserter(facets),
   // Alpha_shape::SINGULAR,0);
 // int s3=facets.size()-s1-s2;
 // for (std::list<Facet>::iterator cit=facets.begin();cit!=facets.end();++cit){
 // tmpV +=
   // ballInter3_V(cit->first->vertex((cit->second+1)&3)->point(),
   // cit->first->vertex((cit->second+2)&3)->point(),
   // cit->first->vertex((cit->second+3)&3)->point());
   // tmpA += ballInter3_A(cit->first->vertex((cit->second+1)&3)->point(),
   // cit->first->vertex((cit->second+2)&3)->point(),
   // cit->first->vertex((cit->second+3)&3)->point());
 // }
#if CGAL_VERSION_NR > 1030701000
 A.get_alpha_shape_facets(std::back_inserter(facets),Alpha_shape::SINGULAR);
#else
 A.get_alpha_shape_facets(std::back_inserter(facets),Alpha_shape::SINGULAR,0);
#endif
 for (std::list<Facet>::iterator cit=facets.begin();cit!=facets.end();++cit){
 tmpV += spacefill.ballInter3_V(cit->first->vertex((cit->second+1)&3)->point(),
                                cit->first->vertex((cit->second+2)&3)->point(),
                                cit->first->vertex((cit->second+3)&3)->point());
 tmpA += spacefill.ballInter3_A(cit->first->vertex((cit->second+1)&3)->point(),
                                cit->first->vertex((cit->second+2)&3)->point(),
                                cit->first->vertex((cit->second+3)&3)->point());
 }
 facets.clear();
#if CGAL_VERSION_NR > 1030701000
 A.get_alpha_shape_facets(std::back_inserter(facets),Alpha_shape::REGULAR);

#else
 A.get_alpha_shape_facets(std::back_inserter(facets),Alpha_shape::REGULAR,0);
#endif
 for (std::list<Facet>::iterator cit=facets.begin();cit!=facets.end();++cit){
 tmpV +=
   0.5*spacefill.ballInter3_V(cit->first->vertex((cit->second+1)&3)->point(),
                              cit->first->vertex((cit->second+2)&3)->point(),
                              cit->first->vertex((cit->second+3)&3)->point());
 tmpA +=
   0.5*spacefill.ballInter3_A(cit->first->vertex((cit->second+1)&3)->point(),
                              cit->first->vertex((cit->second+2)&3)->point(),
                              cit->first->vertex((cit->second+3)&3)->point());
 }
 }
 volumeAccumulator += tmpV;
 surfaceAreaAccumulator += tmpA;
 tmpV = 0;
 tmpA = 0;

 // TETRAHEDRA
 {std::list<Cell_handle> cells;
#if CGAL_VERSION_NR > 1030701000
 A.get_alpha_shape_cells(std::back_inserter(cells),Alpha_shape::INTERIOR);
#else
 A.get_alpha_shape_cells(std::back_inserter(cells),Alpha_shape::INTERIOR,0);
#endif
 for (std::list<Cell_handle>::iterator
        cit=cells.begin();cit!=cells.end();++cit){
 // tmpA-= ballInter4_A((*cit)->vertex(0)->point(),(*cit)->vertex(1)->point(),
   // (*cit)->vertex(2)->point(),(*cit)->vertex(3)->point());
 // tmpV-= ballInter4_V((*cit)->vertex(0)->point(),(*cit)->vertex(1)->point(),
   // (*cit)->vertex(2)->point(),(*cit)->vertex(3)->point());
 tmpA -= spacefill.ballInter4and3_A((*cit)->vertex(0)->point(),
                                    (*cit)->vertex(1)->point(),
                                    (*cit)->vertex(2)->point(),
                                    (*cit)->vertex(3)->point());
 tmpV -= spacefill.ballInter4and3_V((*cit)->vertex(0)->point(),
                                    (*cit)->vertex(1)->point(),
                                    (*cit)->vertex(2)->point(),
                                    (*cit)->vertex(3)->point());
 }
 }
 volumeAccumulator += tmpV;
 surfaceAreaAccumulator += tmpA;

 return std::pair<double,double> (volumeAccumulator,surfaceAreaAccumulator);
}
}

std::pair<double, double>
get_surface_area_and_volume(const algebra::Sphere3Ds &ss) {
  base::Vector<Wpoint> myPoints;
  // first of all, if there are no atoms, there is no computation
  if (ss.size() == 0) return std::pair<double,double>(0,0);
  //
  base::Vector<Wpoint> pts;
  for (unsigned int i=0; i< ss.size(); ++i) {
    pts.push_back(Wpoint(Point(ss[i].get_center()[0],
                               ss[i].get_center()[1],
                               ss[i].get_center()[2]),
                         algebra::get_squared(ss[i].get_radius())));
  }
  Triangulation T(pts.begin(), pts.end());

//  Triangulation T(myPoints.begin(), myPoints.end());
  // If needed insert dummy corner points to level the dimension
  if (T.dimension()< 3) {
    algebra::BoundingBoxD<3> bb;
    for (unsigned int i=0; i< ss.size(); ++i) {
      bb+= IMP::algebra::get_bounding_box(ss[i]);
    }
    bb+=1;
    T.insert(Wpoint(Point(bb.get_corner(0)[0],
                          bb.get_corner(0)[1],
                          bb.get_corner(0)[2]),0));
    if (T.dimension() < 3) {
      T.insert(Wpoint(Point(bb.get_corner(0)[0],
                            bb.get_corner(1)[1],
                            bb.get_corner(0)[2]),0));
    }
    if (T.dimension() < 3) {
      T.insert(Wpoint(Point(bb.get_corner(0)[0],
                            bb.get_corner(0)[1],
                            bb.get_corner(1)[2]),0));
    }
  }
#if CGAL_VERSION_NR > 1030701000
  Alpha_shape A(T, 0);
#else
  Alpha_shape A(T, 0, Alpha_shape::GENERAL);
#endif
  std::pair<double, double> dp= computeVolumetrics(A);
  return std::make_pair(dp.second, dp.first);
}

IMPCGAL_END_INTERNAL_NAMESPACE
