/**
 *  \file density_analysis.cpp
 *  \brief tools for analysing density maps
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/density_analysis.h>
#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/connected_components.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::property<boost::vertex_index_t, long>,
                              boost::property<boost::edge_centrality_t, double
                                               > > DensityGraph;
typedef boost::graph_traits<DensityGraph>::vertex_descriptor DGVertex;
typedef boost::graph_traits<DensityGraph>::edge_descriptor DGEdge;

class DensitySegmentationByCommunities {
public:
  //! Constructor
  /**
     \param[input] dmap the map to segment
     \param[input] dens_t only consider voxels above this threshold
   */
  DensitySegmentationByCommunities(em::DensityMap *dmap,
                                   float dens_t):
    dmap_(dmap),dens_t_(dens_t){}
  //! Build the density graph
  /**
     \param[input] edge_threshold two voxels are considered
     neighbors if the difference between their density does
     not exceed this number
   */
  void build_density_graph(float neighbor_threshold);
  //! Graph clustering based on edge betweenness centrality.
  //! It is an iterative algorithm, where in each step it computes
  //! the edge betweenness centrality (via brandes_betweenness_centrality)
  //! and removes the edge with the maximum betweenness centrality.
  IntsList calculate_communities(int num_clusters);
#if 0
  algebra::Vector3Ds get_centers() {
    algebra::Vector3Ds centers;
    DGVertex v;
    for (std::pair<boost::graph_traits<DensityGraph>::vertex_iterator,
                   boost::graph_traits<DensityGraph>::vertex_iterator> be=
           boost::vertices(g_);
         be.first != be.second; ++be.first) {
      v=*be.first;
      centers.push_back(
          dmap_->get_location_by_voxel(node2voxel_ind_[node_index_[v]]));
    }
    return centers;
  }
#endif
  IntsList calculate_connected_components();
 protected:
  Pointer<em::DensityMap> dmap_;
  DensityGraph g_;
  float dens_t_;
  std::vector<float> weights_;
  std::vector<long> node2voxel_ind_;
  boost::property_map<DensityGraph, boost::vertex_index_t>::type node_index_;
  typedef boost::disjoint_sets<int*, int*> DS;
  /*The function object that indicates termination of the algorithm.
    It must be a ternary function object that accepts the maximum centrality,
    the descriptor of the edge that will be removed, and the graph g.
  */
  struct Done {
    typedef double centrality_type;
    int k_;
    std::vector<int> rank_, parent_;
    //rank stores the size of the set, and parent the parent
    /**
       \pram[in] k the maximum number of clusters
       \param[in] n the number of elements
    */
    Done(int k, int n): k_(k), rank_(n), parent_(n){}
    template <class B>
    /**
       \param[input] c the current maximum centrality
       \param[input] e the edge that is being examined
       \param[input] g the graph
       \note The function calculates the new conneted components
             of the system after the edge was removed
       \note We will be removing edges until we have more than k components
    */
    bool operator()(centrality_type c, const B & e, const DensityGraph &g) {
      std::cout << "Done called on " << boost::source(e, g)
                << "--" << boost::target(e, g)
                << ": " << c << std::endl;
      DS ds(&rank_[0], &parent_[0]);
      //Basic initialization of the disjoint-sets structure.
      //Each vertex in the graph g is in its own set.
      boost::initialize_incremental_components(g, ds);
      //The connected components are calculated based on
      //the edges in the graph g and the information is embedded in ds.
      boost::incremental_components(g, ds);
      unsigned int s= ds.count_sets(boost::vertices(g).first,
                                    boost::vertices(g).second);
      //if there are more than k components we are done
      std::cout<<"s:"<<s<<"k:"<<k_<<std::endl;
      return s >= static_cast<unsigned int>(k_);
    }
  };
};

} // namespace


statistics::Histogram get_density_histogram(const em::DensityMap *dmap,
                                            float threshold,int num_bins) {
  statistics::Histogram hist(threshold-em::EPS,dmap->get_max_value()+.1,
                             num_bins);
  for(long v_ind=0;v_ind<dmap->get_number_of_voxels();v_ind++) {
    if (dmap->get_value(v_ind) > threshold) {
      hist.add(dmap->get_value(v_ind));
    }
  }
  return hist;
}
namespace {
void calc_local_bounding_box(
  em::DensityMap *dmap,long v_index,
  int &iminx,int &iminy, int &iminz,
  int &imaxx,int &imaxy, int &imaxz){
  algebra::Vector3D loc;
  loc = dmap->get_location_by_voxel(v_index);
  int x = dmap->get_dim_index_by_location(loc,0);
  int y = dmap->get_dim_index_by_location(loc,1);
  int z = dmap->get_dim_index_by_location(loc,2);
  int nx=dmap->get_header()->get_nx();
  int ny=dmap->get_header()->get_ny();
  int nz=dmap->get_header()->get_nz();
  iminx=x-1;imaxx=x+1;
  iminy=y-1;imaxy=y+1;
  iminz=z-1;imaxz=z+1;
  if (x==0){iminx=0;}
  if (x==(nx-1)){imaxx=nx-1;}
  if (y==0){iminy=0;}
  if (y==(ny-1)){imaxy=ny-1;}
  if (z==0){iminz=0;}
  if (z==(nz-1)){imaxz=nz-1;}
}
}

void DensitySegmentationByCommunities::build_density_graph(float edge_threshold)
{
  typedef base::map<long, DGVertex> NMAP;
  NMAP voxel2node;
  typedef base::map<DGVertex,long> NMAP2;
  NMAP2 node2voxel;
  int nx=dmap_->get_header()->get_nx();
  int ny=dmap_->get_header()->get_ny();
  int nz=dmap_->get_header()->get_nz();
  int ivox,ivox_z,ivox_zy;
  double *data=dmap_->get_data();
  float dens_val;
  DGVertex v,w;
  //add nodes (all voxels above a threshold
  for(int iz=0;iz<nz;iz++){
    ivox_z=iz * nx*ny;
  for(int iy=0;iy<ny;iy++){
    ivox_zy = ivox_z + iy * nx;
    for(int ix=0;ix<nx;ix++){
      ivox=ivox_zy+ix;
      dens_val=data[ivox];
      if (dens_val<dens_t_) continue;
      v=boost::add_vertex(g_);
      node2voxel_ind_.push_back(ivox);
      voxel2node[ivox]=v;
      node2voxel[v]=ivox;
    }}}

  //add edges(nodes corresponding to nieghboring voxels)
  int iminx,iminy,iminz,imaxx,imaxy,imaxz;
  int v_index;
  //  node_index_ = boost::get(boost::vertex_index, g_);

  for (std::pair<boost::graph_traits<DensityGraph>::vertex_iterator,
                 boost::graph_traits<DensityGraph>::vertex_iterator>
    be= boost::vertices(g_);
       be.first != be.second; ++be.first) {
    v=*be.first;
    v_index = node2voxel[v];
    calc_local_bounding_box(
            dmap_,v_index,
            iminx,iminy, iminz,
            imaxx,imaxy, imaxz);
    //get number of neighbors,if less than 3 it is a background
    int nn=0;
    for(int iz=iminz;iz<=imaxz;iz++){
      ivox_z=iz * nx*ny;
      for(int iy=iminy;iy<=imaxy;iy++){
        ivox_zy = ivox_z + iy * nx;
        for(int ix=iminx;ix<=imaxx;ix++){
          ivox=ivox_zy+ix;
          if (v_index==ivox) continue;
          if (voxel2node.find(ivox) == voxel2node.end()) continue;
          ++nn;}}}
    for(int iz=iminz;iz<=imaxz;iz++){
      ivox_z=iz * nx*ny;
      for(int iy=iminy;iy<=imaxy;iy++){
        ivox_zy = ivox_z + iy * nx;
          for(int ix=iminx;ix<=imaxx;ix++){
            ivox=ivox_zy+ix;
          if (v_index==ivox) continue;
            if (data[ivox]<0.000001) continue;
            if (voxel2node.find(ivox) == voxel2node.end()) continue;
            w=voxel2node[ivox];
            if (boost::edge(v,w,g_).second) continue;//edge exists
            //TODO - edge_threshold should be some fraction of var
            if (std::abs(data[v_index]-data[ivox])<edge_threshold){
              boost::add_edge(v,w,g_);
              weights_.push_back(std::abs(data[v_index]-data[ivox]));
            }
          }}}
  }
  IMP_LOG_TERSE("Graph with "<<boost::num_vertices(g_)
          <<" vertices and " << boost::num_edges(g_)<<" edges"<< std::endl);
  IMP_LOG_TERSE("dens t:"<<dens_t_<<std::endl);
}

class clustering_threshold : public boost::bc_clustering_threshold<double>
{
  typedef boost::bc_clustering_threshold<double> inherited;

 public:
  clustering_threshold(double threshold,const DensityGraph& g,
                       bool normalize)
    : inherited(threshold, g, normalize), iter(1) { }

  bool operator()(double max_centrality, DGEdge e,
                  const DensityGraph& g)
  {
    IMP_LOG_TERSE("Iter: " << iter << " Max Centrality: "
            << (max_centrality / dividend) << std::endl);
    ++iter;
    return inherited::operator()(max_centrality, e, g);
  }

 private:
  unsigned int iter;
};

IntsList
  DensitySegmentationByCommunities::calculate_communities(int num_clusters) {
  //calculate the connected components
  std::vector<int> component(num_vertices(g_));
  int num = boost::connected_components(g_, &component[0]);
  //get the largest connected components
  base::map<int,int> max_comp_list;
  for(int i=0;i<num;i++){
    max_comp_list[i]=0;}
  for(int i=0;i<num;i++){
    max_comp_list[component[i]]+=1;}
  int max_comp=0;
  for(int i=0;i<num;i++){
    if (max_comp_list[i]>max_comp_list[max_comp]){max_comp=i;}
  }
  //remove all nodes not path of the first component
  Ints not_first_comp_indexes;
  for(int i=0;i<(int)component.size();i++) {
    if (component[i] != max_comp) not_first_comp_indexes.push_back(i);
  }
  IMP_LOG_VERBOSE("removing vertices, starting with "
          <<boost::num_vertices(g_)<<std::endl);
  boost::graph_traits<DensityGraph>::vertex_iterator vi, vi_end, next;
  boost::tie(vi, vi_end) = boost::vertices(g_);
  std::pair<boost::graph_traits<DensityGraph>::out_edge_iterator,
            boost::graph_traits<DensityGraph>::out_edge_iterator> ei;
  int rank0_nodes=0;
  int index=0;
  for(;vi!=vi_end;vi++) {
    if (not_first_comp_indexes[index] != (int)*vi) continue;
    ++index;
    ++rank0_nodes;
    for(ei=boost::out_edges(*vi,g_);
        ei.first!=ei.second;ei.first++) {
      boost::remove_edge(ei.first,g_);
    }
  }
  IMP_LOG_VERBOSE("rank0 nodes:"<<rank0_nodes
          <<" not in first cc:"<<not_first_comp_indexes.size()<<std::endl);
  boost::property_map<DensityGraph, boost::edge_centrality_t>::type m
    = boost::get(boost::edge_centrality, g_);
  unsigned int n=boost::num_vertices(g_);
  boost::betweenness_centrality_clustering(
                 g_, Done(num_clusters+rank0_nodes, n),
                 m);//we add all the new rank0 nodes
  IMP_LOG_TERSE("preparing output"<<std::endl);
  std::vector<int> rank(n), parent(n);
  DS ds(&rank[0], &parent[0]);
  boost::initialize_incremental_components(g_, ds);
  boost::incremental_components(g_, ds);
  base::map<int, Ints> sets;
  //voxel indexes correspoding to node indexes for each cluster
  for (unsigned int i=0; i< boost::num_vertices(g_); ++i) {
    int s= ds.find_set(i);
    sets[s].push_back(node2voxel_ind_[node_index_[i]]);
  }
  IntsList clusters;
  for (base::map<int, Ints>::const_iterator it
         = sets.begin(); it != sets.end(); ++it) {
    clusters.push_back(it->second);
  }
  return clusters;
}


IntsList
  DensitySegmentationByCommunities::calculate_connected_components() {

  Ints component(num_vertices(g_));
  int num = boost::connected_components(g_, &component[0]);
  Ints::size_type i;
  IntsList cc_inds;
  cc_inds.insert(cc_inds.end(),num,Ints());
  for (i = 0; i != component.size(); ++i) {
    cc_inds[component[i]].push_back(node2voxel_ind_[node_index_[i]]);
  }
  return cc_inds;
}
namespace {
  em::DensityMap *get_segment_by_indexes(em::DensityMap *dmap,Ints inds){
    //create a density map with indexes of the maximum component
    const em::DensityHeader *h=dmap->get_header();
    Pointer<em::DensityMap> ret(new em::DensityMap(*h));
    ret->reset_data(0.);
    em::emreal* ret_data=ret->get_data();
    em::emreal*dmap_data=dmap->get_data();
    for (int i=0;i<(int)inds.size();i++) {
      ret_data[inds[i]]=dmap_data[inds[i]];
    }
    return ret.release();
  }
}
em::DensityMaps density_segmentation(em::DensityMap *dmap,
                                     float dens_t,float edge_threshold,
                                     int num_clusters) {
  DensitySegmentationByCommunities ds(dmap,dens_t);
  ds.build_density_graph(edge_threshold);
  IntsList cc_inds=ds.calculate_communities(num_clusters);
  em::DensityMaps ret;
  for(int i=0;i<num_clusters;i++) {
    ret.push_back(get_segment_by_indexes(dmap,cc_inds[i]));
  }
  return ret;
}

em::DensityMap* remove_background(em::DensityMap *dmap,
                                  float threshold,float edge_threshold) {
  DensitySegmentationByCommunities ds(dmap,threshold);
  ds.build_density_graph(edge_threshold);
  IntsList cc_inds=ds.calculate_connected_components();
  //get the largest cc:
  Ints sizes;
  int max_ind=0;
  for(int i=0;i<(int)cc_inds.size();i++) {
    sizes.push_back(cc_inds[i].size());
    if (i>1){if (sizes[i]>sizes[max_ind]) max_ind=i;}
  }
  return get_segment_by_indexes(dmap,cc_inds[max_ind]);
}

IntsList get_connected_components(
                                          em::DensityMap *dmap,
                                          float threshold,float edge_threshold)
{
  DensitySegmentationByCommunities ds(dmap,threshold);
  ds.build_density_graph(edge_threshold);
  IntsList cc_inds=ds.calculate_connected_components();
  return cc_inds;
}

IMPMULTIFIT_END_NAMESPACE
