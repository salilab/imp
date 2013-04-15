/** maximal_cliques.h
 * Afra Zomorodian
 * November 10, 2008
 *
 * Assuming Graph is a BGL graph and that vertex_descriptors have
 * operator< defined on them.  It's OK for Nbhd_graph as it's using
 * vecS to store vertices, and vecS vertex_descriptors are unsigned ints.
 *
 * The Algorithm is I. Koch's from Cazals & Karande,
 * "Reporting maximal cliques:  new insights into an old problem",
 * Inria Report 5615, version 2.  It's IK_* in Figure 2, with * = GX
 * algorithm.  The helper procedures are named to match the report.
 *
 * Note:  Since the algorithm is tail-recursive, the parameters are
 * passed by reference.  This makes the algorithm much more efficient.
 *
 * Note:  The cliques are outputed by vertex_descriptors, NOT
 * the vertex names.  If you need the vertex names (as in file),
 * you need to translate them using the name map.
 *
 * The implementation maintains the invariant of having R, P, X,
 * in sorted order, so output cliques are also in sorted order.
 * TO DO:  Was this in the original algorithm, or did I add it?
 */

#ifndef IMPDOMINO_INTERNAL_MAXIMAL_CLIQUES_H
#define IMPDOMINO_INTERNAL_MAXIMAL_CLIQUES_H

// STL
#include <vector>              // storing R, P, X in sorted vectors
#include <functional>          // binary_function
#include <iterator>            // back_insert_iterator
#include <algorithm>           // sort, binary_search, lower_bound, ...

// BGL
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

namespace _maximal_cliques
{
  // forward declaration
  template< class Graph, class CliqueOutputIterator, class Vector>
  void IK_GX( const Graph& graph, CliqueOutputIterator &out,
              Vector& R, Vector& P, Vector& X);
  template< class Graph, class Vector>
  void GX( const Graph& graph, Vector& P, Vector& X,
           typename boost::graph_traits< Graph>::vertex_descriptor& pivot);

  // Binary Predicate:  Degree_less
  // For std::max_element in GX
  template< class Graph>
  class Degree_less :
    public std::binary_function<
    typename boost::graph_traits< Graph>::vertex_descriptor,
    typename boost::graph_traits< Graph>::vertex_descriptor,
    bool>
  {
  private:
    typedef typename
    boost::graph_traits< Graph>::vertex_descriptor vertex_descriptor;
  public:
    Degree_less( const Graph& graph) : graph_( graph) {}
    bool operator()( vertex_descriptor v1, vertex_descriptor v2) {
      return out_degree( v1, graph_) < out_degree( v2, graph_);
    }
  private:
    const Graph& graph_;
  }; // struct Degree_less

} // namespace _maximal_cliques

// Procedure:  maximal_cliques
// ---------------------------
// Given a BGL graph, outputs the cliques as vectors of
// vertex_descriptors in the output iterator
//
template< class Graph, class CliqueOutputIterator>
inline void maximal_cliques( const Graph& graph, CliqueOutputIterator out)
{
  typedef boost::graph_traits< Graph>               graph_traits;
  typedef typename graph_traits::vertex_descriptor  vertex_descriptor;
  typedef typename graph_traits::vertex_iterator    vertex_iterator;
  typedef base::Vector< vertex_descriptor>           Vector;
  // Empty graphs are out
  if( boost::num_vertices( graph) == 0) return;

  // Initially, P = V[G]
  vertex_iterator first, last;
  tie( first, last) = vertices( graph);
  Vector P( first, last);

  // We keep R, P, X as sorted vectors
  std::sort( P.begin(), P.end());

  // Initially, R = X = {}
  // Declared here as IK_GX parameters are by reference
  Vector R, X;
  _maximal_cliques::IK_GX( graph, out, R, P, X);
}

// Private Function Implementation
namespace _maximal_cliques
{
  template <class It>
  inline void opt_incr(bool incr, It &it) {
    if (incr) ++it;
  }


  // Algorithm IK_GX
  // ---------------
  // Figure 2 in Cazals and Karande, INRIA report No 5615, version 2
  //
  template< class Graph, class CliqueOutputIterator, class Vector>
  void IK_GX( const Graph& graph, CliqueOutputIterator &out,
              Vector& R, Vector& P, Vector& X)
  {
    typedef boost::graph_traits< Graph>               graph_traits;
    typedef typename graph_traits::vertex_descriptor  vertex_descriptor;
    typedef typename graph_traits::vertex_iterator    vertex_iterator;
    typedef typename graph_traits::adjacency_iterator adjacency_iterator;
#ifdef DCLIQUE
    //std::cout << R.size() << " " << P.size() << " " << X.size() << std::endl;
    // need to add depth parameter for std::cout << "[" << depth << "]" << " ";
    std::cout << "|R| = " << R.size() << " ";
    std::cout << "|P| = " << P.size() << " ";
    std::cout << "|X| = " << X.size() << " ";
    std::cout << std::endl;
#endif
    if( P.empty() && X.empty()) {
      // Report R as a maximal clique
      *out++ = R;
    } else {
      // Choose pivot using greedy heuristic GX
      vertex_descriptor pivot;
      _maximal_cliques::GX( graph, P, X, pivot);

      // Go backward heuristic (Afra) - Since we're in a vector
      // and most times, the element is deleted, it does
      // less shifts -- not verified with timing yet.

      for( int ii=P.size()-1; ii>=0; --ii) {
        vertex_descriptor u_i = P[ii];
        // If *i is not a neighbor of pivot
        if( !edge( pivot, u_i, graph).second) {

          // P = P - { u_i }
          // Based on Meyer's Effective STL, Item 28
          // hack added by daniel to avoid incremented end iterator
          P.erase( P.begin()+ii);

          // R_new = R U { u_i }
          Vector R_new( R.begin(), R.end());
          typename Vector::iterator i =
            std::lower_bound( R_new.begin(), R_new.end(), u_i);
          R_new.insert( i, u_i);

          // Establish N[ u_i] as as a sorted vector
          adjacency_iterator nbhd_first, nbhd_last;
          boost::tie( nbhd_first, nbhd_last) = adjacent_vertices( u_i, graph);
          Vector Nu_i( nbhd_first, nbhd_last);
          std::sort( Nu_i.begin(), Nu_i.end());

          // P_new = P \cap N[u_i]
          Vector P_new;
          std::set_intersection( Nu_i.begin(), Nu_i.end(),
                                 P.begin(), P.end(),
                                 std::back_insert_iterator< Vector>( P_new));

          // X_new = X \cap N[u_i]
          Vector X_new;
          if (!X.empty() && !Nu_i.empty()) {
            std::set_intersection( Nu_i.begin(), Nu_i.end(),
                                   X.begin(), X.end(),
                                   std::back_insert_iterator< Vector>( X_new));
          }
          // Recursive call
          _maximal_cliques::IK_GX( graph, out, R_new, P_new, X_new);

          // X = X U { u_i }
          i =std::lower_bound( X.begin(), X.end(), u_i);
          X.insert( i, u_i);
        }
      }
    }
  }

  // Algorithm GX
  //
  // Chooses vertex with highest degree in P U X
  template< class Graph, class Vector>
  void GX( const Graph& graph, Vector& P, Vector& X,
           typename boost::graph_traits< Graph>::vertex_descriptor& pivot)
  {
    typedef boost::graph_traits< Graph>               graph_traits;
    typedef typename graph_traits::vertex_descriptor  vertex_descriptor;
    typedef typename graph_traits::degree_size_type   degree_size_type;

    Degree_less< Graph> degree_less( graph);

    // A bit goto-like but cleaner this way

    // both cannot be empty (invariant from IK_GX)
    if( P.empty()) {
      pivot = *std::max_element( X.begin(), X.end(), degree_less);
      return;
    }
    if( X.empty()) {
      pivot = *std::max_element( P.begin(), P.end(), degree_less);
      return;
    }

    // neither is empty
    vertex_descriptor from_P =
      *std::max_element( P.begin(), P.end(), degree_less);
    vertex_descriptor from_X =
      *std::max_element( X.begin(), X.end(), degree_less);
    pivot = std::max( from_P, from_X, degree_less);
  }

} // namespace _maximal_cliques

IMPDOMINO_END_INTERNAL_NAMESPACE

#endif /* IMPDOMINO_INTERNAL_MAXIMAL_CLIQUES_H */
